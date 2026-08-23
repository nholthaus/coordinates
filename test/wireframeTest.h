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

#ifndef wireframeTest_h
#define wireframeTest_h

//------------------------
//	INCLUDES
//------------------------

#include <cstddef>

#include <gtest/gtest.h>

#include "gtest_units.h"
#include "wireframe.h"

inline namespace coordinates
{
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	CLASS		WireframeTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for drawPolyline: it strokes an in-view loop, skips behind-lens edges, and draws onto a View.
	//  ----------------------------------------------------------------------------
	class WireframeTest : public ::testing::Test
	{
	protected:
		static constexpr Color background{255, 255, 255};
		static constexpr Color stroke{255, 0, 0};

		// A camera looking from the origin down world +x, sized so a small square ahead lands well inside the frame.
		static Camera forwardCamera(int width = 640, int height = 480)
		{
			Camera camera(width, height);
			camera.lookAt(CartesianTuple(0.0_m, 0.0_m, 0.0_m), CartesianTuple(1.0_m, 0.0_m, 0.0_m));
			return camera;
		}

		// A unit square in a plane 10 m ahead of the origin (constant +x depth), spanning the y/z axes.
		static CartesianVector squareAhead()
		{
			return CartesianVector{
			    {10.0_m, -1.0_m, -1.0_m},
			    {10.0_m, 1.0_m, -1.0_m},
			    {10.0_m, 1.0_m, 1.0_m},
			    {10.0_m, -1.0_m, 1.0_m},
			};
		}

		// The number of pixels whose RGB differs from the white background (one count per pixel, not per byte).
		static std::size_t nonBackgroundPixels(const Image& image)
		{
			const std::vector<std::uint8_t>& rgb   = image.rgb();
			std::size_t                      count = 0;
			for (std::size_t i = 0; i + 2 < rgb.size(); i += 3)
				if (rgb[i] != background.r || rgb[i + 1] != background.g || rgb[i + 2] != background.b)
					++count;
			return count;
		}
	};

	// Drawing a square through an identity pose, with a camera looking at it, marks stroke pixels on the canvas.
	TEST_F(WireframeTest, drawsAnInViewSquare)
	{
		const Camera camera = forwardCamera();
		Image        image(camera, background);

		drawPolyline(image, camera, Pose::identity(), squareAhead(), stroke);

		EXPECT_GT(nonBackgroundPixels(image), 0u);
	}

	// A polyline entirely behind the lens leaves every pixel at the background: no edge has two on-lens endpoints.
	TEST_F(WireframeTest, allBehindLensLeavesImageUnchanged)
	{
		const Camera camera = forwardCamera();
		Image        image(camera, background);

		// The camera looks down +x; a square at negative x is wholly behind the lens.
		const CartesianVector behind{
		    {-10.0_m, -1.0_m, -1.0_m},
		    {-10.0_m, 1.0_m, -1.0_m},
		    {-10.0_m, 1.0_m, 1.0_m},
		    {-10.0_m, -1.0_m, 1.0_m},
		};

		drawPolyline(image, camera, Pose::identity(), behind, stroke);

		EXPECT_EQ(0u, nonBackgroundPixels(image));
	}

	// A pose that translates the shape in front of the camera brings an otherwise-behind square into view.
	TEST_F(WireframeTest, poseBringsShapeIntoView)
	{
		const Camera camera = forwardCamera();
		Image        image(camera, background);

		// A square centered at the body origin (spanning y/z at x = 0) is on the lens plane and behind it; posing
		// it 10 m down +x (identity attitude) places the whole loop ahead of the camera.
		const CartesianVector atOrigin{
		    {0.0_m, -1.0_m, -1.0_m},
		    {0.0_m, 1.0_m, -1.0_m},
		    {0.0_m, 1.0_m, 1.0_m},
		    {0.0_m, -1.0_m, 1.0_m},
		};
		const Pose forward(CartesianTuple(10.0_m, 0.0_m, 0.0_m), Quaternion::identity());

		drawPolyline(image, camera, forward, atOrigin, stroke);

		EXPECT_GT(nonBackgroundPixels(image), 0u);
	}

	// The stroke color is what lands on the canvas: the marked pixels carry the requested color, not some other.
	TEST_F(WireframeTest, strokeUsesTheRequestedColor)
	{
		const Camera camera = forwardCamera();
		Image        image(camera, background);

		drawPolyline(image, camera, Pose::identity(), squareAhead(), stroke);

		const std::vector<std::uint8_t>& rgb   = image.rgb();
		bool                             found = false;
		for (std::size_t i = 0; i + 2 < rgb.size(); i += 3)
			if (rgb[i] == stroke.r && rgb[i + 1] == stroke.g && rgb[i + 2] == stroke.b)
			{
				found = true;
				break;
			}
		EXPECT_TRUE(found);
	}

	// The View overload draws onto the view's own image, using the view's own camera -- no separately-passed pair.
	TEST_F(WireframeTest, viewOverloadDrawsOntoTheViewImage)
	{
		View view(forwardCamera(), background);
		ASSERT_EQ(0u, nonBackgroundPixels(view.image()));

		drawPolyline(view, Pose::identity(), squareAhead(), stroke);

		EXPECT_GT(nonBackgroundPixels(view.image()), 0u);
	}
}    // namespace coordinates

#endif    // wireframeTest_h
