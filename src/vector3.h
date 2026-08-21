//--------------------------------------------------------------------------------------------------
//
//	Coordinates: A compile-time c++23 coordinate conversion library based on `units`
//
//--------------------------------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//--------------------------------------------------------------------------------------------------
//
// Copyright (c) 2016 Nic Holthaus
//
//--------------------------------------------------------------------------------------------------
//
// A unit-typed Cartesian 3-vector with full geometric algebra. `Vector3<Unit>` holds three components of a
// `units` quantity and provides the dimensionally-correct operations: addition and subtraction of same-unit
// vectors, scaling by a dimensionless factor, dot and cross products with a vector of ANY unit (the result
// carries the product dimension), magnitude, and normalization (a dimensionless direction). It exposes both
// named `x()/y()/z()` accessors and the std::tuple protocol (`get`, `tuple_size`, `tuple_element`, structured
// bindings), so it is a drop-in for anything that treats a point as a three-element tuple.
//
// `CartesianTuple` is the length-typed face -- `Vector3<meters<>>` -- the position/offset vector the frame and
// coordinate machinery is built on. `CartesianVector` is an ordered sequence of them (a polyline / point list).
//
// Eigen interoperability is opt-in and dependency-free: `toEigen()` / `fromEigen()` are compiled only when
// <Eigen/Core> is on the include path (they map to/from `Eigen::Matrix<Unit, 3, 1>`, which `units/eigen.h`
// teaches Eigen to hold as a dimensioned matrix). Without Eigen the type is fully usable and the seam is absent.
//
//--------------------------------------------------------------------------------------------------

#ifndef vector3_h
#define vector3_h

//------------------------
//	INCLUDES
//------------------------

#include <cstddef>
#include <ostream>
#include <tuple>
#include <vector>

#include <units.h>

// Opt-in Eigen interoperability: inert unless <Eigen/Core> is on the include path. Including units/eigen.h
// (itself __has_include-guarded) teaches Eigen to treat a units quantity as a matrix scalar, so
// Eigen::Matrix<Unit, 3, 1> is a well-formed dimensioned vector.
#if defined __has_include
#if __has_include(<Eigen/Core>)
#include <Eigen/Core>
#include <units/eigen.h>
#define VECTOR3_HAS_EIGEN 1
#endif
#endif

inline namespace coordinates
{
	using namespace units;

	//	----------------------------------------------------------------------------
	//	CLASS		Vector3
	//  ----------------------------------------------------------------------------
	///	@brief		A unit-typed Cartesian 3-vector with dimensionally-correct geometric algebra.
	///	@details	Stores three components of a `units` quantity. Same-unit vectors add and subtract; a vector
	///				scales by a dimensionless factor; the dot and cross products accept a vector of any unit and
	///				carry the product dimension; the magnitude is a `Unit`; normalization yields a dimensionless
	///				direction. Exposes `x()/y()/z()` accessors and the std::tuple protocol, so it substitutes for a
	///				three-element tuple in structured bindings, `std::get`, and `std::make_from_tuple`.
	///	@tparam		Unit	the component quantity type (a `units` unit, e.g. `meters<>`).
	//  ----------------------------------------------------------------------------
	template<class Unit>
	class Vector3
	{
	public:
		static_assert(units::traits::is_unit_v<Unit>, "`Unit` must be a units quantity type.");

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using unit_type   = Unit;                                        ///< the component quantity type
		using scalar_type = decltype(pow<2>(std::declval<Unit>()));      ///< the dimension of a same-unit dot/cross

		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/// The zero vector.
		constexpr Vector3() = default;

		/// From three components.
		constexpr Vector3(Unit x, Unit y, Unit z)
		    : m_x(x)
		    , m_y(y)
		    , m_z(z)
		{
		}

		/// From a `(x, y, z)` tuple of the component unit. Implicit, so a `std::tuple`-returning boundary (the
		/// `lib/` rotation/quaternion/helmert transforms, which are tuple-typed to stay coordinate-agnostic)
		/// flows straight into a `Vector3`. The component unit need only be convertible to `Unit`, so a tuple in
		/// any compatible unit (e.g. kilometers into a meters vector) converts on the way in.
		template<class U>
		    requires(std::convertible_to<U, Unit>)
		constexpr Vector3(const std::tuple<U, U, U>& t)
		    : m_x(std::get<0>(t))
		    , m_y(std::get<1>(t))
		    , m_z(std::get<2>(t))
		{
		}

		/// To a `(x, y, z)` tuple of the component unit -- the converse of the tuple constructor, so a `Vector3`
		/// passes straight into a `std::tuple`-typed boundary (a legacy `Coordinate` whose `Tuple` is spelled as
		/// a `std::tuple`, or a `std::tuple`-typed API).
		[[nodiscard]] constexpr operator std::tuple<Unit, Unit, Unit>() const { return {m_x, m_y, m_z}; }

		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS
		//////////////////////////////////////////////////////////////////////////

		[[nodiscard]] constexpr Unit x() const { return m_x; }
		[[nodiscard]] constexpr Unit y() const { return m_y; }
		[[nodiscard]] constexpr Unit z() const { return m_z; }

		constexpr void setX(Unit x) { m_x = x; }
		constexpr void setY(Unit y) { m_y = y; }
		constexpr void setZ(Unit z) { m_z = z; }

		/// The components as a `(x, y, z)` tuple.
		[[nodiscard]] constexpr std::tuple<Unit, Unit, Unit> tuple() const { return {m_x, m_y, m_z}; }

		//////////////////////////////////////////////////////////////////////////
		//		TUPLE PROTOCOL (so a Vector3 substitutes for a std::tuple<Unit, Unit, Unit>)
		//////////////////////////////////////////////////////////////////////////
		//	A mutable and const `get<I>` member plus the std::get / std::tuple_size / std::tuple_element
		//	specializations (below, outside the class) let structured bindings, `std::get<I>(v)`,
		//	`std::get<I>(v) = value`, and `std::make_from_tuple` all treat this vector as its component tuple.

		/// The mutable reference to component `I` (0 = x, 1 = y, 2 = z).
		template<std::size_t I>
		[[nodiscard]] constexpr Unit& get() noexcept
		{
			static_assert(I < 3, "Vector3 has three components.");
			if constexpr (I == 0)
				return m_x;
			else if constexpr (I == 1)
				return m_y;
			else
				return m_z;
		}

		/// Component `I` (0 = x, 1 = y, 2 = z).
		template<std::size_t I>
		[[nodiscard]] constexpr Unit get() const noexcept
		{
			static_assert(I < 3, "Vector3 has three components.");
			if constexpr (I == 0)
				return m_x;
			else if constexpr (I == 1)
				return m_y;
			else
				return m_z;
		}

		//////////////////////////////////////////////////////////////////////////
		//		GEOMETRIC ALGEBRA
		//////////////////////////////////////////////////////////////////////////

		/// The dot product with a vector of any unit; the result carries the product dimension (e.g. a length
		/// vector dotted with a dimensionless direction is a length).
		template<class U>
		[[nodiscard]] constexpr auto dot(const Vector3<U>& v) const
		{
			return m_x * v.x() + m_y * v.y() + m_z * v.z();
		}

		/// The cross product with a vector of any unit; the result vector carries the product dimension.
		template<class U>
		[[nodiscard]] constexpr auto cross(const Vector3<U>& v) const
		{
			using product = decltype(std::declval<Unit>() * std::declval<U>());
			return Vector3<product>(m_y * v.z() - m_z * v.y(), m_z * v.x() - m_x * v.z(), m_x * v.y() - m_y * v.x());
		}

		/// The Euclidean magnitude (a `Unit`).
		[[nodiscard]] Unit magnitude() const { return Unit(units::sqrt(dot(*this))); }

		/// The magnitude squared (an area-dimensioned scalar; avoids the square root).
		[[nodiscard]] constexpr scalar_type magnitudeSquared() const { return dot(*this); }

		/// The unit-length direction (dimensionless components). Returns the zero vector if this is null.
		[[nodiscard]] Vector3<units::dimensionless<>> normalized() const
		{
			const Unit m = magnitude();
			if (m == Unit(0.0))
				return Vector3<units::dimensionless<>>{};
			return Vector3<units::dimensionless<>>(m_x / m, m_y / m, m_z / m);
		}

		/// True if every component is zero.
		[[nodiscard]] constexpr bool isNull() const { return m_x == Unit(0.0) && m_y == Unit(0.0) && m_z == Unit(0.0); }

		//////////////////////////////////////////////////////////////////////////
		//		COMPOUND ASSIGNMENT
		//////////////////////////////////////////////////////////////////////////

		constexpr Vector3& operator+=(const Vector3& v)
		{
			m_x += v.m_x;
			m_y += v.m_y;
			m_z += v.m_z;
			return *this;
		}

		constexpr Vector3& operator-=(const Vector3& v)
		{
			m_x -= v.m_x;
			m_y -= v.m_y;
			m_z -= v.m_z;
			return *this;
		}

		constexpr Vector3& operator*=(units::dimensionless<> factor)
		{
			m_x = m_x * factor;
			m_y = m_y * factor;
			m_z = m_z * factor;
			return *this;
		}

		constexpr Vector3& operator/=(units::dimensionless<> divisor)
		{
			m_x = m_x / divisor;
			m_y = m_y / divisor;
			m_z = m_z / divisor;
			return *this;
		}

		//////////////////////////////////////////////////////////////////////////
		//		COMPARISON
		//////////////////////////////////////////////////////////////////////////

		[[nodiscard]] constexpr bool operator==(const Vector3& v) const { return m_x == v.m_x && m_y == v.m_y && m_z == v.m_z; }
		[[nodiscard]] constexpr bool operator!=(const Vector3& v) const { return !(*this == v); }

		/// Equality with the equivalent component tuple, so a `Vector3` and a `std::tuple<Unit, Unit, Unit>` of
		/// the same components compare equal (a `Vector3` is its tuple).
		[[nodiscard]] constexpr bool operator==(const std::tuple<Unit, Unit, Unit>& t) const
		{
			return m_x == std::get<0>(t) && m_y == std::get<1>(t) && m_z == std::get<2>(t);
		}

#ifdef VECTOR3_HAS_EIGEN
		//////////////////////////////////////////////////////////////////////////
		//		EIGEN INTEROPERABILITY (opt-in; present only when <Eigen/Core> is available)
		//////////////////////////////////////////////////////////////////////////

		/// This vector as an `Eigen::Matrix<Unit, 3, 1>` (a dimensioned Eigen column vector).
		[[nodiscard]] Eigen::Matrix<Unit, 3, 1> toEigen() const { return Eigen::Matrix<Unit, 3, 1>(m_x, m_y, m_z); }

		/// A `Vector3` from an `Eigen::Matrix<Unit, 3, 1>`.
		[[nodiscard]] static Vector3 fromEigen(const Eigen::Matrix<Unit, 3, 1>& v) { return Vector3(v(0), v(1), v(2)); }
#endif

	private:
		Unit m_x{};    ///< first component
		Unit m_y{};    ///< second component
		Unit m_z{};    ///< third component
	};

	//======================================================
	//	FREE OPERATORS
	//======================================================

	/// The sum of two same-unit vectors.
	template<class Unit>
	[[nodiscard]] constexpr Vector3<Unit> operator+(Vector3<Unit> lhs, const Vector3<Unit>& rhs)
	{
		return lhs += rhs;
	}

	/// The difference of two same-unit vectors.
	template<class Unit>
	[[nodiscard]] constexpr Vector3<Unit> operator-(Vector3<Unit> lhs, const Vector3<Unit>& rhs)
	{
		return lhs -= rhs;
	}

	/// The negation of a vector.
	template<class Unit>
	[[nodiscard]] constexpr Vector3<Unit> operator-(const Vector3<Unit>& v)
	{
		return Vector3<Unit>(Unit(0.0) - v.x(), Unit(0.0) - v.y(), Unit(0.0) - v.z());
	}

	/// A vector scaled by a dimensionless factor.
	template<class Unit>
	[[nodiscard]] constexpr Vector3<Unit> operator*(Vector3<Unit> v, units::dimensionless<> factor)
	{
		return v *= factor;
	}

	/// A vector scaled by a dimensionless factor.
	template<class Unit>
	[[nodiscard]] constexpr Vector3<Unit> operator*(units::dimensionless<> factor, Vector3<Unit> v)
	{
		return v *= factor;
	}

	/// A vector divided by a dimensionless divisor.
	template<class Unit>
	[[nodiscard]] constexpr Vector3<Unit> operator/(Vector3<Unit> v, units::dimensionless<> divisor)
	{
		return v /= divisor;
	}

	/// Equality of a component tuple with a `Vector3` (the reversed form of the member `operator==`).
	template<class Unit>
	[[nodiscard]] constexpr bool operator==(const std::tuple<Unit, Unit, Unit>& t, const Vector3<Unit>& v)
	{
		return v == t;
	}

	/// Streams the vector as `(x, y, z)`.
	template<class Unit>
	std::ostream& operator<<(std::ostream& os, const Vector3<Unit>& v)
	{
		return os << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
	}

	//======================================================
	//	CARTESIAN TYPES
	//======================================================

	/// A Cartesian position or offset in meters: the length-typed 3-vector the frame and coordinate machinery is
	/// built on. Carries the full algebra (`dot`/`cross`/`magnitude`/`normalized`, add/subtract, scale), the
	/// `x()/y()/z()` accessors, and the std::tuple protocol.
	using CartesianTuple = Vector3<units::length::meters<>>;

	/// An ordered sequence of Cartesian points -- a polyline or point list.
	using CartesianVector = std::vector<CartesianTuple>;
}    // namespace coordinates

//======================================================
//	STD TUPLE-PROTOCOL SPECIALIZATIONS
//======================================================
//	Teach the standard library that a `Vector3<Unit>` is a three-element tuple of `Unit`, so `std::get<I>(v)`,
//	`std::get<I>(v) = value`, structured bindings, and `std::make_from_tuple<Vector3>` all work.

namespace std
{
	template<class Unit>
	struct tuple_size<coordinates::Vector3<Unit>> : std::integral_constant<std::size_t, 3>
	{
	};

	template<std::size_t I, class Unit>
	struct tuple_element<I, coordinates::Vector3<Unit>>
	{
		using type = Unit;
	};

	template<std::size_t I, class Unit>
	[[nodiscard]] constexpr Unit& get(coordinates::Vector3<Unit>& v) noexcept
	{
		return v.template get<I>();
	}

	template<std::size_t I, class Unit>
	[[nodiscard]] constexpr Unit get(const coordinates::Vector3<Unit>& v) noexcept
	{
		return v.template get<I>();
	}

	template<std::size_t I, class Unit>
	[[nodiscard]] constexpr Unit get(coordinates::Vector3<Unit>&& v) noexcept
	{
		return v.template get<I>();
	}
}    // namespace std

#endif    // vector3_h
