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

#ifndef kinematicStateTest_h
#define kinematicStateTest_h

//------------------------
//	INCLUDES
//------------------------

#include <cmath>

#include <gtest/gtest.h>

#include "gtest_units.h"
#include "kinematicState.h"

inline namespace coordinates
{
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	CLASS		KinematicStateTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for the kinematic state and its propagation.
	//  ----------------------------------------------------------------------------
	class KinematicStateTest : public ::testing::Test
	{
	protected:
		using F   = coordinateFrames::ECEFFrame<horizontalDatums::WGS84_G1674>;
		using mps = units::velocity::meters_per_second<>;
		using rps = units::angular_velocity::radians_per_second<>;
	};

	TEST_F(KinematicStateTest, translationAdvancesByVelocityTimesDt)
	{
		KinematicState<F> s(Pose::identity(), VelocityVector<F>(mps(10.0), mps(0.0), mps(0.0)), AngularRateVector<F>());
		auto              s2 = integrateKinematics(s, AccelerationVector<F>(), AngularRateVector<F>(), 2.0_s);
		EXPECT_UNITS_NEAR(20.0_m, std::get<0>(s2.position()), 1.0e-12_m);
		EXPECT_UNITS_NEAR(0.0_m, std::get<1>(s2.position()), 1.0e-12_m);
		EXPECT_UNITS_NEAR(0.0_m, std::get<2>(s2.position()), 1.0e-12_m);
	}

	TEST_F(KinematicStateTest, linearAccelerationChangesVelocity)
	{
		using mps2 = units::acceleration::meters_per_second_squared<>;
		KinematicState<F> s(Pose::identity(), VelocityVector<F>(), AngularRateVector<F>());
		auto              s2 = integrateKinematics(s, AccelerationVector<F>(mps2(3.0), mps2(0.0), mps2(0.0)), AngularRateVector<F>(), 4.0_s);
		// v = a*t = 12 m/s
		EXPECT_UNITS_NEAR(mps(12.0), std::get<0>(s2.velocity().vector()), 1.0e-12_mps);
	}

	TEST_F(KinematicStateTest, attitudeDerivativePureYaw)
	{
		// For identity attitude and body rate (0,0,wz), q_dot = 1/2 * (0,0,0,wz).
		AngularRateVector<F> omega(rps(0.0), rps(0.0), rps(0.2));
		auto                 qd = attitudeDerivative(rotation::Quaternion::identity(), omega);
		EXPECT_NEAR(qd.w.value(), 0.0, 1e-15);
		EXPECT_NEAR(qd.x.value(), 0.0, 1e-15);
		EXPECT_NEAR(qd.y.value(), 0.0, 1e-15);
		EXPECT_NEAR(qd.z.value(), 0.1, 1e-15);    // 1/2 * 0.2
	}

	TEST_F(KinematicStateTest, constantYawRateIntegratesToExpectedAngle)
	{
		// Constant body yaw rate 0.1 rad/s for 1 s -> yaw ~ 0.1 rad.
		KinematicState<F> r(Pose::identity(), VelocityVector<F>(), AngularRateVector<F>(rps(0.0), rps(0.0), rps(0.1)));
		for (int i = 0; i < 1000; ++i)
			r = integrateKinematics(r, AccelerationVector<F>(), AngularRateVector<F>(), 0.001_s);
		const auto e   = rotation::toEulerAngles(r.attitude());
		const double yaw = units::angle::radians<>(e.yaw()).value();
		EXPECT_NEAR(yaw, 0.1, 1e-4);
	}

	TEST_F(KinematicStateTest, attitudeStaysUnitQuaternion)
	{
		// After propagation the attitude quaternion remains normalized (renormalized each step).
		KinematicState<F> r(Pose::identity(), VelocityVector<F>(), AngularRateVector<F>(rps(0.3), rps(-0.2), rps(0.1)));
		for (int i = 0; i < 500; ++i)
			r = integrateKinematics(r, AccelerationVector<F>(), AngularRateVector<F>(), 0.002_s);
		const auto q = r.attitude();
		EXPECT_NEAR(q.norm().value(), 1.0, 1e-9);
	}
}    // namespace coordinates

#endif    // kinematicStateTest_h
