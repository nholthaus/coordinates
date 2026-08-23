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

	/// The canopy bubble.
	inline constexpr CartesianVector canopy()
	{
		return {
		        {+6.119_m, -0.021_m, +0.000_m}, {+6.069_m, +0.112_m, +0.000_m}, {+5.913_m, +0.268_m, +0.000_m},
		        {+5.419_m, +0.473_m, +0.000_m}, {+5.011_m, +0.515_m, +0.000_m}, {+4.960_m, +0.556_m, +0.000_m},
		        {+4.110_m, +0.556_m, +0.000_m}, {+3.855_m, +0.515_m, +0.000_m}, {+3.482_m, +0.390_m, +0.000_m},
		        {+3.483_m, -0.391_m, +0.000_m}, {+3.772_m, -0.474_m, +0.000_m}, {+4.027_m, -0.515_m, +0.000_m},
		        {+4.385_m, -0.515_m, +0.000_m}, {+4.589_m, -0.556_m, +0.000_m}, {+4.640_m, -0.515_m, +0.000_m},
		        {+5.082_m, -0.515_m, +0.000_m}, {+5.456_m, -0.433_m, +0.000_m}, {+5.879_m, -0.261_m, +0.000_m}};
	}

	/// The left forebody chine / LERX shoulder panel.
	inline constexpr CartesianVector lerxLeft()
	{
		return {
		        {+3.523_m, -1.463_m, +0.000_m}, {+1.213_m, -1.713_m, +0.000_m}, {+0.541_m, -1.875_m, +0.000_m},
		        {-0.208_m, -2.328_m, +0.000_m}, {-0.624_m, -2.396_m, +0.000_m}, {-0.939_m, -0.639_m, +0.000_m},
		        {+0.761_m, -0.639_m, +0.000_m}, {+2.154_m, -0.845_m, +0.000_m}, {+3.145_m, -1.126_m, +0.000_m}};
	}

	/// The right forebody chine / LERX shoulder panel.
	inline constexpr CartesianVector lerxRight()
	{
		return {
		        {+3.523_m, +1.463_m, +0.000_m}, {+1.213_m, +1.713_m, +0.000_m}, {+0.541_m, +1.875_m, +0.000_m},
		        {-0.208_m, +2.328_m, +0.000_m}, {-0.624_m, +2.396_m, +0.000_m}, {-0.939_m, +0.639_m, +0.000_m},
		        {+0.761_m, +0.639_m, +0.000_m}, {+2.154_m, +0.845_m, +0.000_m}, {+3.145_m, +1.126_m, +0.000_m}};
	}

	/// The left DSI intake lip.
	inline constexpr CartesianVector intakeLeft()
	{
		return {
		        {-0.927_m, -1.216_m, +0.000_m}, {-1.135_m, -1.010_m, +0.000_m}, {-2.404_m, -1.010_m, +0.000_m},
		        {-2.285_m, -1.092_m, +0.000_m}, {-2.206_m, -1.092_m, +0.000_m}, {-2.081_m, -1.174_m, +0.000_m},
		        {-1.956_m, -1.216_m, +0.000_m}, {-1.837_m, -1.216_m, +0.000_m}, {-1.790_m, -1.257_m, +0.000_m},
		        {-1.053_m, -1.257_m, +0.000_m}};
	}

	/// The right DSI intake lip.
	inline constexpr CartesianVector intakeRight()
	{
		return {
		        {-0.927_m, +1.257_m, +0.000_m}, {-1.093_m, +1.258_m, +0.000_m}, {-1.140_m, +1.298_m, +0.000_m},
		        {-1.663_m, +1.298_m, +0.000_m}, {-1.710_m, +1.257_m, +0.000_m}, {-1.876_m, +1.256_m, +0.000_m},
		        {-2.040_m, +1.174_m, +0.000_m}, {-2.245_m, +1.133_m, +0.000_m}, {-2.370_m, +1.051_m, +0.000_m},
		        {-1.135_m, +1.051_m, +0.000_m}};
	}

	/// The right wing leading-edge flap: its outboard edge IS the wing leading-edge segment of `outline()` (shared
	/// vertices, so it can never drift off the wing), closed by a hinge line offset a flap chord inboard.
	inline constexpr CartesianVector leadingEdgeFlapRight()
	{
		return {
		        {+0.035_m, +2.190_m, +0.000_m}, {-0.391_m, +2.744_m, +0.000_m}, {-0.805_m, +3.401_m, +0.000_m},
		        {-1.571_m, +4.496_m, +0.000_m}, {-2.165_m, +5.419_m, +0.000_m}, {-2.429_m, +5.239_m, +0.000_m},
		        {-1.835_m, +4.316_m, +0.000_m}, {-1.069_m, +3.221_m, +0.000_m}, {-0.655_m, +2.564_m, +0.000_m},
		        {-0.229_m, +2.010_m, +0.000_m}};
	}

	/// The left wing leading-edge flap (the y-mirror of the right).
	inline constexpr CartesianVector leadingEdgeFlapLeft()
	{
		return {
		        {+0.035_m, -2.190_m, +0.000_m}, {-0.391_m, -2.744_m, +0.000_m}, {-0.805_m, -3.401_m, +0.000_m},
		        {-1.571_m, -4.496_m, +0.000_m}, {-2.165_m, -5.419_m, +0.000_m}, {-2.429_m, -5.239_m, +0.000_m},
		        {-1.835_m, -4.316_m, +0.000_m}, {-1.069_m, -3.221_m, +0.000_m}, {-0.655_m, -2.564_m, +0.000_m},
		        {-0.229_m, -2.010_m, +0.000_m}};
	}

	/// The left engine exhaust nozzle.
	inline constexpr CartesianVector nozzleLeft()
	{
		return {
		        {-5.460_m, -2.246_m, +0.000_m}, {-5.685_m, -2.328_m, +0.000_m}, {-6.807_m, -2.287_m, +0.000_m},
		        {-7.107_m, -2.204_m, +0.000_m}, {-6.333_m, -1.635_m, +0.000_m}, {-6.531_m, -0.895_m, +0.000_m},
		        {-7.196_m, -1.010_m, +0.000_m}, {-7.800_m, -1.051_m, +0.000_m}, {-7.438_m, -2.468_m, +0.000_m},
		        {-7.273_m, -3.228_m, +0.000_m}, {-7.138_m, -3.606_m, +0.000_m}, {-6.292_m, -3.606_m, +0.000_m}};
	}

	/// The right engine exhaust nozzle.
	inline constexpr CartesianVector nozzleRight()
	{
		return {
		        {-5.460_m, +2.246_m, +0.000_m}, {-6.125_m, +3.364_m, +0.000_m}, {-6.347_m, +3.647_m, +0.000_m},
		        {-7.147_m, +3.647_m, +0.000_m}, {-7.397_m, +2.617_m, +0.000_m}, {-7.767_m, +1.299_m, +0.000_m},
		        {-7.790_m, +1.051_m, +0.000_m}, {-6.491_m, +0.968_m, +0.000_m}, {-6.329_m, +1.624_m, +0.000_m},
		        {-7.099_m, +2.237_m, +0.000_m}, {-6.432_m, +2.328_m, +0.000_m}, {-5.606_m, +2.328_m, +0.000_m}};
	}

	/// The sawtooth exhaust lip between the nozzles.
	inline constexpr CartesianVector exhaust()
	{
		return {
		        {-5.584_m, -0.350_m, +0.000_m}, {-5.702_m, -0.139_m, +0.000_m}, {-5.585_m, -0.061_m, +0.000_m},
		        {-5.592_m, +0.094_m, +0.000_m}, {-5.666_m, +0.184_m, +0.000_m}, {-5.586_m, +0.389_m, +0.000_m},
		        {-5.665_m, +0.432_m, +0.000_m}, {-5.668_m, +0.598_m, +0.000_m}, {-6.202_m, +0.345_m, +0.000_m},
		        {-6.202_m, -0.297_m, +0.000_m}, {-5.660_m, -0.592_m, +0.000_m}, {-5.703_m, -0.429_m, +0.000_m}};
	}

	/// The left vertical stabilizer: a swept fin on the boom, root at z=0, tip raked aft and lifted, canted outboard.
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

	/// The left fin's trailing-edge rudder hinge line, on the fin surface.
	inline constexpr CartesianVector rudderLeft()
	{
		return {
		        {-6.196_m, -1.464_m, -0.141_m}, {-6.308_m, -2.150_m, -2.350_m}, {-6.308_m, -2.150_m, -2.350_m},
		        {-6.644_m, -1.639_m, -0.705_m}, {-6.700_m, -1.420_m, -0.000_m}, {-6.644_m, -1.464_m, -0.141_m}};
	}

	/// The right fin's trailing-edge rudder hinge line, on the fin surface.
	inline constexpr CartesianVector rudderRight()
	{
		return {
		        {-6.196_m, +1.464_m, -0.141_m}, {-6.308_m, +2.150_m, -2.350_m}, {-6.308_m, +2.150_m, -2.350_m},
		        {-6.644_m, +1.639_m, -0.705_m}, {-6.700_m, +1.420_m, -0.000_m}, {-6.644_m, +1.464_m, -0.141_m}};
	}

	/// Every planform part, in draw order -- the single list both the runtime attach and the compile-time build
	/// iterate, so a part is added in exactly one place.
	inline const std::array<CartesianVector, 15>& parts()
	{
		static const std::array<CartesianVector, 15> tables{outline(), canopy(), lerxLeft(), lerxRight(), intakeLeft(), intakeRight(), leadingEdgeFlapLeft(), leadingEdgeFlapRight(), nozzleLeft(), nozzleRight(), exhaust(), finLeft(), finRight(), rudderLeft(), rudderRight()};
		return tables;
	}

	/// The total number of planform vertices across every part -- a compile-time constant that sizes the baked
	/// vertex array (so the capacity is DERIVED from the data, never a hand-picked number).
	inline constexpr std::size_t vertexCount()
	{
		return outline().size() + canopy().size() + lerxLeft().size() + lerxRight().size() + intakeLeft().size() + intakeRight().size() + leadingEdgeFlapLeft().size() + leadingEdgeFlapRight().size() + nozzleLeft().size() + nozzleRight().size() + exhaust().size() + finLeft().size() + finRight().size() + rudderLeft().size() + rudderRight().size();
	}

	/// Draw the whole F-35 at the given attitude onto a view -- every planform part posed and stroked (black by
	/// default) as a closed loop by the library's `drawPolyline`. The view carries its own camera and image.
	inline void draw(topography::View& view, const Pose& attitude, topography::Color color = {})
	{
		for (const CartesianVector& part : parts())
			topography::drawPolyline(view, attitude, part, color);
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
		airframe.attach(lerxLeft());
		airframe.attach(lerxRight());
		airframe.attach(intakeLeft());
		airframe.attach(intakeRight());
		airframe.attach(leadingEdgeFlapLeft());
		airframe.attach(leadingEdgeFlapRight());
		airframe.attach(nozzleLeft());
		airframe.attach(nozzleRight());
		airframe.attach(exhaust());
		airframe.attach(finLeft());
		airframe.attach(finRight());
		airframe.attach(rudderLeft());
		airframe.attach(rudderRight());

		std::array<CartesianTuple, vertexCount()> vertices{};
		const auto&                               children = airframe.children();
		for (std::size_t i = 0; i < children.size(); ++i)
			vertices[i] = children[i]->position().point();
		return vertices;
	}
}    // namespace f35

#endif    // f35Planform_h
