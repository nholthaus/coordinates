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

#ifndef coordinateTest_h__
#define coordinateTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <type_traits>

#include <coordinate.h>
#include <coordinates.h>

using namespace coordinates;
using namespace coordinates::coordinateFrames;
using namespace units;
using namespace units::length;
using namespace units::angle;

namespace
{
	class CoordinateTest : public ::testing::Test
	{
	protected:
		CoordinateTest() {}
		~CoordinateTest() override {}
		void SetUp() override {}
		void TearDown() override {}
	};

	using WgsHoriz = horizontalDatums::WGS84_G1674;
	using Wgs      = datums::WGS84_G1674;
	using EcefCoord = Coordinate<ECEFFrame<WgsHoriz>, CartesianTuple>;
	using GeoCoord  = Coordinate<Geodetic3DFrame<Wgs>, SphericalTuple>;

	// The unified body satisfies the same structural contract the position classes do.
	static_assert(coordinates::traits::is_point<EcefCoord>);
	static_assert(coordinates::traits::is_point<GeoCoord>);

	// C-1a: prove the shared body's storage + point interface directly (no aliases wired yet).
	TEST_F(CoordinateTest, storesAndReturnsTheTuple)
	{
		EcefCoord c(CartesianTuple(1.0_m, 2.0_m, 3.0_m));
		EXPECT_UNITS_EQ(1.0_m, std::get<0>(c.point()));
		EXPECT_UNITS_EQ(2.0_m, std::get<1>(c.point()));
		EXPECT_UNITS_EQ(3.0_m, std::get<2>(c.point()));

		c.setPoint(CartesianTuple(4.0_m, 5.0_m, 6.0_m));
		EXPECT_UNITS_EQ(4.0_m, std::get<0>(c.point()));
		EXPECT_UNITS_EQ(6.0_m, std::get<2>(c.point()));
	}

	// Component constructor forwards into the tuple; heterogeneous frames (geodetic = angle/angle/length) work.
	TEST_F(CoordinateTest, componentConstructor)
	{
		GeoCoord g(10.0_deg, 20.0_deg, 100.0_m);
		EXPECT_UNITS_EQ(10.0_deg, std::get<0>(g.point()));
		EXPECT_UNITS_EQ(20.0_deg, std::get<1>(g.point()));
		EXPECT_UNITS_EQ(100.0_m, std::get<2>(g.point()));
	}

	// The converting constructor runs the frame pipeline: an ECEF coordinate built from a geodetic one must
	// match the legacy PositionECEF-from-LLA truth (WGS84, from the existing positionGeodetic test data).
	TEST_F(CoordinateTest, convertingConstructorMatchesLegacy)
	{
		// LLA (42.459284, -71.268002, 50 m) -> ECEF, MATLAB truth from positionGeodeticTest.
		GeoCoord  lla(42.459284_deg, -71.268002_deg, 50.0_m);
		EcefCoord ecef(lla);
		EXPECT_UNITS_NEAR(1513460.90425574_m, std::get<0>(ecef.point()), 1.0e-6_m);
		EXPECT_UNITS_NEAR(-4463118.57366578_m, std::get<1>(ecef.point()), 1.0e-6_m);
		EXPECT_UNITS_NEAR(4283412.81104103_m, std::get<2>(ecef.point()), 1.0e-6_m);

		// round-trip back to geodetic.
		GeoCoord back(ecef);
		EXPECT_UNITS_NEAR(42.459284_deg, std::get<0>(back.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(-71.268002_deg, std::get<1>(back.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(50.0_m, std::get<2>(back.point()), 1.0e-6_m);
	}
}

#endif    // coordinateTest_h__
