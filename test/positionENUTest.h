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

#ifndef positionENUTest_h__
#define positionENUTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <stdexcept>
#include <type_traits>

using namespace coordinates;
using namespace units;
using namespace units::length;
using namespace units::area;
using namespace units::angle;
using namespace units::time;

namespace
{
	// The fixture for testing class Foo.
	class PositionENUTest : public ::testing::Test {
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		PositionENUTest()
		{
			// You can do set-up work for each test here.
		}

		~PositionENUTest() override
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

			Bedford.setPoint(42.4625772_deg, -71.2696797_deg, 15.0_m);
			Bedford_ecef.setPoint(1513242.623813242_m, -4462904.554429035_m, 4283659.060698663_m);

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

		LLA Lexington;

		LLA LA;
		ECEF LA_ecef;

		LLA Bedford;
		ECEF Bedford_ecef;
	};

	TEST_F(PositionENUTest, is_point)
	{
		EXPECT_TRUE(coordinates::traits::is_point<ENU>);
	}

	TEST_F(PositionENUTest, defaultConstructor)
	{
		EXPECT_TRUE(std::is_default_constructible_v<ENU>);
	}

	TEST_F(PositionENUTest, constructor)
	{
		// no date
		ENU enu(1.0_m, 2.0_m, 3.0_m, Boston);
		EXPECT_EQ(1.0_m, std::get<0>(enu.point()));
		EXPECT_EQ(2.0_m, std::get<1>(enu.point()));
		EXPECT_EQ(3.0_m, std::get<2>(enu.point()));
		EXPECT_EQ(Boston.point(), enu.frameData().origin);
		EXPECT_EQ(2005.0_yr, enu.frameData().date);

		// date
		ENU enu2(4.0_m, 5.0_m, 6.0_m, Boston, 2016.0_yr);
		EXPECT_EQ(4.0_m, std::get<0>(enu2.point()));
		EXPECT_EQ(5.0_m, std::get<1>(enu2.point()));
		EXPECT_EQ(6.0_m, std::get<2>(enu2.point()));
		EXPECT_EQ(Boston.point(), enu2.frameData().origin);
		EXPECT_EQ(2016.0_yr, enu2.frameData().date);

		// different units
		ENU enu3(4.0_km, 5.0_km, 6.0_km, Boston);
		EXPECT_EQ(4000.0_m, std::get<0>(enu3.point()));
		EXPECT_EQ(5000.0_m, std::get<1>(enu3.point()));
		EXPECT_EQ(6000.0_m, std::get<2>(enu3.point()));
		EXPECT_EQ(Boston.point(), enu3.frameData().origin);

		// origin with different frame of reference
		ENU enu4(7.0_m, 8.0_m, 9.0_m, Boston_ecef);
		EXPECT_EQ(7.0_m, std::get<0>(enu4.point()));
		EXPECT_EQ(8.0_m, std::get<1>(enu4.point()));
		EXPECT_EQ(9.0_m, std::get<2>(enu4.point()));
		EXPECT_TRUE((Boston.isSame(LLA(enu4.frameData().origin), 1_mm)));	// the origin matches the original LLA within 1mm.
	}

	TEST_F(PositionENUTest, tupleConstructor)
	{
		// no date
		ENU enu(CartesianTuple(1.0_m, 2.0_m, 3.0_m), Boston);
		EXPECT_EQ(1.0_m, std::get<0>(enu.point()));
		EXPECT_EQ(2.0_m, std::get<1>(enu.point()));
		EXPECT_EQ(3.0_m, std::get<2>(enu.point()));
		EXPECT_EQ(Boston.point(), enu.frameData().origin);
		EXPECT_EQ(2005.0_yr, enu.frameData().date);

		// date
		ENU enu2(CartesianTuple(4.0_m, 5.0_m, 6.0_m), Boston, 2016.0_yr);
		EXPECT_EQ(4.0_m, std::get<0>(enu2.point()));
		EXPECT_EQ(5.0_m, std::get<1>(enu2.point()));
		EXPECT_EQ(6.0_m, std::get<2>(enu2.point()));
		EXPECT_EQ(Boston.point(), enu2.frameData().origin);
		EXPECT_EQ(2016.0_yr, enu2.frameData().date);

		// different units
		ENU enu3(CartesianTuple(4.0_km, 5.0_km, 6.0_km), Boston);
		EXPECT_EQ(4000.0_m, std::get<0>(enu3.point()));
		EXPECT_EQ(5000.0_m, std::get<1>(enu3.point()));
		EXPECT_EQ(6000.0_m, std::get<2>(enu3.point()));
		EXPECT_EQ(Boston.point(), enu3.frameData().origin);

		// origin with different frame of reference
		ENU enu4(CartesianTuple(7.0_m, 8.0_m, 9.0_m), Boston_ecef);
		EXPECT_EQ(7.0_m, std::get<0>(enu4.point()));
		EXPECT_EQ(8.0_m, std::get<1>(enu4.point()));
		EXPECT_EQ(9.0_m, std::get<2>(enu4.point()));
		EXPECT_TRUE((Boston.isSame(LLA(enu4.frameData().origin), 1_mm)));	// the origin matches the original LLA within 1mm.
	}

	TEST_F(PositionENUTest, originOnlyConstructor)
	{
		// no date
		ENU enu(Boston);
		EXPECT_EQ(0.0_m, std::get<0>(enu.point()));
		EXPECT_EQ(0.0_m, std::get<1>(enu.point()));
		EXPECT_EQ(0.0_m, std::get<2>(enu.point()));
		EXPECT_EQ(Boston.point(), enu.frameData().origin);
		EXPECT_EQ(2005.0_yr, enu.frameData().date);

		// date
		ENU enu2(Boston, 2016.0_yr);
		EXPECT_EQ(0.0_m, std::get<0>(enu2.point()));
		EXPECT_EQ(0.0_m, std::get<1>(enu2.point()));
		EXPECT_EQ(0.0_m, std::get<2>(enu2.point()));
		EXPECT_EQ(Boston.point(), enu2.frameData().origin);
		EXPECT_EQ(2016.0_yr, enu2.frameData().date);
	}

	TEST_F(PositionENUTest, copyConstructor)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		ENU llab2(place);
		EXPECT_TRUE(coordinates::traits::is_point<decltype(place)>);
		EXPECT_TRUE(coordinates::traits::is_point<decltype(llab2)>);
		EXPECT_TRUE(place == llab2);
	}

	TEST_F(PositionENUTest, implicitConversionConstructor_fromECEF)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		ENU llab2(Bedford_ecef, Boston);
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<0>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<1>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-18.742960276271333_m, std::get<2>(llab2.point()), 5.0e-9_m);
	}

	TEST_F(PositionENUTest, implicitConversionConstructor_fromGeo)
	{
		ENU place(Bedford, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		ENU llab2(Bedford, Boston);
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<0>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<1>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-18.742960276271333_m, std::get<2>(llab2.point()), 5.0e-9_m);
	}

	TEST_F(PositionENUTest, implicitConversionConstructor_fromENUsameOrigin)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		// should just result in a copy
		ENU llab2(place, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(llab2.point()), 5.0e-9_m);
	}

	TEST_F(PositionENUTest, implicitConversionConstructor_fromENUdifferentOrigin)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		// should just result in a copy
		ENU llab2(place, Boston);
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<0>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<1>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-18.742960276271333_m, std::get<2>(llab2.point()), 5.0e-9_m);
	}

		TEST_F(PositionENUTest, implicitConversionConstructor_fromNED_sameOrigin)
	{
		// This previously could bind to the origin-only constructor via implicit conversion to origin_type,
		// collapsing the point to (0,0,0) at a shifted origin. Ensure we perform a real frame conversion.
		NED ned(1000.0_m, 1000.0_m, -1000.0_m, LA);
		ENU enu(ned);

		EXPECT_UNITS_EQ(1000.0_m, std::get<0>(enu.point()));   // East
		EXPECT_UNITS_EQ(1000.0_m, std::get<1>(enu.point()));   // North
		EXPECT_UNITS_EQ(1000.0_m, std::get<2>(enu.point()));   // Up
		EXPECT_EQ(LA.point(), enu.frameData().origin);
	}


TEST_F(PositionENUTest, assignment)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		ENU llab2 = place;
		EXPECT_TRUE(coordinates::traits::is_point<decltype(place)>);
		EXPECT_TRUE(coordinates::traits::is_point<decltype(llab2)>);
		EXPECT_TRUE(place == llab2);
	}

	TEST_F(PositionENUTest, implicitConversionAssignment_fromECEF)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		ENU llab2(Boston);
		llab2 = Bedford_ecef;
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<0>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<1>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-18.742960276271333_m, std::get<2>(llab2.point()), 5.0e-9_m);
	}

	TEST_F(PositionENUTest, implicitConversionAssignment_fromGeo)
	{
		ENU place(Bedford, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		ENU llab2(Boston);
		llab2 = Bedford;
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<0>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<1>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-18.742960276271333_m, std::get<2>(llab2.point()), 5.0e-9_m);
	}

	TEST_F(PositionENUTest, implicitConversionAssignment_fromENUsameOrigin)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		// should just result in a copy
		ENU llab2 = place;
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(llab2.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(llab2.point()), 5.0e-9_m);
		EXPECT_EQ(Lexington.point(), llab2.frameData().origin);

		// should just result in a copy
		ENU llab3(Lexington);
		llab3 = llab2;
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(llab3.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(llab3.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(llab3.point()), 5.0e-9_m);
		EXPECT_EQ(Lexington.point(), llab3.frameData().origin);
	}

	TEST_F(PositionENUTest, implicitConversionAssignment_fromENUdifferentOrigin)
	{
		ENU place(Bedford_ecef, Lexington);
		EXPECT_UNITS_NEAR(-3345.831985833716_m, std::get<0>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(2175.482814617637_m, std::get<1>(place.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(13.751962531646313_m, std::get<2>(place.point()), 5.0e-9_m);

		// should result in an origin translation
		ENU llab3(Boston);
		llab3 = place;
		EXPECT_UNITS_NEAR(-17336.21267499651_m, std::get<0>(llab3.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(11404.80495410014_m, std::get<1>(llab3.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-18.742960276271333_m, std::get<2>(llab3.point()), 5.0e-9_m);
		EXPECT_EQ(Boston.point(), llab3.frameData().origin);
	}

	TEST_F(PositionENUTest, isSame)
	{
		LLA origin1(42.0_deg, -71.0_deg, 0.0_m);	// Boston-ish
		LLA origin2(34.0_deg, -118.0_deg, 0.0_m);	// LA-ish

		using ENU_ft = PositionENU<WGS84_G1674, feet>;
		using ECEF_mm = PositionECEF<WGS84_G1674, millimeters>;

		ENU enu1(1.0_m, 2.0_m, 3.0_m, origin1);
		ENU enu2(4.0_m, 5.0_m, 6.0_m, origin2);

		ENU exactlySame1(1.0_m, 2.0_m, 3.0_m, origin1);
		ENU exactlySame2(4.0_m, 5.0_m, 6.0_m, origin2);

		ENU exactlySameDiffOrigin1(enu1, origin2);	// These are not *actually* 100% the same, since c++ trig functions aren't fully reversible.
		ENU exactlySameDiffOrigin2(enu2, origin1);

		ENU close1(1.001_m, 2.002_m, 3.003_m, origin1);
		ENU close2(4.001_m, 5.002_m, 6.003_m, origin2);

		ENU_ft closeDifferentUnits(3.28083989_ft, 6.56167979_ft, 9.84251968_ft, origin1);
		ECEF_mm millimeterTolerance(1_mm, 2_mm, 3_mm);

		EXPECT_EQ(1.0_m, std::get<0>(enu1.point()));
		EXPECT_EQ(2.0_m, std::get<1>(enu1.point()));
		EXPECT_EQ(3.0_m, std::get<2>(enu1.point()));

		// exactly same, default (0) tolerance
		EXPECT_TRUE(enu1.isSame(exactlySame1));
		EXPECT_FALSE(enu1.isSame(close1));
		EXPECT_FALSE(enu1.isSame(exactlySame2));
		EXPECT_FALSE(enu1.isSame(exactlySameDiffOrigin2));

		EXPECT_TRUE(enu2.isSame(exactlySame2));
		EXPECT_FALSE(enu2.isSame(close2));
		EXPECT_FALSE(enu2.isSame(exactlySame1));
		EXPECT_FALSE(enu2.isSame(exactlySameDiffOrigin1));

 		// close, millimeter tolerance
		EXPECT_TRUE((enu1.isSame(exactlySame1, 1_mm)));
		EXPECT_TRUE((enu1.isSame(exactlySameDiffOrigin1, 1_mm)));
		EXPECT_FALSE((enu1.isSame(close1, 1_mm)));
		EXPECT_FALSE((enu1.isSame(exactlySame2, 1_mm)));
		EXPECT_FALSE((enu1.isSame(exactlySameDiffOrigin2, 1_mm)));

		EXPECT_TRUE((enu2.isSame(exactlySame2, 1_mm)));
		EXPECT_TRUE((enu2.isSame(exactlySameDiffOrigin2, 1_mm)));
		EXPECT_FALSE((enu2.isSame(close2, 1_mm)));
		EXPECT_FALSE((enu2.isSame(exactlySame1, 1_mm)));
		EXPECT_FALSE((enu2.isSame(exactlySameDiffOrigin1, 1_mm)));

		// close, millimeter(s) tolerance - point
		EXPECT_TRUE((enu1.isSame(exactlySame1, millimeterTolerance)));
		EXPECT_TRUE((enu1.isSame(exactlySameDiffOrigin1, millimeterTolerance)));
		EXPECT_FALSE((enu1.isSame(close1, millimeterTolerance)));
		EXPECT_FALSE((enu1.isSame(exactlySame2, millimeterTolerance)));
		EXPECT_FALSE((enu1.isSame(exactlySameDiffOrigin2, millimeterTolerance)));

		EXPECT_TRUE((enu2.isSame(exactlySame2, millimeterTolerance)));
		EXPECT_TRUE((enu2.isSame(exactlySameDiffOrigin2, millimeterTolerance)));
		EXPECT_FALSE((enu2.isSame(close2, millimeterTolerance)));
		EXPECT_FALSE((enu2.isSame(exactlySame1, millimeterTolerance)));
		EXPECT_FALSE((enu2.isSame(exactlySameDiffOrigin1, millimeterTolerance)));

		// close, centimeter tolerance
		EXPECT_TRUE((enu1.isSame(exactlySame1, 1_cm)));
		EXPECT_TRUE((enu1.isSame(exactlySameDiffOrigin1, 1_cm)));
		EXPECT_TRUE((enu1.isSame(close1, 1_cm)));
		EXPECT_FALSE((enu1.isSame(exactlySame2, 1_cm)));
		EXPECT_FALSE((enu1.isSame(exactlySameDiffOrigin2, 1_cm)));

		EXPECT_TRUE((enu2.isSame(exactlySame2, 1_cm)));
		EXPECT_TRUE((enu2.isSame(exactlySameDiffOrigin2, 1_cm)));
		EXPECT_TRUE((enu2.isSame(close2, 1_cm)));
		EXPECT_FALSE((enu2.isSame(exactlySame1, 1_cm)));
		EXPECT_FALSE((enu2.isSame(exactlySameDiffOrigin1, 1_cm)));

		// close, 4100 kilometer tolerance
		EXPECT_TRUE((enu1.isSame(exactlySame1, 4100.0_km)));
		EXPECT_TRUE((enu1.isSame(exactlySameDiffOrigin1, 4100.0_km)));
		EXPECT_TRUE((enu1.isSame(close1, 4100.0_km)));
		EXPECT_TRUE((enu1.isSame(exactlySame2, 4100.0_km)));
		EXPECT_TRUE((enu1.isSame(exactlySameDiffOrigin2, 4100.0_km)));

		EXPECT_TRUE((enu2.isSame(exactlySame2, 4100.0_km)));
		EXPECT_TRUE((enu2.isSame(exactlySameDiffOrigin2, 4100.0_km)));
		EXPECT_TRUE((enu2.isSame(close2, 4100.0_km)));
		EXPECT_TRUE((enu2.isSame(exactlySame1, 4100.0_km)));
		EXPECT_TRUE((enu2.isSame(exactlySameDiffOrigin1, 4100.0_km)));
	}

	TEST_F(PositionENUTest, distance)
	{
		LLA origin1(42.0_deg, -71.0_deg, 0.0_m);	// Boston-ish
		LLA origin2(34.0_deg, -118.0_deg, 0.0_m);	// LA-ish

		ENU enub0(0.0_m, 0.0_m, 0.0_m, origin1);
		ENU enub1(1.0_m, 1.0_m, 1.0_m, origin1);

		ENU enul0(0.0_m, 0.0_m, 0.0_m, origin2);
		ENU enul1(1.0_m, 1.0_m, 1.0_m, origin2);

		// same origin
		EXPECT_UNITS_EQ(meters(sqrt(3.0)), enub0.distance(enub1));
		EXPECT_UNITS_EQ(meters(sqrt(3.0)), enub1.distance(enub0));
		EXPECT_UNITS_EQ(meters(sqrt(3.0)), enul0.distance(enul1));
		EXPECT_UNITS_EQ(meters(sqrt(3.0)), enul1.distance(enul0));

		// different point types
		EXPECT_UNITS_NEAR(meters(sqrt(3.0)), enub1.distance(origin1), 5.0e-10_m);
		EXPECT_UNITS_NEAR(meters(sqrt(3.0)), enul1.distance(origin2), 5.0e-10_m);

		//different origin
		EXPECT_UNITS_NEAR((4094891.087804173_m), enub0.distance(enul0), 1.0e-9_m);
		EXPECT_UNITS_NEAR((4094891.087804173_m), enub0.distance(origin2), 1.0e-9_m);
	}

	TEST_F(PositionENUTest, dotProduct)
	{
		LLA origin1(42.0_deg, -71.0_deg, 0.0_m);	// Boston-ish
		LLA origin2(34.0_deg, -118.0_deg, 0.0_m);	// LA-ish

		ENU enub0(1.0_m, 2.0_m, 3.0_m, origin1);
		ENU enub1(1.0_m, 2.0_m, 3.0_m, origin1);

		ENU enul0(4.0_m, 5.0_m, 6.0_m, origin2);
		ENU enul1(4.0_m, 5.0_m, 6.0_m, origin2);

		// truth data from matlab

		// same origin
		EXPECT_EQ(14_m2, enub0.dotProduct(enub1));
		EXPECT_EQ(77_m2, enul0.dotProduct(enul1));

		// different origins
		EXPECT_UNITS_NEAR(-7327114.484793222_m2, enub0.dotProduct(enul1), 5.0e-8_m2);			// wrt origin1
		EXPECT_UNITS_NEAR(14652796.55974842_m2, enul1.dotProduct(enub0), 5.0e-8_m2);			// wrt origin2

		// different point types
		EXPECT_UNITS_NEAR(-7327114.484793222_m2, enub0.dotProduct(enul1), 5.0e-8_m2);			// wrt origin1
		EXPECT_UNITS_NEAR(14652796.55974842_m2, enul1.dotProduct(enub0), 5.0e-8_m2);			// wrt origin2
	}

	TEST_F(PositionENUTest, magnitude)
	{
		// should be the same if x/y/z values are the same, regardless of origin
		ENU enu1(1.0_m, 2.0_m, 3.0_m, LLA());
		ENU enu2(1.0_m, 2.0_m, 3.0_m, Boston);
		ENU enu3(4.0_m, 5.0_m, 6.0_m, Lexington);

		EXPECT_UNITS_NEAR(3.741657386773941_m, enu1.magnitude(), 5.0e-15_m);
		EXPECT_UNITS_NEAR(3.741657386773941_m, enu2.magnitude(), 5.0e-15_m);
		EXPECT_UNITS_NEAR(8.774964387392123_m, enu3.magnitude(), 5.0e-15_m);
	}

	TEST_F(PositionENUTest, east)
	{
		ENU enu;
		EXPECT_EQ(0.0_m, enu.east());

		ENU enu1(5.0_m, 6.0_m, 7.0_m, LLA());
		EXPECT_EQ(5.0_m, enu1.east());

		ENU enu2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(5_mm, enu2.east());
	}

	TEST_F(PositionENUTest, north)
	{
		ENU enu;
		EXPECT_EQ(0.0_m, enu.north());

		ENU enu1(5.0_m, 6.0_m, 7.0_m, LLA());
		EXPECT_EQ(6.0_m, enu1.north());

		ENU enu2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(6_mm, enu2.north());
	}

	TEST_F(PositionENUTest, up)
	{
		ENU enu;
		EXPECT_EQ(0.0_m, enu.up());

		ENU enu1(5.0_m, 6.0_m, 7.0_m, LLA());
		EXPECT_EQ(7.0_m, enu1.up());

		ENU enu2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(7_mm, enu2.up());
	}

	TEST_F(PositionENUTest, origin)
	{
		ENU enu;
		EXPECT_TRUE(LLA(0.0_deg, 0.0_deg, 0.0_m) == enu.origin());

		ENU enu2(0.0_m, 0.0_m, 0.0_m, Boston);
		EXPECT_TRUE(Boston == enu2.origin());
	}

	TEST_F(PositionENUTest, date)
	{
		ENU enu;
		EXPECT_EQ(2005.0_yr, enu.date());

		ENU enu2(0.0_m, 0.0_m, 0.0_m, LLA(), 2016.421_yr);
		EXPECT_EQ(2016.421_yr, enu2.date());
	}

	TEST_F(PositionENUTest, setEast)
	{
		ENU enu;
		EXPECT_EQ(0.0_m, enu.east());

		enu.setEast(5.0_m);
		EXPECT_EQ(5.0_m, enu.east());
		EXPECT_EQ(0.0_m, enu.north());
		EXPECT_EQ(0.0_m, enu.up());

		enu.setEast(5_mm);
		EXPECT_EQ(0.005_m, enu.east());
		EXPECT_EQ(0.0_m, enu.north());
		EXPECT_EQ(0.0_m, enu.up());
	}

	TEST_F(PositionENUTest, setNorth)
	{
		ENU enu;
		EXPECT_EQ(0.0_m, enu.north());

		enu.setNorth(6.0_m);
		EXPECT_EQ(0.0_m, enu.east());
		EXPECT_EQ(6.0_m, enu.north());
		EXPECT_EQ(0.0_m, enu.up());

		enu.setNorth(6_mm);
		EXPECT_EQ(0.0_m, enu.east());
		EXPECT_EQ(0.006_m, enu.north());
		EXPECT_EQ(0.0_m, enu.up());
	}

	TEST_F(PositionENUTest, setUp)
	{
		ENU enu;
		EXPECT_EQ(0.0_m, enu.up());

		enu.setUp(7.0_m);
		EXPECT_EQ(0.0_m, enu.east());
		EXPECT_EQ(0.0_m, enu.north());
		EXPECT_EQ(7.0_m, enu.up());

		enu.setUp(7_mm);
		EXPECT_EQ(0.0_m, enu.east());
		EXPECT_EQ(0.0_m, enu.north());
		EXPECT_EQ(0.007_m, enu.up());
	}

	TEST_F(PositionENUTest, setOrigin)
	{
		ENU enu;
		EXPECT_TRUE(LLA(0.0_deg, 0.0_deg, 0.0_m) == enu.origin());

		ENU enu2(0.0_m, 0.0_m, 0.0_m, Boston);
		EXPECT_TRUE(Boston == enu2.origin());

		ENU enu3(enu2);
		enu3.setOrigin(Lexington);
		EXPECT_UNITS_NEAR(14013.15891974845_m, enu3.east(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-9194.581078166790_m, enu3.north(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-22.012073203577529_m, enu3.up(), 5.0e-9_m);
		EXPECT_TRUE(Lexington == enu3.origin());
		EXPECT_TRUE(enu2.isSame(enu3, 3_nm));
	}

	TEST_F(PositionENUTest, ostream)
	{
		PositionENU<WGS84_G1674, inches> enu1(12.1_in, 24.2_in, 36.3_in, LLA(42.0_deg, -71.0_deg, 0.0_m));

		EXPECT_EQ(12.1_in, enu1.east());
		EXPECT_EQ(24.2_in, enu1.north());
		EXPECT_EQ(36.3_in, enu1.up());

		testing::internal::CaptureStdout();
		std::cout << enu1;
		std::string output = testing::internal::GetCapturedStdout();

		EXPECT_STREQ("(12.1 in, 24.2 in, 36.3 in) @ (42 deg, -71 deg, 0 m)", output.c_str());
	}

	TEST_F(PositionENUTest, plusEqual)
	{
		ENU enu1(12.0_m, 24.0_m, 36.0_m, Boston);
		ENU enu2(36.0_m, 24.0_m, 12.0_m, Boston);
		ENU enu3(1.0_m, 2.0_m, 3.0_m, Lexington);

		EXPECT_EQ(12.0_m, enu1.east());
		EXPECT_EQ(24.0_m, enu1.north());
		EXPECT_EQ(36.0_m, enu1.up());

		EXPECT_EQ(36.0_m, enu2.east());
		EXPECT_EQ(24.0_m, enu2.north());
		EXPECT_EQ(12.0_m, enu2.up());

		EXPECT_EQ(1.0_m, enu3.east());
		EXPECT_EQ(2.0_m, enu3.north());
		EXPECT_EQ(3.0_m, enu3.up());

		// same units
		enu1 += enu2;

		EXPECT_EQ(48.0_m, enu1.east());
		EXPECT_EQ(48.0_m, enu1.north());
		EXPECT_EQ(48.0_m, enu1.up());

		EXPECT_EQ(36.0_m, enu2.east());
		EXPECT_EQ(24.0_m, enu2.north());
		EXPECT_EQ(12.0_m, enu2.up());

		// different units
		enu2 += enu3;
		EXPECT_UNITS_NEAR(-13957.72648624524_m, enu2.east(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(9248.61838027574_m, enu2.north(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-7.01288568046_m, enu2.up(), 5.0e-9_m);

		EXPECT_EQ(1.0_m, enu3.east());
		EXPECT_EQ(2.0_m, enu3.north());
		EXPECT_EQ(3.0_m, enu3.up());
	}

	TEST_F(PositionENUTest, minusEqual)
	{
		ENU enu1(12.0_m, 24.0_m, 36.0_m, Boston);
		ENU enu2(36.0_m, 24.0_m, 12.0_m, Boston);
		ENU enu3(1.0_m, 2.0_m, 3.0_m, Lexington);

		EXPECT_EQ(12.0_m, enu1.east());
		EXPECT_EQ(24.0_m, enu1.north());
		EXPECT_EQ(36.0_m, enu1.up());

		EXPECT_EQ(36.0_m, enu2.east());
		EXPECT_EQ(24.0_m, enu2.north());
		EXPECT_EQ(12.0_m, enu2.up());

		EXPECT_EQ(1.0_m, enu3.east());
		EXPECT_EQ(2.0_m, enu3.north());
		EXPECT_EQ(3.0_m, enu3.up());

		// same units
		enu1 -= enu2;

		EXPECT_EQ(-24.0_m, enu1.east());
		EXPECT_EQ(0.0_m, enu1.north());
		EXPECT_EQ(24.0_m, enu1.up());

		EXPECT_EQ(36.0_m, enu2.east());
		EXPECT_EQ(24.0_m, enu2.north());
		EXPECT_EQ(12.0_m, enu2.up());

		// different units
		enu2 -= enu3;
		EXPECT_UNITS_NEAR(14029.72648624524_m, enu2.east(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(-9200.61838027574_m, enu2.north(), 5.0e-9_m);
		EXPECT_UNITS_NEAR(31.01288568046_m, enu2.up(), 5.0e-9_m);

		EXPECT_EQ(1.0_m, enu3.east());
		EXPECT_EQ(2.0_m, enu3.north());
		EXPECT_EQ(3.0_m, enu3.up());
	}

	TEST_F(PositionENUTest, timesEqual)
	{
		PositionENU<NAD83, feet> enu_ft(1.0_ft, 2.0_ft, 3.0_ft, Boston);

		enu_ft *= 3;
		EXPECT_EQ(3.0_ft, enu_ft.east());
		EXPECT_EQ(6.0_ft, enu_ft.north());
		EXPECT_EQ(9.0_ft, enu_ft.up());
	}

	TEST_F(PositionENUTest, divideEqual)
	{
		PositionENU<NAD83, feet> enu_ft(1.0_ft, 2.0_ft, 3.0_ft, Boston);

		enu_ft /= 3;
		EXPECT_EQ(feet(1.0 / 3), enu_ft.east());
		EXPECT_EQ(feet(2.0 / 3), enu_ft.north());
		EXPECT_EQ(1.0_ft, enu_ft.up());
	}
}


#endif // positionENUTest_h__