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
// Kinematic state primitives: frame-tagged velocity, acceleration, and angular-rate vectors, plus the
// rotating-frame quantities that relate them across frames (the Earth rate omega_ie, the transport rate
// omega_en, and the Coriolis/centrifugal contribution terms). These are the STATE an external integrator
// reads and writes; this library supplies the typed vectors and the correct transforms, not the integrator
// or the time loop. Each type is a `Vector<Frame, Unit>` with the appropriate `units` quantity, so it
// inherits the frame tag, the rotation-only frame conversion, the algebra, and the opt-in Eigen interop.
//
//--------------------------------------------------------------------------------------------------

#ifndef kinematics_h
#define kinematics_h

//------------------------
//	INCLUDES
//------------------------

#include <units.h>

#include "frameOfReference.h"
#include "vector.h"

inline namespace coordinates
{
	//======================================================
	//	KINEMATIC VECTOR TYPES (frame-tagged, unit-typed)
	//======================================================

	// The kinematic-state vectors are named `...Vector` to read as state and to avoid colliding with the
	// `units` dimension concepts (`units::Velocity`, `units::Acceleration`, `units::AngularVelocity`), which
	// are pulled into this namespace by the `using namespace units` in sibling headers.

	/// A velocity expressed in `Frame` (components in meters per second).
	template<class Frame>
	using VelocityVector = Vector<Frame, units::velocity::meters_per_second<>>;

	/// An acceleration expressed in `Frame` (components in meters per second squared).
	template<class Frame>
	using AccelerationVector = Vector<Frame, units::acceleration::meters_per_second_squared<>>;

	/// An angular rate expressed in `Frame` (components in radians per second). Used for body rates and for
	/// the Earth rate `omega_ie` and transport rate `omega_en`.
	template<class Frame>
	using AngularRateVector = Vector<Frame, units::angular_velocity::radians_per_second<>>;

	//======================================================
	//	EARTH ROTATION
	//======================================================

	namespace earth
	{
		/// Earth's nominal inertial rotation rate (WGS84), 7.292115e-5 rad/s.
		inline constexpr units::angular_velocity::radians_per_second<> ROTATION_RATE{7.2921150e-5};

		//------------------------------------------------------------------------------------------------------
		//	FUNCTION: angularVelocity [free]
		//------------------------------------------------------------------------------------------------------
		/// @brief		Earth's angular-velocity vector omega_ie, expressed in ECEF (and, by conversion, ECI).
		/// @details	Earth rotates about the Z axis of the Earth-fixed / inertial frames, so omega_ie is a
		///				pure +Z angular rate of magnitude `ROTATION_RATE`. Returned as an `AngularRate` in the
		///				ECEF frame of the requested horizontal datum; convert it to any other frame with the
		///				normal (rotation-only) vector conversion.
		/// @tparam		HorizontalDatum	the horizontal datum keying the ECEF frame.
		/// @return		omega_ie as an ECEF-frame `AngularRateVector`.
		//------------------------------------------------------------------------------------------------------
		template<class HorizontalDatum>
		[[nodiscard]] AngularRateVector<coordinateFrames::ECEFFrame<HorizontalDatum>> angularVelocity()
		{
			using RadPerSec = units::angular_velocity::radians_per_second<>;
			return AngularRateVector<coordinateFrames::ECEFFrame<HorizontalDatum>>(RadPerSec(0.0), RadPerSec(0.0), ROTATION_RATE);
		}
	}    // namespace earth
}    // namespace coordinates

#endif    // kinematics_h
