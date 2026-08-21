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
// An F-35 planform defined ENTIRELY from entities: the airframe is one `Entity`, and every outline vertex is a
// child entity mounted at its body-axis offset (nose +x, right +y, down +z, meters; traced from a top-view
// reference and scaled to the real 15.70 m length / 10.75 m span). The two vertical stabilizers are swept 3D
// fins that rise from the twin engine booms: each fin's root chord lies flat on its boom (z = 0) and its tip
// chord is raked aft, canted 26 deg outboard, and lifted 2.5 m up (-z), so the fins stand up out of the
// planform plane. The shape is defined ONCE in body axes; drawing it at any attitude is just reading each
// child's resolved position -- the entity tree IS the rigid body, so the planform foreshortens and rotates
// correctly with yaw/pitch/roll for free, no per-vertex trig, and rolling the airframe tilts the standing fins.
//
// This program renders a seamless-looping animation: the airframe flies a smooth attitude maneuver (a rolling
// wobble with coupled pitch and yaw), viewed through a fixed perspective camera at the canonical high
// rear-quarter angle. The maneuver is driven by full-cycle sinusoids of a shared loop phase, so the last
// frame's attitude equals the first frame's and the sequence loops with no seam. Each frame simply re-poses the
// one airframe entity and re-reads every child vertex's resolved world position -- the composition does all the
// foreshortening and fin tilt. Frames are written as numbered P6 PPMs for an external encoder to assemble.
//
//--------------------------------------------------------------------------------------------------

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "coordinates.h"

using namespace coordinates;
using namespace coordinates::topography;
using namespace coordinates::datums;
using namespace units::literals;

using Wgs = WGS84_G1674;

//----------------------------------
//	F-35 PLANFORM (body axes, meters; +x forward, +y right) -- traced from a 3-view reference.
//----------------------------------
static const CartesianVector OUTLINE = {
    {-2.195_m, -5.376_m, 0.0_m}, {-3.738_m, -5.376_m, 0.0_m}, {-4.616_m, -1.906_m, 0.0_m},
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
    {0.011_m, -2.142_m, 0.0_m}, {-2.174_m, -5.355_m, 0.0_m},
};
static const CartesianVector CANOPY = {
    {4.304_m, -0.478_m, 0.0_m}, {5.101_m, -0.478_m, 0.0_m}, {5.101_m, 0.518_m, 0.0_m},
    {4.503_m, 0.558_m, 0.0_m}, {4.383_m, 0.478_m, 0.0_m}, {3.786_m, 0.398_m, 0.0_m},
    {3.786_m, -0.359_m, 0.0_m}, {4.264_m, -0.438_m, 0.0_m},
};
// The vertical stabilizers as swept 4-corner fins in PURE body XYZ -- no separate cant angle, because the tip's
// (y, z) already encodes the cant. Each corner is a real 3D point measured directly from the three reference
// views: the SIDE view gives x (fore/aft sweep) and z (height, up = -z); the FRONT view gives the tip's
// outboard offset in y; the TOP view gives the root chord's y on the engine boom. Winding root-LE -> tip-LE ->
// tip-TE -> root-TE is a simple (non-self-intersecting) trapezoid. Rolling the airframe rotates these points
// bodily, so the fins tilt correctly under attitude, one way under +roll and the other under -roll.
static const CartesianVector FIN_R = {
    {-3.802_m, 1.478_m, 0.000_m},     // root leading  (inboard boom edge, z = 0)
    {-5.665_m, 2.249_m, -2.000_m},    // tip leading   (outboard + up)
    {-6.586_m, 2.206_m, -2.000_m},    // tip trailing
    {-5.601_m, 1.414_m, 0.000_m},     // root trailing (inboard boom edge, z = 0)
};
static const CartesianVector FIN_L = {
    {-3.802_m, -1.478_m, 0.000_m}, {-5.665_m, -2.249_m, -2.000_m},
    {-6.586_m, -2.206_m, -2.000_m}, {-5.601_m, -1.414_m, 0.000_m},
};
static const CartesianVector INTAKE_L = {
    {0.120_m, -1.474_m, 0.0_m}, {2.351_m, -1.474_m, 0.0_m}, {2.789_m, -1.435_m, 0.0_m},
    {2.909_m, -1.355_m, 0.0_m}, {1.713_m, -1.275_m, 0.0_m}, {-0.199_m, -1.275_m, 0.0_m},
    {-0.677_m, -1.355_m, 0.0_m}, {0.080_m, -1.435_m, 0.0_m},
};
static const CartesianVector INTAKE_R = {
    {0.040_m, 1.275_m, 0.0_m}, {2.710_m, 1.315_m, 0.0_m}, {2.909_m, 1.395_m, 0.0_m},
    {2.710_m, 1.474_m, 0.0_m}, {0.558_m, 1.514_m, 0.0_m}, {-0.598_m, 1.395_m, 0.0_m},
    {-0.638_m, 1.355_m, 0.0_m}, {-0.398_m, 1.315_m, 0.0_m}, {0.000_m, 1.315_m, 0.0_m},
};


//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: attachPolyline [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Attach a body-axis 2D vertex list to a parent entity as an ordered set of child entities.
/// @details	Each body-axis point becomes a child at that offset (planform parts lie at z = 0; the vertical
///				stabs carry real -z height). The returned pointers are IN ORDER so the caller can draw the loop by
///				connecting consecutive children -- every child then resolves to world through the parent's pose,
///				so the whole polyline moves rigidly with the airframe.
/// @param[in]	parent	the entity to attach the vertices to (the airframe).
/// @param[in]	parts	the body-axis (x forward, y right, z down) vertices in meters, in draw order.
/// @return		the child entity pointers in the same order.
//----------------------------------------------------------------------------------------------------------------------
static std::vector<Entity<Wgs>*> attachParts(Entity<Wgs>& parent, const CartesianVector& parts)
{
	std::vector<Entity<Wgs>*> aircraft;
	aircraft.reserve(parts.size());
	for (const auto& part : parts)
		aircraft.push_back(&parent.attach(part));
	return aircraft;
}


//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: main
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Render a seamless-looping animation of the entity-built F-35 flying a smooth attitude maneuver,
///				proving that a shape defined ONCE in body axes draws correctly at every attitude for free.
/// @details	The airframe pose is driven by full-cycle sinusoids of a shared loop phase, so the last frame's
///				attitude equals the first frame's and the sequence loops with no seam. Every frame re-poses the
///				one airframe entity and re-reads each child vertex's resolved world position -- the entity tree
///				IS the rigid body, so the planform foreshortens, rolls, and the standing vstabs tilt with no
///				per-vertex trig. Frames are written as zero-padded P6 PPMs for an external encoder to assemble.
/// @param[in]	argc	argument count.
/// @param[in]	argv	[frameDirectory] [frameCount] -- output directory for the numbered frames and how many
///						frames span one loop (defaults: "frames", 120).
/// @return		0 on success.
//----------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	const std::string frameDir   = (argc > 1) ? argv[1] : "frames";
	const int         frameCount = (argc > 2) ? std::atoi(argv[2]) : 120;

	// One airframe entity; every planform part is a set of child vertices at fixed body offsets. Built once,
	// re-posed each frame -- the children never move in the body frame, only the airframe's pose changes.
	Entity<Wgs> airframe;
	auto        outline = attachParts(airframe, OUTLINE);
	auto        canopy  = attachParts(airframe, CANOPY);
	auto        intakeL = attachParts(airframe, INTAKE_L);
	auto        intakeR = attachParts(airframe, INTAKE_R);
	auto        vstabL  = attachParts(airframe, FIN_L);
	auto        vstabR  = attachParts(airframe, FIN_R);

	// A pinhole camera ahead-of and above the airframe, looking aft and down onto its top: the canonical high
	// rear-quarter F-35 photo angle. One `lookAt` places it; the camera derives its own basis and focal length.
	const int W = 860, H = 650;
	Camera    camera(W, H);
	camera.lookAt(CartesianTuple(15.0_m, -1.0_m, -9.0_m), CartesianTuple(-2.0_m, 0.0_m, 0.0_m));

	auto drawLoop = [&](Image& img, const std::vector<Entity<Wgs>*>& v, Color col) {
		for (std::size_t i = 0; i < v.size(); ++i)
		{
			const Pixel a = camera.project(v[i]->position().point());
			const Pixel b = camera.project(v[(i + 1) % v.size()]->position().point());
			if (!Camera::sees(a) || !Camera::sees(b))
				continue;
			img.line(a, b, col);
			img.line(Pixel{a.row + 1, a.column}, Pixel{b.row + 1, b.column}, col);    // 2 px for a crisp stroke
		}
	};

	for (int frame = 0; frame < frameCount; ++frame)
	{
		// Full-cycle sinusoids of the loop phase: the maneuver returns exactly to its start, so the video loops
		// seamlessly. A rolling wobble with a coupled pitch/yaw oscillation reads as a lively airshow pass.
		const units::angle::turns<> phase{static_cast<double>(frame) / frameCount};
		const degrees<>             yaw   = 10.0_deg * sin(phase);
		const degrees<>             pitch = -8.0_deg + 10.0_deg * sin(phase);
		const degrees<>             roll  = 55.0_deg * sin(phase * 2.0);
		airframe.setPose(Pose(CartesianTuple(0.0_m, 0.0_m, 0.0_m), rotation::EulerAngles{yaw, pitch, roll}));

		Image img(H, W);
		for (int r = 0; r < H; ++r)
			for (int c = 0; c < W; ++c)
				img.plot(Pixel{r, c}, Color{238, 242, 247});
		const Color ink{20, 20, 20};
		drawLoop(img, outline, ink);
		drawLoop(img, intakeL, ink);
		drawLoop(img, intakeR, ink);
		drawLoop(img, canopy, ink);
		drawLoop(img, vstabL, ink);
		drawLoop(img, vstabR, ink);

		char path[512];
		std::snprintf(path, sizeof(path), "%s/frame_%04d.ppm", frameDir.c_str(), frame);
		std::FILE* f = std::fopen(path, "wb");
		if (!f)
		{
			std::fprintf(stderr, "cannot open %s -- create the directory first\n", path);
			return 1;
		}
		std::fprintf(f, "P6\n%d %d\n255\n", W, H);
		std::fwrite(img.rgb().data(), 1, img.rgb().size(), f);
		std::fclose(f);
	}

	std::printf("F-35 entity tree: airframe + %zu children (%zu outline, %zu canopy, %zu+%zu intakes, %zu+%zu vstab)\n",
	            airframe.children().size(), outline.size(), canopy.size(), intakeL.size(), intakeR.size(), vstabL.size(), vstabR.size());
	std::printf("Wrote %d loop frames to %s/frame_####.ppm\n", frameCount, frameDir.c_str());
	return 0;
}
