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
