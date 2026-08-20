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

#ifndef entityTest_h
#define entityTest_h

//------------------------
//	INCLUDES
//------------------------

#include <gtest/gtest.h>

#include "gtest_units.h"
#include "entity.h"

inline namespace coordinates
{
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	CLASS		EntityTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for the Entity composable type: construction, world resolution, and attachment.
	//  ----------------------------------------------------------------------------
	class EntityTest : public ::testing::Test
	{
	protected:
		using Wgs = datums::WGS84_G1674;
	};

	// A bare point entity reports its own position; a root has no parent.
	TEST_F(EntityTest, waypointReportsItsPosition)
	{
		PositionECEF<Wgs> p(6378137.0_m, 0.0_m, 0.0_m);
		Entity<Wgs>       wp(p);
		EXPECT_UNITS_NEAR(6378137.0_m, wp.position().x(), 1e-6_m);
		EXPECT_EQ(wp.parent(), nullptr);
		EXPECT_TRUE(wp.children().empty());
	}

	// The trailing-default constructor covers the posed and moving use cases.
	TEST_F(EntityTest, constructsPosedAndMoving)
	{
		PositionECEF<Wgs> p(6378137.0_m, 0.0_m, 0.0_m);
		Entity<Wgs>       posed(p, Pose::identity());
		EXPECT_UNITS_NEAR(6378137.0_m, posed.position().x(), 1e-6_m);

		using mps = units::velocity::meters_per_second<>;
		Entity<Wgs> ac(p, Pose::identity(), VelocityVector<Entity<Wgs>::frame_type>(mps(250.0), mps(0.0), mps(0.0)), AngularRateVector<Entity<Wgs>::frame_type>());
		EXPECT_UNITS_NEAR(mps(250.0), std::get<0>(ac.velocity().vector()), 1e-9_mps);
	}

	// A child attached at a body offset resolves its position to WORLD through the parent chain.
	TEST_F(EntityTest, childResolvesToWorld)
	{
		PositionECEF<Wgs> p(6378137.0_m, 0.0_m, 0.0_m);
		Entity<Wgs>       aircraft(p, Pose::identity());
		Entity<Wgs>&      pod = aircraft.attach({CartesianTuple(0.0_m, 2.5_m, 0.0_m), Pose::identity()});

		const auto world = pod.position();
		EXPECT_UNITS_NEAR(6378137.0_m, world.x(), 1e-6_m);
		EXPECT_UNITS_NEAR(2.5_m, world.y(), 1e-6_m);
		EXPECT_EQ(pod.parent(), &aircraft);
		EXPECT_EQ(aircraft.children().size(), 1u);
	}

	// A yawed parent rotates the child's body offset into world: +y-body offset under 90 deg yaw points -x world.
	TEST_F(EntityTest, yawedParentRotatesChildOffset)
	{
		PositionECEF<Wgs> p(0.0_m, 0.0_m, 0.0_m);
		const auto        yaw = rotation::toQuaternion(rotation::EulerAngles(90.0_deg, 0.0_deg, 0.0_deg));
		Entity<Wgs>       aircraft(p, Pose(CartesianTuple(0.0_m, 0.0_m, 0.0_m), yaw));
		Entity<Wgs>&      pod = aircraft.attach({CartesianTuple(1.0_m, 0.0_m, 0.0_m), Pose::identity()});    // 1 m forward in body

		// Body forward (+x) under 90 deg yaw maps to world +y.
		const auto world = pod.position();
		EXPECT_UNITS_NEAR(0.0_m, world.x(), 1e-9_m);
		EXPECT_UNITS_NEAR(1.0_m, world.y(), 1e-9_m);
	}

	// A child reported relative to its own parent is just its mount offset (round-trips the world resolution).
	TEST_F(EntityTest, childRelativeToParentIsMount)
	{
		PositionECEF<Wgs> p(6378137.0_m, 100.0_m, 0.0_m);
		Entity<Wgs>       aircraft(p, Pose::identity());
		Entity<Wgs>&      pod = aircraft.attach({CartesianTuple(0.0_m, 2.5_m, 0.0_m), Pose::identity()});

		const auto rel = pod.position(aircraft);
		EXPECT_UNITS_NEAR(0.0_m, rel.x(), 1e-6_m);
		EXPECT_UNITS_NEAR(2.5_m, rel.y(), 1e-6_m);
	}

	// Nesting composes: a camera on a pod on an aircraft resolves through both parents.
	TEST_F(EntityTest, nestedChildrenCompose)
	{
		PositionECEF<Wgs> p(6378137.0_m, 0.0_m, 0.0_m);
		Entity<Wgs>       aircraft(p, Pose::identity());
		Entity<Wgs>&      pod    = aircraft.attach({CartesianTuple(0.0_m, 2.5_m, 0.0_m), Pose::identity()});
		Entity<Wgs>&      camera = pod.attach({CartesianTuple(0.0_m, 0.5_m, 0.0_m), Pose::identity()});

		const auto world = camera.position();
		EXPECT_UNITS_NEAR(3.0_m, world.y(), 1e-6_m);    // 2.5 + 0.5, both +y with identity attitudes
	}

	// The boresight ray is the entity's forward (+x) world axis; an identity pose points along world +x.
	TEST_F(EntityTest, rayIsForwardBoresight)
	{
		PositionECEF<Wgs> p(6378137.0_m, 0.0_m, 0.0_m);
		Entity<Wgs>       e(p, Pose::identity());
		const auto        r = e.ray();
		EXPECT_NEAR(std::get<0>(r.direction().vector()).value(), 1.0, 1e-9);
		EXPECT_NEAR(std::get<1>(r.direction().vector()).value(), 0.0, 1e-9);
	}

	// No field of view = omnidirectional: sees() is pure line of sight. A target far along the boresight, well
	// above the ellipsoid, is visible; there is no cone to exclude it.
	TEST_F(EntityTest, seesWithoutFieldOfViewIsLineOfSight)
	{
		PositionECEF<Wgs> here(6378137.0_m, 0.0_m, 0.0_m);
		PositionECEF<Wgs> ahead(6478137.0_m, 0.0_m, 0.0_m);    // 100 km further out along +x
		Entity<Wgs>       viewer(here, Pose::identity());
		Entity<Wgs>       target(ahead);
		EXPECT_TRUE(viewer.sees(target));
		EXPECT_FALSE(viewer.fieldOfView().has_value());
	}

	// A narrow field of view excludes a target off the boresight even when line of sight is clear.
	TEST_F(EntityTest, fieldOfViewExcludesOffAxisTarget)
	{
		PositionECEF<Wgs> here(6378137.0_m, 0.0_m, 0.0_m);
		Entity<Wgs>       viewer(here, Pose::identity());
		viewer.setFieldOfView(FieldOfView(5.0_deg, 5.0_deg));    // narrow, about forward (+x)

		Entity<Wgs> ahead(PositionECEF<Wgs>(6478137.0_m, 0.0_m, 0.0_m));       // dead ahead
		Entity<Wgs> aside(PositionECEF<Wgs>(6378137.0_m, 100000.0_m, 0.0_m));  // 100 km to the side (+y)
		EXPECT_TRUE(viewer.sees(ahead));
		EXPECT_FALSE(viewer.sees(aside));    // outside the 5 deg cone
	}
}    // namespace coordinates

#endif    // entityTest_h
