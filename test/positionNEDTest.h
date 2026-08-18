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

#ifndef positionNEDTest_h__
#define positionNEDTest_h__

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
using namespace units::area;
using namespace units::angle;
using namespace units::time;

namespace
{
	// The fixture for testing class Foo.
	class PositionNEDTest : public ::testing::Test {
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		PositionNEDTest()
		{
			// You can do set-up work for each test here.
		}

		~PositionNEDTest() override
		{
			// You can do clean-up work that doesn't throw exceptions here.
		}

		// If the constructor and destructor are not enough for setting up
		// and cleaning up each test, you can define the following methods:

		void SetUp() override
		{
			// Code here will be called immediately after the constructor (right
			// before each test).
			Boston.setPoint(42.3601_deg, -71.0589_deg, 0.0_m);
			Boston_ecef = Boston;

			Lexington.setPoint(42.4430_deg, -71.2290_deg, 0.0_m);

			LA.setPoint(34_deg, -118_deg, 0_m);
			LA_ecef = LA;

			Bedford.setPoint(42.4625772_deg, -71.2696797_deg, 15.0_m);
			Bedford_ecef.setPoint(1513242.623813242_m, -4462904.554429035_m, 4283659.060698663_m);

		}

		void TearDown() override
		{
			// Code here will be called immediately after each test (right
			// before the destructor).
		}

		// Objects declared here can be used by all tests in the test case for Foo.
		LLA Boston;
		ECEF Boston_ecef;

		LLA Lexington;

		LLA LA;
		ECEF LA_ecef;

		LLA Bedford;
		ECEF Bedford_ecef;
	};

	TEST_F(PositionNEDTest, is_point)
	{
		EXPECT_TRUE(is_point<NED>);
	}

	TEST_F(PositionNEDTest, defaultConstructor)
	{
		EXPECT_TRUE(std::is_default_constructible_v<NED>);
	}

	TEST_F(PositionNEDTest, constructor)
	{
		// no date
		NED ned(1.0_m, 2.0_m, 3.0_m, Boston);
		EXPECT_EQ(1.0_m, std::get<0>(ned.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ned.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ned.point()));
		EXPECT_EQ(Boston.point(), ned.frameData().origin);
		EXPECT_EQ(2005.0_yr, ned.frameData().date);

		// date
		NED ned2(4.0_m, 5.0_m, 6.0_m, Boston, 2016.0_yr);
		EXPECT_EQ(4.0_m, std::get<0>(ned2.point()));
		EXPECT_EQ(5.0_m, std::get<1>(ned2.point()));
		EXPECT_EQ(6.0_m, std::get<2>(ned2.point()));
		EXPECT_EQ(Boston.point(), ned2.frameData().origin);
		EXPECT_EQ(2016.0_yr, ned2.frameData().date);

		// different units
		NED ned3(4.0_km, 5.0_km, 6.0_km, Boston);
		EXPECT_EQ(4000.0_m, std::get<0>(ned3.point()));
		EXPECT_EQ(5000.0_m, std::get<1>(ned3.point()));
		EXPECT_EQ(6000.0_m, std::get<2>(ned3.point()));
		EXPECT_EQ(Boston.point(), ned3.frameData().origin);

		// origin with different frame of reference
		NED ned4(7.0_m, 8.0_m, 9.0_m, Boston_ecef);
		EXPECT_EQ(7.0_m, std::get<0>(ned4.point()));
		EXPECT_EQ(8.0_m, std::get<1>(ned4.point()));
		EXPECT_EQ(9.0_m, std::get<2>(ned4.point()));
		EXPECT_TRUE((Boston.isSame(LLA(ned4.frameData().origin), 1_mm)));	// the origin matches the original LLA within 1mm.
	}

	TEST_F(PositionNEDTest, tupleConstructor)
	{
		// no date
		NED ned(CartesianTuple(1.0_m, 2.0_m, 3.0_m), Boston);
		EXPECT_EQ(1.0_m, std::get<0>(ned.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ned.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ned.point()));
		EXPECT_EQ(Boston.point(), ned.frameData().origin);
		EXPECT_EQ(2005.0_yr, ned.frameData().date);

		// date
		NED ned2(CartesianTuple(4.0_m, 5.0_m, 6.0_m), Boston, 2016.0_yr);
		EXPECT_EQ(4.0_m, std::get<0>(ned2.point()));
		EXPECT_EQ(5.0_m, std::get<1>(ned2.point()));
		EXPECT_EQ(6.0_m, std::get<2>(ned2.point()));
		EXPECT_EQ(Boston.point(), ned2.frameData().origin);
		EXPECT_EQ(2016.0_yr, ned2.frameData().date);

		// different units
		NED ned3(CartesianTuple(4.0_km, 5.0_km, 6.0_km), Boston);
		EXPECT_EQ(4000.0_m, std::get<0>(ned3.point()));
		EXPECT_EQ(5000.0_m, std::get<1>(ned3.point()));
		EXPECT_EQ(6000.0_m, std::get<2>(ned3.point()));
		EXPECT_EQ(Boston.point(), ned3.frameData().origin);

		// origin with different frame of reference
		NED ned4(CartesianTuple(7.0_m, 8.0_m, 9.0_m), Boston_ecef);
		EXPECT_EQ(7.0_m, std::get<0>(ned4.point()));
		EXPECT_EQ(8.0_m, std::get<1>(ned4.point()));
		EXPECT_EQ(9.0_m, std::get<2>(ned4.point()));
		EXPECT_TRUE((Boston.isSame(LLA(ned4.frameData().origin), 1_mm)));	// the origin matches the original LLA within 1mm.
	}

	TEST_F(PositionNEDTest, originOnlyConstructor)
	{
		// no date
		NED ned(Boston);
		EXPECT_EQ(0.0_m, std::get<0>(ned.point()));
		EXPECT_EQ(0.0_m, std::get<1>(ned.point()));
		EXPECT_EQ(0.0_m, std::get<2>(ned.point()));
		EXPECT_EQ(Boston.point(), ned.frameData().origin);
		EXPECT_EQ(2005.0_yr, ned.frameData().date);

		// date
		NED ned2(Boston, 2016.0_yr);
		EXPECT_EQ(0.0_m, std::get<0>(ned2.point()));
		EXPECT_EQ(0.0_m, std::get<1>(ned2.point()));
		EXPECT_EQ(0.0_m, std::get<2>(ned2.point()));
		EXPECT_EQ(Boston.point(), ned2.frameData().origin);
		EXPECT_EQ(2016.0_yr, ned2.frameData().date);
	}

	TEST_F(PositionNEDTest, copyConstructor)
	{
		NED place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		NED place2(place);
		EXPECT_TRUE(is_point<decltype(place)>);
		EXPECT_TRUE(is_point<decltype(place2)>);
		EXPECT_TRUE(place == place2) << "place:  " << place << std::endl << "place2: " << place2 << std::endl;
	}

	TEST_F(PositionNEDTest, implicitConversionConstructor_fromECEF)
	{
		NED place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		NED place2(Bedford_ecef, Boston);
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<0>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<1>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(18.742960276271333_m, std::get<2>(place2.point()), 5.0e-9_m);
	}

	TEST_F(PositionNEDTest, implicitConversionConstructor_fromGeo)
	{
		NED place(Bedford, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		NED place2(Bedford, Boston);
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<0>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<1>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(18.742960276271333_m, std::get<2>(place2.point()), 5.0e-9_m);
	}

	TEST_F(PositionNEDTest, implicitConversionConstructor_fromENUsameOrigin)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		NED place2(place, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place2.point()), 5.0e-9_m);
	}

	TEST_F(PositionNEDTest, implicitConversionConstructor_fromENUdifferentOrigin)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		// should just result in a copy
		NED place2(place, Boston);
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<0>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<1>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(18.742960276271333_m, std::get<2>(place2.point()), 5.0e-9_m);
	}

		TEST_F(PositionNEDTest, implicitConversionConstructor_fromENU_sameOrigin)
	{
		// This previously could bind to the origin-only constructor via implicit conversion to origin_type,
		// collapsing the point to (0,0,0) at a shifted origin. Ensure we perform a real frame conversion.
		ENU enu(1000.0_m, 1000.0_m, 1000.0_m, LA);
		NED ned(enu);

		EXPECT_UNITS_EQ(1000.0_m, std::get<0>(ned.point()));    // North
		EXPECT_UNITS_EQ(1000.0_m, std::get<1>(ned.point()));    // East
		EXPECT_UNITS_EQ(-1000.0_m, std::get<2>(ned.point()));   // Down
		EXPECT_EQ(LA.point(), ned.frameData().origin);
	}


TEST_F(PositionNEDTest, implicitConversionConstructor_fromNEDsameOrigin)
	{
		NED place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		// should just result in a copy
		NED place2(place, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place2.point()), 5.0e-9_m);
	}

	TEST_F(PositionNEDTest, implicitConversionConstructor_fromNEDdifferentOrigin)
	{
		NED place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		// should just result in a copy
		NED place2(place, Boston);
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<0>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<1>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(18.742960276271333_m, std::get<2>(place2.point()), 5.0e-9_m);
	}

	TEST_F(PositionNEDTest, assignment)
	{
		NED place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		NED place2 = place;
		EXPECT_TRUE(is_point<decltype(place)>);
		EXPECT_TRUE(is_point<decltype(place2)>);
		EXPECT_TRUE(place == place2) << "place:  " << place << std::endl << "place2: " << place2 << std::endl;;
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromECEF)
	{
		NED place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		NED place2(Boston);
		place2 = Bedford_ecef;
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<0>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<1>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(18.742960276271333_m, std::get<2>(place2.point()), 5.0e-9_m);
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromGeo)
	{
		NED place(Bedford, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		NED place2(Boston);
		place2 = Bedford;
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<0>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<1>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(18.742960276271333_m, std::get<2>(place2.point()), 5.0e-9_m);
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromENUsameOrigin)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		// should just result in a copy
		NED place2;
		place2 = place;
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place2.point()), 5.0e-9_m);
		EXPECT_EQ(Lexington.point(), place2.frameData().origin);

		// should just result in a copy
		NED place3(Lexington);
		place3 = place;
		EXPECT_TRUE(place3 == place2) << "place2:  " << place << std::endl << "place3: " << place3 << std::endl; // place2 is not a typo!
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromENUdifferentOrigin)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		// should result in an origin translation
		NED place3(Boston);
		place3 = place;
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<0>(place3.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<1>(place3.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(18.742960276271333_m, std::get<2>(place3.point()), 5.0e-9_m);
		EXPECT_EQ(Boston.point(), place3.frameData().origin);
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromNEDsameOrigin)
	{
		NED place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		// should just result in a copy
		NED place2 = place;
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place2.point()), 5.0e-9_m);
		EXPECT_EQ(Lexington.point(), place2.frameData().origin);

		// should just result in a copy
		NED place3(Lexington);
		place3 = place;
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place3.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place3.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place3.point()), 5.0e-9_m);
		EXPECT_EQ(Lexington.point(), place3.frameData().origin);
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromNEDdifferentOrigin)
	{
		NED place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		// should result in an origin translation
		NED place3(Boston);
		place3 = place;
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<0>(place3.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<1>(place3.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(18.742960276271333_m, std::get<2>(place3.point()), 5.0e-9_m);
		EXPECT_EQ(Boston.point(), place3.frameData().origin);
	}

	TEST_F(PositionNEDTest, isSame)
	{
		LLA origin1(42.0_deg, -71.0_deg, 0.0_m);	// Boston-ish
		LLA origin2(34.0_deg, -118.0_deg, 0.0_m);	// LA-ish

		using NED_ft = PositionNED<WGS84_G1674, feet>;
		using ECEF_mm = PositionECEF<WGS84_G1674, millimeters>;

		NED ned1(1.0_m, 2.0_m, 3.0_m, origin1);
		NED ned2(4.0_m, 5.0_m, 6.0_m, origin2);

		NED exactlySame1(1.0_m, 2.0_m, 3.0_m, origin1);
		NED exactlySame2(4.0_m, 5.0_m, 6.0_m, origin2);

		NED exactlySameDiffOrigin1(ned1, origin2);	// These are not *actually* 100% the same, since c++ trig functions aren't fully reversible.
		NED exactlySameDiffOrigin2(ned2, origin1);

		NED close1(1.001_m, 2.002_m, 3.003_m, origin1);
		NED close2(4.001_m, 5.002_m, 6.003_m, origin2);

		NED_ft closeDifferentUnits(3.28083989_ft, 6.56167979_ft, 9.84251968_ft, origin1);
		ECEF_mm millimeterTolerance(1_mm, 2_mm, 3_mm);

		EXPECT_EQ(1.0_m, std::get<0>(ned1.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ned1.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ned1.point()));

		// exactly same, default (0) tolerance
		EXPECT_TRUE(ned1.isSame(exactlySame1));
		EXPECT_FALSE(ned1.isSame(close1));
		EXPECT_FALSE(ned1.isSame(exactlySame2));
		EXPECT_FALSE(ned1.isSame(exactlySameDiffOrigin2));

		EXPECT_TRUE(ned2.isSame(exactlySame2));
		EXPECT_FALSE(ned2.isSame(close2));
		EXPECT_FALSE(ned2.isSame(exactlySame1));
		EXPECT_FALSE(ned2.isSame(exactlySameDiffOrigin1));

 		// close, millimeter tolerance
		EXPECT_TRUE((ned1.isSame(exactlySame1, 1_mm)));
		EXPECT_TRUE((ned1.isSame(exactlySameDiffOrigin1, 1_mm)));
		EXPECT_FALSE((ned1.isSame(close1, 1_mm)));
		EXPECT_FALSE((ned1.isSame(exactlySame2, 1_mm)));
		EXPECT_FALSE((ned1.isSame(exactlySameDiffOrigin2, 1_mm)));

		EXPECT_TRUE((ned2.isSame(exactlySame2, 1_mm)));
		EXPECT_TRUE((ned2.isSame(exactlySameDiffOrigin2, 1_mm)));
		EXPECT_FALSE((ned2.isSame(close2, 1_mm)));
		EXPECT_FALSE((ned2.isSame(exactlySame1, 1_mm)));
		EXPECT_FALSE((ned2.isSame(exactlySameDiffOrigin1, 1_mm)));

		// close, millimeter(s) tolerance - point
		EXPECT_TRUE((ned1.isSame(exactlySame1, millimeterTolerance)));
		EXPECT_TRUE((ned1.isSame(exactlySameDiffOrigin1, millimeterTolerance)));
		EXPECT_FALSE((ned1.isSame(close1, millimeterTolerance)));
		EXPECT_FALSE((ned1.isSame(exactlySame2, millimeterTolerance)));
		EXPECT_FALSE((ned1.isSame(exactlySameDiffOrigin2, millimeterTolerance)));

		EXPECT_TRUE((ned2.isSame(exactlySame2, millimeterTolerance)));
		EXPECT_TRUE((ned2.isSame(exactlySameDiffOrigin2, millimeterTolerance)));
		EXPECT_FALSE((ned2.isSame(close2, millimeterTolerance)));
		EXPECT_FALSE((ned2.isSame(exactlySame1, millimeterTolerance)));
		EXPECT_FALSE((ned2.isSame(exactlySameDiffOrigin1, millimeterTolerance)));

		// close, centimeter tolerance
		EXPECT_TRUE((ned1.isSame(exactlySame1, 1_cm)));
		EXPECT_TRUE((ned1.isSame(exactlySameDiffOrigin1, 1_cm)));
		EXPECT_TRUE((ned1.isSame(close1, 1_cm)));
		EXPECT_FALSE((ned1.isSame(exactlySame2, 1_cm)));
		EXPECT_FALSE((ned1.isSame(exactlySameDiffOrigin2, 1_cm)));

		EXPECT_TRUE((ned2.isSame(exactlySame2, 1_cm)));
		EXPECT_TRUE((ned2.isSame(exactlySameDiffOrigin2, 1_cm)));
		EXPECT_TRUE((ned2.isSame(close2, 1_cm)));
		EXPECT_FALSE((ned2.isSame(exactlySame1, 1_cm)));
		EXPECT_FALSE((ned2.isSame(exactlySameDiffOrigin1, 1_cm)));

		// close, 4100 kilometer tolerance
		EXPECT_TRUE((ned1.isSame(exactlySame1, 4100.0_km)));
		EXPECT_TRUE((ned1.isSame(exactlySameDiffOrigin1, 4100.0_km)));
		EXPECT_TRUE((ned1.isSame(close1, 4100.0_km)));
		EXPECT_TRUE((ned1.isSame(exactlySame2, 4100.0_km)));
		EXPECT_TRUE((ned1.isSame(exactlySameDiffOrigin2, 4100.0_km)));

		EXPECT_TRUE((ned2.isSame(exactlySame2, 4100.0_km)));
		EXPECT_TRUE((ned2.isSame(exactlySameDiffOrigin2, 4100.0_km)));
		EXPECT_TRUE((ned2.isSame(close2, 4100.0_km)));
		EXPECT_TRUE((ned2.isSame(exactlySame1, 4100.0_km)));
		EXPECT_TRUE((ned2.isSame(exactlySameDiffOrigin1, 4100.0_km)));
	}

	TEST_F(PositionNEDTest, distance)
	{
		LLA origin1(42.0_deg, -71.0_deg, 0.0_m);	// Boston-ish
		LLA origin2(34.0_deg, -118.0_deg, 0.0_m);	// LA-ish

		NED nedb0(0.0_m, 0.0_m, 0.0_m, origin1);
		NED nedb1(1.0_m, 1.0_m, 1.0_m, origin1);

		NED nedl0(0.0_m, 0.0_m, 0.0_m, origin2);
		NED nedl1(1.0_m, 1.0_m, 1.0_m, origin2);

		// same origin
		EXPECT_UNITS_EQ(meters(sqrt(3.0)), nedb0.distance(nedb1));
		EXPECT_UNITS_EQ(meters(sqrt(3.0)), nedb1.distance(nedb0));
		EXPECT_UNITS_EQ(meters(sqrt(3.0)), nedl0.distance(nedl1));
		EXPECT_UNITS_EQ(meters(sqrt(3.0)), nedl1.distance(nedl0));

		// different point types
		EXPECT_UNITS_NEAR(meters(sqrt(3.0)), nedb1.distance(origin1), 5.0e-10_m);
		EXPECT_UNITS_NEAR(meters(sqrt(3.0)), nedl1.distance(origin2), 5.0e-10_m);

		//different origin
		EXPECT_UNITS_NEAR((4094891.087804173_m), nedb0.distance(nedl0), 1.0e-9_m);
		EXPECT_UNITS_NEAR((4094891.087804173_m), nedb0.distance(origin2), 1.0e-9_m);
	}

	TEST_F(PositionNEDTest, dotProduct)
	{
		LLA origin1(42.0_deg, -71.0_deg, 0.0_m);	// Boston-ish
		LLA origin2(34.0_deg, -118.0_deg, 0.0_m);	// LA-ish

		NED nedb0(1.0_m, 2.0_m, 3.0_m, origin1);
		NED nedb1(1.0_m, 2.0_m, 3.0_m, origin1);

		NED nedl0(4.0_m, 5.0_m, 6.0_m, origin2);
		NED nedl1(4.0_m, 5.0_m, 6.0_m, origin2);

		// truth data from matlab

		// same origin
		EXPECT_EQ(14_m2, nedb0.dotProduct(nedb1));
		EXPECT_EQ(77_m2, nedl0.dotProduct(nedl1));

		// different origins
		EXPECT_UNITS_NEAR(-3562875.264019654_m2, nedb0.dotProduct(nedl1), 5.0e-8_m2);			// wrt origin1
		EXPECT_UNITS_NEAR(32155885.49223859_m2, nedl1.dotProduct(nedb0), 5.0e-8_m2);			// wrt origin2

		// different point types
		EXPECT_UNITS_NEAR(-3562875.264019654_m2, nedb0.dotProduct(nedl1), 5.0e-8_m2);			// wrt origin1
		EXPECT_UNITS_NEAR(32155885.49223859_m2, nedl1.dotProduct(nedb0), 5.0e-8_m2);			// wrt origin2
	}

	TEST_F(PositionNEDTest, magnitude)
	{
		// should be the same if x/y/z values are the same, regardless of origin
		NED ned1(1.0_m, 2.0_m, 3.0_m, LLA());
		NED ned2(1.0_m, 2.0_m, 3.0_m, Boston);
		NED ned3(4.0_m, 5.0_m, 6.0_m, Lexington);

		EXPECT_UNITS_NEAR(3.741657386773941_m, ned1.magnitude(), 5.0e-15_m);
		EXPECT_UNITS_NEAR(3.741657386773941_m, ned2.magnitude(), 5.0e-15_m);
		EXPECT_UNITS_NEAR(8.774964387392123_m, ned3.magnitude(), 5.0e-15_m);
	}

	TEST_F(PositionNEDTest, east)
	{
		NED ned;
		EXPECT_EQ(0.0_m, ned.north());

		NED ned1(5.0_m, 6.0_m, 7.0_m, LLA());
		EXPECT_EQ(5.0_m, ned1.north());

		NED ned2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(5_mm, ned2.north());
	}

	TEST_F(PositionNEDTest, north)
	{
		NED ned;
		EXPECT_EQ(0.0_m, ned.east());

		NED ned1(5.0_m, 6.0_m, 7.0_m, LLA());
		EXPECT_EQ(6.0_m, ned1.east());

		NED ned2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(6_mm, ned2.east());
	}

	TEST_F(PositionNEDTest, up)
	{
		NED ned;
		EXPECT_EQ(0.0_m, ned.down());

		NED ned1(5.0_m, 6.0_m, 7.0_m, LLA());
		EXPECT_EQ(7.0_m, ned1.down());

		NED ned2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(7_mm, ned2.down());
	}

	TEST_F(PositionNEDTest, origin)
	{
		NED ned;
		EXPECT_TRUE(LLA(0.0_deg, 0.0_deg, 0.0_m) == ned.origin());

		NED ned2(0.0_m, 0.0_m, 0.0_m, Boston);
		EXPECT_TRUE(Boston == ned2.origin());
	}

	TEST_F(PositionNEDTest, date)
	{
		NED ned;
		EXPECT_EQ(2005.0_yr, ned.date());

		NED ned2(0.0_m, 0.0_m, 0.0_m, LLA(), 2016.421_yr);
		EXPECT_EQ(2016.421_yr, ned2.date());
	}

	TEST_F(PositionNEDTest, setNorth)
	{
		NED ned;
		EXPECT_EQ(0.0_m, ned.north());

		ned.setNorth(5.0_m);
		EXPECT_EQ(5.0_m, ned.north());
		EXPECT_EQ(0.0_m, ned.east());
		EXPECT_EQ(0.0_m, ned.down());

		ned.setNorth(5_mm);
		EXPECT_EQ(0.005_m, ned.north());
		EXPECT_EQ(0.0_m, ned.east());
		EXPECT_EQ(0.0_m, ned.down());
	}

	TEST_F(PositionNEDTest, setEast)
	{
		NED ned;
		EXPECT_EQ(0.0_m, ned.east());

		ned.setEast(6.0_m);
		EXPECT_EQ(0.0_m, ned.north());
		EXPECT_EQ(6.0_m, ned.east());
		EXPECT_EQ(0.0_m, ned.down());

		ned.setEast(6_mm);
		EXPECT_EQ(0.0_m, ned.north());
		EXPECT_EQ(0.006_m, ned.east());
		EXPECT_EQ(0.0_m, ned.down());
	}

	TEST_F(PositionNEDTest, setDown)
	{
		NED ned;
		EXPECT_EQ(0.0_m, ned.down());

		ned.setDown(7.0_m);
		EXPECT_EQ(0.0_m, ned.north());
		EXPECT_EQ(0.0_m, ned.east());
		EXPECT_EQ(7.0_m, ned.down());

		ned.setDown(7_mm);
		EXPECT_EQ(0.0_m, ned.north());
		EXPECT_EQ(0.0_m, ned.east());
		EXPECT_EQ(0.007_m, ned.down());
	}

	TEST_F(PositionNEDTest, setOrigin)
	{
		NED ned;
		EXPECT_TRUE(LLA(0.0_deg, 0.0_deg, 0.0_m) == ned.origin());

		NED ned2(0.0_m, 0.0_m, 0.0_m, Boston);
		EXPECT_TRUE(Boston == ned2.origin());

		NED ned3(ned2);
		ned3.setOrigin(Lexington);
		EXPECT_UNITS_NEAR(-9194.581078166790_m, ned3.north(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(14013.15891974845_m, ned3.east(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(22.012073203577529_m, ned3.down(), 5.0e-9_m);
		EXPECT_TRUE(Lexington == ned3.origin());
		EXPECT_TRUE(ned2.isSame(ned3, 3_nm));
	}

	TEST_F(PositionNEDTest, ostream)
	{
		PositionNED<WGS84_G1674, inches> ned1(12.1_in, 24.2_in, 36.3_in, LLA(42.0_deg, -71.0_deg, 0.0_m));

		EXPECT_EQ(12.1_in, ned1.north());
		EXPECT_EQ(24.2_in, ned1.east());
		EXPECT_EQ(36.3_in, ned1.down());

		testing::internal::CaptureStdout();
		std::cout << ned1;
		std::string output = testing::internal::GetCapturedStdout();

		EXPECT_STREQ("(12.1 in, 24.2 in, 36.3 in) @ (42 deg, -71 deg, 0 m)", output.c_str());
	}

	TEST_F(PositionNEDTest, plusEqual)
	{
		NED ned1(12.0_m, 24.0_m, 36.0_m, Boston);
		NED ned2(36.0_m, 24.0_m, 12.0_m, Boston);
		NED ned3(1.0_m, 2.0_m, 3.0_m, Lexington);

		EXPECT_EQ(12.0_m, ned1.north());
		EXPECT_EQ(24.0_m, ned1.east());
		EXPECT_EQ(36.0_m, ned1.down());

		EXPECT_EQ(36.0_m, ned2.north());
		EXPECT_EQ(24.0_m, ned2.east());
		EXPECT_EQ(12.0_m, ned2.down());

		EXPECT_EQ(1.0_m, ned3.north());
		EXPECT_EQ(2.0_m, ned3.east());
		EXPECT_EQ(3.0_m, ned3.down());

		// same units
		ned1 += ned2;

		EXPECT_EQ(48.0_m, ned1.north());
		EXPECT_EQ(48.0_m, ned1.east());
		EXPECT_EQ(48.0_m, ned1.down());

		EXPECT_EQ(36.0_m, ned2.north());
		EXPECT_EQ(24.0_m, ned2.east());
		EXPECT_EQ(12.0_m, ned2.down());

		// different units
		ned2 += ned3;
		EXPECT_UNITS_NEAR(9259.607688572551_m, ned2.north(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13968.71534922575_m, ned2.east(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(37.009226581068106_m, ned2.down(), 5.0e-9_m);

		EXPECT_EQ(1.0_m, ned3.north());
		EXPECT_EQ(2.0_m, ned3.east());
		EXPECT_EQ(3.0_m, ned3.down());
	}

	TEST_F(PositionNEDTest, minusEqual)
	{
		NED zero;
		NED ned1(12.0_m, 24.0_m, 36.0_m, Boston);
		NED ned2(36.0_m, 24.0_m, 12.0_m, Boston);
		NED ned3(1.0_m, 2.0_m, 3.0_m, Lexington);

		EXPECT_EQ(12.0_m, ned1.north());
		EXPECT_EQ(24.0_m, ned1.east());
		EXPECT_EQ(36.0_m, ned1.down());

		EXPECT_EQ(36.0_m, ned2.north());
		EXPECT_EQ(24.0_m, ned2.east());
		EXPECT_EQ(12.0_m, ned2.down());

		EXPECT_EQ(1.0_m, ned3.north());
		EXPECT_EQ(2.0_m, ned3.east());
		EXPECT_EQ(3.0_m, ned3.down());

		// same units
		ned1 -= ned2;

		EXPECT_EQ(-24.0_m, ned1.north());
		EXPECT_EQ(0.0_m, ned1.east());
		EXPECT_EQ(24.0_m, ned1.down());

		EXPECT_EQ(36.0_m, ned2.north());
		EXPECT_EQ(24.0_m, ned2.east());
		EXPECT_EQ(12.0_m, ned2.down());

		// different units
		ned2 -= ned3;
		EXPECT_UNITS_NEAR(-9187.607688572551_m, ned2.north(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(14016.71534922575_m, ned2.east(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-13.009226581068106_m, ned2.down(), 5.0e-9_m);

		EXPECT_EQ(1.0_m, ned3.north());
		EXPECT_EQ(2.0_m, ned3.east());
		EXPECT_EQ(3.0_m, ned3.down());
	}

	TEST_F(PositionNEDTest, timesEqual)
	{
		PositionNED<NAD83, feet> ned_ft(1.0_ft, 2.0_ft, 3.0_ft, Boston);

		ned_ft *= 3;
		EXPECT_EQ(3.0_ft, ned_ft.north());
		EXPECT_EQ(6.0_ft, ned_ft.east());
		EXPECT_EQ(9.0_ft, ned_ft.down());
	}

	TEST_F(PositionNEDTest, divideEqual)
	{
		PositionNED<NAD83, feet> ned_ft(1.0_ft, 2.0_ft, 3.0_ft, Boston);

		ned_ft /= 3;
		EXPECT_EQ(feet(1.0 / 3), ned_ft.north());
		EXPECT_EQ(feet(2.0 / 3), ned_ft.east());
		EXPECT_EQ(1.0_ft, ned_ft.down());
	}
}


#endif // positionNEDTest_h__