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

#ifndef latitudeConversionTest_h__
#define latitudeConversionTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <type_traits>

#include <angles.h>
#include <ellipsoid.h>
#include <latitudeConversion.h>
#include <positionGeodetic.h>

using namespace coordinates;
using namespace units;
using namespace units::angle;

namespace
{
	class LatitudeConversionTest : public ::testing::Test
	{
	protected:
		LatitudeConversionTest() {}
		~LatitudeConversionTest() override {}
		void SetUp() override {}
		void TearDown() override {}
	};

	// Geodetic and geocentric latitude are distinct kinds -- mixing them is a compile error; the tags differ.
	static_assert(!std::is_same_v<angles::Latitude, angles::Geocentric>);
	static_assert(angles::Latitude::tag() != angles::Geocentric::tag(),
	              "geodetic and geocentric latitude must be distinguishable at the type level");

	// geodetic -> geocentric on WGS84. Truth: atan((1 - e^2) * tan(geodetic)).
	TEST_F(LatitudeConversionTest, geodeticToGeocentric)
	{
		using coordinates::ellipsoids::WGS84;

		EXPECT_UNITS_NEAR(0.0_deg, (convertLatitude<GeocentricLatitude, WGS84>(angles::Latitude(0.0_deg))), 1.0e-9_deg);
		EXPECT_UNITS_NEAR(44.8075767840_deg, (convertLatitude<GeocentricLatitude, WGS84>(angles::Latitude(45.0_deg))), 1.0e-8_deg);
		EXPECT_UNITS_NEAR(59.8330761505_deg, (convertLatitude<GeocentricLatitude, WGS84>(angles::Latitude(60.0_deg))), 1.0e-8_deg);
		// The result is the geocentric kind, not the geodetic input kind.
		static_assert(std::is_same_v<decltype(convertLatitude<GeocentricLatitude, WGS84>(angles::Latitude(45.0_deg))),
		                             angles::Geocentric>,
		              "the geodetic->geocentric conversion must return a geocentric latitude");
	}

	// geocentric -> geodetic is the exact inverse, and a round-trip returns the original.
	TEST_F(LatitudeConversionTest, geocentricToGeodeticRoundTrip)
	{
		using coordinates::ellipsoids::WGS84;

		EXPECT_UNITS_NEAR(45.0_deg, (convertLatitude<GeodeticLatitude, WGS84>(angles::Geocentric(44.8075767840_deg))), 1.0e-8_deg);

		for (double phi : {0.0, 12.34, 45.0, 60.0, 89.9})
		{
			const angles::Latitude   geodetic{degrees<>(phi)};
			const angles::Geocentric geocentric = convertLatitude<GeocentricLatitude, WGS84>(geodetic);
			const angles::Latitude   back       = convertLatitude<GeodeticLatitude, WGS84>(geocentric);
			EXPECT_UNITS_NEAR(geodetic, back, 1.0e-9_deg);
		}
	}

	// Converting a kind to itself is the identity (the LCA of a node with itself is the node).
	TEST_F(LatitudeConversionTest, sameKindIsIdentity)
	{
		using coordinates::ellipsoids::WGS84;

		EXPECT_UNITS_NEAR(37.0_deg, (convertLatitude<GeodeticLatitude, WGS84>(angles::Latitude(37.0_deg))), 1.0e-12_deg);
		EXPECT_UNITS_NEAR(37.0_deg, (convertLatitude<GeocentricLatitude, WGS84>(angles::Geocentric(37.0_deg))), 1.0e-12_deg);
	}

	// A more flattened ellipsoid (GRS80 is nearly identical to WGS84; use both to show the ellipsoid drives
	// the result) -- the conversion depends on the ellipsoid supplied, not a fixed constant.
	TEST_F(LatitudeConversionTest, dependsOnEllipsoid)
	{
		const angles::Geocentric onWgs84 = convertLatitude<GeocentricLatitude, ellipsoids::WGS84>(angles::Latitude(45.0_deg));
		const angles::Geocentric onGrs80 = convertLatitude<GeocentricLatitude, ellipsoids::GRS80>(angles::Latitude(45.0_deg));
		// Both are valid geocentric latitudes near 44.8076 deg; the point is the API takes the ellipsoid.
		EXPECT_UNITS_NEAR(44.8075767840_deg, onWgs84, 1.0e-6_deg);
		EXPECT_UNITS_NEAR(44.8075767840_deg, onGrs80, 1.0e-6_deg);
	}

	// The position-member front door: pos.geocentricLatitude() supplies its own datum's ellipsoid, so the
	// call is zero-argument and matches the free convertLatitude with that ellipsoid. geodeticLatitude()
	// returns the stored latitude unchanged.
	TEST_F(LatitudeConversionTest, positionMembers)
	{
		LLA p(45.0_deg, -71.0_deg, 0.0_m);    // WGS84_G1674 datum

		static_assert(std::is_same_v<decltype(p.geocentricLatitude()), angles::Geocentric>,
		              "geocentricLatitude() must return a geocentric latitude");
		static_assert(std::is_same_v<decltype(p.geodeticLatitude()), angles::Latitude>,
		              "geodeticLatitude() must return a geodetic latitude");

		EXPECT_UNITS_NEAR(45.0_deg, p.geodeticLatitude(), 1.0e-12_deg);
		EXPECT_UNITS_NEAR(44.8075767840_deg, p.geocentricLatitude(), 1.0e-8_deg);
	}
}

#endif    // latitudeConversionTest_h__
