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
// The 6-DOF rigid-body state an aviation-simulation dynamics loop advances, plus the KINEMATIC evolution
// that says how it changes. `RigidBodyState` couples a `Pose` (position + attitude in a parent frame) with a
// linear velocity and a body angular rate. The derivative and the step helper implement pure kinematics --
// the translation derivative is the velocity, and the attitude derivative is the quaternion kinematic
// equation q_dot = 1/2 * q (x) omega. The DYNAMICS -- mass, the inertia tensor, forces and moments, and the
// choice of integrator -- belong to the engine above; this header provides the state container and the
// kinematic glue the engine steps.
//
// Convention (pinned): the pose's quaternion rotates BODY axes into PARENT axes; the body angular rate omega
// is expressed in BODY axes; Euler angles are intrinsic Z-Y-X (yaw-pitch-roll), matching the rotation lib.
//
//--------------------------------------------------------------------------------------------------

#ifndef rigidBody_h
#define rigidBody_h

//------------------------
//	INCLUDES
//------------------------

#include <units.h>

#include "frameOfReference.h"
#include "kinematics.h"
#include "pose.h"
#include "quaternion.h"
#include "vector.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	STRUCT		QuaternionDerivative
	//  ----------------------------------------------------------------------------
	///	@brief		The time-derivative of a unit quaternion (per second), four dimensionless-per-second rates.
	///	@details	Not itself a unit quaternion -- it is `d/dt` of one, so it is a free 4-tuple of rates. Kept
	///				as a distinct type so it can never be mistaken for an orientation.
	//  ----------------------------------------------------------------------------
	struct QuaternionDerivative
	{
		units::frequency::hertz<> w{0.0};    ///< d(scalar)/dt
		units::frequency::hertz<> x{0.0};    ///< d(i)/dt
		units::frequency::hertz<> y{0.0};    ///< d(j)/dt
		units::frequency::hertz<> z{0.0};    ///< d(k)/dt
	};

	//------------------------------------------------------------------------------------------------------
	//	FUNCTION: attitudeDerivative [free]
	//------------------------------------------------------------------------------------------------------
	/// @brief		The quaternion kinematic derivative q_dot = 1/2 * q (x) omega for a body-to-parent
	///				attitude `q` spinning at body angular rate `omega`.
	/// @details	`omega` is expressed in BODY axes and promoted to the pure quaternion (0, wx, wy, wz); the
	///				product is the body-frame kinematic form `q_dot = 1/2 * q (x) omega_quat`. Integrating this
	///				(with periodic renormalization) propagates attitude. Rates are radians/second; the result
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
		using Hz = units::frequency::hertz<>;
		return QuaternionDerivative{
		        Hz(0.5 * (-qx * wx - qy * wy - qz * wz)),
		        Hz(0.5 * (qw * wx + qy * wz - qz * wy)),
		        Hz(0.5 * (qw * wy - qx * wz + qz * wx)),
		        Hz(0.5 * (qw * wz + qx * wy - qy * wx))};
	}

	//	----------------------------------------------------------------------------
	//	CLASS		RigidBodyState
	//  ----------------------------------------------------------------------------
	///	@brief		The 6-DOF state of a rigid body: pose (position + attitude) plus linear velocity and body
	///				angular rate. The state a dynamics integrator reads and writes.
	///	@details	The pose places the body in `Frame` (its quaternion rotates body axes into `Frame` axes).
	///				The linear velocity is expressed in `Frame`; the angular rate is expressed in BODY axes.
	///				This container carries no forces, mass, or inertia -- those are the engine's; it holds the
	///				kinematic state and, with the free derivative/step functions, the kinematic evolution.
	///	@tparam		Frame	the parent frame the body's position and linear velocity are expressed in.
	//  ----------------------------------------------------------------------------
	template<class Frame>
	class RigidBodyState
	{
	public:
		using frame_type    = Frame;
		using velocity_type = VelocityVector<Frame>;
		using rate_type     = AngularRateVector<Frame>;

		//----------------------------------
		//	CONSTRUCTORS
		//----------------------------------

		RigidBodyState() = default;

		/// From a pose, a linear velocity (in `Frame`), and a body angular rate.
		RigidBodyState(const Pose& pose, const velocity_type& velocity, const rate_type& bodyRate)
		    : m_pose(pose)
		    , m_velocity(velocity)
		    , m_bodyRate(bodyRate)
		{
		}

		//----------------------------------
		//	GETTERS
		//----------------------------------

		[[nodiscard]] const Pose&          pose() const { return m_pose; }              ///< position + attitude in `Frame`
		[[nodiscard]] CartesianTuple       position() const { return m_pose.translation(); }
		[[nodiscard]] rotation::Quaternion attitude() const { return m_pose.rotation(); }
		[[nodiscard]] const velocity_type& velocity() const { return m_velocity; }      ///< linear velocity in `Frame`
		[[nodiscard]] const rate_type&     bodyRate() const { return m_bodyRate; }       ///< angular rate in BODY axes

		//----------------------------------
		//	SETTERS
		//----------------------------------

		void setPose(const Pose& pose) { m_pose = pose; }
		void setVelocity(const velocity_type& velocity) { m_velocity = velocity; }
		void setBodyRate(const rate_type& bodyRate) { m_bodyRate = bodyRate; }

	private:
		Pose          m_pose{Pose::identity()};    ///< body placement (position + attitude) in `Frame`
		velocity_type m_velocity{};                ///< linear velocity, expressed in `Frame`
		rate_type     m_bodyRate{};                ///< angular rate, expressed in BODY axes
	};

	//------------------------------------------------------------------------------------------------------
	//	FUNCTION: integrateKinematics [free]
	//------------------------------------------------------------------------------------------------------
	/// @brief		Advance a rigid-body state by `dt` under the KINEMATIC equations, given the engine-supplied
	///				linear and angular accelerations. Explicit Euler with attitude renormalization.
	/// @details	Steps `position += velocity*dt`, `velocity += linearAccel*dt`, `q += q_dot*dt` (then
	///				renormalize), `bodyRate += angularAccel*dt`, where `q_dot = attitudeDerivative(q,
	///				bodyRate)`. This is the SIMPLEST integrator, offered as a convenience -- an engine wanting
	///				RK4 or a fixed-step scheme composes `attitudeDerivative` and the trivial linear derivatives
	///				itself. The library does not mandate a scheme or own the time loop.
	/// @tparam		Frame	the parent frame of the state.
	/// @param[in]	state			the current state.
	/// @param[in]	linearAccel		the linear acceleration in `Frame` (from the engine's force model).
	/// @param[in]	angularAccel	the angular acceleration in BODY axes (from the engine's moment model).
	/// @param[in]	dt				the time step.
	/// @return		the advanced state.
	//------------------------------------------------------------------------------------------------------
	template<class Frame>
	[[nodiscard]] RigidBodyState<Frame> integrateKinematics(const RigidBodyState<Frame>& state,
	                                                        const AccelerationVector<Frame>& linearAccel,
	                                                        const AngularRateVector<Frame>&  angularAccelBody,
	                                                        units::time::seconds<>           dt)
	{
		const double dts = dt.value();

		// --- translation: position += velocity * dt (velocity in Frame) ---
		const auto           v = state.velocity().vector();
		const CartesianTuple pos = state.position();
		const CartesianTuple newPos(std::get<0>(pos) + units::length::meters<>(std::get<0>(v).value() * dts),
		                            std::get<1>(pos) + units::length::meters<>(std::get<1>(v).value() * dts),
		                            std::get<2>(pos) + units::length::meters<>(std::get<2>(v).value() * dts));

		// --- attitude: q += q_dot * dt, renormalize (q_dot from body rate) ---
		const rotation::Quaternion   q  = state.attitude();
		const QuaternionDerivative   qd = attitudeDerivative(q, state.bodyRate());
		const rotation::Quaternion   qNew = rotation::Quaternion(units::dimensionless<>(q.w().value() + qd.w.value() * dts),
		                                                         units::dimensionless<>(q.x().value() + qd.x.value() * dts),
		                                                         units::dimensionless<>(q.y().value() + qd.y.value() * dts),
		                                                         units::dimensionless<>(q.z().value() + qd.z.value() * dts))
		                                        .normalized();

		// --- linear velocity: v += a * dt (both in Frame) ---
		using mps      = units::velocity::meters_per_second<>;
		const auto la  = linearAccel.vector();
		VelocityVector<Frame> newVel(mps(std::get<0>(v).value() + std::get<0>(la).value() * dts),
		                             mps(std::get<1>(v).value() + std::get<1>(la).value() * dts),
		                             mps(std::get<2>(v).value() + std::get<2>(la).value() * dts));

		// --- body rate: omega += alpha * dt (both in body axes; alpha carried as a rate delta per second) ---
		using rps      = units::angular_velocity::radians_per_second<>;
		const auto w   = state.bodyRate().vector();
		const auto al  = angularAccelBody.vector();
		AngularRateVector<Frame> newRate(rps(std::get<0>(w).value() + std::get<0>(al).value() * dts),
		                                 rps(std::get<1>(w).value() + std::get<1>(al).value() * dts),
		                                 rps(std::get<2>(w).value() + std::get<2>(al).value() * dts));

		return RigidBodyState<Frame>(Pose(newPos, qNew), newVel, newRate);
	}
}    // namespace coordinates

#endif    // rigidBody_h
