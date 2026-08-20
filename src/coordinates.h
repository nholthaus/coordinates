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

#ifndef coordinates_h
#define coordinates_h

#if defined(_MSC_VER)
#pragma warning(disable : 4503)    // decorated name length exceeded, name was truncated. This only affects debugging.
#endif

//------------------------
//	INCLUDES
//------------------------
#include <CAS.h>
#include <threadPool.h>
#include <units.h>

#include <abstractTile.h>
#include <abstractTileManager.h>
#include <algorithm.h>
#include <angles.h>

#include <coordinate.h>
#include <datum.h>
#include <frameAxes.h>
#include <frameOfReference.h>
#include <heights.h>
#include <latitudeConversion.h>
#include <ranges.h>
#include <topography.h>

#include <point.h>
#include <positionAER.h>
#include <positionECEF.h>
#include <positionENU.h>
#include <positionGeodetic.h>
#include <positionNED.h>

#include <vector.h>
#include <vectorECEF.h>
#include <vectorENU.h>
#include <vectorNED.h>

#include <kinematics.h>

#include <bodyFrame.h>
#include <pose.h>
#include <kinematicState.h>

#include <ray.h>
#include <sensorFieldOfView.h>
#include <entity.h>

#include <hillshadeCanvas.h>

#if defined(COORDINATES_ENABLE_LOS) && COORDINATES_ENABLE_LOS
#include <lineOfSight.h>
#endif

inline namespace coordinates
{
	//----------------------------------
	//	CONVENIENCE CLASSES
	//----------------------------------
	using ITRS    = PositionECEF<ITRS2008>;
	using ECEF    = PositionECEF<WGS84_G1674>;
	using LLA     = PositionGeodetic<WGS84_G1674>;
	using ENU     = PositionENU<WGS84_G1674>;
	using NED     = PositionNED<WGS84_G1674>;
	using AER     = PositionAER<WGS84_G1674>;
	using VecECEF = VectorECEF<WGS84_G1674>;
	using VecENU  = VectorENU<WGS84_G1674>;
	using VecNED  = VectorNED<WGS84_G1674>;
}    // namespace coordinates

#endif    // coordinates_h