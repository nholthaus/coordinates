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
/// @file	vectorENU.h
/// @brief	Defines the VectorENU class.
//
//--------------------------------------------------------------------------------------------------

#ifndef vectorENU_h
#define vectorENU_h

//------------------------
//	INCLUDES
//------------------------

#include <cassert>
#include <ostream>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include "frameOfReference.h"
#include "point.h"
#include "positionAER.h"
#include "positionENU.h"
#include "positionGeodetic.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		VectorENU
	//  ----------------------------------------------------------------------------
	/// @brief		3D Cartesian vector in local ENU coordinates.
	/// @details
	///			A VectorENU represents a directed quantity (east, north, up) expressed in a
	///			local tangent ENU frame.
	///
	///			Unlike ECEF, an ENU frame is defined relative to an origin, and so VectorENU is
	///			*anchored* to its originating frame data. Arithmetic between ENU vectors (or
	///			between ENU positions and vectors) requires that the frame data match.
	///
	/// @tparam	Datum		Datum defining the horizontal reference used by the underlying ENU frame.
	/// @tparam	DistanceUnits	Units used to store each component internally. Defaults to meters.
	/// @tparam	T		Underlying storage type. Defaults to double.
	//  ----------------------------------------------------------------------------
	template<traits::is_datum Datum, template<class> class DistanceUnits = meters, typename T = double>
	class VectorENU
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//		STATIC ERROR CHECKING
		//////////////////////////////////////////////////////////////////////////

		static_assert(units::traits::is_unit_v<DistanceUnits<T>>, "Template parameter `DistanceUnits` must be a unit type.");
		static_assert(traits::is_datum<Datum>, "`Datum` template parameter does not satisfy the datum concept.");
		static_assert(std::is_arithmetic_v<T>, "`T` template parameter must be an arithmetic type.");

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using datum_type         = Datum;
		using reference_frame    = ENUFrame<typename traits::datum_traits<Datum>::horizontal_datum>;
		using tuple_type         = CartesianTuple;
		using frame_data_type    = FrameData;
		using distance_unit_type = DistanceUnits<T>;

		using origin_type = PositionGeodetic<Datum>;

		using vector_tag = std::true_type;

		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		Default constructor.
		 * @details		Creates a vector with value (0,0,0) anchored to origin (0,0,0).
		 */
		VectorENU()
		    : m_east(0)
		    , m_north(0)
		    , m_up(0)
		    , m_frameData(Datum::epoch())
		{
		}

		/**
		 * @brief		Constructor.
		 * @param[in]	east	East component.
		 * @param[in]	north	North component.
		 * @param[in]	up	Up component.
		 * @param[in]	origin	Geodetic origin anchoring this ENU frame.
		 * @param[in]	dateOfObservation	Date of observation (for datum conversions if needed).
		 */
		VectorENU(distance_unit_type east,
		          distance_unit_type north,
		          distance_unit_type up,
		          const origin_type& origin,
		          years<>            dateOfObservation = Datum::epoch())
		    : m_east(east)
		    , m_north(north)
		    , m_up(up)
		    , m_frameData(origin.point(), dateOfObservation)
		{
		}

		/**
		 * @brief		Tuple constructor.
		 * @details		Constructs a vector from a tuple of (e,n,u) values anchored to the provided origin.
		 * @param[in]	v	Tuple containing (e,n,u).
		 * @param[in]	origin	Geodetic origin anchoring this ENU frame.
		 * @param[in]	dateOfObservation	Date of observation.
		 */
		explicit VectorENU(const tuple_type& v, const origin_type& origin, years<> dateOfObservation = Datum::epoch())
		    : m_east(std::get<0>(v))
		    , m_north(std::get<1>(v))
		    , m_up(std::get<2>(v))
		    , m_frameData(origin.point(), dateOfObservation)
		{
		}

		/**
		 * @brief		Implicit conversion constructor from a VectorECEF.
		 * @details		Converts an ECEF free vector to an ENU anchored vector using the stored origin in the ECEF frame data.
		 */
		template<template<class> class ECEFUnits>
		VectorENU(const VectorECEF<Datum, ECEFUnits, T>& ecef)
		    : m_east(0)
		    , m_north(0)
		    , m_up(0)
		    , m_frameData(ecef.frameData())
		{
			const FrameData               fd = ecef.frameData();
			const PositionGeodetic<Datum> origin(fd.origin, fd.date);

			PositionECEF<Datum, meters, T> eOrg(origin);
			PositionECEF<Datum, meters, T> eTip(eOrg.x() + ecef.x(), eOrg.y() + ecef.y(), eOrg.z() + ecef.z());
			eTip.setFrameData(eOrg.frameData());

			PositionENU<Datum, meters, T> pTip;
			pTip.setFrameData(FrameData(origin.point(), fd.date));
			coordinates::convert(eTip, pTip);

			PositionENU<Datum, meters, T> pOrg(0.0_m, 0.0_m, 0.0_m, origin, fd.date);

			m_east  = pTip.east() - pOrg.east();
			m_north = pTip.north() - pOrg.north();
			m_up    = pTip.up() - pOrg.up();
		}

		/**
		 * @brief		Implicit conversion constructor from a VectorNED.
		 * @details		Uses ECEF as the least-common-ancestor frame for conversion.
		 */
		template<template<class> class NEDUnits>
		VectorENU(const VectorNED<Datum, NEDUnits, T>& ned)
		    : VectorENU(VectorECEF<Datum, meters, T>(ned))
		{
		}

		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS
		//////////////////////////////////////////////////////////////////////////

		[[nodiscard]] distance_unit_type east() const
		{ return m_east; }
		[[nodiscard]] distance_unit_type north() const
		{ return m_north; }
		[[nodiscard]] distance_unit_type up() const
		{ return m_up; }

		[[nodiscard]] frame_data_type frameData() const
		{ return m_frameData; }

		/**
		 * @brief		Vector as a tuple.
		 * @returns		Vector components as a (meters, meters, meters) tuple.
		 */
		[[nodiscard]] tuple_type vector() const
		{ return tuple_type(m_east, m_north, m_up); }

		/**
		 * @brief		Set the vector value from a tuple.
		 * @param[in]	v	Tuple containing (e,n,u).
		 */
		void setVector(const tuple_type& v)
		{
			m_east  = std::get<0>(v);
			m_north = std::get<1>(v);
			m_up    = std::get<2>(v);
		}

		constexpr void setEast(const distance_unit_type& east)
		{ m_east = east; }

		constexpr void setNorth(const distance_unit_type& north)
		{ m_north = north; }

		constexpr void setUp(const distance_unit_type& up)
		{ m_up = up; }

		/**
		 * @brief		Set the vector value from forwarded tuple constructor args.
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

		VectorENU& operator+=(const VectorENU& v)
		{
			requireSameFrameData(m_frameData, v.m_frameData, "VectorENU frame mismatch in operator+=");
			m_east  = m_east + v.east();
			m_north = m_north + v.north();
			m_up    = m_up + v.up();
			return *this;
		}

		VectorENU& operator-=(const VectorENU& v)
		{
			requireSameFrameData(m_frameData, v.m_frameData, "VectorENU frame mismatch in operator-=");
			m_east  = m_east - v.east();
			m_north = m_north - v.north();
			m_up    = m_up - v.up();
			return *this;
		}

		friend std::ostream& operator<<(std::ostream& os, const VectorENU& v)
		{ return os << "(" << v.m_east << ", " << v.m_north << ", " << v.m_up << ")"; }

		//////////////////////////////////////////////////////////////////////////
		//		HELPERS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		Constructs a unit direction VectorENU from azimuth/elevation.
		 * @details		We intentionally deduce the origin's angle-unit *template* from the origin type, and
		 *				accept any az/el angle *types* that are convertible to it.
		 *
		 *				This avoids a common deduction failure when callers use concrete angle aliases (e.g.
		 *				`degrees` as `degrees<double>`), while keeping the refactored template-based origin
		 *				type intact.
		 * @tparam		AzElUnit			Angle unit type of azimuth/elevation (e.g. degrees<double>, radians<double>). Must
		 *									be convertible to the origin's angle unit type.
		 * @tparam		OriginAngleUnits	Angle unit template used by the origin's latitude/longitude.
		 * @tparam		OriginHeightUnits	Height unit template used by the origin.
		 * @param[in]	azimuth				Azimuth.
		 * @param[in]	elevation			Elevation.
		 * @param[in]	origin				Geodetic origin anchoring the ENU frame.
		 * @returns		VectorENU representing the direction (scaled by 1 meter).
		 */
		template<typename AzElUnit, template<class> class OriginAngleUnits, template<class> class OriginHeightUnits>
		    requires(units::traits::is_angle_unit_v<AzElUnit>)
		static VectorENU fromAER(AzElUnit azimuth, AzElUnit elevation, const PositionGeodetic<Datum, OriginAngleUnits, OriginHeightUnits, T>& origin)
		{
			// Range is arbitrary; use 1 meter to yield a unit-length direction vector in ENU.
			const meters<T> range{1};

			// Force az/el into the same angular unit family used by the origin.
			const OriginAngleUnits<T> az(azimuth);
			const OriginAngleUnits<T> el(elevation);

			const PositionAER<Datum, OriginAngleUnits, meters, T> aer(az, el, range, origin, origin.date());
			const PositionENU<Datum, DistanceUnits, T>            enu(aer);

			return VectorENU(enu.east(), enu.north(), enu.up(), origin_type(origin.point(), origin.date()), origin.date());
		}

	private:
		distance_unit_type m_east;
		distance_unit_type m_north;
		distance_unit_type m_up;

		frame_data_type m_frameData;
	};
}    // namespace coordinates

//----------------------------------
//  POSITION/VECTOR ARITHMETIC
//----------------------------------

template<is_datum Datum, template<class> class PosUnits, typename T>
VectorENU<Datum, PosUnits, T> operator-(const PositionENU<Datum, PosUnits, T>& lhs, const PositionENU<Datum, PosUnits, T>& rhs)
{
	requireSameFrameData(lhs.frameData(), rhs.frameData(), "PositionENU frame mismatch in operator-");
	return VectorENU<Datum, PosUnits, T>(lhs.east() - rhs.east(),
	                                     lhs.north() - rhs.north(),
	                                     lhs.up() - rhs.up(),
	                                     PositionGeodetic<Datum>(lhs.frameData().origin, lhs.date()),
	                                     lhs.date());
}

template<is_datum Datum, template<class> class PosUnits, template<class> class VecUnits, typename T>
PositionENU<Datum, PosUnits, T> operator+(PositionENU<Datum, PosUnits, T> lhs, const VectorENU<Datum, VecUnits, T>& rhs)
{
	requireSameFrameData(lhs.frameData(), rhs.frameData(), "PositionENU frame mismatch in operator+");
	lhs.setEast(lhs.east() + rhs.east());
	lhs.setNorth(lhs.north() + rhs.north());
	lhs.setUp(lhs.up() + rhs.up());
	return lhs;
}

template<is_datum Datum, template<class> class PosUnits, template<class> class VecUnits, typename T>
PositionENU<Datum, PosUnits, T> operator-(PositionENU<Datum, PosUnits, T> lhs, const VectorENU<Datum, VecUnits, T>& rhs)
{
	requireSameFrameData(lhs.frameData(), rhs.frameData(), "PositionENU frame mismatch in operator-");
	lhs.setEast(lhs.east() - rhs.east());
	lhs.setNorth(lhs.north() - rhs.north());
	lhs.setUp(lhs.up() - rhs.up());
	return lhs;
}

#include <type_traits>

//----------------------------------
//  SCALAR MULTIPLY / DIVIDE (ENU)
//----------------------------------

template<is_datum Datum, template<class> class VecUnits, typename T, typename S>
    requires(std::is_arithmetic_v<S>)
VectorENU<Datum, VecUnits, std::common_type_t<T, S>> operator*(const VectorENU<Datum, VecUnits, T>& v, const S s)
{
	using R = std::common_type_t<T, S>;
	VectorENU<Datum, VecUnits, R> out(v);
	out.setEast(out.east() * s);
	out.setNorth(out.north() * s);
	out.setUp(out.up() * s);
	return out;
}

template<is_datum Datum, template<class> class VecUnits, typename T, typename S>
    requires(std::is_arithmetic_v<S>)
VectorENU<Datum, VecUnits, std::common_type_t<T, S>> operator*(const S s, const VectorENU<Datum, VecUnits, T>& v)
{ return v * s; }

template<is_datum Datum, template<class> class VecUnits, typename T, typename S>
    requires(std::is_arithmetic_v<S>)
VectorENU<Datum, VecUnits, std::common_type_t<T, S>> operator/(const VectorENU<Datum, VecUnits, T>& v, const S s)
{
	using R = std::common_type_t<T, S>;
	VectorENU<Datum, VecUnits, R> out(VecUnits<R>(v.east()) / static_cast<R>(s),
	                                  VecUnits<R>(v.north()) / static_cast<R>(s),
	                                  VecUnits<R>(v.up()) / static_cast<R>(s),
	                                  v.frameData().origin,
	                                  v.date());
	out.setFrameData(v.frameData());
	return out;
}

//----------------------------------
//  COMPOUND ASSIGN (ENU)
//----------------------------------

template<is_datum Datum, template<class> class VecUnits, typename T, typename S>
    requires(std::is_arithmetic_v<S>)
VectorENU<Datum, VecUnits, T>& operator*=(VectorENU<Datum, VecUnits, T>& v, const S s)
{
	v = v * s;
	return v;
}

template<is_datum Datum, template<class> class VecUnits, typename T, typename S>
    requires(std::is_arithmetic_v<S>)
VectorENU<Datum, VecUnits, T>& operator/=(VectorENU<Datum, VecUnits, T>& v, const S s)
{
	v = v / s;
	return v;
}

#endif    // vectorENU_h
