#ifndef coordinate_h__
#define coordinate_h__

#pragma warning( disable : 4503 )	// decorated name length exceeded, name was truncated. This only affects debugging.

//------------------------
//	INCLUDES
//------------------------
#include <units.h>
#include <CAS.h>
#include <threadPool.h>

#include <coord/abstractTile.h>
#include <coord/abstractTileManager.h>
#include <coord/algorithm.h>

#include <coord/datum.h>
#include <coord/topography.h>
#include <coord/frameOfReference.h>

#include <coord/point.h>
#include <coord/positionECEF.h>
#include <coord/positionGeodetic.h>
#include <coord/positionENU.h>
#include <coord/positionNED.h>
#include <coord/positionAER.h>
#include <coord/positionXYZ.h>

namespace coord
{
	//----------------------------------
	//	CONVENIENCE CLASSES
	//----------------------------------
	using ITRS = PositionECEF<coord::datums::ITRS2008>;
	using ECEF = PositionECEF<coord::datums::WGS84_G1674>;
	using LLA = PositionGeodetic<coord::datums::WGS84_G1674>;
	using ENU = PositionENU<coord::datums::WGS84_G1674>;
	using NED = PositionNED<coord::datums::WGS84_G1674>;
	using AER = PositionAER<coord::datums::WGS84_G1674>;
}

#endif // _h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: