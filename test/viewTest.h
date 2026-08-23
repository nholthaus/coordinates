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

#ifndef viewTest_h
#define viewTest_h

//------------------------
//	INCLUDES
//------------------------

#include <cstddef>

#include <gtest/gtest.h>

#include "camera.h"
#include "color.h"
#include "image.h"
#include "pixel.h"
#include "view.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		ViewTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for the `View` camera-plus-image pairing.
	//  ----------------------------------------------------------------------------
	class ViewTest : public ::testing::Test
	{
	protected:
		//--------------------------------------------------------------------------------------------------------------
		/// @brief		Whether an image pixel holds exactly the given color.
		/// @param[in]	image	the image being sampled.
		/// @param[in]	row		the pixel row.
		/// @param[in]	column	the pixel column.
		/// @param[in]	c		the color to compare against.
		/// @return		true when the pixel's three bytes equal the color's r, g, b.
		//--------------------------------------------------------------------------------------------------------------
		static bool pixelIs(const Image& image, int row, int column, Color c)
		{
			const std::size_t i = (static_cast<std::size_t>(row) * image.columns() + column) * 3;
			return image.rgb()[i + 0] == c.r && image.rgb()[i + 1] == c.g && image.rgb()[i + 2] == c.b;
		}
	};

	// A view sizes its image to the camera's frame: rows == height, columns == width.
	TEST_F(ViewTest, imageSizedFromCameraFrame)
	{
		const Camera camera(640, 480);    // width, height
		const View   view(camera);
		EXPECT_EQ(view.image().rows(), 480);
		EXPECT_EQ(view.image().columns(), 640);
		EXPECT_EQ(view.image().rgb().size(), static_cast<std::size_t>(480) * 640 * 3);
	}

	// The view holds its own copy of the camera and exposes it unchanged.
	TEST_F(ViewTest, cameraAccessorReturnsTheCamera)
	{
		const Camera camera(320, 200);
		const View   view(camera);
		EXPECT_EQ(view.camera().width(), 320);
		EXPECT_EQ(view.camera().height(), 200);
	}

	// The default background is a blank white canvas.
	TEST_F(ViewTest, defaultBackgroundIsWhite)
	{
		const Camera camera(3, 2);
		const View   view(camera);
		for (std::size_t i = 0; i < view.image().rgb().size(); ++i)
			EXPECT_EQ(view.image().rgb()[i], 255u);
	}

	// A non-default background is honored across the whole canvas.
	TEST_F(ViewTest, nonDefaultBackgroundIsHonored)
	{
		const Camera camera(4, 3);
		const View   view(camera, Color{12, 34, 56});
		EXPECT_TRUE(pixelIs(view.image(), 0, 0, Color{12, 34, 56}));
		EXPECT_TRUE(pixelIs(view.image(), 2, 3, Color{12, 34, 56}));
	}

	// The mutable image accessor is drawable and the mutation persists in the view.
	TEST_F(ViewTest, imageIsDrawableAndMutationPersists)
	{
		const Camera camera(5, 5);
		View         view(camera, Color{0, 0, 0});
		view.image().plot(Pixel{2, 3}, Color{255, 0, 0});
		EXPECT_TRUE(pixelIs(view.image(), 2, 3, Color{255, 0, 0}));
		// The const accessor observes the same mutated pixel.
		const View& constView = view;
		EXPECT_TRUE(pixelIs(constView.image(), 2, 3, Color{255, 0, 0}));
	}
}    // namespace coordinates

#endif    // viewTest_h
