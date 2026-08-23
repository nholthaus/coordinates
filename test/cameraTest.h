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

#ifndef cameraTest_h
#define cameraTest_h

//------------------------
//	INCLUDES
//------------------------

#include <gtest/gtest.h>

#include "gtest_units.h"
#include "camera.h"

inline namespace coordinates
{
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	CLASS		CameraTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for the pinhole Camera: construction, sees(), lookAt/project geometry, and field of view.
	//  ----------------------------------------------------------------------------
	class CameraTest : public ::testing::Test
	{
	protected:
		// A camera looking from the origin down world +x. With the default world-up (-z), this camera's screen
		// basis is: world +y -> screen right (larger column), world -z -> screen up (smaller row).
		static Camera forwardCamera(int width = 640, int height = 480)
		{
			Camera camera(width, height);
			camera.lookAt(CartesianTuple(0.0_m, 0.0_m, 0.0_m), CartesianTuple(1.0_m, 0.0_m, 0.0_m));
			return camera;
		}
	};

	// Construction stores the image size and starts at the origin.
	TEST_F(CameraTest, constructionSetsSize)
	{
		Camera camera(640, 480);
		EXPECT_EQ(640, camera.width());
		EXPECT_EQ(480, camera.height());
		EXPECT_UNITS_NEAR(0.0_m, camera.eye().x(), 1e-12_m);
		EXPECT_UNITS_NEAR(0.0_m, camera.eye().y(), 1e-12_m);
		EXPECT_UNITS_NEAR(0.0_m, camera.eye().z(), 1e-12_m);
	}

	// sees() accepts an on-lens pixel and rejects the behind-the-lens sentinel.
	TEST_F(CameraTest, seesAcceptsOnLensRejectsSentinel)
	{
		EXPECT_TRUE(Camera::sees(Pixel{0, 0}));
		EXPECT_TRUE(Camera::sees(Pixel{240, 320}));
		// The sentinel a point at/behind the lens projects to.
		EXPECT_FALSE(Camera::sees(Pixel{-1, -1}));
		// Any negative row is off-lens by the same rule.
		EXPECT_FALSE(Camera::sees(Pixel{-5, 100}));
	}

	// A point on the boresight projects to the image center.
	TEST_F(CameraTest, projectAheadLandsAtCenter)
	{
		const Camera camera = forwardCamera(640, 480);
		const Pixel  center = camera.project(CartesianTuple(10.0_m, 0.0_m, 0.0_m));
		EXPECT_TRUE(Camera::sees(center));
		EXPECT_EQ(240, center.row);       // height / 2
		EXPECT_EQ(320, center.column);    // width / 2
	}

	// A point at the eye and a point behind the lens both return the off-image sentinel.
	TEST_F(CameraTest, projectAtOrBehindLensIsSentinel)
	{
		const Camera camera = forwardCamera();
		// At the eye: zero depth.
		const Pixel atEye = camera.project(CartesianTuple(0.0_m, 0.0_m, 0.0_m));
		EXPECT_EQ(-1, atEye.row);
		EXPECT_EQ(-1, atEye.column);
		EXPECT_FALSE(Camera::sees(atEye));
		// Behind the lens: negative forward depth (camera looks down +x, this point is at -x).
		const Pixel behind = camera.project(CartesianTuple(-10.0_m, 0.0_m, 0.0_m));
		EXPECT_FALSE(Camera::sees(behind));
	}

	// The screen basis: a point to world +y falls right of center; a point to world -y falls left of center.
	TEST_F(CameraTest, horizontalSymmetryAboutCenter)
	{
		const Camera camera = forwardCamera(640, 480);
		const Pixel  right  = camera.project(CartesianTuple(10.0_m, 1.0_m, 0.0_m));
		const Pixel  left   = camera.project(CartesianTuple(10.0_m, -1.0_m, 0.0_m));

		EXPECT_TRUE(Camera::sees(right));
		EXPECT_TRUE(Camera::sees(left));
		EXPECT_GT(right.column, 320);    // world +y projects right of center
		EXPECT_LT(left.column, 320);     // world -y projects left of center
		// Symmetric offsets land symmetrically about the center column, on the center row.
		EXPECT_EQ(320 - left.column, right.column - 320);
		EXPECT_EQ(240, right.row);
		EXPECT_EQ(240, left.row);
	}

	// The screen basis: world -z (aerospace "up") falls above center; world +z falls below center.
	TEST_F(CameraTest, verticalSymmetryAboutCenter)
	{
		const Camera camera = forwardCamera(640, 480);
		const Pixel  up     = camera.project(CartesianTuple(10.0_m, 0.0_m, -1.0_m));
		const Pixel  down   = camera.project(CartesianTuple(10.0_m, 0.0_m, 1.0_m));

		EXPECT_TRUE(Camera::sees(up));
		EXPECT_TRUE(Camera::sees(down));
		EXPECT_LT(up.row, 240);      // world -z projects toward the top (smaller row)
		EXPECT_GT(down.row, 240);    // world +z projects toward the bottom (larger row)
		// Symmetric offsets land symmetrically about the center row, on the center column.
		EXPECT_EQ(240 - up.row, down.row - 240);
		EXPECT_EQ(320, up.column);
		EXPECT_EQ(320, down.column);
	}

	// The focal length is the image half-height over the tangent of the half-angle: a wider field of view yields
	// a shorter focal, so the same off-axis point projects CLOSER to the center.
	TEST_F(CameraTest, widerFieldOfViewProjectsCloserToCenter)
	{
		const CartesianTuple offAxis(10.0_m, 1.0_m, 0.0_m);

		Camera narrow(640, 480, 30.0_deg);
		narrow.lookAt(CartesianTuple(0.0_m, 0.0_m, 0.0_m), CartesianTuple(1.0_m, 0.0_m, 0.0_m));
		Camera wide(640, 480, 90.0_deg);
		wide.lookAt(CartesianTuple(0.0_m, 0.0_m, 0.0_m), CartesianTuple(1.0_m, 0.0_m, 0.0_m));

		const Pixel narrowPixel = narrow.project(offAxis);
		const Pixel widePixel   = wide.project(offAxis);

		// Both are right of center; the narrow (longer focal) spreads the point farther out.
		EXPECT_GT(narrowPixel.column, 320);
		EXPECT_GT(widePixel.column, 320);
		EXPECT_GT(narrowPixel.column - 320, widePixel.column - 320);
	}

	// setFieldOfView re-derives the focal in place: widening it after construction pulls the same point inward.
	TEST_F(CameraTest, setFieldOfViewChangesProjectedSpread)
	{
		const CartesianTuple offAxis(10.0_m, 1.0_m, 0.0_m);

		Camera camera(640, 480, 30.0_deg);
		camera.lookAt(CartesianTuple(0.0_m, 0.0_m, 0.0_m), CartesianTuple(1.0_m, 0.0_m, 0.0_m));
		const Pixel before = camera.project(offAxis);

		camera.setFieldOfView(90.0_deg);
		const Pixel after = camera.project(offAxis);

		EXPECT_GT(before.column, 320);
		EXPECT_GT(after.column, 320);
		EXPECT_GT(before.column - 320, after.column - 320);
	}

	// project respects the eye position: the same relative geometry from a translated eye lands at the same pixel.
	TEST_F(CameraTest, projectIsRelativeToTheEye)
	{
		Camera camera(640, 480);
		camera.lookAt(CartesianTuple(100.0_m, 50.0_m, 25.0_m), CartesianTuple(101.0_m, 50.0_m, 25.0_m));
		// A point 10 m ahead of the (translated) eye along +x is dead center.
		const Pixel center = camera.project(CartesianTuple(110.0_m, 50.0_m, 25.0_m));
		EXPECT_TRUE(Camera::sees(center));
		EXPECT_EQ(240, center.row);
		EXPECT_EQ(320, center.column);
	}
}    // namespace coordinates

#endif    // cameraTest_h
