#ifndef verticalDatumTest_h__
#define verticalDatumTest_h__

//------------------------
//	INCLUDES
//------------------------

namespace 
{
	// The fixture for testing
	class VerticalDatumTest : public ::testing::Test {
	protected:
		VerticalDatumTest(){}
		virtual ~VerticalDatumTest(){}
		virtual void SetUp(){}
		virtual void TearDown(){}
	};

	TEST_F(VerticalDatumTest, is_vertical_datum)
	{
		EXPECT_TRUE(coord::traits::is_vertical_datum<coord::ellipsoids::GRS80>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<coord::ellipsoids::WGS84>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<coord::geoids::GEOID12A>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<coord::geoids::NULL_GEOID>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<coord::geoids::EGM96>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<coord::topography::DTED>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<coord::topography::NULL_TOPOGRAPHY>::value);

		EXPECT_FALSE(coord::traits::is_vertical_datum<coord::horizontalDatums::IGS08>::value);
		EXPECT_FALSE(coord::traits::is_vertical_datum<coord::coordinateFrames::ECEFFrame<coord::horizontalDatums::WGS84_G1674>>::value);
		EXPECT_FALSE(coord::traits::is_vertical_datum<double>::value);
	}

	TEST_F(VerticalDatumTest, vertical_datum_traits)
	{
		EXPECT_TRUE((std::is_same<coord::traits::vertical_datum_traits<coord::geoids::GEOID12A>::base_datum, coord::ellipsoids::GRS80>::value));
		EXPECT_TRUE((std::is_same<coord::traits::vertical_datum_traits<coord::geoids::EGM96>::base_datum, coord::ellipsoids::WGS84>::value));
		EXPECT_TRUE((std::is_same<coord::traits::vertical_datum_traits<coord::topography::DTED>::base_datum, coord::geoids::EGM96>::value));
		EXPECT_TRUE((std::is_same<coord::traits::vertical_datum_traits<coord::ellipsoids::WGS84>::base_datum, coord::ellipsoids::WGS84>::value));

		EXPECT_TRUE((std::is_same<coord::traits::vertical_datum_traits<double>::base_datum, void>::value));

		EXPECT_NEAR(-33.5841, (coord::traits::vertical_datum_traits<coord::geoids::GEOID12A>::correctionValue(41.87917_deg, -87.62917_deg).to<double>()), 1.0e-3);
		EXPECT_NEAR(-33.94, (coord::traits::vertical_datum_traits<coord::geoids::EGM96>::correctionValue(41.87917_deg, -87.62917_deg).to<double>()), 1.0e-2);
		EXPECT_NEAR(0, (coord::traits::vertical_datum_traits<coord::ellipsoids::WGS84>::correctionValue(41.87917_deg, -87.62917_deg).to<double>()), 1.0e-3);
		EXPECT_NEAR(191, (coord::traits::vertical_datum_traits<coord::topography::DTED>::correctionValue(41.87917_deg, -87.62917_deg).to<double>()), 1.0e-2);	// non-interpolated. Truth from `gdallocationinfo -wgs84 n41.dt2 -87.62917 41.87917`
	}

	TEST_F(VerticalDatumTest, convertToEllipsoidHeight)
	{
		// truth data from VDatum: http://vdatum.noaa.gov/download_agreement.php
		EXPECT_NEAR(12.0, (coord::convertToEllipsoidHeight<coord::ellipsoids::WGS84>(41.87917_deg, -87.62917_deg, 12.0_m).to<double>()), 1.5e-4);
		EXPECT_NEAR(-33.5841, (coord::convertToEllipsoidHeight<coord::geoids::GEOID12A>(41.87917_deg, -87.62917_deg, 0_m).to<double>()), 1.5e-4);
	
		// no truth data available for this test... the result seems reasonable.
		EXPECT_NEAR(157.066, (coord::convertToEllipsoidHeight<coord::topography::DTED>(41.87917_deg, -87.62917_deg, 0_m).to<double>()), 5.0e-4);
	}

	TEST_F(VerticalDatumTest, convertFromEllipsoidHeight)
	{
		// truth data from VDatum: http://vdatum.noaa.gov/download_agreement.php
		EXPECT_NEAR(12.0, (coord::convertFromEllipsoidHeight<coord::ellipsoids::WGS84>(41.87917_deg, -87.62917_deg, 12.0_m).to<double>()), 1.5e-4);
		EXPECT_NEAR(0, (coord::convertFromEllipsoidHeight<coord::geoids::GEOID12A>(41.87917_deg, -87.62917_deg, -33.5841_m).to<double>()), 1.5e-4);

		// no truth data available for this test... the result seems reasonable.
		EXPECT_NEAR(0, (coord::convertFromEllipsoidHeight<coord::topography::DTED>(41.87917_deg, -87.62917_deg, 157.066_m).to<double>()), 5.0e-4);
	}
}

#endif // verticalDatumTest_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: