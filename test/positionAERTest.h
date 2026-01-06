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

using namespace coord;
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

		virtual void SetUp()
		{
			std::setprecision(15);

			// Code here will be called immediately after the constructor (right
			// before each test).
			Boston.setPoint(42_deg, -71_deg, 0_m);
			Boston_ecef = Boston;

			LA.setPoint(34_deg, -118_deg, 0_m);
			LA_ecef = LA;
		}

		virtual void TearDown()
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
		// values are from MATLAB
		EXPECT_NEAR(-2.485034262789235e+06, LA_ecef.x().to<double>(),5e-9);
		EXPECT_NEAR(-4.673669705320125e+06, LA_ecef.y().to<double>(),5e-9);
		EXPECT_NEAR(3.546446563780690e+06, LA_ecef.z().to<double>(),5e-9);

		EXPECT_NEAR(1.545471693315307e+06, Boston_ecef.x().to<double>(), 5e-9);
		EXPECT_NEAR(-4.488375702866388e+06, Boston_ecef.y().to<double>(), 5e-9);
		EXPECT_NEAR(4.245603836101115e+06, Boston_ecef.z().to<double>(), 5e-9);
	}

	TEST_F(PositionAERTest, is_point)
	{
		EXPECT_TRUE(coord::traits::is_point<AER>::value);
	}

	TEST_F(PositionAERTest, defaultConstructor)
	{
		EXPECT_TRUE(std::is_default_constructible<AER>::value);
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

	TEST_F(PositionAERTest, tupleConstructor)
	{
		coord::sphericalTuple tup(5_deg, 10_deg, 15_km);

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
		EXPECT_TRUE(coord::traits::is_point<decltype(aer)>::value);
		EXPECT_TRUE(coord::traits::is_point<decltype(aer2)>::value);
		EXPECT_TRUE(aer == aer2);
	}

	TEST_F(PositionAERTest, implicitConversionConstructor_fromECEF)
	{
		AER aer(LA_ecef, LA);
		EXPECT_EQ(0_deg, std::get<0>(aer.point()));
		EXPECT_EQ(0_deg, std::get<1>(aer.point()));
		EXPECT_EQ(0_m, std::get<2>(aer.point()));
		EXPECT_TRUE(LA.isSame(LA_ecef));
		EXPECT_TRUE(LA_ecef.isSame(LA));

		AER aer2(Boston_ecef, Boston);
		EXPECT_EQ(0_deg, std::get<0>(aer2.point()));
		EXPECT_EQ(0_deg, std::get<1>(aer2.point()));
		EXPECT_EQ(0_m, std::get<2>(aer2.point()));
		EXPECT_TRUE(Boston.isSame(Boston_ecef));
		EXPECT_TRUE(Boston_ecef.isSame(Boston));

		AER aer3(LA_ecef, Boston);
		EXPECT_NEAR(273.5656795360105, std::get<0>(aer3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.698699777761998, std::get<1>(aer3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(4094891.087804174, std::get<2>(aer3.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionAERTest, implicitConversionConstructor_fromGeo)
	{
		AER aer(Boston, Boston);
		EXPECT_EQ(0_deg, std::get<0>(aer.point()));
		EXPECT_EQ(0_deg, std::get<1>(aer.point()));
		EXPECT_EQ(0_m, std::get<2>(aer.point()));

		AER aer2(Boston, LA);
		EXPECT_NEAR(63.522735932827878, std::get<0>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.707489622725706, std::get<1>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(4094891.087804174, std::get<2>(aer2.point()).to<double>(), 5.0e-9);

		AER aer3(LA, Boston);
		EXPECT_NEAR(273.5656795360105, std::get<0>(aer3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.698699777761998, std::get<1>(aer3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(4094891.087804174, std::get<2>(aer3.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionAERTest, implicitConversionConstructor_fromENU)
	{
		// same origin
		ENU enu(1_m, 1_m, 1_m, LA);

		// should just result in a copy
		AER aer(enu, LA, 2016_yr);
		EXPECT_NEAR((44.999999922463218_deg).to<double>(), std::get<0>(aer.point()).to<double>(), 5.0e-8);
		EXPECT_NEAR((35.264389654762070_deg).to<double>(), std::get<1>(aer.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR((1.732050806818667_m).to<double>(), std::get<2>(aer.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(LA.point(), aer.frameData().origin);

		// different origin
		AER aer2(enu, Boston);
		EXPECT_NEAR(273.5656861607163, std::get<0>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.698680503461308, std::get<1>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(4094890.138432032, std::get<2>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Boston.point(), aer2.frameData().origin);
	}

	TEST_F(PositionAERTest, implicitConversionConstructor_fromNED)
	{
		// same origin
		NED ned(100_m, 100_m, 100_m, LA);

		// should just result in a copy
		AER aer(ned, LA, 2016_yr);
		EXPECT_NEAR((45.0_deg).to<double>(), std::get<0>(aer.point()).to<double>(), 5.0e-8);
		EXPECT_NEAR((-35.264389682754654_deg).to<double>(), std::get<1>(aer.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR((173.2050807568877_m).to<double>(), std::get<2>(aer.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(LA.point(), aer.frameData().origin);

		// different origin
		AER aer2(ned, Boston);
		EXPECT_NEAR(273.5663436522812, std::get<0>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.699422933503001, std::get<1>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(4094732.003776215, std::get<2>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Boston.point(), aer2.frameData().origin);
	}

	TEST_F(PositionAERTest, implicitConversionConstructor_fromAER)
	{
		// same origin
		AER aer(45_deg, 35_deg, 2_m, LA);

		// should just result in a copy
		AER aer2(aer);
		EXPECT_NEAR((45_deg).to<double>(), std::get<0>(aer2.point()).to<double>(), 5.0e-8);
		EXPECT_NEAR((35_deg).to<double>(), std::get<1>(aer2.point()).to<double>(), 5.1e-9);
		EXPECT_NEAR((2_m).to<double>(), std::get<2>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(LA.point(), aer2.frameData().origin);

		// same origin	
		AER aer3(aer, LA);
		EXPECT_NEAR((45_deg).to<double>(), std::get<0>(aer3.point()).to<double>(), 5.0e-8);
		EXPECT_NEAR((35_deg).to<double>(), std::get<1>(aer3.point()).to<double>(), 5.0e-8);
		EXPECT_NEAR((2_m).to<double>(), std::get<2>(aer3.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(LA.point(), aer3.frameData().origin);

		// different origin
		AER aer4(aer, Boston);
		EXPECT_NEAR((273.5656872105317_deg).to<double>(), std::get<0>(aer4.point()).to<double>(), 5.0e-8);
		EXPECT_NEAR((-18.698677599130299_deg).to<double>(), std::get<1>(aer4.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR((4094889.984373124_m).to<double>(), std::get<2>(aer4.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Boston.point(), aer4.frameData().origin);
	}

	TEST_F(PositionAERTest, copyAssignment)
	{
		AER aer(1_deg, 2_deg, 3_m, LA);
		EXPECT_EQ(1_deg, std::get<0>(aer.point()));
		EXPECT_EQ(2_deg, std::get<1>(aer.point()));
		EXPECT_EQ(3_m, std::get<2>(aer.point()));

		AER aer2 = aer;
		EXPECT_TRUE(coord::traits::is_point<decltype(aer)>::value);
		EXPECT_TRUE(coord::traits::is_point<decltype(aer2)>::value);
		EXPECT_TRUE(aer == aer2);
	}

	TEST_F(PositionAERTest, implicitConversionAssignment_fromECEF)
	{
		AER aer(LA_ecef, LA);
		EXPECT_EQ(0_deg, std::get<0>(aer.point()));
		EXPECT_EQ(0_deg, std::get<1>(aer.point()));
		EXPECT_EQ(0_m, std::get<2>(aer.point()));

		AER aer2(Boston);
		aer2 = aer;
		EXPECT_NEAR(273.5656795360105, std::get<0>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.698699777761998, std::get<1>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(4094891.087804174, std::get<2>(aer2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionAERTest, implicitConversionAssignment_fromGeo)
	{
		AER aer(Boston, Boston);
		EXPECT_EQ(0_deg, std::get<0>(aer.point()));
		EXPECT_EQ(0_deg, std::get<1>(aer.point()));
		EXPECT_EQ(0_m, std::get<2>(aer.point()));

		AER aer2(LA);
		aer2 = Boston;
		EXPECT_NEAR(63.522735932827878, std::get<0>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.707489622725706, std::get<1>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(4094891.087804174, std::get<2>(aer2.point()).to<double>(), 5.0e-9);

		AER aer3(Boston);
		aer3 = LA;
		EXPECT_NEAR(273.5656795360105, std::get<0>(aer3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.698699777761998, std::get<1>(aer3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(4094891.087804174, std::get<2>(aer3.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionAERTest, implicitConversionAssignment_fromENU)
	{
		// same origin
		ENU enu(1_m, 1_m, 1_m, LA);

		// should just result in a copy
		AER aer(LA, 2016_yr);
		aer = enu;
		EXPECT_NEAR((45.0_deg).to<double>(), std::get<0>(aer.point()).to<double>(), 5.4e-8);
		EXPECT_NEAR((35.264389682754654_deg).to<double>(), std::get<1>(aer.point()).to<double>(), 5.0e-8);
		EXPECT_NEAR((1.732050807568877_m).to<double>(), std::get<2>(aer.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(LA.point(), aer.frameData().origin);

		// different origin
		AER aer2(Boston);
		aer2 = enu;
		EXPECT_NEAR(273.5656861607163, std::get<0>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.698680503461308, std::get<1>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(4094890.138432032, std::get<2>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Boston.point(), aer2.frameData().origin);
	}

	TEST_F(PositionAERTest, implicitConversionAssignment_fromNED)
	{
		// same origin
		NED ned(100_m, 100_m, 100_m, LA);

		// should just result in a copy
		AER aer(LA, 2016_yr);
		aer = ned;
		EXPECT_NEAR((45.0_deg).to<double>(), std::get<0>(aer.point()).to<double>(), 5.0e-8);
		EXPECT_NEAR((-35.264389682754654_deg).to<double>(), std::get<1>(aer.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR((173.2050807568877_m).to<double>(), std::get<2>(aer.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(LA.point(), aer.frameData().origin);

		// different origin
		AER aer2(Boston);
		aer2 = ned;
		EXPECT_NEAR(273.5663436522812, std::get<0>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.699422933503001, std::get<1>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(4094732.003776215, std::get<2>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Boston.point(), aer2.frameData().origin);
	}

	TEST_F(PositionAERTest, implicitConversionAssignment_fromAER)
	{
		// same origin
		AER aer(45_deg, 35_deg, 2_m, LA);

		// same origin
		// should just result in a copy
		AER aer2(LA);
		aer2 = aer;
		EXPECT_NEAR((45_deg).to<double>(), std::get<0>(aer2.point()).to<double>(), 5.0e-8);
		EXPECT_NEAR((35_deg).to<double>(), std::get<1>(aer2.point()).to<double>(), 5.0e-8);
		EXPECT_NEAR((2_m).to<double>(), std::get<2>(aer2.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(LA.point(), aer2.frameData().origin);

		// different origin
		AER aer3(Boston);
		aer3 = aer;
		EXPECT_NEAR((273.5656872105317_deg).to<double>(), std::get<0>(aer3.point()).to<double>(), 5.0e-8);
		EXPECT_NEAR((-18.698677599130299_deg).to<double>(), std::get<1>(aer3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR((4094889.984373124_m).to<double>(), std::get<2>(aer3.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Boston.point(), aer3.frameData().origin);
	}

	TEST_F(PositionAERTest, isSame)
	{
		using AER_ft = PositionAER<datums::WGS84_G1674, units::length::feet>;
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
		std::setprecision(16);

		AER aerb0(1_deg, 1_deg, 1_m, Boston);
		AER aerb1(2_deg, 2_deg, 2_m, Boston);

		AER aerl0(1_deg, 1_deg, 1_m, LA);
		AER aerl1(2_deg, 2_deg, 2_m, LA);

		// same origin
		EXPECT_NEAR((1.000608802301425_m).to<double>(), aerb0.distance(aerb1).to<double>(), 5.0e-10);
		EXPECT_NEAR((1.000608802301425_m).to<double>(), aerb1.distance(aerb0).to<double>(), 5.0e-10);
		EXPECT_NEAR((1.000608802301425_m).to<double>(), aerl0.distance(aerl1).to<double>(), 5.0e-10);
		EXPECT_NEAR((1.000608802301425_m).to<double>(), aerl1.distance(aerl0).to<double>(), 5.0e-10);

		// different point types
		EXPECT_NEAR((2_m).to<double>(), aerb1.distance(Boston).to<double>(), 5.0e-10);
		EXPECT_NEAR((2_m).to<double>(), aerl1.distance(LA).to<double>(), 5.0e-10);

		//different origin
		EXPECT_NEAR((4094890.619647854_m).to<double>(), aerb0.distance(aerl0).to<double>(), 5.0e-9);
		EXPECT_NEAR((4094891.051004039_m).to<double>(), aerb0.distance(LA).to<double>(), 5.0e-9);
	}

	TEST_F(PositionAERTest, azimuth)
	{
		AER aer;
		EXPECT_EQ(0_deg, aer.azimuth());

		AER aer1(5_deg, 6_deg, 7_km, LLA());

		EXPECT_EQ(5_deg, aer1.azimuth());

		AER aer2(radian_t(2), radian_t(3), 4_km, LLA());

		EXPECT_EQ(radian_t(2), aer2.azimuth());
	}

	TEST_F(PositionAERTest, elevation)
	{
		AER aer;
		EXPECT_EQ(0_deg, aer.elevation());

		AER aer1(5_deg, 6_deg, 7_km, LLA());
 
		EXPECT_EQ(6_deg, aer1.elevation());

		AER aer2(radian_t(2), radian_t(3), 4_km, LLA());
 
 		EXPECT_EQ(radian_t(3), aer2.elevation());
	}

	TEST_F(PositionAERTest, range)
	{
		AER aer;
		EXPECT_EQ(0_m, aer.range());

		AER aer1(5_deg, 6_deg, 7_km, LLA());
 
		EXPECT_EQ(7_km, aer1.range());

		AER aer2(radian_t(2), radian_t(3), 4_km, LLA());
 
		EXPECT_EQ(4_km, aer2.range());
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
		EXPECT_EQ(0_deg, aer.azimuth());

		aer.setAzimuth(5_deg);
		EXPECT_EQ(5_deg, aer.azimuth());
		EXPECT_EQ(0_deg, aer.elevation());
		EXPECT_EQ(0_m, aer.range());

		aer.setAzimuth(radian_t(5));
		EXPECT_NEAR(286.4788975654116, aer.azimuth().to<double>(), 5.0e-9);
		EXPECT_EQ(0_deg, aer.elevation());
		EXPECT_EQ(0_m, aer.range());
	}
	
	TEST_F(PositionAERTest, setElevation)
	{
		AER aer;
		EXPECT_EQ(0_deg, aer.elevation());

		aer.setElevation(6_deg);
		EXPECT_EQ(0_deg, aer.azimuth());
		EXPECT_EQ(6_deg, aer.elevation());
		EXPECT_EQ(0_m, aer.range());

		aer.setElevation(radian_t(6));
		EXPECT_EQ(0_deg, aer.azimuth());
		EXPECT_NEAR(343.7746770784939, aer.elevation().to<double>(), 5.0e-9);
		EXPECT_EQ(0_m, aer.range());
	}

	TEST_F(PositionAERTest, setRange)
	{
		AER aer;
		EXPECT_EQ(0_m, aer.range());

		aer.setRange(7_m);
		EXPECT_EQ(0_deg, aer.azimuth());
		EXPECT_EQ(0_deg, aer.elevation());
		EXPECT_EQ(7_m, aer.range());

		aer.setRange(7_mm);
		EXPECT_EQ(0_deg, aer.azimuth());
		EXPECT_EQ(0_deg, aer.elevation());
		EXPECT_EQ(0.007_m, aer.range());
	}

	TEST_F(PositionAERTest, setOrigin)
	{
		AER aer;
		EXPECT_TRUE(LLA(0_deg, 0_deg, 0_m) == aer.origin());

		AER aer2(5_deg, 10_deg, 15_km, Boston);
 
 		EXPECT_TRUE(Boston == aer2.origin());

		AER aer3(aer2);
		aer3.setOrigin(LA);
		EXPECT_NEAR(63.304670032255295, aer3.azimuth().to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.674629637629703, aer3.elevation().to<double>(), 5.0e-9);
		EXPECT_NEAR(4096103.677420084, aer3.range().to<double>(), 5.0e-8);
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