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
// A generic, unit-typed 3-vector with full geometric algebra. `Vector3<Unit>` holds three components of a
// `units` quantity (e.g. `meters<double>`) and provides the dimensionally-correct operations: addition and
// subtraction of same-unit vectors, scaling by a dimensionless factor, dot product (dimension squared),
// cross product (dimension squared), magnitude (a `Unit`), and normalization (a dimensionless direction).
// It is frame-agnostic and carries no dependency on the coordinate library -- a reusable building block a
// frame-tagged vector (or any consumer) composes.
//
// Eigen interoperability is opt-in and dependency-free: `toEigen()` / `fromEigen()` are compiled only when
// <Eigen/Core> is on the include path (they map to/from `Eigen::Matrix<Unit, 3, 1>`, which `units/eigen.h`
// teaches Eigen to hold as a dimensioned matrix). Without Eigen the type is fully usable and the seam is
// simply absent -- no hard dependency.
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

namespace vec
{
	using namespace units;

	//	----------------------------------------------------------------------------
	//	CLASS		Vector3
	//  ----------------------------------------------------------------------------
	///	@brief		A unit-typed 3-vector with dimensionally-correct geometric algebra.
	///	@details	Stores three components of a `units` quantity. Same-unit vectors add and subtract; a vector
	///				scales by a dimensionless factor; the dot and cross products carry the squared dimension;
	///				the magnitude is a `Unit`; normalization yields a dimensionless direction. Frame-agnostic
	///				and free of any coordinate-library dependency.
	///	@tparam		Unit	the component quantity type (a `units` unit, e.g. `meters<double>`).
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
		using scalar_type = decltype(pow<2>(std::declval<Unit>()));      ///< the dimension of a dot/cross product

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

		/// From a `(x, y, z)` tuple of the component unit.
		explicit constexpr Vector3(const std::tuple<Unit, Unit, Unit>& t)
		    : m_x(std::get<0>(t))
		    , m_y(std::get<1>(t))
		    , m_z(std::get<2>(t))
		{
		}

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
		//		GEOMETRIC ALGEBRA
		//////////////////////////////////////////////////////////////////////////

		/// The dot product with another same-unit vector (an area-dimensioned scalar).
		[[nodiscard]] constexpr scalar_type dot(const Vector3& v) const { return m_x * v.m_x + m_y * v.m_y + m_z * v.m_z; }

		/// The cross product with another same-unit vector (an area-dimensioned vector).
		[[nodiscard]] constexpr Vector3<scalar_type> cross(const Vector3& v) const
		{
			return Vector3<scalar_type>(m_y * v.m_z - m_z * v.m_y, m_z * v.m_x - m_x * v.m_z, m_x * v.m_y - m_y * v.m_x);
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

	/// Streams the vector as `(x, y, z)`.
	template<class Unit>
	std::ostream& operator<<(std::ostream& os, const Vector3<Unit>& v)
	{
		return os << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
	}
}    // namespace vec

#endif    // vector3_h
