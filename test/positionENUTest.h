#ifndef positionENUTest_h__
#define positionENUTest_h__

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
	class PositionENUTest : public ::testing::Test {
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		PositionENUTest()
		{
			// You can do set-up work for each test here.
		}

		virtual ~PositionENUTest()
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

	TEST_F(PositionENUTest, is_point)
	{
		EXPECT_TRUE(coord::traits::is_point<ENU>::value);
	}
	
	TEST_F(PositionENUTest, defaultConstructor)
	{
		EXPECT_TRUE(std::is_default_constructible<ENU>::value);
	}

	TEST_F(PositionENUTest, constructor)
	{
		// no date
		ENU enu(1_m, 2_m, 3_m, Boston);
		EXPECT_EQ(1_m, std::get<0>(enu.point()));
		EXPECT_EQ(2_m, std::get<1>(enu.point()));
		EXPECT_EQ(3_m, std::get<2>(enu.point()));
		EXPECT_EQ(Boston.point(), enu.frameData().origin);
		EXPECT_EQ(2005_yr, enu.frameData().date);

		// date
		ENU enu2(4_m, 5_m, 6_m, Boston, 2016_yr);
		EXPECT_EQ(4_m, std::get<0>(enu2.point()));
		EXPECT_EQ(5_m, std::get<1>(enu2.point()));
		EXPECT_EQ(6_m, std::get<2>(enu2.point()));
		EXPECT_EQ(Boston.point(), enu2.frameData().origin);
		EXPECT_EQ(2016_yr, enu2.frameData().date);

		// different units
		ENU enu3(4_km, 5_km, 6_km, Boston);
		EXPECT_EQ(4000_m, std::get<0>(enu3.point()));
		EXPECT_EQ(5000_m, std::get<1>(enu3.point()));
		EXPECT_EQ(6000_m, std::get<2>(enu3.point()));
		EXPECT_EQ(Boston.point(), enu3.frameData().origin);

		// origin with different frame of reference
		ENU enu4(7_m, 8_m, 9_m, Boston_ecef);
		EXPECT_EQ(7_m, std::get<0>(enu4.point()));
		EXPECT_EQ(8_m, std::get<1>(enu4.point()));
		EXPECT_EQ(9_m, std::get<2>(enu4.point()));
		EXPECT_TRUE((Boston.isSame(LLA(enu4.frameData().origin), 1_mm)));	// the origin matches the original LLA within 1mm.
	}

	TEST_F(PositionENUTest, tupleConstructor)
	{
		// no date
		ENU enu(coord::cartesianTuple(1_m, 2_m, 3_m), Boston);
		EXPECT_EQ(1_m, std::get<0>(enu.point()));
		EXPECT_EQ(2_m, std::get<1>(enu.point()));
		EXPECT_EQ(3_m, std::get<2>(enu.point()));
		EXPECT_EQ(Boston.point(), enu.frameData().origin);
		EXPECT_EQ(2005_yr, enu.frameData().date);

		// date
		ENU enu2(coord::cartesianTuple(4_m, 5_m, 6_m), Boston, 2016_yr);
		EXPECT_EQ(4_m, std::get<0>(enu2.point()));
		EXPECT_EQ(5_m, std::get<1>(enu2.point()));
		EXPECT_EQ(6_m, std::get<2>(enu2.point()));
		EXPECT_EQ(Boston.point(), enu2.frameData().origin);
		EXPECT_EQ(2016_yr, enu2.frameData().date);

		// different units
		ENU enu3(coord::cartesianTuple(4_km, 5_km, 6_km), Boston);
		EXPECT_EQ(4000_m, std::get<0>(enu3.point()));
		EXPECT_EQ(5000_m, std::get<1>(enu3.point()));
		EXPECT_EQ(6000_m, std::get<2>(enu3.point()));
		EXPECT_EQ(Boston.point(), enu3.frameData().origin);

		// origin with different frame of reference
		ENU enu4(coord::cartesianTuple(7_m, 8_m, 9_m), Boston_ecef);
		EXPECT_EQ(7_m, std::get<0>(enu4.point()));
		EXPECT_EQ(8_m, std::get<1>(enu4.point()));
		EXPECT_EQ(9_m, std::get<2>(enu4.point()));
		EXPECT_TRUE((Boston.isSame(LLA(enu4.frameData().origin), 1_mm)));	// the origin matches the original LLA within 1mm.
	}

	TEST_F(PositionENUTest, originOnlyConstructor)
	{
		// no date
		ENU enu(Boston);
		EXPECT_EQ(0_m, std::get<0>(enu.point()));
		EXPECT_EQ(0_m, std::get<1>(enu.point()));
		EXPECT_EQ(0_m, std::get<2>(enu.point()));
		EXPECT_EQ(Boston.point(), enu.frameData().origin);
		EXPECT_EQ(2005_yr, enu.frameData().date);

		// date
		ENU enu2(Boston, 2016_yr);
		EXPECT_EQ(0_m, std::get<0>(enu2.point()));
		EXPECT_EQ(0_m, std::get<1>(enu2.point()));
		EXPECT_EQ(0_m, std::get<2>(enu2.point()));
		EXPECT_EQ(Boston.point(), enu2.frameData().origin);
		EXPECT_EQ(2016_yr, enu2.frameData().date);
	}

	TEST_F(PositionENUTest, copyConstructor)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		ENU llab2(llab);
		EXPECT_TRUE(coord::traits::is_point<decltype(llab)>::value);
		EXPECT_TRUE(coord::traits::is_point<decltype(llab2)>::value);
		EXPECT_TRUE(llab == llab2);
	}

	TEST_F(PositionENUTest, implicitConversionConstructor_fromECEF)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		ENU llab2(LincolnLab_ecef, Boston);
		EXPECT_NEAR(-17336.21267499651, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(11404.80495410014, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.742960276271333, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionENUTest, implicitConversionConstructor_fromGeo)
	{
		ENU llab(LincolnLab, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		ENU llab2(LincolnLab, Boston);
		EXPECT_NEAR(-17336.21267499651, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(11404.80495410014, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.742960276271333, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionENUTest, implicitConversionConstructor_fromENUsameOrigin)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		// should just result in a copy
		ENU llab2(llab, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}
	
	TEST_F(PositionENUTest, implicitConversionConstructor_fromENUdifferentOrigin)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		// should just result in a copy
		ENU llab2(llab, Boston);
		EXPECT_NEAR(-17336.21267499651, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(11404.80495410014, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.742960276271333, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionENUTest, assignment)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		ENU llab2 = llab;
		EXPECT_TRUE(coord::traits::is_point<decltype(llab)>::value);
		EXPECT_TRUE(coord::traits::is_point<decltype(llab2)>::value);
		EXPECT_TRUE(llab == llab2);
	}

	TEST_F(PositionENUTest, implicitConversionAssignment_fromECEF)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		ENU llab2(Boston);
		llab2 = LincolnLab_ecef;
		EXPECT_NEAR(-17336.21267499651, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(11404.80495410014, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.742960276271333, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionENUTest, implicitConversionAssignment_fromGeo)
	{
		ENU llab(LincolnLab, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		ENU llab2(Boston);
		llab2 = LincolnLab;
		EXPECT_NEAR(-17336.21267499651, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(11404.80495410014, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.742960276271333, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
	}

	TEST_F(PositionENUTest, implicitConversionAssignment_fromENUsameOrigin)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		// should just result in a copy
		ENU llab2 = llab;
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab2.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Lexington.point(), llab2.frameData().origin);

		// should just result in a copy
		ENU llab3(Lexington);
		llab3 = llab2;
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Lexington.point(), llab3.frameData().origin);
	}

	TEST_F(PositionENUTest, implicitConversionAssignment_fromENUdifferentOrigin)
	{
		ENU llab(LincolnLab_ecef, Lexington);
		EXPECT_NEAR(-3345.831985833716, std::get<0>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(2175.482814617637, std::get<1>(llab.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(13.751962531646313, std::get<2>(llab.point()).to<double>(), 5.0e-9);

		// should result in an origin translation
		ENU llab3(Boston);
		llab3 = llab;
		EXPECT_NEAR(-17336.21267499651, std::get<0>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(11404.80495410014, std::get<1>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_NEAR(-18.742960276271333, std::get<2>(llab3.point()).to<double>(), 5.0e-9);
		EXPECT_EQ(Boston.point(), llab3.frameData().origin);
	}

	TEST_F(PositionENUTest, isSame)
	{
		LLA origin1(42_deg, -71_deg, 0_m);	// Boston-ish
		LLA origin2(34_deg, -118_deg, 0_m);	// LA-ish

		using ENU_ft = PositionENU<datums::WGS84_G1674, units::length::feet>;
		using ECEF_mm = PositionECEF<datums::WGS84_G1674, units::length::millimeters>;

		ENU enu1(1_m, 2_m, 3_m, origin1);
		ENU enu2(4_m, 5_m, 6_m, origin2);

		ENU exactlySame1(1_m, 2_m, 3_m, origin1);
		ENU exactlySame2(4_m, 5_m, 6_m, origin2);

		ENU exactlySameDiffOrigin1(enu1, origin2);	// These are not *actually* 100% the same, since c++ trig functions aren't fully reversible.
		ENU exactlySameDiffOrigin2(enu2, origin1);

		ENU close1(1.001_m, 2.002_m, 3.003_m, origin1);
		ENU close2(4.001_m, 5.002_m, 6.003_m, origin2);

		ENU_ft closeDifferentUnits(3.28083989_ft, 6.56167979_ft, 9.84251968_ft, origin1);
		ECEF_mm millimeterTolerance(1_mm, 2_mm, 3_mm);

		EXPECT_EQ(1_m, std::get<0>(enu1.point()));
		EXPECT_EQ(2_m, std::get<1>(enu1.point()));
		EXPECT_EQ(3_m, std::get<2>(enu1.point()));

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
		EXPECT_TRUE((enu1.isSame(exactlySame1, 4100_km)));
		EXPECT_TRUE((enu1.isSame(exactlySameDiffOrigin1, 4100_km)));
		EXPECT_TRUE((enu1.isSame(close1, 4100_km)));
		EXPECT_TRUE((enu1.isSame(exactlySame2, 4100_km)));
		EXPECT_TRUE((enu1.isSame(exactlySameDiffOrigin2, 4100_km)));

		EXPECT_TRUE((enu2.isSame(exactlySame2, 4100_km)));
		EXPECT_TRUE((enu2.isSame(exactlySameDiffOrigin2, 4100_km)));
		EXPECT_TRUE((enu2.isSame(close2, 4100_km)));
		EXPECT_TRUE((enu2.isSame(exactlySame1, 4100_km)));
		EXPECT_TRUE((enu2.isSame(exactlySameDiffOrigin1, 4100_km)));
	}

	TEST_F(PositionENUTest, distance)
	{
		std::setprecision(16);

		LLA origin1(42_deg, -71_deg, 0_m);	// Boston-ish
		LLA origin2(34_deg, -118_deg, 0_m);	// LA-ish

		ENU enub0(0_m, 0_m, 0_m, origin1);
		ENU enub1(1_m, 1_m, 1_m, origin1);

		ENU enul0(0_m, 0_m, 0_m, origin2);
		ENU enul1(1_m, 1_m, 1_m, origin2);

		// same origin
		EXPECT_EQ(meter_t(sqrt(3)), enub0.distance(enub1));
		EXPECT_EQ(meter_t(sqrt(3)), enub1.distance(enub0));
		EXPECT_EQ(meter_t(sqrt(3)), enul0.distance(enul1));
		EXPECT_EQ(meter_t(sqrt(3)), enul1.distance(enul0));

		// different point types
		EXPECT_NEAR(meter_t(sqrt(3)).to<double>(), enub1.distance(origin1).to<double>(), 5.0e-10);
		EXPECT_NEAR(meter_t(sqrt(3)).to<double>(), enul1.distance(origin2).to<double>(), 5.0e-10);

		//different origin
		EXPECT_NEAR((4094891.087804173_m).to<double>(), enub0.distance(enul0).to<double>(), 1.0e-9);
		EXPECT_NEAR((4094891.087804173_m).to<double>(), enub0.distance(origin2).to<double>(), 1.0e-9);
	}

	TEST_F(PositionENUTest, dotProduct)
	{
		std::setprecision(16);

		LLA origin1(42_deg, -71_deg, 0_m);	// Boston-ish
		LLA origin2(34_deg, -118_deg, 0_m);	// LA-ish

		ENU enub0(1_m, 2_m, 3_m, origin1);
		ENU enub1(1_m, 2_m, 3_m, origin1);

		ENU enul0(4_m, 5_m, 6_m, origin2);
		ENU enul1(4_m, 5_m, 6_m, origin2);

		// truth data from matlab

		// same origin
		EXPECT_EQ(14_sq_m, enub0.dotProduct(enub1));
		EXPECT_EQ(77_sq_m, enul0.dotProduct(enul1));

		// different origins
		EXPECT_NEAR(-7327114.484793222, enub0.dotProduct(enul1).to<double>(), 5.0e-9);			// wrt origin1
		EXPECT_NEAR(14652796.55974841, enul1.dotProduct(enub0).to<double>(), 5.0e-9);			// wrt origin2

		// different point types
		EXPECT_NEAR(-7327114.484793222, enub0.dotProduct(enul1).to<double>(), 5.0e-9);			// wrt origin1
		EXPECT_NEAR(14652796.55974841, enul1.dotProduct(enub0).to<double>(), 5.0e-9);			// wrt origin2
	}

	TEST_F(PositionENUTest, magnitude)
	{
		// should be the same if x/y/z values are the same, regardless of origin
		ENU enu1(1_m, 2_m, 3_m, LLA());
		ENU enu2(1_m, 2_m, 3_m, Boston);
		ENU enu3(4_m, 5_m, 6_m, Lexington);

		EXPECT_NEAR(3.741657386773941, enu1.magnitude().to<double>(), 5.0e-15);
		EXPECT_NEAR(3.741657386773941, enu2.magnitude().to<double>(), 5.0e-15);
		EXPECT_NEAR(8.774964387392123, enu3.magnitude().to<double>(), 5.0e-15);
	}

	TEST_F(PositionENUTest, east)
	{
		ENU enu;
		EXPECT_EQ(0_m, enu.east());

		ENU enu1(5_m, 6_m, 7_m, LLA());
		EXPECT_EQ(5_m, enu1.east());

		ENU enu2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(5_mm, enu2.east());
	}

	TEST_F(PositionENUTest, north)
	{
		ENU enu;
		EXPECT_EQ(0_m, enu.north());

		ENU enu1(5_m, 6_m, 7_m, LLA());
		EXPECT_EQ(6_m, enu1.north());

		ENU enu2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(6_mm, enu2.north());
	}

	TEST_F(PositionENUTest, up)
	{
		ENU enu;
		EXPECT_EQ(0_m, enu.up());

		ENU enu1(5_m, 6_m, 7_m, LLA());
		EXPECT_EQ(7_m, enu1.up());

		ENU enu2(5_mm, 6_mm, 7_mm, LLA());
		EXPECT_EQ(7_mm, enu2.up());
	}

	TEST_F(PositionENUTest, origin)
	{
		ENU enu;
		EXPECT_TRUE(LLA(0_deg, 0_deg, 0_m) == enu.origin());

		ENU enu2(0_m, 0_m, 0_m, Boston);
		EXPECT_TRUE(Boston == enu2.origin());
	}

	TEST_F(PositionENUTest, date)
	{
		ENU enu;
		EXPECT_EQ(2005_yr, enu.date());

		ENU enu2(0_m, 0_m, 0_m, LLA(), 2016.421_yr);
		EXPECT_EQ(2016.421_yr, enu2.date());
	}

	TEST_F(PositionENUTest, setEast)
	{
		ENU enu;
		EXPECT_EQ(0_m, enu.east());

		enu.setEast(5_m);
		EXPECT_EQ(5_m, enu.east());
		EXPECT_EQ(0_m, enu.north());
		EXPECT_EQ(0_m, enu.up());

		enu.setEast(5_mm);
		EXPECT_EQ(0.005_m, enu.east());
		EXPECT_EQ(0_m, enu.north());
		EXPECT_EQ(0_m, enu.up());
	}

	TEST_F(PositionENUTest, setNorth)
	{
		ENU enu;
		EXPECT_EQ(0_m, enu.north());

		enu.setNorth(6_m);
		EXPECT_EQ(0_m, enu.east());
		EXPECT_EQ(6_m, enu.north());
		EXPECT_EQ(0_m, enu.up());

		enu.setNorth(6_mm);
		EXPECT_EQ(0_m, enu.east());
		EXPECT_EQ(0.006_m, enu.north());
		EXPECT_EQ(0_m, enu.up());
	}

	TEST_F(PositionENUTest, setUp)
	{
		ENU enu;
		EXPECT_EQ(0_m, enu.up());

		enu.setUp(7_m);
		EXPECT_EQ(0_m, enu.east());
		EXPECT_EQ(0_m, enu.north());
		EXPECT_EQ(7_m, enu.up());

		enu.setUp(7_mm);
		EXPECT_EQ(0_m, enu.east());
		EXPECT_EQ(0_m, enu.north());
		EXPECT_EQ(0.007_m, enu.up());
	}

	TEST_F(PositionENUTest, setOrigin)
	{
		ENU enu;
		EXPECT_TRUE(LLA(0_deg, 0_deg, 0_m) == enu.origin());

		ENU enu2(0_m, 0_m, 0_m, Boston);
		EXPECT_TRUE(Boston == enu2.origin());

		ENU enu3(enu2);
		enu3.setOrigin(Lexington);
		EXPECT_NEAR(14013.15891974845, enu3.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(-9194.581078166790, enu3.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(-22.012073203577529, enu3.up().to<double>(), 5.0e-9);
		EXPECT_TRUE(Lexington == enu3.origin());
		EXPECT_TRUE(enu2.isSame(enu3, 3_nm));
	}

	TEST_F(PositionENUTest, ostream)
	{
		using inch_t = units::length::inch_t;

		PositionENU<datums::WGS84_G1674, units::length::inches> enu1(12.1_in, 24.2_in, 36.3_in, LLA(42_deg, -71_deg, 0_m));

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
		ENU enu1(12_m, 24_m, 36_m, Boston);
		ENU enu2(36_m, 24_m, 12_m, Boston);
		ENU enu3(1_m, 2_m, 3_m, Lexington);

		EXPECT_EQ(12_m, enu1.east());
		EXPECT_EQ(24_m, enu1.north());
		EXPECT_EQ(36_m, enu1.up());

		EXPECT_EQ(36_m, enu2.east());
		EXPECT_EQ(24_m, enu2.north());
		EXPECT_EQ(12_m, enu2.up());

		EXPECT_EQ(1_m, enu3.east());
		EXPECT_EQ(2_m, enu3.north());
		EXPECT_EQ(3_m, enu3.up());

		// same units
		enu1 += enu2;

		EXPECT_EQ(48_m, enu1.east());
		EXPECT_EQ(48_m, enu1.north());
		EXPECT_EQ(48_m, enu1.up());

		EXPECT_EQ(36_m, enu2.east());
		EXPECT_EQ(24_m, enu2.north());
		EXPECT_EQ(12_m, enu2.up());

		// different units
		enu2 += enu3;
		EXPECT_NEAR(-13957.72648624524, enu2.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(9248.61838027574, enu2.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(-7.01288568046, enu2.up().to<double>(), 5.0e-9);

		EXPECT_EQ(1_m, enu3.east());
		EXPECT_EQ(2_m, enu3.north());
		EXPECT_EQ(3_m, enu3.up());
	}

	TEST_F(PositionENUTest, minusEqual)
	{
		ENU zero;
		ENU enu1(12_m, 24_m, 36_m, Boston);
		ENU enu2(36_m, 24_m, 12_m, Boston);
		ENU enu3(1_m, 2_m, 3_m, Lexington);

		EXPECT_EQ(12_m, enu1.east());
		EXPECT_EQ(24_m, enu1.north());
		EXPECT_EQ(36_m, enu1.up());

		EXPECT_EQ(36_m, enu2.east());
		EXPECT_EQ(24_m, enu2.north());
		EXPECT_EQ(12_m, enu2.up());

		EXPECT_EQ(1_m, enu3.east());
		EXPECT_EQ(2_m, enu3.north());
		EXPECT_EQ(3_m, enu3.up());

		// same units
		enu1 -= enu2;

		EXPECT_EQ(-24_m, enu1.east());
		EXPECT_EQ(0_m, enu1.north());
		EXPECT_EQ(24_m, enu1.up());

		EXPECT_EQ(36_m, enu2.east());
		EXPECT_EQ(24_m, enu2.north());
		EXPECT_EQ(12_m, enu2.up());

		// different units
		enu2 -= enu3;
		EXPECT_NEAR(14029.72648624524, enu2.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(-9200.61838027574, enu2.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(31.01288568046, enu2.up().to<double>(), 5.0e-9);

		EXPECT_EQ(1_m, enu3.east());
		EXPECT_EQ(2_m, enu3.north());
		EXPECT_EQ(3_m, enu3.up());
	}

	TEST_F(PositionENUTest, timesEqual)
	{
		PositionENU<datums::NAD83, units::length::feet> enu_ft(1_ft, 2_ft, 3_ft, Boston);

		enu_ft *= 3;
		EXPECT_EQ(3_ft, enu_ft.east());
		EXPECT_EQ(6_ft, enu_ft.north());
		EXPECT_EQ(9_ft, enu_ft.up());
	}

	TEST_F(PositionENUTest, divideEqual)
	{
		PositionENU<datums::NAD83, units::length::feet> enu_ft(1_ft, 2_ft, 3_ft, Boston);

		enu_ft /= 3;
		EXPECT_EQ(foot_t(1.0 / 3), enu_ft.east());
		EXPECT_EQ(foot_t(2.0 / 3), enu_ft.north());
		EXPECT_EQ(1_ft, enu_ft.up());
	}
}


#endif // positionENUTest_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: