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

#ifndef vectorFirstClassTest_h
#define vectorFirstClassTest_h

//------------------------
//	INCLUDES
//------------------------

#include <gtest/gtest.h>

#include "gtest_units.h"
#include "vector.h"
#include "vector3.h"

inline namespace coordinates
{
	using namespace units::literals;
	using namespace units::length;
	using namespace units::area;
	using units::dimensionless;

	//	----------------------------------------------------------------------------
	//	CLASS		VectorFirstClassTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for the frame-agnostic `vec::Vector3` and the frame-tagged `Vector<Frame>`.
	//  ----------------------------------------------------------------------------
	class VectorFirstClassTest : public ::testing::Test
	{
	protected:
		using WgsHoriz = horizontalDatums::WGS84_G1674;
		using Wgs      = datums::WGS84_G1674;
		using VecECEF  = Vector<coordinateFrames::ECEFFrame<WgsHoriz>>;
		using VecENU   = Vector<coordinateFrames::ENUFrame<Wgs>>;
		using VecNED   = Vector<coordinateFrames::NEDFrame<Wgs>>;
	};

	//======================================================
	//	vec::Vector3 -- frame-agnostic algebra
	//======================================================

	TEST_F(VectorFirstClassTest, vector3_algebra)
	{
		vec::Vector3<meters<>> a(3.0_m, 4.0_m, 0.0_m);
		vec::Vector3<meters<>> b(1.0_m, 0.0_m, 0.0_m);

		EXPECT_UNITS_EQ(5.0_m, a.magnitude());
		EXPECT_UNITS_EQ(square_meters<>(25.0), a.magnitudeSquared());
		EXPECT_UNITS_EQ(4.0_m, (a + b).x());
		EXPECT_UNITS_EQ(2.0_m, (a - b).x());
		EXPECT_UNITS_EQ(6.0_m, (a * dimensionless<>(2.0)).x());
		EXPECT_TRUE(vec::Vector3<meters<>>{}.isNull());
		EXPECT_FALSE(a.isNull());

		// dot: (3,4,0).(1,0,0) = 3 m^2
		EXPECT_UNITS_EQ(square_meters<>(3.0), a.dot(b));

		// cross: (1,0,0) x (0,1,0) = (0,0,1)
		vec::Vector3<meters<>> ex(1.0_m, 0.0_m, 0.0_m);
		vec::Vector3<meters<>> ey(0.0_m, 1.0_m, 0.0_m);
		auto                   ez = ex.cross(ey);
		EXPECT_UNITS_EQ(square_meters<>(0.0), ez.x());
		EXPECT_UNITS_EQ(square_meters<>(0.0), ez.y());
		EXPECT_UNITS_EQ(square_meters<>(1.0), ez.z());

		// normalize
		auto n = a.normalized();
		EXPECT_NEAR(n.magnitude().value(), 1.0, 1e-12);
		EXPECT_NEAR(n.x().value(), 0.6, 1e-12);
		EXPECT_NEAR(n.y().value(), 0.8, 1e-12);
	}

	TEST_F(VectorFirstClassTest, vector3_constexpr)
	{
		constexpr vec::Vector3<meters<>> a(1.0_m, 2.0_m, 2.0_m);
		constexpr auto                   d = a.dot(a);
		static_assert(d == square_meters<>(9.0), "constexpr dot product");
		constexpr auto s = (a + vec::Vector3<meters<>>(1.0_m, 0.0_m, 0.0_m)).x();
		static_assert(s == 2.0_m, "constexpr addition");
		SUCCEED();
	}

	//======================================================
	//	Vector<Frame> -- concept classification
	//======================================================

	TEST_F(VectorFirstClassTest, isVectorConcept)
	{
		EXPECT_TRUE((coordinates::traits::is_vector<VecECEF>));
		EXPECT_TRUE((coordinates::traits::is_vector<VecENU>));
		EXPECT_TRUE((coordinates::traits::is_vector<VecNED>));

		// a vector is NOT a point, and a point is NOT a vector
		EXPECT_FALSE((coordinates::traits::is_point<VecECEF>));
		EXPECT_FALSE((coordinates::traits::is_vector<PositionECEF<Wgs>>));
	}

	//======================================================
	//	Vector<Frame> -- algebra
	//======================================================

	TEST_F(VectorFirstClassTest, frameVectorAlgebra)
	{
		VecECEF a(3.0_m, 4.0_m, 0.0_m);
		VecECEF b(1.0_m, 0.0_m, 0.0_m);

		EXPECT_UNITS_EQ(5.0_m, a.magnitude());
		EXPECT_UNITS_EQ(4.0_m, std::get<0>((a + b).vector()));
		EXPECT_UNITS_EQ(2.0_m, std::get<0>((a - b).vector()));
		EXPECT_UNITS_EQ(square_meters<>(3.0), a.dot(b));

		VecECEF ex(1.0_m, 0.0_m, 0.0_m);
		VecECEF ey(0.0_m, 1.0_m, 0.0_m);
		auto    ez = ex.cross(ey);    // Vector<ECEF, square_meters>
		EXPECT_UNITS_EQ(square_meters<>(1.0), std::get<2>(ez.vector()));
	}

	//======================================================
	//	Vector<Frame> -- frame conversion is ROTATION ONLY
	//======================================================

	TEST_F(VectorFirstClassTest, enuToEcefRotationOnly)
	{
		// East unit vector at origin (lat=34, lon=-118): in ECEF this is (-sin lon, cos lon, 0).
		PositionGeodetic<Wgs> origin(34.0_deg, -118.0_deg, 0.0_m);
		VecENU                enuEast(1.0_m, 0.0_m, 0.0_m, origin);
		VecECEF               ecef(enuEast);

		// The tip-minus-origin realization rotates through the ellipsoidal geodetic<->ECEF transform, so a
		// 1 m local offset maps to an ECEF chord that differs from unit length by the local curvature over
		// that metre (~1e-10). The tolerance reflects that geometry, not a translation leak.
		EXPECT_UNITS_NEAR(1.0_m, ecef.magnitude(), 1.0e-8_m);

		// East axis in ECEF: (-sin(-118), cos(-118), 0).
		const double lon = -118.0 * 3.14159265358979323846 / 180.0;
		EXPECT_NEAR(std::get<0>(ecef.vector()).value(), -std::sin(lon), 1e-8);
		EXPECT_NEAR(std::get<1>(ecef.vector()).value(), std::cos(lon), 1e-8);
		EXPECT_NEAR(std::get<2>(ecef.vector()).value(), 0.0, 1e-8);
	}

	TEST_F(VectorFirstClassTest, upVectorToEcefIsRadial)
	{
		// Up unit vector at (lat, lon): in ECEF this is the geodetic radial
		// (cos lat cos lon, cos lat sin lon, sin lat).
		const double         latd = 45.0, lond = 10.0;
		PositionGeodetic<Wgs> origin(degrees<>(latd), degrees<>(lond), 0.0_m);
		VecENU                up(0.0_m, 0.0_m, 1.0_m, origin);
		VecECEF               ecef(up);

		EXPECT_UNITS_NEAR(1.0_m, ecef.magnitude(), 1.0e-8_m);
		const double lat = latd * 3.14159265358979323846 / 180.0;
		const double lon = lond * 3.14159265358979323846 / 180.0;
		EXPECT_NEAR(std::get<0>(ecef.vector()).value(), std::cos(lat) * std::cos(lon), 1e-8);
		EXPECT_NEAR(std::get<1>(ecef.vector()).value(), std::cos(lat) * std::sin(lon), 1e-8);
		EXPECT_NEAR(std::get<2>(ecef.vector()).value(), std::sin(lat), 1e-8);
	}

	//======================================================
	//	Eigen interoperability (opt-in; runs only when <Eigen/Core> is available)
	//======================================================

#ifdef VECTOR3_HAS_EIGEN
	TEST_F(VectorFirstClassTest, eigenRoundTrip)
	{
		vec::Vector3<meters<>>      a(1.0_m, 2.0_m, 3.0_m);
		Eigen::Matrix<meters<>, 3, 1> e = a.toEigen();
		EXPECT_UNITS_EQ(2.0_m, e(1));
		auto b = vec::Vector3<meters<>>::fromEigen(e);
		EXPECT_TRUE(a == b);

		// scaling by a plain scalar preserves dimension through Eigen
		Eigen::Matrix<meters<>, 3, 1> scaled = e * 2.0;
		EXPECT_UNITS_EQ(4.0_m, scaled(1));
	}
#endif
}    // namespace coordinates

#endif    // vectorFirstClassTest_h
