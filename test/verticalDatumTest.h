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

#ifndef verticalDatumTest_h__
#define verticalDatumTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <datum.h>
#include <heights.h>
#include <positionGeodetic.h>

namespace
{
	// The fixture for testing
	class VerticalDatumTest : public ::testing::Test {
	protected:
		VerticalDatumTest(){}
		~VerticalDatumTest() override {}
		void SetUp() override {}
		void TearDown() override {}
	};

	//------------------------------------------------------------------------------------------------------
	//	Compile-time proof that the height kinds enforce the datum distinction. These `static_assert`s fail
	//	to COMPILE if the type-safety regresses, so they are the strongest possible regression guard.
	//------------------------------------------------------------------------------------------------------

	// An ellipsoidal and an orthometric height are DISTINCT types (mixing them is a compile error).
	static_assert(!std::is_same_v<coordinates::heights::Ellipsoidal, coordinates::heights::Orthometric>,
	              "ellipsoidal and orthometric heights must be distinct types");

	// A plain length constructs INTO a kind implicitly (so `LLA p(.., .., 100_m)` still compiles) ...
	static_assert(std::is_constructible_v<coordinates::heights::Ellipsoidal, units::length::meters<double>>,
	              "a plain meters<> must construct into an ellipsoidal height");
	static_assert(std::is_constructible_v<coordinates::heights::Orthometric, units::length::meters<double>>,
	              "a plain meters<> must construct into an orthometric height");

	// ... but the two kinds carry different tags, which is the type-level fact `units::kind` keys on to make
	// mixing them in arithmetic / comparison a hard compile error. (The mixing itself cannot be probed with
	// `requires`: the cross-tag operators exist as overloads whose bodies `static_assert`, so the expression
	// is "well-formed" to overload resolution and only detonates on instantiation -- the tag inequality below
	// is the sound, testable guarantee.)
	static_assert(coordinates::heights::Ellipsoidal::tag() != coordinates::heights::Orthometric::tag(),
	              "ellipsoidal and orthometric heights must carry different kind tags");

	// The datum picks the right height kind: an ellipsoid-referenced datum measures ellipsoidal height, a
	// geoid-referenced datum measures orthometric height.
	static_assert(std::is_same_v<coordinates::heights::kind_for<coordinates::datums::WGS84_G1674>, coordinates::heights::Ellipsoidal>,
	              "an ellipsoid-referenced datum must measure ellipsoidal height");
	static_assert(std::is_same_v<coordinates::heights::kind_for<coordinates::datums::NAD83_NAVD88>, coordinates::heights::Orthometric>,
	              "a geoid-referenced datum must measure orthometric height");

	TEST_F(VerticalDatumTest, is_vertical_datum)
	{
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::ellipsoids::GRS80>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::ellipsoids::WGS84>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::geoids::GEOID12A>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::geoids::NULL_GEOID>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::geoids::EGM96>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::topography::DTED>);
		EXPECT_TRUE(coordinates::traits::is_vertical_datum<coordinates::topography::NULL_TOPOGRAPHY>);

		EXPECT_FALSE(coordinates::traits::is_vertical_datum<coordinates::horizontalDatums::IGS08>);
		EXPECT_FALSE(coordinates::traits::is_vertical_datum<coordinates::coordinateFrames::ECEFFrame<coordinates::horizontalDatums::WGS84_G1674>>);
		EXPECT_FALSE(coordinates::traits::is_vertical_datum<double>);
	}

	TEST_F(VerticalDatumTest, vertical_datum_traits)
	{
		EXPECT_TRUE((std::is_same_v<coordinates::traits::vertical_datum_traits<coordinates::geoids::GEOID12A>::base_datum, coordinates::ellipsoids::GRS80>) );
		EXPECT_TRUE((std::is_same_v<coordinates::traits::vertical_datum_traits<coordinates::geoids::EGM96>::base_datum, coordinates::ellipsoids::WGS84>) );
		EXPECT_TRUE((std::is_same_v<coordinates::traits::vertical_datum_traits<coordinates::topography::DTED>::base_datum, coordinates::geoids::EGM96>) );
		EXPECT_TRUE((std::is_same_v<coordinates::traits::vertical_datum_traits<coordinates::ellipsoids::WGS84>::base_datum, coordinates::ellipsoids::WGS84>) );

		EXPECT_TRUE((std::is_same_v<coordinates::traits::vertical_datum_traits<double>::base_datum, void>) );

		EXPECT_UNITS_NEAR(-33.5841_m, coordinates::traits::vertical_datum_traits<coordinates::geoids::GEOID12A>::correctionValue(41.87917_deg, -87.62917_deg), 1.0e-3_m);
		EXPECT_UNITS_NEAR(-33.94_m, coordinates::traits::vertical_datum_traits<coordinates::geoids::EGM96>::correctionValue(41.87917_deg, -87.62917_deg), 1.0e-2_m);
		EXPECT_UNITS_NEAR(0.0_m, coordinates::traits::vertical_datum_traits<coordinates::ellipsoids::WGS84>::correctionValue(41.87917_deg, -87.62917_deg), 1.0e-3_m);
		EXPECT_UNITS_NEAR(191.0_m, coordinates::traits::vertical_datum_traits<coordinates::topography::DTED>::correctionValue(41.87917_deg, -87.62917_deg), 1.0e-2_m);	// non-interpolated. Truth from `gdallocationinfo -wgs84 n41.dt2 -87.62917 41.87917`
	}

	TEST_F(VerticalDatumTest, convertToEllipsoidHeight)
	{
		// truth data from VDatum: http://vdatum.noaa.gov/download_agreement.php
		EXPECT_UNITS_NEAR(12.0_m, coordinates::convertToEllipsoidHeight<coordinates::ellipsoids::WGS84>(41.87917_deg, -87.62917_deg, 12.0_m), 1.5e-4_m);
		EXPECT_UNITS_NEAR(-33.5841_m, coordinates::convertToEllipsoidHeight<coordinates::geoids::GEOID12A>(41.87917_deg, -87.62917_deg, 0_m), 1.5e-4_m);
	
		// no truth data available for this test... the result seems reasonable.
		EXPECT_UNITS_NEAR(157.056_m, coordinates::convertToEllipsoidHeight<coordinates::topography::DTED>(41.87917_deg, -87.62917_deg, 0_m), 5.0e-4_m);
	}

	TEST_F(VerticalDatumTest, convertFromEllipsoidHeight)
	{
		// truth data from VDatum: http://vdatum.noaa.gov/download_agreement.php
		EXPECT_UNITS_NEAR(12.0_m, coordinates::convertFromEllipsoidHeight<coordinates::ellipsoids::WGS84>(41.87917_deg, -87.62917_deg, 12.0_m), 1.5e-4_m);
		EXPECT_UNITS_NEAR(0_m, coordinates::convertFromEllipsoidHeight<coordinates::geoids::GEOID12A>(41.87917_deg, -87.62917_deg, -33.5841_m), 1.5e-4_m);

		// no truth data available for this test... the result seems reasonable.
		EXPECT_UNITS_NEAR(0_m, coordinates::convertFromEllipsoidHeight<coordinates::topography::DTED>(41.87917_deg, -87.62917_deg, 157.056_m), 5.0e-4_m);
	}

	// The README-advertised height accessors on a geodetic position. A position on a geoid-referenced datum
	// stores an orthometric height; toEllipsoidHeight() adds the undulation, toOrthometricHeight() returns it
	// unchanged. The results are tagged so an HAE and an MSL height can never be silently interchanged.
	TEST_F(VerticalDatumTest, positionHeightAccessors)
	{
		using namespace coordinates;
		using namespace units::literals;

		// Chicago on NAD83/NAVD88 (a GEOID12A datum): a 0 m orthometric height sits at -33.5841 m HAE.
		PositionGeodetic<datums::NAD83_NAVD88> chicago(41.87917_deg, -87.62917_deg, 0.0_m);

		// altitude() is tagged Orthometric for a geoid datum; the stored value is unchanged.
		static_assert(std::is_same_v<decltype(chicago.altitude()), heights::Orthometric>,
		              "a geoid-datum position's altitude() must be an orthometric height");
		EXPECT_UNITS_NEAR(0.0_m, chicago.altitude(), 1.5e-4_m);

		// toEllipsoidHeight() applies the undulation; toOrthometricHeight() is the identity here.
		static_assert(std::is_same_v<decltype(chicago.toEllipsoidHeight()), heights::Ellipsoidal>,
		              "toEllipsoidHeight() must return an ellipsoidal height");
		static_assert(std::is_same_v<decltype(chicago.toOrthometricHeight()), heights::Orthometric>,
		              "toOrthometricHeight() must return an orthometric height");
		EXPECT_UNITS_NEAR(-33.5841_m, chicago.toEllipsoidHeight(), 1.5e-4_m);
		EXPECT_UNITS_NEAR(0.0_m, chicago.toOrthometricHeight(), 1.5e-4_m);

		// A position on an ellipsoid-only datum measures ellipsoidal height, and the two converters agree.
		PositionGeodetic<datums::WGS84_G1674> gps(41.87917_deg, -87.62917_deg, 100.0_m);
		static_assert(std::is_same_v<decltype(gps.altitude()), heights::Ellipsoidal>,
		              "an ellipsoid-datum position's altitude() must be an ellipsoidal height");
		EXPECT_UNITS_NEAR(100.0_m, gps.toEllipsoidHeight(), 1.5e-4_m);
		EXPECT_UNITS_NEAR(100.0_m, gps.toOrthometricHeight(), 1.5e-4_m);
	}
}

#endif // verticalDatumTest_h__