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

#ifndef verticalDatumTest_h__
#define verticalDatumTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <datum.h>

namespace 
{
	// The fixture for testing
	class VerticalDatumTest : public ::testing::Test {
	protected:
		VerticalDatumTest(){}
		~VerticalDatumTest() override {}
		void SetUp() override {}
		void TearDown() override {}
	};

	TEST_F(VerticalDatumTest, is_vertical_datum)
	{
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::ellipsoids::GRS80>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::ellipsoids::WGS84>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::geoids::GEOID12A>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::geoids::NULL_GEOID>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::geoids::EGM96>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::topography::DTED>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::topography::NULL_TOPOGRAPHY>);

		EXPECT_FALSE(coordinates::traits::is_vertical_datum<coordinates::horizontalDatums::IGS08>);
		EXPECT_FALSE(coordinates::traits::is_vertical_datum<coordinates::coordinateFrames::ECEFFrame<coordinates::horizontalDatums::WGS84_G1674>>);
		EXPECT_FALSE(coordinates::traits::is_vertical_datum<double>);
	}

	TEST_F(VerticalDatumTest, vertical_datum_traits)
	{
		EXPECT_TRUE((std::is_same_v<coordinates::traits::vertical_datum_traits<coordinates::geoids::GEOID12A>::base_datum, coordinates::ellipsoids::GRS80>) );
		EXPECT_TRUE((std::is_same_v<coordinates::traits::vertical_datum_traits<coordinates::geoids::EGM96>::base_datum, coordinates::ellipsoids::WGS84>) );
		EXPECT_TRUE((std::is_same_v<coordinates::traits::vertical_datum_traits<coordinates::topography::DTED>::base_datum, coordinates::geoids::EGM96>) );
		EXPECT_TRUE((std::is_same_v<coordinates::traits::vertical_datum_traits<coordinates::ellipsoids::WGS84>::base_datum, coordinates::ellipsoids::WGS84>) );

		EXPECT_TRUE((std::is_same_v<coordinates::traits::vertical_datum_traits<double>::base_datum, void>) );

		EXPECT_UNITS_NEAR(-33.5841_m, coordinates::traits::vertical_datum_traits<coordinates::geoids::GEOID12A>::correctionValue(41.87917_deg, -87.62917_deg), 1.0e-3_m);
		EXPECT_UNITS_NEAR(-33.94_m, coordinates::traits::vertical_datum_traits<coordinates::geoids::EGM96>::correctionValue(41.87917_deg, -87.62917_deg), 1.0e-2_m);
		EXPECT_UNITS_NEAR(0.0_m, coordinates::traits::vertical_datum_traits<coordinates::ellipsoids::WGS84>::correctionValue(41.87917_deg, -87.62917_deg), 1.0e-3_m);
		EXPECT_UNITS_NEAR(191.0_m, coordinates::traits::vertical_datum_traits<coordinates::topography::DTED>::correctionValue(41.87917_deg, -87.62917_deg), 1.0e-2_m);	// non-interpolated. Truth from `gdallocationinfo -wgs84 n41.dt2 -87.62917 41.87917`
	}

	TEST_F(VerticalDatumTest, convertToEllipsoidHeight)
	{
		// truth data from VDatum: http://vdatum.noaa.gov/download_agreement.php
		EXPECT_UNITS_NEAR(12.0_m, coordinates::convertToEllipsoidHeight<coordinates::ellipsoids::WGS84>(41.87917_deg, -87.62917_deg, 12.0_m), 1.5e-4_m);
		EXPECT_UNITS_NEAR(-33.5841_m, coordinates::convertToEllipsoidHeight<coordinates::geoids::GEOID12A>(41.87917_deg, -87.62917_deg, 0_m), 1.5e-4_m);
	
		// no truth data available for this test... the result seems reasonable.
		EXPECT_UNITS_NEAR(157.056_m, coordinates::convertToEllipsoidHeight<coordinates::topography::DTED>(41.87917_deg, -87.62917_deg, 0_m), 5.0e-4_m);
	}

	TEST_F(VerticalDatumTest, convertFromEllipsoidHeight)
	{
		// truth data from VDatum: http://vdatum.noaa.gov/download_agreement.php
		EXPECT_UNITS_NEAR(12.0_m, coordinates::convertFromEllipsoidHeight<coordinates::ellipsoids::WGS84>(41.87917_deg, -87.62917_deg, 12.0_m), 1.5e-4_m);
		EXPECT_UNITS_NEAR(0_m, coordinates::convertFromEllipsoidHeight<coordinates::geoids::GEOID12A>(41.87917_deg, -87.62917_deg, -33.5841_m), 1.5e-4_m);

		// no truth data available for this test... the result seems reasonable.
		EXPECT_UNITS_NEAR(0_m, coordinates::convertFromEllipsoidHeight<coordinates::topography::DTED>(41.87917_deg, -87.62917_deg, 157.056_m), 5.0e-4_m);
	}
}

#endif // verticalDatumTest_h__