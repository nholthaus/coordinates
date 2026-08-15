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
/// @brief      Definition of the `Quaternion` class, the canonical rotation representation.
/// @details    A unit `Quaternion` stores an orientation as four dimensionless components (w, x, y, z).
///             It is the internal representation the other rotation types (`EulerAngles`, `RotationMatrix`,
///             `AxisAngle`) convert to and from. Composition, inversion, vector rotation, and spherical
///             linear interpolation are all `constexpr`-capable: they evaluate at compile time in a
///             constant-expression context and at run time otherwise, with no dependency on any external
///             linear-algebra library. The quaternion follows the Hamilton convention and represents an
///             active, right-handed rotation of a vector within a fixed frame.
//
// ---------------------------------------------------------------------------------------------------------------------

#ifndef quaternion_h
#define quaternion_h

//------------------------
//	INCLUDES
//------------------------

#include <array>
#include <tuple>

#include <units.h>

#include "rotationDetail.h"

inline namespace coordinates
{
	using namespace units;

	inline namespace rotation
	{
		//	----------------------------------------------------------------------------
		//	CLASS		Quaternion
		//  ----------------------------------------------------------------------------
		///	@brief		Canonical unit-quaternion rotation representation.
		///	@details	Hamilton convention, active rotation. Components are dimensionless. The identity
		///				quaternion is (w=1, x=0, y=0, z=0). Constructors do not force normalization; call
		///				`normalized()` (or construct through one of the sibling representations, which
		///				produce unit quaternions) when a unit quaternion is required.
		//  ----------------------------------------------------------------------------
		class Quaternion
		{
		public:
			//----------------------------------
			//	CONSTRUCTORS
			//----------------------------------

			constexpr Quaternion() noexcept = default;

			constexpr Quaternion(dimensionless<> w, dimensionless<> x, dimensionless<> y, dimensionless<> z) noexcept
			    : m_w(w)
			    , m_x(x)
			    , m_y(y)
			    , m_z(z)
			{
			}

			//----------------------------------
			//	GETTERS
			//----------------------------------

			constexpr dimensionless<> w() const noexcept { return m_w; }    ///< scalar component
			constexpr dimensionless<> x() const noexcept { return m_x; }    ///< i (x-axis) component
			constexpr dimensionless<> y() const noexcept { return m_y; }    ///< j (y-axis) component
			constexpr dimensionless<> z() const noexcept { return m_z; }    ///< k (z-axis) component

			//----------------------------------
			//	FACTORIES
			//----------------------------------

			/// The identity (no-op) rotation.
			static constexpr Quaternion identity() noexcept { return Quaternion(1.0, 0.0, 0.0, 0.0); }

			//----------------------------------
			//	QUERIES
			//----------------------------------

			/// Squared L2 norm of the four components.
			constexpr dimensionless<> normSquared() const noexcept { return m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z; }

			/// L2 norm of the four components.
			constexpr dimensionless<> norm() const noexcept { return dimensionless<>(rotation::detail::sqrtDouble(normSquared().value())); }

			//----------------------------------
			//	OPERATIONS
			//----------------------------------

			/// A copy scaled to unit length; the identity if the quaternion is degenerate (zero norm).
			constexpr Quaternion normalized() const noexcept
			{
				const double n = norm().value();
				if (n <= 0.0) return identity();
				const double inv = 1.0 / n;
				return Quaternion(m_w * inv, m_x * inv, m_y * inv, m_z * inv);
			}

			/// The quaternion conjugate (negated vector part). For a unit quaternion this is the inverse rotation.
			constexpr Quaternion conjugate() const noexcept { return Quaternion(m_w, -m_x, -m_y, -m_z); }

			/// The multiplicative inverse. Equals `conjugate()` for a unit quaternion.
			constexpr Quaternion inverse() const noexcept
			{
				const double n2 = normSquared().value();
				if (n2 <= 0.0) return identity();
				const double inv = 1.0 / n2;
				return Quaternion(m_w * inv, -m_x * inv, -m_y * inv, -m_z * inv);
			}

			/// Dot product of the four components with another quaternion.
			constexpr dimensionless<> dot(const Quaternion& other) const noexcept
			{
				return m_w * other.m_w + m_x * other.m_x + m_y * other.m_y + m_z * other.m_z;
			}

			/**
			 * @brief	Rotate a Cartesian vector by this rotation.
			 * @details	Applies q * v * q^-1 using the optimized cross-product form, so the input need not
			 *			be a unit quaternion for the direction to be correct (magnitude scales by norm^2 if
			 *			not unit). Preserves the unit type of the input components.
			 * @tparam	LengthUnit	the component unit of the input vector (any length unit).
			 * @param[in]	v	the vector to rotate, as a 3-tuple of `LengthUnit`.
			 * @return	the rotated vector, as a 3-tuple of `LengthUnit`.
			 */
			template<class LengthUnit>
			constexpr std::tuple<LengthUnit, LengthUnit, LengthUnit> rotate(const std::tuple<LengthUnit, LengthUnit, LengthUnit>& v) const noexcept
			{
				const double vx = std::get<0>(v).template to<double>();
				const double vy = std::get<1>(v).template to<double>();
				const double vz = std::get<2>(v).template to<double>();

				const double qw = m_w.value(), qx = m_x.value(), qy = m_y.value(), qz = m_z.value();

				// t = 2 * cross(q.xyz, v)
				const double tx = 2.0 * (qy * vz - qz * vy);
				const double ty = 2.0 * (qz * vx - qx * vz);
				const double tz = 2.0 * (qx * vy - qy * vx);

				// v' = v + qw * t + cross(q.xyz, t)
				const double rx = vx + qw * tx + (qy * tz - qz * ty);
				const double ry = vy + qw * ty + (qz * tx - qx * tz);
				const double rz = vz + qw * tz + (qx * ty - qy * tx);

				return {LengthUnit(rx), LengthUnit(ry), LengthUnit(rz)};
			}

			//----------------------------------
			//	FACTORIES (GEOMETRIC)
			//----------------------------------

			/**
			 * @brief	Shortest-arc rotation taking unit vector `from` onto unit vector `to`.
			 * @details	Inputs are treated as directions; their magnitudes are normalized internally.
			 *			Handles the antiparallel case by choosing an arbitrary perpendicular axis.
			 * @param[in]	from	the source direction, as a dimensionless 3-tuple.
			 * @param[in]	to		the destination direction, as a dimensionless 3-tuple.
			 * @return	a unit quaternion rotating `from` onto `to`.
			 */
			static constexpr Quaternion fromTwoVectors(const std::tuple<dimensionless<>, dimensionless<>, dimensionless<>>& from,
			                                            const std::tuple<dimensionless<>, dimensionless<>, dimensionless<>>& to) noexcept
			{
				double ax = std::get<0>(from).value(), ay = std::get<1>(from).value(), az = std::get<2>(from).value();
				double bx = std::get<0>(to).value(), by = std::get<1>(to).value(), bz = std::get<2>(to).value();

				const double an = rotation::detail::sqrtDouble(ax * ax + ay * ay + az * az);
				const double bn = rotation::detail::sqrtDouble(bx * bx + by * by + bz * bz);
				if (an <= 0.0 || bn <= 0.0) return identity();
				ax /= an, ay /= an, az /= an;
				bx /= bn, by /= bn, bz /= bn;

				const double d = ax * bx + ay * by + az * bz;
				if (d >= 1.0 - 1e-12) return identity();
				if (d <= -1.0 + 1e-12)
				{
					// antiparallel: rotate pi about any axis perpendicular to `from`
					double px = -ay, py = ax, pz = 0.0;
					if (px * px + py * py + pz * pz < 1e-12) { px = 0.0; py = -az; pz = ay; }
					const double pn = rotation::detail::sqrtDouble(px * px + py * py + pz * pz);
					return Quaternion(0.0, px / pn, py / pn, pz / pn);
				}

				// cross(a, b) forms the vector part; w = 1 + dot, then normalize.
				const double cx = ay * bz - az * by;
				const double cy = az * bx - ax * bz;
				const double cz = ax * by - ay * bx;
				return Quaternion(1.0 + d, cx, cy, cz).normalized();
			}

			//----------------------------------
			//	INTERPOLATION
			//----------------------------------

			/**
			 * @brief	Spherical linear interpolation between two unit quaternions.
			 * @details	Chooses the shorter arc (flips `b` if the dot product is negative). Falls back to
			 *			normalized linear interpolation when the endpoints are nearly identical.
			 * @param[in]	a	the start orientation (t = 0), assumed unit.
			 * @param[in]	b	the end orientation (t = 1), assumed unit.
			 * @param[in]	t	the interpolation parameter, nominally in [0, 1].
			 * @return	the interpolated unit quaternion.
			 */
			static constexpr Quaternion slerp(const Quaternion& a, Quaternion b, double t) noexcept
			{
				double d = a.dot(b).value();
				if (d < 0.0)
				{
					b = Quaternion(-b.m_w, -b.m_x, -b.m_y, -b.m_z);
					d = -d;
				}

				if (d > 1.0 - 1e-9)
				{
					// nearly parallel: normalized lerp avoids division by ~zero sin(theta)
					return Quaternion(a.m_w + (b.m_w - a.m_w) * t,
					                  a.m_x + (b.m_x - a.m_x) * t,
					                  a.m_y + (b.m_y - a.m_y) * t,
					                  a.m_z + (b.m_z - a.m_z) * t)
					    .normalized();
				}

				const double theta    = rotation::detail::atan2Double(rotation::detail::sqrtDouble(1.0 - d * d), d);
				const double sinTheta = rotation::detail::sinDouble(theta);
				const double wa       = rotation::detail::sinDouble((1.0 - t) * theta) / sinTheta;
				const double wb       = rotation::detail::sinDouble(t * theta) / sinTheta;
				return Quaternion(a.m_w * wa + b.m_w * wb, a.m_x * wa + b.m_x * wb, a.m_y * wa + b.m_y * wb, a.m_z * wa + b.m_z * wb);
			}

		private:
			dimensionless<> m_w{1.0};    ///< scalar component
			dimensionless<> m_x{0.0};    ///< i component
			dimensionless<> m_y{0.0};    ///< j component
			dimensionless<> m_z{0.0};    ///< k component
		};

		//----------------------------------
		//	OPERATORS
		//----------------------------------

		/// Hamilton-product composition. `lhs * rhs` applies `rhs` first, then `lhs`.
		constexpr Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs) noexcept
		{
			const double aw = lhs.w().value(), ax = lhs.x().value(), ay = lhs.y().value(), az = lhs.z().value();
			const double bw = rhs.w().value(), bx = rhs.x().value(), by = rhs.y().value(), bz = rhs.z().value();
			return Quaternion(aw * bw - ax * bx - ay * by - az * bz,
			                  aw * bx + ax * bw + ay * bz - az * by,
			                  aw * by - ax * bz + ay * bw + az * bx,
			                  aw * bz + ax * by - ay * bx + az * bw);
		}

		/// Exact component-wise equality. Prefer an approximate comparison for computed rotations.
		constexpr bool operator==(const Quaternion& lhs, const Quaternion& rhs) noexcept
		{
			return lhs.w() == rhs.w() && lhs.x() == rhs.x() && lhs.y() == rhs.y() && lhs.z() == rhs.z();
		}

		constexpr bool operator!=(const Quaternion& lhs, const Quaternion& rhs) noexcept { return !(lhs == rhs); }
	}    // namespace rotation
}    // namespace coordinates

#endif    // quaternion_h
