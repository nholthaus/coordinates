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

#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>

#include "coordinates.h"
#include "f35Planform.h"

using namespace coordinates;
using namespace coordinates::topography;
using namespace coordinates::datums;
using namespace units::literals;

using Wgs = WGS84_G1674;

// The F-35 planform is defined ONCE in f35Planform.h. Prove the whole airframe assembles and its vertices resolve
// through the pose at COMPILE TIME: `buildF35` is consteval, so this constant exists only if the entire
// Entity/Pose/Coordinate composition is constexpr-clean. The runtime animation below builds the same shape live.
constexpr auto COMPILE_TIME_F35 = f35::buildF35(PositionECEF<Wgs>(6378137.0_m, 0.0_m, 0.0_m));
static_assert(COMPILE_TIME_F35.size() == f35::vertexCount(), "the F-35 planform builds at compile time");

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: airshowAttitude [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		The airframe attitude at a normalized loop phase [0, 1): a rolling wobble with coupled pitch and yaw.
/// @details	Every channel is a full-cycle sinusoid of the phase, so the attitude at phase 1 equals the attitude
///				at phase 0 and the animation loops with no seam. This is the ONE source of truth for the maneuver;
///				the airframe pose AND the control-surface deflections are both derived from it, so changing the
///				maneuver here moves the whole aircraft -- surfaces included -- with no other edit.
/// @param[in]	phase	the loop phase as a fraction of one full turn.
/// @return		the airframe orientation as intrinsic Z-Y-X yaw/pitch/roll.
//----------------------------------------------------------------------------------------------------------------------
static rotation::EulerAngles airshowAttitude(turns<> phase)
{
	return {10.0_deg * sin(phase), -8.0_deg + 10.0_deg * sin(phase), 55.0_deg * sin(phase * 2.0)};
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: deflect [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Deflect a control-surface loop about its hinge -- rotate every vertex `angle` about the hinge axis
///				through the hinge point, in body axes.
/// @details	A movable surface is a rigid sub-body hinged to the airframe: the deflection is a `Pose` that
///				rotates about the hinge point (translate to the hinge, rotate about the axis, translate back), and
///				the library's `Pose::transformPoint` applies it. The deflected loop is then drawn through the
///				airframe attitude like any other part, so the control surface composes up the chain for free.
/// @param[in]	loop	the surface's body-frame vertices.
/// @param[in]	hinge	a point on the hinge line, in body axes.
/// @param[in]	axis	the hinge axis direction (need not be normalized).
/// @param[in]	angle	the deflection angle (right-hand about `axis`).
/// @return		the deflected loop, in body axes.
//----------------------------------------------------------------------------------------------------------------------
static CartesianVector deflect(const CartesianVector& loop, const CartesianTuple& hinge, const CartesianTuple& axis, radians<> angle)
{
	const auto                 n = axis.normalized();
	const rotation::Quaternion q = toQuaternion(rotation::AxisAngle(n.x(), n.y(), n.z(), angle));
	CartesianVector            out;
	out.reserve(loop.size());
	for (const CartesianTuple& v : loop)
		out.push_back(q.rotate(v - hinge) + hinge);    // rotate about the hinge point
	return out;
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: drawAirshow [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Draw the F-35 at a loop phase with its control surfaces ARTICULATED: flaperons deflect with the
///				roll command and rudders with the yaw, so the jet flies the maneuver like a real airshow demo.
/// @details	The fixed structure draws through the airframe attitude unchanged; the movable loops (flaperons,
///				rudders) are first `deflect`ed about their hinges, then drawn through the same attitude. The
///				deflections are DERIVED from the maneuver, not hand-authored: a control surface commands an angular
///				RATE, so the ailerons track the maneuver's roll rate and the rudders its yaw rate (both obtained by
///				finite-differencing `airshowAttitude` about the phase). Because the surfaces come from the same
///				single source of truth as the pose, any maneuver -- or any airframe with flaps and rudders -- just
///				"works": the surfaces lead the roll and coordinate the yaw with no per-aircraft tuning.
/// @param[in]	view		the view (camera + image) to draw onto.
/// @param[in]	phase		the loop phase as a fraction of one full turn.
//----------------------------------------------------------------------------------------------------------------------
static void drawAirshow(View& view, turns<> phase)
{
	const rotation::EulerAngles att = airshowAttitude(phase);
	const Pose                  attitude({0.0_m, 0.0_m, 0.0_m}, att);

	// Control surfaces command angular RATES, so derive them from the maneuver's roll/yaw rates -- a central
	// finite difference of `airshowAttitude` about the phase. `gain` maps deg-of-attitude-per-turn to surface
	// deflection; the result is clamped to a realistic throw. This ties articulation to the maneuver SSOT.
	const turns<>   h    = 0.002_tr;
	const auto      rate = [&](auto pick) { return (pick(airshowAttitude(phase + h)) - pick(airshowAttitude(phase - h))) / (2.0 * h / 1.0_tr); };
	const auto      clamp = [](degrees<> a, degrees<> lim) { return units::max(-lim, units::min(lim, a)); };
	const radians<> aileron = clamp(0.09 * rate([](auto e) { return e.roll(); }), 25.0_deg);
	const radians<> rudder  = clamp(0.80 * rate([](auto e) { return e.yaw(); }),  25.0_deg);
	const radians<> leFlap  = clamp(0.60 * rate([](auto e) { return e.pitch(); }) + 12.0_deg, 30.0_deg);

	// Fixed structure.
	for (const CartesianVector& part : {f35::outline(), f35::canopy(), f35::lerxLeft(), f35::lerxRight(),
	                                    f35::intakeLeft(), f35::intakeRight(), f35::ventLeft(), f35::ventRight(),
	                                    f35::nozzleLeft(), f35::nozzleRight(), f35::exhaust(),
	                                    f35::finLeft(), f35::finRight()})
		drawPolyline(view, attitude, part);

	// Flaperons hinge on a spanwise (body-Y) line at the wing trailing edge; deflecting rotates the trailing edge
	// up/down. The two oppose (right-wing-down roll = right flaperon up, left down), so `aileron` negates per side.
	drawPolyline(view, attitude, deflect(f35::flapLeft(),  {-3.20_m, -2.80_m, 0.0_m}, {0.0_m, 1.0_m, 0.0_m}, -aileron));
	drawPolyline(view, attitude, deflect(f35::flapRight(), {-3.20_m,  2.80_m, 0.0_m}, {0.0_m, 1.0_m, 0.0_m},  aileron));

	// Leading-edge flaps hinge on the swept wing leading edge; deflecting droops the strip down. The hinge axis is
	// the LE direction (swept aft-and-outboard), so the flap rotates leading-edge-down about it, both wings alike.
	drawPolyline(view, attitude, deflect(f35::leFlapLeft(),  {0.10_m, -2.25_m, 0.0_m}, {-3.15_m, -2.75_m, 0.0_m}, -leFlap));
	drawPolyline(view, attitude, deflect(f35::leFlapRight(), {0.10_m,  2.25_m, 0.0_m}, {-3.15_m,  2.75_m, 0.0_m},  leFlap));

	// Rudders hinge on their fin's near-vertical trailing edge; deflecting yaws the surface. Both toe the same way.
	drawPolyline(view, attitude, deflect(f35::rudderLeft(),  {-6.60_m, -1.80_m, -1.20_m}, {0.30_m, 0.20_m, -1.0_m}, rudder));
	drawPolyline(view, attitude, deflect(f35::rudderRight(), {-6.60_m,  1.80_m, -1.20_m}, {0.30_m, 0.20_m, -1.0_m}, rudder));
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: writePpm [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Write an image to a binary P6 PPM file (the frame-output plumbing, kept out of the demo loop).
/// @param[in]	path	the file to write.
/// @param[in]	image	the image to serialize.
//----------------------------------------------------------------------------------------------------------------------
static void writePpm(const std::filesystem::path& path, const Image& image)
{
	std::ofstream file(path, std::ios::binary);
	file << std::format("P6\n{} {}\n255\n", image.columns(), image.rows());
	file.write(reinterpret_cast<const char*>(image.rgb().data()), static_cast<std::streamsize>(image.rgb().size()));
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: encodeVideo [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Encode the rendered frames into a looping MP4 and GIF with ffmpeg, or skip cleanly if it is absent.
/// @param[in]	frameDir	the directory holding the numbered frame_####.ppm files (also the output location).
//----------------------------------------------------------------------------------------------------------------------
static void encodeVideo(const std::filesystem::path& frameDir)
{
	// Run a shell command, reporting (never ignoring) a non-zero exit -- glibc marks system() warn_unused_result.
	const auto run = [](const std::string& command) { return std::system(command.c_str()) == 0; };

	if (!run("ffmpeg -version >/dev/null 2>&1"))
	{
		std::cout << "ffmpeg not found; wrote frames only. Install ffmpeg to encode the video.\n";
		return;
	}
	const std::string frames = (frameDir / "frame_%04d.ppm").string();
	const bool         mp4    = run(std::format("ffmpeg -y -framerate 30 -i \"{}\" -c:v libx264 -pix_fmt yuv420p -movflags +faststart \"{}\"",
	                                            frames, (frameDir / "f35_loop.mp4").string()));
	// GIF via a two-pass palette (palettegen/paletteuse): a single-pass palette misquantizes the sparse dark
	// strokes on white and fringes them (they came out yellow); the exact palette keeps them black.
	const bool gif = run(std::format("ffmpeg -y -framerate 30 -i \"{}\" -vf \"fps=25,scale=480:-1:flags=lanczos,split[s0][s1];"
	                                 "[s0]palettegen[p];[s1][p]paletteuse\" \"{}\"",
	                                 frames, (frameDir / "f35_loop.gif").string()));
	std::cout << std::format("{} {}/f35_loop.mp4 and f35_loop.gif\n", (mp4 && gif) ? "Encoded" : "ffmpeg reported an error encoding", frameDir.string());
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
int main(const int argc, char** argv)
{
	const std::filesystem::path frameDir   = (argc > 1) ? argv[1] : "frames";
	const int                   frameCount = (argc > 2) ? std::stoi(argv[2]) : 120;
	std::filesystem::create_directories(frameDir);

	// The camera is placed in the airframe's own body axes (+x forward, +y right, +z down): the eye sits ~15 m
	// AHEAD (+x), 9 m ABOVE (-z), a touch left (-y), and looks at a point ~2 m behind the nose -- the canonical
	// high rear-quarter view. `lookAt` derives the right/up/forward basis and the focal length from those points.
	Camera camera(860, 650);
	camera.lookAt({15.0_m, -1.0_m, -9.0_m}, {-2.0_m, 0.0_m, 0.0_m});

	// The example's whole point: pose the once-defined F-35 through the maneuver and draw it -- correct at every
	// attitude for free, because the pose composition does all the foreshortening and tilt.
	for (const int frame : std::views::iota(0, frameCount))
	{
		View view(camera);
		drawAirshow(view, turns{static_cast<double>(frame) / frameCount});

		writePpm(frameDir / std::format("frame_{:04}.ppm", frame), view.image());
	}

	std::cout << std::format("Wrote {} F-35 loop frames to {}/frame_####.ppm\n", frameCount, frameDir.string());
	encodeVideo(frameDir);
	return 0;
}
