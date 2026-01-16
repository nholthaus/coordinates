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

#ifndef losTest_h
#define losTest_h

//------------------------
//	INCLUDES
//------------------------

#include <gtest/gtest.h>

#include "gtest_units.h"
#include "positionECEF.h"
#include "positionGeodetic.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		LineOfSightTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for ellipsoid-only line-of-sight checks.
	//  ----------------------------------------------------------------------------
	class LineOfSightTest : public ::testing::Test
	{
	};

	TEST_F(LineOfSightTest, surfacePointsOppositeSides_blocked)
	{
		using Datum = WGS84_G1674;
		using G     = PositionGeodetic<Datum>;
		using E     = PositionECEF<Datum>;

		// Two sea-level points on opposite sides of the Earth.
		G p1(0.0_deg, 0.0_deg, 0.0_m);
		G p2(0.0_deg, 180.0_deg, 0.0_m);

		E e1(p1);
		E e2(p2);

		EXPECT_FALSE(isLineOfSight<Datum>(e1, e2));
	}

	TEST_F(LineOfSightTest, nearbyHighAltitudePoints_clear)
	{
		using Datum = WGS84_G1674;
		using G     = PositionGeodetic<Datum>;
		using E     = PositionECEF<Datum>;

		// Two points at 10 km altitude, separated by 0.1 degrees in longitude at the equator.
		G p1(0.0_deg, 0.0_deg, 10000.0_m);
		G p2(0.0_deg, 0.1_deg, 10000.0_m);

		E e1(p1);
		E e2(p2);

		EXPECT_TRUE(isLineOfSight<Datum>(e1, e2));
	}
}

#endif
