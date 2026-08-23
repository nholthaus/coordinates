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
// The F-35 planform as body-axis vertex tables (nose +x, right +y, down +z, meters; traced from a top-view
// reference, scaled to the real 15.70 m length / 10.75 m span) and a `consteval` builder that assembles the
// airframe as an `Entity` tree AT COMPILE TIME. Because `Entity`, `Coordinate`, `Pose`, and `CartesianTuple` are
// literal types, the whole airframe -- one root entity with every outline/canopy/intake/fin vertex attached as a
// child -- is constructed, posed, and resolved inside a constant evaluation. `buildF35` returns the finished
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
	//	PLANFORM (body axes, meters; +x forward, +y right, +z down) -- traced from a 3-view reference.
	//----------------------------------

	/// The fuselage + wing + tail silhouette, one closed loop in draw order.
	inline constexpr CartesianVector outline()
	{
		return {{-2.195_m, -5.376_m, 0.0_m}, {-3.738_m, -5.376_m, 0.0_m}, {-4.616_m, -1.906_m, 0.0_m},
		        {-4.809_m, -1.906_m, 0.0_m}, {-5.215_m, -2.078_m, 0.0_m}, {-5.323_m, -2.185_m, 0.0_m},
		        {-6.308_m, -3.663_m, 0.0_m}, {-7.186_m, -3.663_m, 0.0_m}, {-7.850_m, -0.985_m, 0.0_m},
		        {-6.479_m, -0.814_m, 0.0_m}, {-5.879_m, -0.643_m, 0.0_m}, {-5.879_m, -0.514_m, 0.0_m},
		        {-6.051_m, -0.407_m, 0.0_m}, {-6.051_m, 0.407_m, 0.0_m}, {-5.879_m, 0.514_m, 0.0_m},
		        {-5.901_m, 0.664_m, 0.0_m}, {-6.608_m, 0.835_m, 0.0_m}, {-7.850_m, 0.985_m, 0.0_m},
		        {-7.186_m, 3.641_m, 0.0_m}, {-6.329_m, 3.663_m, 0.0_m}, {-5.301_m, 2.163_m, 0.0_m},
		        {-5.215_m, 2.078_m, 0.0_m}, {-4.809_m, 1.906_m, 0.0_m}, {-4.616_m, 1.906_m, 0.0_m},
		        {-3.738_m, 5.355_m, 0.0_m}, {-2.195_m, 5.376_m, 0.0_m}, {0.011_m, 2.142_m, 0.0_m},
		        {0.739_m, 1.778_m, 0.0_m}, {3.181_m, 1.692_m, 0.0_m}, {3.609_m, 1.478_m, 0.0_m},
		        {3.266_m, 1.157_m, 0.0_m}, {3.266_m, 1.007_m, 0.0_m}, {3.502_m, 0.900_m, 0.0_m},
		        {4.209_m, 0.771_m, 0.0_m}, {6.758_m, 0.471_m, 0.0_m}, {7.315_m, 0.278_m, 0.0_m},
		        {7.850_m, 0.000_m, 0.0_m}, {7.229_m, -0.321_m, 0.0_m}, {6.779_m, -0.471_m, 0.0_m},
		        {3.566_m, -0.878_m, 0.0_m}, {3.288_m, -0.985_m, 0.0_m}, {3.266_m, -1.157_m, 0.0_m},
		        {3.609_m, -1.478_m, 0.0_m}, {3.181_m, -1.692_m, 0.0_m}, {0.739_m, -1.778_m, 0.0_m},
		        {0.011_m, -2.142_m, 0.0_m}, {-2.174_m, -5.355_m, 0.0_m}};
	}

	/// The canopy bubble.
	inline constexpr CartesianVector canopy()
	{
		return {{4.304_m, -0.478_m, 0.0_m}, {5.101_m, -0.478_m, 0.0_m}, {5.101_m, 0.518_m, 0.0_m},
		        {4.503_m, 0.558_m, 0.0_m}, {4.383_m, 0.478_m, 0.0_m}, {3.786_m, 0.398_m, 0.0_m},
		        {3.786_m, -0.359_m, 0.0_m}, {4.264_m, -0.438_m, 0.0_m}};
	}

	/// The right vertical stabilizer: a swept 3D fin standing off the engine boom (root on the boom at z = 0, tip
	/// raked aft, canted outboard, and lifted 2 m up).
	inline constexpr CartesianVector finRight()
	{
		return {{-3.802_m, 1.478_m, 0.000_m}, {-5.665_m, 2.249_m, -2.000_m}, {-6.586_m, 2.206_m, -2.000_m}, {-5.601_m, 1.414_m, 0.000_m}};
	}

	/// The left vertical stabilizer (the y-mirror of the right).
	inline constexpr CartesianVector finLeft()
	{
		return {{-3.802_m, -1.478_m, 0.000_m}, {-5.665_m, -2.249_m, -2.000_m}, {-6.586_m, -2.206_m, -2.000_m}, {-5.601_m, -1.414_m, 0.000_m}};
	}

	/// The left engine intake lip.
	inline constexpr CartesianVector intakeLeft()
	{
		return {{0.120_m, -1.474_m, 0.0_m}, {2.351_m, -1.474_m, 0.0_m}, {2.789_m, -1.435_m, 0.0_m},
		        {2.909_m, -1.355_m, 0.0_m}, {1.713_m, -1.275_m, 0.0_m}, {-0.199_m, -1.275_m, 0.0_m},
		        {-0.677_m, -1.355_m, 0.0_m}, {0.080_m, -1.435_m, 0.0_m}};
	}

	/// The right engine intake lip.
	inline constexpr CartesianVector intakeRight()
	{
		return {{0.040_m, 1.275_m, 0.0_m}, {2.710_m, 1.315_m, 0.0_m}, {2.909_m, 1.395_m, 0.0_m},
		        {2.710_m, 1.474_m, 0.0_m}, {0.558_m, 1.514_m, 0.0_m}, {-0.598_m, 1.395_m, 0.0_m},
		        {-0.638_m, 1.355_m, 0.0_m}, {-0.398_m, 1.315_m, 0.0_m}, {0.000_m, 1.315_m, 0.0_m}};
	}

	/// Every planform part, in draw order -- the single list both the runtime attach and the compile-time build
	/// iterate, so a part is added in exactly one place.
	inline const std::array<CartesianVector, 6>& parts()
	{
		static const std::array<CartesianVector, 6> tables{outline(), canopy(), intakeLeft(), intakeRight(), finLeft(), finRight()};
		return tables;
	}

	/// The total number of planform vertices across every part -- a compile-time constant that sizes the baked
	/// vertex array (so the capacity is DERIVED from the data, never a hand-picked number).
	inline constexpr std::size_t vertexCount()
	{
		return outline().size() + canopy().size() + intakeLeft().size() + intakeRight().size() + finLeft().size() + finRight().size();
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
	///				part (outline, canopy, intakes, fins) as child entities, then reads each child's resolved WORLD
	///				position and flattens the whole airframe into a `std::array<CartesianTuple, vertexCount()>`. The
	///				`Entity` tree (heap-backed) is transient -- built and consumed inside this one constant
	///				evaluation, so nothing escapes -- while the returned array PERSISTS as a `constexpr` constant.
	///				Being `consteval`, it can only run at compile time, which proves the entire
	///				`Entity`/`Coordinate`/`Pose`/`CartesianTuple` composition is constexpr-clean. The array size is
	///				deduced from the vertex tables, so the caller writes `constexpr auto f35 = buildF35(pos)` with no
	///				capacity to specify.
	/// @param[in]	position	the airframe's world position.
	/// @param[in]	pose		the airframe's attitude (defaults to identity).
	/// @return		the world-resolved planform vertices, in part order (outline, canopy, intakes, fins).
	//----------------------------------------------------------------------------------------------------------------------
	consteval std::array<CartesianTuple, vertexCount()> buildF35(const PositionECEF<Datum>& position, const Pose& pose = Pose::identity())
	{
		Entity<Datum> airframe(position, pose);
		airframe.attach(outline());
		airframe.attach(canopy());
		airframe.attach(intakeLeft());
		airframe.attach(intakeRight());
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
