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

#ifndef geodesicDirectResult_h
#define geodesicDirectResult_h

//------------------------
//	INCLUDES
//------------------------

#include <units.h>

#include "angles.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		GeodesicDirectResult
	//  ----------------------------------------------------------------------------
	///	@brief		Result of a direct geodesic solve on an ellipsoid.
	///	@details	Given a start point, initial bearing, and distance, the direct problem returns the
	///				destination point and the forward azimuth at the destination.
	/// @tparam		GeodeticPointType	Geodetic point type of the destination.
	//  ----------------------------------------------------------------------------
	template<class GeodeticPointType>
	class GeodesicDirectResult
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//		STATIC ERROR CHECKING
		//////////////////////////////////////////////////////////////////////////

		static_assert(coordinates::is_point<GeodeticPointType>, "Template parameter `GeodeticPointType` does not satisfy the `point` concept.");

	public:
		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		default constructor
		 * @details		Constructs a default destination point and a 0-degree azimuth.
		 */
		explicit GeodesicDirectResult()
		    : m_destination()
		    , m_finalBearing(0.0_deg)
		{
		}

		/**
		 * @brief		constructor
		 * @details		Constructs a result with explicit values.
		 * @param[in]	destination		Destination geodetic point.
		 * @param[in]	finalBearing	Forward azimuth at destination, normalized to [0, 360).
		 */
		explicit GeodesicDirectResult(const GeodeticPointType& destination, const degrees<> finalBearing)
		    : m_destination(destination)
		    , m_finalBearing(finalBearing)
		{
		}

	public:
		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		Returns the destination point.
		 * @return		Destination point on the ellipsoid.
		 */
		const GeodeticPointType& destination() const
		{
			return m_destination;
		}

		/**
		 * @brief		Returns the forward azimuth at destination.
		 * @return		Final bearing at destination, normalized to [0, 360).
		 */
		angles::Azimuth finalBearing() const
		{
			return angles::Azimuth(m_finalBearing);
		}

	private:
		GeodeticPointType m_destination;
		degrees<> m_finalBearing;
	};
}

#endif
