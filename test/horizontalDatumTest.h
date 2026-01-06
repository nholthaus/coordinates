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