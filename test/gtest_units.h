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

#ifndef COORD_GTEST_UNITS_H
#define COORD_GTEST_UNITS_H

//------------------------------
//  INCLUDES
//------------------------------
// gtest_units.hpp
#pragma once

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <limits>
#include <ostream>
#include <type_traits>

namespace gtest_units
{
    //----------------------------------------------------------------------------------------------
    // Customization point: extract a numeric scalar from a unit/quantity type.
    //
    // Prefer `to<double>()` (per your convention). If your units type instead exposes `.value()`,
    // flip the order or add another branch.
    //----------------------------------------------------------------------------------------------
    template <class U>
    constexpr auto scalar_value(const U& u)
    {
        if constexpr (requires { u.template to<double>(); })
        {
            return u.template to<double>();
        }
        else if constexpr (requires { u.value(); })
        {
            return static_cast<double>(u.value());
        }
        else
        {
            static_assert(sizeof(U) == 0,
                          "gtest_units::scalar_value(U): U must provide either `to<double>()` or `value()`.");
            return 0.0;
        }
    }

    template <class T>
    using decay_t = std::remove_cv_t<std::remove_reference_t<T>>;

    //----------------------------------------------------------------------------------------------
    // Unwrap a `units::kind<>` to its underlying plain unit; a plain unit passes through unchanged. A kind
    // exposes `unit_type` and `to<PlainUnit>()`, so this drops the tag without touching the value. Once both
    // operands are plain units, the strongly-typed comparison/subtraction below reconciles differing units
    // (km vs m) natively -- so a tagged accessor compares correctly against a plain-unit literal.
    //----------------------------------------------------------------------------------------------
    template <class U>
    constexpr auto as_plain(const U& u)
    {
        if constexpr (requires { typename U::unit_type; u.template to<typename U::unit_type>(); })
            return u.template to<typename U::unit_type>();
        else
            return u;
    }

    // Difference of two quantities as a scalar, unit-reconciled: unwrap any kind to its plain unit, then take
    // the strongly-typed `a - b` (which reconciles km vs m to a common unit) and reduce to a scalar.
    template <class A, class B>
    constexpr double scalar_difference(const A& a, const B& b)
    {
        return static_cast<double>(scalar_value(as_plain(a) - as_plain(b)));
    }

    // Ordering / equality: unwrap any kind to its plain unit, then use the strongly-typed operator so units
    // reconcile natively (a plain-vs-plain comparison is unchanged from the original behavior).
    template <class A, class B> constexpr bool scalar_eq(const A& a, const B& b) { return as_plain(a) == as_plain(b); }
    template <class A, class B> constexpr bool scalar_lt(const A& a, const B& b) { return as_plain(a) <  as_plain(b); }
    template <class A, class B> constexpr bool scalar_le(const A& a, const B& b) { return as_plain(a) <= as_plain(b); }
    template <class A, class B> constexpr bool scalar_gt(const A& a, const B& b) { return as_plain(a) >  as_plain(b); }
    template <class A, class B> constexpr bool scalar_ge(const A& a, const B& b) { return as_plain(a) >= as_plain(b); }

    //----------------------------------------------------------------------------------------------
    // Assertion helpers (gtest predicate formatters)
    //----------------------------------------------------------------------------------------------

    // Exact equality (scalar magnitudes, so a tagged kind compares against its plain unit)
    template <class A, class B>
    ::testing::AssertionResult UnitsEq(const char* a_expr,
                                       const char* b_expr,
                                       const A& a,
                                       const B& b)
    {
        if (scalar_eq(a, b)) return ::testing::AssertionSuccess();

        return ::testing::AssertionFailure()
            << "Expected equality:\n"
            << "  " << a_expr << " == " << b_expr << "\n"
            << "  " << a_expr << " = " << a << "\n"
            << "  " << b_expr << " = " << b << "\n";
    }

    // Ordering (uses operator<, <=, >, >=)
    template <class A, class B>
    ::testing::AssertionResult UnitsLt(const char* a_expr,
                                       const char* b_expr,
                                       const A& a,
                                       const B& b)
    {
        if (scalar_lt(a, b)) return ::testing::AssertionSuccess();

        return ::testing::AssertionFailure()
            << "Expected:\n"
            << "  " << a_expr << " < " << b_expr << "\n"
            << "  " << a_expr << " = " << a << "\n"
            << "  " << b_expr << " = " << b << "\n";
    }

    template <class A, class B>
    ::testing::AssertionResult UnitsLe(const char* a_expr,
                                       const char* b_expr,
                                       const A& a,
                                       const B& b)
    {
        if (scalar_le(a, b)) return ::testing::AssertionSuccess();

        return ::testing::AssertionFailure()
            << "Expected:\n"
            << "  " << a_expr << " <= " << b_expr << "\n"
            << "  " << a_expr << " = " << a << "\n"
            << "  " << b_expr << " = " << b << "\n";
    }

    template <class A, class B>
    ::testing::AssertionResult UnitsGt(const char* a_expr,
                                       const char* b_expr,
                                       const A& a,
                                       const B& b)
    {
        if (scalar_gt(a, b)) return ::testing::AssertionSuccess();

        return ::testing::AssertionFailure()
            << "Expected:\n"
            << "  " << a_expr << " > " << b_expr << "\n"
            << "  " << a_expr << " = " << a << "\n"
            << "  " << b_expr << " = " << b << "\n";
    }

    template <class A, class B>
    ::testing::AssertionResult UnitsGe(const char* a_expr,
                                       const char* b_expr,
                                       const A& a,
                                       const B& b)
    {
        if (scalar_ge(a, b)) return ::testing::AssertionSuccess();

        return ::testing::AssertionFailure()
            << "Expected:\n"
            << "  " << a_expr << " >= " << b_expr << "\n"
            << "  " << a_expr << " = " << a << "\n"
            << "  " << b_expr << " = " << b << "\n";
    }

    // Absolute tolerance near: |a - b| <= tol (tol is a unit type)
    template <class A, class B, class Tol>
    ::testing::AssertionResult UnitsNearAbs(const char* a_expr,
                                            const char* b_expr,
                                            const char* tol_expr,
                                            const A& a,
                                            const B& b,
                                            const Tol& tol)
    {
        const auto diff = std::abs(scalar_difference(a, b));
        const auto t = std::abs(scalar_value(as_plain(tol)));

        if (diff <= t) return ::testing::AssertionSuccess();

        return ::testing::AssertionFailure()
            << "Expected absolute near:\n"
            << "  |" << a_expr << " - " << b_expr << "| <= " << tol_expr << "\n"
            << "  " << a_expr << " = " << a << "\n"
            << "  " << b_expr << " = " << b << "\n"
            << "  |diff| (scalar) = " << diff << "\n"
            << "  tol   (scalar)  = " << t << "\n";
    }

    // Relative tolerance near: |a - b| <= rel_tol * max(|a|, |b|, 1)
    // rel_tol is dimensionless scalar (double).
    template <class A, class B>
    ::testing::AssertionResult UnitsNearRel(const char* a_expr,
                                            const char* b_expr,
                                            const char* rtol_expr,
                                            const A& a,
                                            const B& b,
                                            double rel_tol)
    {
        const auto a_s = std::abs(scalar_value(as_plain(a)));
        const auto b_s = std::abs(scalar_value(as_plain(b)));

        const auto diff = std::abs(scalar_difference(a, b));

        // Scale: max(|a|, |b|, 1) — the "1" avoids division-by-zero style blowups near zero.
        const auto scale = std::max({a_s, b_s, 1.0});
        const auto t = std::abs(rel_tol) * scale;

        if (diff <= t) return ::testing::AssertionSuccess();

        return ::testing::AssertionFailure()
            << "Expected relative near:\n"
            << "  |" << a_expr << " - " << b_expr << "| <= " << rtol_expr << " * max(|"
            << a_expr << "|, |" << b_expr << "|, 1)\n"
            << "  " << a_expr << " = " << a << "\n"
            << "  " << b_expr << " = " << b << "\n"
            << "  |diff| (scalar) = " << diff << "\n"
            << "  scale (scalar)  = " << scale << "\n"
            << "  rel_tol         = " << rel_tol << "\n"
            << "  tol (scalar)    = " << t << "\n";
    }
} // namespace gtest_units

//--------------------------------------------------------------------------------------------------
// Public macros
//--------------------------------------------------------------------------------------------------

// Exact equality
#define EXPECT_UNITS_EQ(a, b) EXPECT_PRED_FORMAT2(::gtest_units::UnitsEq, (a), (b))
#define ASSERT_UNITS_EQ(a, b) ASSERT_PRED_FORMAT2(::gtest_units::UnitsEq, (a), (b))

// Ordering
#define EXPECT_UNITS_LT(a, b) EXPECT_PRED_FORMAT2(::gtest_units::UnitsLt, (a), (b))
#define ASSERT_UNITS_LT(a, b) ASSERT_PRED_FORMAT2(::gtest_units::UnitsLt, (a), (b))

#define EXPECT_UNITS_LE(a, b) EXPECT_PRED_FORMAT2(::gtest_units::UnitsLe, (a), (b))
#define ASSERT_UNITS_LE(a, b) ASSERT_PRED_FORMAT2(::gtest_units::UnitsLe, (a), (b))

#define EXPECT_UNITS_GT(a, b) EXPECT_PRED_FORMAT2(::gtest_units::UnitsGt, (a), (b))
#define ASSERT_UNITS_GT(a, b) ASSERT_PRED_FORMAT2(::gtest_units::UnitsGt, (a), (b))

#define EXPECT_UNITS_GE(a, b) EXPECT_PRED_FORMAT2(::gtest_units::UnitsGe, (a), (b))
#define ASSERT_UNITS_GE(a, b) ASSERT_PRED_FORMAT2(::gtest_units::UnitsGe, (a), (b))

// Absolute tolerance near (unit-typed tolerance)
#define EXPECT_UNITS_NEAR(a, b, tol) EXPECT_PRED_FORMAT3(::gtest_units::UnitsNearAbs, (a), (b), (tol))
#define ASSERT_UNITS_NEAR(a, b, tol) ASSERT_PRED_FORMAT3(::gtest_units::UnitsNearAbs, (a), (b), (tol))

// Relative tolerance near (dimensionless scalar tolerance)
#define EXPECT_UNITS_REL_NEAR(a, b, rel_tol) \
    EXPECT_PRED_FORMAT3(::gtest_units::UnitsNearRel, (a), (b), static_cast<double>(rel_tol))

#define ASSERT_UNITS_REL_NEAR(a, b, rel_tol) \
    ASSERT_PRED_FORMAT3(::gtest_units::UnitsNearRel, (a), (b), static_cast<double>(rel_tol))

#endif //COORD_GTEST_UNITS_H