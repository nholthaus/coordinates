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

#ifndef kinematicsTest_h
#define kinematicsTest_h

//------------------------
//	INCLUDES
//------------------------

#include <gtest/gtest.h>

#include "gtest_units.h"
#include "kinematics.h"

inline namespace coordinates
{
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	CLASS		KinematicsTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for the frame-tagged kinematic-state vectors and Earth rotation.
	//  ----------------------------------------------------------------------------
	class KinematicsTest : public ::testing::Test
	{
	protected:
		using WgsH   = horizontalDatums::WGS84_G1674;
		using Wgs    = datums::WGS84_G1674;
		using EcefV  = VelocityVector<coordinateFrames::ECEFFrame<WgsH>>;
		using NedV   = VelocityVector<coordinateFrames::NEDFrame<Wgs>>;
		using EcefW  = AngularRateVector<coordinateFrames::ECEFFrame<WgsH>>;
	};

	TEST_F(KinematicsTest, stateVectorsAreVectors)
	{
		EXPECT_TRUE((coordinates::traits::is_vector<VelocityVector<coordinateFrames::ECEFFrame<WgsH>>>));
		EXPECT_TRUE((coordinates::traits::is_vector<AccelerationVector<coordinateFrames::NEDFrame<Wgs>>>));
		EXPECT_TRUE((coordinates::traits::is_vector<AngularRateVector<coordinateFrames::ENUFrame<Wgs>>>));
	}

	TEST_F(KinematicsTest, velocityAlgebra)
	{
		using mps = units::velocity::meters_per_second<>;
		EcefV v(mps(3.0), mps(4.0), mps(0.0));
		EXPECT_UNITS_EQ(mps(5.0), v.magnitude());

		EcefV w(mps(1.0), mps(0.0), mps(0.0));
		EXPECT_UNITS_EQ(mps(4.0), std::get<0>((v + w).vector()));
		EXPECT_UNITS_EQ(mps(6.0), std::get<0>((v * units::dimensionless<>(2.0)).vector()));
	}

	TEST_F(KinematicsTest, earthRate)
	{
		using rps = units::angular_velocity::radians_per_second<>;
		auto wie = earth::angularVelocity<WgsH>();
		EXPECT_UNITS_EQ(rps(0.0), std::get<0>(wie.vector()));
		EXPECT_UNITS_EQ(rps(0.0), std::get<1>(wie.vector()));
		EXPECT_UNITS_NEAR(rps(7.2921150e-5), std::get<2>(wie.vector()), rps(1.0e-15));
		// magnitude equals the scalar rotation rate
		EXPECT_UNITS_NEAR(rps(7.2921150e-5), wie.magnitude(), rps(1.0e-15));
	}

	TEST_F(KinematicsTest, velocityFrameConversionIsRotation)
	{
		// A velocity expressed in NED at an origin converts into ECEF by rotation only; magnitude is preserved.
		using mps = units::velocity::meters_per_second<>;
		PositionGeodetic<Wgs> origin(45.0_deg, 10.0_deg, 0.0_m);
		NedV                  nedVel(mps(10.0), mps(0.0), mps(0.0), origin);    // 10 m/s due North
		EcefV                 ecefVel(nedVel);
		EXPECT_UNITS_NEAR(mps(10.0), ecefVel.magnitude(), mps(1.0e-7));
	}

	TEST_F(KinematicsTest, transportRateAtEquatorEastward)
	{
		// At the equator (phi=0, h=0) moving East at vE: omega_en = [vE/N, 0, 0], with N = a (equatorial).
		using mps = units::velocity::meters_per_second<>;
		using rps = units::angular_velocity::radians_per_second<>;
		PositionGeodetic<Wgs> pos(0.0_deg, 0.0_deg, 0.0_m);
		NedV                  vNED(mps(0.0), mps(100.0), mps(0.0), pos);    // 100 m/s East

		auto omegaEN = transportRate(pos, vNED);
		using Ellipsoid = traits::horizontal_datum_traits<traits::datum_traits<Wgs>::horizontal_datum>::reference_ellipsoid;
		const double a  = Ellipsoid::a().value();    // N == a at the equator
		EXPECT_NEAR(std::get<0>(omegaEN.vector()).value(), 100.0 / a, 1e-15);    // vE/N
		EXPECT_NEAR(std::get<1>(omegaEN.vector()).value(), 0.0, 1e-18);          // -vN/M = 0
		EXPECT_NEAR(std::get<2>(omegaEN.vector()).value(), 0.0, 1e-18);          // -vE*tan(0)/N = 0
		(void)rps{};
	}

	TEST_F(KinematicsTest, coriolisTerm)
	{
		// omega = [0,0,W], v = [vx,0,0]  ->  2*(omega x v) = [0, 2*W*vx, 0].
		using mps  = units::velocity::meters_per_second<>;
		using rps  = units::angular_velocity::radians_per_second<>;
		using mps2 = units::acceleration::meters_per_second_squared<>;
		const double W  = 7.2921150e-5;
		const double vx = 200.0;
		EcefW omega(rps(0.0), rps(0.0), rps(W));
		EcefV v(mps(vx), mps(0.0), mps(0.0));

		auto acc = coriolisAcceleration(omega, v);
		EXPECT_NEAR(std::get<0>(acc.vector()).value(), 0.0, 1e-18);
		EXPECT_NEAR(std::get<1>(acc.vector()).value(), 2.0 * W * vx, 1e-15);
		EXPECT_NEAR(std::get<2>(acc.vector()).value(), 0.0, 1e-18);
		(void)mps2{};
	}
}    // namespace coordinates

#endif    // kinematicsTest_h
