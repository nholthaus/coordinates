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

#ifndef geodesicInverseResult_h
#define geodesicInverseResult_h

//------------------------
//	INCLUDES
//------------------------

#include <units.h>

#include "angles.h"
#include "ranges.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		GeodesicInverseResult
	//  ----------------------------------------------------------------------------
	///	@brief		Result of an inverse geodesic solve on an ellipsoid.
	///	@details	Provides the surface distance between two geodetic points and the forward azimuths
	///				(initial bearing at point 1 and final bearing at point 2). Bearings are normalized
	///				to the range [0, 360).
	//  ----------------------------------------------------------------------------
	class GeodesicInverseResult
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		default constructor
		 * @details		Constructs a zero-valued result.
		 */
		explicit constexpr GeodesicInverseResult()
		    : m_distance(0.0_m)
		    , m_initialBearing(0.0_deg)
		    , m_finalBearing(0.0_deg)
		{
		}

		/**
		 * @brief		constructor
		 * @details		Constructs a result with explicit values.
		 * @param[in]	distance			Surface distance between points.
		 * @param[in]	initialBearing		Forward azimuth at point 1.
		 * @param[in]	finalBearing		Forward azimuth at point 2.
		 */
		explicit constexpr GeodesicInverseResult(const meters<> distance, const degrees<> initialBearing, const degrees<> finalBearing)
		    : m_distance(distance)
		    , m_initialBearing(initialBearing)
		    , m_finalBearing(finalBearing)
		{
		}

	public:
		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		Returns the surface distance.
		 * @return		Surface distance between the two points.
		 */
		constexpr ranges::Geodesic distance() const
		{
			return ranges::Geodesic(m_distance);
		}

		/**
		 * @brief		Returns the forward azimuth at point 1.
		 * @return		Initial bearing at the first point, normalized to [0, 360).
		 */
		constexpr angles::Azimuth initialBearing() const
		{
			return angles::Azimuth(m_initialBearing);
		}

		/**
		 * @brief		Returns the forward azimuth at point 2.
		 * @return		Final bearing at the second point, normalized to [0, 360).
		 */
		constexpr angles::Azimuth finalBearing() const
		{
			return angles::Azimuth(m_finalBearing);
		}

	private:
		meters<> m_distance;
		degrees<> m_initialBearing;
		degrees<> m_finalBearing;
	};
}

#endif
