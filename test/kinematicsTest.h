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
}    // namespace coordinates

#endif    // kinematicsTest_h
