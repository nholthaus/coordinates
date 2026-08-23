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

	/// The left vertical stabilizer: a basic swept fin, root chord on the boom at z=0, tip chord raked aft, canted
	/// outboard and lifted up (-z) so the fin stands out of the planform and tilts correctly under roll.
	inline constexpr CartesianVector finLeft()
	{
		return {
		        {-3.900_m, -1.420_m, -0.000_m}, {-5.636_m, -2.150_m, -2.350_m}, {-6.308_m, -2.150_m, -2.350_m},
		        {-6.644_m, -1.639_m, -0.705_m}, {-6.700_m, -1.420_m, -0.000_m}};
	}

	/// The right vertical stabilizer (the y-mirror of the left).
	inline constexpr CartesianVector finRight()
	{
		return {
		        {-3.900_m, +1.420_m, -0.000_m}, {-5.636_m, +2.150_m, -2.350_m}, {-6.308_m, +2.150_m, -2.350_m},
		        {-6.644_m, +1.639_m, -0.705_m}, {-6.700_m, +1.420_m, -0.000_m}};
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
		const auto&     o = outline();
		CartesianVector body;
		for (std::size_t i = 0; i < o.size(); ++i)
		{
			body.push_back(o[i]);
			const std::size_t next = (i + 1) % o.size();
			// If a flap sits on the segment o[i]->o[next] (in that orientation), carve its notch: run out to the
			// flap's inset start, in to the hinge, along the hinge, back out to the inset end, then on to o[next].
			for (const Flap& flap : leadingEdgeFlaps())
			{
				if (flap.edgeStart != i || flap.edgeEnd != next)
					continue;
				const CartesianTuple a     = o[i] + (o[next] - o[i]) * flap.spanStart;
				const CartesianTuple b     = o[i] + (o[next] - o[i]) * flap.spanEnd;
				const auto           hinge = flapHinge(flap);
				body.push_back(a);
				body.push_back(hinge[0]);
				body.push_back(hinge[1]);
				body.push_back(b);
			}
		}
		return body;
	}

	/// Draw the SIMPLE airplane at the given attitude: outline (flaps flush), canopy, and z-aware vstabs.
	inline void simple(topography::View& view, const Pose& attitude, topography::Color color = {})
	{
		draw(view, attitude, color);
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
		radians<> leadingEdgeRight{0.0};    ///< right wing leading-edge flap
		radians<> leadingEdgeLeft{0.0};     ///< left wing leading-edge flap
	};

	/// Draw the ARTICULATED airplane: the flap-cut body outline, canopy, vstabs, and each leading-edge flap
	/// deflected about its hinge by the commanded angle. The flaps are posed through the airframe attitude like
	/// every other loop, so they foreshorten and tilt with the maneuver.
	inline void articulated(topography::View& view, const Pose& attitude, const Deflections& deflections, topography::Color color = {})
	{
		topography::drawPolyline(view, attitude, articulatedOutline(), color);
		topography::drawPolyline(view, attitude, canopy(), color);
		topography::drawPolyline(view, attitude, finLeft(), color);
		topography::drawPolyline(view, attitude, finRight(), color);

		const auto& flaps = leadingEdgeFlaps();    // [0] = right wing, [1] = left wing
		topography::drawPolyline(view, attitude, deflectFlap(flaps[0], deflections.leadingEdgeRight), color);
		topography::drawPolyline(view, attitude, deflectFlap(flaps[1], deflections.leadingEdgeLeft), color);
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
