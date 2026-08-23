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

#ifndef anglesTest_h__
#define anglesTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <type_traits>

#include <angles.h>
#include <positionAER.h>
#include <positionGeodetic.h>

using namespace coordinates;
using namespace units;
using namespace units::angle;
using namespace units::length;

namespace
{
	class AnglesTest : public ::testing::Test
	{
	protected:
		AnglesTest() {}
		~AnglesTest() override {}
		void SetUp() override {}
		void TearDown() override {}
	};

	//------------------------------------------------------------------------------------------------------
	//	Compile-time proof that the angle kinds keep the seven angle roles apart. These `static_assert`s
	//	fail to COMPILE if the type-safety regresses. (The mixing itself cannot be probed with `requires`:
	//	`units::kind`'s cross-tag operators exist as overloads whose bodies `static_assert`, so the tag
	//	inequality below is the sound, testable guarantee.)
	//------------------------------------------------------------------------------------------------------

	// The position-angle kinds are all distinct types ...
	static_assert(!std::is_same_v<angles::Latitude, angles::Longitude>);
	static_assert(!std::is_same_v<angles::Latitude, angles::Azimuth>);
	static_assert(!std::is_same_v<angles::Azimuth, angles::Elevation>);

	// ... carrying different tags, which is what makes mixing them a compile error. THE headline: a
	// latitude and an azimuth (both `degrees<>`, both `SphericalTuple` slot 0) can no longer be confused.
	static_assert(angles::Latitude::tag() != angles::Azimuth::tag(),
	              "a latitude and an azimuth must be distinguishable at the type level");
	static_assert(angles::Longitude::tag() != angles::Elevation::tag(),
	              "a longitude and an elevation must be distinguishable at the type level");

	// A plain angle constructs into an angle kind implicitly (so existing construction is unaffected) ...
	static_assert(std::is_constructible_v<angles::Latitude, degrees<double>>);
	static_assert(std::is_constructible_v<angles::Azimuth, degrees<double>>);

	// ... and unwraps to a plain angle explicitly.
	static_assert(std::is_same_v<decltype(std::declval<angles::Azimuth>().to<degrees<double>>()), degrees<double>>);

	TEST_F(AnglesTest, positionAccessorsAreTagged)
	{
		LLA lla(40.7128_deg, -74.0059_deg, 0.0_m);
		static_assert(std::is_same_v<decltype(lla.latitude()), angles::Latitude>,
		              "PositionGeodetic::latitude() must be an angles::Latitude");
		static_assert(std::is_same_v<decltype(lla.longitude()), angles::Longitude>,
		              "PositionGeodetic::longitude() must be an angles::Longitude");
		EXPECT_UNITS_EQ(40.7128_deg, lla.latitude());
		EXPECT_UNITS_EQ(-74.0059_deg, lla.longitude());

		AER aer(30.0_deg, 45.0_deg, 1000.0_m, 40.0_deg, -75.0_deg, 0.0_m);
		static_assert(std::is_same_v<decltype(aer.azimuth()), angles::Azimuth>,
		              "PositionAER::azimuth() must be an angles::Azimuth");
		static_assert(std::is_same_v<decltype(aer.elevation()), angles::Elevation>,
		              "PositionAER::elevation() must be an angles::Elevation");
		EXPECT_UNITS_EQ(30.0_deg, aer.azimuth());
		EXPECT_UNITS_EQ(45.0_deg, aer.elevation());
	}

	// A geodesic bearing is the same kind as an AER azimuth: both are `angles::Azimuth`, so they compare
	// and interoperate freely (a forward azimuth IS an azimuth).
	TEST_F(AnglesTest, bearingIsAzimuth)
	{
		LLA boston(42.3601_deg, -71.0589_deg, 0.0_m);
		LLA nyc(40.7128_deg, -74.0059_deg, 0.0_m);

		const auto inverse = boston.inverseTo(nyc);
		static_assert(std::is_same_v<decltype(inverse.initialBearing()), angles::Azimuth>,
		              "a geodesic bearing must be an angles::Azimuth");
		// same kind -> compares against an AER azimuth and a plain angle without ceremony
		EXPECT_UNITS_GE(inverse.initialBearing(), 0.0_deg);
		EXPECT_UNITS_LE(inverse.initialBearing(), 360.0_deg);
	}
}

#endif    // anglesTest_h__
