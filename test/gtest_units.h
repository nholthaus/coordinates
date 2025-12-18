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

    template <class A, class B>
    using common_units_t = decay_t<decltype(std::declval<A>() - std::declval<B>())>;

    //----------------------------------------------------------------------------------------------
    // Assertion helpers (gtest predicate formatters)
    //----------------------------------------------------------------------------------------------

    // Exact equality (uses operator==)
    template <class A, class B>
    ::testing::AssertionResult UnitsEq(const char* a_expr,
                                       const char* b_expr,
                                       const A& a,
                                       const B& b)
    {
        if (a == b) return ::testing::AssertionSuccess();

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
        if (a < b) return ::testing::AssertionSuccess();

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
        if (a <= b) return ::testing::AssertionSuccess();

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
        if (a > b) return ::testing::AssertionSuccess();

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
        if (a >= b) return ::testing::AssertionSuccess();

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
        using D = common_units_t<A, B>;

        const D diff_units = (a - b);
        const auto diff = std::abs(scalar_value(diff_units));
        const auto t = std::abs(scalar_value(tol));

        if (diff <= t) return ::testing::AssertionSuccess();

        return ::testing::AssertionFailure()
            << "Expected absolute near:\n"
            << "  |" << a_expr << " - " << b_expr << "| <= " << tol_expr << "\n"
            << "  " << a_expr << " = " << a << "\n"
            << "  " << b_expr << " = " << b << "\n"
            << "  (" << a_expr << " - " << b_expr << ") = " << diff_units << "\n"
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
        using D = common_units_t<A, B>;

        const D diff_units = (a - b);
        const auto diff = std::abs(scalar_value(diff_units));

        const auto a_s = std::abs(scalar_value(a));
        const auto b_s = std::abs(scalar_value(b));

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
            << "  (" << a_expr << " - " << b_expr << ") = " << diff_units << "\n"
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
#define EXPECT_EQ_UNITS(a, b) EXPECT_PRED_FORMAT2(::gtest_units::UnitsEq, (a), (b))
#define ASSERT_EQ_UNITS(a, b) ASSERT_PRED_FORMAT2(::gtest_units::UnitsEq, (a), (b))

// Ordering
#define EXPECT_LT_UNITS(a, b) EXPECT_PRED_FORMAT2(::gtest_units::UnitsLt, (a), (b))
#define ASSERT_LT_UNITS(a, b) ASSERT_PRED_FORMAT2(::gtest_units::UnitsLt, (a), (b))

#define EXPECT_LE_UNITS(a, b) EXPECT_PRED_FORMAT2(::gtest_units::UnitsLe, (a), (b))
#define ASSERT_LE_UNITS(a, b) ASSERT_PRED_FORMAT2(::gtest_units::UnitsLe, (a), (b))

#define EXPECT_GT_UNITS(a, b) EXPECT_PRED_FORMAT2(::gtest_units::UnitsGt, (a), (b))
#define ASSERT_GT_UNITS(a, b) ASSERT_PRED_FORMAT2(::gtest_units::UnitsGt, (a), (b))

#define EXPECT_GE_UNITS(a, b) EXPECT_PRED_FORMAT2(::gtest_units::UnitsGe, (a), (b))
#define ASSERT_GE_UNITS(a, b) ASSERT_PRED_FORMAT2(::gtest_units::UnitsGe, (a), (b))

// Absolute tolerance near (unit-typed tolerance)
#define EXPECT_NEAR_UNITS(a, b, tol) EXPECT_PRED_FORMAT3(::gtest_units::UnitsNearAbs, (a), (b), (tol))
#define ASSERT_NEAR_UNITS(a, b, tol) ASSERT_PRED_FORMAT3(::gtest_units::UnitsNearAbs, (a), (b), (tol))

// Relative tolerance near (dimensionless scalar tolerance)
#define EXPECT_REL_NEAR_UNITS(a, b, rel_tol) \
    EXPECT_PRED_FORMAT3(::gtest_units::UnitsNearRel, (a), (b), static_cast<double>(rel_tol))

#define ASSERT_REL_NEAR_UNITS(a, b, rel_tol) \
    ASSERT_PRED_FORMAT3(::gtest_units::UnitsNearRel, (a), (b), static_cast<double>(rel_tol))

#endif //COORD_GTEST_UNITS_H
