#ifndef positionNEDTest_h__
#define positionNEDTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <initializer_list>
#include <stdexcept>
#include <type_traits>

using namespace coord;
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

		virtual ~PositionNEDTest()
		{
			// You can do clean-up work that doesn't throw exceptions here.
		}

		// If the constructor and destructor are not enough for setting up
		// and cleaning up each test, you can define the following methods:

		virtual void SetUp()
		{
			std::setprecision(15);

			// Code here will be called immediately after the constructor (right
			// before each test).
			Boston.setPoint(42.3601_deg, -71.0589_deg, 0_m);
			Boston_ecef = Boston;

			Lexington.setPoint(42.4430_deg, -71.2290_deg, 0_m);

			LincolnLab.setPoint(42.4625772_deg, -71.2696797_deg, 15_m);
			LincolnLab_ecef.setPoint(1513242.623813242_m, -4462904.554429035_m, 4283659.060698663_m);
			
		}

		virtual void TearDown()
		{
			// Code here will be called immediately after each test (right
			// before the destructor).
		}

		// Objects declared here can be used by all tests in the test case for Foo.
		LLA Boston;
		ECEF Boston_ecef;

		LLA Lexington;

		LLA LincolnLab;
		ECEF LincolnLab_ecef;
	};

	TEST_F(PositionNEDTest, is_point)
	{
		EXPECT_TRUE(coord::traits::is_point<NED>::value);
	}
	
	TEST_F(PositionNEDTest, defaultConstructor)
	{
		EXPECT_TRUE(std::is_default_constructible<NED>::value);
	}

	TEST_F(PositionNEDTest, constructor)
	{
		// no date
		NED ned(1_m, 2_m, 3_m, Boston);
		EXPECT_EQ(1_m, std::get<0>(ned.point()));
		EXPECT_EQ(2_m, std::get<1>(ned.point()));
		EXPECT_EQ(3_m, std::get<2>(ned.point()));
		EXPECT_EQ(Boston.point(), ned.frameData().origin);
		EXPECT_EQ(2005_yr, ned.frameData().date);

		// date
		NED ned2(4_m, 5_m, 6_m, Boston, 2016_yr);
		EXPECT_EQ(4_m, std::get<0>(ned2.point()));
		EXPECT_EQ(5_m, std::get<1>(ned2.point()));
		EXPECT_EQ(6_m, std::get<2>(ned2.point()));
		EXPECT_EQ(Boston.point(), ned2.frameData().origin);
		EXPECT_EQ(2016_yr, ned2.frameData().date);

		// different units
		NED ned3(4_km, 5_km, 6_km, Boston);
		EXPECT_EQ(4000_m, std::get<0>(ned3.point()));
		EXPECT_EQ(5000_m, std::get<1>(ned3.point()));
		EXPECT_EQ(6000_m, std::get<2>(ned3.point()));
		EXPECT_EQ(Boston.point(), ned3.frameData().origin);

		// origin with different frame of reference
		NED ned4(7_m, 8_m, 9_m, Boston_ecef);
		EXPECT_EQ(7_m, std::get<0>(ned4.point()));
		EXPECT_EQ(8_m, std::get<1>(ned4.point()));
		EXPECT_EQ(9_m, std::get<2>(ned4.point()));
		EXPECT_TRUE((Boston.isSame(LLA(ned4.frameData().origin), 1_mm)));	// the origin matches the original LLA within 1mm.
	}

	TEST_F(PositionNEDTest, tupleConstructor)
	{
		// no date
		NED ned(coord::cartesianTuple(1_m, 2_m, 3_m), Boston);
		EXPECT_EQ(1_m, std::get<0>(ned.point()));
		EXPECT_EQ(2_m, std::get<1>(ned.point()));
		EXPECT_EQ(3_m, std::get<2>(ned.point()));
		EXPECT_EQ(Boston.point(), ned.frameData().origin);
		EXPECT_EQ(2005_yr, ned.frameData().date);

		// date
		NED ned2(coord::cartesianTuple(4_m, 5_m, 6_m), Boston, 2016_yr);
		EXPECT_EQ(4_m, std::get<0>(ned2.point()));
		EXPECT_EQ(5_m, std::get<1>(ned2.point()));
		EXPECT_EQ(6_m, std::get<2>(ned2.point()));
		EXPECT_EQ(Boston.point(), ned2.frameData().origin);
		EXPECT_EQ(2016_yr, ned2.frameData().date);

		// different units
		NED ned3(coord::cartesianTuple(4_km, 5_km, 6_km), Boston);
		EXPECT_EQ(4000_m, std::get<0>(ned3.point()));
		EXPECT_EQ(5000_m, std::get<1>(ned3.point()));
		EXPECT_EQ(6000_m, std::get<2>(ned3.point()));
		EXPECT_EQ(Boston.point(), ned3.frameData().origin);

		// origin with different frame of reference
		NED ned4(coord::cartesianTuple(7_m, 8_m, 9_m), Boston_ecef);
		EXPECT_EQ(7_m, std::get<0>(ned4.point()));
		EXPECT_EQ(8_m, std::get<1>(ned4.point()));
		EXPECT_EQ(9_m, std::get<2>(ned4.point()));
		EXPECT_TRUE((Boston.isSame(LLA(ned4.frameData().origin), 1_mm)));	// the origin matches the original LLA within 1mm.
	}

	TEST_F(PositionNEDTest, originOnlyConstructor)
	{
		// no date
		NED ned(Boston);
		EXPECT_EQ(0_m, std::get<0>(ned.point()));
		EXPECT_EQ(0_m, std::get<1>(ned.point()));
		EXPECT_EQ(0_m, std::get<2>(ned.point()));
		EXPECT_EQ(Boston.point(), ned.frameData().origin);
		EXPECT_EQ(2005_yr, ned.frameData().date);

		// date
		NED ned2(Boston, 2016_yr);
		EXPECT_EQ(0_m, std::get<0>(ned2.point()));
		EXPECT_EQ(0_m, std::get<1>(ned2.point()));
		EXPECT_EQ(0_m, std::get<2>(ned2.point()));
		EXPECT_EQ(Boston.point(), ned2.frameData().origin);
		EXPECT_EQ(2016_yr, ned2.frameData().date);
	}

	TEST_F(PositionNEDTest, copyConstructor)
	{
		NED llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		NED llab2(llab);
		EXPECT_TRUE(coord::traits::is_point<decltype(llab)>::value);
		EXPECT_TRUE(coord::traits::is_point<decltype(llab2)>::value);
		EXPECT_TRUE(llab == llab2) << "llab:  " << llab << std::endl << "llab2: " << llab2 << std::endl;
	}

	TEST_F(PositionNEDTest, implicitConversionConstructor_fromECEF)
	{
		NED llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		NED llab2(LincolnLab_ecef, Boston);
		EXPECT_NEAR(11404.80495410014, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-17336.21267499651, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(18.742960276271333, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionNEDTest, implicitConversionConstructor_fromGeo)
	{
		NED llab(LincolnLab, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		NED llab2(LincolnLab, Boston);
		EXPECT_NEAR(11404.80495410014, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-17336.21267499651, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(18.742960276271333, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionNEDTest, implicitConversionConstructor_fromENUsameOrigin)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		NED llab2(llab, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionNEDTest, implicitConversionConstructor_fromENUdifferentOrigin)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		// should just result in a copy
		NED llab2(llab, Boston);
		EXPECT_NEAR(11404.80495410014, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-17336.21267499651, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(18.742960276271333, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionNEDTest, implicitConversionConstructor_fromNEDsameOrigin)
	{
		NED llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		// should just result in a copy
		NED llab2(llab, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}
	
	TEST_F(PositionNEDTest, implicitConversionConstructor_fromNEDdifferentOrigin)
	{
		NED llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		// should just result in a copy
		NED llab2(llab, Boston);
		EXPECT_NEAR(11404.80495410014, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-17336.21267499651, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(18.742960276271333, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionNEDTest, assignment)
	{
		NED llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		NED llab2 = llab;
		EXPECT_TRUE(coord::traits::is_point<decltype(llab)>::value);
		EXPECT_TRUE(coord::traits::is_point<decltype(llab2)>::value);
		EXPECT_TRUE(llab == llab2) << "llab:  " << llab << std::endl << "llab2: " << llab2 << std::endl;;
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromECEF)
	{
		NED llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		NED llab2(Boston);
		llab2 = LincolnLab_ecef;
		EXPECT_NEAR(11404.80495410014, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-17336.21267499651, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(18.742960276271333, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromGeo)
	{
		NED llab(LincolnLab, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		NED llab2(Boston);
		llab2 = LincolnLab;
		EXPECT_NEAR(11404.80495410014, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-17336.21267499651, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(18.742960276271333, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromENUsameOrigin)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		// should just result in a copy
		NED llab2;
		llab2 = llab;
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Lexington.point(), llab2.frameData().origin);

		// should just result in a copy
		NED llab3(Lexington);
		llab3 = llab;
		EXPECT_TRUE(llab3 == llab2) << "llab2:  " << llab << std::endl << "llab3: " << llab3 << std::endl; // llab2 is not a typo!
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromENUdifferentOrigin)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		// should result in an origin translation
		NED llab3(Boston);
		llab3 = llab;
		EXPECT_NEAR(11404.80495410014, std::get<0>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-17336.21267499651, std::get<1>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(18.742960276271333, std::get<2>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Boston.point(), llab3.frameData().origin);
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromNEDsameOrigin)
	{
		NED llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		// should just result in a copy
		NED llab2 = llab;
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Lexington.point(), llab2.frameData().origin);

		// should just result in a copy
		NED llab3(Lexington);
		llab3 = llab;
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Lexington.point(), llab3.frameData().origin);
	}

	TEST_F(PositionNEDTest, implicitConversionAssignment_fromNEDdifferentOrigin)
	{
		NED llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(2175.482814617637, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-3345.831985833716, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		// should result in an origin translation
		NED llab3(Boston);
		llab3 = llab;
		EXPECT_NEAR(11404.80495410014, std::get<0>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-17336.21267499651, std::get<1>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(18.742960276271333, std::get<2>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Boston.point(), llab3.frameData().origin);
	}

	TEST_F(PositionNEDTest, isSame)
	{
		LLA origin1(42_deg, -71_deg, 0_m);	// Boston-ish
		LLA origin2(34_deg, -118_deg, 0_m);	// LA-ish

		using NED_ft = PositionNED<datums::WGS84_G1674, units::length::feet>;
		using ECEF_mm = PositionECEF<datums::WGS84_G1674, units::length::millimeters>;

		NED ned1(1_m, 2_m, 3_m, origin1);
		NED ned2(4_m, 5_m, 6_m, origin2);

		NED exactlySame1(1_m, 2_m, 3_m, origin1);
		NED exactlySame2(4_m, 5_m, 6_m, origin2);

		NED exactlySameDiffOrigin1(ned1, origin2);	// These are not *actually* 100% the same, since c++ trig functions aren't fully reversible.
		NED exactlySameDiffOrigin2(ned2, origin1);

		NED close1(1.001_m, 2.002_m, 3.003_m, origin1);
		NED close2(4.001_m, 5.002_m, 6.003_m, origin2);

		NED_ft closeDifferentUnits(3.28083989_ft, 6.56167979_ft, 9.84251968_ft, origin1);
		ECEF_mm millimeterTolerance(1_mm, 2_mm, 3_mm);

		EXPECT_EQ(1_m, std::get<0>(ned1.point()));
		EXPECT_EQ(2_m, std::get<1>(ned1.point()));
		EXPECT_EQ(3_m, std::get<2>(ned1.point()));

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
		EXPECT_TRUE((ned1.isSame(exactlySame1, 4100_km)));
		EXPECT_TRUE((ned1.isSame(exactlySameDiffOrigin1, 4100_km)));
		EXPECT_TRUE((ned1.isSame(close1, 4100_km)));
		EXPECT_TRUE((ned1.isSame(exactlySame2, 4100_km)));
		EXPECT_TRUE((ned1.isSame(exactlySameDiffOrigin2, 4100_km)));

		EXPECT_TRUE((ned2.isSame(exactlySame2, 4100_km)));
		EXPECT_TRUE((ned2.isSame(exactlySameDiffOrigin2, 4100_km)));
		EXPECT_TRUE((ned2.isSame(close2, 4100_km)));
		EXPECT_TRUE((ned2.isSame(exactlySame1, 4100_km)));
		EXPECT_TRUE((ned2.isSame(exactlySameDiffOrigin1, 4100_km)));
	}

	TEST_F(PositionNEDTest, distance)
	{
		std::setprecision(16);

		LLA origin1(42_deg, -71_deg, 0_m);	// Boston-ish
		LLA origin2(34_deg, -118_deg, 0_m);	// LA-ish

		NED nedb0(0_m, 0_m, 0_m, origin1);
		NED nedb1(1_m, 1_m, 1_m, origin1);

		NED nedl0(0_m, 0_m, 0_m, origin2);
		NED nedl1(1_m, 1_m, 1_m, origin2);

		// same origin
		EXPECT_EQ(meter_t(sqrt(3)), nedb0.distance(nedb1));
		EXPECT_EQ(meter_t(sqrt(3)), nedb1.distance(nedb0));
		EXPECT_EQ(meter_t(sqrt(3)), nedl0.distance(nedl1));
		EXPECT_EQ(meter_t(sqrt(3)), nedl1.distance(nedl0));

		// different point types
		EXPECT_NEAR(meter_t(sqrt(3)).to<double>(), nedb1.distance(origin1).to<double>(), 5.0e-10);
		EXPECT_NEAR(meter_t(sqrt(3)).to<double>(), nedl1.distance(origin2).to<double>(), 5.0e-10);

		//different origin
		EXPECT_NEAR((4094891.087804173_m).to<double>(), nedb0.distance(nedl0).to<double>(), 1.0e-9);
		EXPECT_NEAR((4094891.087804173_m).to<double>(), nedb0.distance(origin2).to<double>(), 1.0e-9);
	}

	TEST_F(PositionNEDTest, dotProduct)
	{
		std::setprecision(16);

		LLA origin1(42_deg, -71_deg, 0_m);	// Boston-ish
		LLA origin2(34_deg, -118_deg, 0_m);	// LA-ish

		NED nedb0(1_m, 2_m, 3_m, origin1);
		NED nedb1(1_m, 2_m, 3_m, origin1);

		NED nedl0(4_m, 5_m, 6_m, origin2);
		NED nedl1(4_m, 5_m, 6_m, origin2);

		// truth data from matlab

		// same origin
		EXPECT_EQ(14_sq_m, nedb0.dotProduct(nedb1));
		EXPECT_EQ(77_sq_m, nedl0.dotProduct(nedl1));

		// different origins
		EXPECT_NEAR(-3562875.264019654, nedb0.dotProduct(nedl1).to<double>(), 5.0e-8);			// wrt origin1
		EXPECT_NEAR(32155885.49223858, nedl1.dotProduct(nedb0).to<double>(), 5.0e-9);			// wrt origin2

		// different point types
		EXPECT_NEAR(-3562875.264019654, nedb0.dotProduct(nedl1).to<double>(), 5.0e-8);			// wrt origin1
		EXPECT_NEAR(32155885.49223858, nedl1.dotProduct(nedb0).to<double>(), 5.0e-9);			// wrt origin2
	}

	TEST_F(PositionNEDTest, magnitude)
	{
		// should be the same if x/y/z values are the same, regardless of origin
		NED ned1(1_m, 2_m, 3_m, LLA());
		NED ned2(1_m, 2_m, 3_m, Boston);
		NED ned3(4_m, 5_m, 6_m, Lexington);

		EXPECT_NEAR(3.741657386773941, ned1.magnitude().to<double>(), 5.0e-15);
		EXPECT_NEAR(3.741657386773941, ned2.magnitude().to<double>(), 5.0e-15);
		EXPECT_NEAR(8.774964387392123, ned3.magnitude().to<double>(), 5.0e-15);
	}

	TEST_F(PositionNEDTest, east)
	{
		NED ned;
		EXPECT_EQ(0_m, ned.north());

		NED ned1(5_m, 6_m, 7_m, LLA());
		EXPECT_EQ(5_m, ned1.north());

		NED ned2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(5_mm, ned2.north());
	}

	TEST_F(PositionNEDTest, north)
	{
		NED ned;
		EXPECT_EQ(0_m, ned.east());

		NED ned1(5_m, 6_m, 7_m, LLA());
		EXPECT_EQ(6_m, ned1.east());

		NED ned2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(6_mm, ned2.east());
	}

	TEST_F(PositionNEDTest, up)
	{
		NED ned;
		EXPECT_EQ(0_m, ned.down());

		NED ned1(5_m, 6_m, 7_m, LLA());
		EXPECT_EQ(7_m, ned1.down());

		NED ned2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(7_mm, ned2.down());
	}

	TEST_F(PositionNEDTest, origin)
	{
		NED ned;
		EXPECT_TRUE(LLA(0_deg, 0_deg, 0_m) == ned.origin());

		NED ned2(0_m, 0_m, 0_m, Boston);
		EXPECT_TRUE(Boston == ned2.origin());
	}

	TEST_F(PositionNEDTest, date)
	{
		NED ned;
		EXPECT_EQ(2005_yr, ned.date());

		NED ned2(0_m, 0_m, 0_m, LLA(), 2016.421_yr);
		EXPECT_EQ(2016.421_yr, ned2.date());
	}

	TEST_F(PositionNEDTest, setNorth)
	{
		NED ned;
		EXPECT_EQ(0_m, ned.north());

		ned.setNorth(5_m);
		EXPECT_EQ(5_m, ned.north());
		EXPECT_EQ(0_m, ned.east());
		EXPECT_EQ(0_m, ned.down());

		ned.setNorth(5_mm);
		EXPECT_EQ(0.005_m, ned.north());
		EXPECT_EQ(0_m, ned.east());
		EXPECT_EQ(0_m, ned.down());
	}

	TEST_F(PositionNEDTest, setEast)
	{
		NED ned;
		EXPECT_EQ(0_m, ned.east());

		ned.setEast(6_m);
		EXPECT_EQ(0_m, ned.north());
		EXPECT_EQ(6_m, ned.east());
		EXPECT_EQ(0_m, ned.down());

		ned.setEast(6_mm);
		EXPECT_EQ(0_m, ned.north());
		EXPECT_EQ(0.006_m, ned.east());
		EXPECT_EQ(0_m, ned.down());
	}

	TEST_F(PositionNEDTest, setDown)
	{
		NED ned;
		EXPECT_EQ(0_m, ned.down());

		ned.setDown(7_m);
		EXPECT_EQ(0_m, ned.north());
		EXPECT_EQ(0_m, ned.east());
		EXPECT_EQ(7_m, ned.down());

		ned.setDown(7_mm);
		EXPECT_EQ(0_m, ned.north());
		EXPECT_EQ(0_m, ned.east());
		EXPECT_EQ(0.007_m, ned.down());
	}

	TEST_F(PositionNEDTest, setOrigin)
	{
		NED ned;
		EXPECT_TRUE(LLA(0_deg, 0_deg, 0_m) == ned.origin());

		NED ned2(0_m, 0_m, 0_m, Boston);
		EXPECT_TRUE(Boston == ned2.origin());

		NED ned3(ned2);
		ned3.setOrigin(Lexington);
		EXPECT_NEAR(-9194.581078166790, ned3.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(14013.15891974845, ned3.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(22.012073203577529, ned3.down().to<double>(), 5.0e-9);
		EXPECT_TRUE(Lexington == ned3.origin());
		EXPECT_TRUE(ned2.isSame(ned3, 3_nm));
	}

	TEST_F(PositionNEDTest, ostream)
	{
		using inch_t = units::length::inch_t;

		PositionNED<datums::WGS84_G1674, units::length::inches> ned1(12.1_in, 24.2_in, 36.3_in, LLA(42_deg, -71_deg, 0_m));

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
		NED ned1(12_m, 24_m, 36_m, Boston);
		NED ned2(36_m, 24_m, 12_m, Boston);
		NED ned3(1_m, 2_m, 3_m, Lexington);

		EXPECT_EQ(12_m, ned1.north());
		EXPECT_EQ(24_m, ned1.east());
		EXPECT_EQ(36_m, ned1.down());

		EXPECT_EQ(36_m, ned2.north());
		EXPECT_EQ(24_m, ned2.east());
		EXPECT_EQ(12_m, ned2.down());

		EXPECT_EQ(1_m, ned3.north());
		EXPECT_EQ(2_m, ned3.east());
		EXPECT_EQ(3_m, ned3.down());

		// same units
		ned1 += ned2;

		EXPECT_EQ(48_m, ned1.north());
		EXPECT_EQ(48_m, ned1.east());
		EXPECT_EQ(48_m, ned1.down());

		EXPECT_EQ(36_m, ned2.north());
		EXPECT_EQ(24_m, ned2.east());
		EXPECT_EQ(12_m, ned2.down());

		// different units
		ned2 += ned3;
		EXPECT_NEAR(9259.607688572551, ned2.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(-13968.71534922575, ned2.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(37.009226581068106, ned2.down().to<double>(), 5.0e-9);

		EXPECT_EQ(1_m, ned3.north());
		EXPECT_EQ(2_m, ned3.east());
		EXPECT_EQ(3_m, ned3.down());
	}

	TEST_F(PositionNEDTest, minusEqual)
	{
		NED zero;
		NED ned1(12_m, 24_m, 36_m, Boston);
		NED ned2(36_m, 24_m, 12_m, Boston);
		NED ned3(1_m, 2_m, 3_m, Lexington);

		EXPECT_EQ(12_m, ned1.north());
		EXPECT_EQ(24_m, ned1.east());
		EXPECT_EQ(36_m, ned1.down());

		EXPECT_EQ(36_m, ned2.north());
		EXPECT_EQ(24_m, ned2.east());
		EXPECT_EQ(12_m, ned2.down());

		EXPECT_EQ(1_m, ned3.north());
		EXPECT_EQ(2_m, ned3.east());
		EXPECT_EQ(3_m, ned3.down());

		// same units
		ned1 -= ned2;

		EXPECT_EQ(-24_m, ned1.north());
		EXPECT_EQ(0_m, ned1.east());
		EXPECT_EQ(24_m, ned1.down());

		EXPECT_EQ(36_m, ned2.north());
		EXPECT_EQ(24_m, ned2.east());
		EXPECT_EQ(12_m, ned2.down());

		// different units
		ned2 -= ned3;
		EXPECT_NEAR(-9187.607688572551, ned2.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(14016.71534922575, ned2.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(-13.009226581068106, ned2.down().to<double>(), 5.0e-9);

		EXPECT_EQ(1_m, ned3.north());
		EXPECT_EQ(2_m, ned3.east());
		EXPECT_EQ(3_m, ned3.down());
	}

	TEST_F(PositionNEDTest, timesEqual)
	{
		PositionNED<datums::NAD83, units::length::feet> ned_ft(1_ft, 2_ft, 3_ft, Boston);

		ned_ft *= 3;
		EXPECT_EQ(3_ft, ned_ft.north());
		EXPECT_EQ(6_ft, ned_ft.east());
		EXPECT_EQ(9_ft, ned_ft.down());
	}

	TEST_F(PositionNEDTest, divideEqual)
	{
		PositionNED<datums::NAD83, units::length::feet> ned_ft(1_ft, 2_ft, 3_ft, Boston);

		ned_ft /= 3;
		EXPECT_EQ(foot_t(1.0 / 3), ned_ft.north());
		EXPECT_EQ(foot_t(2.0 / 3), ned_ft.east());
		EXPECT_EQ(1_ft, ned_ft.down());
	}
}


#endif // positionNEDTest_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: