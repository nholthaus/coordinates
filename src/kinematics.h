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

	//======================================================
	//	ROTATING-FRAME RATE TERMS
	//======================================================
	//	The additive pieces an external integrator sums to relate rates and accelerations across the rotating
	//	Earth. This library provides the terms; it does NOT integrate them.

	//------------------------------------------------------------------------------------------------------
	//	FUNCTION: transportRate [free]
	//------------------------------------------------------------------------------------------------------
	/// @brief		The transport rate omega_en: the angular rate of the local NED frame with respect to the
	///				Earth-fixed frame, produced by moving over the curved ellipsoid.
	/// @details	In NED, `omega_en = [ vE/(N+h), -vN/(M+h), -vE*tan(phi)/(N+h) ]`, where `M` and `N` are the
	///				meridian and prime-vertical radii of curvature of the datum's ellipsoid at the geodetic
	///				latitude, `h` the geodetic height, and `(vN, vE)` the north/east ground-velocity
	///				components. Returned as an `AngularRateVector` in the NED frame anchored at `position`.
	/// @tparam		GeodeticPoint	a geodetic point type (carries latitude, height, and its datum).
	/// @param[in]	position	the geodetic position (supplies latitude, height, and the ellipsoid).
	/// @param[in]	velocityNED	the ground velocity expressed in the local NED frame.
	/// @return		omega_en as an NED-frame `AngularRateVector`.
	//------------------------------------------------------------------------------------------------------
	template<class GeodeticPoint>
	    requires(traits::is_point<GeodeticPoint>)
	[[nodiscard]] auto transportRate(const GeodeticPoint& position,
	                                 const VelocityVector<coordinateFrames::NEDFrame<typename traits::point_traits<GeodeticPoint>::reference_frame::datum_type>>& velocityNED)
	{
		using Datum      = typename traits::point_traits<GeodeticPoint>::reference_frame::datum_type;
		using Ellipsoid  = typename traits::horizontal_datum_traits<typename traits::datum_traits<Datum>::horizontal_datum>::reference_ellipsoid;
		using RadPerSec  = units::angular_velocity::radians_per_second<>;
		using meters     = units::length::meters<>;

		const double phi = units::angle::radians<>(units::angle::degrees<>(std::get<0>(position.point()))).value();
		const meters h   = std::get<2>(position.point());

		const double a  = Ellipsoid::a().value();
		const double e2 = Ellipsoid::e2();
		const double s  = std::sin(phi);
		const double w  = std::sqrt(1.0 - e2 * s * s);
		const meters N(a / w);                              // prime-vertical radius of curvature
		const meters M(a * (1.0 - e2) / (w * w * w));       // meridian radius of curvature

		const double vN = std::get<0>(velocityNED.vector()).value();
		const double vE = std::get<1>(velocityNED.vector()).value();

		const double Nh = (N + h).value();
		const double Mh = (M + h).value();

		return AngularRateVector<coordinateFrames::NEDFrame<Datum>>(
		        RadPerSec(vE / Nh),
		        RadPerSec(-vN / Mh),
		        RadPerSec(-vE * std::tan(phi) / Nh));
	}

	//------------------------------------------------------------------------------------------------------
	//	FUNCTION: coriolisAcceleration [free]
	//------------------------------------------------------------------------------------------------------
	/// @brief		The Coriolis acceleration term `2 * omega x v` for a body moving at velocity `v` in a
	///				frame rotating at angular rate `omega` (both expressed in the same frame).
	/// @details	A pure cross product scaled by two; the caller supplies the total rotation rate of the
	///				frame (e.g. `omega_ie + omega_en` for a local-level frame) and the body velocity, both in
	///				that frame. This library computes the term; the integrator sums it into the acceleration.
	/// @tparam		Frame	the common frame of the rate and the velocity.
	/// @param[in]	omega		the frame's total angular rate.
	/// @param[in]	velocity	the body's velocity in that frame.
	/// @return		the Coriolis acceleration as an `AccelerationVector` in `Frame`.
	//------------------------------------------------------------------------------------------------------
	template<class Frame>
	[[nodiscard]] AccelerationVector<Frame> coriolisAcceleration(const AngularRateVector<Frame>& omega, const VelocityVector<Frame>& velocity)
	{
		// omega (rad/s) x velocity (m/s) has units rad*m/s^2; a radian is dimensionless, so this is m/s^2.
		using mps2      = units::acceleration::meters_per_second_squared<>;
		const auto   w  = omega.vector();
		const auto   v  = velocity.vector();
		const double wx = std::get<0>(w).value(), wy = std::get<1>(w).value(), wz = std::get<2>(w).value();
		const double vx = std::get<0>(v).value(), vy = std::get<1>(v).value(), vz = std::get<2>(v).value();
		// 2 * (omega x v)
		return AccelerationVector<Frame>(mps2(2.0 * (wy * vz - wz * vy)), mps2(2.0 * (wz * vx - wx * vz)), mps2(2.0 * (wx * vy - wy * vx)));
	}
}    // namespace coordinates

#endif    // kinematics_h
