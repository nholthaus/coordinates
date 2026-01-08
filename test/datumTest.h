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

#ifndef datumTest_h__
#define datumTest_h__

//------------------------
//	INCLUDES
//------------------------

using namespace coordinates;

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
		EXPECT_TRUE(coordinates::traits::is_datum<datums::WGS84_G1674>::value);
		EXPECT_TRUE(coordinates::traits::is_datum<datums::IGS08>::value);
		EXPECT_TRUE(coordinates::traits::is_datum<datums::NAD83>::value);
		EXPECT_TRUE(coordinates::traits::is_datum<datums::NAD83_MSL>::value);
		
		EXPECT_FALSE(coordinates::traits::is_datum<horizontalDatums::WGS84_G1674>::value);
		EXPECT_FALSE(coordinates::traits::is_datum<horizontalDatums::IGS08>::value);
		EXPECT_FALSE(coordinates::traits::is_datum<horizontalDatums::NAD83>::value);
		
		EXPECT_FALSE(coordinates::traits::is_datum<ellipsoids::GRS80>::value);
		EXPECT_FALSE(coordinates::traits::is_datum<geoids::GEOID12A>::value);
		EXPECT_FALSE(coordinates::traits::is_datum<topography::DTED>::value);
	}
	
	TEST_F(DatumTest, is_horizontal_datum)
	{
		// a datum is also a horizontal datum
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<datums::WGS84_G1674>::value);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<datums::WGS84_G1674_MSL>::value);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<datums::WGS84_G1674_AGL>::value);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<datums::IGS08>::value);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<datums::NAD83>::value);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<datums::NAD83_MSL>::value);
		EXPECT_TRUE(coordinates::traits::is_horizontal_datum<datums::NAD83_NAVD88>::value);
	}
	
	TEST_F(DatumTest, is_vertical_datum)
	{
		// a datum is also a horizontal datum
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<datums::WGS84_G1674>::value);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<datums::WGS84_G1674_MSL>::value);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<datums::WGS84_G1674_AGL>::value);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<datums::IGS08>::value);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<datums::NAD83>::value);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<datums::NAD83_MSL>::value);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<datums::NAD83_NAVD88>::value);
	}
}

#endif // datumTest_h__