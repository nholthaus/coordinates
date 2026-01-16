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
/// @file	vectorNED.h
/// @brief	Defines the VectorNED class.
//
//--------------------------------------------------------------------------------------------------

#ifndef vectorNED_h
#define vectorNED_h

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

inline namespace coordinates
{
	namespace detail
	{
		inline void requireSameFrameData(const FrameData& lhs, const FrameData& rhs, const char* message)
		{
#ifndef NDEBUG
			assert((lhs == rhs) && message);
#endif

			if (!(lhs == rhs))
				throw std::logic_error(message);
		}
	}    // namespace detail

	//	----------------------------------------------------------------------------
	//	CLASS		VectorNED
	//  ----------------------------------------------------------------------------
	/// @brief		3D Cartesian vector in local NED coordinates.
	/// @details
	///			A VectorNED represents a directed quantity (north, east, down) expressed in a
	///			local tangent NED frame.
	///
	///			Unlike ECEF, an NED frame is defined relative to an origin, and so VectorNED is
	///			*anchored* to its originating frame data. Arithmetic between NED vectors (or
	///			between NED positions and vectors) requires that the frame data match.
	///
	/// @tparam	Datum			Datum defining the horizontal reference used by the underlying NED frame.
	/// @tparam	DistanceUnits	Units used to store each component internally. Defaults to meters.
	/// @tparam	T			Underlying storage type. Defaults to double.
	//  ----------------------------------------------------------------------------
	template<is_datum Datum, template<class> class DistanceUnits = meters, typename T = double>
	class VectorNED
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//		STATIC ERROR CHECKING
		//////////////////////////////////////////////////////////////////////////

		static_assert(units::traits::is_unit_v<DistanceUnits<T>>, "Template parameter `DistanceUnits` must be a unit type.");
		static_assert(is_datum<Datum>, "`Datum` template parameter does not satisfy the datum concept.");
		static_assert(std::is_arithmetic_v<T>, "`T` template parameter must be an arithmetic type.");

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using datum_type         = Datum;
		using reference_frame    = NEDFrame<typename datum_traits<Datum>::horizontal_datum>;
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
		VectorNED()
		    : m_north(0)
		    , m_east(0)
		    , m_down(0)
		    , m_frameData(Datum::epoch())
		{
		}

		/**
		 * @brief		Constructor.
		 * @param[in]	north	North component.
		 * @param[in]	east	East component.
		 * @param[in]	down	Down component.
		 * @param[in]	origin	Geodetic origin anchoring this NED frame.
		 * @param[in]	dateOfObservation	Date of observation.
		 */
		VectorNED(distance_unit_type north,
		          distance_unit_type east,
		          distance_unit_type down,
		          const origin_type& origin,
		          years<>            dateOfObservation = Datum::epoch())
		    : m_north(north)
		    , m_east(east)
		    , m_down(down)
		    , m_frameData(origin.point(), dateOfObservation)
		{
		}

		/**
		 * @brief		Tuple constructor.
		 * @details		Constructs a vector from a tuple of (n,e,d) values anchored to the provided origin.
		 * @param[in]	v	Tuple containing (n,e,d).
		 * @param[in]	origin	Geodetic origin anchoring this NED frame.
		 * @param[in]	dateOfObservation	Date of observation.
		 */
		explicit VectorNED(const tuple_type& v, const origin_type& origin, years<> dateOfObservation = Datum::epoch())
		    : m_north(std::get<0>(v))
		    , m_east(std::get<1>(v))
		    , m_down(std::get<2>(v))
		    , m_frameData(origin.point(), dateOfObservation)
		{
		}

		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS
		//////////////////////////////////////////////////////////////////////////

		[[nodiscard]] distance_unit_type north() const { return m_north; }
		[[nodiscard]] distance_unit_type east() const { return m_east; }
		[[nodiscard]] distance_unit_type down() const { return m_down; }

		[[nodiscard]] frame_data_type frameData() const { return m_frameData; }

		/**
		 * @brief		Vector as a tuple.
		 * @returns		Vector components as a (meters, meters, meters) tuple.
		 */
		[[nodiscard]] tuple_type vector() const { return tuple_type(m_north, m_east, m_down); }

		/**
		 * @brief		Set the vector value from a tuple.
		 */
		void setVector(const tuple_type& v)
		{
			m_north = std::get<0>(v);
			m_east  = std::get<1>(v);
			m_down  = std::get<2>(v);
		}

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

		VectorNED& operator+=(const VectorNED& v)
		{
			requireSameFrameData(m_frameData, v.m_frameData, "VectorNED frame mismatch in operator+=");
			m_north = m_north + v.north();
			m_east  = m_east + v.east();
			m_down  = m_down + v.down();
			return *this;
		}

		VectorNED& operator-=(const VectorNED& v)
		{
			requireSameFrameData(m_frameData, v.m_frameData, "VectorNED frame mismatch in operator-=");
			m_north = m_north - v.north();
			m_east  = m_east - v.east();
			m_down  = m_down - v.down();
			return *this;
		}

		friend std::ostream& operator<<(std::ostream& os, const VectorNED& v) { return os << "(" << v.m_north << ", " << v.m_east << ", " << v.m_down << ")"; }

	private:
		distance_unit_type m_north;
		distance_unit_type m_east;
		distance_unit_type m_down;

		frame_data_type m_frameData;
	};
}    // namespace coordinates

//----------------------------------
//  POSITION/VECTOR ARITHMETIC
//----------------------------------

#include "positionNED.h"

template<is_datum Datum, template<class> class PosUnits, typename T>
VectorNED<Datum, PosUnits, T> operator-(const PositionNED<Datum, PosUnits, T>& lhs,
                                                     const PositionNED<Datum, PosUnits, T>& rhs)
{
	requireSameFrameData(lhs.frameData(), rhs.frameData(), "PositionNED frame mismatch in operator-");
	return VectorNED<Datum, PosUnits, T>(lhs.north() - rhs.north(),
	                                                  lhs.east() - rhs.east(),
	                                                  lhs.down() - rhs.down(),
	                                                  PositionGeodetic<Datum>(lhs.frameData().origin, lhs.date()),
	                                                  lhs.date());
}

template<is_datum Datum, template<class> class PosUnits, template<class> class VecUnits, typename T>
PositionNED<Datum, PosUnits, T> operator+(PositionNED<Datum, PosUnits, T> lhs, const VectorNED<Datum, VecUnits, T>& rhs)
{
	requireSameFrameData(lhs.frameData(), rhs.frameData(), "PositionNED frame mismatch in operator+");
	lhs.setNorth(lhs.north() + rhs.north());
	lhs.setEast(lhs.east() + rhs.east());
	lhs.setDown(lhs.down() + rhs.down());
	return lhs;
}

template<is_datum Datum, template<class> class PosUnits, template<class> class VecUnits, typename T>
PositionNED<Datum, PosUnits, T> operator-(PositionNED<Datum, PosUnits, T> lhs, const VectorNED<Datum, VecUnits, T>& rhs)
{
	requireSameFrameData(lhs.frameData(), rhs.frameData(), "PositionNED frame mismatch in operator-");
	lhs.setNorth(lhs.north() - rhs.north());
	lhs.setEast(lhs.east() - rhs.east());
	lhs.setDown(lhs.down() - rhs.down());
	return lhs;
}

#endif    // vectorNED_h
