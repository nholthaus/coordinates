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

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

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
//	FUNCTION: drawAirshow [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Draw the F-35 at a loop phase, posed through the maneuver attitude.
/// @details	The airframe attitude is the single source of truth for the frame; `f35::draw` poses every planform
///				part through it and strokes each closed loop, so the shape defined once in body axes foreshortens and
///				rotates correctly for free -- the entity/pose composition does the work, no per-vertex trig. When
///				`articulated` is set the control surfaces additionally deflect with the maneuver; the simple airplane
///				draws them flush.
/// @param[in]	view		the view (camera + image) to draw onto.
/// @param[in]	phase		the loop phase as a fraction of one full turn.
/// @param[in]	articulated	draw the deflecting-control-surface jet rather than the flush simple airplane.
//----------------------------------------------------------------------------------------------------------------------
enum class Fidelity
{
	Simple,        ///< outline, canopy, z-aware vstabs -- the clean jet
	Detailed,      ///< the simple jet plus every interior detail line -- the hero render
	Articulated    ///< the flap-cut jet with control surfaces deflecting to fly the maneuver
};

static void drawAirshow(View& view, turns<> phase, Fidelity fidelity)
{
	const Pose attitude({0.0_m, 0.0_m, 0.0_m}, airshowAttitude(phase));
	if (fidelity == Fidelity::Simple)
	{
		f35::simple(view, attitude);
		return;
	}
	if (fidelity == Fidelity::Detailed)
	{
		f35::detailed(view, attitude);
		return;
	}

	// Drive each control-surface family from the maneuver axis it actually commands on an F-35, with the correct
	// sense and a realistic throw. All from the single maneuver source, so the whole aircraft flies coherently:
	//   * Flaperons roll the aircraft: differential, right-wing-down roll = right flaperon trailing-edge UP, left
	//     DOWN (deflectFlap positive = free edge DOWN, so right = -roll, left = +roll).
	//   * Tailerons (all-moving) are the primary pitch AND roll surface: they deflect TOGETHER with the pitch
	//     command (nose-up pitch = trailing edge up = leading edge down = negative taileron angle, since
	//     deflectTaileron positive pitches the leading edge UP) and DIFFERENTIALLY with the roll command.
	//   * Rudders coordinate the yaw: both toe the same way with the yaw command.
	//   * Leading-edge flaps are lift devices, drooped SYMMETRICALLY with nose-up pitch (angle of attack), not a
	//     roll control -- both droop the same amount.
	const rotation::EulerAngles att   = airshowAttitude(phase);
	const radians<>             roll  = att.roll();
	const radians<>             pitch = att.pitch();
	const radians<>             yaw   = att.yaw();

	const radians<> taileronPitch = -0.5 * pitch;    // nose-up pitch -> leading edge down (both)
	const radians<> taileronRoll  = 0.5 * roll;      // right-wing-down roll -> right LE down, left up
	const radians<> leDroop       = units::max(radians<>(0.0), -0.4 * pitch + 0.10_rad);    // AoA-scheduled droop

	f35::articulated(view, attitude,
	                 {.leadingEdgeRight = leDroop,
	                  .leadingEdgeLeft  = leDroop,
	                  .trailingEdgeRight = -0.6 * roll,
	                  .trailingEdgeLeft  = +0.6 * roll,
	                  .rudderRight       = 1.2 * yaw,
	                  .rudderLeft        = 1.2 * yaw,
	                  .taileronRight     = taileronPitch + taileronRoll,
	                  .taileronLeft      = taileronPitch - taileronRoll});
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
/// @param[in]	argv	[frameDirectory] [frameCount] [--detailed|--articulated] -- output directory for the
///						numbered frames, how many frames span one loop (defaults: "frames", 120), and which
///						fidelity to fly: the flush simple jet (default), the detailed jet (every interior line),
///						or the articulated jet (deflecting control surfaces).
/// @return		0 on success.
//----------------------------------------------------------------------------------------------------------------------
int main(const int argc, char** argv)
{
	// A fidelity flag may appear anywhere; the remaining arguments are the positional frame directory and count.
	const std::vector<std::string> args(argv + 1, argv + argc);
	const Fidelity                 fidelity = (std::ranges::find(args, "--articulated") != args.end()) ? Fidelity::Articulated
	                                          : (std::ranges::find(args, "--detailed") != args.end())  ? Fidelity::Detailed
	                                                                                                   : Fidelity::Simple;
	std::vector<std::string> positional;
	std::ranges::copy_if(args, std::back_inserter(positional), [](const std::string& a) { return !a.starts_with("--"); });

	const std::filesystem::path frameDir   = !positional.empty() ? positional[0] : "frames";
	const int                   frameCount = positional.size() > 1 ? std::stoi(positional[1]) : 120;
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
		drawAirshow(view, turns{static_cast<double>(frame) / frameCount}, fidelity);

		writePpm(frameDir / std::format("frame_{:04}.ppm", frame), view.image());
	}

	const char* mode = (fidelity == Fidelity::Articulated) ? "articulated" : (fidelity == Fidelity::Detailed) ? "detailed" : "simple";
	std::cout << std::format("Wrote {} {} F-35 loop frames to {}/frame_####.ppm\n", frameCount, mode, frameDir.string());
	encodeVideo(frameDir);
	return 0;
}
