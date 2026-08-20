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

#ifndef rangesTest_h__
#define rangesTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <type_traits>

#include <positionAER.h>
#include <positionECEF.h>
#include <positionGeodetic.h>
#include <ranges.h>

using namespace coordinates;
using namespace units;
using namespace units::angle;
using namespace units::length;

namespace
{
	class RangesTest : public ::testing::Test
	{
	protected:
		RangesTest() {}
		~RangesTest() override {}
		void SetUp() override {}
		void TearDown() override {}
	};

	//------------------------------------------------------------------------------------------------------
	//	Compile-time proof of the range-kind distinction -- and of NON-distinction where the quantity is the
	//	same. A straight-line distance through 3-space (Euclidean, and equally a "slant range") carries no
	//	reference surface, so it is ONE kind. A geodesic distance measures an arc along the surface, so it is
	//	a distinct kind: a surface distance and a straight-line distance must not be silently interchanged.
	//------------------------------------------------------------------------------------------------------

	static_assert(!std::is_same_v<ranges::Geodesic, ranges::Euclidean>);
	static_assert(ranges::Geodesic::tag() != ranges::Euclidean::tag(),
	              "a geodesic (surface) distance and a straight-line distance must be distinguishable");

	// A plain length constructs into a range implicitly, and unwraps explicitly.
	static_assert(std::is_constructible_v<ranges::Euclidean, meters<double>>);
	static_assert(std::is_same_v<decltype(std::declval<ranges::Euclidean>().to<meters<double>>()), meters<double>>);

	TEST_F(RangesTest, accessorsAreTagged)
	{
		// A slant range is a straight-line distance: AER::range() is a Euclidean, not a distinct kind.
		AER aer(30.0_deg, 45.0_deg, 1000.0_m, 40.0_deg, -75.0_deg, 0.0_m);
		static_assert(std::is_same_v<decltype(aer.range()), ranges::Euclidean>,
		              "PositionAER::range() (a slant range) is a straight-line Euclidean distance");
		EXPECT_UNITS_EQ(1000.0_m, aer.range());

		ECEF a(1.0_m, 0.0_m, 0.0_m);
		ECEF b(0.0_m, 0.0_m, 0.0_m);
		static_assert(std::is_same_v<decltype(a.distance(b)), ranges::Euclidean>,
		              "a straight-line distance must be a Euclidean distance");
		static_assert(std::is_same_v<decltype(a.magnitude()), ranges::Euclidean>,
		              "a magnitude must be a Euclidean distance");
		EXPECT_UNITS_EQ(1.0_m, a.distance(b));

		LLA boston(42.3601_deg, -71.0589_deg, 0.0_m);
		LLA nyc(40.7128_deg, -74.0059_deg, 0.0_m);
		static_assert(std::is_same_v<decltype(boston.distanceTo(nyc)), ranges::Geodesic>,
		              "a geodesic surface distance must be a Geodesic range");
		EXPECT_UNITS_GT(boston.distanceTo(nyc), 0.0_m);
	}

	// The AER origin altitude is a height, decoupled from the slant-range unit: the two template
	// parameters can be set independently. Default AER keeps both as meters.
	TEST_F(RangesTest, originAltitudeDecoupledFromRange)
	{
		// Range in kilometers, origin altitude given in feet (converted into the geodetic origin).
		PositionAER<datums::WGS84_G1674, degrees, kilometers> aer(
		        10.0_deg, 20.0_deg, 5.0_km, 40.0_deg, -75.0_deg, 100.0_ft);
		static_assert(std::is_same_v<decltype(aer.range()), ranges::Euclidean>);
		EXPECT_UNITS_EQ(5.0_km, aer.range());
	}
}

#endif    // rangesTest_h__
