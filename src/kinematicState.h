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
// The instantaneous state of a moving body -- where it is, how it is oriented, and how both are changing --
// plus the KINEMATIC evolution that advances it. `KinematicState` couples a `Pose` (position + orientation in
// a parent frame) with a linear velocity and a body angular rate. The derivative and step helpers implement
// pure kinematics: the translation derivative is the velocity, and the orientation derivative is the
// quaternion kinematic equation q_dot = 1/2 * q (x) omega. The DYNAMICS -- mass, the inertia tensor, forces
// and moments, and the choice of integrator -- belong to the engine above; this header provides the state
// container and the kinematic glue the engine steps.
//
// Convention (pinned): the pose's quaternion rotates BODY axes into PARENT axes; the body angular rate omega
// is expressed in BODY axes; Euler angles are intrinsic Z-Y-X (yaw-pitch-roll), matching the rotation lib.
//
//--------------------------------------------------------------------------------------------------

#ifndef kinematicState_h
#define kinematicState_h

//------------------------
//	INCLUDES
//------------------------

#include <units.h>

#include "frameOfReference.h"
#include "kinematics.h"
#include "pose.h"
#include "quaternion.h"
#include "ray.h"
#include "vector.h"

inline namespace coordinates
{
	using namespace units;
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	STRUCT		QuaternionDerivative
	//  ----------------------------------------------------------------------------
	///	@brief		The time-derivative of a unit quaternion (per second), four dimensionless-per-second rates.
	///	@details	Not itself a unit quaternion -- it is `d/dt` of one, so it is a free 4-tuple of rates. Kept
	///				as a distinct type so it can never be mistaken for an orientation.
	//  ----------------------------------------------------------------------------
	struct QuaternionDerivative
	{
		hertz<> w{0.0};    ///< d(scalar)/dt
		hertz<> x{0.0};    ///< d(i)/dt
		hertz<> y{0.0};    ///< d(j)/dt
		hertz<> z{0.0};    ///< d(k)/dt
	};

	//------------------------------------------------------------------------------------------------------
	//	FUNCTION: attitudeDerivative [free]
	//------------------------------------------------------------------------------------------------------
	/// @brief		The quaternion kinematic derivative q_dot = 1/2 * q (x) omega for a body-to-parent
	///				orientation `q` spinning at body angular rate `omega`.
	/// @details	`omega` is expressed in BODY axes and promoted to the pure quaternion (0, wx, wy, wz); the
	///				product is the body-frame kinematic form `q_dot = 1/2 * q (x) omega_quat`. Integrating this
	///				(with periodic renormalization) propagates orientation. Rates are radians/second; the result
	///				is a per-second quaternion derivative.
	/// @tparam		Frame	the body frame the angular rate is expressed in.
	/// @param[in]	q		the current body-to-parent orientation.
	/// @param[in]	omega	the body angular rate.
	/// @return		the quaternion time-derivative.
	//------------------------------------------------------------------------------------------------------
	template<class Frame>
	[[nodiscard]] constexpr QuaternionDerivative attitudeDerivative(const rotation::Quaternion& q, const AngularRateVector<Frame>& omega)
	{
		const double wx = std::get<0>(omega.vector()).value();
		const double wy = std::get<1>(omega.vector()).value();
		const double wz = std::get<2>(omega.vector()).value();

		const double qw = q.w().value(), qx = q.x().value(), qy = q.y().value(), qz = q.z().value();

		// q (x) (0, wx, wy, wz), then halved (Hamilton product).
		using Hz = hertz<>;
		return QuaternionDerivative{
		        Hz(0.5 * (-qx * wx - qy * wy - qz * wz)),
		        Hz(0.5 * (qw * wx + qy * wz - qz * wy)),
		        Hz(0.5 * (qw * wy - qx * wz + qz * wx)),
		        Hz(0.5 * (qw * wz + qx * wy - qy * wx))};
	}

	//	----------------------------------------------------------------------------
	//	CLASS		KinematicState
	//  ----------------------------------------------------------------------------
	///	@brief		The instantaneous state of motion of a body: its pose (position + orientation) plus linear
	///				velocity and body angular rate. The state a dynamics integrator reads and writes.
	///	@details	The pose places the body in `Frame` (its quaternion rotates body axes into `Frame` axes).
	///				The linear velocity is expressed in `Frame`; the angular rate is expressed in BODY axes.
	///				This container carries no forces, mass, or inertia -- those are the engine's; it holds the
	///				kinematic state and, with the free derivative/step functions, the kinematic evolution.
	///	@tparam		Frame	the parent frame the body's position and linear velocity are expressed in.
	//  ----------------------------------------------------------------------------
	template<class Frame>
	class KinematicState
	{
	public:
		using frame_type    = Frame;
		using velocity_type = VelocityVector<Frame>;
		using rate_type     = AngularRateVector<Frame>;

		//----------------------------------
		//	CONSTRUCTORS
		//----------------------------------

		KinematicState() = default;

		/// From a pose, a linear velocity (in `Frame`), and a body angular rate.
		KinematicState(const Pose& pose, const velocity_type& velocity, const rate_type& bodyRate)
		    : m_pose(pose)
		    , m_velocity(velocity)
		    , m_bodyRate(bodyRate)
		{
		}

		//----------------------------------
		//	GETTERS
		//----------------------------------

		[[nodiscard]] const Pose&          pose() const { return m_pose; }              ///< position + orientation in `Frame`
		[[nodiscard]] CartesianTuple       position() const { return m_pose.translation(); }
		[[nodiscard]] rotation::Quaternion attitude() const { return m_pose.rotation(); }
		[[nodiscard]] const velocity_type& velocity() const { return m_velocity; }      ///< linear velocity in `Frame`
		[[nodiscard]] const rate_type&     bodyRate() const { return m_bodyRate; }       ///< angular rate in BODY axes

		//----------------------------------
		//	RAYS
		//----------------------------------

		/// The boresight ray: from the body's position, straight down its forward (+x body) axis in `Frame`.
		[[nodiscard]] Ray<Frame> ray() const { return ray(0.0_deg, 0.0_deg); }

		/// A ray steered off the boresight by delta-azimuth (about the body's down/+z axis, +right) and
		/// delta-elevation (about the body's right/+y axis, +down), from the body's position. This is the
		/// "sensor looking down-and-right of the nose" pointing: the deltas are BODY-relative, off the forward axis.
		[[nodiscard]] Ray<Frame> ray(degrees<> deltaAzimuth, degrees<> deltaElevation) const
		{
			return steeredRay(m_pose, deltaAzimuth, deltaElevation);
		}

		/// A mounted sensor's ray: the mount is a `Pose` (its offset from the body origin + its rotation relative
		/// to body axes); the sensor pose is `pose() * mount`, and the ray is steered off ITS boresight by the
		/// deltas. Carries the body's position and attitude AND the mount, so nothing but the deltas is passed.
		[[nodiscard]] Ray<Frame> ray(const Pose& mount, degrees<> deltaAzimuth = 0.0_deg, degrees<> deltaElevation = 0.0_deg) const
		{
			return steeredRay(m_pose * mount, deltaAzimuth, deltaElevation);
		}

		//----------------------------------
		//	SETTERS
		//----------------------------------

		void setPose(const Pose& pose) { m_pose = pose; }
		void setVelocity(const velocity_type& velocity) { m_velocity = velocity; }
		void setBodyRate(const rate_type& bodyRate) { m_bodyRate = bodyRate; }

	private:
		//	----------------------------------------------------------------------------
		//	FUNCTION: steeredRay [static, private]
		//  ----------------------------------------------------------------------------
		///	@brief		Build the ray from a sensor pose, steered off its forward (+x) boresight by the deltas.
		///	@details	The delta-azimuth rotates about the body down (+z) axis (+ toward the right), the
		///				delta-elevation about the body right (+y) axis (+ downward); the resulting body-axis
		///				direction is carried into `Frame` by the pose. Zero deltas give the pure boresight.
		///	@param[in]	sensorPose	the sensor's pose in `Frame` (body pose, optionally composed with a mount).
		///	@param[in]	deltaAzimuth	the azimuth offset off boresight (body axes).
		///	@param[in]	deltaElevation	the elevation offset off boresight (body axes).
		///	@return		the steered ray in `Frame`.
		//  ----------------------------------------------------------------------------
		static Ray<Frame> steeredRay(const Pose& sensorPose, degrees<> deltaAzimuth, degrees<> deltaElevation)
		{
			// Forward (+x) boresight rotated within body axes: yaw by deltaAzimuth about +z, pitch by
			// deltaElevation about +y. cos(el) forward, sin(daz) right, sin(el) down.
			const double az = radians<>(deltaAzimuth).value();
			const double el = radians<>(deltaElevation).value();
			const CartesianTuple bodyDirection(meters<>(std::cos(el) * std::cos(az)),    // x forward
			                                   meters<>(std::cos(el) * std::sin(az)),    // y right
			                                   meters<>(std::sin(el)));                  // z down
			return Ray<Frame>::fromPose(sensorPose, bodyDirection);
		}

		Pose          m_pose{Pose::identity()};    ///< body placement (position + orientation) in `Frame`
		velocity_type m_velocity{};                ///< linear velocity, expressed in `Frame`
		rate_type     m_bodyRate{};                ///< angular rate, expressed in BODY axes
	};

	//------------------------------------------------------------------------------------------------------
	//	FUNCTION: integrateKinematics [free]
	//------------------------------------------------------------------------------------------------------
	/// @brief		Advance a kinematic state by `dt` under the KINEMATIC equations, given the engine-supplied
	///				linear and angular accelerations. Explicit Euler with orientation renormalization.
	/// @details	Steps `position += velocity*dt`, `velocity += linearAccel*dt`, `q += q_dot*dt` (then
	///				renormalize), `bodyRate += angularAccel*dt`, where `q_dot = attitudeDerivative(q,
	///				bodyRate)`. This is the SIMPLEST integrator, offered as a convenience -- an engine wanting
	///				RK4 or a fixed-step scheme composes `attitudeDerivative` and the trivial linear derivatives
	///				itself. The library does not mandate a scheme or own the time loop.
	/// @tparam		Frame	the parent frame of the state.
	/// @param[in]	state			the current state.
	/// @param[in]	linearAccel		the linear acceleration in `Frame` (from the engine's force model).
	/// @param[in]	angularAccelBody	the angular acceleration in BODY axes (from the engine's moment model).
	/// @param[in]	dt				the time step.
	/// @return		the advanced state.
	//------------------------------------------------------------------------------------------------------
	template<class Frame>
	[[nodiscard]] KinematicState<Frame> integrateKinematics(const KinematicState<Frame>&    state,
	                                                        const AccelerationVector<Frame>& linearAccel,
	                                                        const AngularRateVector<Frame>&  angularAccelBody,
	                                                        seconds<>           dt)
	{
		const double dts = dt.value();

		// --- translation: position += velocity * dt (velocity in Frame) ---
		const auto           v = state.velocity().vector();
		const CartesianTuple pos = state.position();
		const CartesianTuple newPos(std::get<0>(pos) + meters<>(std::get<0>(v).value() * dts),
		                            std::get<1>(pos) + meters<>(std::get<1>(v).value() * dts),
		                            std::get<2>(pos) + meters<>(std::get<2>(v).value() * dts));

		// --- orientation: q += q_dot * dt, renormalize (q_dot from body rate) ---
		const rotation::Quaternion   q  = state.attitude();
		const QuaternionDerivative   qd = attitudeDerivative(q, state.bodyRate());
		const rotation::Quaternion   qNew = rotation::Quaternion(dimensionless<>(q.w().value() + qd.w.value() * dts),
		                                                         dimensionless<>(q.x().value() + qd.x.value() * dts),
		                                                         dimensionless<>(q.y().value() + qd.y.value() * dts),
		                                                         dimensionless<>(q.z().value() + qd.z.value() * dts))
		                                        .normalized();

		// --- linear velocity: v += a * dt (both in Frame) ---
		using mps      = meters_per_second<>;
		const auto la  = linearAccel.vector();
		VelocityVector<Frame> newVel(mps(std::get<0>(v).value() + std::get<0>(la).value() * dts),
		                             mps(std::get<1>(v).value() + std::get<1>(la).value() * dts),
		                             mps(std::get<2>(v).value() + std::get<2>(la).value() * dts));

		// --- body rate: omega += alpha * dt (both in body axes; alpha carried as a rate delta per second) ---
		using rps      = radians_per_second<>;
		const auto w   = state.bodyRate().vector();
		const auto al  = angularAccelBody.vector();
		AngularRateVector<Frame> newRate(rps(std::get<0>(w).value() + std::get<0>(al).value() * dts),
		                                 rps(std::get<1>(w).value() + std::get<1>(al).value() * dts),
		                                 rps(std::get<2>(w).value() + std::get<2>(al).value() * dts));

		return KinematicState<Frame>(Pose(newPos, qNew), newVel, newRate);
	}
}    // namespace coordinates

#endif    // kinematicState_h
