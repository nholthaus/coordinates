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

	/// The fuselage + wing + tail silhouette, one closed loop in draw order.
	inline constexpr CartesianVector outline()
	{
		return {
		        {+7.816_m, +0.055_m, +0.000_m}, {+7.207_m, +0.350_m, +0.000_m}, {+6.691_m, +0.515_m, +0.000_m},
		        {+5.977_m, +0.556_m, +0.000_m}, {+5.135_m, +0.639_m, +0.000_m}, {+4.455_m, +0.762_m, +0.000_m},
		        {+4.034_m, +0.804_m, +0.000_m}, {+3.243_m, +1.010_m, +0.000_m}, {+3.168_m, +1.107_m, +0.000_m},
		        {+3.605_m, +1.462_m, +0.000_m}, {+3.323_m, +1.663_m, +0.000_m}, {+3.148_m, +1.710_m, +0.000_m},
		        {+2.271_m, +1.710_m, +0.000_m}, {+2.191_m, +1.751_m, +0.000_m}, {+1.217_m, +1.751_m, +0.000_m},
		        {+0.636_m, +1.878_m, +0.000_m}, {+0.035_m, +2.190_m, +0.000_m}, {-0.391_m, +2.744_m, +0.000_m},
		        {-0.805_m, +3.401_m, +0.000_m}, {-1.571_m, +4.496_m, +0.000_m}, {-2.165_m, +5.419_m, +0.000_m},
		        {-3.723_m, +5.419_m, +0.000_m}, {-3.813_m, +4.923_m, +0.000_m}, {-3.980_m, +4.262_m, +0.000_m},
		        {-4.059_m, +4.100_m, +0.000_m}, {-4.553_m, +1.967_m, +0.000_m}, {-4.656_m, +1.875_m, +0.000_m},
		        {-5.356_m, +2.183_m, +0.000_m}, {-7.567_m, +2.158_m, +0.000_m}, {-7.644_m, +1.947_m, +0.000_m},
		        {-7.849_m, +1.010_m, +0.000_m}, {-7.411_m, +1.010_m, +0.000_m}, {-6.489_m, +0.884_m, +0.000_m},
		        {-5.777_m, +0.721_m, +0.000_m}, {-5.720_m, +0.626_m, +0.000_m}, {-6.240_m, +0.353_m, +0.000_m},
		        {-6.243_m, -0.328_m, +0.000_m}, {-5.692_m, -0.613_m, +0.000_m}, {-5.710_m, -0.680_m, +0.000_m},
		        {-6.647_m, -0.886_m, +0.000_m}, {-7.246_m, -0.968_m, +0.000_m}, {-7.685_m, -0.968_m, +0.000_m},
		        {-7.850_m, -1.022_m, +0.000_m}, {-7.726_m, -1.604_m, +0.000_m}, {-7.520_m, -2.298_m, +0.000_m},
		        {-5.303_m, -2.130_m, +0.000_m}, {-4.643_m, -1.841_m, +0.000_m}, {-4.553_m, -1.935_m, +0.000_m},
		        {-4.430_m, -2.566_m, +0.000_m}, {-4.141_m, -3.616_m, +0.000_m}, {-3.812_m, -5.038_m, +0.000_m},
		        {-3.682_m, -5.419_m, +0.000_m}, {-2.220_m, -5.419_m, +0.000_m}, {-2.147_m, -5.361_m, +0.000_m},
		        {-1.371_m, -4.173_m, +0.000_m}, {-1.267_m, -4.069_m, +0.000_m}, {-0.342_m, -2.650_m, +0.000_m},
		        {-0.051_m, -2.235_m, +0.000_m}, {+0.499_m, -1.916_m, +0.000_m}, {+1.015_m, -1.751_m, +0.000_m},
		        {+1.582_m, -1.710_m, +0.000_m}, {+2.654_m, -1.710_m, +0.000_m}, {+2.734_m, -1.669_m, +0.000_m},
		        {+3.222_m, -1.669_m, +0.000_m}, {+3.593_m, -1.450_m, +0.000_m}, {+3.171_m, -1.032_m, +0.000_m},
		        {+3.728_m, -0.846_m, +0.000_m}, {+4.358_m, -0.721_m, +0.000_m}, {+6.092_m, -0.556_m, +0.000_m},
		        {+6.739_m, -0.472_m, +0.000_m}, {+7.256_m, -0.309_m, +0.000_m}, {+7.781_m, -0.048_m, +0.000_m}};
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
		        {+3.523_m, -1.463_m, +0.000_m}, {+3.105_m, -1.092_m, +0.000_m}, {+1.825_m, -0.762_m, +0.000_m},
		        {+0.609_m, -0.639_m, +0.000_m}, {-0.492_m, -0.598_m, +0.000_m}, {-1.949_m, -0.680_m, +0.000_m},
		        {-3.222_m, -0.845_m, +0.000_m}, {-4.191_m, -0.845_m, +0.000_m}, {-5.682_m, -0.696_m, +0.000_m},
		        {-6.444_m, -0.933_m, +0.000_m}, {-6.284_m, -1.537_m, +0.000_m}, {-6.112_m, -1.298_m, +0.000_m},
		        {-4.879_m, -1.216_m, +0.000_m}, {-3.759_m, -1.392_m, +0.000_m}, {-3.722_m, -1.497_m, +0.000_m},
		        {-4.332_m, -1.751_m, +0.000_m}, {-3.812_m, -1.900_m, +0.000_m}, {-3.127_m, -4.734_m, +0.000_m},
		        {-3.806_m, -4.759_m, +0.000_m}, {-3.662_m, -5.362_m, +0.000_m}, {-2.625_m, -5.378_m, +0.000_m},
		        {-0.686_m, -2.458_m, +0.000_m}, {-0.161_m, -2.287_m, +0.000_m}, {+0.679_m, -1.794_m, +0.000_m},
		        {+1.521_m, -1.669_m, +0.000_m}, {+3.219_m, -1.628_m, +0.000_m}};
	}

	/// The right forebody chine / LERX shoulder panel.
	inline constexpr CartesianVector lerxRight()
	{
		return {
		        {+3.523_m, +1.463_m, +0.000_m}, {+3.189_m, +1.669_m, +0.000_m}, {+1.213_m, +1.713_m, +0.000_m},
		        {+0.541_m, +1.875_m, +0.000_m}, {-0.208_m, +2.328_m, +0.000_m}, {-0.624_m, +2.396_m, +0.000_m},
		        {-2.628_m, +5.378_m, +0.000_m}, {-3.662_m, +5.378_m, +0.000_m}, {-3.801_m, +4.801_m, +0.000_m},
		        {-3.111_m, +4.727_m, +0.000_m}, {-3.837_m, +1.850_m, +0.000_m}, {-4.299_m, +1.751_m, +0.000_m},
		        {-3.694_m, +1.457_m, +0.000_m}, {-4.719_m, +1.257_m, +0.000_m}, {-5.754_m, +1.257_m, +0.000_m},
		        {-6.202_m, +1.399_m, +0.000_m}, {-6.242_m, +1.585_m, +0.000_m}, {-6.424_m, +0.886_m, +0.000_m},
		        {-5.679_m, +0.721_m, +0.000_m}, {-4.622_m, +0.845_m, +0.000_m}, {-2.922_m, +0.845_m, +0.000_m},
		        {-2.439_m, +0.762_m, +0.000_m}, {-0.939_m, +0.639_m, +0.000_m}, {+0.761_m, +0.639_m, +0.000_m},
		        {+2.154_m, +0.845_m, +0.000_m}, {+3.145_m, +1.126_m, +0.000_m}};
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

	/// The left wing-root vent.
	inline constexpr CartesianVector ventLeft()
	{
		return {
		        {-0.185_m, -2.369_m, +0.000_m}, {-0.540_m, -2.411_m, +0.000_m}, {-1.174_m, -3.317_m, +0.000_m},
		        {-1.218_m, -3.443_m, +0.000_m}, {-2.491_m, -5.334_m, +0.000_m}, {-2.249_m, -5.378_m, +0.000_m},
		        {-1.504_m, -4.344_m, +0.000_m}, {-0.844_m, -3.316_m, +0.000_m}};
	}

	/// The right wing-root vent.
	inline constexpr CartesianVector ventRight()
	{
		return {
		        {-0.185_m, +2.411_m, +0.000_m}, {-1.374_m, +4.135_m, +0.000_m}, {-1.634_m, +4.560_m, +0.000_m},
		        {-2.218_m, +5.378_m, +0.000_m}, {-2.485_m, +5.378_m, +0.000_m}, {-2.411_m, +5.215_m, +0.000_m},
		        {-1.673_m, +4.146_m, +0.000_m}, {-0.558_m, +2.453_m, +0.000_m}};
	}

	/// The left wing trailing-edge flap panel line.
	inline constexpr CartesianVector flapLeft()
	{
		return {
		        {-3.194_m, -4.677_m, +0.000_m}, {-3.729_m, -2.658_m, +0.000_m}, {-3.897_m, -1.875_m, +0.000_m},
		        {-4.553_m, -1.867_m, +0.000_m}, {-4.224_m, -3.078_m, +0.000_m}, {-3.839_m, -4.677_m, +0.000_m}};
	}

	/// The right wing trailing-edge flap panel line.
	inline constexpr CartesianVector flapRight()
	{
		return {
		        {-3.209_m, +4.718_m, +0.000_m}, {-3.853_m, +4.716_m, +0.000_m}, {-4.471_m, +2.124_m, +0.000_m},
		        {-4.512_m, +1.834_m, +0.000_m}, {-3.900_m, +1.916_m, +0.000_m}, {-3.523_m, +3.522_m, +0.000_m}};
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

	/// The left wing leading-edge flap.
	inline constexpr CartesianVector leFlapLeft()
	{
		return {
		        {+0.100_m, -2.250_m, +0.000_m}, {-3.050_m, -5.000_m, +0.000_m}, {-3.600_m, -4.700_m, +0.000_m},
		        {-0.500_m, -2.100_m, +0.000_m}};
	}

	/// The right wing leading-edge flap.
	inline constexpr CartesianVector leFlapRight()
	{
		return {
		        {+0.100_m, +2.250_m, +0.000_m}, {-3.050_m, +5.000_m, +0.000_m}, {-3.600_m, +4.700_m, +0.000_m},
		        {-0.500_m, +2.100_m, +0.000_m}};
	}

	/// Every planform part, in draw order -- the single list both the runtime attach and the compile-time build
	/// iterate, so a part is added in exactly one place.
	inline const std::array<CartesianVector, 19>& parts()
	{
		static const std::array<CartesianVector, 19> tables{outline(), canopy(), lerxLeft(), lerxRight(), intakeLeft(), intakeRight(), ventLeft(), ventRight(), flapLeft(), flapRight(), nozzleLeft(), nozzleRight(), exhaust(), finLeft(), finRight(), rudderLeft(), rudderRight(), leFlapLeft(), leFlapRight()};
		return tables;
	}

	/// The total number of planform vertices across every part -- a compile-time constant that sizes the baked
	/// vertex array (so the capacity is DERIVED from the data, never a hand-picked number).
	inline constexpr std::size_t vertexCount()
	{
		return outline().size() + canopy().size() + lerxLeft().size() + lerxRight().size() + intakeLeft().size() + intakeRight().size() + ventLeft().size() + ventRight().size() + flapLeft().size() + flapRight().size() + nozzleLeft().size() + nozzleRight().size() + exhaust().size() + finLeft().size() + finRight().size() + rudderLeft().size() + rudderRight().size() + leFlapLeft().size() + leFlapRight().size();
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
		airframe.attach(ventLeft());
		airframe.attach(ventRight());
		airframe.attach(flapLeft());
		airframe.attach(flapRight());
		airframe.attach(nozzleLeft());
		airframe.attach(nozzleRight());
		airframe.attach(exhaust());
		airframe.attach(finLeft());
		airframe.attach(finRight());
		airframe.attach(rudderLeft());
		airframe.attach(rudderRight());
		airframe.attach(leFlapLeft());
		airframe.attach(leFlapRight());

		std::array<CartesianTuple, vertexCount()> vertices{};
		const auto&                               children = airframe.children();
		for (std::size_t i = 0; i < children.size(); ++i)
			vertices[i] = children[i]->position().point();
		return vertices;
	}
}    // namespace f35

#endif    // f35Planform_h
