#ifndef datumTest_h__
#define datumTest_h__

//------------------------
//	INCLUDES
//------------------------

using namespace coord;

namespace
{
	// The fixture for testing
	class DatumTest : public ::testing::Test {
	protected:
		DatumTest(){}
		virtual ~DatumTest(){}
		virtual void SetUp(){}
		virtual void TearDown(){}
	};
	
	TEST_F(DatumTest, is_datum)
	{
		EXPECT_TRUE(coord::traits::is_datum<datums::WGS84_G1674>::value);
		EXPECT_TRUE(coord::traits::is_datum<datums::IGS08>::value);
		EXPECT_TRUE(coord::traits::is_datum<datums::NAD83>::value);
		EXPECT_TRUE(coord::traits::is_datum<datums::NAD83_MSL>::value);
		
		EXPECT_FALSE(coord::traits::is_datum<horizontalDatums::WGS84_G1674>::value);
		EXPECT_FALSE(coord::traits::is_datum<horizontalDatums::IGS08>::value);
		EXPECT_FALSE(coord::traits::is_datum<horizontalDatums::NAD83>::value);
		
		EXPECT_FALSE(coord::traits::is_datum<ellipsoids::GRS80>::value);
		EXPECT_FALSE(coord::traits::is_datum<geoids::GEOID12A>::value);
		EXPECT_FALSE(coord::traits::is_datum<topography::DTED>::value);
	}
	
	TEST_F(DatumTest, is_horizontal_datum)
	{
		// a datum is also a horizontal datum
		EXPECT_TRUE(coord::traits::is_horizontal_datum<datums::WGS84_G1674>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<datums::WGS84_G1674_MSL>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<datums::WGS84_G1674_AGL>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<datums::IGS08>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<datums::NAD83>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<datums::NAD83_MSL>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<datums::NAD83_NAVD88>::value);
	}
	
	TEST_F(DatumTest, is_vertical_datum)
	{
		// a datum is also a horizontal datum
		EXPECT_TRUE(coord::traits::is_vertical_datum<datums::WGS84_G1674>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<datums::WGS84_G1674_MSL>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<datums::WGS84_G1674_AGL>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<datums::IGS08>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<datums::NAD83>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<datums::NAD83_MSL>::value);
		EXPECT_TRUE(coord::traits::is_vertical_datum<datums::NAD83_NAVD88>::value);
	}
}

#endif // datumTest_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: