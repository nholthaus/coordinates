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
/// @brief      Definition of the `Pose` class -- a runtime 6-DOF rigid transform (position + orientation).
/// @details    Where `BodyFrame` fixes a rigid transform in the frame TYPE (compile-time, for a static
///             mounting), `Pose` carries a rigid transform as a runtime VALUE, for a body whose position
///             and attitude vary over time (a moving vehicle, a slewing sensor). A `Pose` is a translation
///             (a Cartesian offset) plus a rotation (a `Quaternion`); poses compose so that a chain of
///             bodies -- camera in wingtip, wingtip in aircraft, aircraft in world -- is built by
///             multiplying their poses. `transformPoint` maps a point from the pose's local frame into its
///             parent frame (rotate then translate); `inverse` gives the parent-to-local pose. Every
///             operation is `constexpr`-capable.
//
// ---------------------------------------------------------------------------------------------------------------------

#ifndef pose_h
#define pose_h

//------------------------
//	INCLUDES
//------------------------

#include <tuple>

#include <units.h>

#include "frameOfReference.h"
#include "quaternion.h"
#include "rotation.h"

inline namespace coordinates
{
	using namespace units;

	//	----------------------------------------------------------------------------
	//	CLASS		Pose
	//  ----------------------------------------------------------------------------
	///	@brief		A runtime 6-DOF rigid transform: a translation plus a rotation.
	///	@details	Represents the placement of a local frame within a parent frame. The rotation takes local
	///				axes to parent axes; the translation is the local origin expressed in the parent frame. A
	///				point is mapped local-to-parent by rotating then translating. Poses compose with
	///				`operator*` such that `parentFromLocal = parentFromMid * midFromLocal` applies the
	///				right-hand (innermost) transform first.
	//  ----------------------------------------------------------------------------
	class Pose
	{
	public:
		//----------------------------------
		//	CONSTRUCTORS
		//----------------------------------

		constexpr Pose() noexcept = default;

		/**
		 * @brief		Construct from a translation and a rotation.
		 * @param[in]	translation	the local origin expressed in the parent frame.
		 * @param[in]	rotation	the rotation from local axes to parent axes.
		 */
		constexpr Pose(const CartesianTuple& translation, const Quaternion& rotation) noexcept
		    : m_translation(translation)
		    , m_rotation(rotation)
		{
		}

		/**
		 * @brief		Construct from a translation and Euler angles.
		 * @param[in]	translation	the local origin expressed in the parent frame.
		 * @param[in]	orientation	the orientation as intrinsic Z-Y-X yaw/pitch/roll.
		 */
		constexpr Pose(const CartesianTuple& translation, const EulerAngles& orientation) noexcept
		    : m_translation(translation)
		    , m_rotation(toQuaternion(orientation))
		{
		}

		//----------------------------------
		//	GETTERS
		//----------------------------------

		constexpr const CartesianTuple& translation() const noexcept { return m_translation; }    ///< local origin in the parent frame
		constexpr const Quaternion&     rotation() const noexcept { return m_rotation; }           ///< local-to-parent rotation
		constexpr EulerAngles           orientation() const noexcept { return toEulerAngles(m_rotation); }

		//----------------------------------
		//	FACTORIES
		//----------------------------------

		/// The identity pose (no translation, no rotation).
		static constexpr Pose identity() noexcept { return Pose(CartesianTuple(0.0_m, 0.0_m, 0.0_m), Quaternion::identity()); }

		//----------------------------------
		//	OPERATIONS
		//----------------------------------

		/**
		 * @brief		Map a point from the local frame into the parent frame.
		 * @details		Rotates the point from local axes to parent axes, then adds the translation.
		 * @param[in]	point	the point in local Cartesian coordinates.
		 * @return		the point in parent Cartesian coordinates.
		 */
		constexpr CartesianTuple transformPoint(const CartesianTuple& point) const noexcept
		{
			const CartesianTuple rotated = m_rotation.rotate(point);
			return CartesianTuple(std::get<0>(rotated) + std::get<0>(m_translation),
			                      std::get<1>(rotated) + std::get<1>(m_translation),
			                      std::get<2>(rotated) + std::get<2>(m_translation));
		}

		/**
		 * @brief		The inverse pose (parent-to-local).
		 * @details		If this pose maps local into parent, the inverse maps parent into local.
		 * @return		the inverse rigid transform.
		 */
		constexpr Pose inverse() const noexcept
		{
			const Quaternion     invRotation = m_rotation.conjugate();
			const CartesianTuple negated(-std::get<0>(m_translation), -std::get<1>(m_translation), -std::get<2>(m_translation));
			return Pose(invRotation.rotate(negated), invRotation);
		}

	private:
		CartesianTuple m_translation{0.0_m, 0.0_m, 0.0_m};    ///< local origin expressed in the parent frame
		Quaternion     m_rotation{Quaternion::identity()};    ///< rotation from local axes to parent axes
	};

	//----------------------------------
	//	OPERATORS
	//----------------------------------

	/**
	 * @brief		Compose two poses: `lhs * rhs` applies `rhs` (inner) then `lhs` (outer).
	 * @details		If `lhs` is parent-from-mid and `rhs` is mid-from-local, the result is
	 *				parent-from-local. The composed translation is `lhs.rotate(rhs.translation) +
	 *				lhs.translation`; the composed rotation is `lhs.rotation * rhs.rotation`.
	 * @param[in]	lhs	the outer (parent-side) pose.
	 * @param[in]	rhs	the inner (local-side) pose.
	 * @return		the composed pose.
	 */
	constexpr Pose operator*(const Pose& lhs, const Pose& rhs) noexcept
	{
		const CartesianTuple rotatedRhs = lhs.rotation().rotate(rhs.translation());
		const CartesianTuple translation(std::get<0>(rotatedRhs) + std::get<0>(lhs.translation()),
		                                 std::get<1>(rotatedRhs) + std::get<1>(lhs.translation()),
		                                 std::get<2>(rotatedRhs) + std::get<2>(lhs.translation()));
		return Pose(translation, lhs.rotation() * rhs.rotation());
	}
}    // namespace coordinates

#endif    // pose_h
