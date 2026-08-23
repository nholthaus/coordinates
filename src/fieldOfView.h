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
//
// A sensor's angular field of view, tested for containment. `FieldOfView` is a rectangular frustum in
// BODY axes: a boresight direction (default forward, +x) plus a horizontal and a vertical half-angle. A
// direction or a world-space target is inside the field of view when, resolved into the sensor's body axes and
// measured off the boresight, its azimuth is within the horizontal half-angle and its elevation within the
// vertical half-angle. A `Pose` places the sensor, so a world target is rotated into body axes by the pose's
// inverse before the angular test. The type is geometry only -- containment and the boresight look direction --
// with no detection, range, or sensor physics; combine it with a terrain query for full visibility.
//
//--------------------------------------------------------------------------------------------------

#ifndef fieldOfView_h
#define fieldOfView_h

//------------------------
//	INCLUDES
//------------------------

#include <cmath>

#include <units.h>

#include "frameOfReference.h"
#include "pose.h"

inline namespace coordinates
{
	using namespace units;
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	CLASS		FieldOfView
	//  ----------------------------------------------------------------------------
	///	@brief		A sensor's rectangular angular field of view in body axes: a boresight plus horizontal and
	///				vertical half-angles, with containment tests.
	///	@details	The boresight is a body-axis direction (default forward, +x). A candidate direction is
	///				resolved into body axes and its offset from the boresight measured as an azimuth (about the
	///				body down/+z axis, + toward +y) and an elevation (about the body right/+y axis, + toward +z);
	///				it is contained when |azimuth| <= the horizontal half-angle and |elevation| <= the vertical
	///				half-angle. With a `Pose`, a world-space direction or target is rotated into body axes first.
	//  ----------------------------------------------------------------------------
	class FieldOfView
	{
	public:
		//----------------------------------
		//	CONSTRUCTORS
		//----------------------------------

		/// A field of view about the forward (+x) boresight with the given half-angles.
		constexpr FieldOfView(degrees<> horizontalHalfAngle, degrees<> verticalHalfAngle)
		    : FieldOfView(CartesianTuple(1.0_m, 0.0_m, 0.0_m), horizontalHalfAngle, verticalHalfAngle)
		{
		}

		/// A field of view about an arbitrary body-axis boresight with the given half-angles.
		constexpr FieldOfView(const CartesianTuple& boresight, degrees<> horizontalHalfAngle, degrees<> verticalHalfAngle)
		    : m_boresight(normalized(boresight))
		    , m_horizontalHalfAngle(horizontalHalfAngle)
		    , m_verticalHalfAngle(verticalHalfAngle)
		{
		}

		//----------------------------------
		//	GETTERS
		//----------------------------------

		[[nodiscard]] constexpr const CartesianTuple&    boresight() const { return m_boresight; }
		[[nodiscard]] constexpr degrees<>  horizontalHalfAngle() const { return m_horizontalHalfAngle; }
		[[nodiscard]] constexpr degrees<>  verticalHalfAngle() const { return m_verticalHalfAngle; }

		//----------------------------------
		//	CONTAINMENT
		//----------------------------------

		/// True when a BODY-axis direction lies within the field of view (its azimuth/elevation off the
		/// boresight are within the half-angles). The direction need not be normalized.
		[[nodiscard]] bool contains(const CartesianTuple& bodyDirection) const
		{
			// Offset of the direction from the boresight, decomposed against the boresight's own local frame.
			// Build an orthonormal boresight basis: forward = boresight, right = forward x up_hint, up = right x
			// forward. Then azimuth = atan2(dir.right, dir.forward), elevation = atan2(dir.up, dir.forward).
			const double bx = std::get<0>(m_boresight).value(), by = std::get<1>(m_boresight).value(), bz = std::get<2>(m_boresight).value();
			double       dx = std::get<0>(bodyDirection).value(), dy = std::get<1>(bodyDirection).value(), dz = std::get<2>(bodyDirection).value();
			const double dn = std::sqrt(dx * dx + dy * dy + dz * dz);
			if (dn == 0.0)
				return false;
			dx /= dn; dy /= dn; dz /= dn;

			// A stable "up hint" not parallel to the boresight: body down (+z) unless the boresight is nearly
			// vertical, in which case body forward (+x).
			double ux = 0.0, uy = 0.0, uz = 1.0;
			if (std::abs(bz) > 0.9)
			{
				ux = 1.0; uy = 0.0; uz = 0.0;
			}
			// right = forward x up_hint
			double rx = by * uz - bz * uy, ry = bz * ux - bx * uz, rz = bx * uy - by * ux;
			const double rn = std::sqrt(rx * rx + ry * ry + rz * rz);
			rx /= rn; ry /= rn; rz /= rn;
			// up = right x forward
			const double upx = ry * bz - rz * by, upy = rz * bx - rx * bz, upz = rx * by - ry * bx;

			const double fwd   = dx * bx + dy * by + dz * bz;      // component along boresight
			const double right = dx * rx + dy * ry + dz * rz;      // component to the right
			const double up    = dx * upx + dy * upy + dz * upz;   // component up
			if (fwd <= 0.0)
				return false;    // behind the sensor

			const double azimuth   = std::atan2(right, fwd);
			const double elevation = std::atan2(up, fwd);
			const double hh        = radians<>(m_horizontalHalfAngle).value();
			const double vh        = radians<>(m_verticalHalfAngle).value();
			return std::abs(azimuth) <= hh && std::abs(elevation) <= vh;
		}

		/// True when a world-space target POINT lies within the field of view, given the viewer's pose. The look
		/// direction from the viewer's position (the pose translation) to the target is rotated into body axes by
		/// the pose's inverse, then tested against the half-angles.
		[[nodiscard]] bool contains(const Pose& viewerPose, const CartesianTuple& worldTarget) const
		{
			const CartesianTuple origin = viewerPose.translation();
			const CartesianTuple lookWorld(std::get<0>(worldTarget) - std::get<0>(origin),
			                               std::get<1>(worldTarget) - std::get<1>(origin),
			                               std::get<2>(worldTarget) - std::get<2>(origin));
			return contains(viewerPose.inverse().rotateDirection(lookWorld));
		}

	private:
		//	----------------------------------------------------------------------------
		//	FUNCTION: normalized [static, private]
		//  ----------------------------------------------------------------------------
		///	@brief		Return a direction scaled to unit length (a zero vector defaults to forward, +x).
		///	@param[in]	direction	the direction to normalize.
		///	@return		the unit-length direction.
		//  ----------------------------------------------------------------------------
		static constexpr CartesianTuple normalized(const CartesianTuple& direction)
		{
			const double dx = std::get<0>(direction).value(), dy = std::get<1>(direction).value(), dz = std::get<2>(direction).value();
			const double n  = units::sqrt(dx * dx + dy * dy + dz * dz);
			if (n == 0.0)
				return CartesianTuple(1.0_m, 0.0_m, 0.0_m);
			return CartesianTuple(meters<>(dx / n), meters<>(dy / n), meters<>(dz / n));
		}

		CartesianTuple          m_boresight{1.0_m, 0.0_m, 0.0_m};    ///< the look direction in body axes (unit)
		degrees<> m_horizontalHalfAngle{0.0};          ///< half the horizontal angular extent
		degrees<> m_verticalHalfAngle{0.0};            ///< half the vertical angular extent
	};
}    // namespace coordinates

#endif    // fieldOfView_h
