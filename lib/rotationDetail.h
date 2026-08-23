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
/// @brief      Constant-evaluation helpers shared by the rotation-math representations.
/// @details    Supplies `constexpr` trigonometric primitives used ONLY when a rotation conversion is
///             evaluated in a constant-expression context; at runtime the callers dispatch to the
///             standard-library / `units` trig for full-accuracy results. Also provides small
///             dependency-free scalar helpers (dimensionless square root, hypot) so the representations
///             need no third-party linear-algebra library.
//
// ---------------------------------------------------------------------------------------------------------------------

#ifndef rotationDetail_h
#define rotationDetail_h

//------------------------
//	INCLUDES
//------------------------

#include <cmath>

#include <units.h>

inline namespace coordinates
{
	using namespace units;

	inline namespace rotation
	{
	namespace detail
	{
		//----------------------------------
		//	CONSTANTS
		//----------------------------------

		inline constexpr double PI = 3.14159265358979323846264338327950288;

		//----------------------------------
		//	CONSTEXPR TRIG PRIMITIVES
		//----------------------------------

		/**
		 * @brief	Reduce an angle in radians to the interval (-pi, pi].
		 */
		constexpr double wrapRadians(double radians) noexcept
		{
			const double twoPi = 2.0 * PI;
			while (radians > PI) radians -= twoPi;
			while (radians <= -PI) radians += twoPi;
			return radians;
		}

		/**
		 * @brief	Constant-evaluable sine (radians), Taylor series about the reduced angle.
		 */
		constexpr double sinConstexpr(double radians) noexcept
		{
			const double x    = wrapRadians(radians);
			const double x2   = x * x;
			double       term = x;
			double       sum  = x;
			for (int n = 1; n < 24; ++n)
			{
				term *= -x2 / static_cast<double>((2 * n) * (2 * n + 1));
				sum += term;
			}
			return sum;
		}

		/**
		 * @brief	Constant-evaluable cosine (radians), Taylor series about the reduced angle.
		 */
		constexpr double cosConstexpr(double radians) noexcept
		{
			const double x    = wrapRadians(radians);
			const double x2   = x * x;
			double       term = 1.0;
			double       sum  = 1.0;
			for (int n = 1; n < 24; ++n)
			{
				term *= -x2 / static_cast<double>((2 * n - 1) * (2 * n));
				sum += term;
			}
			return sum;
		}

		/**
		 * @brief	Constant-evaluable square root (Newton-Raphson), for non-negative arguments.
		 */
		constexpr double sqrtConstexpr(double value) noexcept
		{
			if (value <= 0.0) return 0.0;
			double guess = value;
			for (int n = 0; n < 64; ++n)
			{
				const double next = 0.5 * (guess + value / guess);
				if (next == guess) break;
				guess = next;
			}
			return guess;
		}

		/**
		 * @brief	Constant-evaluable arcsine, in radians, for arguments in [-1, 1].
		 * @details	Uses the identity asin(x) = atan(x / sqrt(1 - x^2)) with a range-reduced atan.
		 */
		constexpr double asinConstexpr(double value) noexcept;

		/**
		 * @brief	Constant-evaluable arctangent, in radians.
		 * @details	Range-reduces to |x| <= 1 via atan(x) = pi/2 - atan(1/x), then applies the
		 *			Euler accelerated series which converges for |x| <= 1.
		 */
		constexpr double atanConstexpr(double value) noexcept
		{
			const bool negative = value < 0.0;
			double     x        = negative ? -value : value;
			bool       reduced  = false;
			if (x > 1.0)
			{
				x       = 1.0 / x;
				reduced = true;
			}
			const double x2 = x * x;
			// Euler's series: atan(x) = (x / (1 + x^2)) * sum_{n>=0} prod_{k=1..n} (2k x^2)/((2k+1)(1+x^2))
			double term = x / (1.0 + x2);
			double sum  = term;
			for (int n = 1; n < 60; ++n)
			{
				term *= (2.0 * n * x2) / ((2.0 * n + 1.0) * (1.0 + x2));
				sum += term;
			}
			double result = reduced ? (PI / 2.0 - sum) : sum;
			return negative ? -result : result;
		}

		//----------------------------------------------------------------------------------------------------------------------
		constexpr double asinConstexpr(double value) noexcept
		{
			if (value >= 1.0) return PI / 2.0;
			if (value <= -1.0) return -PI / 2.0;
			return atanConstexpr(value / sqrtConstexpr(1.0 - value * value));
		}

		/**
		 * @brief	Constant-evaluable four-quadrant arctangent, in radians.
		 */
		constexpr double atan2Constexpr(double y, double x) noexcept
		{
			if (x > 0.0) return atanConstexpr(y / x);
			if (x < 0.0) return (y >= 0.0) ? atanConstexpr(y / x) + PI : atanConstexpr(y / x) - PI;
			// x == 0
			if (y > 0.0) return PI / 2.0;
			if (y < 0.0) return -PI / 2.0;
			return 0.0;
		}

		//----------------------------------
		//	CONSTEVAL-AWARE DISPATCHERS
		//----------------------------------

		/**
		 * @brief	Sine of an angle in radians: `std::sin` at runtime, the `constexpr` series when
		 *			constant-evaluated.
		 */
		constexpr double sinDouble(double radians) noexcept
		{
			if consteval
			{
				return sinConstexpr(radians);
			}
			else
			{
				return std::sin(radians);
			}
		}

		/**
		 * @brief	Cosine of an angle in radians: `std::cos` at runtime, the `constexpr` series when
		 *			constant-evaluated.
		 */
		constexpr double cosDouble(double radians) noexcept
		{
			if consteval
			{
				return cosConstexpr(radians);
			}
			else
			{
				return std::cos(radians);
			}
		}

		/**
		 * @brief	Square root: `std::sqrt` at runtime, Newton-Raphson when constant-evaluated.
		 */
		constexpr double sqrtDouble(double value) noexcept
		{
			if consteval
			{
				return sqrtConstexpr(value);
			}
			else
			{
				return std::sqrt(value);
			}
		}

		/**
		 * @brief	Arcsine (radians): `std::asin` at runtime, the `constexpr` reduction otherwise.
		 */
		constexpr double asinDouble(double value) noexcept
		{
			if consteval
			{
				return asinConstexpr(value);
			}
			else
			{
				return std::asin(value);
			}
		}

		/**
		 * @brief	Four-quadrant arctangent (radians): `std::atan2` at runtime, the `constexpr`
		 *			reduction otherwise.
		 */
		constexpr double atan2Double(double y, double x) noexcept
		{
			if consteval
			{
				return atan2Constexpr(y, x);
			}
			else
			{
				return std::atan2(y, x);
			}
		}
	}    // namespace detail
	}    // namespace rotation
}    // namespace coordinates

#endif    // rotationDetail_h
