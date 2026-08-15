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
/// @brief      Definitions of the `EulerAngles`, `RotationMatrix`, and `AxisAngle` rotation
///             representations, and their interconversions with `Quaternion`.
/// @details    These four representations describe the same object -- a 3D rotation -- in the form best
///             suited to a given task: `Quaternion` for composition/interpolation, `EulerAngles` for
///             human-readable yaw/pitch/roll, `RotationMatrix` for direct application to Cartesian
///             vectors, and `AxisAngle` for an intuitive axis-and-magnitude description. Every
///             representation converts to and from every other via the quaternion, and every operation is
///             `constexpr`-capable (using the shared constant-evaluable trig in `rotationDetail.h`) so a
///             rotation known at compile time is computed at compile time. Euler angles use the intrinsic
///             Z-Y-X (Tait-Bryan) convention: yaw about Z, then pitch about the new Y, then roll about the
///             new X -- the aerospace body-axis convention.
//
// ---------------------------------------------------------------------------------------------------------------------

#ifndef rotation_h
#define rotation_h

//------------------------
//	INCLUDES
//------------------------

#include <array>
#include <tuple>

#include <units.h>

#include "quaternion.h"
#include "rotationDetail.h"

inline namespace coordinates
{
	using namespace units;

	inline namespace rotation
	{
		//	----------------------------------------------------------------------------
		//	CLASS		EulerAngles
		//  ----------------------------------------------------------------------------
		///	@brief		Yaw/pitch/roll rotation, intrinsic Z-Y-X (Tait-Bryan) convention.
		///	@details	`yaw` rotates about the Z axis, `pitch` about the resulting Y axis, `roll` about the
		///				resulting X axis. Stored as `degrees<>` for readability; all trigonometry converts
		///				to radians internally.
		//  ----------------------------------------------------------------------------
		class EulerAngles
		{
		public:
			constexpr EulerAngles() noexcept = default;

			constexpr EulerAngles(degrees<> yaw, degrees<> pitch, degrees<> roll) noexcept
			    : m_yaw(yaw)
			    , m_pitch(pitch)
			    , m_roll(roll)
			{
			}

			constexpr degrees<> yaw() const noexcept { return m_yaw; }        ///< rotation about Z
			constexpr degrees<> pitch() const noexcept { return m_pitch; }    ///< rotation about the new Y
			constexpr degrees<> roll() const noexcept { return m_roll; }      ///< rotation about the new X

			/// The tuple form matching `FrameData::orientation` (yaw, pitch, roll), all `degrees<>`.
			constexpr std::tuple<degrees<>, degrees<>, degrees<>> toTuple() const noexcept { return {m_yaw, m_pitch, m_roll}; }

		private:
			degrees<> m_yaw{0.0};
			degrees<> m_pitch{0.0};
			degrees<> m_roll{0.0};
		};

		//	----------------------------------------------------------------------------
		//	CLASS		AxisAngle
		//  ----------------------------------------------------------------------------
		///	@brief		Rotation as a unit axis and an angle about it (right-handed).
		///	@details	The axis is stored as three dimensionless components and normalized on use; the angle
		///				is stored in `radians<>`. A zero angle is the identity regardless of axis.
		//  ----------------------------------------------------------------------------
		class AxisAngle
		{
		public:
			constexpr AxisAngle() noexcept = default;

			constexpr AxisAngle(dimensionless<> axisX, dimensionless<> axisY, dimensionless<> axisZ, radians<> angle) noexcept
			    : m_axisX(axisX)
			    , m_axisY(axisY)
			    , m_axisZ(axisZ)
			    , m_angle(angle)
			{
			}

			constexpr dimensionless<> axisX() const noexcept { return m_axisX; }
			constexpr dimensionless<> axisY() const noexcept { return m_axisY; }
			constexpr dimensionless<> axisZ() const noexcept { return m_axisZ; }
			constexpr radians<>       angle() const noexcept { return m_angle; }

		private:
			dimensionless<> m_axisX{0.0};
			dimensionless<> m_axisY{0.0};
			dimensionless<> m_axisZ{1.0};
			radians<>       m_angle{0.0};
		};

		//	----------------------------------------------------------------------------
		//	CLASS		RotationMatrix
		//  ----------------------------------------------------------------------------
		///	@brief		3x3 direction-cosine matrix representation of a rotation.
		///	@details	Row-major storage of nine dimensionless elements. Applying the matrix to a column
		///				vector rotates it (active convention), consistent with `Quaternion::rotate`.
		//  ----------------------------------------------------------------------------
		class RotationMatrix
		{
		public:
			constexpr RotationMatrix() noexcept = default;

			/// Construct from nine row-major elements (m[row][col]).
			constexpr RotationMatrix(dimensionless<> m00, dimensionless<> m01, dimensionless<> m02,
			                         dimensionless<> m10, dimensionless<> m11, dimensionless<> m12,
			                         dimensionless<> m20, dimensionless<> m21, dimensionless<> m22) noexcept
			    : m_e{m00, m01, m02, m10, m11, m12, m20, m21, m22}
			{
			}

			/// Element access, row-major, `row`/`col` in [0, 2].
			constexpr dimensionless<> at(int row, int col) const noexcept { return m_e[static_cast<std::size_t>(row * 3 + col)]; }

			static constexpr RotationMatrix identity() noexcept { return RotationMatrix(1, 0, 0, 0, 1, 0, 0, 0, 1); }

			/**
			 * @brief	Apply the matrix to a Cartesian vector (active rotation).
			 * @tparam	LengthUnit	the component unit of the vector.
			 * @param[in]	v	the vector to rotate, as a 3-tuple of `LengthUnit`.
			 * @return	the rotated vector, as a 3-tuple of `LengthUnit`.
			 */
			template<class LengthUnit>
			constexpr std::tuple<LengthUnit, LengthUnit, LengthUnit> rotate(const std::tuple<LengthUnit, LengthUnit, LengthUnit>& v) const noexcept
			{
				const double vx = std::get<0>(v).template to<double>();
				const double vy = std::get<1>(v).template to<double>();
				const double vz = std::get<2>(v).template to<double>();
				return {LengthUnit(m_e[0].value() * vx + m_e[1].value() * vy + m_e[2].value() * vz),
				        LengthUnit(m_e[3].value() * vx + m_e[4].value() * vy + m_e[5].value() * vz),
				        LengthUnit(m_e[6].value() * vx + m_e[7].value() * vy + m_e[8].value() * vz)};
			}

		private:
			std::array<dimensionless<>, 9> m_e{dimensionless<>(1.0), dimensionless<>(0.0), dimensionless<>(0.0),
			                                   dimensionless<>(0.0), dimensionless<>(1.0), dimensionless<>(0.0),
			                                   dimensionless<>(0.0), dimensionless<>(0.0), dimensionless<>(1.0)};
		};

		//----------------------------------
		//	CONVERSIONS: -> Quaternion
		//----------------------------------

		/**
		 * @brief	Build a quaternion from an axis-angle rotation.
		 * @param[in]	aa	the axis-angle rotation (axis normalized internally).
		 * @return	the equivalent unit quaternion (identity if the axis is degenerate).
		 */
		constexpr Quaternion toQuaternion(const AxisAngle& aa) noexcept
		{
			double ax = aa.axisX().value(), ay = aa.axisY().value(), az = aa.axisZ().value();
			const double n = detail::sqrtDouble(ax * ax + ay * ay + az * az);
			if (n <= 0.0) return Quaternion::identity();
			ax /= n, ay /= n, az /= n;

			const double half = 0.5 * aa.angle().value();
			const double s    = detail::sinDouble(half);
			const double c    = detail::cosDouble(half);
			return Quaternion(c, ax * s, ay * s, az * s);
		}

		/**
		 * @brief	Build a quaternion from intrinsic Z-Y-X (yaw-pitch-roll) Euler angles.
		 * @param[in]	e	the Euler angles.
		 * @return	the equivalent unit quaternion.
		 */
		constexpr Quaternion toQuaternion(const EulerAngles& e) noexcept
		{
			const double halfYaw   = 0.5 * radians<>(e.yaw()).value();
			const double halfPitch = 0.5 * radians<>(e.pitch()).value();
			const double halfRoll  = 0.5 * radians<>(e.roll()).value();

			const double cy = detail::cosDouble(halfYaw), sy = detail::sinDouble(halfYaw);
			const double cp = detail::cosDouble(halfPitch), sp = detail::sinDouble(halfPitch);
			const double cr = detail::cosDouble(halfRoll), sr = detail::sinDouble(halfRoll);

			return Quaternion(cr * cp * cy + sr * sp * sy,
			                  sr * cp * cy - cr * sp * sy,
			                  cr * sp * cy + sr * cp * sy,
			                  cr * cp * sy - sr * sp * cy);
		}

		/**
		 * @brief	Build a quaternion from a rotation matrix (Shepperd's method).
		 * @param[in]	m	the rotation matrix (assumed orthonormal).
		 * @return	the equivalent unit quaternion.
		 */
		constexpr Quaternion toQuaternion(const RotationMatrix& m) noexcept
		{
			const double m00 = m.at(0, 0).value(), m01 = m.at(0, 1).value(), m02 = m.at(0, 2).value();
			const double m10 = m.at(1, 0).value(), m11 = m.at(1, 1).value(), m12 = m.at(1, 2).value();
			const double m20 = m.at(2, 0).value(), m21 = m.at(2, 1).value(), m22 = m.at(2, 2).value();

			const double trace = m00 + m11 + m22;
			if (trace > 0.0)
			{
				const double s = 0.5 / detail::sqrtDouble(trace + 1.0);
				return Quaternion(0.25 / s, (m21 - m12) * s, (m02 - m20) * s, (m10 - m01) * s);
			}
			if (m00 > m11 && m00 > m22)
			{
				const double s = 2.0 * detail::sqrtDouble(1.0 + m00 - m11 - m22);
				return Quaternion((m21 - m12) / s, 0.25 * s, (m01 + m10) / s, (m02 + m20) / s);
			}
			if (m11 > m22)
			{
				const double s = 2.0 * detail::sqrtDouble(1.0 + m11 - m00 - m22);
				return Quaternion((m02 - m20) / s, (m01 + m10) / s, 0.25 * s, (m12 + m21) / s);
			}
			const double s = 2.0 * detail::sqrtDouble(1.0 + m22 - m00 - m11);
			return Quaternion((m10 - m01) / s, (m02 + m20) / s, (m12 + m21) / s, 0.25 * s);
		}

		//----------------------------------
		//	CONVERSIONS: Quaternion ->
		//----------------------------------

		/**
		 * @brief	Extract the rotation matrix of a quaternion.
		 * @param[in]	q	the rotation (normalized internally).
		 * @return	the equivalent 3x3 direction-cosine matrix.
		 */
		constexpr RotationMatrix toRotationMatrix(const Quaternion& q) noexcept
		{
			const Quaternion u = q.normalized();
			const double w = u.w().value(), x = u.x().value(), y = u.y().value(), z = u.z().value();
			return RotationMatrix(1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y - w * z), 2.0 * (x * z + w * y),
			                      2.0 * (x * y + w * z), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z - w * x),
			                      2.0 * (x * z - w * y), 2.0 * (y * z + w * x), 1.0 - 2.0 * (x * x + y * y));
		}

		/**
		 * @brief	Extract the intrinsic Z-Y-X (yaw-pitch-roll) Euler angles of a quaternion.
		 * @details	Handles gimbal-lock (|pitch| = 90 deg) by assigning the coupled rotation to yaw.
		 * @param[in]	q	the rotation (normalized internally).
		 * @return	the equivalent Euler angles, in degrees.
		 */
		constexpr EulerAngles toEulerAngles(const Quaternion& q) noexcept
		{
			const Quaternion u = q.normalized();
			const double w = u.w().value(), x = u.x().value(), y = u.y().value(), z = u.z().value();

			// pitch (Y): sin(pitch) = 2(wy - zx), clamped to the valid asin domain.
			double sinPitch = 2.0 * (w * y - z * x);
			sinPitch        = sinPitch > 1.0 ? 1.0 : (sinPitch < -1.0 ? -1.0 : sinPitch);

			const double radToDeg = 180.0 / detail::PI;

			// At |pitch| = 90 deg the yaw and roll axes align (gimbal lock): only their sum (pitch = +90)
			// or difference (pitch = -90) is observable. Resolve to roll = 0 and place the coupled angle
			// in yaw, which reproduces the same rotation.
			if (sinPitch > 1.0 - 1e-9 || sinPitch < -1.0 + 1e-9)
			{
				const double pitch = detail::asinDouble(sinPitch);
				// Gimbal lock: only one of (yaw - roll) [pitch = +90] or (yaw + roll) [pitch = -90] is
				// observable. Resolve to roll = 0 with the coupled angle in yaw; 2*atan2(z, w) reproduces
				// the observable for both pitch signs (derived from this Z-Y-X quaternion construction).
				const double coupledYaw = 2.0 * detail::atan2Double(z, w);
				return EulerAngles(degrees<>(coupledYaw * radToDeg), degrees<>(pitch * radToDeg), degrees<>(0.0));
			}

			const double pitch = detail::asinDouble(sinPitch);

			// roll (X)
			const double sinRollCosPitch = 2.0 * (w * x + y * z);
			const double cosRollCosPitch = 1.0 - 2.0 * (x * x + y * y);
			const double roll            = detail::atan2Double(sinRollCosPitch, cosRollCosPitch);

			// yaw (Z)
			const double sinYawCosPitch = 2.0 * (w * z + x * y);
			const double cosYawCosPitch = 1.0 - 2.0 * (y * y + z * z);
			const double yaw            = detail::atan2Double(sinYawCosPitch, cosYawCosPitch);

			return EulerAngles(degrees<>(yaw * radToDeg), degrees<>(pitch * radToDeg), degrees<>(roll * radToDeg));
		}

		/**
		 * @brief	Extract the axis-angle form of a quaternion.
		 * @param[in]	q	the rotation (normalized internally).
		 * @return	the equivalent axis-angle rotation; the +Z axis with zero angle for the identity.
		 */
		constexpr AxisAngle toAxisAngle(const Quaternion& q) noexcept
		{
			const Quaternion u = q.normalized();
			const double w = u.w().value();
			const double vx = u.x().value(), vy = u.y().value(), vz = u.z().value();

			const double sinHalf = detail::sqrtDouble(vx * vx + vy * vy + vz * vz);
			if (sinHalf <= 1e-12) return AxisAngle(0.0, 0.0, 1.0, radians<>(0.0));

			const double clampedW = w > 1.0 ? 1.0 : (w < -1.0 ? -1.0 : w);
			const double angle    = 2.0 * detail::atan2Double(sinHalf, clampedW);
			return AxisAngle(vx / sinHalf, vy / sinHalf, vz / sinHalf, radians<>(angle));
		}

		//----------------------------------
		//	CONVENIENCE CROSS-CONVERSIONS
		//----------------------------------

		constexpr RotationMatrix toRotationMatrix(const EulerAngles& e) noexcept { return toRotationMatrix(toQuaternion(e)); }
		constexpr RotationMatrix toRotationMatrix(const AxisAngle& aa) noexcept { return toRotationMatrix(toQuaternion(aa)); }
		constexpr EulerAngles    toEulerAngles(const RotationMatrix& m) noexcept { return toEulerAngles(toQuaternion(m)); }
		constexpr EulerAngles    toEulerAngles(const AxisAngle& aa) noexcept { return toEulerAngles(toQuaternion(aa)); }
		constexpr AxisAngle      toAxisAngle(const EulerAngles& e) noexcept { return toAxisAngle(toQuaternion(e)); }
		constexpr AxisAngle      toAxisAngle(const RotationMatrix& m) noexcept { return toAxisAngle(toQuaternion(m)); }
	}    // namespace rotation
}    // namespace coordinates

#endif    // rotation_h
