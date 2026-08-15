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
/// @brief      Exhaustive GoogleTest suite for the `BodyFrame` frame of reference, its `BodyTransform`
///             policy (`Offset`/`Attitude` aliases), and the runtime `Pose` rigid transform. Verifies the
///             offset/rotation accessors, single-level and nested body-frame conversions and their
///             round-trips, integration with the frame graph (`is_frame_of_reference`, `convert<...>`),
///             pose composition/inversion/point-mapping, the equivalence of the compile-time (nested
///             `BodyFrame`) and runtime (`Pose`) models for one physical mounting, and compile-time
///             (`constexpr`) evaluability of both.
//
// ---------------------------------------------------------------------------------------------------------------------

#pragma once

//------------------------
//	INCLUDES
//------------------------

#include <tuple>

#include <gtest/gtest.h>

#include "bodyFrame.h"
#include "datum.h"
#include "frameOfReference.h"
#include "pose.h"

using namespace coordinates;
using namespace coordinates::coordinateFrames;
using namespace units;
using namespace units::literals;

//======================================================
//	TEST-LOCAL HELPERS (anonymous namespace -> no ODR clash with sibling test files)
//======================================================

namespace
{
	//------------------------------------------------------------------------------------------------------------------
	/// @brief		Constant-evaluable approximate scalar comparison, for `static_assert` regression guards.
	/// @param[in]	a		first value.
	/// @param[in]	b		second value.
	/// @param[in]	tol		absolute tolerance.
	/// @return		true when |a - b| <= tol.
	//------------------------------------------------------------------------------------------------------------------
	static constexpr bool bfApprox(double a, double b, double tol) noexcept { return (a - b <= tol) && (b - a <= tol); }

	//------------------------------------------------------------------------------------------------------------------
	/// @brief		Runtime helper: assert a Cartesian (metre-valued) tuple matches expected components.
	/// @param[in]	actual		the computed point.
	/// @param[in]	ex			expected X component, in metres.
	/// @param[in]	ey			expected Y component, in metres.
	/// @param[in]	ez			expected Z component, in metres.
	/// @param[in]	tol			absolute tolerance, in metres.
	//------------------------------------------------------------------------------------------------------------------
	static void bfExpectVecNear(const CartesianTuple& actual, double ex, double ey, double ez, double tol)
	{
		EXPECT_NEAR(std::get<0>(actual).to<double>(), ex, tol);
		EXPECT_NEAR(std::get<1>(actual).to<double>(), ey, tol);
		EXPECT_NEAR(std::get<2>(actual).to<double>(), ez, tol);
	}

	//------------------------------------------------------------------------------------------------------------------
	/// @brief		Runtime helper: assert two Cartesian tuples are equal component-wise within tolerance.
	/// @param[in]	actual		the computed point.
	/// @param[in]	expected	the reference point.
	/// @param[in]	tol			absolute tolerance, in metres.
	//------------------------------------------------------------------------------------------------------------------
	static void bfExpectTupleNear(const CartesianTuple& actual, const CartesianTuple& expected, double tol)
	{
		bfExpectVecNear(actual, std::get<0>(expected).to<double>(), std::get<1>(expected).to<double>(), std::get<2>(expected).to<double>(), tol);
	}

	/// A convenient concrete datum for the frame-graph tests. Named to avoid the `coordinates::Datum`
	/// class template that `using namespace coordinates` brings into scope.
	using WgsDatum = datums::WGS84_G1674;

	/// The local-level frame the body frames attach to.
	using LocalNED = NEDFrame<WgsDatum>;

	//------------------------------------------------------------------------------------------------------------------
	//	PHYSICAL SCENARIO: a camera mounted, pointing aft, on the wingtip of a plane whose nose points East.
	//	  PlaneBody : yaw 90 deg (nose -> parent +Y / East), no offset.
	//	  Wingtip   : pure offset (0.5, 3.2, -0.1) m in the plane body frame, no rotation.
	//	  CameraAft : yaw 180 deg (bore points aft along the wingtip -X), no offset.
	//------------------------------------------------------------------------------------------------------------------
	using PlaneBody = BodyFrame<LocalNED, Attitude<90.0_deg, 0.0_deg, 0.0_deg>>;
	using Wingtip   = BodyFrame<PlaneBody, Offset<0.5_m, 3.2_m, -0.1_m>>;
	using CameraAft = BodyFrame<Wingtip, Attitude<180.0_deg, 0.0_deg, 0.0_deg>>;

	/// A full 6-DOF transform used throughout the round-trip tests.
	using SixDof = BodyTransform<1.5_m, -2.5_m, 4.0_m, 30.0_deg, -15.0_deg, 20.0_deg>;
}    // namespace

//======================================================
//	BodyTransform - ACCESSORS
//======================================================

TEST(BodyFrameTransform, OffsetAccessorReturnsConstructedTranslation)
{
	using T = BodyTransform<1.0_m, 2.0_m, 3.0_m, 10.0_deg, 20.0_deg, 30.0_deg>;
	const CartesianTuple o = T::offset();
	EXPECT_EQ(1.0_m, std::get<0>(o));
	EXPECT_EQ(2.0_m, std::get<1>(o));
	EXPECT_EQ(3.0_m, std::get<2>(o));
}

TEST(BodyFrameTransform, EulerAccessorReturnsConstructedAngles)
{
	using T = BodyTransform<1.0_m, 2.0_m, 3.0_m, 10.0_deg, 20.0_deg, 30.0_deg>;
	const EulerAngles e = T::euler();
	EXPECT_EQ(10.0_deg, e.yaw());
	EXPECT_EQ(20.0_deg, e.pitch());
	EXPECT_EQ(30.0_deg, e.roll());
}

TEST(BodyFrameTransform, RotationMatchesQuaternionOfEuler)
{
	using T                = BodyTransform<0.0_m, 0.0_m, 0.0_m, 10.0_deg, 20.0_deg, 30.0_deg>;
	const Quaternion rot   = T::rotation();
	const Quaternion truth = toQuaternion(EulerAngles(10.0_deg, 20.0_deg, 30.0_deg));
	EXPECT_NEAR(rot.w().value(), truth.w().value(), 1e-15);
	EXPECT_NEAR(rot.x().value(), truth.x().value(), 1e-15);
	EXPECT_NEAR(rot.y().value(), truth.y().value(), 1e-15);
	EXPECT_NEAR(rot.z().value(), truth.z().value(), 1e-15);
}

TEST(BodyFrameTransform, OffsetAliasHasIdentityRotation)
{
	using T              = Offset<7.0_m, -8.0_m, 9.0_m>;
	const Quaternion rot = T::rotation();
	// Identity: (1,0,0,0). Rotating an arbitrary point must leave it unchanged.
	EXPECT_NEAR(rot.w().value(), 1.0, 1e-15);
	EXPECT_NEAR(rot.x().value(), 0.0, 1e-15);
	EXPECT_NEAR(rot.y().value(), 0.0, 1e-15);
	EXPECT_NEAR(rot.z().value(), 0.0, 1e-15);
	bfExpectVecNear(rot.rotate(CartesianTuple(3.0_m, 5.0_m, -2.0_m)), 3.0, 5.0, -2.0, 1e-12);

	const CartesianTuple o = T::offset();
	EXPECT_EQ(7.0_m, std::get<0>(o));
	EXPECT_EQ(-8.0_m, std::get<1>(o));
	EXPECT_EQ(9.0_m, std::get<2>(o));
}

TEST(BodyFrameTransform, AttitudeAliasHasZeroOffset)
{
	using T                = Attitude<45.0_deg, 12.0_deg, -33.0_deg>;
	const CartesianTuple o = T::offset();
	EXPECT_EQ(0.0_m, std::get<0>(o));
	EXPECT_EQ(0.0_m, std::get<1>(o));
	EXPECT_EQ(0.0_m, std::get<2>(o));

	// The rotation is still the requested attitude.
	const Quaternion truth = toQuaternion(EulerAngles(45.0_deg, 12.0_deg, -33.0_deg));
	const Quaternion rot   = T::rotation();
	EXPECT_NEAR(rot.w().value(), truth.w().value(), 1e-15);
	EXPECT_NEAR(rot.x().value(), truth.x().value(), 1e-15);
	EXPECT_NEAR(rot.y().value(), truth.y().value(), 1e-15);
	EXPECT_NEAR(rot.z().value(), truth.z().value(), 1e-15);
}

TEST(BodyFrameTransform, FullSixDofCarriesBothOffsetAndRotation)
{
	const CartesianTuple o = SixDof::offset();
	EXPECT_EQ(1.5_m, std::get<0>(o));
	EXPECT_EQ(-2.5_m, std::get<1>(o));
	EXPECT_EQ(4.0_m, std::get<2>(o));

	const EulerAngles e = SixDof::euler();
	EXPECT_EQ(30.0_deg, e.yaw());
	EXPECT_EQ(-15.0_deg, e.pitch());
	EXPECT_EQ(20.0_deg, e.roll());
}

//======================================================
//	BodyFrame - SINGLE-LEVEL CONVERSIONS
//======================================================

TEST(BodyFrameConvert, PureOffsetShiftsPointNoRotation)
{
	using Body = BodyFrame<LocalNED, Offset<10.0_m, 20.0_m, 30.0_m>>;
	// A body point at the body origin lands exactly on the offset in the parent.
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(0.0_m, 0.0_m, 0.0_m), FrameData{}), 10.0, 20.0, 30.0, 1e-12);
	// A generic body point is simply shifted; no axis change.
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(1.0_m, 2.0_m, 3.0_m), FrameData{}), 11.0, 22.0, 33.0, 1e-12);
}

TEST(BodyFrameConvert, PureYaw90SendsBodyXToParentY)
{
	using Body = BodyFrame<LocalNED, Attitude<90.0_deg, 0.0_deg, 0.0_deg>>;
	// yaw +90 about Z: body +X -> parent +Y, body +Y -> parent -X, body +Z unchanged.
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(1.0_m, 0.0_m, 0.0_m), FrameData{}), 0.0, 1.0, 0.0, 1e-12);
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(0.0_m, 1.0_m, 0.0_m), FrameData{}), -1.0, 0.0, 0.0, 1e-12);
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(0.0_m, 0.0_m, 1.0_m), FrameData{}), 0.0, 0.0, 1.0, 1e-12);
}

TEST(BodyFrameConvert, PurePitch90SendsBodyXToParentDownNed)
{
	using Body = BodyFrame<LocalNED, Attitude<0.0_deg, 90.0_deg, 0.0_deg>>;
	// pitch +90 about Y: body +X -> parent -Z, body +Z -> parent +X, body +Y unchanged.
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(1.0_m, 0.0_m, 0.0_m), FrameData{}), 0.0, 0.0, -1.0, 1e-12);
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(0.0_m, 0.0_m, 1.0_m), FrameData{}), 1.0, 0.0, 0.0, 1e-12);
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(0.0_m, 1.0_m, 0.0_m), FrameData{}), 0.0, 1.0, 0.0, 1e-12);
}

TEST(BodyFrameConvert, PureRoll90SendsBodyYToParentZ)
{
	using Body = BodyFrame<LocalNED, Attitude<0.0_deg, 0.0_deg, 90.0_deg>>;
	// roll +90 about X: body +Y -> parent +Z, body +Z -> parent -Y, body +X unchanged.
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(0.0_m, 1.0_m, 0.0_m), FrameData{}), 0.0, 0.0, 1.0, 1e-12);
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(0.0_m, 0.0_m, 1.0_m), FrameData{}), 0.0, -1.0, 0.0, 1e-12);
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(1.0_m, 0.0_m, 0.0_m), FrameData{}), 1.0, 0.0, 0.0, 1e-12);
}

TEST(BodyFrameConvert, CombinedRotateThenOffset)
{
	// yaw 90 then offset (100, 200, 300). body (1,0,0) -> rotate -> (0,1,0) -> +offset -> (100, 201, 300).
	using Body = BodyFrame<LocalNED, BodyTransform<100.0_m, 200.0_m, 300.0_m, 90.0_deg, 0.0_deg, 0.0_deg>>;
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(1.0_m, 0.0_m, 0.0_m), FrameData{}), 100.0, 201.0, 300.0, 1e-12);
	bfExpectVecNear(Body::convertToBaseFrame(CartesianTuple(0.0_m, 0.0_m, 0.0_m), FrameData{}), 100.0, 200.0, 300.0, 1e-12);
}

TEST(BodyFrameConvert, FromBaseFrameUndoesOffsetThenRotation)
{
	using Body = BodyFrame<LocalNED, BodyTransform<100.0_m, 200.0_m, 300.0_m, 90.0_deg, 0.0_deg, 0.0_deg>>;
	// Inverse of the CombinedRotateThenOffset case: parent (100,201,300) -> body (1,0,0).
	bfExpectVecNear(Body::convertFromBaseFrame(CartesianTuple(100.0_m, 201.0_m, 300.0_m), FrameData{}), 1.0, 0.0, 0.0, 1e-12);
	bfExpectVecNear(Body::convertFromBaseFrame(CartesianTuple(100.0_m, 200.0_m, 300.0_m), FrameData{}), 0.0, 0.0, 0.0, 1e-12);
}

TEST(BodyFrameConvert, SingleLevelRoundTripManyTransforms)
{
	using B1 = BodyFrame<LocalNED, Offset<5.0_m, -3.0_m, 2.0_m>>;
	using B2 = BodyFrame<LocalNED, Attitude<37.0_deg, -22.0_deg, 68.0_deg>>;
	using B3 = BodyFrame<LocalNED, SixDof>;

	const CartesianTuple points[] = {
	    CartesianTuple(0.0_m, 0.0_m, 0.0_m),
	    CartesianTuple(1.0_m, 2.0_m, 3.0_m),
	    CartesianTuple(-7.5_m, 4.25_m, -9.0_m),
	    CartesianTuple(1000.0_m, -2000.0_m, 500.0_m),
	};

	for (const auto& p : points)
	{
		bfExpectTupleNear(B1::convertFromBaseFrame(B1::convertToBaseFrame(p, FrameData{}), FrameData{}), p, 1e-9);
		bfExpectTupleNear(B2::convertFromBaseFrame(B2::convertToBaseFrame(p, FrameData{}), FrameData{}), p, 1e-9);
		bfExpectTupleNear(B3::convertFromBaseFrame(B3::convertToBaseFrame(p, FrameData{}), FrameData{}), p, 1e-9);
		// and the other direction
		bfExpectTupleNear(B3::convertToBaseFrame(B3::convertFromBaseFrame(p, FrameData{}), FrameData{}), p, 1e-9);
	}
}

//======================================================
//	BodyFrame - FRAME-GRAPH CONCEPT SATISFACTION
//======================================================

TEST(BodyFrameGraph, SatisfiesIsFrameOfReference)
{
	static_assert(coordinates::traits::is_frame_of_reference<BodyFrame<LocalNED, Offset<1.0_m, 2.0_m, 3.0_m>>>,
	              "a single-level body frame must be a frame of reference");
	static_assert(coordinates::traits::is_frame_of_reference<PlaneBody>, "PlaneBody must be a frame of reference");
	static_assert(coordinates::traits::is_frame_of_reference<Wingtip>, "nested (2-level) body frame must be a frame of reference");
	static_assert(coordinates::traits::is_frame_of_reference<CameraAft>, "nested (3-level) body frame must be a frame of reference");
	SUCCEED();
}

TEST(BodyFrameGraph, IsCartesianAndSharesBaseFrameWithParent)
{
	static_assert(coordinates::traits::is_cartesian_frame<PlaneBody>, "a body frame is Cartesian");
	static_assert(coordinates::traits::is_cartesian_frame<CameraAft>, "a nested body frame is Cartesian");
	// A body frame is convertible with its NED parent (they share the same lowest base frame, ECEF/ITRF).
	// The parentheses are required: a two-argument concept inside static_assert would otherwise split on the comma.
	static_assert((coordinates::traits::is_convertible_frame<CameraAft, LocalNED>), "camera frame converts with its NED parent");
	static_assert((coordinates::traits::is_convertible_frame<CameraAft, ENUFrame<WgsDatum>>), "camera frame converts with ENU (shared base)");
	SUCCEED();
}

TEST(BodyFrameGraph, BaseTupleTypeIsParentTuple)
{
	static_assert(std::is_same_v<Wingtip::base_tuple_type, CartesianTuple>, "wingtip base tuple is the plane-body Cartesian tuple");
	static_assert(std::is_same_v<CameraAft::tuple_type, CartesianTuple>, "camera tuple type is Cartesian");
	SUCCEED();
}

//======================================================
//	BodyFrame - NESTED (camera-on-wingtip) CONVERSIONS
//======================================================

TEST(BodyFrameNested, TwoLevelCameraThroughWingtipToPlaneBody)
{
	// A point 10 m in front of the aft-pointing camera.
	const CartesianTuple pCam(10.0_m, 0.0_m, 0.0_m);

	// Camera -> Wingtip: yaw 180 flips +X -> -X (no offset).
	const CartesianTuple pWing = CameraAft::convertToBaseFrame(pCam, FrameData{});
	bfExpectVecNear(pWing, -10.0, 0.0, 0.0, 1e-9);

	// Wingtip -> PlaneBody: pure offset (0.5, 3.2, -0.1).
	const CartesianTuple pBody = Wingtip::convertToBaseFrame(pWing, FrameData{});
	bfExpectVecNear(pBody, -9.5, 3.2, -0.1, 1e-9);
}

TEST(BodyFrameNested, ThreeLevelCameraThroughPlaneToNed)
{
	const CartesianTuple pCam(10.0_m, 0.0_m, 0.0_m);

	// Full 3-level graph conversion camera -> NED.
	const CartesianTuple pNed = coordinates::convert<CameraAft, LocalNED>(pCam, FrameData{}, FrameData{});

	// Hand-derived: cam (10,0,0) -> wing (-10,0,0) -> body (-9.5, 3.2, -0.1)
	//   -> NED via yaw 90 about Z: (bx,by,bz) -> (-by, bx, bz) -> (-3.2, -9.5, -0.1).
	bfExpectVecNear(pNed, -3.2, -9.5, -0.1, 1e-6);
}

TEST(BodyFrameNested, FullGraphRoundTripsToOriginalPoint)
{
	const CartesianTuple points[] = {
	    CartesianTuple(10.0_m, 0.0_m, 0.0_m),
	    CartesianTuple(1.0_m, 2.0_m, 3.0_m),
	    CartesianTuple(-4.0_m, 6.5_m, -8.25_m),
	    CartesianTuple(0.0_m, 0.0_m, 0.0_m),
	};

	for (const auto& pCam : points)
	{
		const CartesianTuple pNed  = coordinates::convert<CameraAft, LocalNED>(pCam, FrameData{}, FrameData{});
		const CartesianTuple pBack = coordinates::convert<LocalNED, CameraAft>(pNed, FrameData{}, FrameData{});
		bfExpectTupleNear(pBack, pCam, 1e-6);
	}
}

TEST(BodyFrameNested, DeepNestingMatchesStepwiseComposition)
{
	// Verify the 3-level graph conversion equals the explicit per-level chain.
	const CartesianTuple pCam(2.0_m, -3.0_m, 5.0_m);

	const CartesianTuple stepWing = CameraAft::convertToBaseFrame(pCam, FrameData{});
	const CartesianTuple stepBody = Wingtip::convertToBaseFrame(stepWing, FrameData{});
	const CartesianTuple stepNed  = PlaneBody::convertToBaseFrame(stepBody, FrameData{});

	const CartesianTuple graphNed = coordinates::convert<CameraAft, LocalNED>(pCam, FrameData{}, FrameData{});

	bfExpectTupleNear(graphNed, stepNed, 1e-9);
}

//======================================================
//	Pose - CONSTRUCTION & ACCESSORS
//======================================================

TEST(PoseConstruct, DefaultIsIdentity)
{
	const Pose p;
	bfExpectVecNear(p.translation(), 0.0, 0.0, 0.0, 1e-15);
	const Quaternion r = p.rotation();
	EXPECT_NEAR(r.w().value(), 1.0, 1e-15);
	EXPECT_NEAR(r.x().value(), 0.0, 1e-15);
	EXPECT_NEAR(r.y().value(), 0.0, 1e-15);
	EXPECT_NEAR(r.z().value(), 0.0, 1e-15);
}

TEST(PoseConstruct, FromTranslationAndQuaternion)
{
	const Quaternion q = toQuaternion(EulerAngles(30.0_deg, 0.0_deg, 0.0_deg));
	const Pose       p(CartesianTuple(1.0_m, 2.0_m, 3.0_m), q);
	bfExpectVecNear(p.translation(), 1.0, 2.0, 3.0, 1e-15);
	EXPECT_NEAR(p.rotation().w().value(), q.w().value(), 1e-15);
	EXPECT_NEAR(p.rotation().z().value(), q.z().value(), 1e-15);
}

TEST(PoseConstruct, FromEulerMatchesFromQuaternion)
{
	const EulerAngles e(35.0_deg, -12.0_deg, 47.0_deg);
	const Pose        fromEuler(CartesianTuple(1.0_m, 2.0_m, 3.0_m), e);
	const Pose        fromQuat(CartesianTuple(1.0_m, 2.0_m, 3.0_m), toQuaternion(e));

	// Same rotation applied to a test point must land at the same place.
	const CartesianTuple test(4.0_m, -5.0_m, 6.0_m);
	bfExpectTupleNear(fromEuler.transformPoint(test), fromQuat.transformPoint(test), 1e-12);
}

TEST(PoseConstruct, OrientationRoundTripsThroughEuler)
{
	const EulerAngles e(35.0_deg, -12.0_deg, 47.0_deg);
	const Pose        p(CartesianTuple(0.0_m, 0.0_m, 0.0_m), e);
	const EulerAngles back = p.orientation();
	EXPECT_NEAR(back.yaw().to<double>(), 35.0, 1e-9);
	EXPECT_NEAR(back.pitch().to<double>(), -12.0, 1e-9);
	EXPECT_NEAR(back.roll().to<double>(), 47.0, 1e-9);
}

TEST(PoseConstruct, IdentityFactory)
{
	const Pose p = Pose::identity();
	const CartesianTuple test(7.0_m, -8.0_m, 9.0_m);
	bfExpectTupleNear(p.transformPoint(test), test, 1e-15);
}

//======================================================
//	Pose - transformPoint
//======================================================

TEST(PoseTransform, IdentityLeavesPointsUnchanged)
{
	const Pose id = Pose::identity();
	bfExpectVecNear(id.transformPoint(CartesianTuple(3.0_m, -4.0_m, 5.5_m)), 3.0, -4.0, 5.5, 1e-15);
}

TEST(PoseTransform, PureTranslation)
{
	const Pose p(CartesianTuple(10.0_m, 20.0_m, 30.0_m), Quaternion::identity());
	bfExpectVecNear(p.transformPoint(CartesianTuple(1.0_m, 2.0_m, 3.0_m)), 11.0, 22.0, 33.0, 1e-12);
}

TEST(PoseTransform, PureRotationYaw90)
{
	const Pose p(CartesianTuple(0.0_m, 0.0_m, 0.0_m), toQuaternion(EulerAngles(90.0_deg, 0.0_deg, 0.0_deg)));
	// yaw +90: +X -> +Y, +Y -> -X.
	bfExpectVecNear(p.transformPoint(CartesianTuple(1.0_m, 0.0_m, 0.0_m)), 0.0, 1.0, 0.0, 1e-12);
	bfExpectVecNear(p.transformPoint(CartesianTuple(0.0_m, 1.0_m, 0.0_m)), -1.0, 0.0, 0.0, 1e-12);
}

TEST(PoseTransform, RotateThenTranslateOrder)
{
	// yaw 90 THEN + (100,200,300). (1,0,0) rotates to (0,1,0), then + offset -> (100,201,300).
	const Pose p(CartesianTuple(100.0_m, 200.0_m, 300.0_m), toQuaternion(EulerAngles(90.0_deg, 0.0_deg, 0.0_deg)));
	bfExpectVecNear(p.transformPoint(CartesianTuple(1.0_m, 0.0_m, 0.0_m)), 100.0, 201.0, 300.0, 1e-12);
}

//======================================================
//	Pose - inverse
//======================================================

TEST(PoseInverse, TransformPointRoundTrip)
{
	const Pose p(CartesianTuple(1.5_m, -2.5_m, 4.0_m), toQuaternion(EulerAngles(30.0_deg, -15.0_deg, 20.0_deg)));
	const Pose inv = p.inverse();

	const CartesianTuple points[] = {
	    CartesianTuple(0.0_m, 0.0_m, 0.0_m),
	    CartesianTuple(1.0_m, 2.0_m, 3.0_m),
	    CartesianTuple(-7.5_m, 4.25_m, -9.0_m),
	};
	for (const auto& pt : points)
		bfExpectTupleNear(inv.transformPoint(p.transformPoint(pt)), pt, 1e-9);
}

TEST(PoseInverse, ComposeWithSelfInverseIsIdentity)
{
	const Pose p(CartesianTuple(1.5_m, -2.5_m, 4.0_m), toQuaternion(EulerAngles(30.0_deg, -15.0_deg, 20.0_deg)));
	const CartesianTuple test(4.0_m, -5.0_m, 6.0_m);

	const Pose leftInv  = p.inverse() * p;
	const Pose rightInv = p * p.inverse();
	bfExpectTupleNear(leftInv.transformPoint(test), test, 1e-9);
	bfExpectTupleNear(rightInv.transformPoint(test), test, 1e-9);
}

//======================================================
//	Pose - operator* COMPOSITION
//======================================================

TEST(PoseCompose, ComposedEqualsSequentialApplication)
{
	// parentFromLocal = parentFromMid * midFromLocal ; applying the product to a local point
	// equals applying midFromLocal then parentFromMid.
	const Pose parentFromMid(CartesianTuple(10.0_m, 0.0_m, 0.0_m), toQuaternion(EulerAngles(90.0_deg, 0.0_deg, 0.0_deg)));
	const Pose midFromLocal(CartesianTuple(0.0_m, 5.0_m, 0.0_m), toQuaternion(EulerAngles(0.0_deg, 45.0_deg, 0.0_deg)));

	const Pose composed = parentFromMid * midFromLocal;

	const CartesianTuple points[] = {
	    CartesianTuple(0.0_m, 0.0_m, 0.0_m),
	    CartesianTuple(1.0_m, 2.0_m, 3.0_m),
	    CartesianTuple(-2.0_m, 7.0_m, -4.0_m),
	};
	for (const auto& local : points)
	{
		const CartesianTuple viaComposed   = composed.transformPoint(local);
		const CartesianTuple viaSequential = parentFromMid.transformPoint(midFromLocal.transformPoint(local));
		bfExpectTupleNear(viaComposed, viaSequential, 1e-9);
	}
}

TEST(PoseCompose, IdentityIsNeutralElement)
{
	const Pose p(CartesianTuple(3.0_m, -1.0_m, 2.0_m), toQuaternion(EulerAngles(25.0_deg, 10.0_deg, -5.0_deg)));
	const CartesianTuple test(4.0_m, -5.0_m, 6.0_m);

	bfExpectTupleNear((p * Pose::identity()).transformPoint(test), p.transformPoint(test), 1e-12);
	bfExpectTupleNear((Pose::identity() * p).transformPoint(test), p.transformPoint(test), 1e-12);
}

TEST(PoseCompose, Associativity)
{
	const Pose a(CartesianTuple(1.0_m, 0.0_m, 0.0_m), toQuaternion(EulerAngles(30.0_deg, 0.0_deg, 0.0_deg)));
	const Pose b(CartesianTuple(0.0_m, 2.0_m, 0.0_m), toQuaternion(EulerAngles(0.0_deg, 40.0_deg, 0.0_deg)));
	const Pose c(CartesianTuple(0.0_m, 0.0_m, 3.0_m), toQuaternion(EulerAngles(0.0_deg, 0.0_deg, 50.0_deg)));

	const Pose left  = (a * b) * c;
	const Pose right = a * (b * c);

	const CartesianTuple test(2.0_m, -3.0_m, 5.0_m);
	bfExpectTupleNear(left.transformPoint(test), right.transformPoint(test), 1e-9);
}

//======================================================
//	EQUIVALENCE OF THE TWO MODELS (nested BodyFrame == composed Pose)
//======================================================

TEST(PoseCompose, CameraOnWingtipEqualsNestedBodyFrames)
{
	// Same physical mounting as PlaneBody/Wingtip/CameraAft, expressed as composed poses.
	// A frame's placement pose maps LOCAL points to its PARENT, matching convertToBaseFrame
	// (rotate by the transform's rotation, then add the offset).
	const Pose planeInNed(CartesianTuple(0.0_m, 0.0_m, 0.0_m), toQuaternion(EulerAngles(90.0_deg, 0.0_deg, 0.0_deg)));
	const Pose wingInPlane(CartesianTuple(0.5_m, 3.2_m, -0.1_m), Quaternion::identity());
	const Pose cameraInWing(CartesianTuple(0.0_m, 0.0_m, 0.0_m), toQuaternion(EulerAngles(180.0_deg, 0.0_deg, 0.0_deg)));

	// cameraInNed = planeInNed * wingInPlane * cameraInWing  (outer-most on the left)
	const Pose cameraInNed = planeInNed * wingInPlane * cameraInWing;

	const CartesianTuple points[] = {
	    CartesianTuple(10.0_m, 0.0_m, 0.0_m),
	    CartesianTuple(1.0_m, 2.0_m, 3.0_m),
	    CartesianTuple(-4.0_m, 6.5_m, -8.25_m),
	    CartesianTuple(0.0_m, 0.0_m, 0.0_m),
	};

	for (const auto& pCam : points)
	{
		const CartesianTuple viaFrames = coordinates::convert<CameraAft, LocalNED>(pCam, FrameData{}, FrameData{});
		const CartesianTuple viaPoses  = cameraInNed.transformPoint(pCam);
		bfExpectTupleNear(viaPoses, viaFrames, 1e-6);
	}
}

//======================================================
//	CONSTEXPR PROOFS
//======================================================

// `FrameData` is not a literal type (it has no constexpr constructor), so `convertToBaseFrame`, which
// takes a `const FrameData&`, cannot itself be invoked inside a constant expression. The body-frame
// transform, however, IS fully constexpr: `Transform::rotation()`, `Quaternion::rotate`, and
// `Transform::offset()` are the exact computation the conversion performs. These static_asserts prove
// that computation is compile-time evaluable; the runtime `BodyFrameConvert`/`BodyFrameNested` suites
// prove `convertToBaseFrame` itself produces the same numbers.

//------------------------------------------------------------------------------------------------------------------
/// @brief		Constant-evaluable stand-in for `BodyFrame::convertToBaseFrame` (rotate then offset).
/// @tparam		Transform	the `BodyTransform` policy to apply.
/// @param[in]	point		the body-local point.
/// @return		the point expressed in the parent frame.
//------------------------------------------------------------------------------------------------------------------
template<class Transform>
static constexpr CartesianTuple bfToParent(const CartesianTuple& point) noexcept
{
	const CartesianTuple rotated = Transform::rotation().rotate(point);
	const CartesianTuple offset  = Transform::offset();
	return CartesianTuple(std::get<0>(rotated) + std::get<0>(offset),
	                      std::get<1>(rotated) + std::get<1>(offset),
	                      std::get<2>(rotated) + std::get<2>(offset));
}

TEST(BodyFrameConstexpr, TransformMathEvaluatesAtCompileTime)
{
	using Transform = BodyTransform<100.0_m, 200.0_m, 300.0_m, 90.0_deg, 0.0_deg, 0.0_deg>;

	// Single-level rotate-then-offset, evaluated entirely at compile time.
	constexpr CartesianTuple toParent = bfToParent<Transform>(CartesianTuple(1.0_m, 0.0_m, 0.0_m));
	static_assert(bfApprox(std::get<0>(toParent).to<double>(), 100.0, 1e-9), "constexpr body->parent X");
	static_assert(bfApprox(std::get<1>(toParent).to<double>(), 201.0, 1e-9), "constexpr body->parent Y");
	static_assert(bfApprox(std::get<2>(toParent).to<double>(), 300.0, 1e-9), "constexpr body->parent Z");

	// A nested (camera -> wingtip -> plane body) chain, at compile time.
	constexpr CartesianTuple pCam(10.0_m, 0.0_m, 0.0_m);
	constexpr CartesianTuple pWing = bfToParent<Attitude<180.0_deg, 0.0_deg, 0.0_deg>>(pCam);
	constexpr CartesianTuple pBody = bfToParent<Offset<0.5_m, 3.2_m, -0.1_m>>(pWing);
	static_assert(bfApprox(std::get<0>(pBody).to<double>(), -9.5, 1e-9), "constexpr nested X");
	static_assert(bfApprox(std::get<1>(pBody).to<double>(), 3.2, 1e-9), "constexpr nested Y");
	static_assert(bfApprox(std::get<2>(pBody).to<double>(), -0.1, 1e-9), "constexpr nested Z");

	SUCCEED();
}

TEST(PoseConstexpr, OperationsEvaluateAtCompileTime)
{
	// identity transformPoint is the identity, at compile time.
	constexpr Pose           id = Pose::identity();
	constexpr CartesianTuple probe(2.0_m, -3.0_m, 5.0_m);
	constexpr CartesianTuple viaIdentity = id.transformPoint(probe);
	static_assert(bfApprox(std::get<0>(viaIdentity).to<double>(), 2.0, 1e-12), "constexpr identity X");
	static_assert(bfApprox(std::get<1>(viaIdentity).to<double>(), -3.0, 1e-12), "constexpr identity Y");
	static_assert(bfApprox(std::get<2>(viaIdentity).to<double>(), 5.0, 1e-12), "constexpr identity Z");

	// pure-translation transformPoint at compile time.
	constexpr Pose           shift(CartesianTuple(10.0_m, 20.0_m, 30.0_m), Quaternion::identity());
	constexpr CartesianTuple shifted = shift.transformPoint(CartesianTuple(1.0_m, 2.0_m, 3.0_m));
	static_assert(bfApprox(std::get<0>(shifted).to<double>(), 11.0, 1e-12), "constexpr shift X");
	static_assert(bfApprox(std::get<1>(shifted).to<double>(), 22.0, 1e-12), "constexpr shift Y");
	static_assert(bfApprox(std::get<2>(shifted).to<double>(), 33.0, 1e-12), "constexpr shift Z");

	// compose + inverse round-trip, at compile time.
	constexpr Pose           p(CartesianTuple(1.5_m, -2.5_m, 4.0_m), Quaternion(0.9238795325112867, 0.0, 0.0, 0.3826834323650898));
	constexpr Pose           roundTripPose = p.inverse() * p;
	constexpr CartesianTuple test(4.0_m, -5.0_m, 6.0_m);
	constexpr CartesianTuple back = roundTripPose.transformPoint(test);
	static_assert(bfApprox(std::get<0>(back).to<double>(), 4.0, 1e-9), "constexpr compose/inverse X");
	static_assert(bfApprox(std::get<1>(back).to<double>(), -5.0, 1e-9), "constexpr compose/inverse Y");
	static_assert(bfApprox(std::get<2>(back).to<double>(), 6.0, 1e-9), "constexpr compose/inverse Z");

	SUCCEED();
}
