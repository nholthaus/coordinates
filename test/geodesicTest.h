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

#ifndef geodesicTest_h
#define geodesicTest_h

//------------------------
//	INCLUDES
//------------------------

#include <gtest/gtest.h>

#include "gtest_units.h"
#include "positionGeodetic.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		GeodesicTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for geodesic algorithms (inverse/direct).
	///	@details	Truth data derived from GeographicLib's GeodSolve examples (WGS84 ellipsoid).
	//  ----------------------------------------------------------------------------
	class GeodesicTest : public ::testing::Test
	{
	};

	TEST_F(GeodesicTest, inverse_JFK_to_Singapore_truthData)
	{
		using G = PositionGeodetic<WGS84_G1674>;

		// JFK Airport: 40:38:23N 073:46:44W
		G jfk(40.63972222222222_deg, -73.77888888888889_deg, 0.0_m);

		// Singapore Changi: 01:21:33N 103:59:22E
		G sin(1.3591666666666669_deg, 103.98944444444444_deg, 0.0_m);

		auto result = geodesicInverse<WGS84_G1674>(jfk, sin);

		// GeodSolve -i -: -p 0 output:
		//   003:18:29.9 177:29:09.2 15347628
		EXPECT_UNITS_NEAR(15347628.0_m, result.distance(), 1.0_m);
		EXPECT_UNITS_NEAR(3.308305555555555_deg, result.initialBearing(), 1.0e-4_deg);
		EXPECT_UNITS_NEAR(177.48588888888887_deg, result.finalBearing(), 1.0e-4_deg);
	}

	TEST_F(GeodesicTest, direct_JFK_to_Singapore_roundTrip)
	{
		using G = PositionGeodetic<WGS84_G1674>;

		G jfk(40.63972222222222_deg, -73.77888888888889_deg, 0.0_m);

		// Solve direct using the GeodSolve example values.
		auto direct = geodesicDirect<WGS84_G1674>(jfk, 3.308305555555555_deg, 15347628.0_m);

		// Expected destination is Singapore Changi (from the inverse example inputs).
		EXPECT_UNITS_NEAR(1.3591666666666669_deg, direct.destination().latitude(), 5.0e-6_deg);
		EXPECT_UNITS_NEAR(103.98944444444444_deg, direct.destination().longitude(), 5.0e-6_deg);

		// Expected final bearing from GeodSolve output.
		EXPECT_UNITS_NEAR(177.48588888888887_deg, direct.finalBearing(), 1.0e-4_deg);
	}

	TEST_F(GeodesicTest, wrapHelpers_basic)
	{
		EXPECT_UNITS_EQ(0.0_deg, wrap360(360.0_deg));
		EXPECT_UNITS_EQ(359.0_deg, wrap360(-1.0_deg));
		EXPECT_UNITS_EQ(-1.0_deg, wrap180(359.0_deg));
		EXPECT_UNITS_EQ(180.0_deg, wrap180(180.0_deg));
	}
}

#endif
