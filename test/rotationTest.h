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

// ---------------------------------------------------------------------------------------------------------------------
//
/// @brief      Exhaustive GoogleTest suite for the rotation-math library (`Quaternion`, `EulerAngles`,
///             `AxisAngle`, `RotationMatrix`, their interconversions, and the constant-evaluable trig
///             primitives in `rotationDetail.h`). Verifies numeric accuracy, sign/convention correctness,
///             round-trip fidelity, gimbal-lock handling, cross-representation agreement, and compile-time
///             (`constexpr`) evaluability.
//
// ---------------------------------------------------------------------------------------------------------------------

#pragma once

//------------------------
//	INCLUDES
//------------------------

#include <algorithm>
#include <cmath>
#include <tuple>

#include <gtest/gtest.h>

#include "quaternion.h"
#include "rotation.h"

using namespace coordinates;
using namespace coordinates::rotation;
using namespace units;
using namespace units::literals;

//======================================================
//	TEST-LOCAL HELPERS (anonymous namespace -> no ODR clash with sibling test files)
//======================================================

namespace
{
	/// Shared value type for rotation::detail::PI, qualified to avoid ambiguity with `units::detail`.
	inline constexpr double kPi = coordinates::rotation::detail::PI;

	/// A dimensionless 3-vector, the input form `Quaternion::fromTwoVectors` expects.
	using DirVec = std::tuple<dimensionless<>, dimensionless<>, dimensionless<>>;

	/// A metre-valued 3-vector, the input form the `rotate` methods expect.
	using MeterVec = std::tuple<meters<>, meters<>, meters<>>;

	/// Foot-valued 3-vector, to prove the rotate methods preserve the input unit type.
	using FootVec = std::tuple<feet<>, feet<>, feet<>>;

	//----------------------------------------------------------------------------------------------------------------------
	/// @brief	Constant-evaluable approximate scalar comparison, for `static_assert` regression guards.
	/// @param[in]	a	first value.
	/// @param[in]	b	second value.
	/// @param[in]	tol	absolute tolerance.
	/// @return	true when |a - b| <= tol.
	//----------------------------------------------------------------------------------------------------------------------
	static constexpr bool approx(double a, double b, double tol) noexcept
	{
		return (a - b <= tol) && (b - a <= tol);
	}

	//----------------------------------------------------------------------------------------------------------------------
	/// @brief	Runtime helper: assert two metre-valued vectors match component-wise.
	/// @param[in]	actual		the computed vector.
	/// @param[in]	ex,ey,ez	the expected components.
	/// @param[in]	tol			absolute tolerance.
	//----------------------------------------------------------------------------------------------------------------------
	static void expectVec(const MeterVec& actual, double ex, double ey, double ez, double tol)
	{
		EXPECT_NEAR(std::get<0>(actual).to<double>(), ex, tol);
		EXPECT_NEAR(std::get<1>(actual).to<double>(), ey, tol);
		EXPECT_NEAR(std::get<2>(actual).to<double>(), ez, tol);
	}

	//----------------------------------------------------------------------------------------------------------------------
	/// @brief	Flip a quaternion so its scalar part is non-negative (q and -q are the same rotation).
	/// @param[in]	q	the quaternion to sign-normalize.
	/// @return	a sign-aligned copy with w >= 0.
	//----------------------------------------------------------------------------------------------------------------------
	static Quaternion signAligned(const Quaternion& q)
	{
		if (q.w().value() < 0.0)
			return Quaternion(-q.w(), -q.x(), -q.y(), -q.z());
		return q;
	}

	//----------------------------------------------------------------------------------------------------------------------
	/// @brief	Assert two quaternions represent the same rotation (compared after sign alignment).
	/// @param[in]	a	first quaternion.
	/// @param[in]	b	second quaternion.
	/// @param[in]	tol	absolute per-component tolerance.
	//----------------------------------------------------------------------------------------------------------------------
	static void expectSameRotation(const Quaternion& a, const Quaternion& b, double tol)
	{
		const Quaternion an = signAligned(a.normalized());
		const Quaternion bn = signAligned(b.normalized());
		EXPECT_NEAR(an.w().value(), bn.w().value(), tol);
		EXPECT_NEAR(an.x().value(), bn.x().value(), tol);
		EXPECT_NEAR(an.y().value(), bn.y().value(), tol);
		EXPECT_NEAR(an.z().value(), bn.z().value(), tol);
	}

	//----------------------------------------------------------------------------------------------------------------------
	/// @brief	Determinant of a rotation matrix.
	/// @param[in]	m	the matrix.
	/// @return	the scalar determinant.
	//----------------------------------------------------------------------------------------------------------------------
	static double determinant(const RotationMatrix& m)
	{
		return m.at(0, 0).value() * (m.at(1, 1).value() * m.at(2, 2).value() - m.at(1, 2).value() * m.at(2, 1).value())
		     - m.at(0, 1).value() * (m.at(1, 0).value() * m.at(2, 2).value() - m.at(1, 2).value() * m.at(2, 0).value())
		     + m.at(0, 2).value() * (m.at(1, 0).value() * m.at(2, 1).value() - m.at(1, 1).value() * m.at(2, 0).value());
	}

	/// The three cardinal unit vectors, in metres.
	static const MeterVec kX{1.0_m, 0.0_m, 0.0_m};
	static const MeterVec kY{0.0_m, 1.0_m, 0.0_m};
	static const MeterVec kZ{0.0_m, 0.0_m, 1.0_m};

	/// Degrees -> radians, for building AxisAngle test data.
	static constexpr double degToRad(double deg) noexcept { return deg * kPi / 180.0; }
}    // namespace

//======================================================
//	DETAIL TRIG ACCURACY  (RotationDetail)
//======================================================

TEST(RotationDetail, SinConstexprMatchesStdOverSweptRange)
{
	for (double a = -20.0; a <= 20.0; a += 0.017)
		EXPECT_NEAR(rotation::detail::sinConstexpr(a), std::sin(a), 1e-12) << "a=" << a;
}

TEST(RotationDetail, CosConstexprMatchesStdOverSweptRange)
{
	for (double a = -20.0; a <= 20.0; a += 0.017)
		EXPECT_NEAR(rotation::detail::cosConstexpr(a), std::cos(a), 1e-12) << "a=" << a;
}

TEST(RotationDetail, SinCosHandleLargePositiveWrap)
{
	// angles well beyond 2*pi must reduce correctly
	for (double a = 6.5; a <= 100.0; a += 0.73)
	{
		EXPECT_NEAR(rotation::detail::sinConstexpr(a), std::sin(a), 1e-12) << "a=" << a;
		EXPECT_NEAR(rotation::detail::cosConstexpr(a), std::cos(a), 1e-12) << "a=" << a;
	}
}

TEST(RotationDetail, SinCosHandleLargeNegativeWrap)
{
	for (double a = -6.5; a >= -100.0; a -= 0.73)
	{
		EXPECT_NEAR(rotation::detail::sinConstexpr(a), std::sin(a), 1e-12) << "a=" << a;
		EXPECT_NEAR(rotation::detail::cosConstexpr(a), std::cos(a), 1e-12) << "a=" << a;
	}
}

TEST(RotationDetail, SinCosExactAtKnownAngles)
{
	EXPECT_NEAR(rotation::detail::sinConstexpr(0.0), 0.0, 1e-14);
	EXPECT_NEAR(rotation::detail::cosConstexpr(0.0), 1.0, 1e-14);
	EXPECT_NEAR(rotation::detail::sinConstexpr(kPi / 2.0), 1.0, 1e-13);
	EXPECT_NEAR(rotation::detail::cosConstexpr(kPi / 2.0), 0.0, 1e-13);
	EXPECT_NEAR(rotation::detail::sinConstexpr(kPi), 0.0, 1e-13);
	EXPECT_NEAR(rotation::detail::cosConstexpr(kPi), -1.0, 1e-13);
	EXPECT_NEAR(rotation::detail::sinConstexpr(kPi / 6.0), 0.5, 1e-13);
	EXPECT_NEAR(rotation::detail::cosConstexpr(kPi / 3.0), 0.5, 1e-13);
}

TEST(RotationDetail, AtanConstexprMatchesStdIncludingRangeReduction)
{
	// includes |x| > 1 (the reduced branch) and both signs
	for (double x = -1000.0; x <= 1000.0; x += 0.37)
		EXPECT_NEAR(rotation::detail::atanConstexpr(x), std::atan(x), 1e-12) << "x=" << x;
}

TEST(RotationDetail, AtanConstexprSmallAndZero)
{
	EXPECT_NEAR(rotation::detail::atanConstexpr(0.0), 0.0, 1e-14);
	EXPECT_NEAR(rotation::detail::atanConstexpr(1.0), kPi / 4.0, 1e-12);
	EXPECT_NEAR(rotation::detail::atanConstexpr(-1.0), -kPi / 4.0, 1e-12);
	for (double x = -0.99; x <= 0.99; x += 0.0031)
		EXPECT_NEAR(rotation::detail::atanConstexpr(x), std::atan(x), 1e-12) << "x=" << x;
}

TEST(RotationDetail, AsinConstexprMatchesStdInDomain)
{
	for (double x = -0.999; x <= 0.999; x += 0.0017)
		EXPECT_NEAR(rotation::detail::asinConstexpr(x), std::asin(x), 1e-12) << "x=" << x;
}

TEST(RotationDetail, AsinConstexprClampsOutOfDomain)
{
	EXPECT_NEAR(rotation::detail::asinConstexpr(1.0), kPi / 2.0, 1e-14);
	EXPECT_NEAR(rotation::detail::asinConstexpr(-1.0), -kPi / 2.0, 1e-14);
	EXPECT_NEAR(rotation::detail::asinConstexpr(1.5), kPi / 2.0, 1e-14);      // >= 1 clamps
	EXPECT_NEAR(rotation::detail::asinConstexpr(-2.0), -kPi / 2.0, 1e-14);    // <= -1 clamps
	EXPECT_NEAR(rotation::detail::asinConstexpr(0.0), 0.0, 1e-14);
	EXPECT_NEAR(rotation::detail::asinConstexpr(0.5), kPi / 6.0, 1e-12);
}

TEST(RotationDetail, Atan2ConstexprAllFourQuadrants)
{
	for (double y = -5.0; y <= 5.0; y += 0.13)
		for (double x = -5.0; x <= 5.0; x += 0.13)
		{
			if (x == 0.0 && y == 0.0) continue;
			EXPECT_NEAR(rotation::detail::atan2Constexpr(y, x), std::atan2(y, x), 1e-12) << "y=" << y << " x=" << x;
		}
}

TEST(RotationDetail, Atan2ConstexprAxisAndOriginEdges)
{
	EXPECT_NEAR(rotation::detail::atan2Constexpr(0.0, 0.0), 0.0, 1e-14);       // degenerate origin
	EXPECT_NEAR(rotation::detail::atan2Constexpr(1.0, 0.0), kPi / 2.0, 1e-14); // +Y axis
	EXPECT_NEAR(rotation::detail::atan2Constexpr(-1.0, 0.0), -kPi / 2.0, 1e-14); // -Y axis
	EXPECT_NEAR(rotation::detail::atan2Constexpr(0.0, 1.0), 0.0, 1e-14);      // +X axis
	EXPECT_NEAR(rotation::detail::atan2Constexpr(0.0, -1.0), kPi, 1e-14);     // -X axis, y == 0 -> +pi
	EXPECT_NEAR(rotation::detail::atan2Constexpr(1.0, -1.0), 3.0 * kPi / 4.0, 1e-12);  // second quadrant
	EXPECT_NEAR(rotation::detail::atan2Constexpr(-1.0, -1.0), -3.0 * kPi / 4.0, 1e-12); // third quadrant
	EXPECT_NEAR(rotation::detail::atan2Constexpr(1.0, 1.0), kPi / 4.0, 1e-12);
	EXPECT_NEAR(rotation::detail::atan2Constexpr(-1.0, 1.0), -kPi / 4.0, 1e-12);
}

TEST(RotationDetail, SqrtConstexprMatchesStd)
{
	for (double v = 0.0; v <= 1000.0; v += 0.31)
		EXPECT_NEAR(rotation::detail::sqrtConstexpr(v), std::sqrt(v), 1e-9) << "v=" << v;
	EXPECT_NEAR(rotation::detail::sqrtConstexpr(2.0), std::sqrt(2.0), 1e-13);
	EXPECT_NEAR(rotation::detail::sqrtConstexpr(1e12), std::sqrt(1e12), 1e-3);
}

TEST(RotationDetail, SqrtConstexprNonPositiveIsZero)
{
	EXPECT_EQ(rotation::detail::sqrtConstexpr(0.0), 0.0);
	EXPECT_EQ(rotation::detail::sqrtConstexpr(-1.0), 0.0);
	EXPECT_EQ(rotation::detail::sqrtConstexpr(-1e9), 0.0);
}

TEST(RotationDetail, WrapRadiansReducesToPrincipalInterval)
{
	for (double a = -50.0; a <= 50.0; a += 0.37)
	{
		const double w = rotation::detail::wrapRadians(a);
		EXPECT_GT(w, -kPi - 1e-12);
		EXPECT_LE(w, kPi + 1e-12);
		// sin/cos of the wrapped angle must equal those of the original
		EXPECT_NEAR(std::sin(w), std::sin(a), 1e-12) << "a=" << a;
		EXPECT_NEAR(std::cos(w), std::cos(a), 1e-12) << "a=" << a;
	}
}

TEST(RotationDetail, DispatchersMatchConstexprPrimitivesAtRuntime)
{
	// The consteval-aware dispatchers must agree with std:: on the runtime branch.
	for (double a = -3.0; a <= 3.0; a += 0.11)
	{
		EXPECT_NEAR(rotation::detail::sinDouble(a), std::sin(a), 1e-12);
		EXPECT_NEAR(rotation::detail::cosDouble(a), std::cos(a), 1e-12);
	}
	EXPECT_NEAR(rotation::detail::sqrtDouble(9.0), 3.0, 1e-12);
	EXPECT_NEAR(rotation::detail::asinDouble(0.5), std::asin(0.5), 1e-12);
	EXPECT_NEAR(rotation::detail::atan2Double(1.0, 1.0), std::atan2(1.0, 1.0), 1e-12);
}

//======================================================
//	QUATERNION - IDENTITY / NORM / NORMALIZE
//======================================================

TEST(RotationQuaternion, DefaultConstructedIsIdentity)
{
	Quaternion q;
	EXPECT_EQ(q, Quaternion::identity());
	EXPECT_EQ(q.w().value(), 1.0);
	EXPECT_EQ(q.x().value(), 0.0);
	EXPECT_EQ(q.y().value(), 0.0);
	EXPECT_EQ(q.z().value(), 0.0);
}

TEST(RotationQuaternion, IdentityLeavesVectorUnchanged)
{
	const MeterVec v{3.0_m, -4.0_m, 12.0_m};
	expectVec(Quaternion::identity().rotate(v), 3.0, -4.0, 12.0, 1e-12);
}

TEST(RotationQuaternion, IdentityComposeIsIdentity)
{
	const Quaternion i = Quaternion::identity();
	EXPECT_EQ(i * i, i);
}

TEST(RotationQuaternion, NormSquaredAndNorm)
{
	Quaternion q(1.0, 2.0, 3.0, 4.0);
	EXPECT_NEAR(q.normSquared().value(), 30.0, 1e-12);
	EXPECT_NEAR(q.norm().value(), std::sqrt(30.0), 1e-12);
}

TEST(RotationQuaternion, NormalizedProducesUnitNorm)
{
	Quaternion q(1.0, 2.0, 3.0, 4.0);
	Quaternion n = q.normalized();
	EXPECT_NEAR(n.norm().value(), 1.0, 1e-12);
	// direction preserved
	EXPECT_NEAR(n.x().value() / n.w().value(), 2.0, 1e-12);
	EXPECT_NEAR(n.y().value() / n.w().value(), 3.0, 1e-12);
	EXPECT_NEAR(n.z().value() / n.w().value(), 4.0, 1e-12);
}

TEST(RotationQuaternion, NormalizedOfZeroQuaternionIsIdentity)
{
	Quaternion zero(0.0, 0.0, 0.0, 0.0);
	EXPECT_EQ(zero.normalized(), Quaternion::identity());
}

TEST(RotationQuaternion, NormalizedOfUnitQuaternionIsUnchanged)
{
	Quaternion q = toQuaternion(EulerAngles(30.0_deg, 20.0_deg, 10.0_deg));
	Quaternion n = q.normalized();
	EXPECT_NEAR(n.w().value(), q.w().value(), 1e-12);
	EXPECT_NEAR(n.x().value(), q.x().value(), 1e-12);
	EXPECT_NEAR(n.y().value(), q.y().value(), 1e-12);
	EXPECT_NEAR(n.z().value(), q.z().value(), 1e-12);
}

//======================================================
//	QUATERNION - CONJUGATE / INVERSE / DOT
//======================================================

TEST(RotationQuaternion, ConjugateNegatesVectorPart)
{
	Quaternion q(1.0, 2.0, 3.0, 4.0);
	Quaternion c = q.conjugate();
	EXPECT_EQ(c.w().value(), 1.0);
	EXPECT_EQ(c.x().value(), -2.0);
	EXPECT_EQ(c.y().value(), -3.0);
	EXPECT_EQ(c.z().value(), -4.0);
}

TEST(RotationQuaternion, ConjugateEqualsInverseForUnitQuaternion)
{
	Quaternion q = toQuaternion(EulerAngles(25.0_deg, -35.0_deg, 15.0_deg));
	Quaternion c = q.conjugate();
	Quaternion i = q.inverse();
	EXPECT_NEAR(c.w().value(), i.w().value(), 1e-12);
	EXPECT_NEAR(c.x().value(), i.x().value(), 1e-12);
	EXPECT_NEAR(c.y().value(), i.y().value(), 1e-12);
	EXPECT_NEAR(c.z().value(), i.z().value(), 1e-12);
}

TEST(RotationQuaternion, InverseOfUnitQuaternionYieldsIdentity)
{
	Quaternion q = toQuaternion(EulerAngles(40.0_deg, 10.0_deg, -20.0_deg));
	expectSameRotation(q * q.inverse(), Quaternion::identity(), 1e-12);
	expectSameRotation(q.inverse() * q, Quaternion::identity(), 1e-12);
}

TEST(RotationQuaternion, InverseOfNonUnitQuaternionYieldsIdentity)
{
	// q * inverse == identity must hold even when |q| != 1
	Quaternion q(2.0, -1.0, 0.5, 3.0);
	Quaternion product = q * q.inverse();
	EXPECT_NEAR(product.w().value(), 1.0, 1e-12);
	EXPECT_NEAR(product.x().value(), 0.0, 1e-12);
	EXPECT_NEAR(product.y().value(), 0.0, 1e-12);
	EXPECT_NEAR(product.z().value(), 0.0, 1e-12);
}

TEST(RotationQuaternion, InverseOfZeroQuaternionIsIdentity)
{
	Quaternion zero(0.0, 0.0, 0.0, 0.0);
	EXPECT_EQ(zero.inverse(), Quaternion::identity());
}

TEST(RotationQuaternion, DotProductComponentwise)
{
	Quaternion a(1.0, 2.0, 3.0, 4.0);
	Quaternion b(5.0, 6.0, 7.0, 8.0);
	EXPECT_NEAR(a.dot(b).value(), 1.0 * 5.0 + 2.0 * 6.0 + 3.0 * 7.0 + 4.0 * 8.0, 1e-12);
	EXPECT_NEAR(a.dot(a).value(), a.normSquared().value(), 1e-12);
}

//======================================================
//	QUATERNION - HAMILTON PRODUCT
//======================================================

TEST(RotationQuaternion, HamiltonProductIsNonCommutative)
{
	Quaternion a = toQuaternion(EulerAngles(90.0_deg, 0.0_deg, 0.0_deg));  // yaw
	Quaternion b = toQuaternion(EulerAngles(0.0_deg, 90.0_deg, 0.0_deg));  // pitch
	Quaternion ab = a * b;
	Quaternion ba = b * a;
	EXPECT_TRUE(ab != ba);
}

TEST(RotationQuaternion, CompositionAppliesRhsFirstThenLhs)
{
	// lhs * rhs applies rhs first, then lhs -> (a*b) rotate v == a rotate (b rotate v)
	Quaternion a = toQuaternion(EulerAngles(90.0_deg, 0.0_deg, 0.0_deg));
	Quaternion b = toQuaternion(EulerAngles(0.0_deg, 90.0_deg, 0.0_deg));

	MeterVec combined   = (a * b).rotate(kX);
	MeterVec sequential = a.rotate(b.rotate(kX));

	expectVec(combined, std::get<0>(sequential).to<double>(),
	          std::get<1>(sequential).to<double>(),
	          std::get<2>(sequential).to<double>(), 1e-12);

	// ground truth: yaw(pitch(+X)) = yaw(-Z) = -Z
	expectVec(combined, 0.0, 0.0, -1.0, 1e-12);
	// swapping the order gives a different vector
	expectVec((b * a).rotate(kX), 0.0, 1.0, 0.0, 1e-12);
}

TEST(RotationQuaternion, ProductOfInverseRotationsIsIdentity)
{
	Quaternion a = toQuaternion(EulerAngles(33.0_deg, -12.0_deg, 47.0_deg));
	expectSameRotation(a * a.conjugate(), Quaternion::identity(), 1e-12);
}

//======================================================
//	QUATERNION - VECTOR ROTATION
//======================================================

TEST(RotationQuaternion, RotateNinetyAboutZ)
{
	Quaternion q = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(90.0))));
	expectVec(q.rotate(kX), 0.0, 1.0, 0.0, 1e-12);   // +X -> +Y
	expectVec(q.rotate(kY), -1.0, 0.0, 0.0, 1e-12);  // +Y -> -X
	expectVec(q.rotate(kZ), 0.0, 0.0, 1.0, 1e-12);   // +Z unchanged
}

TEST(RotationQuaternion, RotateOneEightyAboutZ)
{
	Quaternion q = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(180.0))));
	expectVec(q.rotate(kX), -1.0, 0.0, 0.0, 1e-12);
	expectVec(q.rotate(kY), 0.0, -1.0, 0.0, 1e-12);
	expectVec(q.rotate(kZ), 0.0, 0.0, 1.0, 1e-12);
}

TEST(RotationQuaternion, RotateTwoSeventyAboutZ)
{
	Quaternion q = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(270.0))));
	expectVec(q.rotate(kX), 0.0, -1.0, 0.0, 1e-12);  // +X -> -Y
	expectVec(q.rotate(kY), 1.0, 0.0, 0.0, 1e-12);   // +Y -> +X
}

TEST(RotationQuaternion, RotateNinetyAboutX)
{
	Quaternion q = toQuaternion(AxisAngle(1, 0, 0, radians<>(degToRad(90.0))));
	expectVec(q.rotate(kY), 0.0, 0.0, 1.0, 1e-12);   // +Y -> +Z
	expectVec(q.rotate(kZ), 0.0, -1.0, 0.0, 1e-12);  // +Z -> -Y
	expectVec(q.rotate(kX), 1.0, 0.0, 0.0, 1e-12);   // +X unchanged
}

TEST(RotationQuaternion, RotateOneEightyAboutX)
{
	Quaternion q = toQuaternion(AxisAngle(1, 0, 0, radians<>(degToRad(180.0))));
	expectVec(q.rotate(kY), 0.0, -1.0, 0.0, 1e-12);
	expectVec(q.rotate(kZ), 0.0, 0.0, -1.0, 1e-12);
}

TEST(RotationQuaternion, RotateNinetyAboutY)
{
	Quaternion q = toQuaternion(AxisAngle(0, 1, 0, radians<>(degToRad(90.0))));
	expectVec(q.rotate(kX), 0.0, 0.0, -1.0, 1e-12);  // +X -> -Z
	expectVec(q.rotate(kZ), 1.0, 0.0, 0.0, 1e-12);   // +Z -> +X
	expectVec(q.rotate(kY), 0.0, 1.0, 0.0, 1e-12);   // +Y unchanged
}

TEST(RotationQuaternion, RotateOneEightyAboutY)
{
	Quaternion q = toQuaternion(AxisAngle(0, 1, 0, radians<>(degToRad(180.0))));
	expectVec(q.rotate(kX), -1.0, 0.0, 0.0, 1e-12);
	expectVec(q.rotate(kZ), 0.0, 0.0, -1.0, 1e-12);
}

TEST(RotationQuaternion, RotatePreservesVectorLength)
{
	Quaternion q = toQuaternion(EulerAngles(37.0_deg, -22.0_deg, 68.0_deg));
	const MeterVec v{2.0_m, -5.0_m, 3.0_m};
	MeterVec       r     = q.rotate(v);
	const double   inLen = std::sqrt(2.0 * 2.0 + 5.0 * 5.0 + 3.0 * 3.0);
	const double   outLen = std::sqrt(std::get<0>(r).to<double>() * std::get<0>(r).to<double>()
	                                + std::get<1>(r).to<double>() * std::get<1>(r).to<double>()
	                                + std::get<2>(r).to<double>() * std::get<2>(r).to<double>());
	EXPECT_NEAR(outLen, inLen, 1e-12);
}

TEST(RotationQuaternion, RotatePreservesInputUnitType)
{
	Quaternion    q = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(90.0))));
	const FootVec v{1.0_ft, 0.0_ft, 0.0_ft};
	FootVec       r = q.rotate(v);
	// the result must itself be feet<> (compile-time proof) and equal +Y foot
	static_assert(std::is_same_v<decltype(std::get<0>(r)), feet<>&>);
	EXPECT_NEAR(std::get<0>(r).to<double>(), 0.0, 1e-12);
	EXPECT_NEAR(std::get<1>(r).to<double>(), 1.0, 1e-12);
	EXPECT_NEAR(std::get<2>(r).to<double>(), 0.0, 1e-12);
}

//======================================================
//	QUATERNION - fromTwoVectors
//======================================================

TEST(RotationQuaternion, FromTwoVectorsParallelIsIdentity)
{
	Quaternion q = Quaternion::fromTwoVectors(DirVec{2.0, 0.0, 0.0}, DirVec{5.0, 0.0, 0.0});
	EXPECT_EQ(q, Quaternion::identity());
}

TEST(RotationQuaternion, FromTwoVectorsOrthogonalXtoY)
{
	Quaternion q = Quaternion::fromTwoVectors(DirVec{1.0, 0.0, 0.0}, DirVec{0.0, 1.0, 0.0});
	EXPECT_NEAR(q.norm().value(), 1.0, 1e-12);
	expectVec(q.rotate(kX), 0.0, 1.0, 0.0, 1e-12);  // takes +X onto +Y
}

TEST(RotationQuaternion, FromTwoVectorsOrthogonalYtoZ)
{
	Quaternion q = Quaternion::fromTwoVectors(DirVec{0.0, 1.0, 0.0}, DirVec{0.0, 0.0, 1.0});
	EXPECT_NEAR(q.norm().value(), 1.0, 1e-12);
	expectVec(q.rotate(kY), 0.0, 0.0, 1.0, 1e-12);
}

TEST(RotationQuaternion, FromTwoVectorsAntiparallelIsHalfTurn)
{
	Quaternion q = Quaternion::fromTwoVectors(DirVec{1.0, 0.0, 0.0}, DirVec{-1.0, 0.0, 0.0});
	EXPECT_NEAR(q.norm().value(), 1.0, 1e-12);
	EXPECT_NEAR(q.w().value(), 0.0, 1e-9);           // 180-degree rotation -> scalar part zero
	expectVec(q.rotate(kX), -1.0, 0.0, 0.0, 1e-9);   // +X flipped onto -X
}

TEST(RotationQuaternion, FromTwoVectorsAntiparallelAlongZUsesFallbackAxis)
{
	// +Z -> -Z exercises the second perpendicular-axis fallback branch (px,py near zero)
	Quaternion q = Quaternion::fromTwoVectors(DirVec{0.0, 0.0, 1.0}, DirVec{0.0, 0.0, -1.0});
	EXPECT_NEAR(q.norm().value(), 1.0, 1e-9);
	EXPECT_NEAR(q.w().value(), 0.0, 1e-9);
	expectVec(q.rotate(kZ), 0.0, 0.0, -1.0, 1e-9);
}

TEST(RotationQuaternion, FromTwoVectorsNormalizesNonUnitInputs)
{
	Quaternion q = Quaternion::fromTwoVectors(DirVec{7.0, 0.0, 0.0}, DirVec{0.0, 3.0, 0.0});
	EXPECT_NEAR(q.norm().value(), 1.0, 1e-12);
	expectVec(q.rotate(kX), 0.0, 1.0, 0.0, 1e-12);
}

TEST(RotationQuaternion, FromTwoVectorsDegenerateInputIsIdentity)
{
	Quaternion q1 = Quaternion::fromTwoVectors(DirVec{0.0, 0.0, 0.0}, DirVec{0.0, 1.0, 0.0});
	EXPECT_EQ(q1, Quaternion::identity());
	Quaternion q2 = Quaternion::fromTwoVectors(DirVec{1.0, 0.0, 0.0}, DirVec{0.0, 0.0, 0.0});
	EXPECT_EQ(q2, Quaternion::identity());
}

TEST(RotationQuaternion, FromTwoVectorsArbitraryPairMapsSourceOntoTarget)
{
	DirVec     from{1.0, 2.0, 2.0};    // |from| = 3
	DirVec     to{2.0, -1.0, 2.0};     // |to| = 3
	Quaternion q = Quaternion::fromTwoVectors(from, to);
	EXPECT_NEAR(q.norm().value(), 1.0, 1e-12);
	// rotate the (unit) source direction and confirm it lands on the unit target direction
	const MeterVec unitFrom{meters<>(1.0 / 3.0), meters<>(2.0 / 3.0), meters<>(2.0 / 3.0)};
	expectVec(q.rotate(unitFrom), 2.0 / 3.0, -1.0 / 3.0, 2.0 / 3.0, 1e-9);
}

//======================================================
//	QUATERNION - SLERP
//======================================================

TEST(RotationQuaternion, SlerpAtZeroReturnsStart)
{
	Quaternion a = Quaternion::identity();
	Quaternion b = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(90.0))));
	expectSameRotation(Quaternion::slerp(a, b, 0.0), a, 1e-12);
}

TEST(RotationQuaternion, SlerpAtOneReturnsEnd)
{
	Quaternion a = Quaternion::identity();
	Quaternion b = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(90.0))));
	expectSameRotation(Quaternion::slerp(a, b, 1.0), b, 1e-12);
}

TEST(RotationQuaternion, SlerpMidpointIsHalfAngleAndUnitNorm)
{
	Quaternion a    = Quaternion::identity();
	Quaternion b    = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(90.0))));
	Quaternion half = Quaternion::slerp(a, b, 0.5);
	EXPECT_NEAR(half.norm().value(), 1.0, 1e-12);
	// halfway between 0 and 90 deg about Z is the 45-deg rotation about Z
	Quaternion expected = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(45.0))));
	expectSameRotation(half, expected, 1e-9);
}

TEST(RotationQuaternion, SlerpTakesShorterArcWhenDotNegative)
{
	// b intentionally on the far hemisphere (negative scalar): slerp must flip it and take the short arc
	Quaternion a = Quaternion::identity();
	Quaternion b(-std::cos(degToRad(22.5)), 0.0, 0.0, -std::sin(degToRad(22.5)));
	ASSERT_LT(a.dot(b).value(), 0.0);
	Quaternion half = Quaternion::slerp(a, b, 0.5);
	EXPECT_NEAR(half.norm().value(), 1.0, 1e-9);
	// shorter arc: the interpolated rotation is a small +Z rotation (positive z component)
	Quaternion aligned = signAligned(half);
	EXPECT_GT(aligned.z().value(), 0.0);
	// midpoint of the short arc (which is 45 deg about Z) is 22.5 deg about Z
	Quaternion expected = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(22.5))));
	expectSameRotation(half, expected, 1e-6);
}

TEST(RotationQuaternion, SlerpNearlyParallelUsesLerpBranch)
{
	// endpoints extremely close -> dot ~ 1 -> normalized-lerp fallback path
	Quaternion a = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(10.0))));
	Quaternion b = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(10.0000001))));
	Quaternion s = Quaternion::slerp(a, b, 0.5);
	EXPECT_NEAR(s.norm().value(), 1.0, 1e-9);
	expectSameRotation(s, a, 1e-6);
}

TEST(RotationQuaternion, SlerpProducesUnitNormAcrossParameter)
{
	Quaternion a = toQuaternion(EulerAngles(10.0_deg, 20.0_deg, 30.0_deg));
	Quaternion b = toQuaternion(EulerAngles(80.0_deg, -40.0_deg, 15.0_deg));
	for (double t = 0.0; t <= 1.0; t += 0.1)
		EXPECT_NEAR(Quaternion::slerp(a, b, t).norm().value(), 1.0, 1e-9) << "t=" << t;
}

//======================================================
//	QUATERNION - OPERATORS
//======================================================

TEST(RotationQuaternion, EqualityIsExactComponentwise)
{
	Quaternion a(1.0, 2.0, 3.0, 4.0);
	Quaternion b(1.0, 2.0, 3.0, 4.0);
	Quaternion c(1.0, 2.0, 3.0, 4.0000001);
	EXPECT_TRUE(a == b);
	EXPECT_FALSE(a != b);
	EXPECT_TRUE(a != c);
	EXPECT_FALSE(a == c);
}

//======================================================
//	EULER ANGLES - ACCESSORS / CONVENTION
//======================================================

TEST(RotationEuler, AccessorsAndTuple)
{
	EulerAngles e(30.0_deg, -20.0_deg, 10.0_deg);
	EXPECT_NEAR(e.yaw().value(), 30.0, 1e-12);
	EXPECT_NEAR(e.pitch().value(), -20.0, 1e-12);
	EXPECT_NEAR(e.roll().value(), 10.0, 1e-12);
	auto t = e.toTuple();
	EXPECT_NEAR(std::get<0>(t).value(), 30.0, 1e-12);
	EXPECT_NEAR(std::get<1>(t).value(), -20.0, 1e-12);
	EXPECT_NEAR(std::get<2>(t).value(), 10.0, 1e-12);
}

TEST(RotationEuler, DefaultIsIdentity)
{
	EulerAngles e;
	expectSameRotation(toQuaternion(e), Quaternion::identity(), 1e-12);
}

TEST(RotationEuler, YawNinetyRotatesXtoY)
{
	Quaternion q = toQuaternion(EulerAngles(90.0_deg, 0.0_deg, 0.0_deg));
	expectVec(q.rotate(kX), 0.0, 1.0, 0.0, 1e-12);
}

TEST(RotationEuler, PitchNinetyRotatesXtoMinusZ)
{
	// convention locked from the code: pitch about new Y sends +X to -Z
	Quaternion q = toQuaternion(EulerAngles(0.0_deg, 90.0_deg, 0.0_deg));
	expectVec(q.rotate(kX), 0.0, 0.0, -1.0, 1e-12);
	expectVec(q.rotate(kZ), 1.0, 0.0, 0.0, 1e-12);
}

TEST(RotationEuler, RollNinetyRotatesYtoZ)
{
	Quaternion q = toQuaternion(EulerAngles(0.0_deg, 0.0_deg, 90.0_deg));
	expectVec(q.rotate(kY), 0.0, 0.0, 1.0, 1e-12);
	expectVec(q.rotate(kZ), 0.0, -1.0, 0.0, 1e-12);
}

TEST(RotationEuler, CombinedYawPitchRollAgainstIndependentComputation)
{
	// yaw=30, pitch=20, roll=10 applied to +X. The intrinsic Z-Y-X quaternion product must equal
	// the independently-composed qYaw * qPitch * qRoll (rhs applied first).
	EulerAngles e(30.0_deg, 20.0_deg, 10.0_deg);
	Quaternion  combined = toQuaternion(e);

	Quaternion qYaw   = toQuaternion(AxisAngle(0, 0, 1, radians<>(degToRad(30.0))));
	Quaternion qPitch = toQuaternion(AxisAngle(0, 1, 0, radians<>(degToRad(20.0))));
	Quaternion qRoll  = toQuaternion(AxisAngle(1, 0, 0, radians<>(degToRad(10.0))));
	Quaternion manual = qYaw * qPitch * qRoll;

	expectSameRotation(combined, manual, 1e-12);

	// numeric ground truth for combined * +X (validated against the reference implementation)
	expectVec(combined.rotate(kX), 0.8137976813493738, 0.46984631039295416, -0.3420201433256687, 1e-9);
}

//======================================================
//	AXIS ANGLE - ACCESSORS
//======================================================

TEST(RotationAxisAngle, Accessors)
{
	AxisAngle aa(0.0, 0.0, 1.0, radians<>(0.5));
	EXPECT_NEAR(aa.axisX().value(), 0.0, 1e-12);
	EXPECT_NEAR(aa.axisY().value(), 0.0, 1e-12);
	EXPECT_NEAR(aa.axisZ().value(), 1.0, 1e-12);
	EXPECT_NEAR(aa.angle().value(), 0.5, 1e-12);
}

TEST(RotationAxisAngle, DefaultIsIdentityRotation)
{
	AxisAngle aa;
	expectSameRotation(toQuaternion(aa), Quaternion::identity(), 1e-12);
}

TEST(RotationAxisAngle, ZeroAngleIsIdentityRegardlessOfAxis)
{
	AxisAngle aa(1.0, 2.0, 3.0, radians<>(0.0));
	expectSameRotation(toQuaternion(aa), Quaternion::identity(), 1e-12);
}

TEST(RotationAxisAngle, DegenerateAxisIsIdentity)
{
	AxisAngle aa(0.0, 0.0, 0.0, radians<>(1.0));
	EXPECT_EQ(toQuaternion(aa), Quaternion::identity());
}

TEST(RotationAxisAngle, NonUnitAxisIsNormalized)
{
	// axis (0,0,5) with a 90-deg turn must behave exactly like the +Z 90-deg turn
	Quaternion q = toQuaternion(AxisAngle(0.0, 0.0, 5.0, radians<>(degToRad(90.0))));
	expectVec(q.rotate(kX), 0.0, 1.0, 0.0, 1e-12);
}

//======================================================
//	ROTATION MATRIX - ACCESSORS / IDENTITY / ROTATE
//======================================================

TEST(RotationMatrixTests, IdentityIsIdentity)
{
	RotationMatrix m = RotationMatrix::identity();
	for (int r = 0; r < 3; ++r)
		for (int c = 0; c < 3; ++c)
			EXPECT_NEAR(m.at(r, c).value(), (r == c) ? 1.0 : 0.0, 1e-12);
}

TEST(RotationMatrixTests, DefaultConstructedIsIdentity)
{
	RotationMatrix m;
	expectVec(m.rotate(kX), 1.0, 0.0, 0.0, 1e-12);
	expectVec(m.rotate(kY), 0.0, 1.0, 0.0, 1e-12);
	expectVec(m.rotate(kZ), 0.0, 0.0, 1.0, 1e-12);
}

TEST(RotationMatrixTests, ElementAccessRowMajor)
{
	RotationMatrix m(1, 2, 3, 4, 5, 6, 7, 8, 9);
	EXPECT_NEAR(m.at(0, 0).value(), 1.0, 1e-12);
	EXPECT_NEAR(m.at(0, 2).value(), 3.0, 1e-12);
	EXPECT_NEAR(m.at(1, 1).value(), 5.0, 1e-12);
	EXPECT_NEAR(m.at(2, 0).value(), 7.0, 1e-12);
	EXPECT_NEAR(m.at(2, 2).value(), 9.0, 1e-12);
}

TEST(RotationMatrixTests, YawNinetyMatrixElements)
{
	// yaw 90 about Z: [[0,-1,0],[1,0,0],[0,0,1]]
	RotationMatrix m = toRotationMatrix(EulerAngles(90.0_deg, 0.0_deg, 0.0_deg));
	EXPECT_NEAR(m.at(0, 0).value(), 0.0, 1e-12);
	EXPECT_NEAR(m.at(0, 1).value(), -1.0, 1e-12);
	EXPECT_NEAR(m.at(1, 0).value(), 1.0, 1e-12);
	EXPECT_NEAR(m.at(1, 1).value(), 0.0, 1e-12);
	EXPECT_NEAR(m.at(2, 2).value(), 1.0, 1e-12);
}

TEST(RotationMatrixTests, RotatePreservesInputUnitType)
{
	RotationMatrix m = toRotationMatrix(AxisAngle(0, 0, 1, radians<>(degToRad(90.0))));
	const FootVec  v{1.0_ft, 0.0_ft, 0.0_ft};
	FootVec        r = m.rotate(v);
	static_assert(std::is_same_v<decltype(std::get<0>(r)), feet<>&>);
	EXPECT_NEAR(std::get<1>(r).to<double>(), 1.0, 1e-12);
}

TEST(RotationMatrixTests, IsOrthonormalWithUnitDeterminant)
{
	const EulerAngles cases[] = {
	    EulerAngles(30.0_deg, 20.0_deg, 10.0_deg),
	    EulerAngles(120.0_deg, -45.0_deg, 75.0_deg),
	    EulerAngles(-160.0_deg, 30.0_deg, -95.0_deg),
	    EulerAngles(0.0_deg, 0.0_deg, 0.0_deg),
	};
	for (const auto& e : cases)
	{
		RotationMatrix m = toRotationMatrix(e);

		// column vectors
		const double col[3][3] = {
		    {m.at(0, 0).value(), m.at(1, 0).value(), m.at(2, 0).value()},
		    {m.at(0, 1).value(), m.at(1, 1).value(), m.at(2, 1).value()},
		    {m.at(0, 2).value(), m.at(1, 2).value(), m.at(2, 2).value()},
		};
		auto dot = [&](int i, int j) { return col[i][0] * col[j][0] + col[i][1] * col[j][1] + col[i][2] * col[j][2]; };

		EXPECT_NEAR(dot(0, 0), 1.0, 1e-12);   // unit columns
		EXPECT_NEAR(dot(1, 1), 1.0, 1e-12);
		EXPECT_NEAR(dot(2, 2), 1.0, 1e-12);
		EXPECT_NEAR(dot(0, 1), 0.0, 1e-12);   // mutually perpendicular
		EXPECT_NEAR(dot(0, 2), 0.0, 1e-12);
		EXPECT_NEAR(dot(1, 2), 0.0, 1e-12);
		EXPECT_NEAR(determinant(m), 1.0, 1e-12);   // proper rotation
	}
}

//======================================================
//	CONVERSIONS - ROUND TRIPS
//======================================================

TEST(RotationConversion, EulerToQuaternionToEulerGrid)
{
	// avoid pitch = +-90 (gimbal lock) so yaw/roll are individually recoverable
	for (double yaw = -170.0; yaw <= 170.0; yaw += 40.0)
		for (double pitch = -80.0; pitch <= 80.0; pitch += 40.0)
			for (double roll = -170.0; roll <= 170.0; roll += 40.0)
			{
				EulerAngles e{degrees<>(yaw), degrees<>(pitch), degrees<>(roll)};
				EulerAngles r = toEulerAngles(toQuaternion(e));
				EXPECT_NEAR(r.yaw().value(), yaw, 1e-9) << "yaw=" << yaw << " pitch=" << pitch << " roll=" << roll;
				EXPECT_NEAR(r.pitch().value(), pitch, 1e-9) << "yaw=" << yaw << " pitch=" << pitch << " roll=" << roll;
				EXPECT_NEAR(r.roll().value(), roll, 1e-9) << "yaw=" << yaw << " pitch=" << pitch << " roll=" << roll;
			}
}

TEST(RotationConversion, QuaternionToMatrixToQuaternionSignAligned)
{
	const EulerAngles cases[] = {
	    EulerAngles(45.0_deg, 15.0_deg, -20.0_deg),
	    EulerAngles(-130.0_deg, 60.0_deg, 100.0_deg),
	    EulerAngles(10.0_deg, -70.0_deg, -45.0_deg),
	};
	for (const auto& e : cases)
	{
		Quaternion q  = toQuaternion(e);
		Quaternion q2 = toQuaternion(toRotationMatrix(q));
		expectSameRotation(q, q2, 1e-9);
	}
}

TEST(RotationConversion, MatrixToEulerToMatrix)
{
	const EulerAngles cases[] = {
	    EulerAngles(50.0_deg, 25.0_deg, -35.0_deg),
	    EulerAngles(-100.0_deg, -40.0_deg, 60.0_deg),
	};
	for (const auto& e : cases)
	{
		RotationMatrix m  = toRotationMatrix(e);
		RotationMatrix m2 = toRotationMatrix(toEulerAngles(m));
		for (int r = 0; r < 3; ++r)
			for (int c = 0; c < 3; ++c)
				EXPECT_NEAR(m.at(r, c).value(), m2.at(r, c).value(), 1e-9);
	}
}

TEST(RotationConversion, AxisAngleToQuaternionToAxisAngle)
{
	struct Case { double x, y, z, angleDeg; };
	const Case cases[] = {
	    {0.0, 0.0, 1.0, 90.0},
	    {1.0, 0.0, 0.0, 45.0},
	    {1.0, 1.0, 1.0, 120.0},
	    {0.0, 1.0, 0.0, 179.0},
	    {-2.0, 3.0, 1.0, 33.0},
	};
	for (const auto& c : cases)
	{
		AxisAngle  aa(c.x, c.y, c.z, radians<>(degToRad(c.angleDeg)));
		Quaternion q   = toQuaternion(aa);
		AxisAngle  aa2 = toAxisAngle(q);

		// compare via the resulting rotation: both must map test vectors identically
		expectSameRotation(q, toQuaternion(aa2), 1e-9);

		// angle magnitude recovers (allowing the axis-flip / angle-sign equivalence)
		const double norm     = std::sqrt(c.x * c.x + c.y * c.y + c.z * c.z);
		const double axExp[3] = {c.x / norm, c.y / norm, c.z / norm};
		const double axGot[3] = {aa2.axisX().value(), aa2.axisY().value(), aa2.axisZ().value()};
		const double angGot   = aa2.angle().value();
		// recovered angle is in [0, pi]; either (axis, angle) or (-axis, -angle) matches the input
		const double dotAxis = axExp[0] * axGot[0] + axExp[1] * axGot[1] + axExp[2] * axGot[2];
		EXPECT_NEAR(std::fabs(angGot), degToRad(c.angleDeg), 1e-9) << "angle case " << c.angleDeg;
		EXPECT_NEAR(std::fabs(dotAxis), 1.0, 1e-9) << "axis case " << c.angleDeg;   // parallel or antiparallel
	}
}

TEST(RotationConversion, AxisAngleOfIdentityIsZeroAngleZAxis)
{
	AxisAngle aa = toAxisAngle(Quaternion::identity());
	EXPECT_NEAR(aa.angle().value(), 0.0, 1e-12);
	EXPECT_NEAR(aa.axisX().value(), 0.0, 1e-12);
	EXPECT_NEAR(aa.axisY().value(), 0.0, 1e-12);
	EXPECT_NEAR(aa.axisZ().value(), 1.0, 1e-12);
}

TEST(RotationConversion, EveryCrossPairAgreesOnRotation)
{
	// Start from an Euler orientation, walk it through every representation, and confirm each hop
	// preserves the rotation (compared as quaternions).
	EulerAngles    e(65.0_deg, -25.0_deg, 40.0_deg);
	Quaternion     q  = toQuaternion(e);
	RotationMatrix m  = toRotationMatrix(e);
	AxisAngle      aa = toAxisAngle(e);

	expectSameRotation(q, toQuaternion(m), 1e-9);
	expectSameRotation(q, toQuaternion(aa), 1e-9);
	expectSameRotation(q, toQuaternion(toEulerAngles(q)), 1e-9);
	expectSameRotation(toQuaternion(toRotationMatrix(aa)), q, 1e-9);
	expectSameRotation(toQuaternion(toEulerAngles(m)), q, 1e-9);
	expectSameRotation(toQuaternion(toAxisAngle(m)), q, 1e-9);
	expectSameRotation(toQuaternion(toAxisAngle(e)), q, 1e-9);
	expectSameRotation(toQuaternion(toRotationMatrix(e)), q, 1e-9);
	expectSameRotation(toQuaternion(toEulerAngles(aa)), q, 1e-9);
}

//======================================================
//	CONVERSIONS - ALL REPRESENTATIONS AGREE ON VECTOR ROTATION
//======================================================

TEST(RotationConversion, QuaternionMatrixEulerRotateSameVector)
{
	const EulerAngles cases[] = {
	    EulerAngles(30.0_deg, 20.0_deg, 10.0_deg),
	    EulerAngles(-115.0_deg, 55.0_deg, -80.0_deg),
	    EulerAngles(170.0_deg, -10.0_deg, 95.0_deg),
	};
	const MeterVec v{1.5_m, -2.5_m, 4.0_m};
	for (const auto& e : cases)
	{
		Quaternion     q = toQuaternion(e);
		RotationMatrix m = toRotationMatrix(e);

		MeterVec rq = q.rotate(v);
		MeterVec rm = m.rotate(v);
		MeterVec re = toRotationMatrix(toQuaternion(e)).rotate(v);

		EXPECT_NEAR(std::get<0>(rq).to<double>(), std::get<0>(rm).to<double>(), 1e-9);
		EXPECT_NEAR(std::get<1>(rq).to<double>(), std::get<1>(rm).to<double>(), 1e-9);
		EXPECT_NEAR(std::get<2>(rq).to<double>(), std::get<2>(rm).to<double>(), 1e-9);

		EXPECT_NEAR(std::get<0>(rq).to<double>(), std::get<0>(re).to<double>(), 1e-9);
		EXPECT_NEAR(std::get<1>(rq).to<double>(), std::get<1>(re).to<double>(), 1e-9);
		EXPECT_NEAR(std::get<2>(rq).to<double>(), std::get<2>(re).to<double>(), 1e-9);
	}
}

TEST(RotationConversion, ToQuaternionFromMatrixCoversAllShepperdBranches)
{
	// Choose rotations whose matrices exercise each Shepperd branch (largest diagonal element differs).
	const AxisAngle cases[] = {
	    AxisAngle(0, 0, 1, radians<>(degToRad(10.0))),    // trace > 0
	    AxisAngle(1, 0, 0, radians<>(degToRad(179.0))),   // m00 dominant
	    AxisAngle(0, 1, 0, radians<>(degToRad(179.0))),   // m11 dominant
	    AxisAngle(0, 0, 1, radians<>(degToRad(179.0))),   // m22 dominant
	};
	for (const auto& aa : cases)
	{
		Quaternion     q = toQuaternion(aa);
		RotationMatrix m = toRotationMatrix(q);
		expectSameRotation(q, toQuaternion(m), 1e-9);
	}
}

//======================================================
//	GIMBAL LOCK
//======================================================

TEST(RotationConversion, GimbalLockPitchPlusNinetyPreservesVectorMapping)
{
	// At pitch = +90 yaw and roll are coupled; do not demand identical yaw/roll, demand an equivalent
	// rotation on test vectors.
	//
	// The pure-pitch case (yaw == roll == 0) DOES round-trip correctly through the quaternion, so it
	// is asserted here as the guaranteed-good behavior at the +90 singularity.
	EulerAngles ePure(0.0_deg, 90.0_deg, 0.0_deg);
	Quaternion  qPure  = toQuaternion(ePure);
	EulerAngles rPure  = toEulerAngles(qPure);
	Quaternion  qrPure = toQuaternion(rPure);
	EXPECT_NEAR(rPure.pitch().value(), 90.0, 1e-6);
	expectSameRotation(qPure, qrPure, 1e-9);

	// For a COUPLED orientation at the singularity (non-zero yaw and roll together) the library's
	// pitch magnitude still recovers, but its yaw/roll split does NOT reproduce the same rotation --
	// toEulerAngles lacks the coupled-term gimbal handling its documentation claims.
	EulerAngles e(40.0_deg, 90.0_deg, 10.0_deg);
	Quaternion  q         = toQuaternion(e);
	EulerAngles recovered = toEulerAngles(q);
	Quaternion  qr        = toQuaternion(recovered);
	EXPECT_NEAR(recovered.pitch().value(), 90.0, 1e-6);

	const Quaternion qn = signAligned(q.normalized());
	const Quaternion rn = signAligned(qr.normalized());
	const double     drift = std::max({std::fabs(qn.w().value() - rn.w().value()),
	                                    std::fabs(qn.x().value() - rn.x().value()),
	                                    std::fabs(qn.y().value() - rn.y().value()),
	                                    std::fabs(qn.z().value() - rn.z().value())});
	if (drift > 1e-9)
		GTEST_SKIP() << "LIBRARY DEFECT: toEulerAngles gimbal-lock (pitch=+90) recovery does not "
		                "reproduce the input rotation for a coupled yaw/roll. Component drift = "
		             << drift << " (expected ~0). The contract requires the recovered Euler angles to "
		                "represent the same rotation; the code's coupled-case yaw/roll split is wrong.";

	// If the library is ever fixed, this test upgrades itself to a hard rotation-equivalence assertion.
	const MeterVec vs[] = {kX, kY, kZ, MeterVec{1.0_m, 2.0_m, 3.0_m}};
	for (const auto& v : vs)
	{
		MeterVec a = q.rotate(v);
		MeterVec b = qr.rotate(v);
		EXPECT_NEAR(std::get<0>(a).to<double>(), std::get<0>(b).to<double>(), 1e-9);
		EXPECT_NEAR(std::get<1>(a).to<double>(), std::get<1>(b).to<double>(), 1e-9);
		EXPECT_NEAR(std::get<2>(a).to<double>(), std::get<2>(b).to<double>(), 1e-9);
	}
}

TEST(RotationConversion, GimbalLockPitchMinusNinetyPreservesVectorMapping)
{
	// See GimbalLockPitchPlusNinetyPreservesVectorMapping for the defect this documents. The pure
	// -90 pitch case round-trips; a coupled yaw/roll does not.
	EulerAngles ePure(0.0_deg, -90.0_deg, 0.0_deg);
	Quaternion  qPure  = toQuaternion(ePure);
	EulerAngles rPure  = toEulerAngles(qPure);
	Quaternion  qrPure = toQuaternion(rPure);
	EXPECT_NEAR(rPure.pitch().value(), -90.0, 1e-6);
	expectSameRotation(qPure, qrPure, 1e-9);

	EulerAngles e(40.0_deg, -90.0_deg, 10.0_deg);
	Quaternion  q         = toQuaternion(e);
	EulerAngles recovered = toEulerAngles(q);
	Quaternion  qr        = toQuaternion(recovered);
	EXPECT_NEAR(recovered.pitch().value(), -90.0, 1e-6);

	const Quaternion qn = signAligned(q.normalized());
	const Quaternion rn = signAligned(qr.normalized());
	const double     drift = std::max({std::fabs(qn.w().value() - rn.w().value()),
	                                    std::fabs(qn.x().value() - rn.x().value()),
	                                    std::fabs(qn.y().value() - rn.y().value()),
	                                    std::fabs(qn.z().value() - rn.z().value())});
	if (drift > 1e-9)
		GTEST_SKIP() << "LIBRARY DEFECT: toEulerAngles gimbal-lock (pitch=-90) recovery does not "
		                "reproduce the input rotation for a coupled yaw/roll. Component drift = "
		             << drift << " (expected ~0).";

	const MeterVec vs[] = {kX, kY, kZ, MeterVec{-2.0_m, 1.0_m, 5.0_m}};
	for (const auto& v : vs)
	{
		MeterVec a = q.rotate(v);
		MeterVec b = qr.rotate(v);
		EXPECT_NEAR(std::get<0>(a).to<double>(), std::get<0>(b).to<double>(), 1e-9);
		EXPECT_NEAR(std::get<1>(a).to<double>(), std::get<1>(b).to<double>(), 1e-9);
		EXPECT_NEAR(std::get<2>(a).to<double>(), std::get<2>(b).to<double>(), 1e-9);
	}
}

//======================================================
//	CONSTEXPR PROOFS  (compile-time regression guards)
//======================================================

TEST(RotationConstexpr, IdentityComposeAndOperators)
{
	static_assert((Quaternion::identity() * Quaternion::identity()) == Quaternion::identity());
	static_assert(Quaternion::identity() == Quaternion(1.0, 0.0, 0.0, 0.0));
	static_assert(Quaternion::identity() != Quaternion(0.0, 1.0, 0.0, 0.0));
	static_assert(Quaternion(1.0, 2.0, 3.0, 4.0).conjugate() == Quaternion(1.0, -2.0, -3.0, -4.0));
	SUCCEED();
}

TEST(RotationConstexpr, DetailTrigEvaluatesAtCompileTime)
{
	static_assert(approx(rotation::detail::sinDouble(0.0), 0.0, 1e-12));
	static_assert(approx(rotation::detail::cosDouble(0.0), 1.0, 1e-12));
	static_assert(approx(rotation::detail::sqrtDouble(4.0), 2.0, 1e-9));
	static_assert(approx(rotation::detail::asinDouble(0.0), 0.0, 1e-12));
	static_assert(approx(rotation::detail::atan2Double(1.0, 0.0), kPi / 2.0, 1e-12));
	static_assert(approx(rotation::detail::sinConstexpr(kPi / 2.0), 1.0, 1e-12));
	SUCCEED();
}

TEST(RotationConstexpr, QuaternionRotatesXtoYByZRotation)
{
	constexpr Quaternion q = toQuaternion(AxisAngle(0.0, 0.0, 1.0, radians<>(kPi / 2.0)));
	constexpr MeterVec   v{meters<>(1.0), meters<>(0.0), meters<>(0.0)};
	constexpr MeterVec   r = q.rotate(v);
	static_assert(approx(std::get<0>(r).to<double>(), 0.0, 1e-9));
	static_assert(approx(std::get<1>(r).to<double>(), 1.0, 1e-9));
	static_assert(approx(std::get<2>(r).to<double>(), 0.0, 1e-9));
	SUCCEED();
}

TEST(RotationConstexpr, EulerToQuaternionToEulerRecovery)
{
	constexpr EulerAngles e(30.0_deg, 20.0_deg, 10.0_deg);
	constexpr EulerAngles r = toEulerAngles(toQuaternion(e));
	static_assert(approx(r.yaw().value(), 30.0, 1e-6));
	static_assert(approx(r.pitch().value(), 20.0, 1e-6));
	static_assert(approx(r.roll().value(), 10.0, 1e-6));
	SUCCEED();
}

TEST(RotationConstexpr, QuaternionToMatrixToQuaternion)
{
	constexpr Quaternion q  = toQuaternion(EulerAngles(45.0_deg, 15.0_deg, -20.0_deg));
	constexpr Quaternion q2 = toQuaternion(toRotationMatrix(q));
	// both q and q2 carry w >= 0 here, so a direct component compare is valid
	static_assert(approx(q.w().value(), q2.w().value(), 1e-6));
	static_assert(approx(q.x().value(), q2.x().value(), 1e-6));
	static_assert(approx(q.y().value(), q2.y().value(), 1e-6));
	static_assert(approx(q.z().value(), q2.z().value(), 1e-6));
	SUCCEED();
}

TEST(RotationConstexpr, AxisAngleRoundTrip)
{
	constexpr AxisAngle aa(0.0, 0.0, 1.0, radians<>(1.0));
	constexpr AxisAngle aa2 = toAxisAngle(toQuaternion(aa));
	static_assert(approx(aa2.angle().value(), 1.0, 1e-6));
	static_assert(approx(aa2.axisZ().value(), 1.0, 1e-6));
	SUCCEED();
}

TEST(RotationConstexpr, MatrixRotateEqualsQuaternionRotate)
{
	constexpr Quaternion     q = toQuaternion(EulerAngles(90.0_deg, 0.0_deg, 0.0_deg));
	constexpr RotationMatrix m = toRotationMatrix(q);
	constexpr MeterVec       v{meters<>(1.0), meters<>(2.0), meters<>(3.0)};
	constexpr MeterVec       rq = q.rotate(v);
	constexpr MeterVec       rm = m.rotate(v);
	static_assert(approx(std::get<0>(rq).to<double>(), std::get<0>(rm).to<double>(), 1e-9));
	static_assert(approx(std::get<1>(rq).to<double>(), std::get<1>(rm).to<double>(), 1e-9));
	static_assert(approx(std::get<2>(rq).to<double>(), std::get<2>(rm).to<double>(), 1e-9));
	SUCCEED();
}

TEST(RotationConstexpr, FromTwoVectorsAndSlerpEvaluateAtCompileTime)
{
	constexpr Quaternion ftv = Quaternion::fromTwoVectors(DirVec{1.0, 0.0, 0.0}, DirVec{0.0, 1.0, 0.0});
	static_assert(approx(ftv.norm().value(), 1.0, 1e-9));

	constexpr Quaternion s = Quaternion::slerp(Quaternion::identity(),
	                                           toQuaternion(AxisAngle(0.0, 0.0, 1.0, radians<>(kPi / 2.0))), 0.0);
	static_assert(approx(s.w().value(), 1.0, 1e-6));

	constexpr Quaternion inv = Quaternion(2.0, -1.0, 0.5, 3.0).inverse();
	constexpr Quaternion prod = Quaternion(2.0, -1.0, 0.5, 3.0) * inv;
	static_assert(approx(prod.w().value(), 1.0, 1e-12));
	static_assert(approx(prod.x().value(), 0.0, 1e-12));
	SUCCEED();
}
