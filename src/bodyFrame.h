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
/// @brief      Definition of the `BodyFrame` frame of reference and its rigid-body transform policy.
/// @details    A `BodyFrame` is a Cartesian frame rigidly attached to a parent frame at a fixed offset and
///             orientation -- an aircraft body relative to local NED, a sensor relative to that body, a
///             camera relative to that sensor. The offset and orientation are carried in the frame TYPE (a
///             compile-time `BodyTransform` policy of unit-typed non-type template parameters), so a body
///             frame slots into the existing frame graph with no dispatcher changes and NESTS to arbitrary
///             depth: `BodyFrame<BodyFrame<NEDFrame<Datum>, Wingtip>, Camera>` resolves each level's own
///             rigid transform during the type-driven conversion. The rotation is applied via the
///             `Quaternion` from the rotation-math library; every transform is `constexpr`-capable.
//
// ---------------------------------------------------------------------------------------------------------------------

#ifndef bodyFrame_h
#define bodyFrame_h

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

	inline namespace coordinateFrames
	{
		//	----------------------------------------------------------------------------
		//	STRUCT		BodyTransform
		//  ----------------------------------------------------------------------------
		///	@brief		Compile-time rigid transform of a body frame relative to its parent.
		///	@details	Encodes the body frame's origin offset (Cartesian, in the parent frame) and its
		///				orientation (intrinsic Z-Y-X yaw/pitch/roll) as unit-typed non-type template
		///				parameters, so the transform is part of the frame's type. The offset is the location
		///				of the body-frame origin expressed in the parent frame; the orientation is the
		///				rotation from the parent axes to the body axes, applied as intrinsic Z-Y-X
		///				(Tait-Bryan): yaw about Z, then pitch about the new Y, then roll about the new X. In an
		///				aerospace body frame (X forward, Y right, Z down) this is the conventional sense:
		///				positive yaw turns the nose right, positive pitch is nose-up (the forward axis tilts
		///				toward -Z), positive roll banks right.
		///	@tparam		OffsetX		body-origin offset along the parent X axis
		///	@tparam		OffsetY		body-origin offset along the parent Y axis
		///	@tparam		OffsetZ		body-origin offset along the parent Z axis
		///	@tparam		Yaw			rotation about Z, from parent axes to body axes
		///	@tparam		Pitch		rotation about the new Y
		///	@tparam		Roll		rotation about the new X
		//  ----------------------------------------------------------------------------
		template<meters<double>  OffsetX,
		         meters<double>  OffsetY,
		         meters<double>  OffsetZ,
		         degrees<double> Yaw,
		         degrees<double> Pitch,
		         degrees<double> Roll>
		struct BodyTransform
		{
			/// The body-origin offset, expressed in the parent frame.
			static constexpr CartesianTuple offset() noexcept { return CartesianTuple(OffsetX, OffsetY, OffsetZ); }

			/// The orientation of the body axes relative to the parent axes, as yaw/pitch/roll.
			static constexpr EulerAngles euler() noexcept { return EulerAngles(Yaw, Pitch, Roll); }

			/// The orientation as a quaternion (parent-from-body rotation).
			static constexpr Quaternion rotation() noexcept { return toQuaternion(euler()); }
		};

		/// A body transform that is a pure offset (no rotation).
		template<meters<double> OffsetX, meters<double> OffsetY, meters<double> OffsetZ>
		using Offset = BodyTransform<OffsetX, OffsetY, OffsetZ, degrees<>(0.0), degrees<>(0.0), degrees<>(0.0)>;

		/// A body transform that is a pure rotation (no offset).
		template<degrees<double> Yaw, degrees<double> Pitch, degrees<double> Roll>
		using Attitude = BodyTransform<meters<>(0.0), meters<>(0.0), meters<>(0.0), Yaw, Pitch, Roll>;

		//	----------------------------------------------------------------------------
		//	STRUCT		BodyFrame
		//  ----------------------------------------------------------------------------
		///	@brief		A Cartesian frame rigidly attached to a parent frame.
		///	@details	Converts between body-local coordinates and the parent frame by rotating a body point
		///				into the parent axes and translating by the fixed offset. Because the transform is
		///				part of the type, body frames compose to arbitrary depth through the frame graph with
		///				no dispatcher changes. The `FrameData` argument is accepted for interface uniformity
		///				but is unused: a body frame's transform is fixed by its type, and any runtime-varying
		///				pose is modeled as a `Pose` value rather than a frame.
		///	@tparam		Parent		the parent frame of reference this body attaches to.
		///	@tparam		Transform	the `BodyTransform` giving the offset + orientation relative to `Parent`.
		//  ----------------------------------------------------------------------------
		template<class Parent, class Transform>
		struct BodyFrame : frameOfReference<typename frame_traits<Parent>::datum_type, Parent, CartesianTuple>
		{
			using tuple_type      = CartesianTuple;
			using base_tuple_type = typename frame_traits<Parent>::tuple_type;

			/**
			 * @brief		Convert a body-local point into the parent frame.
			 * @details		Rotates the point from body axes into parent axes, then translates by the body
			 *				origin's offset in the parent frame.
			 * @tparam		Args	unused arguments forwarded by the conversion dispatcher.
			 * @param[in]	point	the point in body-local Cartesian coordinates.
			 * @return		the equivalent point in the parent frame's Cartesian coordinates.
			 */
			template<class... Args>
			static constexpr base_tuple_type convertToBaseFrame(const tuple_type& point, const FrameData&, Args...) noexcept
			{
				const CartesianTuple rotated = Transform::rotation().rotate(point);
				const CartesianTuple offset  = Transform::offset();
				return base_tuple_type(std::get<0>(rotated) + std::get<0>(offset),
				                       std::get<1>(rotated) + std::get<1>(offset),
				                       std::get<2>(rotated) + std::get<2>(offset));
			}

			/**
			 * @brief		Convert a parent-frame point into body-local coordinates.
			 * @details		Translates by the negated offset, then rotates from parent axes into body axes
			 *				with the inverse rotation.
			 * @tparam		Args	unused arguments forwarded by the conversion dispatcher.
			 * @param[in]	point	the point in the parent frame's Cartesian coordinates.
			 * @return		the equivalent point in body-local Cartesian coordinates.
			 */
			template<class... Args>
			static constexpr tuple_type convertFromBaseFrame(const base_tuple_type& point, const FrameData&, Args...) noexcept
			{
				const CartesianTuple offset = Transform::offset();
				const CartesianTuple shifted(std::get<0>(point) - std::get<0>(offset),
				                             std::get<1>(point) - std::get<1>(offset),
				                             std::get<2>(point) - std::get<2>(offset));
				return Transform::rotation().conjugate().rotate(shifted);
			}
		};
	}    // namespace coordinateFrames
}    // namespace coordinates

#endif    // bodyFrame_h
