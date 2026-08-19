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

#ifndef coordinateTest_h__
#define coordinateTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <type_traits>

#include <coordinate.h>
#include <coordinates.h>

using namespace coordinates;
using namespace coordinates::coordinateFrames;
using namespace units;
using namespace units::length;
using namespace units::angle;

namespace
{
	class CoordinateTest : public ::testing::Test
	{
	protected:
		CoordinateTest() {}
		~CoordinateTest() override {}
		void SetUp() override {}
		void TearDown() override {}
	};

	using WgsHoriz = horizontalDatums::WGS84_G1674;
	using Wgs      = datums::WGS84_G1674;
	using EcefCoord = Coordinate<ECEFFrame<WgsHoriz>, CartesianTuple>;
	using GeoCoord  = Coordinate<Geodetic3DFrame<Wgs>, SphericalTuple>;

	// The unified body satisfies the same structural contract the position classes do.
	static_assert(coordinates::traits::is_point<EcefCoord>);
	static_assert(coordinates::traits::is_point<GeoCoord>);

	// C-1a: prove the shared body's storage + point interface directly (no aliases wired yet).
	TEST_F(CoordinateTest, storesAndReturnsTheTuple)
	{
		EcefCoord c(CartesianTuple(1.0_m, 2.0_m, 3.0_m));
		EXPECT_UNITS_EQ(1.0_m, std::get<0>(c.point()));
		EXPECT_UNITS_EQ(2.0_m, std::get<1>(c.point()));
		EXPECT_UNITS_EQ(3.0_m, std::get<2>(c.point()));

		c.setPoint(CartesianTuple(4.0_m, 5.0_m, 6.0_m));
		EXPECT_UNITS_EQ(4.0_m, std::get<0>(c.point()));
		EXPECT_UNITS_EQ(6.0_m, std::get<2>(c.point()));
	}

	// Component constructor forwards into the tuple; heterogeneous frames (geodetic = angle/angle/length) work.
	TEST_F(CoordinateTest, componentConstructor)
	{
		GeoCoord g(10.0_deg, 20.0_deg, 100.0_m);
		EXPECT_UNITS_EQ(10.0_deg, std::get<0>(g.point()));
		EXPECT_UNITS_EQ(20.0_deg, std::get<1>(g.point()));
		EXPECT_UNITS_EQ(100.0_m, std::get<2>(g.point()));
	}

	// The converting constructor runs the frame pipeline: an ECEF coordinate built from a geodetic one must
	// match the legacy PositionECEF-from-LLA truth (WGS84, from the existing positionGeodetic test data).
	TEST_F(CoordinateTest, convertingConstructorMatchesLegacy)
	{
		// LLA (42.459284, -71.268002, 50 m) -> ECEF, MATLAB truth from positionGeodeticTest.
		GeoCoord  lla(42.459284_deg, -71.268002_deg, 50.0_m);
		EcefCoord ecef(lla);
		EXPECT_UNITS_NEAR(1513460.90425574_m, std::get<0>(ecef.point()), 1.0e-6_m);
		EXPECT_UNITS_NEAR(-4463118.57366578_m, std::get<1>(ecef.point()), 1.0e-6_m);
		EXPECT_UNITS_NEAR(4283412.81104103_m, std::get<2>(ecef.point()), 1.0e-6_m);

		// round-trip back to geodetic.
		GeoCoord back(ecef);
		EXPECT_UNITS_NEAR(42.459284_deg, std::get<0>(back.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(-71.268002_deg, std::get<1>(back.point()), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(50.0_m, std::get<2>(back.point()), 1.0e-6_m);
	}

	// A local (origin-carrying) frame converts through the is_local branch. Build an ENU Coordinate with an
	// origin, then a NED Coordinate from it: the NED<->ENU shortcut is a pure axis swap (E,N,U)->(N,E,-U).
	TEST_F(CoordinateTest, localFrameConversionAndFastPath)
	{
		using EnuCoord = Coordinate<ENUFrame<WgsHoriz>, CartesianTuple>;
		using NedCoord = Coordinate<NEDFrame<WgsHoriz>, CartesianTuple>;
		using AerCoord = Coordinate<AERFrame<WgsHoriz>, CartesianTuple>;    // AER stores its SphericalTuple

		static_assert(coordinates::traits::is_local_frame<ENUFrame<WgsHoriz>>);

		// An ENU point relative to a Boston-ish origin, expressed via a legacy PositionENU with the same origin
		// so both share the identical FrameData.origin (the fast-path precondition).
		LLA origin(42.3601_deg, -71.0589_deg, 0.0_m);
		ENU legacyEnu(100.0_m, 200.0_m, 300.0_m, origin);    // (E,N,U) about origin

		// Build the unified ENU Coordinate from the legacy ENU (adopts its origin), then swap to NED. ENU->ENU
		// is a different C++ type but the same frame, so it round-trips through ECEF (as the legacy code also
		// does -- no ENU->ENU shortcut exists), hence the ~nm float drift, not exact.
		EnuCoord enu(legacyEnu);
		EXPECT_UNITS_NEAR(100.0_m, std::get<0>(enu.point()), 5.0e-6_m);
		EXPECT_UNITS_NEAR(200.0_m, std::get<1>(enu.point()), 5.0e-6_m);
		EXPECT_UNITS_NEAR(300.0_m, std::get<2>(enu.point()), 5.0e-6_m);

		NedCoord ned(enu);    // NED<->ENU fast path: (E,N,U) -> (N,E,-U), exact
		EXPECT_UNITS_NEAR(200.0_m, std::get<0>(ned.point()), 5.0e-6_m);     // N
		EXPECT_UNITS_NEAR(100.0_m, std::get<1>(ned.point()), 5.0e-6_m);     // E
		EXPECT_UNITS_NEAR(-300.0_m, std::get<2>(ned.point()), 5.0e-6_m);    // D = -U

		// And the unified NED Coordinate matches the legacy PositionNED-from-ENU for the same input.
		NED legacyNed(legacyEnu);
		EXPECT_UNITS_NEAR(std::get<0>(legacyNed.point()), std::get<0>(ned.point()), 5.0e-9_m);
		EXPECT_UNITS_NEAR(std::get<2>(legacyNed.point()), std::get<2>(ned.point()), 5.0e-9_m);
	}

	// The frame_axes-driven AxisAccessors mixin gives each frame its named member accessors with the right
	// (possibly tagged) return type -- ecef.x(), lla.latitude(), aer.azimuth() -- on the ONE Coordinate body.
	TEST_F(CoordinateTest, namedMemberAccessors)
	{
		EcefCoord ecef(CartesianTuple(1.0_m, 2.0_m, 3.0_m));
		static_assert(std::is_same_v<decltype(ecef.x()), meters<double>>);
		EXPECT_UNITS_EQ(1.0_m, ecef.x());
		EXPECT_UNITS_EQ(2.0_m, ecef.y());
		EXPECT_UNITS_EQ(3.0_m, ecef.z());
		ecef.setZ(9.0_m);
		EXPECT_UNITS_EQ(9.0_m, ecef.z());

		GeoCoord lla(40.0_deg, -75.0_deg, 100.0_m);
		static_assert(std::is_same_v<decltype(lla.latitude()), angles::Latitude>);
		static_assert(std::is_same_v<decltype(lla.altitude()), heights::Ellipsoidal>);    // WGS84 is ellipsoid-referenced
		EXPECT_UNITS_EQ(40.0_deg, lla.latitude());
		EXPECT_UNITS_EQ(-75.0_deg, lla.longitude());
		EXPECT_UNITS_EQ(100.0_m, lla.altitude());
		EXPECT_UNITS_NEAR(39.8107_deg, lla.geocentricLatitude(), 1.0e-3_deg);    // geocentric < geodetic

		Coordinate<AERFrame<WgsHoriz>, SphericalTuple> aer(SphericalTuple(30.0_deg, 45.0_deg, 1000.0_m));
		static_assert(std::is_same_v<decltype(aer.azimuth()), angles::Azimuth>);
		static_assert(std::is_same_v<decltype(aer.range()), ranges::Euclidean>);
		EXPECT_UNITS_EQ(30.0_deg, aer.azimuth());
		EXPECT_UNITS_EQ(1000.0_m, aer.range());
	}
}

#endif    // coordinateTest_h__
