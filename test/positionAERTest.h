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

#ifndef positionAERTest_h__
#define positionAERTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <iostream>

#include "gtest_units.h"

using namespace coordinates;
using namespace units;
using namespace units::length;
using namespace units::area;
using namespace units::angle;
using namespace units::time;

namespace
{
	// The fixture for testing class Foo.
	class PositionAERTest : public ::testing::Test {
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		PositionAERTest()
		{
			// You can do set-range work for each test here.
		}

		virtual ~PositionAERTest()
		{
			// You can do clean-range work that doesn't throw exceptions here.
		}

		// If the constructor and destructor are not enough for setting range
		// and cleaning range each test, you can define the following methods:

		void SetUp() override
		{
			// Code here will be called immediately after the constructor (right
			// before each test).
			Boston.setPoint(42_deg, -71_deg, 0_m);
			Boston_ecef = Boston;

			LA.setPoint(34_deg, -118_deg, 0_m);
			LA_ecef = LA;
		}

		void TearDown() override
		{
			// Code here will be called immediately after each test (right
			// before the destructor).
		}

		// Objects declared here can be used by all tests in the test case for Foo.
		LLA Boston;
		ECEF Boston_ecef;

		LLA  LA;
		ECEF LA_ecef;
	};


TEST_F(PositionAERTest, prerequisites)
{
	// Make sure the ECEF/LLA conversions look OK
	// Truth data: MATLAB Mapping Toolbox (WGS84), using wgs84Ellipsoid + geodetic2ecef.
	EXPECT_UNITS_NEAR(-2485034.262789235_m, LA_ecef.x(), 1.0e-3_m);
	EXPECT_UNITS_NEAR(-4673669.705320125_m, LA_ecef.y(), 1.0e-3_m);
	EXPECT_UNITS_NEAR(3546446.563780690_m,  LA_ecef.z(), 1.0e-3_m);

	EXPECT_UNITS_NEAR(1545471.693315307_m,  Boston_ecef.x(), 1.0e-3_m);
	EXPECT_UNITS_NEAR(-4488375.702866388_m, Boston_ecef.y(), 1.0e-3_m);
	EXPECT_UNITS_NEAR(4245603.836101115_m,  Boston_ecef.z(), 1.0e-3_m);
}

	TEST_F(PositionAERTest, is_point)
	{
		EXPECT_TRUE(is_point<AER>);
	}

	TEST_F(PositionAERTest, defaultConstructor)
	{
		EXPECT_TRUE(std::is_default_constructible_v<AER>);
	}

	TEST_F(PositionAERTest, constructor)
	{
		// no date
		AER aer(5_deg, 10_deg, 15000_m, Boston);
		EXPECT_EQ(5_deg, std::get<0>(aer.point()));
		EXPECT_EQ(10_deg, std::get<1>(aer.point()));
		EXPECT_EQ(15000_m, std::get<2>(aer.point()));
		EXPECT_EQ(Boston.point(), aer.frameData().origin);
		EXPECT_EQ(2005_yr, aer.frameData().date);

		// date
		AER aer2(5_deg, 10_deg, 15000_m, Boston, 2016_yr);
		EXPECT_EQ(5_deg, std::get<0>(aer2.point()));
		EXPECT_EQ(10_deg, std::get<1>(aer2.point()));
		EXPECT_EQ(15000_m, std::get<2>(aer2.point()));
		EXPECT_EQ(Boston.point(), aer2.frameData().origin);
		EXPECT_EQ(2016_yr, aer2.frameData().date);

		// different units
		AER aer3(5_deg, 10_deg, 15_km, Boston);
		EXPECT_EQ(5_deg, std::get<0>(aer3.point()));
		EXPECT_EQ(10_deg, std::get<1>(aer3.point()));
		EXPECT_EQ(15000_m, std::get<2>(aer3.point()));
		EXPECT_EQ(Boston.point(), aer3.frameData().origin);

		// origin with different frame of reference
		AER aer4(5_deg, 10_deg, 15_km, Boston_ecef);
		EXPECT_EQ(5_deg, std::get<0>(aer4.point()));
		EXPECT_EQ(10_deg, std::get<1>(aer4.point()));
		EXPECT_EQ(15000_m, std::get<2>(aer4.point()));
		EXPECT_EQ(Boston.point(), aer4.frameData().origin);
	}

	TEST_F(PositionAERTest, scalarOriginConstructor)
	{
		// Origin supplied as raw latitude/longitude/altitude scalars (not a Position object).
		AER aer(5_deg, 10_deg, 15000_m, 42_deg, -71_deg, 0_m);
		EXPECT_EQ(5_deg, std::get<0>(aer.point()));
		EXPECT_EQ(10_deg, std::get<1>(aer.point()));
		EXPECT_EQ(15000_m, std::get<2>(aer.point()));
		EXPECT_EQ(Boston.point(), aer.frameData().origin);

		// with an explicit date of observation
		AER aer2(5_deg, 10_deg, 15000_m, 42_deg, -71_deg, 0_m, 2016_yr);
		EXPECT_EQ(Boston.point(), aer2.frameData().origin);
		EXPECT_EQ(2016_yr, aer2.frameData().date);
	}

	TEST_F(PositionAERTest, tupleConstructor)
	{
		SphericalTuple tup(5_deg, 10_deg, 15_km);

		// no date
		AER aer(tup, Boston);
		EXPECT_EQ(5_deg, std::get<0>(aer.point()));
		EXPECT_EQ(10_deg, std::get<1>(aer.point()));
		EXPECT_EQ(15000_m, std::get<2>(aer.point()));
		EXPECT_EQ(Boston.point(), aer.frameData().origin);
		EXPECT_EQ(2005_yr, aer.frameData().date);

		// date
		AER aer2(tup, Boston, 2016_yr);
		EXPECT_EQ(5_deg, std::get<0>(aer2.point()));
		EXPECT_EQ(10_deg, std::get<1>(aer2.point()));
		EXPECT_EQ(15000_m, std::get<2>(aer2.point()));
		EXPECT_EQ(Boston.point(), aer2.frameData().origin);
		EXPECT_EQ(2016_yr, aer2.frameData().date);

		// different units
		AER aer3(tup, Boston);
		EXPECT_EQ(5_deg, std::get<0>(aer3.point()));
		EXPECT_EQ(10_deg, std::get<1>(aer3.point()));
		EXPECT_EQ(15000_m, std::get<2>(aer3.point()));
		EXPECT_EQ(Boston.point(), aer3.frameData().origin);

		// origin with different frame of reference
		AER aer4(tup, Boston_ecef);
		EXPECT_EQ(5_deg, std::get<0>(aer4.point()));
		EXPECT_EQ(10_deg, std::get<1>(aer4.point()));
		EXPECT_EQ(15000_m, std::get<2>(aer4.point()));
		EXPECT_EQ(Boston.point(), aer4.frameData().origin);
	}

	TEST_F(PositionAERTest, originOnlyConstructor)
	{
		// no date
		AER aer(Boston);
		EXPECT_EQ(0_deg, std::get<0>(aer.point()));
		EXPECT_EQ(0_deg, std::get<1>(aer.point()));
		EXPECT_EQ(0_m, std::get<2>(aer.point()));
		EXPECT_EQ(Boston.point(), aer.frameData().origin);
		EXPECT_EQ(2005_yr, aer.frameData().date);

		// date
		AER aer2(Boston, 2016_yr);
		EXPECT_EQ(0_deg, std::get<0>(aer2.point()));
		EXPECT_EQ(0_deg, std::get<1>(aer2.point()));
		EXPECT_EQ(0_m, std::get<2>(aer2.point()));
		EXPECT_EQ(Boston.point(), aer2.frameData().origin);
		EXPECT_EQ(2016_yr, aer2.frameData().date);
	}

	TEST_F(PositionAERTest, copyConstructor)
	{
		AER aer(1_deg, 2_deg, 3_m, LA);
		EXPECT_EQ(1_deg, std::get<0>(aer.point()));
		EXPECT_EQ(2_deg, std::get<1>(aer.point()));
		EXPECT_EQ(3_m, std::get<2>(aer.point()));

		AER aer2(aer);
		EXPECT_TRUE(is_point<decltype(aer)>);
		EXPECT_TRUE(is_point<decltype(aer2)>);
		EXPECT_TRUE(aer == aer2);
	}


TEST_F(PositionAERTest, implicitConversionConstructor_fromECEF)
{
	AER aer(LA_ecef, LA);
	EXPECT_UNITS_EQ(0_deg, std::get<0>(aer.point()));
	EXPECT_UNITS_EQ(0_deg, std::get<1>(aer.point()));
	EXPECT_UNITS_EQ(0_m,   std::get<2>(aer.point()));
	EXPECT_TRUE(LA.isSame(LA_ecef));
	EXPECT_TRUE(LA_ecef.isSame(LA));
	EXPECT_UNITS_EQ(2005_yr, aer.frameData().date);

	AER aer2(Boston_ecef, Boston);
	EXPECT_UNITS_EQ(0_deg, std::get<0>(aer2.point()));
	EXPECT_UNITS_EQ(0_deg, std::get<1>(aer2.point()));
	EXPECT_UNITS_EQ(0_m,   std::get<2>(aer2.point()));
	EXPECT_TRUE(Boston.isSame(Boston_ecef));
	EXPECT_TRUE(Boston_ecef.isSame(Boston));
	EXPECT_UNITS_EQ(2005_yr, aer2.frameData().date);

	// Boston observing LA
	AER aer3(LA_ecef, Boston);
	EXPECT_UNITS_NEAR(273.5656795360105_deg,   std::get<0>(aer3.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.698699777761998_deg, std::get<1>(aer3.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4094891.087804174_m,     std::get<2>(aer3.point()), 1.0e-3_m);
	EXPECT_UNITS_EQ(2005_yr, aer3.frameData().date);
}


TEST_F(PositionAERTest, implicitConversionConstructor_fromGeo)
{
	AER aer(Boston, Boston);
	EXPECT_UNITS_EQ(0_deg, std::get<0>(aer.point()));
	EXPECT_UNITS_EQ(0_deg, std::get<1>(aer.point()));
	EXPECT_UNITS_EQ(0_m,   std::get<2>(aer.point()));
	EXPECT_UNITS_EQ(2005_yr, aer.frameData().date);

	// LA observing Boston
	AER aer2(Boston, LA);
	EXPECT_UNITS_NEAR(63.522735932827878_deg,   std::get<0>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.707489622725706_deg,  std::get<1>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4094891.087804174_m,      std::get<2>(aer2.point()), 1.0e-3_m);
	EXPECT_UNITS_EQ(2005_yr, aer2.frameData().date);

	// Boston observing LA
	AER aer3(LA, Boston);
	EXPECT_UNITS_NEAR(273.5656795360105_deg,    std::get<0>(aer3.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.698699777761998_deg,  std::get<1>(aer3.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4094891.087804174_m,      std::get<2>(aer3.point()), 1.0e-3_m);
	EXPECT_UNITS_EQ(2005_yr, aer3.frameData().date);
}


TEST_F(PositionAERTest, implicitConversionConstructor_fromENU)
{
	// same origin
	ENU enu(1000.0_m, 1000.0_m, 1000.0_m, LA);
	NED ned(enu);

	// Sanity: ENU->NED at same origin is a pure axis swap + sign on "up/down"
	EXPECT_UNITS_EQ(1000.0_m, std::get<0>(ned.point())); // N
	EXPECT_UNITS_EQ(1000.0_m, std::get<1>(ned.point())); // E
	EXPECT_UNITS_EQ(-1000.0_m, std::get<2>(ned.point())); // D
	EXPECT_EQ(LA.point(), ned.frameData().origin);

	// should just result in a copy (computed directly from ENU when origins match)
	AER aer(enu, LA, 2016.0_yr);
	EXPECT_UNITS_NEAR(45.0_deg,                 std::get<0>(aer.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(35.264389682754654_deg,   std::get<1>(aer.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(1732.0508075688772_m,     std::get<2>(aer.point()), 5.0e-9_m);
	EXPECT_EQ(LA.point(), aer.frameData().origin);
	EXPECT_UNITS_EQ(2016.0_yr, aer.frameData().date);

	// different origin (truth data: MATLAB enu2ecef + ecef2aer, WGS84)
	AER aer2(enu, Boston);
	EXPECT_UNITS_NEAR(273.5723050226951_deg,    std::get<0>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.6794208964861_deg,    std::get<1>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4093941.971723601_m,      std::get<2>(aer2.point()), 1.0e-8_m);
	EXPECT_EQ(Boston.point(), aer2.frameData().origin);
	EXPECT_UNITS_EQ(2005_yr, aer2.frameData().date);
}




TEST_F(PositionAERTest, azimuthNormalization_fromENU_fastPath)
{
	// When converting directly from ENU with matching origins, azimuth is normalized to [0, 360).
	ENU enu(-1.0_m, 1.0_m, 0.0_m, LA); // E=-1, N=+1 => atan2(-1,1) = -45 deg -> 315 deg
	AER aer(enu, LA);
	EXPECT_UNITS_NEAR(315.0_deg, std::get<0>(aer.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(0.0_deg,   std::get<1>(aer.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(sqrt(2.0) * 1.0_m, std::get<2>(aer.point()), 5.0e-9_m);
	EXPECT_EQ(LA.point(), aer.frameData().origin);
}
TEST_F(PositionAERTest, implicitConversionConstructor_fromNED)
{
	// same origin
	NED ned(100_m, 100_m, 100_m, LA);

	// should just result in a copy (computed directly from NED when origins match)
	AER aer(ned, LA, 2016_yr);
	EXPECT_UNITS_NEAR(45.0_deg,                  std::get<0>(aer.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(-35.264389682754654_deg,   std::get<1>(aer.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(173.20508075688772_m,      std::get<2>(aer.point()), 5.0e-9_m);
	EXPECT_EQ(LA.point(), aer.frameData().origin);
	EXPECT_UNITS_EQ(2016_yr, aer.frameData().date);

	// different origin (truth data: MATLAB geodetic2ecef + ecef2aer, WGS84)
	AER aer2(ned, Boston);
	EXPECT_UNITS_NEAR(273.5663436522812_deg,     std::get<0>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.699422933503001_deg,   std::get<1>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4094732.003776215_m,       std::get<2>(aer2.point()), 1.0e-3_m);
	EXPECT_EQ(Boston.point(), aer2.frameData().origin);
	EXPECT_UNITS_EQ(2005_yr, aer2.frameData().date);
}


TEST_F(PositionAERTest, implicitConversionConstructor_fromAER)
{
	// same origin
	AER aer(45_deg, 35_deg, 2_m, LA);

	// should just result in a copy
	AER aer2(aer);
	EXPECT_UNITS_NEAR(45.0_deg, std::get<0>(aer2.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(35.0_deg, std::get<1>(aer2.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(2.0_m,    std::get<2>(aer2.point()), 5.0e-9_m);
	EXPECT_EQ(LA.point(), aer2.frameData().origin);

	// same origin
	AER aer3(aer, LA);
	EXPECT_UNITS_NEAR(45.0_deg, std::get<0>(aer3.point()), 5.0e-8_deg);
	EXPECT_UNITS_NEAR(35.0_deg, std::get<1>(aer3.point()), 5.0e-8_deg);
	EXPECT_UNITS_NEAR(2.0_m,    std::get<2>(aer3.point()), 5.0e-9_m);
	EXPECT_EQ(LA.point(), aer3.frameData().origin);

	// different origin (truth data: MATLAB ecef2aer, WGS84)
	AER aer4(aer, Boston);
	EXPECT_UNITS_NEAR(273.5656872105317_deg,     std::get<0>(aer4.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.698677599130299_deg,   std::get<1>(aer4.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4094889.984373124_m,       std::get<2>(aer4.point()), 1.0e-3_m);
	EXPECT_EQ(Boston.point(), aer4.frameData().origin);
}

	TEST_F(PositionAERTest, copyAssignment)
	{
		AER aer(1_deg, 2_deg, 3_m, LA);
		EXPECT_EQ(1_deg, std::get<0>(aer.point()));
		EXPECT_EQ(2_deg, std::get<1>(aer.point()));
		EXPECT_EQ(3_m, std::get<2>(aer.point()));

		AER aer2 = aer;
		EXPECT_TRUE(is_point<decltype(aer)>);
		EXPECT_TRUE(is_point<decltype(aer2)>);
		EXPECT_TRUE(aer == aer2);
	}


TEST_F(PositionAERTest, implicitConversionAssignment_fromECEF)
{
	AER aer(LA_ecef, LA);
	EXPECT_UNITS_EQ(0_deg, std::get<0>(aer.point()));
	EXPECT_UNITS_EQ(0_deg, std::get<1>(aer.point()));
	EXPECT_UNITS_EQ(0_m,   std::get<2>(aer.point()));

	AER aer2(Boston);
	aer2 = aer;

	EXPECT_UNITS_NEAR(273.5656795360105_deg,    std::get<0>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.698699777761998_deg,  std::get<1>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4094891.087804174_m,      std::get<2>(aer2.point()), 1.0e-3_m);
	EXPECT_EQ(Boston.point(), aer2.frameData().origin);
	EXPECT_UNITS_EQ(2005_yr, aer2.frameData().date);
}


TEST_F(PositionAERTest, implicitConversionAssignment_fromGeo)
{
	AER aer(Boston, Boston);
	EXPECT_UNITS_EQ(0_deg, std::get<0>(aer.point()));
	EXPECT_UNITS_EQ(0_deg, std::get<1>(aer.point()));
	EXPECT_UNITS_EQ(0_m,   std::get<2>(aer.point()));

	AER aer2(LA);
	aer2 = Boston;
	EXPECT_UNITS_NEAR(63.522735932827878_deg,   std::get<0>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.707489622725706_deg,  std::get<1>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4094891.087804174_m,      std::get<2>(aer2.point()), 1.0e-3_m);
	EXPECT_EQ(LA.point(), aer2.frameData().origin);
	EXPECT_UNITS_EQ(2005_yr, aer2.frameData().date);

	AER aer3(Boston);
	aer3 = LA;
	EXPECT_UNITS_NEAR(273.5656795360105_deg,    std::get<0>(aer3.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.698699777761998_deg,  std::get<1>(aer3.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4094891.087804174_m,      std::get<2>(aer3.point()), 1.0e-3_m);
	EXPECT_EQ(Boston.point(), aer3.frameData().origin);
	EXPECT_UNITS_EQ(2005_yr, aer3.frameData().date);
}


TEST_F(PositionAERTest, implicitConversionAssignment_fromENU)
{
	// same origin
	ENU enu(1.0_m, 1.0_m, 1.0_m, LA);

	AER aer(LA, 2016.0_yr);
	aer = enu;

	EXPECT_UNITS_NEAR(45.0_deg,                std::get<0>(aer.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(35.264389682754654_deg,  std::get<1>(aer.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(1.7320508075688772_m,    std::get<2>(aer.point()), 5.0e-9_m);
	EXPECT_EQ(LA.point(), aer.frameData().origin);

	// NOTE: current semantics: assignment overwrites date with the source point's date when origin is set.
	EXPECT_UNITS_EQ(2005_yr, aer.frameData().date);

	// different origin (truth data: MATLAB enu2ecef + ecef2aer, WGS84)
	AER aer2(Boston);
	aer2 = enu;

	EXPECT_UNITS_NEAR(273.5656861607163_deg,   std::get<0>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.698680503461308_deg, std::get<1>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4094890.138432032_m,     std::get<2>(aer2.point()), 1.0e-3_m);
	EXPECT_EQ(Boston.point(), aer2.frameData().origin);
	EXPECT_UNITS_EQ(2005_yr, aer2.frameData().date);
}


TEST_F(PositionAERTest, implicitConversionAssignment_fromNED)
{
	// same origin
	NED ned(100_m, 100_m, 100_m, LA);

	AER aer(LA, 2016_yr);
	aer = ned;

	EXPECT_UNITS_NEAR(45.0_deg,                 std::get<0>(aer.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(-35.264389682754654_deg,  std::get<1>(aer.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(173.20508075688772_m,     std::get<2>(aer.point()), 5.0e-9_m);
	EXPECT_EQ(LA.point(), aer.frameData().origin);
	EXPECT_UNITS_EQ(2005_yr, aer.frameData().date);

	// different origin
	AER aer2(Boston);
	aer2 = ned;

	EXPECT_UNITS_NEAR(273.5663436522812_deg,    std::get<0>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.699422933503001_deg,  std::get<1>(aer2.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4094732.003776215_m,      std::get<2>(aer2.point()), 1.0e-3_m);
	EXPECT_EQ(Boston.point(), aer2.frameData().origin);
	EXPECT_UNITS_EQ(2005_yr, aer2.frameData().date);
}


TEST_F(PositionAERTest, implicitConversionAssignment_fromAER)
{
	// same origin
	AER aer(45_deg, 35_deg, 2_m, LA);

	// should just result in a copy
	AER aer2(LA);
	aer2 = aer;

	EXPECT_UNITS_NEAR(45_deg, std::get<0>(aer2.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(35_deg, std::get<1>(aer2.point()), 5.0e-9_deg);
	EXPECT_UNITS_NEAR(2_m,    std::get<2>(aer2.point()), 5.0e-9_m);
	EXPECT_EQ(LA.point(), aer2.frameData().origin);

	// different origin
	AER aer3(Boston);
	aer3 = aer;

	EXPECT_UNITS_NEAR(273.5656872105317_deg,    std::get<0>(aer3.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.698677599130299_deg,  std::get<1>(aer3.point()), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4094889.984373124_m,      std::get<2>(aer3.point()), 1.0e-3_m);
	EXPECT_EQ(Boston.point(), aer3.frameData().origin);
	EXPECT_UNITS_EQ(2005_yr, aer3.frameData().date);
}

	TEST_F(PositionAERTest, isSame)
	{
		using ECEF_mm = PositionECEF<datums::WGS84_G1674, units::length::millimeters>;

		AER aer1(5_deg, 10_deg, 15_km, Boston);

 		AER aer2(1_deg, 2_deg, 3_km, LA);

		AER exactlySame1(5_deg, 10_deg, 15_km, Boston);
		AER exactlySame2(1_deg, 2_deg, 3_km, LA);

		AER exactlySameDiffBoston(aer1, LA);	// These are not *actually* 100% the same, since c++ trig functions aren't fully reversible.
		AER exactlySameDiffLA(aer2, Boston);

		AER close1(5_deg, 10_deg, 15000.01_m, Boston);
		AER close2(1_deg, 2_deg, 3000.01_m, LA);

		ECEF_mm millimeterTolerance(1_mm, 1_mm, 1_mm);

		EXPECT_EQ(5_deg, std::get<0>(aer1.point()));
		EXPECT_EQ(10_deg, std::get<1>(aer1.point()));
		EXPECT_EQ(15000_m, std::get<2>(aer1.point()));

		// exactly same, default (0) tolerance
		EXPECT_TRUE(aer1.isSame(exactlySame1));
		EXPECT_FALSE(aer1.isSame(close1));
		EXPECT_FALSE(aer1.isSame(exactlySame2));
		EXPECT_FALSE(aer1.isSame(exactlySameDiffLA));

		EXPECT_TRUE(aer2.isSame(exactlySame2));
		EXPECT_FALSE(aer2.isSame(close2));
		EXPECT_FALSE(aer2.isSame(exactlySame1));
		EXPECT_FALSE(aer2.isSame(exactlySameDiffBoston));

		// close, millimeter tolerance
 		EXPECT_TRUE((aer1.isSame(exactlySame1, 1_mm)));
		EXPECT_TRUE((aer1.isSame(exactlySameDiffBoston, 1_mm)));
		EXPECT_FALSE((aer1.isSame(close1, 1_mm)));
		EXPECT_FALSE((aer1.isSame(exactlySame2, 1_mm)));
		EXPECT_FALSE((aer1.isSame(exactlySameDiffLA, 1_mm)));

		EXPECT_TRUE((aer2.isSame(exactlySame2, 1_mm)));
		EXPECT_TRUE((aer2.isSame(exactlySameDiffLA, 1_mm)));
		EXPECT_FALSE((aer2.isSame(close2, 1_mm)));
 		EXPECT_FALSE((aer2.isSame(exactlySame1, 1_mm)));
		EXPECT_FALSE((aer2.isSame(exactlySameDiffBoston, 1_mm)));

 		// close, millimeter(s) tolerance - point
 		EXPECT_TRUE((aer1.isSame(exactlySame1, millimeterTolerance)));
		EXPECT_TRUE((aer1.isSame(exactlySameDiffBoston, millimeterTolerance)));
		EXPECT_FALSE((aer1.isSame(close1, millimeterTolerance)));
		EXPECT_FALSE((aer1.isSame(exactlySame2, millimeterTolerance)));
		EXPECT_FALSE((aer1.isSame(exactlySameDiffLA, millimeterTolerance)));

		EXPECT_TRUE((aer2.isSame(exactlySame2, millimeterTolerance)));
		EXPECT_TRUE((aer2.isSame(exactlySameDiffLA, millimeterTolerance)));
		EXPECT_FALSE((aer2.isSame(close2, millimeterTolerance)));
 		EXPECT_FALSE((aer2.isSame(exactlySame1, millimeterTolerance)));
		EXPECT_FALSE((aer2.isSame(exactlySameDiffBoston, millimeterTolerance)));

		// close, centimeter tolerance
 		EXPECT_TRUE((aer1.isSame(exactlySame1, 1_cm)));
		EXPECT_TRUE((aer1.isSame(exactlySameDiffBoston, 1_cm)));
		EXPECT_TRUE((aer1.isSame(close1, 1_cm)));
		EXPECT_FALSE((aer1.isSame(exactlySame2, 1_cm)));
		EXPECT_FALSE((aer1.isSame(exactlySameDiffLA, 1_cm)));

		EXPECT_TRUE((aer2.isSame(exactlySame2, 1_cm)));
		EXPECT_TRUE((aer2.isSame(exactlySameDiffLA, 1_cm)));
		EXPECT_TRUE((aer2.isSame(close2, 1_cm)));
		EXPECT_FALSE((aer2.isSame(exactlySame1, 1_cm)));
		EXPECT_FALSE((aer2.isSame(exactlySameDiffBoston, 1_cm)));

 		// close, 4100 kilometer tolerance
 		EXPECT_TRUE((aer1.isSame(exactlySame1, 4100_km)));
		EXPECT_TRUE((aer1.isSame(exactlySameDiffBoston, 4100_km)));
		EXPECT_TRUE((aer1.isSame(close1, 4100_km)));
		EXPECT_TRUE((aer1.isSame(exactlySame2, 4100_km)));
		EXPECT_TRUE((aer1.isSame(exactlySameDiffLA, 4100_km)));

		EXPECT_TRUE((aer2.isSame(exactlySame2, 4100_km)));
		EXPECT_TRUE((aer2.isSame(exactlySameDiffLA, 4100_km)));
		EXPECT_TRUE((aer2.isSame(close2, 4100_km)));
 		EXPECT_TRUE((aer2.isSame(exactlySame1, 4100_km)));
		EXPECT_TRUE((aer2.isSame(exactlySameDiffBoston, 4100_km)));
 	}


TEST_F(PositionAERTest, distance)
{
	AER aerb0(1_deg, 1_deg, 1_m, Boston);
	AER aerb1(2_deg, 2_deg, 2_m, Boston);

	AER aerl0(1_deg, 1_deg, 1_m, LA);
	AER aerl1(2_deg, 2_deg, 2_m, LA);

	// same origin (straight-line, not great-circle)
	EXPECT_UNITS_NEAR(1.000608802301425_m, aerb0.distance(aerb1), 5.0e-9_m);
	EXPECT_UNITS_NEAR(1.000608802301425_m, aerb1.distance(aerb0), 5.0e-9_m);
	EXPECT_UNITS_NEAR(1.000608802301425_m, aerl0.distance(aerl1), 5.0e-9_m);
	EXPECT_UNITS_NEAR(1.000608802301425_m, aerl1.distance(aerl0), 5.0e-9_m);

	// different point types
	EXPECT_UNITS_NEAR(2_m, aerb1.distance(Boston), 5.0e-9_m);
	EXPECT_UNITS_NEAR(2_m, aerl1.distance(LA),     5.0e-9_m);

	// different origin (truth data: MATLAB norm(ecef(target)-ecef(observer)))
	EXPECT_UNITS_NEAR(4094890.619647854_m, aerb0.distance(aerl0), 1.0e-3_m);
	EXPECT_UNITS_NEAR(4094891.051004039_m, aerb0.distance(LA),    1.0e-3_m);
}

	TEST_F(PositionAERTest, azimuth)
	{
		AER aer;
		EXPECT_UNITS_EQ(0_deg, aer.azimuth());

		AER aer1(5_deg, 6_deg, 7_km, LLA());

		EXPECT_UNITS_EQ(5_deg, aer1.azimuth());

		AER aer2(radians(2.0), radians(3.0), 4_km, LLA());

		EXPECT_UNITS_EQ(radians(2.0), aer2.azimuth());
	}

	TEST_F(PositionAERTest, elevation)
	{
		AER aer;
		EXPECT_UNITS_EQ(0_deg, aer.elevation());

		AER aer1(5_deg, 6_deg, 7_km, LLA());

		EXPECT_UNITS_EQ(6_deg, aer1.elevation());

		AER aer2(radians(2.0), radians(3.0), 4_km, LLA());

 		EXPECT_UNITS_EQ(radians(3.0), aer2.elevation());
	}

	TEST_F(PositionAERTest, range)
	{
		AER aer;
		EXPECT_UNITS_EQ(0_m, aer.range());

		AER aer1(5_deg, 6_deg, 7_km, LLA());

		EXPECT_UNITS_EQ(7_km, aer1.range());

		AER aer2(radians(2.0), radians(3.0), 4_km, LLA());

		EXPECT_UNITS_EQ(4_km, aer2.range());
	}

	TEST_F(PositionAERTest, origin)
	{
		AER aer;
		EXPECT_TRUE(LLA(0_deg, 0_deg, 0_m) == aer.origin());

		AER aer2(5_deg, 10_deg, 15_km, Boston);

 		EXPECT_TRUE(Boston == aer2.origin());
	}

	TEST_F(PositionAERTest, date)
	{
		AER aer;
		EXPECT_EQ(2005_yr, aer.date());

		AER aer2(5_deg, 10_deg, 15_km, Boston, 2016.421_yr);

 		EXPECT_EQ(2016.421_yr, aer2.date());
	}


TEST_F(PositionAERTest, setAzimuth)
{
	AER aer;
	EXPECT_UNITS_EQ(0_deg, aer.azimuth());

	aer.setAzimuth(5_deg);
	EXPECT_UNITS_EQ(5_deg, aer.azimuth());
	EXPECT_UNITS_EQ(0_deg, aer.elevation());
	EXPECT_UNITS_EQ(0_m,   aer.range());

	// Unit conversion on assignment (radians -> degrees)
	aer.setAzimuth(radians(5.0));
	EXPECT_UNITS_NEAR(286.4788975654116_deg, aer.azimuth(), 5.0e-9_deg);
	EXPECT_UNITS_EQ(0_deg, aer.elevation());
	EXPECT_UNITS_EQ(0_m,   aer.range());
}


TEST_F(PositionAERTest, setElevation)
{
	AER aer;
	EXPECT_UNITS_EQ(0_deg, aer.elevation());

	aer.setElevation(6_deg);
	EXPECT_UNITS_EQ(0_deg, aer.azimuth());
	EXPECT_UNITS_EQ(6_deg, aer.elevation());
	EXPECT_UNITS_EQ(0_m,   aer.range());

	// Unit conversion on assignment (radians -> degrees)
	aer.setElevation(radians(6.0));
	EXPECT_UNITS_EQ(0_deg, aer.azimuth());
	EXPECT_UNITS_NEAR(343.7746770784939_deg, aer.elevation(), 5.0e-9_deg);
	EXPECT_UNITS_EQ(0_m,   aer.range());
}

	TEST_F(PositionAERTest, setRange)
	{
		AER aer;
		EXPECT_UNITS_EQ(0_m, aer.range());

		aer.setRange(7_m);
		EXPECT_UNITS_EQ(0_deg, aer.azimuth());
		EXPECT_UNITS_EQ(0_deg, aer.elevation());
		EXPECT_UNITS_EQ(7_m, aer.range());

		aer.setRange(7_mm);
		EXPECT_UNITS_EQ(0_deg, aer.azimuth());
		EXPECT_UNITS_EQ(0_deg, aer.elevation());
		EXPECT_UNITS_EQ(0.007_m, aer.range());
	}


TEST_F(PositionAERTest, setOrigin)
{
	AER aer;
	EXPECT_TRUE(LLA(0_deg, 0_deg, 0_m) == aer.origin());

	AER aer2(5_deg, 10_deg, 15_km, Boston);
	EXPECT_TRUE(Boston == aer2.origin());

	AER aer3(aer2);
	aer3.setOrigin(LA);
	EXPECT_UNITS_NEAR(63.304670032255295_deg,  aer3.azimuth(),   1.0e-8_deg);
	EXPECT_UNITS_NEAR(-18.674629637629703_deg, aer3.elevation(), 1.0e-8_deg);
	EXPECT_UNITS_NEAR(4096103.677420084_m,     aer3.range(),     1.0e-3_m);
	EXPECT_TRUE(LA == aer3.origin());
	EXPECT_TRUE(aer2.isSame(aer3, 100_nm));
}

	TEST_F(PositionAERTest, ostream)
	{
		AER aer1(5_deg, 10_deg, 15_km, Boston);

		testing::internal::CaptureStdout();
		std::cout << aer1;
		std::string output = testing::internal::GetCapturedStdout();

		EXPECT_STREQ("(5 deg, 10 deg, 15000 m) @ (42 deg, -71 deg, 0 m)", output.c_str());
	}
}


#endif // positionAERTest_h__