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

using namespace coordinates;

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
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF2008>);
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF2014>);
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF2005>);
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF2000>);
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF97>);
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF96>);
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF94>);
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF93>);
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF92>);
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF91>);
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF89>);
		EXPECT_TRUE(coordinates::traits::is_itrf<horizontalDatums::ITRF88>);

		EXPECT_FALSE(coordinates::traits::is_itrf<horizontalDatums::WGS84_G1674>);
		EXPECT_FALSE(coordinates::traits::is_itrf<horizontalDatums::NAD83>);
		EXPECT_FALSE(coordinates::traits::is_itrf<horizontalDatums::ETRS89>);
		EXPECT_FALSE(coordinates::traits::is_itrf<horizontalDatums::ETRF2000>);
		EXPECT_FALSE(coordinates::traits::is_itrf<horizontalDatums::GDA94>);

		EXPECT_FALSE(coordinates::traits::is_itrf<double>);
	}

	TEST_F(HorizontalDatumTest, is_horizontal_datum)
	{
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF2008>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF2014>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF2005>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF2000>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF97>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF96>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF94>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF93>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF92>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF91>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF89>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ITRF88>);

		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::WGS84_G1674>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::NAD83>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ETRS89>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::ETRF2000>);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<horizontalDatums::GDA94>);

		EXPECT_FALSE(coordinates::traits::is_horizontal_datum<double>);
	}
}
#endif // horizontalDatumTest_h__