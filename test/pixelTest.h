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

#ifndef pixelTest_h
#define pixelTest_h

//------------------------
//	INCLUDES
//------------------------

#include <gtest/gtest.h>

#include <units.h>

#include "pixel.h"

inline namespace coordinates
{
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	CLASS		PixelTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for the `Pixel` raster address struct and the `pixels` unit.
	//  ----------------------------------------------------------------------------
	class PixelTest : public ::testing::Test
	{
	};

	// A default-constructed pixel addresses the top-left corner (row 0, column 0).
	TEST_F(PixelTest, defaultIsOrigin)
	{
		const Pixel p;
		EXPECT_EQ(p.row, 0);
		EXPECT_EQ(p.column, 0);
	}

	// Aggregate initialization sets row then column, in that order.
	TEST_F(PixelTest, aggregateInitializesRowThenColumn)
	{
		const Pixel p{3, 7};
		EXPECT_EQ(p.row, 3);
		EXPECT_EQ(p.column, 7);
	}

	// The members are independently assignable after construction.
	TEST_F(PixelTest, membersAreAssignable)
	{
		Pixel p;
		p.row    = 12;
		p.column = 34;
		EXPECT_EQ(p.row, 12);
		EXPECT_EQ(p.column, 34);
	}

	// The off-image sentinel {-1, -1} the camera returns is a representable pixel value.
	TEST_F(PixelTest, offImageSentinelIsRepresentable)
	{
		const Pixel sentinel{-1, -1};
		EXPECT_EQ(sentinel.row, -1);
		EXPECT_EQ(sentinel.column, -1);
	}

	// The `pixels` unit carries a raster measurement as a typed quantity, convertible to a bare double.
	TEST_F(PixelTest, pixelsUnitCarriesRasterMeasurement)
	{
		const auto extent = 512.0_px;
		EXPECT_EQ(extent.to<double>(), 512.0);
	}

	// The `pixels` unit adds within its own dimension.
	TEST_F(PixelTest, pixelsUnitAdds)
	{
		const auto total = 100.0_px + 28.0_px;
		EXPECT_EQ(total.to<double>(), 128.0);
	}
}    // namespace coordinates

#endif    // pixelTest_h
