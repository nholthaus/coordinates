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
/// @file	vectorECEF.h
/// @brief	Defines the VectorECEF class.
//
//--------------------------------------------------------------------------------------------------

#ifndef vectorECEF_h
#define vectorECEF_h

//------------------------
//	INCLUDES
//------------------------

#include <cassert>
#include <ostream>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include "frameOfReference.h"
#include "positionGeodetic.h"
#include "positionENU.h"
#include "positionNED.h"
#include "positionECEF.h"

inline namespace coordinates
{
	// Forward declarations (defined in their respective headers)
	template<is_datum Datum, template<class> class DistanceUnits, typename T>
	class VectorENU;

	template<is_datum Datum, template<class> class DistanceUnits, typename T>
	class VectorNED;

	//	----------------------------------------------------------------------------
	//	CLASS		VectorECEF
	//  ----------------------------------------------------------------------------
	/// @brief		3D Cartesian vector in ECEF coordinates.
	/// @details
	///			A VectorECEF represents a directed quantity (dx, dy, dz) expressed in an
	///			Earth-Centered, Earth-Fixed Cartesian frame.
	///
	///			Unlike positions, vectors do not represent an absolute location and should
	///			never be added to other vectors/positions unless the operation is
	///			mathematically meaningful. In particular, VectorECEF is a *free vector*;
	///			it is not anchored to any origin.
	///
	/// @tparam	Datum		Datum defining the horizontal reference used by the underlying ECEF frame.
	/// @tparam	DistanceUnits	Units used to store each component internally. Defaults to meters.
	/// @tparam	T		Underlying storage type. Defaults to double.
	//  ----------------------------------------------------------------------------
	// The horizontal datum an ECEF frame is keyed on, whether `Datum` is a full 3-D datum (take its
	// horizontal component) or already a horizontal-only datum (use it directly). An ECEF vector is a purely
	// horizontal-datum quantity, so it accepts either and reduces to the horizontal datum. A full datum also
	// satisfies `is_horizontal_datum`, so the full-datum case is discriminated by `is_datum` (which a
	// horizontal-only datum does NOT satisfy), and the primary template is the horizontal-only identity.
	template<class Datum>
	struct ecef_horizontal_datum
	{
		using type = Datum;
	};
	template<class Datum>
	    requires(coordinates::traits::is_datum<Datum>)
	struct ecef_horizontal_datum<Datum>
	{
		using type = typename datum_traits<Datum>::horizontal_datum;
	};
	template<class Datum>
	using ecef_horizontal_datum_t = typename ecef_horizontal_datum<Datum>::type;

	template<class Datum, template<class> class DistanceUnits = meters, typename T = double>
	    requires(coordinates::traits::is_datum<Datum> || coordinates::traits::is_horizontal_datum<Datum>)
	class VectorECEF
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//		STATIC ERROR CHECKING
		//////////////////////////////////////////////////////////////////////////

		static_assert(units::traits::is_unit_v<DistanceUnits<T>>, "Template parameter `DistanceUnits` must be a unit type.");
		static_assert(coordinates::traits::is_datum<Datum> || coordinates::traits::is_horizontal_datum<Datum>, "`Datum` template parameter must be a datum or horizontal datum.");
		static_assert(std::is_arithmetic_v<T>, "`T` template parameter must be an arithmetic type.");

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using datum_type         = Datum;
		using reference_frame    = ECEFFrame<ecef_horizontal_datum_t<Datum>>;
		using tuple_type         = CartesianTuple;
		using frame_data_type    = FrameData;
		using distance_unit_type = DistanceUnits<T>;

		using vector_tag = std::true_type;

		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		Default constructor.
		 * @details		Creates a vector with value (0,0,0).
		 */
		VectorECEF()
		    : m_x(0)
		    , m_y(0)
		    , m_z(0)
		    , m_frameData(Datum::epoch())
		{
			// Frame data is stored only to satisfy shared traits/concepts patterns.
			// It has no semantic meaning for an ECEF free vector.
		}

		/**
		 * @brief		Constructor.
		 * @param[in]	X	X component.
		 * @param[in]	Y	Y component.
		 * @param[in]	Z	Z component.
		 */
		VectorECEF(distance_unit_type X, distance_unit_type Y, distance_unit_type Z)
		    : m_x(X)
		    , m_y(Y)
		    , m_z(Z)
		    , m_frameData(Datum::epoch())
		{
		}

		/**
		 * @brief		Tuple constructor.
		 * @details		Constructs a vector from a tuple of (x,y,z) values, performing unit conversions
		 *			as necessary.
		 * @param[in]	v	Tuple containing the vector components.
		 */
		explicit VectorECEF(const tuple_type& v)
		    : m_x(std::get<0>(v))
		    , m_y(std::get<1>(v))
		    , m_z(std::get<2>(v))
		    , m_frameData(Datum::epoch())
		{
		}

		/**
		 * @brief		Conversion constructor from a VectorENU.
		 * @details		Converts an ENU anchored vector into an ECEF free vector using the normal
		 *			coordinate conversion pipeline.
		 * @tparam		ENUUnits	Units used by the ENU vector.
		 * @param[in]		enu		ENU vector to convert.
		 */
		template<template<class> class ENUUnits>
		VectorECEF(const VectorENU<Datum, ENUUnits, T>& enu);

		/**
		 * @brief		Conversion constructor from a VectorNED.
		 * @details		Converts a NED anchored vector into an ECEF free vector using the normal
		 *			coordinate conversion pipeline.
		 * @tparam		NEDUnits	Units used by the NED vector.
		 * @param[in]		ned		NED vector to convert.
		 */
		template<template<class> class NEDUnits>
		VectorECEF(const VectorNED<Datum, NEDUnits, T>& ned);

		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS
		//////////////////////////////////////////////////////////////////////////

		[[nodiscard]] distance_unit_type x() const { return m_x; }
		[[nodiscard]] distance_unit_type y() const { return m_y; }
		[[nodiscard]] distance_unit_type z() const { return m_z; }



		[[nodiscard]] frame_data_type frameData() const { return m_frameData; }
		void setFrameData(const frame_data_type& f) { m_frameData = f; }

/**
		 * @brief		Vector as a tuple.
		 * @returns		Vector components as a (meters, meters, meters) tuple.
		 */
		[[nodiscard]] tuple_type vector() const { return tuple_type(m_x, m_y, m_z); }

		/**
		 * @brief		Set the vector value from a tuple.
		 * @param[in]	v	Tuple containing the vector components.
		 */
		void setVector(const tuple_type& v)
		{
			m_x = std::get<0>(v);
			m_y = std::get<1>(v);
			m_z = std::get<2>(v);
		}

		/**
		 * @brief		Set the vector value from forwarded tuple constructor args.
		 * @tparam	Args	Argument pack forwarded to tuple_type construction.
		 * @param[in]	args	Values forwarded to tuple_type constructor.
		 */
		template<class... Args>
		    requires(sizeof...(Args) > 0) && std::constructible_from<tuple_type, Args...>
		void setVector(Args... args)
		{ this->setVector(std::make_tuple<Args...>(std::forward<Args>(args)...)); }

		/**
		 * @brief		Helper for constructing tuples without std::make_tuple at call sites.
		 */
		template<class... Args>
		    requires(sizeof...(Args) > 0) && std::constructible_from<tuple_type, Args...>
		static tuple_type makeTuple(Args... args)
		{ return std::make_tuple<Args...>(std::forward<Args>(args)...); }

		//////////////////////////////////////////////////////////////////////////
		//		ARITHMETIC
		//////////////////////////////////////////////////////////////////////////

		VectorECEF& operator+=(const VectorECEF& v)
		{
			m_x = m_x + v.x();
			m_y = m_y + v.y();
			m_z = m_z + v.z();
			return *this;
		}

		VectorECEF& operator-=(const VectorECEF& v)
		{
			m_x = m_x - v.x();
			m_y = m_y - v.y();
			m_z = m_z - v.z();
			return *this;
		}

		friend std::ostream& operator<<(std::ostream& os, const VectorECEF& v)
		{ return os << "(" << v.m_x << ", " << v.m_y << ", " << v.m_z << ")"; }

	private:
		distance_unit_type m_x;
		distance_unit_type m_y;
		distance_unit_type m_z;

		frame_data_type m_frameData;
	};
} // namespace coordinates

//----------------------------------
//  POSITION/VECTOR ARITHMETIC
//----------------------------------

template<class Datum,
         template<class> class DistanceUnits,
         typename T>
    requires(coordinates::traits::is_datum<Datum> || coordinates::traits::is_horizontal_datum<Datum>)
template<template<class> class ENUUnits>
VectorECEF<Datum, DistanceUnits, T>::VectorECEF(const VectorENU<Datum, ENUUnits, T>& enu)
    : m_x(0)
    , m_y(0)
    , m_z(0)
    , m_frameData(enu.frameData())
{
    // Build a tip point in the ENU frame, convert both tip and origin to ECEF, and subtract.
    const FrameData fd = enu.frameData();
    const PositionGeodetic<Datum> origin(fd.origin, fd.date);

    const PositionENU<Datum, ENUUnits> pTip(enu.east(), enu.north(), enu.up(), origin, fd.date);
    const PositionECEF<Datum, meters>  eTip(pTip);
    const PositionECEF<Datum, meters>  eOrg(origin);

    m_x = eTip.x() - eOrg.x();
    m_y = eTip.y() - eOrg.y();
    m_z = eTip.z() - eOrg.z();
}

template<class Datum,
         template<class> class DistanceUnits,
         typename T>
    requires(coordinates::traits::is_datum<Datum> || coordinates::traits::is_horizontal_datum<Datum>)
template<template<class> class NEDUnits>
VectorECEF<Datum, DistanceUnits, T>::VectorECEF(const VectorNED<Datum, NEDUnits, T>& ned)
    : m_x(0)
    , m_y(0)
    , m_z(0)
    , m_frameData(ned.frameData())
{
    const FrameData fd = ned.frameData();
    const PositionGeodetic<Datum> origin(fd.origin, fd.date);

    const PositionNED<Datum, NEDUnits> pTip(ned.north(), ned.east(), ned.down(), origin, fd.date);
    const PositionECEF<Datum, meters>  eTip(pTip);
    const PositionECEF<Datum, meters>  eOrg(origin);

    m_x = eTip.x() - eOrg.x();
    m_y = eTip.y() - eOrg.y();
    m_z = eTip.z() - eOrg.z();
}

// An ECEF point: a point whose reference frame is a base (root) ECEF frame. Constrained structurally so the
// operators deduce the concrete (aliased) point type rather than naming the non-deducible PositionECEF alias.
template<class P>
concept ecef_point =
        coordinates::traits::is_point<P> && !coordinates::traits::is_vector<P>
        && std::same_as<typename coordinates::traits::point_traits<P>::reference_frame,
                        coordinateFrames::ECEFFrame<typename coordinates::traits::frame_traits<typename coordinates::traits::point_traits<P>::reference_frame>::datum_type>>;

template<ecef_point EcefPoint>
VectorECEF<typename coordinates::traits::frame_traits<typename coordinates::traits::point_traits<EcefPoint>::reference_frame>::datum_type> operator-(const EcefPoint& lhs, const EcefPoint& rhs)
{
	using Datum = typename coordinates::traits::frame_traits<typename coordinates::traits::point_traits<EcefPoint>::reference_frame>::datum_type;
	return VectorECEF<Datum>(lhs.x() - rhs.x(), lhs.y() - rhs.y(), lhs.z() - rhs.z());
}

template<ecef_point EcefPoint, class VecDatum, template<class> class VecUnits, typename T>
    requires(std::same_as<ecef_horizontal_datum_t<VecDatum>, typename coordinates::traits::frame_traits<typename coordinates::traits::point_traits<EcefPoint>::reference_frame>::datum_type>)
EcefPoint operator+(EcefPoint lhs, const VectorECEF<VecDatum, VecUnits, T>& rhs)
{
	lhs.setX(lhs.x() + rhs.x());
	lhs.setY(lhs.y() + rhs.y());
	lhs.setZ(lhs.z() + rhs.z());
	return lhs;
}

template<ecef_point EcefPoint, class VecDatum, template<class> class VecUnits, typename T>
    requires(std::same_as<ecef_horizontal_datum_t<VecDatum>, typename coordinates::traits::frame_traits<typename coordinates::traits::point_traits<EcefPoint>::reference_frame>::datum_type>)
EcefPoint operator-(EcefPoint lhs, const VectorECEF<VecDatum, VecUnits, T>& rhs)
{
	lhs.setX(lhs.x() - rhs.x());
	lhs.setY(lhs.y() - rhs.y());
	lhs.setZ(lhs.z() - rhs.z());
	return lhs;
}


//----------------------------------
//  VECTOR ARITHMETIC (LCA = ECEF)
//----------------------------------

template<class VL, class VR>
	requires(
		requires(const VL& v) { typename VL::datum_type; v.vector(); v.frameData(); } &&
		requires(const VR& v) { typename VR::datum_type; v.vector(); v.frameData(); } &&
		std::is_same_v<typename VL::datum_type, typename VR::datum_type>)
VectorECEF<typename VL::datum_type> operator+(const VL& lhs, const VR& rhs)
{
	VectorECEF<typename VL::datum_type> l(lhs);
	VectorECEF<typename VL::datum_type> r(rhs);

	requireSameFrameData(l.frameData(), r.frameData(), "Vector frame mismatch in operator+ (LCA=ECEF)");

	l += r;
	return l;
}

template<class VL, class VR>
	requires(
		requires(const VL& v) { typename VL::datum_type; v.vector(); v.frameData(); } &&
		requires(const VR& v) { typename VR::datum_type; v.vector(); v.frameData(); } &&
		std::is_same_v<typename VL::datum_type, typename VR::datum_type>)
VectorECEF<typename VL::datum_type> operator-(const VL& lhs, const VR& rhs)
{
	VectorECEF<typename VL::datum_type> l(lhs);
	VectorECEF<typename VL::datum_type> r(rhs);

	requireSameFrameData(l.frameData(), r.frameData(), "Vector frame mismatch in operator- (LCA=ECEF)");

	l -= r;
	return l;
}

//----------------------------------
//  POSITION + VECTOR (mixed frames -> LCA=ECEF)
//----------------------------------

template<ecef_point EcefPoint, class AnyVector>
	requires(
		requires(const AnyVector& v) { typename AnyVector::datum_type; v.vector(); v.frameData(); } &&
		std::is_same_v<ecef_horizontal_datum_t<typename AnyVector::datum_type>, typename coordinates::traits::frame_traits<typename coordinates::traits::point_traits<EcefPoint>::reference_frame>::datum_type>
	)
EcefPoint operator+(EcefPoint lhs, const AnyVector& rhs)
{
	VectorECEF<typename AnyVector::datum_type, meters> v(rhs);
	return lhs + v;
}

template<ecef_point EcefPoint, class AnyVector>
	requires(
		requires(const AnyVector& v) { typename AnyVector::datum_type; v.vector(); v.frameData(); } &&
		std::is_same_v<typename AnyVector::datum_type, typename coordinates::traits::frame_traits<typename coordinates::traits::point_traits<EcefPoint>::reference_frame>::datum_type>
	)
EcefPoint operator-(EcefPoint lhs, const AnyVector& rhs)
{
	using Datum = typename coordinates::traits::frame_traits<typename coordinates::traits::point_traits<EcefPoint>::reference_frame>::datum_type;
	VectorECEF<Datum, meters> v(rhs);
	return lhs - v;
}




//----------------------------------
//  SCALAR MULTIPLY / DIVIDE (ECEF)
//----------------------------------

template<is_datum Datum, template<class> class VecUnits, typename T, typename S>
	requires(std::is_arithmetic_v<S>)
VectorECEF<Datum, VecUnits, std::common_type_t<T, S>>
operator*(const VectorECEF<Datum, VecUnits, T>& v, const S s)
{
	using R = std::common_type_t<T, S>;
	VectorECEF<Datum, VecUnits, R> out(
		VecUnits<R>(v.x()) * static_cast<R>(s),
		VecUnits<R>(v.y()) * static_cast<R>(s),
		VecUnits<R>(v.z()) * static_cast<R>(s)
	);
	out.setFrameData(v.frameData());
	return out;
}

template<is_datum Datum, template<class> class VecUnits, typename T, typename S>
	requires(std::is_arithmetic_v<S>)
VectorECEF<Datum, VecUnits, std::common_type_t<T, S>>
operator*(const S s, const VectorECEF<Datum, VecUnits, T>& v)
{
	return v * s;
}

template<is_datum Datum, template<class> class VecUnits, typename T, typename S>
	requires(std::is_arithmetic_v<S>)
VectorECEF<Datum, VecUnits, std::common_type_t<T, S>>
operator/(const VectorECEF<Datum, VecUnits, T>& v, const S s)
{
	using R = std::common_type_t<T, S>;
	VectorECEF<Datum, VecUnits, R> out(
		VecUnits<R>(v.x()) / static_cast<R>(s),
		VecUnits<R>(v.y()) / static_cast<R>(s),
		VecUnits<R>(v.z()) / static_cast<R>(s)
	);
	out.setFrameData(v.frameData());
	return out;
}

//----------------------------------
//  COMPOUND ASSIGN (ECEF)
//----------------------------------

template<is_datum Datum, template<class> class VecUnits, typename T, typename S>
	requires(std::is_arithmetic_v<S>)
VectorECEF<Datum, VecUnits, T>&
operator*=(VectorECEF<Datum, VecUnits, T>& v, const S s)
{
	v = v * s;
	return v;
}

template<is_datum Datum, template<class> class VecUnits, typename T, typename S>
	requires(std::is_arithmetic_v<S>)
VectorECEF<Datum, VecUnits, T>&
operator/=(VectorECEF<Datum, VecUnits, T>& v, const S s)
{
	v = v / s;
	return v;
}

#endif    // vectorECEF_h
