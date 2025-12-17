#ifndef pointTest_h__
#define pointTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <initializer_list>
#include <stdexcept>
#include <type_traits>

using namespace coord;
using namespace units;

namespace
{
	// The fixture for testing class Foo.
	class PointTest : public ::testing::Test {
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		PointTest()
		{
			// You can do set-up work for each test here.
			boston_wgs84 = std::make_tuple<double, double, double>(42.354991, -71.065599, 0.0);
			nevada_wgs84 = std::make_tuple<double, double, double>(37.235000, -115.811100, 0.0);
			boston_nad83 = std::make_tuple<double, double, double>(42.354982, -71.065598, 0.0);
			nevada_nad83 = std::make_tuple<double, double, double>(37.234995, -115.811088, 0.0);
		}

		virtual ~PointTest()
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



		std::tuple<double, double, double> boston_wgs84;
		std::tuple<double, double, double> nevada_wgs84;
		std::tuple<double, double, double> boston_nad83;
		std::tuple<double, double, double> nevada_nad83;

		struct NotAPoint {};


	};

	struct TestPoint : public coord::Point<ENUFrame<horizontalDatums::ITRF2008>, coord::cartesianTuple, coord::FrameData>
	{
		TestPoint() {};

		using coord::Point<ENUFrame<horizontalDatums::ITRF2008>, coord::cartesianTuple, coord::FrameData>::tuple_type;
		using coord::Point<ENUFrame<horizontalDatums::ITRF2008>, coord::cartesianTuple, coord::FrameData>::frame_data_type;

		tuple_type point() const { return tuple_type(); }
		void setPoint(const tuple_type& t) {}
		void setPoint(tuple_type&& t) {}

		frame_data_type frameData() const { return frame_data_type(); }
		void setFrameData(const frame_data_type& frameData) { return; }
		void setFrameData(frame_data_type&& frameData) { return; }
	};

	struct TestSphericalPoint : public coord::Point<Geodetic2DFrame<horizontalDatums::NAD83>, coord::sphericalTuple, coord::FrameData>
	{
		TestSphericalPoint() {};

		using coord::Point<Geodetic2DFrame<horizontalDatums::NAD83>, coord::sphericalTuple, coord::FrameData>::tuple_type;
		using coord::Point<Geodetic2DFrame<horizontalDatums::NAD83>, coord::sphericalTuple, coord::FrameData>::frame_data_type;
		using coord::Point<Geodetic2DFrame<horizontalDatums::NAD83>, coord::sphericalTuple, coord::FrameData>::reference_frame;

		tuple_type point() const { return tuple_type(); }
		void setPoint(const tuple_type& t) {}
		void setPoint(tuple_type&& t) {}

		frame_data_type frameData() const { return frame_data_type(); }
		void setFrameData(const frame_data_type& t) {}
		void setFrameData(frame_data_type&& t) {}
	};

	TEST_F(PointTest, is_default_constructible)
	{
		EXPECT_TRUE((std::is_default_constructible<double>::value));
		EXPECT_TRUE((std::is_default_constructible<NotAPoint>::value));
		EXPECT_TRUE((std::is_default_constructible<TestPoint>::value));
	}

	TEST_F(PointTest, has_reference_frame)
	{
		EXPECT_FALSE((coord::traits::has_reference_frame<double>::value));
		EXPECT_FALSE((coord::traits::has_reference_frame<NotAPoint>::value));
		EXPECT_TRUE((coord::traits::has_reference_frame<TestPoint>::value));
	}

	TEST_F(PointTest, has_tuple_type)
	{
		EXPECT_FALSE((coord::traits::has_tuple_type<double, coord::traits::point_traits>::value));
		EXPECT_FALSE((coord::traits::has_tuple_type<NotAPoint, coord::traits::point_traits>::value));
		EXPECT_TRUE((coord::traits::has_tuple_type<TestPoint, coord::traits::point_traits>::value));
	}

	TEST_F(PointTest, has_frame_data_type)
	{
		EXPECT_FALSE((coord::traits::has_frame_data_type<double>::value));
		EXPECT_FALSE((coord::traits::has_frame_data_type<NotAPoint>::value));
		EXPECT_TRUE((coord::traits::has_frame_data_type<TestPoint>::value));
	}

	TEST_F(PointTest, has_point)
	{
		EXPECT_FALSE((coord::traits::has_point<double>::value));
		EXPECT_FALSE((coord::traits::has_point<NotAPoint>::value));
		EXPECT_TRUE((coord::traits::has_point<TestPoint>::value));
	}

	TEST_F(PointTest, has_setPoint)
	{
		EXPECT_FALSE((coord::traits::has_setPoint<double>::value));
		EXPECT_FALSE((coord::traits::has_setPoint<NotAPoint>::value));
		EXPECT_TRUE((coord::traits::has_setPoint<TestPoint>::value));
	}

	TEST_F(PointTest, has_frameData)
	{
		EXPECT_FALSE((coord::traits::has_frameData<double>::value));
		EXPECT_FALSE((coord::traits::has_frameData<NotAPoint>::value));
		EXPECT_TRUE((coord::traits::has_frameData<TestPoint>::value));
	}

	TEST_F(PointTest, has_setFrameData)
	{
		EXPECT_FALSE((coord::traits::has_setFrameData<double>::value));
		EXPECT_FALSE((coord::traits::has_setFrameData<NotAPoint>::value));
		EXPECT_TRUE((coord::traits::has_setFrameData<TestPoint>::value));
	}

	TEST_F(PointTest, is_point)
	{
		EXPECT_FALSE((coord::traits::is_point<double>::value));
		EXPECT_FALSE((coord::traits::is_point<NotAPoint>::value));
		EXPECT_TRUE((coord::traits::is_point<TestPoint>::value));
		EXPECT_TRUE((coord::traits::is_point<TestSphericalPoint>::value));
	}

	TEST_F(PointTest, is_cartesian_point)
	{
		EXPECT_FALSE((coord::traits::is_cartesian_point<double>::value));
		EXPECT_FALSE((coord::traits::is_cartesian_point<NotAPoint>::value));
		EXPECT_TRUE((coord::traits::is_cartesian_point<TestPoint>::value));
		EXPECT_FALSE((coord::traits::is_cartesian_point<TestSphericalPoint>::value));
	}

	TEST_F(PointTest, is_convertible_point)
	{
		EXPECT_FALSE((coord::traits::is_convertible_point<double, ECEF>::value));
		EXPECT_FALSE((coord::traits::is_convertible_point<NotAPoint, ECEF>::value));
		EXPECT_TRUE((coord::traits::is_convertible_point<TestPoint, ECEF>::value));
		EXPECT_TRUE((coord::traits::is_convertible_point<ECEF, ECEF>::value));
	}

	TEST_F(PointTest, equal)
	{
		using inch_t = units::length::inch_t;

		PositionECEF<datums::NAD83, units::length::inches> ecef1(12_in, 24_in, 36_in);
		PositionECEF<datums::NAD83, units::length::inches> ecef2(12_in, 24_in, 36_in);
		PositionECEF<datums::NAD83, units::length::feet> ecef3(1_ft, 2_ft, 3_ft);
		ECEF ecef4(0_m, 0_m, 0_m);

		EXPECT_TRUE(ecef1 == ecef2);
		EXPECT_TRUE(ecef1 == ecef3);
		EXPECT_FALSE(ecef1 == ecef4);

		PositionGeodetic<datums::NAD83> geo1(1_deg, 2_deg, 36_in);
		PositionGeodetic<datums::NAD83, units::angle::radians> geo2(1_deg, 2_deg, 36_in);
		PositionGeodetic<datums::NAD83, units::angle::degrees, units::length::feet> geo3(1_deg, 2_deg, 3_ft);
		LLA geo4(0_deg, 0_deg, 0_m);

		EXPECT_TRUE(geo1 == geo2);
		EXPECT_TRUE(geo1 == geo3);
		EXPECT_FALSE(geo1 == geo4);

		PositionENU<datums::NAD83> enu1(0_m, 0_m, 0_m, 42_deg, -71_deg, 0_m);
		PositionENU<datums::NAD83> enu2(0_m, 0_m, 0_m, 42_deg, -71_deg, 0_m);
		PositionENU<datums::NAD83> enu3(1_m, 2_m, 3_m, 42_deg, -71_deg, 0_m);
		PositionENU<datums::NAD83> enu4(0_m, 0_m, 0_m, 34_deg, -118_deg, 0_m);

		EXPECT_TRUE(enu1 == enu2);
		EXPECT_FALSE(enu1 == enu3);
		EXPECT_FALSE(enu1 == enu4);

		ENU enu5(enu1);
		EXPECT_TRUE(enu1 == enu5);
	}

	TEST_F(PointTest, notEqual)
	{
		using inch_t = units::length::inch_t;

		PositionECEF<datums::NAD83, units::length::inches> ecef1(12_in, 24_in, 36_in);
		PositionECEF<datums::NAD83, units::length::inches> ecef2(12_in, 24_in, 36_in);
		PositionECEF<datums::NAD83, units::length::feet> ecef3(1_ft, 2_ft, 3_ft);
		ECEF ecef4(0_m, 0_m, 0_m);

		EXPECT_FALSE(ecef1 != ecef2);
		EXPECT_FALSE(ecef1 != ecef3);
		EXPECT_TRUE(ecef1 != ecef4);

		PositionGeodetic<datums::NAD83> geo1(1_deg, 2_deg, 36_in);
		PositionGeodetic<datums::NAD83, units::angle::radians> geo2(1_deg, 2_deg, 36_in);
		PositionGeodetic<datums::NAD83, units::angle::degrees, units::length::feet> geo3(1_deg, 2_deg, 3_ft);
		LLA geo4(0_deg, 0_deg, 0_m);

		EXPECT_FALSE(geo1 != geo2);
		EXPECT_FALSE(geo1 != geo3);
		EXPECT_TRUE(geo1 != geo4);

		PositionENU<datums::NAD83> enu1(0_m, 0_m, 0_m, 42_deg, -71_deg, 0_m);
		PositionENU<datums::NAD83> enu2(0_m, 0_m, 0_m, 42_deg, -71_deg, 0_m);
		PositionENU<datums::NAD83> enu3(1_m, 2_m, 3_m, 42_deg, -71_deg, 0_m);
		PositionENU<datums::NAD83> enu4(0_m, 0_m, 0_m, 34_deg, -118_deg, 0_m);

		EXPECT_FALSE(enu1 != enu2);
		EXPECT_TRUE(enu1 != enu3);
		EXPECT_TRUE(enu1 != enu4);

		ENU enu5(enu1);
		EXPECT_FALSE(enu1 != enu5);
	}

	TEST_F(PointTest, plus)
	{
		using inch_t = units::length::inch_t;
		using foot_t = units::length::foot_t;

		PositionECEF<datums::NAD83, units::length::inches> ecef1(12_in, 24_in, 36_in);
		PositionECEF<datums::NAD83, units::length::inches> ecef2(36_in, 24_in, 12_in);
		PositionECEF<datums::NAD83, units::length::feet>	ecef_ft(1_ft, 2_ft, 3_ft);

		EXPECT_EQ(12_in, ecef1.x());
		EXPECT_EQ(24_in, ecef1.y());
		EXPECT_EQ(36_in, ecef1.z());

		EXPECT_EQ(36_in, ecef2.x());
		EXPECT_EQ(24_in, ecef2.y());
		EXPECT_EQ(12_in, ecef2.z());

		EXPECT_EQ(1_ft, ecef_ft.x());
		EXPECT_EQ(2_ft, ecef_ft.y());
		EXPECT_EQ(3_ft, ecef_ft.z());

		// same units
		auto test1 = ecef1 + ecef2;

		EXPECT_EQ(48_in, test1.x());
		EXPECT_EQ(48_in, test1.y());
		EXPECT_EQ(48_in, test1.z());

		EXPECT_EQ(12_in, ecef1.x());
		EXPECT_EQ(24_in, ecef1.y());
		EXPECT_EQ(36_in, ecef1.z());

		EXPECT_EQ(36_in, ecef2.x());
		EXPECT_EQ(24_in, ecef2.y());
		EXPECT_EQ(12_in, ecef2.z());

		// different units
		auto test2 = ecef2 + ecef_ft;

		EXPECT_EQ(48_in, test2.x());
		EXPECT_EQ(48_in, test2.y());
		EXPECT_EQ(48_in, test2.z());

		EXPECT_EQ(36_in, ecef2.x());
		EXPECT_EQ(24_in, ecef2.y());
		EXPECT_EQ(12_in, ecef2.z());

		EXPECT_EQ(1_ft, ecef_ft.x());
		EXPECT_EQ(2_ft, ecef_ft.y());
		EXPECT_EQ(3_ft, ecef_ft.z());

		// different units, reversed
		auto test3 = ecef_ft + ecef2;

		EXPECT_EQ(4_ft, test3.x());
		EXPECT_EQ(4_ft, test3.y());
		EXPECT_EQ(4_ft, test3.z());

		EXPECT_EQ(36_in, ecef2.x());
		EXPECT_EQ(24_in, ecef2.y());
		EXPECT_EQ(12_in, ecef2.z());

		EXPECT_EQ(1_ft, ecef_ft.x());
		EXPECT_EQ(2_ft, ecef_ft.y());
		EXPECT_EQ(3_ft, ecef_ft.z());

		LLA Boston(42_deg, -71_deg, 0_m);
		LLA LA(34_deg, -118_deg, 0_m);
		
		ENU enu1(12_m, 24_m, 36_m, Boston);
		ENU enu2(12_m, 24_m, 36_m, Boston);
		ENU enu3(0_m, 0_m, 0_m, LA);
		ENU enu4(12_m, 24_m, 36_m, LA);

		// same units/origin
		auto test4 = enu1 + enu2;
		EXPECT_EQ(24_m, test4.east());
		EXPECT_EQ(48_m, test4.north());
		EXPECT_EQ(72_m, test4.up());
		EXPECT_TRUE(test4.frameData() == enu1.frameData());

		EXPECT_EQ(12_m, enu1.east());
		EXPECT_EQ(24_m, enu1.north());
		EXPECT_EQ(36_m, enu1.up());
		EXPECT_TRUE(enu1.frameData().origin == Boston.point());

		EXPECT_EQ(12_m, enu2.east());
		EXPECT_EQ(24_m, enu2.north());
		EXPECT_EQ(36_m, enu2.up());
		EXPECT_TRUE(enu2.frameData().origin == Boston.point());

		// same units/different origin
		// when dissimilar origins are added, the result is equal to the left-hand side, plus the
		// right-hand side *expressed in the ENU coordinate frame of the left-hand side*. This is *not*
		// equivalent to adding the two points/vectors in ECEF space.
		auto test5 = enu1 + enu3;
		EXPECT_NEAR(-3871232.076971372, test5.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(241254.0467010568, test5.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(-1312751.256969204, test5.up().to<double>(), 5.0e-9);
		EXPECT_TRUE(test5.frameData() == enu1.frameData());

		EXPECT_EQ(12_m, enu1.east());
		EXPECT_EQ(24_m, enu1.north());
		EXPECT_EQ(36_m, enu1.up());
		EXPECT_TRUE(enu1.frameData().origin == Boston.point());

		EXPECT_EQ(12_m, enu2.east());
		EXPECT_EQ(24_m, enu2.north());
		EXPECT_EQ(36_m, enu2.up());
		EXPECT_TRUE(enu2.frameData().origin == Boston.point());

		NED ned1(12_m, 24_m, 36_m, Boston);
		NED ned2(12_m, 24_m, 36_m, Boston);
		NED ned3(0_m, 0_m, 0_m, LA);

		// same units/origin
		auto test6 = ned1 + ned2;
		EXPECT_EQ(24_m, test6.north());
		EXPECT_EQ(48_m, test6.east());
		EXPECT_EQ(72_m, test6.down());
		EXPECT_TRUE(test6.frameData() == enu1.frameData());

		EXPECT_EQ(12_m, ned1.north());
		EXPECT_EQ(24_m, ned1.east());
		EXPECT_EQ(36_m, ned1.down());
		EXPECT_TRUE(ned1.frameData().origin == Boston.point());

		EXPECT_EQ(12_m, ned2.north());
		EXPECT_EQ(24_m, ned2.east());
		EXPECT_EQ(36_m, ned2.down());
		EXPECT_TRUE(ned2.frameData().origin == Boston.point());

		// same units/different origin
		// when dissimilar origins are added, the result is equal to the left-hand side, plus the
		// right-hand side *expressed in the ENU coordinate frame of the left-hand side*. This is *not*
		// equivalent to adding the two points/vectors in ECEF space.
		auto test7 = ned1 + ned3;
		EXPECT_NEAR(241242.0467010568, test7.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(-3871220.076971372, test7.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(1312823.256969204, test7.down().to<double>(), 5.0e-9);
		EXPECT_TRUE(test7.frameData() == ned1.frameData());

		EXPECT_EQ(12_m, ned1.north());
		EXPECT_EQ(24_m, ned1.east());
		EXPECT_EQ(36_m, ned1.down());
		EXPECT_TRUE(ned1.frameData().origin == Boston.point());

		EXPECT_EQ(0_m, ned3.north());
		EXPECT_EQ(0_m, ned3.east());
		EXPECT_EQ(0_m, ned3.down());
		EXPECT_TRUE(ned3.frameData().origin == LA.point());

		// ENU + NED, same origin
		auto test8 = enu1 + ned1;
		EXPECT_NEAR(36, test8.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(36, test8.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(0, test8.up().to<double>(), 5.0e-9);
		EXPECT_TRUE(test8.frameData() == ned1.frameData());

		EXPECT_EQ(12_m, ned1.north());
		EXPECT_EQ(24_m, ned1.east());
		EXPECT_EQ(36_m, ned1.down());
		EXPECT_TRUE(ned1.frameData().origin == Boston.point());

		EXPECT_EQ(12_m, enu1.east());
		EXPECT_EQ(24_m, enu1.north());
		EXPECT_EQ(36_m, enu1.up());
		EXPECT_TRUE(enu1.frameData().origin == Boston.point());

		// NED + ENU, different origin
		auto test9 = ned1 + enu4;
		EXPECT_NEAR(241258.4253779157, test9.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(-3871223.905272964, test9.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(1312781.626655115, test9.down().to<double>(), 5.0e-9);
		EXPECT_TRUE(test9.frameData() == ned1.frameData());

		EXPECT_EQ(12_m, ned1.north());
		EXPECT_EQ(24_m, ned1.east());
		EXPECT_EQ(36_m, ned1.down());
		EXPECT_TRUE(ned1.frameData().origin == Boston.point());

		EXPECT_EQ(12_m, enu4.east());
		EXPECT_EQ(24_m, enu4.north());
		EXPECT_EQ(36_m, enu4.up());
		EXPECT_TRUE(enu4.frameData().origin == LA.point());

	}

	TEST_F(PointTest, minus)
	{
		using inch_t = units::length::inch_t;
		using foot_t = units::length::foot_t;

		PositionECEF<datums::NAD83, units::length::inches> zero;
		PositionECEF<datums::NAD83, units::length::inches> ecef1(12_in, 24_in, 36_in);
		PositionECEF<datums::NAD83, units::length::inches> ecef2(2_in, 4_in, 6_in);
		PositionECEF<datums::NAD83, units::length::feet> ecef_ft(1_ft, 2_ft, 3_ft);

		EXPECT_EQ(12_in, ecef1.x());
		EXPECT_EQ(24_in, ecef1.y());
		EXPECT_EQ(36_in, ecef1.z());

		EXPECT_EQ(2_in, ecef2.x());
		EXPECT_EQ(4_in, ecef2.y());
		EXPECT_EQ(6_in, ecef2.z());

		EXPECT_EQ(1_ft, ecef_ft.x());
		EXPECT_EQ(2_ft, ecef_ft.y());
		EXPECT_EQ(3_ft, ecef_ft.z());

		// same units
		auto test1 = ecef1 - ecef2;

		EXPECT_EQ(10_in, test1.x());
		EXPECT_EQ(20_in, test1.y());
		EXPECT_EQ(30_in, test1.z());

		EXPECT_EQ(12_in, ecef1.x());
		EXPECT_EQ(24_in, ecef1.y());
		EXPECT_EQ(36_in, ecef1.z());

		EXPECT_EQ(2_in, ecef2.x());
		EXPECT_EQ(4_in, ecef2.y());
		EXPECT_EQ(6_in, ecef2.z());

		// different units
		auto test2 = ecef1 - ecef_ft;

		EXPECT_TRUE((test2.isSame(zero, inch_t(1e-14))));

		EXPECT_EQ(12_in, ecef1.x());
		EXPECT_EQ(24_in, ecef1.y());
		EXPECT_EQ(36_in, ecef1.z());

		EXPECT_EQ(1_ft, ecef_ft.x());
		EXPECT_EQ(2_ft, ecef_ft.y());
		EXPECT_EQ(3_ft, ecef_ft.z());

		// different units, reversed
		auto test3 = ecef_ft - ecef1;

		EXPECT_TRUE((test3.isSame(zero, inch_t(1e-14))));

		EXPECT_EQ(12_in, ecef1.x());
		EXPECT_EQ(24_in, ecef1.y());
		EXPECT_EQ(36_in, ecef1.z());

		EXPECT_EQ(1_ft, ecef_ft.x());
		EXPECT_EQ(2_ft, ecef_ft.y());
		EXPECT_EQ(3_ft, ecef_ft.z());

		LLA Boston(42_deg, -71_deg, 0_m);
		LLA LA(34_deg, -118_deg, 0_m);

		ENU enu1(12_m, 24_m, 36_m, Boston);
		ENU enu2(12_m, 24_m, 36_m, Boston);
		ENU enu3(0_m, 0_m, 0_m, LA);
		ENU enu4(12_m, 24_m, 36_m, LA);

		// same units/origin
		auto test4 = enu1 - enu2;
		EXPECT_EQ(0_m, test4.east());
		EXPECT_EQ(0_m, test4.north());
		EXPECT_EQ(0_m, test4.up());
		EXPECT_TRUE(test4.frameData() == enu1.frameData());

		EXPECT_EQ(12_m, enu1.east());
		EXPECT_EQ(24_m, enu1.north());
		EXPECT_EQ(36_m, enu1.up());
		EXPECT_TRUE(enu1.frameData().origin == Boston.point());

		EXPECT_EQ(12_m, enu2.east());
		EXPECT_EQ(24_m, enu2.north());
		EXPECT_EQ(36_m, enu2.up());
		EXPECT_TRUE(enu2.frameData().origin == Boston.point());

		// same units/different origin
		// when dissimilar origins are added, the result is equal to the left-hand side, plus the
		// right-hand side *expressed in the ENU coordinate frame of the left-hand side*. This is *not*
		// equivalent to adding the two points/vectors in ECEF space.
		auto test5 = enu1 - enu3;
		EXPECT_NEAR(3871256.076971372, test5.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(-241206.0467010568, test5.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(1312823.256969204, test5.up().to<double>(), 5.0e-9);
		EXPECT_TRUE(test5.frameData() == enu1.frameData());

		EXPECT_EQ(12_m, enu1.east());
		EXPECT_EQ(24_m, enu1.north());
		EXPECT_EQ(36_m, enu1.up());
		EXPECT_TRUE(enu1.frameData().origin == Boston.point());

		EXPECT_EQ(12_m, enu2.east());
		EXPECT_EQ(24_m, enu2.north());
		EXPECT_EQ(36_m, enu2.up());
		EXPECT_TRUE(enu2.frameData().origin == Boston.point());

		NED ned1(12_m, 24_m, 36_m, Boston);
		NED ned2(12_m, 24_m, 36_m, Boston);
		NED ned3(0_m, 0_m, 0_m, LA);

		// same units/origin
		auto test6 = ned1 - ned2;
		EXPECT_EQ(0_m, test6.north());
		EXPECT_EQ(0_m, test6.east());
		EXPECT_EQ(0_m, test6.down());
		EXPECT_TRUE(test6.frameData() == enu1.frameData());

		EXPECT_EQ(12_m, ned1.north());
		EXPECT_EQ(24_m, ned1.east());
		EXPECT_EQ(36_m, ned1.down());
		EXPECT_TRUE(ned1.frameData().origin == Boston.point());

		EXPECT_EQ(12_m, ned2.north());
		EXPECT_EQ(24_m, ned2.east());
		EXPECT_EQ(36_m, ned2.down());
		EXPECT_TRUE(ned2.frameData().origin == Boston.point());

		// same units/different origin
		// when dissimilar origins are added, the result is equal to the left-hand side, plus the
		// right-hand side *expressed in the ENU coordinate frame of the left-hand side*. This is *not*
		// equivalent to adding the two points/vectors in ECEF space.
		auto test7 = ned1 - ned3;
		EXPECT_NEAR(-241218.0467010568, test7.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(3871268.076971372, test7.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(-1312751.256969204, test7.down().to<double>(), 5.0e-9);
		EXPECT_TRUE(test7.frameData() == ned1.frameData());

		EXPECT_EQ(12_m, ned1.north());
		EXPECT_EQ(24_m, ned1.east());
		EXPECT_EQ(36_m, ned1.down());
		EXPECT_TRUE(ned1.frameData().origin == Boston.point());

		EXPECT_EQ(0_m, ned3.north());
		EXPECT_EQ(0_m, ned3.east());
		EXPECT_EQ(0_m, ned3.down());
		EXPECT_TRUE(ned3.frameData().origin == LA.point());

		// ENU - NED, same origin
		auto test8 = enu1 - ned1;
		EXPECT_NEAR(-12, test8.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(12, test8.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(72, test8.up().to<double>(), 5.0e-9);
		EXPECT_TRUE(test8.frameData() == ned1.frameData());

		EXPECT_EQ(12_m, ned1.north());
		EXPECT_EQ(24_m, ned1.east());
		EXPECT_EQ(36_m, ned1.down());
		EXPECT_TRUE(ned1.frameData().origin == Boston.point());

		EXPECT_EQ(12_m, enu1.east());
		EXPECT_EQ(24_m, enu1.north());
		EXPECT_EQ(36_m, enu1.up());
		EXPECT_TRUE(enu1.frameData().origin == Boston.point());

		// NED - ENU, different origin
		auto test9 = ned1 - enu4;
		EXPECT_NEAR(-241234.4253779157, test9.north().to<double>(), 5.0e-9);
		EXPECT_NEAR(3871271.905272964, test9.east().to<double>(), 5.0e-9);
		EXPECT_NEAR(-1312709.626655115, test9.down().to<double>(), 5.0e-9);
		EXPECT_TRUE(test9.frameData() == ned1.frameData());

		EXPECT_EQ(12_m, ned1.north());
		EXPECT_EQ(24_m, ned1.east());
		EXPECT_EQ(36_m, ned1.down());
		EXPECT_TRUE(ned1.frameData().origin == Boston.point());

		EXPECT_EQ(12_m, enu4.east());
		EXPECT_EQ(24_m, enu4.north());
		EXPECT_EQ(36_m, enu4.up());
		EXPECT_TRUE(enu4.frameData().origin == LA.point());
	}

	TEST_F(PointTest, multiply)
	{
		// ECEF
		using nauticalMile_t = units::length::nauticalMile_t;

		PositionECEF<datums::NAD83, nauticalMiles> ecef1(4_nmi, 4_nmi, 4_nmi);
		EXPECT_EQ(4_nmi, ecef1.x());
		EXPECT_EQ(4_nmi, ecef1.y());
		EXPECT_EQ(4_nmi, ecef1.z());

		// point first
		auto test1 = ecef1 * 4.0;
		EXPECT_EQ(16_nmi, test1.x());
		EXPECT_EQ(16_nmi, test1.y());
		EXPECT_EQ(16_nmi, test1.z());

		// factor first
		auto test2 = 3.0 * ecef1;
		EXPECT_EQ(12_nmi, test2.x());
		EXPECT_EQ(12_nmi, test2.y());
		EXPECT_EQ(12_nmi, test2.z());
		
		// ENU
		LLA Boston(42_deg, -71_deg, 0_m);
		ENU enu1(4_nmi, 4_nmi, 4_nmi, Boston);
		EXPECT_EQ(4_nmi, enu1.east());
		EXPECT_EQ(4_nmi, enu1.north());
		EXPECT_EQ(4_nmi, enu1.up());
		EXPECT_TRUE(enu1.frameData().origin == Boston.point());

		// point first
		auto test3 = enu1 * 4.0;
		EXPECT_EQ(16_nmi, test3.east());
		EXPECT_EQ(16_nmi, test3.north());
		EXPECT_EQ(16_nmi, test3.up());
		EXPECT_TRUE(test3.frameData() == enu1.frameData());

		// factor first
		auto test4 = 3.0 * enu1;
		EXPECT_EQ(12_nmi, test4.east());
		EXPECT_EQ(12_nmi, test4.north());
		EXPECT_EQ(12_nmi, test4.up());
		EXPECT_TRUE(test4.frameData() == enu1.frameData());

		// different origin
		ENU enu2(4_nmi, 4_nmi, 4_nmi, 34_deg, -118_deg, 0_m, 2018_yr);
		auto test5 = 3.0 * enu2;
		EXPECT_EQ(12_nmi, test5.east());
		EXPECT_EQ(12_nmi, test5.north());
		EXPECT_EQ(12_nmi, test5.up());
		EXPECT_TRUE(test5.frameData() == enu2.frameData());

		// NED
		NED ned1(4_nmi, 4_nmi, 4_nmi, Boston);
		EXPECT_EQ(4_nmi, ned1.north());
		EXPECT_EQ(4_nmi, ned1.east());
		EXPECT_EQ(4_nmi, ned1.down());
		EXPECT_TRUE(ned1.frameData().origin == Boston.point());

		// point first
		auto test6 = ned1 * 4.0;
		EXPECT_EQ(16_nmi, test6.north());
		EXPECT_EQ(16_nmi, test6.east());
		EXPECT_EQ(16_nmi, test6.down());
		EXPECT_TRUE(test6.frameData() == ned1.frameData());

		// factor first
		auto test7 = 3.0 * ned1;
		EXPECT_EQ(12_nmi, test7.north());
		EXPECT_EQ(12_nmi, test7.east());
		EXPECT_EQ(12_nmi, test7.down());
		EXPECT_TRUE(test7.frameData() == ned1.frameData());

		// different origin
		NED ned2(4_nmi, 4_nmi, 4_nmi, 34_deg, -118_deg, 0_m, 2018_yr);
		auto test8 = 3.0 * ned2;
		EXPECT_EQ(12_nmi, test8.north());
		EXPECT_EQ(12_nmi, test8.east());
		EXPECT_EQ(12_nmi, test8.down());
		EXPECT_TRUE(test8.frameData() == ned2.frameData());
	}

	TEST_F(PointTest, divide)
	{
		using nauticalMile_t = units::length::nauticalMile_t;

		PositionECEF<datums::NAD83, nauticalMiles> ecef1(4_nmi, 4_nmi, 4_nmi);
		EXPECT_EQ(4_nmi, ecef1.x());
		EXPECT_EQ(4_nmi, ecef1.y());
		EXPECT_EQ(4_nmi, ecef1.z());

		auto test1 = ecef1 / 2.0;
		EXPECT_EQ(2_nmi, test1.x());
		EXPECT_EQ(2_nmi, test1.y());
		EXPECT_EQ(2_nmi, test1.z());

		// ENU
		LLA Boston(42_deg, -71_deg, 0_m);
		ENU enu1(4_nmi, 4_nmi, 4_nmi, Boston);
		EXPECT_EQ(4_nmi, enu1.east());
		EXPECT_EQ(4_nmi, enu1.north());
		EXPECT_EQ(4_nmi, enu1.up());
		EXPECT_TRUE(enu1.frameData().origin == Boston.point());

		// point first
		auto test3 = enu1 / 2.0;
		EXPECT_EQ(2_nmi, test3.east());
		EXPECT_EQ(2_nmi, test3.north());
		EXPECT_EQ(2_nmi, test3.up());
		EXPECT_TRUE(test3.frameData() == enu1.frameData());

		// different origin
		ENU enu2(4_nmi, 4_nmi, 4_nmi, 34_deg, -118_deg, 0_m, 2018_yr);
		auto test4 = enu2 / 2;
		EXPECT_EQ(2_nmi, test4.east());
		EXPECT_EQ(2_nmi, test4.north());
		EXPECT_EQ(2_nmi, test4.up());
		EXPECT_TRUE(test4.frameData() == enu2.frameData());

		// NED
		NED ned1(4_nmi, 4_nmi, 4_nmi, Boston);
		EXPECT_EQ(4_nmi, ned1.north());
		EXPECT_EQ(4_nmi, ned1.east());
		EXPECT_EQ(4_nmi, ned1.down());
		EXPECT_TRUE(ned1.frameData().origin == Boston.point());

		// point first
		auto test5 = ned1 / 2.0;
		EXPECT_EQ(2_nmi, test5.north());
		EXPECT_EQ(2_nmi, test5.east());
		EXPECT_EQ(2_nmi, test5.down());
		EXPECT_TRUE(test5.frameData() == ned1.frameData());

		// different origin
		NED ned2(4_nmi, 4_nmi, 4_nmi, 34_deg, -118_deg, 0_m, 2018_yr);
		auto test6 = ned2 / 2;
		EXPECT_EQ(2_nmi, test6.north());
		EXPECT_EQ(2_nmi, test6.east());
		EXPECT_EQ(2_nmi, test6.down());
		EXPECT_TRUE(test6.frameData() == ned2.frameData());
	}
}

#endif // pointTest_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: