#pragma once

#include "gtest_units.h"

#include <coordinates.h>

using namespace coordinates;
using namespace coordinates::datums;
using namespace units;
using namespace units::angle;
using namespace units::length;

//======================================================
//  Vector / Position Truth Tests
//======================================================

class VectorTest : public ::testing::Test
{
protected:
	typedef WGS84_G1674 Datum;

	// Canonical origin: equator, prime meridian
	PositionGeodetic<Datum, degrees, meters, double> origin{degrees<double>{0}, degrees<double>{0}, meters<double>{0}};

	static constexpr double eps = 1e-9;
};

TEST_F(VectorTest, VectorConceptClassification)
{
	static_assert(is_vector<VectorECEF<WGS84_G1674>>);
	static_assert(is_vector<VectorENU<WGS84_G1674>>);
	static_assert(is_vector<VectorNED<WGS84_G1674>>);

	static_assert(!is_vector<PositionECEF<WGS84_G1674>>);
	static_assert(!is_vector<PositionENU<WGS84_G1674>>);
	static_assert(!is_vector<PositionNED<WGS84_G1674>>);
}

TEST_F(VectorTest, AERToECEFValidatedReference)
{
	// Convention used by library:
	//  - Azimuth measured from North, positive toward East (navigation convention)
	//  - Elevation from local horizontal, positive up
	//  - Right-handed ENU frame, then rotated to ECEF
	//
	// Origin: WGS84, lat = 34°, lon = -118°
	// az = 123°, el = 17°
	//
	// Validated reference (see derivation below):
	// vECEF ≈ [ +0.2923717051744461,
	//            +0.8020246528124292,
	//            -0.5208408994679886 ]

	auto vENU = VectorENU<WGS84_G1674>::fromAER(degrees<double>{123}, degrees<double>{17}, origin);

	VectorECEF<WGS84_G1674> vECEF = vENU;

	EXPECT_NEAR(vECEF.x().value(), 0.2923717051744461, 1e-12);
	EXPECT_NEAR(vECEF.y().value(), 0.8020246528124292, 1e-12);
	EXPECT_NEAR(vECEF.z().value(), -0.5208408994679886, 1e-12);
}

//======================================================
// Group A: Vector construction & accessors
//======================================================

TEST_F(VectorTest, ConstructENUAndAccessors)
{
	VecENU v{meters<double>{3}, meters<double>{-4}, meters<double>{12}, origin};

	EXPECT_NEAR(v.east().value(), 3.0, eps);
	EXPECT_NEAR(v.north().value(), -4.0, eps);
	EXPECT_NEAR(v.up().value(), 12.0, eps);
}

TEST_F(VectorTest, ConstructNEDAndAccessors)
{
	VectorNED<Datum, meters, double> v{meters<double>{3}, meters<double>{-4}, meters<double>{12}, origin};

	EXPECT_NEAR(v.north().value(), 3.0, eps);
	EXPECT_NEAR(v.east().value(), -4.0, eps);
	EXPECT_NEAR(v.down().value(), 12.0, eps);
}

TEST_F(VectorTest, ConstructECEFAndAccessors)
{
	VecECEF v{meters<double>{1}, meters<double>{2}, meters<double>{3}};

	EXPECT_NEAR(v.x().value(), 1.0, eps);
	EXPECT_NEAR(v.y().value(), 2.0, eps);
	EXPECT_NEAR(v.z().value(), 3.0, eps);
}

//======================================================
// Group B: Vector arithmetic (same frame)
//======================================================

TEST_F(VectorTest, ENUVectorArithmetic)
{
	VecENU a{meters<double>{1}, meters<double>{2}, meters<double>{3}, origin};
	VecENU b{meters<double>{-4}, meters<double>{5}, meters<double>{-6}, origin};

	VecENU c = a + b;

	EXPECT_NEAR(c.east().value(), -3.0, eps);
	EXPECT_NEAR(c.north().value(), 7.0, eps);
	EXPECT_NEAR(c.up().value(), -3.0, eps);
}

TEST_F(VectorTest, ENUVectorScalarMultiply)
{
	VecENU v{meters<double>{2}, meters<double>{-3}, meters<double>{4}, origin};

	VecENU s = v * 2;

	EXPECT_NEAR(s.east().value(), 4.0, eps);
	EXPECT_NEAR(s.north().value(), -6.0, eps);
	EXPECT_NEAR(s.up().value(), 8.0, eps);
}

//======================================================
// Group C: Vector ↔ Vector implicit conversions
// Truth data from MATLAB at lat=0, lon=0
//
// ENU basis:
//   East  -> (0, +1, 0)
//   North -> (0,  0, +1)
//   Up    -> (+1, 0, 0)
//======================================================

TEST_F(VectorTest, ENUToECEFAxisEast)
{
	VecENU enu{meters<double>{1}, meters<double>{0}, meters<double>{0}, origin};

	VecECEF ecef = enu;

	EXPECT_NEAR(ecef.x().value(), 0.0, eps);
	EXPECT_NEAR(ecef.y().value(), 1.0, eps);
	EXPECT_NEAR(ecef.z().value(), 0.0, eps);
}

TEST_F(VectorTest, ENUToECEFAxisUp)
{
	VecENU enu{meters<double>{0}, meters<double>{0}, meters<double>{1}, origin};

	VecECEF ecef = enu;

	EXPECT_NEAR(ecef.x().value(), 1.0, eps);
	EXPECT_NEAR(ecef.y().value(), 0.0, eps);
	EXPECT_NEAR(ecef.z().value(), 0.0, eps);
}

TEST_F(VectorTest, ENUToECEFNonTrivialVector)
{
	// MATLAB:
	// v_enu = [3; -4; 12];
	// v_ecef = [12; 3; -4]
	VecENU enu{meters<double>{3}, meters<double>{-4}, meters<double>{12}, origin};

	VecECEF ecef = enu;

	EXPECT_NEAR(ecef.x().value(), 12.0, eps);
	EXPECT_NEAR(ecef.y().value(), 3.0, eps);
	EXPECT_NEAR(ecef.z().value(), -4.0, eps);
}

TEST_F(VectorTest, ECEFToENUInverse)
{
	VecECEF ecef{meters<double>{12}, meters<double>{3}, meters<double>{-4}};

	VecENU enu = ecef;

	EXPECT_NEAR(enu.east().value(), 3.0, eps);
	EXPECT_NEAR(enu.north().value(), -4.0, eps);
	EXPECT_NEAR(enu.up().value(), 12.0, eps);
}

//======================================================
// Group D: Position ± Vector
//======================================================

TEST_F(VectorTest, PositionENUPlusVectorENU)
{
	PositionENU<Datum, meters, double> p{meters<double>{10}, meters<double>{20}, meters<double>{30}, origin};

	VecENU v{meters<double>{-5}, meters<double>{4}, meters<double>{-10}, origin};

	auto q = p + v;

	EXPECT_NEAR(q.east().value(), 5.0, eps);
	EXPECT_NEAR(q.north().value(), 24.0, eps);
	EXPECT_NEAR(q.up().value(), 20.0, eps);
}

TEST_F(VectorTest, PositionECEFPlusVectorENUImplicit)
{
	// Same math, but via implicit ENU→ECEF
	PositionENU<Datum, meters, double>  p_enu{meters<double>{10}, meters<double>{20}, meters<double>{30}, origin};
	PositionECEF<Datum, meters, double> p_ecef = p_enu;

	VecENU v_enu{meters<double>{-5}, meters<double>{4}, meters<double>{-10}, origin};

	auto q = p_ecef + v_enu;

	// Convert back to ENU for validation
	PositionENU<Datum, meters, double> q_enu = q;

	EXPECT_NEAR(q_enu.east().value(), 5.0, eps);
	EXPECT_NEAR(q_enu.north().value(), 24.0, eps);
	EXPECT_NEAR(q_enu.up().value(), 20.0, eps);
}

//======================================================
// Group E: Position − Position → Vector
//======================================================

TEST_F(VectorTest, PositionDifferenceENU)
{
	PositionENU<Datum, meters, double> p1{meters<double>{10}, meters<double>{20}, meters<double>{30}, origin};
	PositionENU<Datum, meters, double> p2{meters<double>{-5}, meters<double>{40}, meters<double>{-10}, origin};

	auto d = p2 - p1;

	EXPECT_NEAR(d.east().value(), -15.0, eps);
	EXPECT_NEAR(d.north().value(), 20.0, eps);
	EXPECT_NEAR(d.up().value(), -40.0, eps);
}

TEST_F(VectorTest, PositionDifferenceECEFImplicit)
{
	PositionENU<Datum, meters, double> p1{meters<double>{10}, meters<double>{20}, meters<double>{30}, origin};
	PositionENU<Datum, meters, double> p2{meters<double>{-5}, meters<double>{40}, meters<double>{-10}, origin};

	PositionECEF<Datum, meters, double> e1 = p1;
	PositionECEF<Datum, meters, double> e2 = p2;

	auto d = e2 - e1;

	// MATLAB: dENU = (-15,20,-40) → dECEF = (-40,-15,20)
	EXPECT_NEAR(d.x().value(), -40.0, eps);
	EXPECT_NEAR(d.y().value(), -15.0, eps);
	EXPECT_NEAR(d.z().value(), 20.0, eps);
}

//======================================================
// Group F: AER-derived vectors
//======================================================

TEST_F(VectorTest, AERToENUCardinal)
{
	auto v = VecENU::fromAER(degrees<double>{0}, degrees<double>{90}, origin);

	EXPECT_NEAR(v.east().value(), 0.0, eps);
	EXPECT_NEAR(v.north().value(), 0.0, eps);
	EXPECT_NEAR(v.up().value(), 1.0, eps);
}

TEST_F(VectorTest, AERToENUNonTrivial)
{
	// az=45°, el=30°
	// ENU = (cos30*cos45, cos30*sin45, sin30)
	const double c30 = std::sqrt(3.0) / 2.0;
	const double s30 = 0.5;
	const double c45 = std::sqrt(2.0) / 2.0;

	auto v = VecENU::fromAER(degrees<double>{45}, degrees<double>{30}, origin);

	EXPECT_NEAR(v.east().value(), c30 * c45, eps);
	EXPECT_NEAR(v.north().value(), c30 * c45, eps);
	EXPECT_NEAR(v.up().value(), s30, eps);
}

//======================================================
// Compile-time legality (positive only)
//======================================================

static_assert(std::is_constructible_v<VectorENU<WGS84_G1674, meters, double>, VectorECEF<WGS84_G1674, meters, double>>);

static_assert(std::is_constructible_v<PositionENU<WGS84_G1674, meters, double>, PositionECEF<WGS84_G1674, meters, double>>);
