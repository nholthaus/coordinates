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

#ifndef ECEFTest_h__
#define ECEFTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <stdexcept>

using namespace coordinates;
using namespace units;
using namespace units::literals;
using namespace units::length;
using namespace units::angle;
using namespace units::time;

namespace
{
	// The fixture for testing class Foo.
	class PositionECEFTest : public ::testing::Test {
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		PositionECEFTest()
		{
			// You can do set-up work for each test here.
		}

		~PositionECEFTest() override
		{
			// You can do clean-up work that doesn't throw exceptions here.
		}

		// If the constructor and destructor are not enough for setting up
		// and cleaning up each test, you can define the following methods:

		void SetUp() override
		{
			// Code here will be called immediately after the constructor (right
			// before each test).
		}

		void TearDown() override
		{
			// Code here will be called immediately after each test (right
			// before the destructor).
		}

		// Objects declared here can be used by all tests in the test case for Foo.
	};
	TEST_F(PositionECEFTest, is_point)
	{
		EXPECT_TRUE(coordinates::traits::is_point<ECEF>);
		EXPECT_TRUE((coordinates::traits::is_point<PositionECEF<datums::ITRS2000, cubits, long double>>));
	}

	TEST_F(PositionECEFTest, defaultConstructor)
	{
		ECEF ecef;
		EXPECT_EQ(0.0_m, std::get<0>(ecef.point()));
		EXPECT_EQ(0.0_m, std::get<1>(ecef.point()));
		EXPECT_EQ(0.0_m, std::get<2>(ecef.point()));
		EXPECT_EQ(FrameData(2005.0_yr), ecef.frameData());
	}

	// TESTS THE  MEMBER
	TEST_F(PositionECEFTest, constructor)
	{
		ECEF ecef(1.0_m, 2.0_m, 3.0_m);
		EXPECT_EQ(1.0_m, std::get<0>(ecef.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ecef.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ecef.point()));
		EXPECT_EQ(FrameData(2005.0_yr), ecef.frameData());

		// with observation date
		ECEF ecef2(1.0_m, 2.0_m, 3.0_m, 2016.413_yr);
		EXPECT_EQ(1.0_m, std::get<0>(ecef2.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ecef2.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ecef2.point()));
		EXPECT_EQ(FrameData(2016.413_yr), ecef2.frameData());
	}

	// TESTS THAT MULTIPLE INSTANTIATIONS OF THE TEMPLATE WITH DIFFERENT UNITS WILL WORK
	TEST_F(PositionECEFTest, constructorWithDifferentUnits)
	{

		// Different input units
		ECEF ecef_m(0.004_km, 0.005_km, 0.006_km);
		EXPECT_EQ(4.0_m, std::get<0>(ecef_m.point()));
		EXPECT_EQ(5.0_m, std::get<1>(ecef_m.point()));
		EXPECT_EQ(6.0_m, std::get<2>(ecef_m.point()));

		// different storage units
		PositionECEF<datums::NAD83, kilometers> ecef_km(1.0_km, 2.0_km, 3.0_km);
		EXPECT_EQ(1000.0_m, std::get<0>(ecef_km.point()));
		EXPECT_EQ(2000.0_m, std::get<1>(ecef_km.point()));
		EXPECT_EQ(3000.0_m, std::get<2>(ecef_km.point()));

		// different input AND storage units
		PositionECEF<datums::NAD83, kilometers> ecef_km2(1.0_m, 2.0_m, 3.0_m);
		EXPECT_EQ(0.001_km, std::get<0>(ecef_km2.point()));
		EXPECT_EQ(0.002_km, std::get<1>(ecef_km2.point()));
		EXPECT_EQ(0.003_km, std::get<2>(ecef_km2.point()));
	}

	// TESTS THE  MEMBER
	TEST_F(PositionECEFTest, tupleConstructor)
	{
		coordinates::CartesianTuple t(1.0_m, 2.0_m, 3.0_m);
		ECEF ecef(t);
		EXPECT_EQ(1.0_m, std::get<0>(ecef.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ecef.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ecef.point()));
		EXPECT_EQ(FrameData(2005.0_yr), ecef.frameData());

		// different observation date
		ECEF ecef2(t, 2016.413_yr);
		EXPECT_EQ(1.0_m, std::get<0>(ecef2.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ecef2.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ecef2.point()));
		EXPECT_EQ(FrameData(2016.413_yr), ecef2.frameData());
	}

	TEST_F(PositionECEFTest, copyConstructor)
	{
		ECEF ecef(1.0_m, 2.0_m, 3.0_m, 2016.413_yr);
		EXPECT_EQ(1.0_m, std::get<0>(ecef.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ecef.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ecef.point()));
		EXPECT_EQ(FrameData(2016.413_yr), ecef.frameData());

		ECEF ecef2(ecef);
		EXPECT_EQ(1.0_m, std::get<0>(ecef2.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ecef2.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ecef2.point()));
		EXPECT_EQ(FrameData(2016.413_yr), ecef2.frameData());
	}

	TEST_F(PositionECEFTest, implicitConversionConstructor)
	{
		// source: http://webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php?locale=en (disable 'interpolate velocities')

		// Implicit datum transformation
		PositionECEF<datums::ITRS2000> itrs2000(1532138.956_m, -4464558.719_m, 4275244.397_m);
		PositionECEF<datums::ITRS88> itrs88Expected(1532138.996_m, -4464558.756_m, 4275244.319_m);

		PositionECEF<datums::ITRS88> itrs88(itrs2000);
		EXPECT_UNITS_NEAR(std::get<0>(itrs88Expected.point()), std::get<0>(itrs88.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<1>(itrs88Expected.point()), std::get<1>(itrs88.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<2>(itrs88Expected.point()), std::get<2>(itrs88.point()), 1.0e-3_m);
		EXPECT_EQ(FrameData(2000.0_yr), itrs88.frameData());

		// implicit datum transformation @ epoch 2014
		PositionECEF<datums::ITRS2000> itrs2000_2014(1532138.956_m, -4464558.719_m, 4275244.397_m, 2014.0_yr);
		PositionECEF<datums::ITRS2008> itrs2008_2014_expected(1532138.953_m, -4464558.708_m, 4275244.422_m);

		PositionECEF<datums::ITRS2008> itrs2008_2014(itrs2000_2014);
		EXPECT_UNITS_NEAR(std::get<0>(itrs2008_2014_expected.point()), std::get<0>(itrs2008_2014.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<1>(itrs2008_2014_expected.point()), std::get<1>(itrs2008_2014.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<2>(itrs2008_2014_expected.point()), std::get<2>(itrs2008_2014.point()), 1.0e-3_m);
		EXPECT_EQ(FrameData(2014.0_yr), itrs2008_2014.frameData());

		// implicit datum transformation @ epoch 2014
		PositionECEF<datums::ITRS88> itrs88_2014_expected(1532139.002_m, -4464558.763_m, 4275244.300_m);

		PositionECEF<datums::ITRS88> itrs88_2014(itrs2000_2014);
		EXPECT_UNITS_NEAR(std::get<0>(itrs88_2014_expected.point()), std::get<0>(itrs88_2014.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<1>(itrs88_2014_expected.point()), std::get<1>(itrs88_2014.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<2>(itrs88_2014_expected.point()), std::get<2>(itrs88_2014.point()), 1.0e-3_m);
		EXPECT_EQ(FrameData(2014.0_yr), itrs2008_2014.frameData());
	}

	TEST_F(PositionECEFTest, assignment)
	{
		ECEF ecef(1.0_m, 2.0_m, 3.0_m, 2016.413_yr);
		EXPECT_EQ(1.0_m, std::get<0>(ecef.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ecef.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ecef.point()));
		EXPECT_EQ(FrameData(2016.413_yr), ecef.frameData());

		ECEF ecef2 = ecef;
		EXPECT_EQ(1.0_m, std::get<0>(ecef2.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ecef2.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ecef2.point()));
		EXPECT_EQ(FrameData(2016.413_yr), ecef2.frameData());
	}

	TEST_F(PositionECEFTest, implicitConversionAssignment)
	{
		// source: http://webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php?locale=en (disable 'interpolate velocities')

		// Implicit datum transformation
		PositionECEF<datums::ITRS2000> itrs2000(1532138.956_m, -4464558.719_m, 4275244.397_m);
		PositionECEF<datums::ITRS88> itrs88Expected(1532138.996_m, -4464558.756_m, 4275244.319_m);

		PositionECEF<datums::ITRS88> itrs88 = itrs2000;
		EXPECT_UNITS_NEAR(std::get<0>(itrs88Expected.point()), std::get<0>(itrs88.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<1>(itrs88Expected.point()), std::get<1>(itrs88.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<2>(itrs88Expected.point()), std::get<2>(itrs88.point()), 1.0e-3_m);
		EXPECT_EQ(FrameData(2000.0_yr), itrs88.frameData());

		// implicit datum transformation @ epoch 2014
		PositionECEF<datums::ITRS2000> itrs2000_2014(1532138.956_m, -4464558.719_m, 4275244.397_m, 2014.0_yr);
		PositionECEF<datums::ITRS2008> itrs2008_2014_expected(1532138.953_m, -4464558.708_m, 4275244.422_m);

		PositionECEF<datums::ITRS2008> itrs2008_2014 = itrs2000_2014;
		EXPECT_UNITS_NEAR(std::get<0>(itrs2008_2014_expected.point()), std::get<0>(itrs2008_2014.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<1>(itrs2008_2014_expected.point()), std::get<1>(itrs2008_2014.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<2>(itrs2008_2014_expected.point()), std::get<2>(itrs2008_2014.point()), 1.0e-3_m);
		EXPECT_EQ(FrameData(2014.0_yr), itrs2008_2014.frameData());

		// implicit datum transformation @ epoch 2014
		PositionECEF<datums::ITRS88> itrs88_2014_expected(1532139.002_m, -4464558.763_m, 4275244.300_m);

		PositionECEF<datums::ITRS88> itrs88_2014 = itrs2000_2014;
		EXPECT_UNITS_NEAR(std::get<0>(itrs88_2014_expected.point()), std::get<0>(itrs88_2014.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<1>(itrs88_2014_expected.point()), std::get<1>(itrs88_2014.point()), 1.0e-3_m);
		EXPECT_UNITS_NEAR(std::get<2>(itrs88_2014_expected.point()), std::get<2>(itrs88_2014.point()), 1.0e-3_m);
		EXPECT_EQ(FrameData(2014.0_yr), itrs2008_2014.frameData());
	}

	TEST_F(PositionECEFTest, isNull)
	{
		ECEF ecef;

		EXPECT_EQ(0.0_m, std::get<0>(ecef.point()));
		EXPECT_EQ(0.0_m, std::get<1>(ecef.point()));
		EXPECT_EQ(0.0_m, std::get<2>(ecef.point()));

		EXPECT_TRUE(ecef.isNull());

		ECEF ecef2(1.0_m, 2.0_m, 3.0_m);

		EXPECT_EQ(1.0_m, std::get<0>(ecef2.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ecef2.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ecef2.point()));

		EXPECT_FALSE(ecef2.isNull());
	}

	TEST_F(PositionECEFTest, isSame)
	{
		using ECEF_ft = PositionECEF<datums::WGS84_G1674, feet>;
		using ECEF_nm = PositionECEF<datums::WGS84_G1674, nanometers>;

		ECEF ecef(1.0_m, 2.0_m, 3.0_m);
		ECEF exactlySame(1.0_m, 2.0_m, 3.0_m);
		ECEF close(1.001_m, 2.002_m, 3.003_m);
		ECEF_ft closeDifferentUnits(3.28083989_ft, 6.56167979_ft, 9.84251968_ft);
		ECEF_nm nanometerTolerance(1_nm, 1_nm, 1_nm);

		EXPECT_EQ(1.0_m, std::get<0>(ecef.point()));
		EXPECT_EQ(2.0_m, std::get<1>(ecef.point()));
		EXPECT_EQ(3.0_m, std::get<2>(ecef.point()));

		// exactly same, default tolerance
		EXPECT_TRUE(ecef.isSame(exactlySame));

		// close, single tolerance
		EXPECT_TRUE(ecef.isSame(close, 0.005_m));

		// close, point tolerance
		EXPECT_TRUE(ecef.isSame(close, ECEF(0.0015_m, 0.0025_m, 0.0035_m)));

		// close, not within tolerance
		EXPECT_FALSE(ecef.isSame(close, 0.0005_m));

		// close, not within point tolerance
		EXPECT_FALSE(ecef.isSame(close, ECEF(0.0009_m, 0.0019_m, 0.0029_m)));

		// close, different units, single tolerance
		EXPECT_TRUE(ecef.isSame(closeDifferentUnits, 50_nm));

		// close, different units, point tolerance
		EXPECT_TRUE(ecef.isSame(closeDifferentUnits, ECEF(0.0015_m, 0.0025_m, 0.0035_m)));

		// close, different units, not within tolerance
		EXPECT_FALSE(ecef.isSame(closeDifferentUnits, 1_nm));

		// close, different units, not within point tolerance
		EXPECT_FALSE(ecef.isSame(closeDifferentUnits, ECEF(1_nm, 1_nm, 1_nm)));

		// close, different units, not within point tolerance which has different units
		EXPECT_FALSE(ecef.isSame(closeDifferentUnits, nanometerTolerance));
	}

	TEST_F(PositionECEFTest, distance)
	{
		ECEF ecef1(1.0_m, 1.0_m, 1.0_m);
		ECEF ecef2(2.0_m, 2.0_m, 2.0_m);

		EXPECT_EQ(meters(sqrt(3.0)), ecef1.distance(ecef2));
	}

	TEST_F(PositionECEFTest, magnitude)
	{
		ECEF ecef(2.0_m, 2.0_m, 2.0_m);

		EXPECT_EQ(meters(2 * sqrt(3.0)), ecef.magnitude());
	}

	TEST_F(PositionECEFTest, dotProduct)
	{
		ECEF ecef1(1.0_m, -1.0_m, 3.0_m);
		ECEF ecef2(3.0_m, 3.0_m, 0.0_m);

		EXPECT_EQ(0.0_m2, ecef1.dotProduct(ecef2));
	}

	TEST_F(PositionECEFTest, x)
	{
		ECEF ecef;
		EXPECT_EQ(0.0_m, ecef.x());

		ECEF ecef1(5.0_m, 6.0_m, 7.0_m);
		EXPECT_EQ(5.0_m, ecef1.x());

		ECEF ecef2(5_mm, 6_mm, 7_mm);
		EXPECT_EQ(5_mm, ecef2.x());
	}

	TEST_F(PositionECEFTest, y)
	{
		ECEF ecef;
		EXPECT_EQ(0.0_m, ecef.y());

		ECEF ecef1(5.0_m, 6.0_m, 7.0_m);
		EXPECT_EQ(6.0_m, ecef1.y());

		ECEF ecef2(5_mm, 6_mm, 7_mm);
		EXPECT_EQ(6_mm, ecef2.y());
	}

	TEST_F(PositionECEFTest, z)
	{
		ECEF ecef;
		EXPECT_EQ(0.0_m, ecef.z());

		ECEF ecef1(5.0_m, 6.0_m, 7.0_m);
		EXPECT_EQ(7.0_m, ecef1.z());

		ECEF ecef2(5_mm, 6_mm, 7_mm);
		EXPECT_EQ(7_mm, ecef2.z());
	}

	TEST_F(PositionECEFTest, date)
	{
		ECEF ecef;
		EXPECT_EQ(2005.0_yr, ecef.date());

		ECEF ecef2(0.0_m, 0.0_m, 0.0_m, 2016.421_yr);
		EXPECT_EQ(2016.421_yr, ecef2.date());
	}

	TEST_F(PositionECEFTest, setX)
	{
		ECEF ecef;
		EXPECT_EQ(0.0_m, ecef.x());

		ecef.setX(5.0_m);
		EXPECT_EQ(5.0_m, ecef.x());
		EXPECT_EQ(0.0_m, ecef.y());
		EXPECT_EQ(0.0_m, ecef.z());

		ecef.setX(5_mm);
		EXPECT_EQ(0.005_m, ecef.x());
		EXPECT_EQ(0.0_m, ecef.y());
		EXPECT_EQ(0.0_m, ecef.z());
	}

	TEST_F(PositionECEFTest, setY)
	{
		ECEF ecef;
		EXPECT_EQ(0.0_m, ecef.y());

		ecef.setY(6.0_m);
		EXPECT_EQ(0.0_m, ecef.x());
		EXPECT_EQ(6.0_m, ecef.y());
		EXPECT_EQ(0.0_m, ecef.z());

		ecef.setY(6_mm);
		EXPECT_EQ(0.0_m, ecef.x());
		EXPECT_EQ(0.006_m, ecef.y());
		EXPECT_EQ(0.0_m, ecef.z());
	}

	TEST_F(PositionECEFTest, setZ)
	{
		ECEF ecef;
		EXPECT_EQ(0.0_m, ecef.z());

		ecef.setZ(7.0_m);
		EXPECT_EQ(0.0_m, ecef.x());
		EXPECT_EQ(0.0_m, ecef.y());
		EXPECT_EQ(7.0_m, ecef.z());

		ecef.setZ(7_mm);
		EXPECT_EQ(0.0_m, ecef.x());
		EXPECT_EQ(0.0_m, ecef.y());
		EXPECT_EQ(0.007_m, ecef.z());
	}

	TEST_F(PointTest, ostream)
	{
		PositionECEF<datums::NAD83, inches> ecef1(12.1_in, 24.2_in, 36.3_in);

		EXPECT_EQ(12.1_in, ecef1.x());
		EXPECT_EQ(24.2_in, ecef1.y());
		EXPECT_EQ(36.3_in, ecef1.z());

		testing::internal::CaptureStdout();
		std::cout << ecef1;
		std::string output = testing::internal::GetCapturedStdout();

		EXPECT_STREQ("(12.1 in, 24.2 in, 36.3 in)", output.c_str());
	}

	TEST_F(PositionECEFTest, plusEqual)
	{
		PositionECEF<datums::NAD83, inches> ecef1(12_in, 24_in, 36_in);
		PositionECEF<datums::NAD83, inches> ecef2(36_in, 24_in, 12_in);
		PositionECEF<datums::NAD83, feet> ecef_ft(1.0_ft, 2.0_ft, 3.0_ft);

		EXPECT_EQ(12_in, ecef1.x());
		EXPECT_EQ(24_in, ecef1.y());
		EXPECT_EQ(36_in, ecef1.z());

		EXPECT_EQ(36_in, ecef2.x());
		EXPECT_EQ(24_in, ecef2.y());
		EXPECT_EQ(12_in, ecef2.z());

		EXPECT_EQ(1.0_ft, ecef_ft.x());
		EXPECT_EQ(2.0_ft, ecef_ft.y());
		EXPECT_EQ(3.0_ft, ecef_ft.z());

		// same units
		ecef1 += ecef2;

		EXPECT_EQ(48_in, ecef1.x());
		EXPECT_EQ(48_in, ecef1.y());
		EXPECT_EQ(48_in, ecef1.z());

		EXPECT_EQ(36_in, ecef2.x());
		EXPECT_EQ(24_in, ecef2.y());
		EXPECT_EQ(12_in, ecef2.z());

		// different units
		ecef2 += ecef_ft;
		EXPECT_EQ(48_in, ecef2.x());
		EXPECT_EQ(48_in, ecef2.y());
		EXPECT_EQ(48_in, ecef2.z());

		EXPECT_EQ(1.0_ft, ecef_ft.x());
		EXPECT_EQ(2.0_ft, ecef_ft.y());
		EXPECT_EQ(3.0_ft, ecef_ft.z());
	}

	TEST_F(PositionECEFTest, minusEqual)
	{
		PositionECEF<datums::NAD83, inches> zero;
		PositionECEF<datums::NAD83, inches> ecef1(12_in, 24_in, 36_in);
		PositionECEF<datums::NAD83, inches> ecef2(36_in, 24_in, 12_in);
		PositionECEF<datums::NAD83, feet> ecef_ft(1.0_ft, 2.0_ft, 3.0_ft);

		EXPECT_EQ(12_in, ecef1.x());
		EXPECT_EQ(24_in, ecef1.y());
		EXPECT_EQ(36_in, ecef1.z());

		EXPECT_EQ(36_in, ecef2.x());
		EXPECT_EQ(24_in, ecef2.y());
		EXPECT_EQ(12_in, ecef2.z());

		EXPECT_EQ(1.0_ft, ecef_ft.x());
		EXPECT_EQ(2.0_ft, ecef_ft.y());
		EXPECT_EQ(3.0_ft, ecef_ft.z());

		// same units
		ecef1 -= ecef2;

		EXPECT_EQ(-24_in, ecef1.x());
		EXPECT_UNITS_NEAR((0_in), ecef1.y(), 5.0e-15_m);
		EXPECT_EQ(24_in, ecef1.z());

		EXPECT_EQ(36_in, ecef2.x());
		EXPECT_EQ(24_in, ecef2.y());
		EXPECT_EQ(12_in, ecef2.z());

		// different units
		ecef2 -= ecef_ft;
		EXPECT_EQ(24_in, ecef2.x());
		EXPECT_UNITS_NEAR((0_in), ecef2.y(), 5.0e-15_m);
		EXPECT_EQ(-24_in, ecef2.z());

		EXPECT_EQ(1.0_ft, ecef_ft.x());
		EXPECT_EQ(2.0_ft, ecef_ft.y());
		EXPECT_EQ(3.0_ft, ecef_ft.z());
	}

	TEST_F(PositionECEFTest, timesEqual)
	{
		PositionECEF<datums::NAD83, feet> ecef_ft(1.0_ft, 2.0_ft, 3.0_ft);

		ecef_ft *= 3;
		EXPECT_EQ(3.0_ft, ecef_ft.x());
		EXPECT_EQ(6.0_ft, ecef_ft.y());
		EXPECT_EQ(9.0_ft, ecef_ft.z());
	}

	TEST_F(PositionECEFTest, divideEqual)
	{
		PositionECEF<datums::NAD83, feet> ecef_ft(1.0_ft, 2.0_ft, 3.0_ft);

		ecef_ft /= 3;
		EXPECT_EQ(feet(1.0/3), ecef_ft.x());
		EXPECT_EQ(feet(2.0/3), ecef_ft.y());
		EXPECT_EQ(1.0_ft, ecef_ft.z());
	}
}
#endif // ECEFTest_h__