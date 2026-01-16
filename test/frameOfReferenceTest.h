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

#ifndef frameOfReferenceTest_h__
#define frameOfReferenceTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <initializer_list>
#include <stdexcept>
#include <type_traits>

using namespace coordinates::coordinateFrames;

namespace
{
	// The fixture for testing class Foo.
	class FrameOfReferenceTest : public ::testing::Test {
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		FrameOfReferenceTest()
		{
			// You can do set-up work for each test here.

		}

		virtual ~FrameOfReferenceTest()
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

		struct NotAFrame
		{
			typedef int		base_frame_type;
			typedef int		tuple_type;
		};
	};

	// The fixture for testing
	class FrameConversionTest : public ::testing::Test {
	protected:
		FrameConversionTest(){}
		virtual ~FrameConversionTest(){}
		virtual void SetUp(){}
		virtual void TearDown(){}
	};

	TEST_F(FrameOfReferenceTest, has_base_frame_type)
	{
		EXPECT_TRUE(coordinates::traits::has_base_frame_type<ECEFFrame<horizontalDatums::ITRF2008>>);
		EXPECT_TRUE(coordinates::traits::has_base_frame_type<ECEFFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_base_frame_type<Geodetic2DFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_base_frame_type<Geodetic3DFrame<datums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_base_frame_type<ENUFrame<horizontalDatums::WGS84_G1674>>);

		EXPECT_FALSE(coordinates::traits::has_base_frame_type<double>);
		EXPECT_FALSE(coordinates::traits::has_base_frame_type<ellipsoids::GRS80>);
		EXPECT_FALSE(coordinates::traits::has_base_frame_type<NotAFrame>);
	}

	TEST_F(FrameOfReferenceTest, has_datum_type)
	{
		EXPECT_TRUE(coordinates::traits::has_datum_type<ECEFFrame<horizontalDatums::ITRF2008>>);
		EXPECT_TRUE(coordinates::traits::has_datum_type<ECEFFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_datum_type<Geodetic2DFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_datum_type<Geodetic3DFrame<datums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_datum_type<ENUFrame<horizontalDatums::WGS84_G1674>>);

		EXPECT_FALSE(coordinates::traits::has_datum_type<double>);
		EXPECT_FALSE(coordinates::traits::has_datum_type<ellipsoids::GRS80>);
		EXPECT_FALSE(coordinates::traits::has_datum_type<NotAFrame>);
	}

	TEST_F(FrameOfReferenceTest, has_tuple_type)
	{
		EXPECT_TRUE(coordinates::traits::has_tuple_type<ECEFFrame<horizontalDatums::ITRF2008>>);
		EXPECT_TRUE(coordinates::traits::has_base_frame_type<ECEFFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_tuple_type<Geodetic2DFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_tuple_type<Geodetic3DFrame<datums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_tuple_type<ENUFrame<horizontalDatums::WGS84_G1674>>);

		EXPECT_FALSE(coordinates::traits::has_tuple_type<double>);
		EXPECT_FALSE(coordinates::traits::has_tuple_type<ellipsoids::GRS80>);
		EXPECT_FALSE(coordinates::traits::has_tuple_type<NotAFrame>);
	}

	TEST_F(FrameOfReferenceTest, has_convertFromBaseFrame)
	{
		EXPECT_TRUE(coordinates::traits::has_convertFromBaseFrame<ECEFFrame<horizontalDatums::ITRF2008>>);
		EXPECT_TRUE(coordinates::traits::has_base_frame_type<ECEFFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_convertFromBaseFrame<Geodetic2DFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_convertFromBaseFrame<Geodetic3DFrame<datums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_convertFromBaseFrame<ENUFrame<horizontalDatums::WGS84_G1674>>);

		EXPECT_FALSE(coordinates::traits::has_convertFromBaseFrame<double>);
		EXPECT_FALSE(coordinates::traits::has_convertFromBaseFrame<ellipsoids::GRS80>);
		EXPECT_FALSE(coordinates::traits::has_convertFromBaseFrame<NotAFrame>);
	}

	TEST_F(FrameOfReferenceTest, has_convertToBaseFrame)
	{
		EXPECT_TRUE(coordinates::traits::has_convertToBaseFrame<ECEFFrame<horizontalDatums::ITRF2008>>);
		EXPECT_TRUE(coordinates::traits::has_base_frame_type<ECEFFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_convertToBaseFrame<Geodetic2DFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_convertToBaseFrame<Geodetic3DFrame<datums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::has_convertToBaseFrame<ENUFrame<horizontalDatums::WGS84_G1674>>);

		EXPECT_FALSE(coordinates::traits::has_convertToBaseFrame<double>);
		EXPECT_FALSE(coordinates::traits::has_convertToBaseFrame<ellipsoids::GRS80>);
		EXPECT_FALSE(coordinates::traits::has_convertToBaseFrame<NotAFrame>);
	}

	TEST_F(FrameOfReferenceTest, is_frame_of_reference)
	{
		EXPECT_TRUE(coordinates::traits::is_frame_of_reference<ECEFFrame<horizontalDatums::ITRF2008>>);
		EXPECT_TRUE(coordinates::traits::has_base_frame_type<ECEFFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::is_frame_of_reference<Geodetic2DFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::is_frame_of_reference<Geodetic3DFrame<datums::WGS84_G1674>>);
		EXPECT_TRUE(coordinates::traits::is_frame_of_reference<ENUFrame<horizontalDatums::WGS84_G1674>>);

		EXPECT_FALSE(coordinates::traits::is_frame_of_reference<double>);
		EXPECT_FALSE(coordinates::traits::is_frame_of_reference<ellipsoids::GRS80>);
		EXPECT_FALSE(coordinates::traits::is_frame_of_reference<NotAFrame>);
	}

	TEST_F(FrameOfReferenceTest, is_base_frame)
	{
		EXPECT_TRUE(coordinates::traits::is_base_frame<ECEFFrame<horizontalDatums::ITRF2008>>);
		EXPECT_FALSE(coordinates::traits::is_base_frame<ECEFFrame<horizontalDatums::ITRF2014>>);
		EXPECT_FALSE(coordinates::traits::is_base_frame<ECEFFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_FALSE(coordinates::traits::is_base_frame<Geodetic2DFrame<horizontalDatums::WGS84_G1674>>);
		EXPECT_FALSE(coordinates::traits::is_base_frame<Geodetic3DFrame<datums::WGS84_G1674>>);
		EXPECT_FALSE(coordinates::traits::is_base_frame<ENUFrame<horizontalDatums::WGS84_G1674>>);

		EXPECT_FALSE(coordinates::traits::is_base_frame<double>);
		EXPECT_FALSE(coordinates::traits::is_base_frame<ellipsoids::GRS80>);
		EXPECT_FALSE(coordinates::traits::is_base_frame<NotAFrame>);
	}

	TEST_F(FrameOfReferenceTest, lowest_base_frame)
	{
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::ITRF2008>,
			std::decay_t<typename coordinates::traits::lowest_base_frame<ECEFFrame<horizontalDatums::ITRF2008>>::type>>::value));
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::ITRF2008>,
			typename std::decay<typename coordinates::traits::lowest_base_frame<ECEFFrame<horizontalDatums::ITRF2014>>::type>::type>::value));
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::ITRF2008>,
			typename std::decay<typename coordinates::traits::lowest_base_frame<ECEFFrame<horizontalDatums::WGS84_G1674>>::type>::type>::value));
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::ITRF2008>,
			typename std::decay<typename coordinates::traits::lowest_base_frame<Geodetic2DFrame<horizontalDatums::WGS84_G1674>>::type>::type>::value));
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::ITRF2008>,
			typename std::decay<typename coordinates::traits::lowest_base_frame<Geodetic3DFrame<datums::WGS84_G1674>>::type>::type>::value));
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::ITRF2008>,
			typename std::decay<typename coordinates::traits::lowest_base_frame<ENUFrame<horizontalDatums::WGS84_G1674>>::type>::type>::value));
	}

	TEST_F(FrameOfReferenceTest, is_convertible_frame)
	{
		EXPECT_TRUE((coordinates::traits::is_convertible_frame<coordinateFrames::ECEFFrame<horizontalDatums::ITRF2008>, ECEFFrame<horizontalDatums::ITRF2014>>));
		EXPECT_TRUE((coordinates::traits::is_convertible_frame<coordinateFrames::ECEFFrame<horizontalDatums::ITRF2008>, ECEFFrame<horizontalDatums::WGS84_G1674>>));
		EXPECT_TRUE((coordinates::traits::is_convertible_frame<coordinateFrames::ECEFFrame<horizontalDatums::ITRF2008>, Geodetic2DFrame<horizontalDatums::WGS84_G1674>>));
		EXPECT_TRUE((coordinates::traits::is_convertible_frame<coordinateFrames::ECEFFrame<horizontalDatums::ITRF2014>, Geodetic2DFrame<horizontalDatums::WGS84_G1674>>));
		EXPECT_TRUE((coordinates::traits::is_convertible_frame<coordinateFrames::ECEFFrame<horizontalDatums::ITRF2014>, Geodetic3DFrame<datums::WGS84_G1674>>));
		EXPECT_TRUE((coordinates::traits::is_convertible_frame<coordinateFrames::ECEFFrame<horizontalDatums::NAD83>, ECEFFrame<horizontalDatums::WGS84_G1674>>));
		EXPECT_TRUE((coordinates::traits::is_convertible_frame<coordinateFrames::Geodetic2DFrame<horizontalDatums::NAD83>, Geodetic2DFrame<horizontalDatums::WGS84_G1674>>));
		EXPECT_TRUE((coordinates::traits::is_convertible_frame<coordinateFrames::Geodetic3DFrame<datums::NAD83>, Geodetic3DFrame<datums::WGS84_G1674>>));
		EXPECT_TRUE((coordinates::traits::is_convertible_frame<coordinateFrames::ENUFrame<horizontalDatums::NAD83>, ECEFFrame<horizontalDatums::WGS84_G1674>>));
		EXPECT_TRUE((coordinates::traits::is_convertible_frame<coordinateFrames::ENUFrame<horizontalDatums::NAD83>, Geodetic2DFrame<horizontalDatums::WGS84_G1674>>));
		EXPECT_TRUE((coordinates::traits::is_convertible_frame<coordinateFrames::ENUFrame<horizontalDatums::NAD83>, Geodetic3DFrame<datums::WGS84_G1674>>));

		EXPECT_FALSE((coordinates::traits::is_convertible_frame<double, double>));
		EXPECT_FALSE((coordinates::traits::is_convertible_frame<double, Geodetic2DFrame<horizontalDatums::WGS84_G1674>>));
		EXPECT_FALSE((coordinates::traits::is_convertible_frame<NotAFrame, NotAFrame>));
		EXPECT_FALSE((coordinates::traits::is_convertible_frame<NotAFrame, coordinateFrames::ECEFFrame<horizontalDatums::ITRF2008>>));
		EXPECT_FALSE((coordinates::traits::is_convertible_frame<NotAFrame, coordinateFrames::ECEFFrame<horizontalDatums::WGS84_G1674>>));
		EXPECT_FALSE((coordinates::traits::is_convertible_frame<NotAFrame, coordinateFrames::Geodetic2DFrame<horizontalDatums::WGS84_G1674>>));
		EXPECT_FALSE((coordinates::traits::is_convertible_frame<NotAFrame, coordinateFrames::Geodetic2DFrame<horizontalDatums::NAD83>>));
		EXPECT_FALSE((coordinates::traits::is_convertible_frame<NotAFrame, coordinateFrames::Geodetic3DFrame<datums::WGS84_G1674>>));
		EXPECT_FALSE((coordinates::traits::is_convertible_frame<NotAFrame, coordinateFrames::Geodetic3DFrame<datums::NAD83>>));
		EXPECT_FALSE((coordinates::traits::is_convertible_frame<NotAFrame, coordinateFrames::ENUFrame<horizontalDatums::ITRF2008>>));
	}

	TEST_F(FrameOfReferenceTest, is_cartesian_frame)
	{
		EXPECT_FALSE(coordinates::traits::is_cartesian_frame<double>);
		EXPECT_FALSE(coordinates::traits::is_cartesian_frame<ellipsoids::GRS80>);
		EXPECT_FALSE(coordinates::traits::is_cartesian_frame<NotAFrame>);
		EXPECT_TRUE(coordinates::traits::is_cartesian_frame<ECEFFrame<datums::IGS08>>);
		EXPECT_TRUE(coordinates::traits::is_cartesian_frame<ECEFFrame<datums::NAD83>>);
		EXPECT_TRUE(coordinates::traits::is_cartesian_frame<ECEFFrame<datums::WGS84_G1674>>);
		EXPECT_FALSE(coordinates::traits::is_cartesian_frame<Geodetic2DFrame<datums::NAD83>>);
		EXPECT_FALSE(coordinates::traits::is_cartesian_frame<Geodetic3DFrame<datums::NAD83>>);
		EXPECT_FALSE(coordinates::traits::is_cartesian_frame<Geodetic3DFrame<datums::NAD83_MSL>>);
		EXPECT_TRUE(coordinates::traits::is_cartesian_frame<ENUFrame<datums::NAD83>>);
		EXPECT_TRUE(coordinates::traits::is_cartesian_frame<NEDFrame<datums::WGS84_G1674>>);
		EXPECT_FALSE(coordinates::traits::is_cartesian_frame<AERFrame<datums::NAD83>>);
	}

	TEST_F(FrameOfReferenceTest, least_common_ancestor)
	{
		// same frame
		EXPECT_TRUE((std::is_same<Geodetic3DFrame<datums::NAD83>, typename coordinates::traits::least_common_ancestor<Geodetic3DFrame<datums::NAD83>, Geodetic3DFrame<datums::NAD83>>::type>::value));

		// different datums
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::ITRF2008>, typename coordinates::traits::least_common_ancestor<Geodetic3DFrame<datums::NAD83>, Geodetic3DFrame<datums::WGS84_G1674>>::type>::value));
		
		// different frames and datums
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::ITRF2008>, typename coordinates::traits::least_common_ancestor<Geodetic2DFrame<horizontalDatums::NAD83>, Geodetic3DFrame<datums::WGS84_G1674>>::type>::value));
		
		// different frames
		EXPECT_TRUE((std::is_same<Geodetic2DFrame<horizontalDatums::NAD83>, typename coordinates::traits::least_common_ancestor<Geodetic2DFrame<horizontalDatums::NAD83>, Geodetic3DFrame<datums::NAD83>>::type>::value));
		
		// one of the two frames IS the LCA
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::NAD83>, typename coordinates::traits::least_common_ancestor<ENUFrame<horizontalDatums::NAD83>, Geodetic3DFrame<datums::NAD83>>::type>::value));
		
		// different vertical frames
		EXPECT_TRUE((std::is_same<Geodetic2DFrame<horizontalDatums::WGS84_G1674>, typename coordinates::traits::least_common_ancestor<Geodetic3DFrame<datums::WGS84_G1674_MSL>, Geodetic3DFrame<datums::WGS84_G1674_AGL>>::type>::value));
	}

	TEST_F(FrameOfReferenceTest, least_common_cartesian_ancestor)
	{
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::ITRF2008>, typename coordinates::traits::least_common_cartesian_ancestor<Geodetic3DFrame<datums::NAD83>, Geodetic3DFrame<datums::WGS84_G1674>>::type>::value));
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::ITRF2008>, typename coordinates::traits::least_common_cartesian_ancestor<Geodetic2DFrame<horizontalDatums::NAD83>, Geodetic3DFrame<datums::WGS84_G1674>>::type>::value));
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::NAD83>, typename coordinates::traits::least_common_cartesian_ancestor<Geodetic2DFrame<horizontalDatums::NAD83>, Geodetic3DFrame<datums::NAD83>>::type>::value));
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::NAD83>, typename coordinates::traits::least_common_cartesian_ancestor<ENUFrame<horizontalDatums::NAD83>, Geodetic3DFrame<datums::NAD83>>::type>::value));
		EXPECT_TRUE((std::is_same<ECEFFrame<horizontalDatums::WGS84_G1674>, typename coordinates::traits::least_common_cartesian_ancestor<Geodetic3DFrame<datums::WGS84_G1674>, Geodetic3DFrame<datums::WGS84_G1674>>::type>::value));
		EXPECT_TRUE((std::is_same<ENUFrame<datums::WGS84_G1674>, typename coordinates::traits::least_common_cartesian_ancestor<ENUFrame<datums::WGS84_G1674>, ENUFrame<datums::WGS84_G1674>>::type>::value));
		EXPECT_TRUE((std::is_same<NEDFrame<datums::WGS84_G1674>, typename coordinates::traits::least_common_cartesian_ancestor<NEDFrame<datums::WGS84_G1674>, NEDFrame<datums::WGS84_G1674>>::type>::value));
		EXPECT_TRUE((std::is_same<NEDFrame<datums::WGS84_G1674>, typename coordinates::traits::least_common_cartesian_ancestor<AERFrame<datums::WGS84_G1674>, AERFrame<datums::WGS84_G1674>>::type>::value));
	}

	TEST_F(FrameConversionTest, same)
	{
			// convert to self
		const std::tuple sameECEF(1513460.904_m, -4463118.573_m, 4283412.811_m);
		EXPECT_EQ(sameECEF, (coordinates::convert<ECEFFrame<horizontalDatums::ITRF2008>, ECEFFrame<horizontalDatums::ITRF2008>>(sameECEF)));
		EXPECT_EQ(sameECEF, (coordinates::convert<ECEFFrame<horizontalDatums::ITRF2014>, ECEFFrame<horizontalDatums::ITRF2014>>(sameECEF)));
		EXPECT_EQ(sameECEF, (coordinates::convert<ECEFFrame<horizontalDatums::NAD83>, ECEFFrame<horizontalDatums::NAD83>>(sameECEF)));
		EXPECT_EQ(sameECEF, (coordinates::convert<ECEFFrame<horizontalDatums::WGS84_G1674>, ECEFFrame<horizontalDatums::WGS84_G1674>>(sameECEF)));

		const std::tuple sameGEO(42.459284_deg, -71.268002_deg, 50.0_m);
		EXPECT_EQ(sameGEO, (coordinates::convert<Geodetic2DFrame<horizontalDatums::NAD83>, Geodetic2DFrame<horizontalDatums::NAD83>>(sameGEO)));
		EXPECT_EQ(sameGEO, (coordinates::convert<Geodetic2DFrame<horizontalDatums::WGS84_G1674>, Geodetic2DFrame<horizontalDatums::WGS84_G1674>>(sameGEO)));
		EXPECT_EQ(sameGEO, (coordinates::convert<Geodetic3DFrame<datums::NAD83>, Geodetic3DFrame<datums::NAD83>>(sameGEO)));
		EXPECT_EQ(sameGEO, (coordinates::convert<Geodetic3DFrame<datums::WGS84_G1674>, Geodetic3DFrame<datums::WGS84_G1674>>(sameGEO)));
	}

	TEST_F(FrameConversionTest, IRTF2008_to_ITRFXX)
	{
		std::tuple itrf2008(1513460.904_m, -4463118.573_m, 4283412.811_m);

		// Convert ITRF. Truth data: http://webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php?locale=en (make sure 'Longitude Positive West' is unchecked)
		// Note: NOAA's VDatum seems to perform these transformations INCORECTLY, so don't use it!
		std::tuple itrf2005_expected(1513460.903_m, -4463118.578_m, 4283412.810_m);
		auto itrf2005_calculated = coordinates::convert<ECEFFrame<horizontalDatums::ITRF2008>, ECEFFrame<horizontalDatums::ITRF2005>>(itrf2008);
		EXPECT_NEAR(std::get<0>(itrf2005_expected).to<double>(), std::get<0>(itrf2005_calculated).to<double>(), 5.0e-4);
		EXPECT_NEAR(std::get<1>(itrf2005_expected).to<double>(), std::get<1>(itrf2005_calculated).to<double>(), 5.0e-4);
		EXPECT_NEAR(std::get<2>(itrf2005_expected).to<double>(), std::get<2>(itrf2005_calculated).to<double>(), 5.0e-4);

		std::tuple itrf2000_expected(1513460.904_m, -4463118.581_m, 4283412.806_m);
		auto itrf2000_calculated = coordinates::convert<ECEFFrame<horizontalDatums::ITRF2008>, ECEFFrame<horizontalDatums::ITRF2000>>(itrf2008);
		EXPECT_NEAR(std::get<0>(itrf2000_expected).to<double>(), std::get<0>(itrf2000_calculated).to<double>(), 5.0e-4);
		EXPECT_NEAR(std::get<1>(itrf2000_expected).to<double>(), std::get<1>(itrf2000_calculated).to<double>(), 5.0e-4);
		EXPECT_NEAR(std::get<2>(itrf2000_expected).to<double>(), std::get<2>(itrf2000_calculated).to<double>(), 5.0e-4);

		std::tuple itrf96_expected(1513460.915_m, -4463118.583_m, 4283412.790_m);
		auto itrf96_calculated = coordinates::convert<ECEFFrame<horizontalDatums::ITRF2008>, ECEFFrame<horizontalDatums::ITRF96>>(itrf2008);
		EXPECT_NEAR(std::get<0>(itrf96_expected).to<double>(), std::get<0>(itrf96_calculated).to<double>(), 5.0e-4);
		EXPECT_NEAR(std::get<1>(itrf96_expected).to<double>(), std::get<1>(itrf96_calculated).to<double>(), 5.0e-4);
		EXPECT_NEAR(std::get<2>(itrf96_expected).to<double>(), std::get<2>(itrf96_calculated).to<double>(), 5.0e-4);

		std::tuple itrf88_expected(1513460.944_m, -4463118.618_m, 4283412.728_m);
		auto itrf88_calculated = coordinates::convert<ECEFFrame<horizontalDatums::ITRF2008>, ECEFFrame<horizontalDatums::ITRF88>>(itrf2008);
		EXPECT_NEAR(std::get<0>(itrf88_expected).to<double>(), std::get<0>(itrf88_calculated).to<double>(), 5.0e-4);
		EXPECT_NEAR(std::get<1>(itrf88_expected).to<double>(), std::get<1>(itrf88_calculated).to<double>(), 5.0e-4);
		EXPECT_NEAR(std::get<2>(itrf88_expected).to<double>(), std::get<2>(itrf88_calculated).to<double>(), 5.0e-4);
	}

	TEST_F(FrameConversionTest, IRTFXX_to_ITRFYY)
	{
		std::tuple itrf(1513460.904_m, -4463118.573_m, 4283412.811_m);

		// Convert ITRF. Truth data: http://webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php?locale=en (make sure 'Longitude Positive West' is unchecked)
		// Note: NOAA's VDatum seems to perform these transformations INCORECTLY, so don't use it!
		std::tuple itrf90_expected(1513460.923_m, -4463118.565_m, 4283412.785_m);
		auto itrf90_calculated = coordinates::convert<ECEFFrame<horizontalDatums::ITRF96>, ECEFFrame<horizontalDatums::ITRF90>>(itrf);
		EXPECT_NEAR(std::get<0>(itrf90_expected).to<double>(), std::get<0>(itrf90_calculated).to<double>(), 1.0e-3);
		EXPECT_NEAR(std::get<1>(itrf90_expected).to<double>(), std::get<1>(itrf90_calculated).to<double>(), 1.0e-3);
		EXPECT_NEAR(std::get<2>(itrf90_expected).to<double>(), std::get<2>(itrf90_calculated).to<double>(), 1.0e-3);

		std::tuple itrf88_expected(1513460.944_m, -4463118.610_m, 4283412.733_m);
		auto itrf88_calculated = coordinates::convert<ECEFFrame<horizontalDatums::ITRF2000>, ECEFFrame<horizontalDatums::ITRF88>>(itrf);
		EXPECT_NEAR(std::get<0>(itrf88_expected).to<double>(), std::get<0>(itrf88_calculated).to<double>(), 1.0e-3);
		EXPECT_NEAR(std::get<1>(itrf88_expected).to<double>(), std::get<1>(itrf88_calculated).to<double>(), 1.0e-3);
		EXPECT_NEAR(std::get<2>(itrf88_expected).to<double>(), std::get<2>(itrf88_calculated).to<double>(), 1.0e-3);
	}

	TEST_F(FrameConversionTest, WGS84_LLA_to_ECEF)
	{
		// truth data from: Louis' matlab calculations

		auto test1 = std::tuple(42.459284_deg, -71.268002_deg, 50.0_m);
		auto test2 = std::tuple(8.72053_deg, 167.730482_deg, 500.0_m);
		auto test3 = std::tuple(-33.932048_deg, 18.397849_deg, 1000.0_m);
		auto test4 = std::tuple<degrees<>, degrees<>, meters<>>(40.417638_deg, -3.719343_deg, 0.0_km);

		auto result1 = coordinates::convert<Geodetic3DFrame<datums::WGS84_G1674>, ECEFFrame<horizontalDatums::WGS84_G1674>>(test1);
		auto result2 = coordinates::convert<Geodetic3DFrame<datums::WGS84_G1674>, ECEFFrame<horizontalDatums::WGS84_G1674>>(test2);
		auto result3 = coordinates::convert<Geodetic3DFrame<datums::WGS84_G1674>, ECEFFrame<horizontalDatums::WGS84_G1674>>(test3);
		auto result4 = coordinates::convert<Geodetic3DFrame<datums::WGS84_G1674>, ECEFFrame<horizontalDatums::WGS84_G1674>>(test4);

		EXPECT_NEAR(1513460.90425574, std::get<0>(result1).to<double>(), 1.0e-8);
		EXPECT_NEAR(-4463118.57366578, std::get<1>(result1).to<double>(), 1.0e-8);
		EXPECT_NEAR(4283412.81104103, std::get<2>(result1).to<double>(), 1.0e-8);

		EXPECT_NEAR(-6161359.99808945, std::get<0>(result2).to<double>(), 1.0e-8);
		EXPECT_NEAR(1339960.38688496, std::get<1>(result2).to<double>(), 1.0e-8);
		EXPECT_NEAR(960697.380382176, std::get<2>(result2).to<double>(), 1.0e-8);

		EXPECT_NEAR(5027491.76199413, std::get<0>(result3).to<double>(), 1.0e-8);
		EXPECT_NEAR(1672214.28779206, std::get<1>(result3).to<double>(), 1.0e-8);
		EXPECT_NEAR(-3540753.52360714, std::get<2>(result3).to<double>(), 1.0e-8);

		EXPECT_NEAR(4852527.699157, std::get<0>(result4).to<double>(), 1.0e-8);
		EXPECT_NEAR(-315443.985333516, std::get<1>(result4).to<double>(), 1.0e-8);
		EXPECT_NEAR(4113401.13079432, std::get<2>(result4).to<double>(), 1.0e-8);
	}

	TEST_F(FrameConversionTest, WGS84_ECEF_to_LLA)
	{
		// truth data from: Louis' matlab calculations

		auto test1 = std::tuple(1513460.90425574_m, -4463118.57366578_m, 4283412.81104103_m);
		auto test2 = std::tuple(-6161359.99808945_m, 1339960.38688496_m, 960697.380382176_m);
		auto test3 = std::tuple(5027491.76199413_m, 1672214.28779206_m, -3540753.52360714_m);
		auto test4 = std::tuple(4852.527699157_km, -315.443985333516_km, 4113.40113079432_km);

		auto result1 = coordinates::convert<ECEFFrame<horizontalDatums::WGS84_G1674>, Geodetic3DFrame<datums::WGS84_G1674>>(test1);
		auto result2 = coordinates::convert<ECEFFrame<horizontalDatums::WGS84_G1674>, Geodetic3DFrame<datums::WGS84_G1674>>(test2);
		auto result3 = coordinates::convert<ECEFFrame<horizontalDatums::WGS84_G1674>, Geodetic3DFrame<datums::WGS84_G1674>>(test3);
		auto result4 = coordinates::convert<ECEFFrame<horizontalDatums::WGS84_G1674>, Geodetic3DFrame<datums::WGS84_G1674>>(test4);

		EXPECT_NEAR(42.459284, std::get<0>(result1).to<double>(), 1.0e-8);
		EXPECT_NEAR(-71.268002, std::get<1>(result1).to<double>(), 1.0e-8);
		EXPECT_NEAR(50.0, std::get<2>(result1).to<double>(), 1.0e-8);

		EXPECT_NEAR(8.72053, std::get<0>(result2).to<double>(), 1.0e-8);
		EXPECT_NEAR(167.730482, std::get<1>(result2).to<double>(), 1.0e-8);
		EXPECT_NEAR(500.0, std::get<2>(result2).to<double>(), 1.0e-8);

		EXPECT_NEAR(-33.932048, std::get<0>(result3).to<double>(), 1.0e-8);
		EXPECT_NEAR(18.397849, std::get<1>(result3).to<double>(), 1.0e-8);
		EXPECT_NEAR(1000, std::get<2>(result3).to<double>(), 1.0e-8);

		EXPECT_NEAR(40.417638, std::get<0>(result4).to<double>(), 1.0e-8);
		EXPECT_NEAR(-3.719343, std::get<1>(result4).to<double>(), 1.0e-8);
		EXPECT_NEAR(0.0, std::get<2>(result4).to<double>(), 1.0e-8);
	}

	TEST_F(FrameConversionTest, WGS84_to_NAD83)
	{
		// Datum conversions
		// Horizontal are from: http://tagis.dep.wv.gov/convert/
		// Vertical are from: http://vdatum.noaa.gov/welcome.html are are a bit suspect (since they don't match the
		// tagis horizontal values). They're probably only accurate to one decimal place.
		auto boston_wgs84 = std::tuple(42.354991_deg, -71.065599_deg, -1.2192_m);
		auto nevada_wgs84 = std::tuple(37.235000_deg, -115.811100_deg, -0.6969_m);
		auto boston_nad83 = std::tuple(42.354982_deg, -71.065598_deg, 0.0_m);
		auto nevada_nad83 = std::tuple(37.234995_deg, -115.811088_deg, 0.0_m);

		auto bostonCalculated = coordinates::convert<Geodetic2DFrame<horizontalDatums::WGS84_G1674>, Geodetic2DFrame<horizontalDatums::NAD83>>(boston_wgs84);

		EXPECT_NEAR(std::get<0>(boston_nad83).to<double>(), std::get<0>(bostonCalculated).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<1>(boston_nad83).to<double>(), std::get<1>(bostonCalculated).to<double>(), 5.0e-7);

		auto nevadaCalculated = coordinates::convert<Geodetic2DFrame<horizontalDatums::WGS84_G1674>, Geodetic2DFrame<horizontalDatums::NAD83>>(nevada_wgs84);

		EXPECT_NEAR(std::get<0>(nevada_nad83).to<double>(), std::get<0>(nevadaCalculated).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<1>(nevada_nad83).to<double>(), std::get<1>(nevadaCalculated).to<double>(), 5.0e-7);

		auto bostonCalculated3D = coordinates::convert<Geodetic3DFrame<datums::WGS84_G1674>, Geodetic3DFrame<datums::NAD83>>(boston_wgs84);

		EXPECT_NEAR(std::get<0>(boston_nad83).to<double>(), std::get<0>(bostonCalculated3D).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<1>(boston_nad83).to<double>(), std::get<1>(bostonCalculated3D).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<2>(boston_nad83).to<double>(), std::get<2>(bostonCalculated3D).to<double>(), 5.0e-2);

		auto nevadaCalculated3D = coordinates::convert<Geodetic3DFrame<datums::WGS84_G1674>, Geodetic3DFrame<datums::NAD83>>(nevada_wgs84);

		EXPECT_NEAR(std::get<0>(nevada_nad83).to<double>(), std::get<0>(nevadaCalculated3D).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<1>(nevada_nad83).to<double>(), std::get<1>(nevadaCalculated3D).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<2>(nevada_nad83).to<double>(), std::get<2>(nevadaCalculated3D).to<double>(), 5.0e-2);
	}

	TEST_F(FrameConversionTest, NAD83_to_WGS84)
	{
		// Datum conversions
		// Horizontal are from: http://tagis.dep.wv.gov/convert/
		// Vertical are from: http://vdatum.noaa.gov/welcome.html are are a bit suspect (since they don't match the
		// tagis horizontal values). They're probably only accurate to one decimal place.
		auto boston_wgs84 = std::tuple(42.354991_deg, -71.065599_deg, -1.2192_m);
		auto nevada_wgs84 = std::tuple(37.235000_deg, -115.811100_deg, -0.6969_m);
		auto boston_nad83 = std::tuple(42.354982_deg, -71.065598_deg, 0.0_m);
		auto nevada_nad83 = std::tuple(37.234995_deg, -115.811088_deg, 0.0_m);

		auto bostonCalculated = coordinates::convert<Geodetic2DFrame<horizontalDatums::NAD83>, Geodetic2DFrame<horizontalDatums::WGS84_G1674>>(boston_nad83);

		EXPECT_NEAR(std::get<0>(boston_wgs84).to<double>(), std::get<0>(bostonCalculated).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<1>(boston_wgs84).to<double>(), std::get<1>(bostonCalculated).to<double>(), 5.0e-7);

		auto nevadaCalculated = coordinates::convert<Geodetic2DFrame<horizontalDatums::NAD83>, Geodetic2DFrame<horizontalDatums::WGS84_G1674>>(nevada_nad83);

		EXPECT_NEAR(std::get<0>(nevada_wgs84).to<double>(), std::get<0>(nevadaCalculated).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<1>(nevada_wgs84).to<double>(), std::get<1>(nevadaCalculated).to<double>(), 5.0e-7);

		auto bostonCalculated3D = coordinates::convert<Geodetic3DFrame<datums::NAD83>, Geodetic3DFrame<datums::WGS84_G1674>>(boston_nad83);

		EXPECT_NEAR(std::get<0>(boston_wgs84).to<double>(), std::get<0>(bostonCalculated3D).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<1>(boston_wgs84).to<double>(), std::get<1>(bostonCalculated3D).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<2>(boston_wgs84).to<double>(), std::get<2>(bostonCalculated3D).to<double>(), 5.0e-2);

		auto nevadaCalculated3D = coordinates::convert<Geodetic3DFrame<datums::NAD83>, Geodetic3DFrame<datums::WGS84_G1674>>(nevada_nad83);

		EXPECT_NEAR(std::get<0>(nevada_wgs84).to<double>(), std::get<0>(nevadaCalculated3D).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<1>(nevada_wgs84).to<double>(), std::get<1>(nevadaCalculated3D).to<double>(), 5.0e-7);
		EXPECT_NEAR(std::get<2>(nevada_wgs84).to<double>(), std::get<2>(nevadaCalculated3D).to<double>(), 5.0e-2);
	}

	TEST_F(FrameConversionTest, ENU_to_ECEF)
	{
		// truth data from Louis' matlab
		auto origin = std::tuple(42.459284_deg, -71.268002_deg, 50_m);

		auto test1 = CartesianTuple(0_m,0_m,0_m);
		auto test2 = CartesianTuple(-5404681.93164781_m, 2922440.82470195_m,  -8115897.48471693_m);
		auto test3 = CartesianTuple(5298209.06171991_m, -2611778.32868034_m, -8735896.96152653_m);
		auto test4 = CartesianTuple(4494192.24378046_m, 1802333.53686265_m, -2221554.73053747_m);

		auto expected1 = CartesianTuple(1513460.90425574_m, -4463118.57366578_m, 4283412.81104103_m);
		auto expected2 = CartesianTuple(-6161359.99808944_m, 1339960.38688495_m, 960697.380382177_m);
		auto expected3 = CartesianTuple(5027491.76199413_m, 1672214.28779206_m, -3540753.52360714_m);
		auto expected4 = CartesianTuple(4852527.699157_m, -315443.985333514_m, 4113401.13079431_m);

		auto result1 = coordinates::convert<ENUFrame<horizontalDatums::WGS84_G1674>, ECEFFrame<horizontalDatums::WGS84_G1674>>(test1, origin, origin);
		auto result2 = coordinates::convert<ENUFrame<horizontalDatums::WGS84_G1674>, ECEFFrame<horizontalDatums::WGS84_G1674>>(test2, origin, origin);
		auto result3 = coordinates::convert<ENUFrame<horizontalDatums::WGS84_G1674>, ECEFFrame<horizontalDatums::WGS84_G1674>>(test3, origin, origin);
		auto result4 = coordinates::convert<ENUFrame<horizontalDatums::WGS84_G1674>, ECEFFrame<horizontalDatums::WGS84_G1674>>(test4, origin, origin);

		EXPECT_NEAR(std::get<0>(expected1).to<double>(), std::get<0>(result1).to<double>(), 1.0e-8);
		EXPECT_NEAR(std::get<1>(expected1).to<double>(), std::get<1>(result1).to<double>(), 1.0e-8);
		EXPECT_NEAR(std::get<2>(expected1).to<double>(), std::get<2>(result1).to<double>(), 1.0e-8);

		EXPECT_NEAR(std::get<0>(expected2).to<double>(), std::get<0>(result2).to<double>(), 1.0e-8);
		EXPECT_NEAR(std::get<1>(expected2).to<double>(), std::get<1>(result2).to<double>(), 1.0e-8);
		EXPECT_NEAR(std::get<2>(expected2).to<double>(), std::get<2>(result2).to<double>(), 1.0e-8);

		EXPECT_NEAR(std::get<0>(expected3).to<double>(), std::get<0>(result3).to<double>(), 1.0e-8);
		EXPECT_NEAR(std::get<1>(expected3).to<double>(), std::get<1>(result3).to<double>(), 1.0e-8);
		EXPECT_NEAR(std::get<2>(expected3).to<double>(), std::get<2>(result3).to<double>(), 1.0e-8);
			
		EXPECT_NEAR(std::get<0>(expected4).to<double>(), std::get<0>(result4).to<double>(), 1.0e-8);
		EXPECT_NEAR(std::get<1>(expected4).to<double>(), std::get<1>(result4).to<double>(), 1.0e-8);
		EXPECT_NEAR(std::get<2>(expected4).to<double>(), std::get<2>(result4).to<double>(), 1.0e-8);
	}

	TEST_F(FrameConversionTest, ECEF_to_ENU)
	{
		// truth data from Louis' matlab
		auto origin = std::tuple(42.459284_deg, -71.268002_deg, 50_m);
		auto origin2 = std::tuple(42.459284_deg, -71.268002_deg, 50000_m);	// boston @ 50km

		auto test1 = CartesianTuple(1513460.90425574_m, -4463118.57366578_m, 4283412.81104103_m);
		auto test2 = CartesianTuple(-6161359.99808944_m, 1339960.38688495_m, 960697.380382177_m);
		auto test3 = CartesianTuple(5027491.76199413_m, 1672214.28779206_m, -3540753.52360714_m);
		auto test4 = CartesianTuple(4852527.699157_m, -315443.985333514_m, 4113401.13079431_m);
		auto test5 = CartesianTuple(1516998.391159392_m, -4463557.703477143_m, 4282381.812770258_m); //lexington @ 500m

		auto expected1 = CartesianTuple(0_m, 0_m, 0_m);
		auto expected2 = CartesianTuple(-5404681.93164781_m, 2922440.82470195_m, -8115897.48471693_m);
		auto expected3 = CartesianTuple(5298209.06171991_m, -2611778.32868034_m, -8735896.96152653_m);
		auto expected4 = CartesianTuple(4494192.24378046_m, 1802333.53686265_m, -2221554.73053747_m);
		auto expected5 = CartesianTuple(3209.087037877383_m, -1808.264874640016_m, -49501.06287128705_m);

		auto result1 = coordinates::convert<ECEFFrame<horizontalDatums::WGS84_G1674>, ENUFrame<horizontalDatums::WGS84_G1674>>(test1, origin, origin);
		auto result2 = coordinates::convert<ECEFFrame<horizontalDatums::WGS84_G1674>, ENUFrame<horizontalDatums::WGS84_G1674>>(test2, origin, origin);
		auto result3 = coordinates::convert<ECEFFrame<horizontalDatums::WGS84_G1674>, ENUFrame<horizontalDatums::WGS84_G1674>>(test3, origin, origin);
		auto result4 = coordinates::convert<ECEFFrame<horizontalDatums::WGS84_G1674>, ENUFrame<horizontalDatums::WGS84_G1674>>(test4, origin, origin);
		auto result5 = coordinates::convert<ECEFFrame<horizontalDatums::WGS84_G1674>, ENUFrame<horizontalDatums::WGS84_G1674>>(test5, origin2, origin2);

		EXPECT_NEAR(std::get<0>(expected1).to<double>(), std::get<0>(result1).to<double>(), 5.0e-9);
		EXPECT_NEAR(std::get<1>(expected1).to<double>(), std::get<1>(result1).to<double>(), 5.0e-9);
		EXPECT_NEAR(std::get<2>(expected1).to<double>(), std::get<2>(result1).to<double>(), 5.12e-9);

		EXPECT_NEAR(std::get<0>(expected2).to<double>(), std::get<0>(result2).to<double>(), 5.0e-9);
		EXPECT_NEAR(std::get<1>(expected2).to<double>(), std::get<1>(result2).to<double>(), 5.0e-9);
		EXPECT_NEAR(std::get<2>(expected2).to<double>(), std::get<2>(result2).to<double>(), 5.0e-9);

		EXPECT_NEAR(std::get<0>(expected3).to<double>(), std::get<0>(result3).to<double>(), 5.0e-9);
		EXPECT_NEAR(std::get<1>(expected3).to<double>(), std::get<1>(result3).to<double>(), 5.0e-9);
		EXPECT_NEAR(std::get<2>(expected3).to<double>(), std::get<2>(result3).to<double>(), 5.0e-9);

		EXPECT_NEAR(std::get<0>(expected4).to<double>(), std::get<0>(result4).to<double>(), 5.0e-9);
		EXPECT_NEAR(std::get<1>(expected4).to<double>(), std::get<1>(result4).to<double>(), 5.0e-9);
		EXPECT_NEAR(std::get<2>(expected4).to<double>(), std::get<2>(result4).to<double>(), 5.0e-9);

		EXPECT_NEAR(std::get<0>(expected5).to<double>(), std::get<0>(result5).to<double>(), 5.0e-9);
		EXPECT_NEAR(std::get<1>(expected5).to<double>(), std::get<1>(result5).to<double>(), 5.0e-9);
		EXPECT_NEAR(std::get<2>(expected5).to<double>(), std::get<2>(result5).to<double>(), 5.0e-9);
	}

	TEST_F(FrameConversionTest, ENU_to_NED)
	{
		auto test = coordinates::CartesianTuple(1_m, 2_m, 3_m);

		auto expected = coordinates::CartesianTuple(2_m, 1_m, -3_m);
		auto result = coordinates::convert<ENUFrame<horizontalDatums::WGS84_G1674>, NEDFrame<horizontalDatums::WGS84_G1674>>(test);

		EXPECT_NEAR(std::get<0>(expected).to<double>(), std::get<0>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<1>(expected).to<double>(), std::get<1>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<2>(expected).to<double>(), std::get<2>(result).to<double>(), 5.0e-10);
	}

	TEST_F(FrameConversionTest, NED_to_NED)
	{
		auto test = coordinates::CartesianTuple(2_m, 1_m, -3_m);

		auto expected = coordinates::CartesianTuple(1_m, 2_m, 3_m);
		auto result = coordinates::convert<NEDFrame<horizontalDatums::WGS84_G1674>, ENUFrame<horizontalDatums::WGS84_G1674>>(test);

		EXPECT_NEAR(std::get<0>(expected).to<double>(), std::get<0>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<1>(expected).to<double>(), std::get<1>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<2>(expected).to<double>(), std::get<2>(result).to<double>(), 5.0e-10);
	}

	TEST_F(FrameConversionTest, GEO_to_NED)
	{
		// truth data from MATLAB: [n, e, d] = geodetic2ned(39, -76, 12000, 39, -75, 4000, wgs84Ellipsoid)
		auto test = coordinates::SphericalTuple(39_deg, -76_deg, 12000_m);
		auto origin = coordinates::SphericalTuple(39_deg, -75_deg, 4000_m);

		auto expected = coordinates::CartesianTuple(4.766215572076285e+02_m, -8.678476280073922e+04_m, -7.411421394260990e+03_m);
		auto result = coordinates::convert<Geodetic2DFrame<horizontalDatums::WGS84_G1674>, NEDFrame<horizontalDatums::WGS84_G1674>>(test, origin, origin);

		EXPECT_NEAR(std::get<0>(expected).to<double>(), std::get<0>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<1>(expected).to<double>(), std::get<1>(result).to<double>(), 7.0e-10);
		EXPECT_NEAR(std::get<2>(expected).to<double>(), std::get<2>(result).to<double>(), 7.0e-10);
	}

	TEST_F(FrameConversionTest, NED_TO_GEO)
	{
		// truth data from MATLAB: [n, e, d] = geodetic2ned(39, -76, 12000, 39, -75, 4000, wgs84Ellipsoid)
		auto test = coordinates::CartesianTuple(4.766215572076285e+02_m, -8.678476280073922e+04_m, -7.411421394260990e+03_m);
		auto origin = coordinates::SphericalTuple(39_deg, -75_deg, 4000_m);

		auto expected = coordinates::SphericalTuple(39_deg, -76_deg, 12000_m);
		auto result = coordinates::convert<NEDFrame<horizontalDatums::WGS84_G1674>, Geodetic2DFrame<horizontalDatums::WGS84_G1674>>(test, origin, origin);

		EXPECT_NEAR(std::get<0>(expected).to<double>(), std::get<0>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<1>(expected).to<double>(), std::get<1>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<2>(expected).to<double>(), std::get<2>(result).to<double>(), 5.0e-6);
	}

	TEST_F(FrameConversionTest, AER_to_NED)
	{
		// truth data from MATLAB: [n,e,d] = aer2ned(270, 5, 85000)
		auto test = coordinates::SphericalTuple(270_deg, 5_deg, 85000_m);
		auto origin = coordinates::SphericalTuple(39_deg, -75_deg, 4000_m);

		auto expected = coordinates::CartesianTuple(0_m, -8.467654933779837e+04_m, -7.408238133550944e+03_m);
		auto result = coordinates::convert<AERFrame<horizontalDatums::WGS84_G1674>, NEDFrame<horizontalDatums::WGS84_G1674>>(test, origin, origin);

		EXPECT_NEAR(std::get<0>(expected).to<double>(), std::get<0>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<1>(expected).to<double>(), std::get<1>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<2>(expected).to<double>(), std::get<2>(result).to<double>(), 5.0e-10);
	}

	TEST_F(FrameConversionTest, NED_TO_AER)
	{
		// truth data from MATLAB: [n,e,d] = aer2ned(270, 5, 85000)
		auto test = coordinates::CartesianTuple(0_m, -8.467654933779837e+04_m, -7.408238133550944e+03_m);
		auto origin = coordinates::SphericalTuple(39_deg, -75_deg, 4000_m);

		auto expected = coordinates::SphericalTuple(270_deg, 5_deg, 85000_m);
		auto result = coordinates::convert<NEDFrame<horizontalDatums::WGS84_G1674>, AERFrame<horizontalDatums::WGS84_G1674>>(test, origin, origin);

		EXPECT_NEAR(std::get<0>(expected).to<double>(), std::get<0>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<1>(expected).to<double>(), std::get<1>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<2>(expected).to<double>(), std::get<2>(result).to<double>(), 5.0e-10);
	}

	TEST_F(FrameConversionTest, ECEF_to_AER)
	{
		// truth data from MATLAB: [az, el, range] = geodetic2aer(39, -76, 12000, 39, -75, 4000, wgs84Ellipsoid)
		auto testGeo = coordinates::SphericalTuple(39_deg, -76_deg, 12000_m);
		auto origin = coordinates::SphericalTuple(39_deg, -75_deg, 4000_m);

		auto test = coordinates::convert<Geodetic2DFrame<horizontalDatums::WGS84_G1674>, ECEFFrame<horizontalDatums::WGS84_G1674>>(testGeo, origin, origin);
		auto expected = coordinates::SphericalTuple(270.3146650197312_deg, 4.881144545764371_deg, 87101.95973439791_m);
		auto result = coordinates::convert<ECEFFrame<horizontalDatums::WGS84_G1674>, AERFrame<horizontalDatums::WGS84_G1674>>(test, origin, origin);

		EXPECT_NEAR(std::get<0>(expected).to<double>(), std::get<0>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<1>(expected).to<double>(), std::get<1>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<2>(expected).to<double>(), std::get<2>(result).to<double>(), 7.0e-10);
	}

	TEST_F(FrameConversionTest, AER_to_ECEF)
	{
		// truth data from MATLAB: [x,y,z] = aer2ecef(270, 5, 85000, 39, -75, 4000, wgs84Ellipsoid)
		auto test = coordinates::SphericalTuple(270_deg, 5_deg, 85000_m);
		auto origin = coordinates::SphericalTuple(39_deg, -75_deg, 4000_m);

		auto expected = coordinates::CartesianTuple(1.205107031443989e+06_m, -4.824685733453152e+06_m, 3.999496459635123e+06_m);
		auto result = coordinates::convert<AERFrame<horizontalDatums::WGS84_G1674>, ECEFFrame<horizontalDatums::WGS84_G1674>>(test, origin);

		EXPECT_NEAR(std::get<0>(expected).to<double>(), std::get<0>(result).to<double>(), 5.0e-10);
		EXPECT_NEAR(std::get<1>(expected).to<double>(), std::get<1>(result).to<double>(), 5.0e-9);
		EXPECT_NEAR(std::get<2>(expected).to<double>(), std::get<2>(result).to<double>(), 5.0e-10);
	}
}
#endif // frameOfReferenceTest_h__