//--------------------------------------------------------------------------------------------------
//
//	Vector Tests
//
//--------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>

#include <cmath>
#include <type_traits>

#include "../src/coordinates.h"

using namespace coordinates;

namespace
{
	template<typename U>
	double toDouble(const U& u)
	{
		return u.value();
	}

	template<typename U>
	void expectNearUnit(const U& a, const U& b, double tol, const char* msg)
	{
		EXPECT_NEAR(toDouble(a), toDouble(b), tol) << msg;
	}
}

TEST(VectorConceptTest, IsVectorConcept)
{
	static_assert(is_vector<VectorECEF<WGS84_G1674>>);
	static_assert(is_vector<VectorENU<WGS84_G1674>>);
	static_assert(is_vector<VectorNED<WGS84_G1674>>);

	static_assert(!is_vector<PositionECEF<WGS84_G1674>>);
	static_assert(!is_vector<PositionENU<WGS84_G1674>>);
	static_assert(!is_vector<PositionNED<WGS84_G1674>>);
}

TEST(VectorArithmeticTest, ECEFIdentityLaw)
{
	using D = WGS84_G1674;

	PositionECEF<D> p1(10.0_m, -20.0_m, 30.0_m);
	PositionECEF<D> p2(-7.0_m, 5.0_m, 99.0_m);

	const auto v = p2 - p1;
	const auto p3 = p1 + v;

	expectNearUnit(p3.x(), p2.x(), 1e-12, "x");
	expectNearUnit(p3.y(), p2.y(), 1e-12, "y");
	expectNearUnit(p3.z(), p2.z(), 1e-12, "z");
}

TEST(VectorArithmeticTest, ENUIdentityLaw)
{
	using D = WGS84_G1674;

	const PositionGeodetic<D> origin(34.0_deg, -118.0_deg, 1000.0_m);
	PositionENU<D> p1(10.0_m, 20.0_m, 30.0_m, origin, origin.date());
	PositionENU<D> p2(-7.0_m, 5.0_m, 99.0_m, origin, origin.date());

	const auto v = p2 - p1;
	const auto p3 = p1 + v;

	expectNearUnit(p3.east(), p2.east(), 1e-12, "east");
	expectNearUnit(p3.north(), p2.north(), 1e-12, "north");
	expectNearUnit(p3.up(), p2.up(), 1e-12, "up");
}

TEST(VectorArithmeticTest, NEDIdentityLaw)
{
	using D = WGS84_G1674;

	const PositionGeodetic<D> origin(34.0_deg, -118.0_deg, 1000.0_m);
	PositionNED<D> p1(10.0_m, 20.0_m, 30.0_m, origin, origin.date());
	PositionNED<D> p2(-7.0_m, 5.0_m, 99.0_m, origin, origin.date());

	const auto v = p2 - p1;
	const auto p3 = p1 + v;

	expectNearUnit(p3.north(), p2.north(), 1e-12, "north");
	expectNearUnit(p3.east(), p2.east(), 1e-12, "east");
	expectNearUnit(p3.down(), p2.down(), 1e-12, "down");
}

TEST(VectorArithmeticTest, ENUFrameMismatchThrowsOrAsserts)
{
	using D = WGS84_G1674;

	const PositionGeodetic<D> originA(0.0_deg, 0.0_deg, 0.0_m);
	const PositionGeodetic<D> originB(1.0_deg, 0.0_deg, 0.0_m);

	PositionENU<D> pA(0.0_m, 0.0_m, 0.0_m, originA, originA.date());
	PositionENU<D> pB(0.0_m, 0.0_m, 0.0_m, originB, originB.date());

#ifndef NDEBUG
	EXPECT_DEATH({ auto v = pA - pB; (void)v; }, "");
#else
	EXPECT_THROW({ auto v = pA - pB; (void)v; }, std::logic_error);
#endif
}

TEST(VectorConversionTest, AERManualECEFEquivalence)
{
	const LLA origin(34.0_deg, -118.0_deg, 0.0_m);

	degrees azDeg(123.0);
	degrees elDeg(17.0);

	// Vector-based pipeline
	const auto vENU = VecENU::fromAER(azDeg, elDeg, origin);
	const VectorECEF<WGS84_G1674> vECEF(vENU);

	// Manual reference (matches your original snippet)
	const radians<double> az(azDeg);
	const radians<double> el(elDeg);

	const double cEl = std::cos(el.value());
	const double sEl = std::sin(el.value());

	const meters<double> e(cEl * std::sin(az.value()));
	const meters<double> n(cEl * std::cos(az.value()));
	const meters<double> u(sEl);

	const radians<double> lat(origin.latitude());
	const radians<double> lon(origin.longitude());

	const double sLat = std::sin(lat.value());
	const double cLat = std::cos(lat.value());
	const double sLon = std::sin(lon.value());
	const double cLon = std::cos(lon.value());

	const meters<double> dx((-sLon) * e.value() + (-sLat * cLon) * n.value() + (cLat * cLon) * u.value());
	const meters<double> dy((cLon) * e.value() + (-sLat * sLon) * n.value() + (cLat * sLon) * u.value());
	const meters<double> dz((cLat) * n.value() + (sLat) * u.value());

	expectNearUnit(vECEF.x(), dx, 1e-9, "dx");
	expectNearUnit(vECEF.y(), dy, 1e-9, "dy");
	expectNearUnit(vECEF.z(), dz, 1e-9, "dz");
}

TEST(VectorCompileTimeTest, IllegalPositionAdditionDoesNotCompile)
{
	using D = WGS84_G1674;
	using P = PositionECEF<D>;

	static_assert(!requires(P a, P b) { a + b; });
	static_assert(!requires(P a, P b) { a - b + a; }); // still ok only if - returns vector and + is defined with vector
}
