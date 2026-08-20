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
// A frame-tagged geometric vector. `Vector<Frame, Unit>` is a directed quantity expressed in a coordinate
// frame -- the free-vector counterpart to `Coordinate<Frame, Tuple>`. It composes the frame-agnostic
// `vec::Vector3<Unit>` (the algebra + optional Eigen interop) with a frame tag and the frame data (the
// origin a local/tangent frame is anchored to, and the observation date). It satisfies the `is_vector`
// concept and converts between frames by ROTATION ONLY: a free vector has no position, so a frame change
// rotates its components but never translates them. The rotation is realized by converting the vector's tip
// and its origin as points through the position conversion graph and subtracting -- the translation cancels,
// leaving pure rotation, and the proven position machinery is reused rather than duplicated.
//
//--------------------------------------------------------------------------------------------------

#ifndef vector_h
#define vector_h

//------------------------
//	INCLUDES
//------------------------

#include <ostream>
#include <tuple>

#include <units.h>

#include "coordinate.h"
#include "coordinates_fwd.h"
#include "frameOfReference.h"
#include "point.h"
#include "vector3.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		Vector
	//  ----------------------------------------------------------------------------
	///	@brief		A directed quantity expressed in a coordinate frame: the frame-tagged geometric vector.
	///	@details	Composes `vec::Vector3<Unit>` (the unit-typed algebra + opt-in Eigen interop) with a frame
	///				tag and frame data (origin + date). It is a FREE vector: a frame change rotates its
	///				components but never translates them, realized by converting the tip and origin as points
	///				and subtracting. Satisfies the `is_vector` concept and interoperates with the frame graph.
	///	@tparam		Frame	the frame of reference the vector is expressed in.
	///	@tparam		Unit	the component quantity type (defaults to `meters<>`).
	//  ----------------------------------------------------------------------------
	template<class Frame, class Unit = units::length::meters<>>
	class Vector
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using reference_frame = Frame;
		using tuple_type      = std::tuple<Unit, Unit, Unit>;
		using frame_data_type = FrameData;
		using datum_type      = typename traits::frame_traits<Frame>::datum_type;
		using unit_type       = Unit;
		using storage_type    = vec::Vector3<Unit>;
		using vector_tag      = std::true_type;    ///< marks this a vector (not a point) for the `is_vector` concept

		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/// The zero vector at the datum epoch.
		Vector()
		    : m_vector{}
		    , m_frameData(datum_type::epoch())
		{
		}

		/// From three components at the datum epoch.
		Vector(Unit x, Unit y, Unit z)
		    : m_vector(x, y, z)
		    , m_frameData(datum_type::epoch())
		{
		}

		/// From a `vec::Vector3` at the datum epoch.
		explicit Vector(const storage_type& v)
		    : m_vector(v)
		    , m_frameData(datum_type::epoch())
		{
		}

		/// From components anchored to a geodetic origin (for a local/tangent frame), at an optional date.
		template<traits::is_point OriginPoint>
		    requires(traits::is_local_frame<Frame>)
		Vector(Unit x, Unit y, Unit z, const OriginPoint& origin, years<> dateOfObservation = datum_type::epoch())
		    : m_vector(x, y, z)
		    , m_frameData(PositionGeodetic<datum_type>(origin).point(), dateOfObservation)
		{
		}

		/// Converting constructor: rotate any other frame's vector into this frame.
		template<class V>
		    requires(traits::is_vector<V> && !std::same_as<std::remove_cvref_t<V>, Vector>)
		Vector(const V& other)
		    : Vector()
		{
			*this = other;
		}

		//////////////////////////////////////////////////////////////////////////
		//		FRAME CONVERSION (rotation only)
		//////////////////////////////////////////////////////////////////////////

		/// Rotate any other frame's vector into this frame. A free vector carries no position, so the source's
		/// tip and origin are converted as points into this frame and subtracted: the translation cancels and
		/// only the inter-frame rotation remains. This coordinate's own origin/date are preserved.
		template<class V>
		    requires(traits::is_vector<V> && !std::same_as<std::remove_cvref_t<V>, Vector>)
		Vector& operator=(const V& other)
		{
			using SourceFrame = typename traits::point_traits<V>::reference_frame;

			// Express the source vector as a tip point and an origin point in the SOURCE frame, convert both
			// into THIS frame, and subtract. Points carry the source's frame data so translation is applied
			// consistently to both endpoints and cancels in the difference.
			Coordinate<SourceFrame, std::tuple<Unit, Unit, Unit>> srcTip;
			Coordinate<SourceFrame, std::tuple<Unit, Unit, Unit>> srcOrg;
			srcTip.setFrameData(other.frameData());
			srcOrg.setFrameData(other.frameData());
			const auto v = other.vector();
			srcTip.setPoint(std::get<0>(v), std::get<1>(v), std::get<2>(v));
			srcOrg.setPoint(Unit(0.0), Unit(0.0), Unit(0.0));

			Coordinate<Frame, std::tuple<Unit, Unit, Unit>> dstTip;
			Coordinate<Frame, std::tuple<Unit, Unit, Unit>> dstOrg;
			dstTip.setFrameData(m_frameData);
			dstOrg.setFrameData(m_frameData);
			coordinates::convert(srcTip, dstTip);
			coordinates::convert(srcOrg, dstOrg);

			const auto tip = dstTip.point();
			const auto org = dstOrg.point();
			m_vector       = storage_type(std::get<0>(tip) - std::get<0>(org), std::get<1>(tip) - std::get<1>(org), std::get<2>(tip) - std::get<2>(org));
			return *this;
		}

		//////////////////////////////////////////////////////////////////////////
		//		VECTOR INTERFACE (satisfies the `is_vector` concept)
		//////////////////////////////////////////////////////////////////////////

		[[nodiscard]] tuple_type vector() const { return m_vector.tuple(); }
		void                     setVector(const tuple_type& v) { m_vector = storage_type(v); }

		template<class... Args>
		    requires(sizeof...(Args) == 3)
		void setVector(Args... args) { m_vector = storage_type(std::move(args)...); }

		[[nodiscard]] frame_data_type frameData() const { return m_frameData; }
		void                          setFrameData(const frame_data_type& frameData) { m_frameData = frameData; }

		/// Date of observation (relevant only when converting between datums).
		[[nodiscard]] years<> date() const { return m_frameData.date; }

		//////////////////////////////////////////////////////////////////////////
		//		ALGEBRA (forwarded to the storage vector)
		//////////////////////////////////////////////////////////////////////////

		[[nodiscard]] Unit                        magnitude() const { return m_vector.magnitude(); }
		[[nodiscard]] typename storage_type::scalar_type magnitudeSquared() const { return m_vector.magnitudeSquared(); }
		[[nodiscard]] bool                        isNull() const { return m_vector.isNull(); }

		/// Dot product with a same-frame vector (an area-dimensioned scalar).
		template<class V>
		    requires(traits::is_vector<V> && std::same_as<typename traits::point_traits<V>::reference_frame, Frame>)
		[[nodiscard]] auto dot(const V& other) const { return m_vector.dot(storage_type(other.vector())); }

		/// Cross product with a same-frame vector (an area-dimensioned same-frame vector).
		template<class V>
		    requires(traits::is_vector<V> && std::same_as<typename traits::point_traits<V>::reference_frame, Frame>)
		[[nodiscard]] Vector<Frame, typename storage_type::scalar_type> cross(const V& other) const
		{
			const auto c = m_vector.cross(storage_type(other.vector()));
			Vector<Frame, typename storage_type::scalar_type> result(c.x(), c.y(), c.z());
			result.setFrameData(m_frameData);
			return result;
		}

		Vector& operator+=(const Vector& v)
		{
			m_vector += v.m_vector;
			return *this;
		}
		Vector& operator-=(const Vector& v)
		{
			m_vector -= v.m_vector;
			return *this;
		}
		Vector& operator*=(units::dimensionless<> factor)
		{
			m_vector *= factor;
			return *this;
		}
		Vector& operator/=(units::dimensionless<> divisor)
		{
			m_vector /= divisor;
			return *this;
		}

		/// The underlying frame-agnostic algebra vector (for direct `vec::Vector3` use, incl. Eigen interop).
		[[nodiscard]] const storage_type& components() const { return m_vector; }

		/// Streams as `(x, y, z)`.
		friend std::ostream& operator<<(std::ostream& os, const Vector& v) { return os << v.m_vector; }

	private:
		storage_type    m_vector;       ///< the frame-agnostic component algebra
		frame_data_type m_frameData;    ///< origin (for a local frame) + observation date
	};

	//======================================================
	//	FREE OPERATORS
	//======================================================

	/// The sum of two same-frame vectors.
	template<class Frame, class Unit>
	[[nodiscard]] Vector<Frame, Unit> operator+(Vector<Frame, Unit> lhs, const Vector<Frame, Unit>& rhs)
	{
		return lhs += rhs;
	}

	/// The difference of two same-frame vectors.
	template<class Frame, class Unit>
	[[nodiscard]] Vector<Frame, Unit> operator-(Vector<Frame, Unit> lhs, const Vector<Frame, Unit>& rhs)
	{
		return lhs -= rhs;
	}

	/// A vector scaled by a dimensionless factor.
	template<class Frame, class Unit>
	[[nodiscard]] Vector<Frame, Unit> operator*(Vector<Frame, Unit> v, units::dimensionless<> factor)
	{
		return v *= factor;
	}

	/// A vector scaled by a dimensionless factor.
	template<class Frame, class Unit>
	[[nodiscard]] Vector<Frame, Unit> operator*(units::dimensionless<> factor, Vector<Frame, Unit> v)
	{
		return v *= factor;
	}
}    // namespace coordinates

#endif    // vector_h
