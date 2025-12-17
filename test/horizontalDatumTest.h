#ifndef horizontalDatumTest_h__
#define horizontalDatumTest_h__

//------------------------
//	INCLUDES
//------------------------

using namespace coord;

namespace
{
	// The fixture for testing
	class HorizontalDatumTest : public ::testing::Test {
	protected:
		HorizontalDatumTest(){}
		virtual ~HorizontalDatumTest(){}
		virtual void SetUp(){}
		virtual void TearDown(){}
	};

	TEST_F(HorizontalDatumTest, is_itrf)
	{
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF2008>::value);
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF2014>::value);
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF2005>::value);
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF2000>::value);
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF97>::value);
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF96>::value);
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF94>::value);
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF93>::value);
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF92>::value);
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF91>::value);
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF89>::value);
		EXPECT_TRUE(coord::traits::is_itrf<horizontalDatums::ITRF88>::value);

		EXPECT_FALSE(coord::traits::is_itrf<horizontalDatums::WGS84_G1674>::value);
		EXPECT_FALSE(coord::traits::is_itrf<horizontalDatums::NAD83>::value);
		EXPECT_FALSE(coord::traits::is_itrf<horizontalDatums::ETRS89>::value);
		EXPECT_FALSE(coord::traits::is_itrf<horizontalDatums::ETRF2000>::value);
		EXPECT_FALSE(coord::traits::is_itrf<horizontalDatums::GDA94>::value);

		EXPECT_FALSE(coord::traits::is_itrf<double>::value);
	}

	TEST_F(HorizontalDatumTest, is_horizontal_datum)
	{
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF2008>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF2014>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF2005>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF2000>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF97>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF96>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF94>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF93>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF92>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF91>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF89>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ITRF88>::value);

		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::WGS84_G1674>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::NAD83>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ETRS89>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::ETRF2000>::value);
		EXPECT_TRUE(coord::traits::is_horizontal_datum<horizontalDatums::GDA94>::value);

		EXPECT_FALSE(coord::traits::is_horizontal_datum<double>::value);
	}
}
#endif // horizontalDatumTest_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: