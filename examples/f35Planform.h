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
// The F-35 planform as body-axis vertex tables (nose +x, right +y, down +z, meters; each loop traced from a
// top-view silhouette -- dense boundary resample then Visvalingam-reduced -- scaled to the real 15.70 m length)
// and a `consteval` builder that assembles the airframe as an `Entity` tree AT COMPILE TIME. Because `Entity`,
// `Coordinate`, `Pose`, and `CartesianTuple` are literal types, the whole airframe -- one root entity with every
// outline and detail-loop vertex attached as a child -- is constructed, posed, and resolved inside a constant
// evaluation. `buildF35` returns the finished
// tree; `f35ChildCount` proves the build is a compile-time constant. The render program includes this header for
// its vertex data, so the shape has a single source of truth shared by the compile-time build and the runtime
// animation.
//
//--------------------------------------------------------------------------------------------------

#ifndef f35Planform_h
#define f35Planform_h

#include "coordinates.h"

namespace f35
{
	using namespace coordinates;
	using namespace coordinates::datums;
	using namespace units::literals;

	/// The datum whose ECEF frame the airframe lives in.
	using Datum = WGS84_G1674;

	//----------------------------------
	//	PLANFORM (body axes, meters; +x forward, +y right, +z down) -- each loop traced from a top-view
	//	silhouette (dense boundary resample, then Visvalingam-reduced), scaled to the 15.70 m length.
	//----------------------------------

	/// The SIMPLE outline: the fuselage + wing + tail silhouette as one closed loop, control surfaces flush.
	/// Traced from the top-view reference (silhouette flood-fill, one half mirrored for exact symmetry, then
	/// simplified) and scaled to the real 15.70 m length / 10.75 m span.
	inline constexpr CartesianVector outline()
	{
		return {
		        {-7.850_m, -1.248_m, +0.000_m}, {-7.283_m, -3.443_m, +0.000_m}, {-5.883_m, -3.443_m, +0.000_m},
		        {-4.937_m, -1.967_m, +0.000_m}, {-4.559_m, -1.854_m, +0.000_m}, {-3.689_m, -5.334_m, +0.000_m},
		        {-2.137_m, -5.372_m, +0.000_m}, {+0.019_m, -2.156_m, +0.000_m}, {+0.776_m, -1.778_m, +0.000_m},
		        {+3.272_m, -1.665_m, +0.000_m}, {+3.575_m, -1.513_m, +0.000_m}, {+3.310_m, -1.173_m, +0.000_m},
		        {+3.386_m, -0.946_m, +0.000_m}, {+6.942_m, -0.454_m, +0.000_m}, {+7.850_m, +0.000_m, +0.000_m},
		        {+6.942_m, +0.454_m, +0.000_m}, {+3.386_m, +0.946_m, +0.000_m}, {+3.310_m, +1.173_m, +0.000_m},
		        {+3.575_m, +1.513_m, +0.000_m}, {+3.272_m, +1.665_m, +0.000_m}, {+0.776_m, +1.778_m, +0.000_m},
		        {-0.019_m, +2.194_m, +0.000_m}, {-2.137_m, +5.372_m, +0.000_m}, {-3.651_m, +5.372_m, +0.000_m},
		        {-4.559_m, +1.854_m, +0.000_m}, {-4.937_m, +1.967_m, +0.000_m}, {-5.883_m, +3.443_m, +0.000_m},
		        {-7.320_m, +3.405_m, +0.000_m}, {-7.850_m, +1.059_m, +0.000_m}, {-5.845_m, +0.719_m, +0.000_m},
		        {-5.807_m, +0.530_m, +0.000_m}, {-6.185_m, +0.378_m, +0.000_m}, {-6.185_m, -0.378_m, +0.000_m},
		        {-5.845_m, -0.492_m, +0.000_m}, {-5.807_m, -0.681_m, +0.000_m}, {-7.812_m, -1.021_m, +0.000_m}};
	}

	/// The canopy bubble, a pointed oval on the forward fuselage (windscreen apex aft, rounded point forward).
	inline constexpr CartesianVector canopy()
	{
		return {
		        {+3.045_m, +0.000_m, +0.000_m}, {+3.272_m, -0.303_m, +0.000_m}, {+3.651_m, -0.416_m, +0.000_m},
		        {+4.332_m, -0.454_m, +0.000_m}, {+5.013_m, -0.454_m, +0.000_m}, {+5.542_m, -0.378_m, +0.000_m},
		        {+5.921_m, -0.265_m, +0.000_m}, {+6.148_m, -0.113_m, +0.000_m}, {+6.223_m, +0.000_m, +0.000_m},
		        {+6.148_m, +0.113_m, +0.000_m}, {+5.921_m, +0.265_m, +0.000_m}, {+5.542_m, +0.378_m, +0.000_m},
		        {+5.013_m, +0.454_m, +0.000_m}, {+4.332_m, +0.454_m, +0.000_m}, {+3.651_m, +0.416_m, +0.000_m},
		        {+3.272_m, +0.303_m, +0.000_m}};
	}

	/// The left vertical stabilizer: a 4-point swept fin, root chord on the boom at z=0, tip chord raked aft and
	/// lifted up (-z), canted outboard, so the fin stands out of the planform and tilts under roll. Profile (leading
	/// edge rake, trailing edge, taper) traced from the side view. Vertices: root-LE, tip-LE, tip-TE, root-TE.
	inline constexpr CartesianVector finLeft()
	{
		return {{-3.730_m, -1.420_m, +0.000_m}, {-5.694_m, -2.150_m, -1.922_m}, {-6.962_m, -2.150_m, -1.922_m}, {-6.307_m, -1.420_m, +0.000_m}};
	}

	/// The right vertical stabilizer (the y-mirror of the left).
	inline constexpr CartesianVector finRight()
	{
		return {{-3.730_m, +1.420_m, +0.000_m}, {-5.694_m, +2.150_m, -1.922_m}, {-6.962_m, +2.150_m, -1.922_m}, {-6.307_m, +1.420_m, +0.000_m}};
	}

	/// The fraction of the fin chord occupied by the rudder (the aft strip), measured from the side view.
	inline constexpr double rudderChordFraction() { return 0.20; }

	//	----------------------------------------------------------------------------
	//	ALL-MOVING HORIZONTAL TAILS (stabilators) -- the WHOLE surface pivots about a
	//	spanwise quarter-chord axis: together for pitch, differential for roll.
	//	----------------------------------------------------------------------------

	/// The right all-moving horizontal tail (stabilator) as a closed quad, DERIVED from the outline's taileron
	/// vertices (root-LE, tip-LE, tip-TE, root-TE = outline indices 25, 26, 27, 28), so it shares those points with
	/// the body and never drifts. The whole surface pivots for pitch/roll.
	inline CartesianVector taileronRight()
	{
		const auto& o = outline();
		return {o[25], o[26], o[27], o[28]};    // root-LE, tip-LE, tip-TE, root-TE
	}

	/// The left all-moving horizontal tail (the y-mirror; outline indices 3, 2, 1, 0 = root-LE, tip-LE, tip-TE, root-TE).
	inline CartesianVector taileronLeft()
	{
		const auto& o = outline();
		return {o[3], o[2], o[1], o[0]};
	}

	/// The fraction of the stabilator chord at which the spanwise pivot shaft sits (the aerodynamic center).
	inline constexpr double taileronPivotChordFraction() { return 0.25; }

	/// The stabilator's spanwise pivot axis: the quarter-chord points at the root and tip. The surface rotates
	/// about the line through them (leading edge up/down). `t` is `taileronRight()`/`taileronLeft()`.
	inline std::array<CartesianTuple, 2> taileronPivot(const CartesianVector& t)
	{
		const double         f       = taileronPivotChordFraction();
		const CartesianTuple rootLE = t[0], tipLE = t[1], tipTE = t[2], rootTE = t[3];
		const CartesianTuple rootPivot = rootLE + (rootTE - rootLE) * f;
		const CartesianTuple tipPivot  = tipLE + (tipTE - tipLE) * f;
		return {rootPivot, tipPivot};
	}

	/// A stabilator deflected by `angle` about its spanwise quarter-chord pivot -- rotate the whole quad about the
	/// pivot axis through the root pivot point. Positive angle pitches the leading edge up (trailing edge down).
	inline CartesianVector deflectTaileron(const CartesianVector& t, radians<> angle)
	{
		const auto                 pivot = taileronPivot(t);
		const auto                 axis  = (pivot[1] - pivot[0]).normalized();
		const rotation::Quaternion q =
		        rotation::toQuaternion(rotation::AxisAngle(axis.x().value(), axis.y().value(), axis.z().value(), angle));
		CartesianVector deflected;
		for (const CartesianTuple& v : t)
			deflected.push_back(q.rotate(v - pivot[0]) + pivot[0]);
		return deflected;
	}

	/// A fin's rudder panel: a 4-point quad on the aft strip of the fin, DERIVED from the fin's trailing-edge
	/// corners (tip = index 2, root = index 4) pulled forward toward the leading edge by the rudder chord, so the
	/// panel shares the fin's trailing edge and hinges on a straight line parallel to it. `fin` is `finLeft()` or
	/// `finRight()`.
	inline CartesianVector rudderPanel(const CartesianVector& fin)
	{
		const CartesianTuple leadingRoot = fin[0], leadingTip = fin[1];    // fin leading edge (root, tip)
		const CartesianTuple trailTip = fin[2], trailRoot = fin[3];        // fin trailing edge (tip, root)
		const double         f = rudderChordFraction();
		const CartesianTuple hingeTip  = trailTip + (leadingTip - trailTip) * f;
		const CartesianTuple hingeRoot = trailRoot + (leadingRoot - trailRoot) * f;
		return {trailTip, trailRoot, hingeRoot, hingeTip};    // TE (tip->root) then hinge back (root->tip)
	}

	/// The FIXED part of a fin with its rudder CUT OUT: the fin outline up to the rudder hinge line (leading edge,
	/// tip, then the hinge instead of the trailing edge), so the aft strip belongs to the deflecting rudder panel.
	/// Shares the hinge points with `rudderPanel`, so the fixed fin and the rudder always meet exactly.
	inline CartesianVector rudderCutFin(const CartesianVector& fin)
	{
		const CartesianVector rudder = rudderPanel(fin);    // {trailTip, trailRoot, hingeRoot, hingeTip}
		// Fin without the aft strip: root-LE, tip-LE, then down the hinge (tipHinge -> rootHinge).
		return {fin[0], fin[1], rudder[3], rudder[2]};
	}

	/// A fin's rudder deflected by `angle` about its hinge -- the near-vertical forward edge of the rudder panel
	/// (hingeRoot->hingeTip). Positive angle yaws the trailing edge to one side. `fin` is `finLeft()`/`finRight()`.
	inline CartesianVector deflectRudder(const CartesianVector& fin, radians<> angle)
	{
		const CartesianVector      panel     = rudderPanel(fin);
		const CartesianTuple       hingeRoot = panel[2], hingeTip = panel[3];    // the rudder hinge line
		const auto                 axis      = (hingeTip - hingeRoot).normalized();
		const rotation::Quaternion q =
		        rotation::toQuaternion(rotation::AxisAngle(axis.x().value(), axis.y().value(), axis.z().value(), angle));
		CartesianVector deflected;
		for (const CartesianTuple& v : panel)
			deflected.push_back(q.rotate(v - hingeRoot) + hingeRoot);
		return deflected;
	}

	//	----------------------------------------------------------------------------
	//	CONTROL SURFACES (derived from the outline -- never a duplicated edge table)
	//	----------------------------------------------------------------------------

	/// A wing flap described purely by reference to the SIMPLE outline: the wing edge it lies on is the outline
	/// segment `outline()[edgeStart] -> outline()[edgeEnd]`; the flap spans the fraction `[spanStart, spanEnd]` of
	/// that segment and has chord `chord` inboard of it. Its geometry is DERIVED from the outline (single source of
	/// truth), so the flap can never drift off the wing and the outline edge is never copied.
	struct Flap
	{
		std::size_t edgeStart;    ///< index into `outline()` of the wing-edge segment's first vertex
		std::size_t edgeEnd;      ///< index into `outline()` of the wing-edge segment's second vertex
		double      spanStart;    ///< where the flap begins along the edge, as a fraction [0,1]
		double      spanEnd;      ///< where the flap ends along the edge, as a fraction [0,1]
		meters<>    chord;        ///< the flap chord, offset inboard (normal to the edge, into the wing)
	};

	/// The inboard unit normal of a flap's wing edge -- points from the leading edge into the wing interior, i.e.
	/// toward the fuselage centerline (the normal whose lateral component opposes the edge's own side).
	inline constexpr CartesianTuple flapInboardNormal(const Flap& flap)
	{
		const auto&          o    = outline();
		const auto           d    = (o[flap.edgeEnd] - o[flap.edgeStart]).normalized();
		const CartesianTuple n(d.y() * 1.0_m, -d.x() * 1.0_m, 0.0_m);    // in-plane normal, unit length
		// Inboard is toward the centerline: choose the sign so the normal points opposite the edge's y-side.
		const auto side = o[flap.edgeStart].y() + o[flap.edgeEnd].y();    // >0 on the right wing, <0 on the left
		const bool pointsInboard = (n.y() * side) < 0.0_m * 1.0_m;
		return pointsInboard ? n : CartesianTuple(-n.x(), -n.y(), 0.0_m);
	}

	/// The flap's two hinge-line endpoints -- the inboard edge about which it rotates. On the ARTICULATED airplane
	/// the body outline runs along this line where the flap is (the outer edge belongs to the flap).
	inline constexpr std::array<CartesianTuple, 2> flapHinge(const Flap& flap)
	{
		const auto&          o = outline();
		const CartesianTuple a = o[flap.edgeStart] + (o[flap.edgeEnd] - o[flap.edgeStart]) * flap.spanStart;
		const CartesianTuple b = o[flap.edgeStart] + (o[flap.edgeEnd] - o[flap.edgeStart]) * flap.spanEnd;
		const CartesianTuple n = flapInboardNormal(flap);
		return {a + n * (flap.chord / 1.0_m), b + n * (flap.chord / 1.0_m)};
	}

	/// The flap panel as a closed quad: the outer wing edge (shared with the outline) plus the hinge line back.
	inline constexpr CartesianVector flapPanel(const Flap& flap)
	{
		const auto&          o = outline();
		const CartesianTuple a = o[flap.edgeStart] + (o[flap.edgeEnd] - o[flap.edgeStart]) * flap.spanStart;
		const CartesianTuple b = o[flap.edgeStart] + (o[flap.edgeEnd] - o[flap.edgeStart]) * flap.spanEnd;
		const auto           hinge = flapHinge(flap);
		return {a, b, hinge[1], hinge[0]};
	}

	/// The leading-edge flaps, one per wing, defined by reference to the outline's wing-LE segments (right wing
	/// LE is outline[21]->[22], left wing LE is outline[6]->[7]). Chord and span measured from the reference.
	inline const std::array<Flap, 2>& leadingEdgeFlaps()
	{
		static const std::array<Flap, 2> flaps{{
		        {21, 22, 0.10, 0.92, 0.40_m},    // right wing
		        {6, 7, 0.10, 0.92, 0.40_m},      // left wing
		}};
		return flaps;
	}

	/// The trailing-edge flaperons, one per wing, on the outline's wing-TE segments (right wing TE is
	/// outline[23]->[24], left wing TE is outline[4]->[5]). Chord and span measured from the reference.
	inline const std::array<Flap, 2>& trailingEdgeFlaps()
	{
		static const std::array<Flap, 2> flaps{{
		        {23, 24, 0.08, 0.90, 0.55_m},    // right wing
		        {4, 5, 0.10, 0.92, 0.55_m},      // left wing
		}};
		return flaps;
	}

	//	----------------------------------------------------------------------------
	//	INTERIOR DETAIL LINES (the DETAILED airplane only -- panels, seams, intakes,
	//	engine-face hatching; none overlap the structural outline/flaps/tails).
	//	----------------------------------------------------------------------------

	/// The upper DSI intake lip (inner oval line).
	inline constexpr CartesianVector intakeUpper()
	{
		return {
		        {-0.397_m, -1.702_m, +0.000_m}, {+0.019_m, -1.702_m, +0.000_m}, {+0.284_m, -1.513_m, +0.000_m},
		        {+1.721_m, -1.513_m, +0.000_m}, {+1.759_m, -1.627_m, +0.000_m}, {+2.062_m, -1.702_m, +0.000_m},
		        {+2.516_m, -1.702_m, +0.000_m}, {+2.516_m, -1.475_m, +0.000_m}, {-0.397_m, -1.475_m, +0.000_m},
		        {-0.397_m, -1.702_m, +0.000_m}};
	}

	/// The lower DSI intake lip (inner oval line).
	inline constexpr CartesianVector intakeLower()
	{
		return {
		        {-0.397_m, +1.702_m, +0.000_m}, {-0.397_m, +1.475_m, +0.000_m}, {+2.516_m, +1.475_m, +0.000_m},
		        {+2.516_m, +1.702_m, +0.000_m}, {+2.062_m, +1.702_m, +0.000_m}, {+1.759_m, +1.627_m, +0.000_m},
		        {+1.721_m, +1.513_m, +0.000_m}, {+0.284_m, +1.513_m, +0.000_m}, {+0.019_m, +1.702_m, +0.000_m},
		        {-0.397_m, +1.702_m, +0.000_m}};
	}

	/// The upper DSI intake outer perimeter.
	inline constexpr CartesianVector intakeUpperOuter()
	{
		return {
		        {-1.003_m, -1.438_m, +0.000_m}, {+0.322_m, -1.702_m, +0.000_m}, {+1.721_m, -1.627_m, +0.000_m},
		        {+2.667_m, -1.702_m, +0.000_m}, {+2.856_m, -1.665_m, +0.000_m}, {+2.743_m, -1.551_m, +0.000_m},
		        {+2.894_m, -1.475_m, +0.000_m}, {+2.894_m, -1.362_m, +0.000_m}, {+2.667_m, -1.173_m, +0.000_m},
		        {+2.024_m, -1.173_m, +0.000_m}, {+1.986_m, -1.248_m, +0.000_m}, {+0.322_m, -1.173_m, +0.000_m},
		        {-0.813_m, -1.286_m, +0.000_m}, {-1.003_m, -1.438_m, +0.000_m}};
	}

	/// The lower DSI intake outer perimeter.
	inline constexpr CartesianVector intakeLowerOuter()
	{
		return {
		        {-1.003_m, +1.438_m, +0.000_m}, {-0.813_m, +1.286_m, +0.000_m}, {+0.322_m, +1.173_m, +0.000_m},
		        {+1.986_m, +1.248_m, +0.000_m}, {+2.024_m, +1.173_m, +0.000_m}, {+2.667_m, +1.173_m, +0.000_m},
		        {+2.894_m, +1.362_m, +0.000_m}, {+2.894_m, +1.475_m, +0.000_m}, {+2.743_m, +1.551_m, +0.000_m},
		        {+2.856_m, +1.665_m, +0.000_m}, {+2.667_m, +1.702_m, +0.000_m}, {+1.721_m, +1.627_m, +0.000_m},
		        {+0.322_m, +1.702_m, +0.000_m}, {-1.003_m, +1.438_m, +0.000_m}};
	}

	/// The upper fuselage-side spine seam.
	inline constexpr CartesianVector spineSeamUpper()
	{
		return {
		        {-5.429_m, -0.605_m, +0.000_m}, {-3.386_m, -0.643_m, +0.000_m}, {+1.835_m, -0.416_m, +0.000_m},
		        {+3.197_m, -0.574_m, +0.000_m}, {+3.651_m, -0.504_m, +0.000_m}, {+4.332_m, -0.631_m, +0.000_m},
		        {+5.240_m, -0.562_m, +0.000_m}};
	}

	/// The lower fuselage-side spine seam.
	inline constexpr CartesianVector spineSeamLower()
	{
		return {
		        {+5.240_m, +0.562_m, +0.000_m}, {+4.332_m, +0.631_m, +0.000_m}, {+3.651_m, +0.504_m, +0.000_m},
		        {+3.197_m, +0.574_m, +0.000_m}, {+1.835_m, +0.416_m, +0.000_m}, {-3.386_m, +0.643_m, +0.000_m},
		        {-5.429_m, +0.605_m, +0.000_m}};
	}

	/// The forward centerline weapons-bay / gun panel.
	inline constexpr CartesianVector weaponsBox()
	{
		return {
		        {+0.359_m, -0.265_m, +0.000_m}, {+0.738_m, -0.227_m, +0.000_m}, {+0.965_m, -0.265_m, +0.000_m},
		        {+1.003_m, -0.378_m, +0.000_m}, {+1.343_m, -0.378_m, +0.000_m}, {+1.381_m, -0.227_m, +0.000_m},
		        {+1.230_m, -0.189_m, +0.000_m}, {+1.230_m, -0.038_m, +0.000_m}, {+1.381_m, +0.000_m, +0.000_m},
		        {+1.343_m, +0.378_m, +0.000_m}, {+0.662_m, +0.227_m, +0.000_m}, {+0.359_m, +0.303_m, +0.000_m},
		        {+0.359_m, +0.000_m, +0.000_m}, {+0.473_m, -0.038_m, +0.000_m}, {+0.359_m, -0.265_m, +0.000_m}};
	}

	/// The refuel receptacle panel on the spine.
	inline constexpr CartesianVector spineReceptacle()
	{
		return {
		        {-0.435_m, +0.000_m, +0.000_m}, {-0.208_m, -0.265_m, +0.000_m}, {+0.397_m, -0.265_m, +0.000_m},
		        {+0.473_m, -0.189_m, +0.000_m}, {+0.359_m, -0.265_m, +0.000_m}, {-0.208_m, -0.265_m, +0.000_m},
		        {-0.322_m, -0.113_m, +0.000_m}, {-0.284_m, +0.000_m, +0.000_m}, {+0.473_m, +0.000_m, +0.000_m},
		        {-0.322_m, +0.076_m, +0.000_m}, {-0.208_m, +0.303_m, +0.000_m}, {+0.359_m, +0.303_m, +0.000_m},
		        {+0.473_m, +0.189_m, +0.000_m}, {+0.359_m, +0.303_m, +0.000_m}, {-0.208_m, +0.303_m, +0.000_m},
		        {-0.435_m, +0.000_m, +0.000_m}};
	}

	/// The refuel receptacle divider bar.
	inline constexpr CartesianVector spineReceptacleBar()
	{
		return {
		        {-0.322_m, +0.000_m, +0.000_m}, {+0.435_m, +0.000_m, +0.000_m}};
	}

	/// The hexagonal access panel on the boom.
	inline constexpr CartesianVector hexPanel()
	{
		return {
		        {-3.613_m, -0.076_m, +0.000_m}, {-3.424_m, -0.227_m, +0.000_m}, {-3.159_m, -0.227_m, +0.000_m},
		        {-3.045_m, +0.076_m, +0.000_m}, {-3.159_m, +0.189_m, +0.000_m}, {-3.424_m, +0.189_m, +0.000_m},
		        {-3.613_m, +0.076_m, +0.000_m}, {-3.613_m, -0.076_m, +0.000_m}};
	}

	/// The small port on the boom, forward of the weapons panel.
	inline constexpr CartesianVector roundPort()
	{
		return {
		        {+0.700_m, -0.189_m, +0.000_m}, {+0.927_m, -0.189_m, +0.000_m}, {+0.851_m, -0.113_m, +0.000_m},
		        {+0.851_m, +0.113_m, +0.000_m}, {+0.927_m, +0.151_m, +0.000_m}, {+0.700_m, +0.151_m, +0.000_m},
		        {+0.776_m, +0.113_m, +0.000_m}, {+0.776_m, -0.113_m, +0.000_m}, {+0.700_m, -0.189_m, +0.000_m}};
	}

	/// Engine nozzle hatch stroke 1.
	inline constexpr CartesianVector nozzleHatch1()
	{
		return {
		        {-5.883_m, -0.492_m, +0.000_m}, {-5.504_m, -0.492_m, +0.000_m}};
	}

	/// Engine nozzle hatch stroke 2.
	inline constexpr CartesianVector nozzleHatch2()
	{
		return {
		        {-6.185_m, -0.340_m, +0.000_m}, {-5.504_m, -0.340_m, +0.000_m}};
	}

	/// Engine nozzle hatch stroke 3.
	inline constexpr CartesianVector nozzleHatch3()
	{
		return {
		        {-6.185_m, -0.189_m, +0.000_m}, {-5.504_m, -0.189_m, +0.000_m}};
	}

	/// Engine nozzle hatch stroke 4.
	inline constexpr CartesianVector nozzleHatch4()
	{
		return {
		        {-6.185_m, -0.038_m, +0.000_m}, {-5.504_m, -0.038_m, +0.000_m}};
	}

	/// Engine nozzle hatch stroke 5.
	inline constexpr CartesianVector nozzleHatch5()
	{
		return {
		        {-6.185_m, +0.113_m, +0.000_m}, {-5.504_m, +0.113_m, +0.000_m}};
	}

	/// Engine nozzle hatch stroke 6.
	inline constexpr CartesianVector nozzleHatch6()
	{
		return {
		        {-6.185_m, +0.265_m, +0.000_m}, {-5.504_m, +0.265_m, +0.000_m}};
	}

	/// Engine nozzle hatch stroke 7.
	inline constexpr CartesianVector nozzleHatch7()
	{
		return {
		        {-6.110_m, +0.416_m, +0.000_m}, {-5.504_m, +0.416_m, +0.000_m}};
	}

	/// The upper wing panel dash line.
	inline constexpr CartesianVector wingDashUpper()
	{
		return {
		        {-1.873_m, -3.594_m, +0.000_m}, {-1.381_m, -2.837_m, +0.000_m}};
	}

	/// The lower wing panel dash line.
	inline constexpr CartesianVector wingDashLower()
	{
		return {
		        {-1.381_m, +2.837_m, +0.000_m}, {-1.873_m, +3.594_m, +0.000_m}};
	}

	/// The canopy windscreen bow frame.
	inline constexpr CartesianVector windscreenBow()
	{
		return {
		        {+6.337_m, -0.227_m, +0.000_m}, {+6.337_m, +0.227_m, +0.000_m}};
	}

	/// The aft canopy bulkhead frame.
	inline constexpr CartesianVector canopyAftFrame()
	{
		return {
		        {+3.575_m, -0.378_m, +0.000_m}, {+3.575_m, +0.643_m, +0.000_m}};
	}

	/// The vertical-stabilizer root fairing vent.
	inline constexpr CartesianVector vstabFairingUpper()
	{
		return {
		        {-3.386_m, -1.248_m, +0.000_m}, {-2.970_m, -1.400_m, +0.000_m}, {-2.970_m, -1.059_m, +0.000_m},
		        {-3.235_m, -1.097_m, +0.000_m}, {-3.386_m, -1.248_m, +0.000_m}};
	}

	/// Every planform part, in draw order -- the single list both the runtime attach and the compile-time build
	/// iterate, so a part is added in exactly one place. The simple airplane: outline, canopy, and z-aware vstabs.
	inline const std::array<CartesianVector, 4>& parts()
	{
		static const std::array<CartesianVector, 4> tables{outline(), canopy(), finLeft(), finRight()};
		return tables;
	}

	/// The total number of planform vertices across every part -- a compile-time constant that sizes the baked
	/// vertex array (so the capacity is DERIVED from the data, never a hand-picked number).
	inline constexpr std::size_t vertexCount()
	{
		return outline().size() + canopy().size() + finLeft().size() + finRight().size();
	}

	/// Draw the whole F-35 at the given attitude onto a view -- every planform part posed and stroked (black by
	/// default) as a closed loop by the library's `drawPolyline`. The view carries its own camera and image.
	inline void draw(topography::View& view, const Pose& attitude, topography::Color color = {})
	{
		for (const CartesianVector& part : parts())
			topography::drawPolyline(view, attitude, part, color);
	}

	/// The ARTICULATED body outline: the simple outline with each flap CUT OUT. Where a flap lies on a wing edge,
	/// the body detours inboard along the flap's hinge line (a notch), because the outer edge belongs to the flap
	/// panel, which is drawn separately and rotates about that hinge. The notch is DERIVED from the same outline
	/// segment and `Flap` the panel uses, so the two always share the hinge exactly.
	inline CartesianVector articulatedOutline()
	{
		const auto& o = outline();

		// Carve one flap's notch into the body where it sits on the segment o[i]->o[next]: run out to the flap's
		// inset start, in to the hinge, along the hinge, back out to the inset end. Shared with the panel, so the
		// body edge and the flap's hinge coincide exactly.
		const auto carve = [&](CartesianVector& body, std::size_t i, std::size_t next, const Flap& flap) {
			if (flap.edgeStart != i || flap.edgeEnd != next)
				return;
			const CartesianTuple a     = o[i] + (o[next] - o[i]) * flap.spanStart;
			const CartesianTuple b     = o[i] + (o[next] - o[i]) * flap.spanEnd;
			const auto           hinge = flapHinge(flap);
			body.push_back(a);
			body.push_back(hinge[0]);
			body.push_back(hinge[1]);
			body.push_back(b);
		};

		// The all-moving tailerons are cut out entirely: the body skips each taileron's outer trapezoid vertices
		// and runs straight along its root chord (root-LE to root-TE, on the fuselage boom), because the whole
		// surface belongs to the deflecting stabilator. Right taileron outer vertices = {26,27}, left = {1,2}
		// (the root corners 25/28 and 3/0 stay, so the body runs straight along each root chord).
		const auto skipped = [](std::size_t i) { return i == 26 || i == 27 || i == 1 || i == 2; };

		CartesianVector body;
		for (std::size_t i = 0; i < o.size(); ++i)
		{
			if (skipped(i))
				continue;
			body.push_back(o[i]);
			const std::size_t next = (i + 1) % o.size();
			for (const Flap& flap : leadingEdgeFlaps())
				carve(body, i, next, flap);
			for (const Flap& flap : trailingEdgeFlaps())
				carve(body, i, next, flap);
		}
		return body;
	}

	/// Draw the SIMPLE airplane at the given attitude: outline (flaps flush), canopy, and z-aware vstabs.
	inline void simple(topography::View& view, const Pose& attitude, topography::Color color = {})
	{
		draw(view, attitude, color);
	}

	/// Draw the DETAILED airplane: the simple airplane plus every interior detail line -- the intakes, fuselage
	/// seams, weapons-bay and boom panels, engine-face hatching, wing dashes, and canopy framing. The detail is
	/// fixed structure that sits clear of the control-surface edges, so it draws only here, not on the simple jet.
	inline void detailed(topography::View& view, const Pose& attitude, topography::Color color = {})
	{
		simple(view, attitude, color);
		const CartesianVector closed[] = {intakeUpper(), intakeLower(), intakeUpperOuter(), intakeLowerOuter(),
		                                  weaponsBox(), spineReceptacle(), hexPanel(), roundPort(), vstabFairingUpper()};
		for (const CartesianVector& part : closed)
			topography::drawPolyline(view, attitude, part, color);

		// Open seams (not closed loops): draw each as consecutive 2-point segments so the polyline is not wrapped
		// end-to-start. A 2-point segment drawn as a loop simply retraces itself, which is harmless.
		const auto drawOpen = [&](const CartesianVector& seam) {
			for (std::size_t i = 0; i + 1 < seam.size(); ++i)
				topography::drawPolyline(view, attitude, CartesianVector{seam[i], seam[i + 1]}, color);
		};
		const CartesianVector open[] = {spineSeamUpper(), spineSeamLower(), spineReceptacleBar(),
		                                nozzleHatch1(), nozzleHatch2(), nozzleHatch3(), nozzleHatch4(), nozzleHatch5(),
		                                nozzleHatch6(), nozzleHatch7(), wingDashUpper(), wingDashLower(),
		                                windscreenBow(), canopyAftFrame()};
		for (const CartesianVector& seam : open)
			drawOpen(seam);
	}

	/// A flap panel deflected about its hinge by `angle` -- rotate every panel vertex about the hinge axis through
	/// the hinge point. The axis runs hinge[1]->hinge[0] so a positive angle drops the free edge DOWN (+z is down).
	inline CartesianVector deflectFlap(const Flap& flap, radians<> angle)
	{
		const auto                 hinge = flapHinge(flap);
		const auto                 axis  = (hinge[0] - hinge[1]).normalized();    // dimensionless direction
		const rotation::Quaternion q =
		        rotation::toQuaternion(rotation::AxisAngle(axis.x().value(), axis.y().value(), axis.z().value(), angle));
		CartesianVector deflected;
		for (const CartesianTuple& v : flapPanel(flap))
			deflected.push_back(q.rotate(v - hinge[0]) + hinge[0]);
		return deflected;
	}

	/// The commanded deflection of each control-surface family (right-hand about each hinge axis, free edge down).
	struct Deflections
	{
		radians<> leadingEdgeRight{0.0};     ///< right wing leading-edge flap
		radians<> leadingEdgeLeft{0.0};      ///< left wing leading-edge flap
		radians<> trailingEdgeRight{0.0};    ///< right wing trailing-edge flaperon
		radians<> trailingEdgeLeft{0.0};     ///< left wing trailing-edge flaperon
		radians<> rudderRight{0.0};          ///< right fin rudder (yaw)
		radians<> rudderLeft{0.0};           ///< left fin rudder (yaw)
		radians<> taileronRight{0.0};        ///< right all-moving horizontal tail (pitch together, roll differential)
		radians<> taileronLeft{0.0};         ///< left all-moving horizontal tail
	};

	/// Draw the ARTICULATED airplane: the flap-cut body outline, canopy, vstabs, and each control surface deflected
	/// about its hinge by the commanded angle. The surfaces are posed through the airframe attitude like every other
	/// loop, so they foreshorten and tilt with the maneuver.
	inline void articulated(topography::View& view, const Pose& attitude, const Deflections& deflections, topography::Color color = {})
	{
		topography::drawPolyline(view, attitude, articulatedOutline(), color);
		topography::drawPolyline(view, attitude, canopy(), color);
		topography::drawPolyline(view, attitude, rudderCutFin(finLeft()), color);     // fin with rudder cut out
		topography::drawPolyline(view, attitude, rudderCutFin(finRight()), color);

		const auto& le = leadingEdgeFlaps();     // [0] = right wing, [1] = left wing
		const auto& te = trailingEdgeFlaps();
		topography::drawPolyline(view, attitude, deflectFlap(le[0], deflections.leadingEdgeRight), color);
		topography::drawPolyline(view, attitude, deflectFlap(le[1], deflections.leadingEdgeLeft), color);
		topography::drawPolyline(view, attitude, deflectFlap(te[0], deflections.trailingEdgeRight), color);
		topography::drawPolyline(view, attitude, deflectFlap(te[1], deflections.trailingEdgeLeft), color);
		topography::drawPolyline(view, attitude, deflectRudder(finRight(), deflections.rudderRight), color);
		topography::drawPolyline(view, attitude, deflectRudder(finLeft(), deflections.rudderLeft), color);
		topography::drawPolyline(view, attitude, deflectTaileron(taileronRight(), deflections.taileronRight), color);
		topography::drawPolyline(view, attitude, deflectTaileron(taileronLeft(), deflections.taileronLeft), color);
	}

	//----------------------------------------------------------------------------------------------------------------------
	//	FUNCTION: buildF35 [consteval]
	//----------------------------------------------------------------------------------------------------------------------
	/// @brief		Build the F-35 airframe at compile time and bake its world-resolved vertices into a fixed array.
	/// @details	Constructs the airframe as an `Entity` at the given position and pose, attaches every planform
	///				part (outline, canopy, chines, intakes, vents, flaps, strakes, nozzles) as child entities, then
	///				reads each child's resolved WORLD
	///				position and flattens the whole airframe into a `std::array<CartesianTuple, vertexCount()>`. The
	///				`Entity` tree (heap-backed) is transient -- built and consumed inside this one constant
	///				evaluation, so nothing escapes -- while the returned array PERSISTS as a `constexpr` constant.
	///				Being `consteval`, it can only run at compile time, which proves the entire
	///				`Entity`/`Coordinate`/`Pose`/`CartesianTuple` composition is constexpr-clean. The array size is
	///				deduced from the vertex tables, so the caller writes `constexpr auto f35 = buildF35(pos)` with no
	///				capacity to specify.
	/// @param[in]	position	the airframe's world position.
	/// @param[in]	pose		the airframe's attitude (defaults to identity).
	/// @return		the world-resolved planform vertices, in part order (outline first, then the detail loops).
	//----------------------------------------------------------------------------------------------------------------------
	consteval std::array<CartesianTuple, vertexCount()> buildF35(const PositionECEF<Datum>& position, const Pose& pose = Pose::identity())
	{
		Entity<Datum> airframe(position, pose);
		airframe.attach(outline());
		airframe.attach(canopy());
		airframe.attach(finLeft());
		airframe.attach(finRight());

		std::array<CartesianTuple, vertexCount()> vertices{};
		const auto&                               children = airframe.children();
		for (std::size_t i = 0; i < children.size(); ++i)
			vertices[i] = children[i]->position().point();
		return vertices;
	}
}    // namespace f35

#endif    // f35Planform_h
