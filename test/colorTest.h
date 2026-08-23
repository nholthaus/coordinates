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

#ifndef colorTest_h
#define colorTest_h

//------------------------
//	INCLUDES
//------------------------

#include <cstdint>

#include <gtest/gtest.h>

#include "color.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		ColorTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for the `Color` RGB struct and the `channel` intensity conversion.
	//  ----------------------------------------------------------------------------
	class ColorTest : public ::testing::Test
	{
	};

	// A default-constructed color is opaque black: every channel zero.
	TEST_F(ColorTest, defaultIsBlack)
	{
		const Color c;
		EXPECT_EQ(c.r, 0u);
		EXPECT_EQ(c.g, 0u);
		EXPECT_EQ(c.b, 0u);
	}

	// Aggregate initialization sets each channel in red-green-blue order.
	TEST_F(ColorTest, aggregateInitializesChannelsInOrder)
	{
		const Color c{10, 20, 30};
		EXPECT_EQ(c.r, 10u);
		EXPECT_EQ(c.g, 20u);
		EXPECT_EQ(c.b, 30u);
	}

	// A partial aggregate fills the trailing channels with their zero defaults.
	TEST_F(ColorTest, partialAggregateDefaultsTrailingChannels)
	{
		const Color c{200};
		EXPECT_EQ(c.r, 200u);
		EXPECT_EQ(c.g, 0u);
		EXPECT_EQ(c.b, 0u);
	}

	// Channels are independently assignable after construction.
	TEST_F(ColorTest, channelsAreAssignable)
	{
		Color c;
		c.r = 1;
		c.g = 2;
		c.b = 3;
		EXPECT_EQ(c.r, 1u);
		EXPECT_EQ(c.g, 2u);
		EXPECT_EQ(c.b, 3u);
	}

	// The channels hold the full 8-bit range without truncation.
	TEST_F(ColorTest, channelsHoldFullByteRange)
	{
		const Color white{255, 255, 255};
		EXPECT_EQ(white.r, 255u);
		EXPECT_EQ(white.g, 255u);
		EXPECT_EQ(white.b, 255u);
	}

	// `channel` maps the [0, 1] endpoints to the 8-bit endpoints.
	TEST_F(ColorTest, channelMapsEndpoints)
	{
		EXPECT_EQ(channel(0.0), 0u);
		EXPECT_EQ(channel(1.0), 255u);
	}

	// `channel` rounds a mid-scale intensity to nearest (0.5 -> lround(127.5) = 128).
	TEST_F(ColorTest, channelRoundsMidScale)
	{
		EXPECT_EQ(channel(0.5), 128u);
	}

	// `channel` clamps out-of-range intensities to the [0, 255] endpoints rather than wrapping.
	TEST_F(ColorTest, channelClampsOutOfRange)
	{
		EXPECT_EQ(channel(-0.5), 0u);
		EXPECT_EQ(channel(2.0), 255u);
	}
}    // namespace coordinates

#endif    // colorTest_h
