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
///	@file			dted.h
///	@brief			definition of dted topography class
/// @details
//
//--------------------------------------------------------------------------------------------------


#ifndef dted_h
#define dted_h

//------------------------
//	INCLUDES
//------------------------

#include "datum.h"
#include "topography.h"
#include "dtedTileManager.h"

inline namespace coordinates
{
	inline namespace topography
	{
		//	----------------------------------------------------------------------------
		//	CLASS		DTED
		//  ----------------------------------------------------------------------------
		///	@brief		Topography class for reading Digital Terrain Elevation Data (DTED)
		///	@details	This class is intended for use as part of a coord library `datum`
		///				definition. DTED is based on the EGM96 Geoid, which is itself
		///				referenced to the WGS84 (GPS) Ellipsoid.
		//  ----------------------------------------------------------------------------
		class DTED : public Topography<geoids::EGM96>
		{
		public:
			static meters<> orthometricHeight(const degrees<>& latitude, const degrees<>& longitude)
			{
				return DTEDTileManager::instance().elevation(latitude, longitude);
			}
		};
	}

	inline namespace datums
	{
		//	----------------------------------------------------------------------------
		//	CLASS		WGS84_G1674_AGL
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		//  ----------------------------------------------------------------------------
		class WGS84_G1674_AGL : public Datum<horizontalDatums::WGS84_G1674, DTED>{};
	}
}


#endif // dted_h