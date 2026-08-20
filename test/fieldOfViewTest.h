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

#ifndef fieldOfViewTest_h
#define fieldOfViewTest_h

//------------------------
//	INCLUDES
//------------------------

#include <cmath>

#include <gtest/gtest.h>

#include "pose.h"
#include "fieldOfView.h"

inline namespace coordinates
{
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	CLASS		FieldOfViewTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for the sensor field-of-view containment tests.
	//  ----------------------------------------------------------------------------
	class FieldOfViewTest : public ::testing::Test
	{
	protected:
		// A body direction at the given azimuth (right, about +z) and elevation (down, about +y) off forward.
		static CartesianTuple dir(double azDeg, double elDeg)
		{
			const double az = azDeg * M_PI / 180.0, el = elDeg * M_PI / 180.0;
			return CartesianTuple(units::length::meters<>(std::cos(el) * std::cos(az)),
			                      units::length::meters<>(std::cos(el) * std::sin(az)),
			                      units::length::meters<>(std::sin(el)));
		}
	};

	// The boresight itself is always contained.
	TEST_F(FieldOfViewTest, boresightIsContained)
	{
		FieldOfView fov(30.0_deg, 20.0_deg);
		EXPECT_TRUE(fov.contains(CartesianTuple(1.0_m, 0.0_m, 0.0_m)));
	}

	// Just inside each half-angle passes; just outside fails.
	TEST_F(FieldOfViewTest, horizontalHalfAngleBoundary)
	{
		FieldOfView fov(30.0_deg, 20.0_deg);
		EXPECT_TRUE(fov.contains(dir(29.0, 0.0)));
		EXPECT_FALSE(fov.contains(dir(31.0, 0.0)));
		EXPECT_TRUE(fov.contains(dir(-29.0, 0.0)));
		EXPECT_FALSE(fov.contains(dir(-31.0, 0.0)));
	}

	TEST_F(FieldOfViewTest, verticalHalfAngleBoundary)
	{
		FieldOfView fov(30.0_deg, 20.0_deg);
		EXPECT_TRUE(fov.contains(dir(0.0, 19.0)));
		EXPECT_FALSE(fov.contains(dir(0.0, 21.0)));
	}

	// A rectangular frustum: horizontally in but vertically out is NOT contained.
	TEST_F(FieldOfViewTest, cornerOutsideRectangle)
	{
		FieldOfView fov(30.0_deg, 20.0_deg);
		EXPECT_TRUE(fov.contains(dir(25.0, 15.0)));      // both within
		EXPECT_FALSE(fov.contains(dir(25.0, 25.0)));     // horiz in, vert out
	}

	// Anything behind the sensor is never contained.
	TEST_F(FieldOfViewTest, behindIsNeverContained)
	{
		FieldOfView fov(80.0_deg, 80.0_deg);       // very wide, still not > 90 behind
		EXPECT_FALSE(fov.contains(CartesianTuple(-1.0_m, 0.0_m, 0.0_m)));
	}

	// An off-boresight cone: boresight to the right (+y), a target to the right is contained.
	TEST_F(FieldOfViewTest, arbitraryBoresight)
	{
		FieldOfView fov(CartesianTuple(0.0_m, 1.0_m, 0.0_m), 20.0_deg, 20.0_deg);    // looking +y
		EXPECT_TRUE(fov.contains(CartesianTuple(0.0_m, 1.0_m, 0.0_m)));                     // dead on
		EXPECT_FALSE(fov.contains(CartesianTuple(1.0_m, 0.0_m, 0.0_m)));                    // forward is 90 deg off
	}

	// With a pose, a world-space target point is rotated into body axes before the test. A 90-deg-yaw viewer at
	// the origin looking down world +y: a target on world +y is dead-on its (forward) boresight.
	TEST_F(FieldOfViewTest, worldTargetThroughPose)
	{
		const auto  q = rotation::toQuaternion(rotation::EulerAngles(90.0_deg, 0.0_deg, 0.0_deg));
		Pose        pose(CartesianTuple(0.0_m, 0.0_m, 0.0_m), q);
		FieldOfView fov(10.0_deg, 10.0_deg);
		EXPECT_TRUE(fov.contains(pose, CartesianTuple(0.0_m, 1.0_m, 0.0_m)));     // target on world +y == body forward
		EXPECT_FALSE(fov.contains(pose, CartesianTuple(1.0_m, 0.0_m, 0.0_m)));    // world +x is 90 deg off
	}

	// The look vector is target - viewer position: a viewer offset from the origin still aims correctly.
	TEST_F(FieldOfViewTest, worldTargetPoint)
	{
		Pose        pose(CartesianTuple(100.0_m, 0.0_m, 0.0_m), rotation::Quaternion::identity());
		FieldOfView fov(15.0_deg, 15.0_deg);
		EXPECT_TRUE(fov.contains(pose, CartesianTuple(200.0_m, 0.0_m, 0.0_m)));    // straight ahead (+x)
		EXPECT_FALSE(fov.contains(pose, CartesianTuple(100.0_m, 100.0_m, 0.0_m))); // 90 deg to the side
	}
}    // namespace coordinates

#endif    // fieldOfViewTest_h
