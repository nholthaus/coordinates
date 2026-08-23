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
// The one composable thing above a `Coordinate`: an `Entity`. A point is first-class; an `Entity` is a point
// that MAY also carry an orientation (a pose), motion, a field of view, and child entities. Everything is
// optional and either set at construction or with a plain `set*` afterward. Attitude is relational, so an
// entity's pose is expressed relative to its parent; attaching children builds a rigid tree (the entity tree
// IS the rigid body) whose child transforms compose up to the world. A top-level entity is world-posed, and a
// child reports its position and pose in WORLD coordinates by default -- resolved by walking the parent chain
// -- with an overload to report relative to any other entity instead. The heavier verbs (rays, visibility,
// propagation) are layered on in later steps; this is the composable state + frame resolution.
//
//--------------------------------------------------------------------------------------------------

#ifndef entity_h
#define entity_h

//------------------------
//	INCLUDES
//------------------------

#include <memory>
#include <vector>

#include <units.h>

#include <optional>

#include "algorithm.h"
#include "coordinates_fwd.h"
#include "fieldOfView.h"
#include "frameOfReference.h"
#include "kinematics.h"
#include "pose.h"
#include "positionECEF.h"
#include "ray.h"

#if defined(COORDINATES_ENABLE_LOS) && COORDINATES_ENABLE_LOS
#include "lineOfSight.h"
#endif

inline namespace coordinates
{
	using namespace units;
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	STRUCT		QuaternionDerivative
	//  ----------------------------------------------------------------------------
	///	@brief		The time-derivative of a unit quaternion (per second): four dimensionless-per-second rates.
	///	@details	Not itself a unit quaternion -- it is `d/dt` of one, a free 4-tuple of rates. A distinct type
	///				so it is never mistaken for an orientation. The kinematic primitive `Entity::propagate` steps.
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
	/// @details	`omega` is in BODY axes, promoted to the pure quaternion (0, wx, wy, wz); the product is the
	///				body-frame form q_dot = 1/2 q (x) omega_quat. Integrating it (with renormalization) propagates
	///				orientation. Rates are radians/second; the result is a per-second quaternion derivative.
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

		// q (x) (0, wx, wy, wz), halved (Hamilton product).
		return QuaternionDerivative{
		        hertz<>(0.5 * (-qx * wx - qy * wy - qz * wz)),
		        hertz<>(0.5 * (qw * wx + qy * wz - qz * wy)),
		        hertz<>(0.5 * (qw * wy - qx * wz + qz * wx)),
		        hertz<>(0.5 * (qw * wz + qx * wy - qy * wx))};
	}

	//	----------------------------------------------------------------------------
	//	CLASS		Entity
	//  ----------------------------------------------------------------------------
	///	@brief		A composable point that may carry a pose, motion, a field of view, and child entities.
	///	@details	`Entity` is the single first-class thing above `Coordinate`. Its position is a
	///				`PositionECEF<Datum>` (the world/rigid-body frame); its pose is an orientation (with a mount
	///				offset) relative to its parent; children attach rigidly to form a tree that moves as one -- the
	///				entity tree is the rigid body. A top-level entity is world-posed; a child resolves its position
	///				and pose to WORLD by composing rigid transforms up the parent chain, and an overload reports
	///				relative to another entity instead. All state is optional and set at construction or via `set*`.
	///	@tparam		Datum	the geodetic datum whose ECEF frame the entity's position and motion live in.
	//  ----------------------------------------------------------------------------
	template<class Datum>
	class Entity
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using datum_type    = Datum;
		using position_type = PositionECEF<Datum>;
		using frame_type    = coordinateFrames::ECEFFrame<typename traits::datum_traits<Datum>::horizontal_datum>;
		using velocity_type = VelocityVector<frame_type>;
		using rate_type     = AngularRateVector<frame_type>;

		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/// An empty entity (no position, identity pose, no motion). Fill it with `set*`.
		constexpr Entity() = default;

		/// The natural-use-case constructor: a position, and optionally an orientation and motion. Every case is
		/// a prefix of the argument list -- a bare point `Entity{p}`, a posed thing `Entity{p, pose}`, or a moving
		/// body `Entity{p, pose, velocity, rate}`. Field of view is set separately (`setFieldOfView`), added in a
		/// later step, since it is the least common field.
		explicit constexpr Entity(const position_type& position, const Pose& pose = Pose::identity(), const velocity_type& velocity = velocity_type{}, const rate_type& angularRate = rate_type{})
		    : m_position(position)
		    , m_pose(pose)
		    , m_velocity(velocity)
		    , m_angularRate(angularRate)
		{
		}

		/// Deep copy: clones the whole child subtree and re-points each clone's parent to this new tree, so a
		/// copied entity is a fully independent rigid body (no child dangles back to the original). The `m_parent`
		/// of the copy itself is left null -- a copy is a fresh root until re-attached.
		constexpr Entity(const Entity& other)
		    : m_position(other.m_position)
		    , m_pose(other.m_pose)
		    , m_fieldOfView(other.m_fieldOfView)
		    , m_velocity(other.m_velocity)
		    , m_angularRate(other.m_angularRate)
		{
			cloneChildrenFrom(other);
		}

		/// Deep copy-assignment (copy-and-swap-free: clear, then clone), keeping this entity a fresh root.
		constexpr Entity& operator=(const Entity& other)
		{
			if (this != &other)
			{
				m_position    = other.m_position;
				m_pose        = other.m_pose;
				m_fieldOfView = other.m_fieldOfView;
				m_velocity    = other.m_velocity;
				m_angularRate = other.m_angularRate;
				m_children.clear();
				cloneChildrenFrom(other);
			}
			return *this;
		}

		/// Move: steals the child subtree and re-points the top-level children's parent to this entity.
		constexpr Entity(Entity&& other) noexcept
		    : m_position(other.m_position)
		    , m_pose(other.m_pose)
		    , m_fieldOfView(std::move(other.m_fieldOfView))
		    , m_velocity(other.m_velocity)
		    , m_angularRate(other.m_angularRate)
		    , m_children(std::move(other.m_children))
		{
			for (auto& child : m_children)
				child->m_parent = this;
		}

		/// Move-assignment (steal + re-point the top-level children's parent).
		constexpr Entity& operator=(Entity&& other) noexcept
		{
			if (this != &other)
			{
				m_position    = other.m_position;
				m_pose        = other.m_pose;
				m_fieldOfView = std::move(other.m_fieldOfView);
				m_velocity    = other.m_velocity;
				m_angularRate = other.m_angularRate;
				m_children    = std::move(other.m_children);
				for (auto& child : m_children)
					child->m_parent = this;
			}
			return *this;
		}

		constexpr ~Entity() = default;

		//////////////////////////////////////////////////////////////////////////
		//		SETTERS
		//////////////////////////////////////////////////////////////////////////

		constexpr void setPosition(const position_type& position) { m_position = position; }        ///< the entity's location
		constexpr void setPose(const Pose& pose) { m_pose = pose; }                                  ///< orientation relative to the parent
		constexpr void setVelocity(const velocity_type& velocity) { m_velocity = velocity; }         ///< linear velocity in the ECEF frame
		constexpr void setAngularRate(const rate_type& angularRate) { m_angularRate = angularRate; } ///< body angular rate

		//////////////////////////////////////////////////////////////////////////
		//		GETTERS (world-resolved by default)
		//////////////////////////////////////////////////////////////////////////

		/// The entity's position in WORLD coordinates (a root's own position; a child resolved up the chain).
		[[nodiscard]] constexpr position_type position() const
		{
			if (m_parent == nullptr)
				return m_position;
			position_type worldPosition;
			worldPosition.setPoint(m_parent->pose().transformPoint(localPose().translation()));
			return worldPosition;
		}

		/// The entity's position expressed relative to another entity's frame.
		[[nodiscard]] constexpr position_type position(const Entity& relativeTo) const
		{
			position_type result;
			result.setPoint(relativeTo.pose().inverse().transformPoint(position().point()));
			return result;
		}

		/// The entity's pose in WORLD coordinates: a root's own pose, or a child's local pose composed up the
		/// parent chain (`worldFromParent * parentFromChild`).
		[[nodiscard]] constexpr Pose pose() const
		{
			if (m_parent == nullptr)
				return localPose();
			return m_parent->pose() * localPose();
		}

		/// The entity's pose expressed relative to another entity's frame.
		[[nodiscard]] constexpr Pose pose(const Entity& relativeTo) const { return relativeTo.pose().inverse() * pose(); }

		[[nodiscard]] constexpr const velocity_type& velocity() const { return m_velocity; }       ///< linear velocity (ECEF frame)
		[[nodiscard]] constexpr const rate_type&     angularRate() const { return m_angularRate; } ///< body angular rate

		//////////////////////////////////////////////////////////////////////////
		//		CHILDREN (rigid attachment -- the entity tree is the rigid body)
		//////////////////////////////////////////////////////////////////////////

		//	----------------------------------------------------------------------------
		//	STRUCT		Mount
		//  ----------------------------------------------------------------------------
		///	@brief		A child's rigid placement on its parent: an offset from the parent origin plus an
		///				orientation, both in the parent's body axes.
		///	@details	Implicitly constructs from a bare offset (orientation defaults to identity), so a child at a
		///				pure translation can be attached as `attach(offset)` without spelling out a `Mount`.
		//  ----------------------------------------------------------------------------
		struct Mount
		{
			CartesianTuple offset{0.0_m, 0.0_m, 0.0_m};    ///< the child origin in the parent's body axes
			Pose           orientation{Pose::identity()};  ///< the child's rotation relative to the parent's axes

			constexpr Mount() = default;
			constexpr Mount(CartesianTuple offsetIn, Pose orientationIn = Pose::identity()) : offset(offsetIn), orientation(orientationIn) {}
		};

		/// Attach a child entity rigidly at a mount; the parent OWNS the child and returns a reference to it for
		/// further configuration. The child's local pose is the mount (offset + orientation) relative to this
		/// entity, so the child resolves to world through this parent.
		constexpr Entity& attach(const Mount& mount)
		{
			auto child      = std::make_unique<Entity>();
			child->m_parent = this;
			child->m_pose   = Pose(mount.offset, mount.orientation.rotation());
			Entity& ref     = *child;
			m_children.push_back(std::move(child));
			return ref;
		}

		/// Attach a whole sequence of body-axis points as child entities, in order, each at its offset (identity
		/// orientation). Returns the child pointers in the same order, so a caller can treat them as an ordered
		/// polyline (e.g. an outline) that resolves to world rigidly through this parent's pose.
		constexpr std::vector<Entity*> attach(const CartesianVector& offsets)
		{
			std::vector<Entity*> children;
			children.reserve(offsets.size());
			for (const CartesianTuple& offset : offsets)
				children.push_back(&attach(Mount(offset)));
			return children;
		}

		[[nodiscard]] constexpr const std::vector<std::unique_ptr<Entity>>& children() const { return m_children; }
		[[nodiscard]] constexpr const Entity*                               parent() const { return m_parent; }

		//////////////////////////////////////////////////////////////////////////
		//		FIELD OF VIEW (optional)
		//////////////////////////////////////////////////////////////////////////

		constexpr void                              setFieldOfView(const FieldOfView& fieldOfView) { m_fieldOfView = fieldOfView; }
		[[nodiscard]] constexpr std::optional<FieldOfView> fieldOfView() const { return m_fieldOfView; }

		//////////////////////////////////////////////////////////////////////////
		//		RAYS & VISIBILITY (derived; minimal inputs)
		//////////////////////////////////////////////////////////////////////////

		/// The boresight ray the entity looks along -- its world pose's forward (+x) axis from its world position.
		/// Takes nothing: the entity already knows where it is and where it points.
		[[nodiscard]] Ray<frame_type> ray() const { return Ray<frame_type>::fromPose(pose(), CartesianTuple(1.0_m, 0.0_m, 0.0_m)); }

		/// The boresight ray expressed in another entity's frame.
		[[nodiscard]] Ray<frame_type> ray(const Entity& relativeTo) const
		{
			return Ray<frame_type>::fromPose(pose(relativeTo), CartesianTuple(1.0_m, 0.0_m, 0.0_m));
		}

		/// True when this entity can see `other`: `other` has line of sight (ellipsoid, and terrain when the
		/// line-of-sight support is built in) AND -- if this entity has a field of view -- lies within it. With
		/// no field of view the entity is omnidirectional, so `sees` is pure line of sight.
		[[nodiscard]] bool sees(const Entity& other) const
		{
			if (m_fieldOfView.has_value() && !m_fieldOfView->contains(pose(), other.position().point()))
				return false;

			const position_type here  = position();
			const position_type there = other.position();
#if defined(COORDINATES_ENABLE_LOS) && COORDINATES_ENABLE_LOS
			const LineOfSight<Datum> los{PositionGeodetic<Datum>(here)};
			return los.lineOfSightTerrain(PositionGeodetic<Datum>(there));
#else
			return isLineOfSight(here, there);
#endif
		}

		//////////////////////////////////////////////////////////////////////////
		//		MOTION
		//////////////////////////////////////////////////////////////////////////

		/// Advance the entity by `dt` under its own KINEMATICS: the position steps by the linear velocity, and
		/// the orientation by the quaternion kinematic equation q_dot = 1/2 q (x) omega (renormalized). Children
		/// are fixed relative to this entity, so the whole attached assembly moves rigidly with it -- no child
		/// update needed. This is pure kinematics; a SIM layer computes the velocity/rate that drive it.
		void propagate(seconds<> dt)
		{
			const double dts = dt.value();

			const auto v = m_velocity.vector();
			m_position.setPoint(std::get<0>(m_position.point()) + meters<>(std::get<0>(v).value() * dts),
			                    std::get<1>(m_position.point()) + meters<>(std::get<1>(v).value() * dts),
			                    std::get<2>(m_position.point()) + meters<>(std::get<2>(v).value() * dts));

			const rotation::Quaternion q  = m_pose.rotation();
			const QuaternionDerivative qd = attitudeDerivative(q, m_angularRate);
			const rotation::Quaternion advanced = rotation::Quaternion(dimensionless<>(q.w().value() + qd.w.value() * dts),
			                                                           dimensionless<>(q.x().value() + qd.x.value() * dts),
			                                                           dimensionless<>(q.y().value() + qd.y.value() * dts),
			                                                           dimensionless<>(q.z().value() + qd.z.value() * dts))
			                                          .normalized();
			m_pose = Pose(m_pose.translation(), advanced);
		}

	private:
		//	----------------------------------------------------------------------------
		//	FUNCTION: localPose [private]
		//  ----------------------------------------------------------------------------
		///	@brief		The entity's pose in its PARENT's frame: its orientation with the position as translation.
		///	@details	For a root, the translation is the entity's world ECEF position, so the local pose IS the
		///				world pose. For a child, the translation is the mount offset (the pose set at `attach`), so
		///				composing with the parent's world pose places the child.
		///	@return		the pose relative to the parent (or world, for a root).
		//  ----------------------------------------------------------------------------
		[[nodiscard]] constexpr Pose localPose() const
		{
			if (m_parent == nullptr)
				return Pose(m_position.point(), m_pose.rotation());
			return m_pose;
		}

		//	----------------------------------------------------------------------------
		//	FUNCTION: cloneChildrenFrom [private]
		//  ----------------------------------------------------------------------------
		///	@brief		Deep-clone another entity's child subtree into this entity, re-pointing every clone's parent.
		///	@details	Each source child is cloned (which recurses through its own subtree), then its parent is set
		///				to this entity, so the copied tree is fully self-consistent and independent of the source.
		///	@param[in]	other	the entity whose children to clone.
		//  ----------------------------------------------------------------------------
		constexpr void cloneChildrenFrom(const Entity& other)
		{
			m_children.reserve(other.m_children.size());
			for (const auto& child : other.m_children)
			{
				auto copy      = std::make_unique<Entity>(*child);    // recursive deep copy of the subtree
				copy->m_parent = this;
				m_children.push_back(std::move(copy));
			}
		}

		position_type                        m_position;                    ///< location (world for a root, unused for a child)
		Pose                                 m_pose{Pose::identity()};      ///< orientation; for a child, its mount pose in the parent
		std::optional<FieldOfView>           m_fieldOfView;                 ///< the entity's view cone (none = omnidirectional)
		velocity_type                        m_velocity{};                  ///< linear velocity in the ECEF frame
		rate_type                            m_angularRate{};               ///< body angular rate
		const Entity*                        m_parent{nullptr};             ///< non-owning; a parent outlives the children it owns
		std::vector<std::unique_ptr<Entity>> m_children;                    ///< owned child entities (the rigid tree)
	};
}    // namespace coordinates

#endif    // entity_h
