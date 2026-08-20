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

#ifndef rayTest_h
#define rayTest_h

//------------------------
//	INCLUDES
//------------------------

#include <cmath>

#include <gtest/gtest.h>

#include "gtest_units.h"
#include "kinematicState.h"
#include "ray.h"

inline namespace coordinates
{
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	CLASS		RayTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for the Ray type, its factories, and the point-member ray emitters.
	//  ----------------------------------------------------------------------------
	class RayTest : public ::testing::Test
	{
	protected:
		using Wgs   = datums::WGS84_G1674;
		using Frame = coordinateFrames::ECEFFrame<horizontalDatums::WGS84_G1674>;

		static double len(const Ray<Frame>& r)
		{
			const auto d = r.direction().vector();
			return std::sqrt(std::get<0>(d).value() * std::get<0>(d).value() + std::get<1>(d).value() * std::get<1>(d).value()
			                 + std::get<2>(d).value() * std::get<2>(d).value());
		}
	};

	// A ray's direction is normalized on construction, regardless of the input magnitude.
	TEST_F(RayTest, directionIsNormalized)
	{
		Coordinate<Frame, CartesianTuple> origin;
		origin.setPoint(6378137.0_m, 0.0_m, 0.0_m);
		Vector<Frame> dir(3.0_m, 4.0_m, 0.0_m);    // length 5
		Ray<Frame>    r(origin, dir);
		EXPECT_NEAR(len(r), 1.0, 1e-12);
		EXPECT_NEAR(std::get<0>(r.direction().vector()).value(), 0.6, 1e-12);
		EXPECT_NEAR(std::get<1>(r.direction().vector()).value(), 0.8, 1e-12);
	}

	// pointAt walks a metric distance along the unit direction from the origin.
	TEST_F(RayTest, pointAtWalksAlongDirection)
	{
		Coordinate<Frame, CartesianTuple> origin;
		origin.setPoint(6378137.0_m, 0.0_m, 0.0_m);
		Vector<Frame> dir(1.0_m, 0.0_m, 0.0_m);
		Ray<Frame>    r(origin, dir);
		const auto    p = r.pointAt(1000.0_m);
		EXPECT_UNITS_NEAR(units::length::meters<>(6378137.0 + 1000.0), std::get<0>(p.point()), 1e-6_m);
		EXPECT_UNITS_NEAR(0.0_m, std::get<1>(p.point()), 1e-6_m);
	}

	// fromPose starts at the pose translation and rotates the body direction into the frame.
	TEST_F(RayTest, fromPoseRotatesBodyDirection)
	{
		Pose       pose(CartesianTuple(6378137.0_m, 0.0_m, 0.0_m), rotation::Quaternion::identity());
		Ray<Frame> r = Ray<Frame>::fromPose(pose, CartesianTuple(0.0_m, 0.0_m, 1.0_m));
		EXPECT_NEAR(std::get<2>(r.direction().vector()).value(), 1.0, 1e-12);
		EXPECT_UNITS_NEAR(6378137.0_m, std::get<0>(r.origin().point()), 1e-6_m);
	}

	// A level east beam (az=90, el=0) from a site has zero local-up component: its ECEF direction is
	// perpendicular to the site's up (radial) axis at the equator-free general case -> dot with up ~ 0.
	TEST_F(RayTest, azimuthElevationLevelBeamHasNoUpComponent)
	{
		PositionGeodetic<Wgs> site(37.65639_deg, -114.32944_deg, 2078.0_m);
		auto                  beam = ray(site, 90.0_deg, 0.0_deg);    // free point-emitter
		const double          phi = units::angle::radians<>(37.65639_deg).value();
		const double          lam = units::angle::radians<>(-114.32944_deg).value();
		const double          ux = std::cos(phi) * std::cos(lam), uy = std::cos(phi) * std::sin(lam), uz = std::sin(phi);
		const auto            d = beam.direction().vector();
		const double          dotUp = std::get<0>(d).value() * ux + std::get<1>(d).value() * uy + std::get<2>(d).value() * uz;
		EXPECT_NEAR(dotUp, 0.0, 1e-9);    // level -> no vertical component
	}

	// A straight-up look-angle (el=90) points along the site's local up (radial) axis.
	TEST_F(RayTest, azimuthElevationUpBeamIsRadial)
	{
		PositionGeodetic<Wgs> site(37.65639_deg, -114.32944_deg, 2078.0_m);
		auto                  beam = ray(site, 0.0_deg, 90.0_deg);
		const double          phi = units::angle::radians<>(37.65639_deg).value();
		const double          lam = units::angle::radians<>(-114.32944_deg).value();
		const auto            d = beam.direction().vector();
		EXPECT_NEAR(std::get<0>(d).value(), std::cos(phi) * std::cos(lam), 1e-9);
		EXPECT_NEAR(std::get<1>(d).value(), std::cos(phi) * std::sin(lam), 1e-9);
		EXPECT_NEAR(std::get<2>(d).value(), std::sin(phi), 1e-9);
	}

	// KinematicState::ray() with no angles is the boresight: the body forward (+x) axis, rotated by the pose.
	TEST_F(RayTest, kinematicStateBoresightIsForwardAxis)
	{
		Pose              pose(CartesianTuple(6378137.0_m, 0.0_m, 0.0_m), rotation::Quaternion::identity());
		KinematicState<Frame> body(pose, VelocityVector<Frame>(), AngularRateVector<Frame>());
		auto              r = body.ray();
		EXPECT_NEAR(std::get<0>(r.direction().vector()).value(), 1.0, 1e-12);    // pure forward
		EXPECT_NEAR(std::get<1>(r.direction().vector()).value(), 0.0, 1e-12);
		EXPECT_NEAR(std::get<2>(r.direction().vector()).value(), 0.0, 1e-12);
	}

	// Off-boresight deltas steer the ray within body axes: +az toward +y (right), +el toward +z (down).
	TEST_F(RayTest, kinematicStateOffBoresightSteersInBodyAxes)
	{
		Pose              pose(CartesianTuple(6378137.0_m, 0.0_m, 0.0_m), rotation::Quaternion::identity());
		KinematicState<Frame> body(pose, VelocityVector<Frame>(), AngularRateVector<Frame>());
		auto              r = body.ray(90.0_deg, 0.0_deg);    // 90 deg right, level -> pure +y
		EXPECT_NEAR(std::get<0>(r.direction().vector()).value(), 0.0, 1e-9);
		EXPECT_NEAR(std::get<1>(r.direction().vector()).value(), 1.0, 1e-9);
		auto rDown = body.ray(0.0_deg, 90.0_deg);    // straight down body -> pure +z
		EXPECT_NEAR(std::get<2>(rDown.direction().vector()).value(), 1.0, 1e-9);
	}

	// A mount Pose offsets the ray's origin: a +y-body offset with identity attitude shifts the origin +y.
	TEST_F(RayTest, kinematicStateMountOffsetsOrigin)
	{
		Pose              pose(CartesianTuple(6378137.0_m, 0.0_m, 0.0_m), rotation::Quaternion::identity());
		KinematicState<Frame> body(pose, VelocityVector<Frame>(), AngularRateVector<Frame>());
		Pose              mount(CartesianTuple(0.0_m, 2.5_m, 0.0_m), rotation::Quaternion::identity());
		auto              r = body.ray(mount);
		EXPECT_UNITS_NEAR(2.5_m, std::get<1>(r.origin().point()), 1e-9_m);
	}
}    // namespace coordinates

#endif    // rayTest_h
