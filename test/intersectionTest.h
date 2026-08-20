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

#ifndef intersectionTest_h
#define intersectionTest_h

//------------------------
//	INCLUDES
//------------------------

#include <gtest/gtest.h>

#include "gtest_units.h"
#include "positionECEF.h"
#include "ellipsoid.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		IntersectionTest
	//  ----------------------------------------------------------------------------
	///	@brief		Unit tests for ellipsoid intersection utilities.
	//  ----------------------------------------------------------------------------
	class IntersectionTest : public ::testing::Test
	{
	};

	TEST_F(IntersectionTest, rayIntersectsEllipsoid_frontFace)
	{
		using Ecef  = PositionECEF<WGS84_G1674>;

		const auto a = horizontal_datum_traits<datum_traits<WGS84_G1674>::horizontal_datum>::reference_ellipsoid::a();

		// Start 1000 m above the +X axis and shoot back toward origin.
		const Ecef origin((a + 1000.0_m), 0.0_m, 0.0_m);
		const CartesianTuple dir(-1.0_m, 0.0_m, 0.0_m);    // direction need not be normalized

		auto hit = intersectEllipsoid(origin, dir);

		EXPECT_TRUE(hit.hit());
		EXPECT_TRUE(hit.hitEllipsoid());
		EXPECT_UNITS_NEAR(a, std::get<0>(hit.ellipsoidECEF()), 1.0e-6_m);
		EXPECT_UNITS_NEAR(0.0_m, std::get<1>(hit.ellipsoidECEF()), 1.0e-6_m);
		EXPECT_UNITS_NEAR(0.0_m, std::get<2>(hit.ellipsoidECEF()), 1.0e-6_m);
	}

	TEST_F(IntersectionTest, rayMissesEllipsoid)
	{
		using Ecef  = PositionECEF<WGS84_G1674>;

		const auto a = traits::horizontal_datum_traits<typename traits::datum_traits<WGS84_G1674>::horizontal_datum>::reference_ellipsoid::a();

		Ecef origin((a + 1000.0_m), 0.0_m, 0.0_m);

		// Shoot in +Y direction (tangent-ish from +X); should miss.
		CartesianTuple dir(0.0_m, 1.0_m, 0.0_m);

		auto hit = intersectEllipsoid(origin, dir);

		EXPECT_FALSE(hit.hit());
		EXPECT_FALSE(hit.hitEllipsoid());
	}
}

#endif
