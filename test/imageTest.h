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

#ifndef imageTest_h
#define imageTest_h

//------------------------
//	INCLUDES
//------------------------

#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "camera.h"
#include "color.h"
#include "image.h"
#include "pixel.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		ImageTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for the `Image` RGB raster and its clipped `plot`/`disc`/`line` primitives.
	//  ----------------------------------------------------------------------------
	class ImageTest : public ::testing::Test
	{
	protected:
		//--------------------------------------------------------------------------------------------------------------
		/// @brief		Byte offset of a pixel's red channel in the row-major buffer.
		/// @param[in]	image	the image being indexed.
		/// @param[in]	row		the pixel row.
		/// @param[in]	column	the pixel column.
		/// @return		the index of the red byte; green and blue follow at +1 and +2.
		//--------------------------------------------------------------------------------------------------------------
		static std::size_t index(const Image& image, int row, int column)
		{
			return (static_cast<std::size_t>(row) * image.columns() + column) * 3;
		}

		//--------------------------------------------------------------------------------------------------------------
		/// @brief		Whether a pixel holds exactly the given color.
		/// @param[in]	image	the image being sampled.
		/// @param[in]	row		the pixel row.
		/// @param[in]	column	the pixel column.
		/// @param[in]	c		the color to compare against.
		/// @return		true when the pixel's three bytes equal the color's r, g, b.
		//--------------------------------------------------------------------------------------------------------------
		static bool pixelIs(const Image& image, int row, int column, Color c)
		{
			const std::size_t i = index(image, row, column);
			return image.rgb()[i + 0] == c.r && image.rgb()[i + 1] == c.g && image.rgb()[i + 2] == c.b;
		}

		//--------------------------------------------------------------------------------------------------------------
		/// @brief		Count of pixels whose color differs from a reference (background) color.
		/// @param[in]	image		the image to scan.
		/// @param[in]	background	the color a pixel must differ from to be counted.
		/// @return		the number of changed pixels.
		//--------------------------------------------------------------------------------------------------------------
		static int changedPixels(const Image& image, Color background)
		{
			int count = 0;
			for (int r = 0; r < image.rows(); ++r)
				for (int c = 0; c < image.columns(); ++c)
					if (!pixelIs(image, r, c, background))
						++count;
			return count;
		}
	};

	// The size constructor records rows and columns and sizes the buffer to three bytes per pixel.
	TEST_F(ImageTest, constructorSetsDimensionsAndBufferSize)
	{
		const Image image(4, 6);
		EXPECT_EQ(image.rows(), 4);
		EXPECT_EQ(image.columns(), 6);
		EXPECT_EQ(image.rgb().size(), static_cast<std::size_t>(4) * 6 * 3);
	}

	// The size constructor fills every pixel with its background color (default white).
	TEST_F(ImageTest, constructorFillsBackground)
	{
		const Image image(3, 3);    // default white
		for (std::size_t i = 0; i < image.rgb().size(); ++i)
			EXPECT_EQ(image.rgb()[i], 255u);
	}

	// A non-default background fills the whole buffer with that color.
	TEST_F(ImageTest, constructorHonorsNonDefaultBackground)
	{
		const Image image(2, 2, Color{10, 20, 30});
		for (int r = 0; r < image.rows(); ++r)
			for (int c = 0; c < image.columns(); ++c)
				EXPECT_TRUE(pixelIs(image, r, c, Color{10, 20, 30}));
	}

	// `fill` overwrites every pixel with a single color; spot-check the byte triples.
	TEST_F(ImageTest, fillSetsEveryPixel)
	{
		Image image(3, 4);
		image.fill(Color{1, 2, 3});
		EXPECT_TRUE(pixelIs(image, 0, 0, Color{1, 2, 3}));
		EXPECT_TRUE(pixelIs(image, 2, 3, Color{1, 2, 3}));
		EXPECT_TRUE(pixelIs(image, 1, 2, Color{1, 2, 3}));
		EXPECT_EQ(changedPixels(image, Color{1, 2, 3}), 0);
	}

	// `plot` writes exactly the target pixel's three bytes at the documented index.
	TEST_F(ImageTest, plotSetsTargetPixelBytes)
	{
		Image image(5, 5, Color{0, 0, 0});
		image.plot(Pixel{2, 3}, Color{100, 150, 200});
		const std::size_t i = index(image, 2, 3);
		EXPECT_EQ(image.rgb()[i + 0], 100u);
		EXPECT_EQ(image.rgb()[i + 1], 150u);
		EXPECT_EQ(image.rgb()[i + 2], 200u);
	}

	// `plot` leaves every neighboring pixel untouched: exactly one pixel changes.
	TEST_F(ImageTest, plotLeavesNeighborsUnchanged)
	{
		Image image(5, 5, Color{0, 0, 0});
		image.plot(Pixel{2, 3}, Color{255, 0, 0});
		EXPECT_EQ(changedPixels(image, Color{0, 0, 0}), 1);
		EXPECT_TRUE(pixelIs(image, 2, 3, Color{255, 0, 0}));
	}

	// `plot` clips out-of-range addresses to a safe no-op that leaves the image unchanged.
	TEST_F(ImageTest, plotOutOfRangeIsNoOp)
	{
		Image image(4, 4, Color{50, 60, 70});
		image.plot(Pixel{-1, 0}, Color{1, 1, 1});
		image.plot(Pixel{0, -1}, Color{1, 1, 1});
		image.plot(Pixel{4, 0}, Color{1, 1, 1});    // row == rows(), out of range
		image.plot(Pixel{0, 4}, Color{1, 1, 1});    // column == columns(), out of range
		image.plot(Pixel{100, 100}, Color{1, 1, 1});
		EXPECT_EQ(changedPixels(image, Color{50, 60, 70}), 0);
	}

	// A line sets both endpoints and at least one pixel; a horizontal run covers its whole span.
	TEST_F(ImageTest, lineSetsPixelsAlongIt)
	{
		Image image(5, 7, Color{0, 0, 0});
		image.line(Pixel{2, 0}, Pixel{2, 6}, Color{255, 255, 255});
		EXPECT_TRUE(pixelIs(image, 2, 0, Color{255, 255, 255}));    // endpoint a
		EXPECT_TRUE(pixelIs(image, 2, 6, Color{255, 255, 255}));    // endpoint b
		EXPECT_TRUE(pixelIs(image, 2, 3, Color{255, 255, 255}));    // interior
		// A straight horizontal line touches exactly one pixel per column of its row.
		EXPECT_EQ(changedPixels(image, Color{0, 0, 0}), 7);
	}

	// A single-point line (a == b) sets exactly that one pixel.
	TEST_F(ImageTest, lineDegenerateSetsSinglePixel)
	{
		Image image(4, 4, Color{0, 0, 0});
		image.line(Pixel{1, 1}, Pixel{1, 1}, Color{9, 9, 9});
		EXPECT_TRUE(pixelIs(image, 1, 1, Color{9, 9, 9}));
		EXPECT_EQ(changedPixels(image, Color{0, 0, 0}), 1);
	}

	// A diagonal line sets its endpoints and more than a single pixel.
	TEST_F(ImageTest, lineDiagonalSetsEndpointsAndInterior)
	{
		Image image(5, 5, Color{0, 0, 0});
		image.line(Pixel{0, 0}, Pixel{4, 4}, Color{200, 100, 50});
		EXPECT_TRUE(pixelIs(image, 0, 0, Color{200, 100, 50}));
		EXPECT_TRUE(pixelIs(image, 4, 4, Color{200, 100, 50}));
		EXPECT_GT(changedPixels(image, Color{0, 0, 0}), 2);
	}

	// A line entirely outside the bounds clips every pixel and touches nothing (and does not crash).
	TEST_F(ImageTest, lineFullyOutOfRangeIsNoOp)
	{
		Image image(4, 4, Color{7, 8, 9});
		image.line(Pixel{-10, -10}, Pixel{-2, -3}, Color{1, 1, 1});
		EXPECT_EQ(changedPixels(image, Color{7, 8, 9}), 0);
	}

	// A line crossing the boundary sets the in-range portion while clipping the out-of-range portion.
	TEST_F(ImageTest, lineClipsAtBoundary)
	{
		Image image(5, 5, Color{0, 0, 0});
		image.line(Pixel{2, -3}, Pixel{2, 3}, Color{255, 255, 255});
		// Columns -3..-1 clip; columns 0..3 (four pixels) on row 2 are set.
		EXPECT_EQ(changedPixels(image, Color{0, 0, 0}), 4);
		EXPECT_TRUE(pixelIs(image, 2, 0, Color{255, 255, 255}));
		EXPECT_TRUE(pixelIs(image, 2, 3, Color{255, 255, 255}));
	}

	// A radius-0 disc sets exactly the center pixel.
	TEST_F(ImageTest, discRadiusZeroSetsCenterOnly)
	{
		Image image(7, 7, Color{0, 0, 0});
		image.disc(Pixel{3, 3}, 0, Color{255, 0, 0});
		EXPECT_TRUE(pixelIs(image, 3, 3, Color{255, 0, 0}));
		EXPECT_EQ(changedPixels(image, Color{0, 0, 0}), 1);
	}

	// A disc sets its center and a blob whose extent grows with the radius.
	TEST_F(ImageTest, discBlobGrowsWithRadius)
	{
		Image small(9, 9, Color{0, 0, 0});
		small.disc(Pixel{4, 4}, 1, Color{255, 0, 0});
		EXPECT_TRUE(pixelIs(small, 4, 4, Color{255, 0, 0}));
		const int smallCount = changedPixels(small, Color{0, 0, 0});
		EXPECT_GT(smallCount, 1);

		Image large(9, 9, Color{0, 0, 0});
		large.disc(Pixel{4, 4}, 3, Color{255, 0, 0});
		const int largeCount = changedPixels(large, Color{0, 0, 0});
		EXPECT_GT(largeCount, smallCount);
	}

	// A radius-1 disc is the classic five-pixel plus (center plus the four orthogonal neighbors).
	TEST_F(ImageTest, discRadiusOneIsFivePixelPlus)
	{
		Image image(9, 9, Color{0, 0, 0});
		image.disc(Pixel{4, 4}, 1, Color{1, 2, 3});
		EXPECT_EQ(changedPixels(image, Color{0, 0, 0}), 5);
		EXPECT_TRUE(pixelIs(image, 4, 4, Color{1, 2, 3}));    // center
		EXPECT_TRUE(pixelIs(image, 3, 4, Color{1, 2, 3}));    // up
		EXPECT_TRUE(pixelIs(image, 5, 4, Color{1, 2, 3}));    // down
		EXPECT_TRUE(pixelIs(image, 4, 3, Color{1, 2, 3}));    // left
		EXPECT_TRUE(pixelIs(image, 4, 5, Color{1, 2, 3}));    // right
		EXPECT_FALSE(pixelIs(image, 3, 3, Color{1, 2, 3}));   // corner excluded by the radius test
	}

	// A disc clips against the bounds: a disc centered at a corner sets only its in-range pixels.
	TEST_F(ImageTest, discClipsAtCorner)
	{
		Image image(9, 9, Color{0, 0, 0});
		image.disc(Pixel{0, 0}, 2, Color{255, 0, 0});
		EXPECT_TRUE(pixelIs(image, 0, 0, Color{255, 0, 0}));
		// Every changed pixel lies inside the bounds; nothing off-image was written (no crash).
		const int inRange = changedPixels(image, Color{0, 0, 0});
		EXPECT_GT(inRange, 0);
		// A full (unclipped) radius-2 disc is 13 pixels; the corner-clipped one is fewer.
		EXPECT_LT(inRange, 13);
	}

	// The camera constructor sizes the image to the camera's rows (height) by columns (width).
	TEST_F(ImageTest, cameraConstructorSizesToCameraFrame)
	{
		const Camera camera(640, 480);    // width, height
		const Image  image(camera);
		EXPECT_EQ(image.rows(), 480);       // rows == height
		EXPECT_EQ(image.columns(), 640);    // columns == width
		EXPECT_EQ(image.rgb().size(), static_cast<std::size_t>(480) * 640 * 3);
	}

	// The camera constructor honors a non-default background.
	TEST_F(ImageTest, cameraConstructorHonorsBackground)
	{
		const Camera camera(4, 3);
		const Image  image(camera, Color{5, 6, 7});
		EXPECT_TRUE(pixelIs(image, 0, 0, Color{5, 6, 7}));
		EXPECT_TRUE(pixelIs(image, 2, 3, Color{5, 6, 7}));
	}
}    // namespace coordinates

#endif    // imageTest_h
