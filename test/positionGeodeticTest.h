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

#ifndef LLATest_h__
#define LLATest_h__

//------------------------
//	INCLUDES
//------------------------

#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include "gtest_units.h"

using namespace coordinates;
using namespace units;
using namespace units::length;
using namespace units::angle;
using namespace units::time;

namespace
{
	// The fixture for testing class Foo.
	class PositionGeodeticTest : public ::testing::Test {
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		PositionGeodeticTest()
		{
			// You can do set-up work for each test here.
		}

		virtual ~PositionGeodeticTest()
		{
			// You can do clean-up work that doesn't throw exceptions here.
		}

		// If the constructor and destructor are not enough for setting up
		// and cleaning up each test, you can define the following methods:

		virtual void SetUp()
		{
			// Code here will be called immediately after the constructor (right
			// before each test).
		}

		virtual void TearDown()
		{
			// Code here will be called immediately after each test (right
			// before the destructor).
		}

		// Objects declared here can be used by all tests in the test case for Foo.
	};

	TEST_F(PositionGeodeticTest, is_point)
	{
		EXPECT_TRUE(coordinates::traits::is_point<LLA>);
	}

	TEST_F(PositionGeodeticTest, defaultConstructor)
	{
		LLA lla;
		EXPECT_EQ(0.0_deg, std::get<0>(lla.point()));
		EXPECT_EQ(0.0_deg, std::get<1>(lla.point()));
		EXPECT_EQ(0.0_m, std::get<2>(lla.point()));
	}

	TEST_F(PositionGeodeticTest, constructor)
	{
		LLA lla(1.0_deg, 2.0_deg, 3.0_m);
		EXPECT_EQ(1.0_deg, std::get<0>(lla.point()));
		EXPECT_EQ(2.0_deg, std::get<1>(lla.point()));
		EXPECT_EQ(3.0_m, std::get<2>(lla.point()));

		// different units
		PositionGeodetic<datums::WGS84_G1674, units::angle::radians, units::length::miles> lla2(radians(1.0), radians(2.0), 3.0_mi);
		EXPECT_EQ(radians(1.0), std::get<0>(lla2.point()));
		EXPECT_EQ(radians(2.0), std::get<1>(lla2.point()));
		EXPECT_EQ(3.0_mi, std::get<2>(lla2.point()));

		// with epoch
		LLA lla3(4.0_deg, 5.0_deg, 6.0_m, 2016.4_yr);
		EXPECT_EQ(4.0_deg, std::get<0>(lla3.point()));
		EXPECT_EQ(5.0_deg, std::get<1>(lla3.point()));
		EXPECT_EQ(6.0_m, std::get<2>(lla3.point()));
		EXPECT_EQ(2016.4_yr, lla3.frameData().date);
	}


	// TESTS THE  MEMBER
	TEST_F(PositionGeodeticTest, tupleConstructor)
	{
		coordinates::SphericalTuple t(1.0_deg, 2.0_deg, 3.0_m);
		LLA lla(t);
		EXPECT_EQ(1.0_deg, std::get<0>(lla.point()));
		EXPECT_EQ(2.0_deg, std::get<1>(lla.point()));
		EXPECT_EQ(3.0_m, std::get<2>(lla.point()));
		EXPECT_EQ(FrameData(2005.0_yr), lla.frameData());

		// different observation date
		LLA lla2(t, 2016.413_yr);
		EXPECT_EQ(1.0_deg, std::get<0>(lla2.point()));
		EXPECT_EQ(2.0_deg, std::get<1>(lla2.point()));
		EXPECT_EQ(3.0_m, std::get<2>(lla2.point()));
		EXPECT_EQ(FrameData(2016.413_yr), lla2.frameData());
	}

	TEST_F(PositionGeodeticTest, copyConstructor)
	{
		LLA lla(1.0_deg, 2.0_deg, 3.0_m, 2016.413_yr);
		EXPECT_EQ(1.0_deg, std::get<0>(lla.point()));
		EXPECT_EQ(2.0_deg, std::get<1>(lla.point()));
		EXPECT_EQ(3.0_m, std::get<2>(lla.point()));
		EXPECT_EQ(FrameData(2016.413_yr), lla.frameData());

		LLA lla2(lla);
		EXPECT_EQ(1.0_deg, std::get<0>(lla2.point()));
		EXPECT_EQ(2.0_deg, std::get<1>(lla2.point()));
		EXPECT_EQ(3.0_m, std::get<2>(lla2.point()));
		EXPECT_EQ(FrameData(2016.413_yr), lla2.frameData());
	}

	TEST_F(PositionGeodeticTest, implicitConversionConstructor_fromECEF)
	{
		// WGS84 ECEF TO LLA
		// source: Matlab
		ECEF test1(1513460.90425574_m, -4463118.57366578_m, 4283412.81104103_m);
		ECEF test2(-6161359.99808945_m, 1339960.38688496_m, 960697.380382176_m);
		ECEF test3(5027491.76199413_m, 1672214.28779206_m, -3540753.52360714_m);
		ECEF test4(4852.527699157_km, -315.443985333516_km, 4113.40113079432_km);

		LLA result1(test1);
		LLA result2(test2);
		LLA result3(test3);
		LLA result4(test4);

		EXPECT_UNITS_NEAR(42.459284_deg, std::get<0>(result1.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(-71.268002_deg, std::get<1>(result1.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(50.0_m, std::get<2>(result1.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(8.72053_deg, std::get<0>(result2.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(167.730482_deg, std::get<1>(result2.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(500.0_m, std::get<2>(result2.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(-33.932048_deg, std::get<0>(result3.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(18.397849_deg, std::get<1>(result3.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(1000.0_m, std::get<2>(result3.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(40.417638_deg, std::get<0>(result4.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(-3.719343_deg, std::get<1>(result4.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(0.0_m, std::get<2>(result4.point()), 1.0e-8_m);
	}

	TEST_F(PositionGeodeticTest, implicitConversionConstructor_toECEF)
	{
		// WGS84 LLA to ECEF
		// truth data from: Louis' matlab calculations
		LLA test1(42.459284_deg, -71.268002_deg, 50.0_m);
		LLA test2(8.72053_deg, 167.730482_deg, 500.0_m);
		LLA test3(-33.932048_deg, 18.397849_deg, 1000.0_m);
		LLA test4(40.417638_deg, -3.719343_deg, 0.0_km);

		ECEF result1(test1);
		ECEF result2(test2);
		ECEF result3(test3);
		ECEF result4(test4);

		EXPECT_UNITS_NEAR(1513460.90425574_m, std::get<0>(result1.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(-4463118.57366578_m, std::get<1>(result1.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(4283412.81104103_m, std::get<2>(result1.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(-6161359.99808945_m, std::get<0>(result2.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(1339960.38688496_m, std::get<1>(result2.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(960697.380382176_m, std::get<2>(result2.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(5027491.76199413_m, std::get<0>(result3.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(1672214.28779206_m, std::get<1>(result3.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(-3540753.52360714_m, std::get<2>(result3.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(4852527.699157_m, std::get<0>(result4.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(-315443.985333516_m, std::get<1>(result4.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(4113401.13079432_m, std::get<2>(result4.point()), 1.0e-8_m);
	}

	TEST_F(PositionGeodeticTest, implicitConversionConstructor_WGS84toNAD83)
	{
		// Datum conversions
		// Source: http://webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php?locale=en (latitude positive west disabled, epoch 2000, wgs84 == itrf2008)
		LLA boston_wgs84(42.354991_deg, -71.065599_deg, -1.2192_m, 2000.0_yr);
		LLA nevada_wgs84(37.235000_deg, -115.811100_deg, -0.6969_m, 2000.0_yr);
		PositionGeodetic<datums::NAD83> boston_nad83(42.35498209_deg, -71.06559782_deg, 0.006_m);
		PositionGeodetic<datums::NAD83> nevada_nad83(37.23499488_deg, -115.81108746_deg, 0.013_m);

		PositionGeodetic<datums::NAD83> bostonCalculated(boston_wgs84);

		EXPECT_UNITS_NEAR(std::get<0>(boston_nad83.point()), std::get<0>(bostonCalculated.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(boston_nad83.point()), std::get<1>(bostonCalculated.point()), 5.0e-9_deg);

		PositionGeodetic<datums::NAD83> nevadaCalculated(nevada_wgs84);

		EXPECT_UNITS_NEAR(std::get<0>(nevada_nad83.point()), std::get<0>(nevadaCalculated.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(nevada_nad83.point()), std::get<1>(nevadaCalculated.point()), 5.0e-9_deg);

		PositionGeodetic<datums::NAD83> bostonCalculated3D(boston_wgs84);

		EXPECT_UNITS_NEAR(std::get<0>(boston_nad83.point()), std::get<0>(bostonCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(boston_nad83.point()), std::get<1>(bostonCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<2>(boston_nad83.point()), std::get<2>(bostonCalculated3D.point()), 5.0e-4_deg);

		PositionGeodetic<datums::NAD83> nevadaCalculated3D(nevada_wgs84);

		EXPECT_UNITS_NEAR(std::get<0>(nevada_nad83.point()), std::get<0>(nevadaCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(nevada_nad83.point()), std::get<1>(nevadaCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<2>(nevada_nad83.point()), std::get<2>(nevadaCalculated3D.point()), 5.0e-4_deg);
	}

	TEST_F(PositionGeodeticTest, implicitConversionConstructor_NAD83toWGS84)
	{
		// Datum conversions
		// Source: http://webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php?locale=en (latitude positive west disabled, epoch 2000, wgs84 == itrf2008)
		LLA boston_wgs84(42.354991_deg, -71.065599_deg, -1.2192_m);
		LLA nevada_wgs84(37.235000_deg, -115.811100_deg, -0.6969_m);
		PositionGeodetic<datums::NAD83> boston_nad83(42.35498209_deg, -71.06559782_deg, 0.006_m, 2000.0_yr);
		PositionGeodetic<datums::NAD83> nevada_nad83(37.23499488_deg, -115.81108746_deg, 0.013_m, 2000.0_yr);

		LLA bostonCalculated(boston_nad83);

		EXPECT_UNITS_NEAR(std::get<0>(boston_wgs84.point()), std::get<0>(bostonCalculated.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(boston_wgs84.point()), std::get<1>(bostonCalculated.point()), 5.0e-9_deg);

		LLA nevadaCalculated(nevada_nad83);

		EXPECT_UNITS_NEAR(std::get<0>(nevada_wgs84.point()), std::get<0>(nevadaCalculated.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(nevada_wgs84.point()), std::get<1>(nevadaCalculated.point()), 5.0e-9_deg);

		LLA bostonCalculated3D(boston_nad83);

		EXPECT_UNITS_NEAR(std::get<0>(boston_wgs84.point()), std::get<0>(bostonCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(boston_wgs84.point()), std::get<1>(bostonCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<2>(boston_wgs84.point()), std::get<2>(bostonCalculated3D.point()), 5.0e-4_deg);

		LLA nevadaCalculated3D(nevada_nad83);

		EXPECT_UNITS_NEAR(std::get<0>(nevada_wgs84.point()), std::get<0>(nevadaCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(nevada_wgs84.point()), std::get<1>(nevadaCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<2>(nevada_wgs84.point()), std::get<2>(nevadaCalculated3D.point()), 5.0e-4_deg);
	}

	TEST_F(PositionGeodeticTest, implicitConversionConstructor_WGS84MSLtoWGS84)
	{
		// Datum conversions
		// Source: http://vdatum.noaa.gov/ (latitude positive west disabled, epoch 2000, wgs84 == itrf2008)
		// choose a lat/lon that doesn't need to be interpolated in the geoid data so that we get a good match w/ vdatum
		PositionGeodetic<datums::NAD83_NAVD88> boston_nad83_navd88(42.35_deg, -71.05_deg, 0.0_m);
		PositionGeodetic<datums::NAD83> boston_nad83(42.35_deg, -71.05_deg, -27.7517_m);

		PositionGeodetic<datums::NAD83> bostonCalculated(boston_nad83_navd88);

		EXPECT_UNITS_NEAR(std::get<0>(boston_nad83.point()), std::get<0>(bostonCalculated.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(boston_nad83.point()), std::get<1>(bostonCalculated.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<2>(boston_nad83.point()), std::get<2>(bostonCalculated.point()), 5.0e-5_deg);
	}

	TEST_F(PositionGeodeticTest, assignment)
	{
		LLA lla(1.0_deg, 2.0_deg, 3.0_m, 2016.413_yr);
		EXPECT_EQ(1.0_deg, std::get<0>(lla.point()));
		EXPECT_EQ(2.0_deg, std::get<1>(lla.point()));
		EXPECT_EQ(3.0_m, std::get<2>(lla.point()));
		EXPECT_EQ(FrameData(2016.413_yr), lla.frameData());

		LLA lla2 = lla;
		EXPECT_EQ(1.0_deg, std::get<0>(lla2.point()));
		EXPECT_EQ(2.0_deg, std::get<1>(lla2.point()));
		EXPECT_EQ(3.0_m, std::get<2>(lla2.point()));
		EXPECT_EQ(FrameData(2016.413_yr), lla2.frameData());
	}

	TEST_F(PositionGeodeticTest, implicitConversionAssignment_fromECEF)
	{
		// WGS84 ECEF TO LLA
		// source: Matlab
		ECEF test1(1513460.90425574_m, -4463118.57366578_m, 4283412.81104103_m);
		ECEF test2(-6161359.99808945_m, 1339960.38688496_m, 960697.380382176_m);
		ECEF test3(5027491.76199413_m, 1672214.28779206_m, -3540753.52360714_m);
		ECEF test4(4852.527699157_km, -315.443985333516_km, 4113.40113079432_km);

		LLA result1 = test1;
		LLA result2 = test2;
		LLA result3 = test3;
		LLA result4 = test4;

		EXPECT_UNITS_NEAR(42.459284_deg, std::get<0>(result1.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(-71.268002_deg, std::get<1>(result1.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(50.0_m, std::get<2>(result1.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(8.72053_deg, std::get<0>(result2.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(167.730482_deg, std::get<1>(result2.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(500.0_m, std::get<2>(result2.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(-33.932048_deg, std::get<0>(result3.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(18.397849_deg, std::get<1>(result3.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(1000.0_m, std::get<2>(result3.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(40.417638_deg, std::get<0>(result4.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(-3.719343_deg, std::get<1>(result4.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(0.0_m, std::get<2>(result4.point()), 1.0e-8_m);
	}

	TEST_F(PositionGeodeticTest, implicitConversionAssignment_toECEF)
	{
		// WGS84 LLA to ECEF
		// truth data from: Louis' matlab calculations
		LLA test1(42.459284_deg, -71.268002_deg, 50.0_m);
		LLA test2(8.72053_deg, 167.730482_deg, 500.0_m);
		LLA test3(-33.932048_deg, 18.397849_deg, 1000.0_m);
		LLA test4(40.417638_deg, -3.719343_deg, 0.0_km);

		ECEF result1 = test1;
		ECEF result2 = test2;
		ECEF result3 = test3;
		ECEF result4 = test4;

		EXPECT_UNITS_NEAR(1513460.90425574_m, std::get<0>(result1.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(-4463118.57366578_m, std::get<1>(result1.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(4283412.81104103_m, std::get<2>(result1.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(-6161359.99808945_m, std::get<0>(result2.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(1339960.38688496_m, std::get<1>(result2.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(960697.380382176_m, std::get<2>(result2.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(5027491.76199413_m, std::get<0>(result3.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(1672214.28779206_m, std::get<1>(result3.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(-3540753.52360714_m, std::get<2>(result3.point()), 1.0e-8_m);

		EXPECT_UNITS_NEAR(4852527.699157_m, std::get<0>(result4.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(-315443.985333516_m, std::get<1>(result4.point()), 1.0e-8_m);
		EXPECT_UNITS_NEAR(4113401.13079432_m, std::get<2>(result4.point()), 1.0e-8_m);

		// truth data from: Part 1 Guidelines IOGP report number 430 - 1, describing the GIGS process
		//	- See more at : http ://www.iogp.org/geomatics#2521115-gigs

		ECEF actual0(LLA(80.0_deg, 150.0_deg, 1214.137_m));
		ECEF actual1(LLA(80.0_deg, 150.0_deg, 0.0_m));
		ECEF actual2(LLA(60.00475191_deg, 119.99524538_deg, 619.632_m));
		ECEF actual3(LLA(60.00475258_deg, 119.99524470_deg, -280.368_m));
		ECEF actual4(LLA(30.0_deg, 60.0_deg, 189.569_m));
		ECEF actual5(LLA(30.0_deg, 60.0_deg, 0.0_m));
		ECEF actual6(LLA(0.00392509_deg, -0.00100615_deg, -202.588_m));
		ECEF actual7(LLA(0.00392695_deg, -0.00100662_deg, -3202.588_m));
		ECEF actual8(LLA(0.00393129_deg, -0.00100773_deg, -10202.588_m));
		ECEF actual9(LLA(-30.0_deg, -60.0_deg, 0.0_m));
		ECEF actual10(LLA(-30.0_deg, -60.0_deg, -526.476_m));
		ECEF actual11(LLA(-30.0_deg, -60.0_deg, -571.476_m));
		ECEF actual12(LLA(-59.99934884_deg, -119.99323757_deg, -935.100_m));
		ECEF actual13(LLA(-59.99934874_deg, -119.99323663_deg, -1835.100_m));
		ECEF actual14(LLA(-80.0_deg, -150.0_deg, 0.0_m));
		ECEF actual15(LLA(-80.0_deg, -150.0_deg, -971.255_m));
		ECEF actual16(LLA(-80.0_deg, -150.0_deg, -3316.255_m));
		ECEF actual17(LLA(70.00490733_deg, -179.99706624_deg, -223.618_m));
		ECEF actual18(LLA(50.0_deg, -135.0_deg, 0.0_m));
		ECEF actual19(LLA(25.00366329_deg, -89.99632465_deg, -274.729_m));
		ECEF actual20(LLA(0.0_deg, 0.0_deg, 0.0_m));
		ECEF actual21(LLA(-37.65282217_deg, 143.92649252_deg, 738.0_m));
		ECEF actual22(LLA(-37.65282206_deg, 143.92649211_deg, -1099.229_m));
		ECEF actual23(LLA(-37.65282187_deg, 143.92649143_deg, -4099.229_m));
		ECEF actual24(LLA(-37.65282143_deg, 143.92648984_deg, -11099.229_m));
		ECEF actual25(LLA(-50.0_deg, 135.0_deg, 0.0_m));
		ECEF actual26(LLA(-70.00224647_deg, -179.99706624_deg, -1039.290_m));

		ECEF expected0(-962479.592_m, 555687.852_m, 6260738.653_m);
		ECEF expected1(-962297.006_m, 555582.435_m, 6259542.961_m);
		ECEF expected2(-1598248.169_m, 2768777.623_m, 5501278.468_m);
		ECEF expected3(-1598023.169_m, 2768387.912_m, 5500499.045_m);
		ECEF expected4(2764210.405_m, 4787752.865_m, 3170468.520_m);
		ECEF expected5(2764128.320_m, 4787610.688_m, 3170373.735_m);
		ECEF expected6(6377934.396_m, -112.000_m, 434.000_m);
		ECEF expected7(6374934.396_m, -112.000_m, 434.000_m);
		ECEF expected8(6367934.396_m, -112.000_m, 434.000_m);
		ECEF expected9(2764128.320_m, -4787610.688_m, -3170373.735_m);
		ECEF expected10(2763900.349_m, -4787215.831_m, -3170110.497_m);
		ECEF expected11(2763880.863_m, -4787182.081_m, -3170087.997_m);
		ECEF expected12(-1598023.169_m, -2768611.912_m, -5499631.045_m);
		ECEF expected13(-1597798.169_m, -2768222.201_m, -5498851.622_m);
		ECEF expected14(-962297.006_m, -555582.435_m, -6259542.961_m);
		ECEF expected15(-962150.945_m, -555498.107_m, -6258586.462_m);
		ECEF expected16(-961798.295_m, -555294.505_m, -6256277.087_m);
		ECEF expected17(-2187336.719_m, -112.000_m, 5971017.093_m);
		ECEF expected18(-2904698.555_m, -2904698.555_m, 4862789.038_m);
		ECEF expected19(371.000_m, -5783593.614_m, 2679326.110_m);
		ECEF expected20(6378137.000_m, 0.000_m, 0.000_m);
		ECEF expected21(-4087095.478_m, 2977467.559_m, -3875457.429_m);
		ECEF expected22(-4085919.959_m, 2976611.233_m, -3874335.274_m);
		ECEF expected23(-4084000.165_m, 2975212.729_m, -3872502.631_m);
		ECEF expected24(-4079520.647_m, 2971949.553_m, -3868226.465_m);
		ECEF expected25(-2904698.555_m, 2904698.555_m, -4862789.038_m);
		ECEF expected26(-2187336.719_m, -112.000_m, -5970149.093_m);

		// test for millimeter accuracy
		std::cout << std::setprecision(15);
		EXPECT_TRUE(expected0.isSame (actual0, 1_mm));
		EXPECT_TRUE(expected1.isSame (actual1, 1_mm));
		EXPECT_TRUE(expected2.isSame (actual2, 1_mm));
		EXPECT_TRUE(expected3.isSame (actual3, 1_mm));
		EXPECT_TRUE(expected4.isSame (actual4, 1_mm));
		EXPECT_TRUE(expected5.isSame (actual5, 1_mm));
		EXPECT_TRUE(expected6.isSame (actual6, 1_mm));
		EXPECT_TRUE(expected7.isSame (actual7, 1_mm));
		EXPECT_TRUE(expected8.isSame (actual8, 1_mm));
		EXPECT_TRUE(expected9.isSame (actual9, 1_mm));
		EXPECT_TRUE(expected10.isSame(actual10, 1_mm));
		EXPECT_TRUE(expected11.isSame(actual11, 1_mm));
		EXPECT_TRUE(expected12.isSame(actual12, 1_mm));
		EXPECT_TRUE(expected13.isSame(actual13, 1_mm));
		EXPECT_TRUE(expected14.isSame(actual14, 1_mm));
		EXPECT_TRUE(expected15.isSame(actual15, 1_mm));
		EXPECT_TRUE(expected16.isSame(actual16, 1_mm));
		EXPECT_TRUE(expected17.isSame(actual17, 1_mm));
		EXPECT_TRUE(expected18.isSame(actual18, 1_mm));
		EXPECT_TRUE(expected19.isSame(actual19, 1_mm));
		EXPECT_TRUE(expected20.isSame(actual20, 1_mm));
		EXPECT_TRUE(expected21.isSame(actual21, 200_mm));	// this value is a bit wonky, while none of the others are. I think it's an issue with the c++ trig functions near the asymptotes.
		EXPECT_TRUE(expected22.isSame(actual22, 1_mm));
		EXPECT_TRUE(expected23.isSame(actual23, 1_mm));
		EXPECT_TRUE(expected24.isSame(actual24, 1_mm));
		EXPECT_TRUE(expected25.isSame(actual25, 1_mm));
		EXPECT_TRUE(expected26.isSame(actual26, 1_mm));
	}

	TEST_F(PositionGeodeticTest, implicitConversionAssignment_WGS84toNAD83)
	{
		// Datum conversions
		// Source: http://webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php?locale=en (latitude positive west disabled, epoch 2000, wgs84 == itrf2008)
		LLA boston_wgs84(42.354991_deg, -71.065599_deg, -1.2192_m, 2000.0_yr);
		LLA nevada_wgs84(37.235000_deg, -115.811100_deg, -0.6969_m, 2000.0_yr);
		PositionGeodetic<datums::NAD83> boston_nad83(42.35498209_deg, -71.06559782_deg, 0.006_m);
		PositionGeodetic<datums::NAD83> nevada_nad83(37.23499488_deg, -115.81108746_deg, 0.013_m);

		PositionGeodetic<datums::NAD83> bostonCalculated = boston_wgs84;

		EXPECT_UNITS_NEAR(std::get<0>(boston_nad83.point()), std::get<0>(bostonCalculated.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(boston_nad83.point()), std::get<1>(bostonCalculated.point()), 5.0e-9_deg);

		PositionGeodetic<datums::NAD83> nevadaCalculated = nevada_wgs84;

		EXPECT_UNITS_NEAR(std::get<0>(nevada_nad83.point()), std::get<0>(nevadaCalculated.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(nevada_nad83.point()), std::get<1>(nevadaCalculated.point()), 5.0e-9_deg);

		PositionGeodetic<datums::NAD83> bostonCalculated3D = boston_wgs84;

		EXPECT_UNITS_NEAR(std::get<0>(boston_nad83.point()), std::get<0>(bostonCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(boston_nad83.point()), std::get<1>(bostonCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<2>(boston_nad83.point()), std::get<2>(bostonCalculated3D.point()), 5.0e-4_deg);

		PositionGeodetic<datums::NAD83> nevadaCalculated3D = nevada_wgs84;

		EXPECT_UNITS_NEAR(std::get<0>(nevada_nad83.point()), std::get<0>(nevadaCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(nevada_nad83.point()), std::get<1>(nevadaCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<2>(nevada_nad83.point()), std::get<2>(nevadaCalculated3D.point()), 5.0e-4_deg);
	}

	TEST_F(PositionGeodeticTest, implicitConversionAssignment_NAD83toWGS84)
	{
		// Datum conversions
		// Source: http://webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php?locale=en (latitude positive west disabled, epoch 2000, wgs84 == itrf2008)
		LLA boston_wgs84(42.354991_deg, -71.065599_deg, -1.2192_m);
		LLA nevada_wgs84(37.235000_deg, -115.811100_deg, -0.6969_m);
		PositionGeodetic<datums::NAD83> boston_nad83(42.35498209_deg, -71.06559782_deg, 0.006_m, 2000.0_yr);
		PositionGeodetic<datums::NAD83> nevada_nad83(37.23499488_deg, -115.81108746_deg, 0.013_m, 2000.0_yr);

		LLA bostonCalculated = boston_nad83;

		EXPECT_UNITS_NEAR(std::get<0>(boston_wgs84.point()), std::get<0>(bostonCalculated.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(boston_wgs84.point()), std::get<1>(bostonCalculated.point()), 5.0e-9_deg);

		LLA nevadaCalculated = nevada_nad83;

		EXPECT_UNITS_NEAR(std::get<0>(nevada_wgs84.point()), std::get<0>(nevadaCalculated.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(nevada_wgs84.point()), std::get<1>(nevadaCalculated.point()), 5.0e-9_deg);

		LLA bostonCalculated3D = boston_nad83;

		EXPECT_UNITS_NEAR(std::get<0>(boston_wgs84.point()), std::get<0>(bostonCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(boston_wgs84.point()), std::get<1>(bostonCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<2>(boston_wgs84.point()), std::get<2>(bostonCalculated3D.point()), 5.0e-4_deg);

		LLA nevadaCalculated3D = nevada_nad83;

		EXPECT_UNITS_NEAR(std::get<0>(nevada_wgs84.point()), std::get<0>(nevadaCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<1>(nevada_wgs84.point()), std::get<1>(nevadaCalculated3D.point()), 5.0e-9_deg);
		EXPECT_UNITS_NEAR(std::get<2>(nevada_wgs84.point()), std::get<2>(nevadaCalculated3D.point()), 5.0e-4_deg);
	}

	TEST_F(PositionGeodeticTest, isNull)
	{
		LLA lla;

		EXPECT_EQ(0.0_deg, std::get<0>(lla.point()));
		EXPECT_EQ(0.0_deg, std::get<1>(lla.point()));
		EXPECT_EQ(0.0_m, std::get<2>(lla.point()));

		EXPECT_TRUE(lla.isNull());

		LLA lla2(1.0_deg, 2.0_deg, 3.0_m);

		EXPECT_EQ(1.0_deg, std::get<0>(lla2.point()));
		EXPECT_EQ(2.0_deg, std::get<1>(lla2.point()));
		EXPECT_EQ(3.0_m, std::get<2>(lla2.point()));

		EXPECT_FALSE(lla2.isNull());
	}

	TEST_F(PositionGeodeticTest, isSame)
	{
		LLA lla(42.3601_deg, -71.0589_deg, 0.0_m);
		LLA exactlySame(42.3601_deg, -71.0589_deg, 0.0_m);
		LLA close(42.36011_deg, -71.05891_deg, 0.0_m);
		LLA closeButNoCigar(42.36012_deg, -71.05892_deg, 0.0_m);

		LLA  angleTolerance(0.00001_deg, 0.00001_deg, 0.0_m);

		// exactly same, default tolerance
		EXPECT_TRUE(lla.isSame(exactlySame));

		// close, single tolerance
		EXPECT_TRUE(lla.isSame(close, 1.5_m));

		// close, point tolerance
		EXPECT_TRUE(lla.isSame(close, angleTolerance));

		// close, not within tolerance
		EXPECT_FALSE(lla.isSame(closeButNoCigar, 1.5_m));

		// close, not within point tolerance
		EXPECT_FALSE(lla.isSame(closeButNoCigar, angleTolerance));
	}

	TEST_F(PositionGeodeticTest, distance)
	{
		LLA NYC(40.7128_deg, -74.0059_deg, 30.0_km);
		LLA LA(34.0522_deg, -118.2437_deg, 30.0_km);

		EXPECT_UNITS_NEAR(3900253.57184229_m, NYC.distance(LA), 5.0e-9_m);
	}

	// The directional two-point measurement family (a.measureTo(b)): each returns its distinctly-tagged
	// kind and agrees with the existing accessor / free function it forwards to.
	TEST_F(PositionGeodeticTest, measurementMembers)
	{
		LLA NYC(40.7128_deg, -74.0059_deg, 30.0_km);
		LLA LA(34.0522_deg, -118.2437_deg, 30.0_km);

		// euclidean and slant range are the same straight-line magnitude, differently tagged.
		static_assert(std::is_same_v<decltype(NYC.euclideanDistanceTo(LA)), ranges::Euclidean>);
		static_assert(std::is_same_v<decltype(NYC.slantRangeTo(LA)), ranges::Euclidean>);
		EXPECT_UNITS_NEAR(NYC.distance(LA), NYC.euclideanDistanceTo(LA), 5.0e-9_m);
		EXPECT_UNITS_NEAR(NYC.euclideanDistanceTo(LA), NYC.slantRangeTo(LA), 5.0e-9_m);

		// geodesicDistanceTo is the uniform-named companion to distanceTo (surface distance).
		static_assert(std::is_same_v<decltype(NYC.geodesicDistanceTo(LA)), ranges::Geodesic>);
		EXPECT_UNITS_NEAR(NYC.distanceTo(LA), NYC.geodesicDistanceTo(LA), 5.0e-9_m);

		// bearingTo is the initial bearing.
		static_assert(std::is_same_v<decltype(NYC.bearingTo(LA)), angles::Azimuth>);
		EXPECT_UNITS_NEAR(NYC.initialBearingTo(LA), NYC.bearingTo(LA), 1.0e-9_deg);
	}

	TEST_F(PositionGeodeticTest, latitude)
	{
		LLA NYC(40.7128_deg, -74.0059_deg, 30.0_km);
		EXPECT_UNITS_EQ(40.7128_deg, NYC.latitude());
	}

	TEST_F(PositionGeodeticTest, longitude)
	{
		LLA NYC(40.7128_deg, -74.0059_deg, 30.0_km);
		EXPECT_UNITS_EQ(-74.0059_deg, NYC.longitude());
	}

	TEST_F(PositionGeodeticTest, altitude)
	{
		LLA NYC(40.7128_deg, -74.0059_deg, 30.0_km);
		EXPECT_UNITS_EQ(30000.0_m, NYC.altitude());
	}

	TEST_F(PositionGeodeticTest, date)
	{
		LLA NYC(40.7128_deg, -74.0059_deg, 30.0_km, 2016.0_yr);
		EXPECT_EQ(2016.0_yr, NYC.date());
	}

	TEST_F(PositionGeodeticTest, setLatitude)
	{
		LLA NYC;
		NYC.setLatitude(40.7128_deg);

		EXPECT_UNITS_EQ(40.7128_deg, NYC.latitude());
		EXPECT_UNITS_EQ(0.0_deg, NYC.longitude());
		EXPECT_UNITS_EQ(0.0_m, NYC.altitude());
	}

	TEST_F(PositionGeodeticTest, setLongitude)
	{
		LLA NYC;
		NYC.setLongitude(-74.0059_deg);

		EXPECT_UNITS_EQ(0.0_deg, NYC.latitude());
		EXPECT_UNITS_EQ(-74.0059_deg, NYC.longitude());
		EXPECT_UNITS_EQ(0.0_m, NYC.altitude());
	}

	TEST_F(PositionGeodeticTest, setAltitude)
	{
		LLA NYC;
		NYC.setAltitude(30.0_km);

		EXPECT_UNITS_EQ(0.0_deg, NYC.latitude());
		EXPECT_UNITS_EQ(0.0_deg, NYC.longitude());
		EXPECT_UNITS_EQ(30.0_km, NYC.altitude());
	}

	TEST_F(PositionGeodeticTest, ostream)
	{
		LLA lla(42.3601_deg, -71.0589_deg, 0.0_m);

		testing::internal::CaptureStdout();
		std::cout << lla;
		std::string output = testing::internal::GetCapturedStdout();

		EXPECT_STREQ("(42.3601 deg, -71.0589 deg, 0 m)", output.c_str());
	}

	TEST_F(PositionGeodeticTest, operatorEqual)
	{
		LLA lla(42.3601_deg, -71.0589_deg, 0.0_m);
		LLA lla2(42.3601_deg, -71.0589_deg, 0.0_m);
		LLA lla3(42.365_deg, -71.5_deg, 1.0_m);

		EXPECT_TRUE(lla == lla2);
		EXPECT_FALSE(lla == lla3);
	}

	TEST_F(PositionGeodeticTest, operatorNotEqual)
	{
		LLA lla(42.3601_deg, -71.0589_deg, 0.0_m);
		LLA lla2(42.3601_deg, -71.0589_deg, 0.0_m);
		LLA lla3(42.365_deg, -71.5_deg, 1.0_m);

		EXPECT_FALSE(lla != lla2);
		EXPECT_TRUE(lla != lla3);
	}
}
#endif // LLATest_h__