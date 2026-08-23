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
// End-to-end case study exercising the whole substrate: an aircraft flies a maneuvering (pitch/roll/yaw) track
// over Nevada terrain carrying a HARD-MOUNTED sensor pod out on the wing. The aircraft is an `Entity`; the pod
// is a CHILD entity attached to it -- rigidly, at a wing offset with a fixed body-axis boresight (down and
// right of the nose). Because the pod is a child, it rolls rigidly WITH the aircraft: as the aircraft banks
// through its turns, the pod's fixed body-axis look sweeps across the ground (its ground azimuth relative to
// the nose is only constant wings-level; a hard bank swings it). Each step the pod's own ray -- `pod.ray()`,
// resolved to world through the parent chain -- is marched to the DTED terrain (`terrainIntersection`), and the
// geodetic hit recorded; the flight path and hit track are drawn on the tile's hillshade with a `HillshadeCanvas`.
//
// Every geometric and rendering step is a library call: the Entity tree carries the aircraft pose and the pod's
// rigid mount, `pod.ray()` composes body->wing->world with no hand-rolled rotations, `terrainIntersection` is
// the geodesic terrain march, and `HillshadeCanvas` is the colorized hillshade + projection + drawing.
//
//--------------------------------------------------------------------------------------------------

#include <algorithm>
#include <filesystem>
#include <format>
#include <iostream>
#include <ranges>
#include <string>

#include "coordinates.h"
#include "dtedTile.h"
#include "lineOfSight.h"    // terrainIntersection (coordinates.h includes it only when LOS is enabled)
#include "f35Planform.h"

using namespace coordinates;
using namespace coordinates::topography;
using namespace coordinates::datums;
using namespace units::literals;

using Wgs = WGS84_G1674;
using Lla = PositionGeodetic<Wgs>;

/// The aircraft attitude the maneuver commands at a point along the track: heading, pitch, and bank.
struct FlightAttitude
{
	degrees<> yaw;      ///< heading, from north, positive east
	degrees<> pitch;    ///< nose-up positive
	degrees<> roll;     ///< right-wing-down positive
};

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: nedToEcefAttitude [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		The body-to-ECEF orientation of an aircraft at a geodetic point with a given yaw/pitch/roll.
/// @details	Composes the local-level (NED) frame's orientation in ECEF at the point with the body-to-NED
///				attitude, so a body forward/down points at the local terrain rather than at the ECEF axes. The
///				NED-in-ECEF rotation is the frame graph's own: a north-pointing NED unit vector converted to a
///				`VectorECEF` gives the local north, so the attitude falls out of the library rather than a
///				hand-built direction-cosine matrix.
/// @param[in]	position	the aircraft's geodetic position.
/// @param[in]	yaw			heading, from north, positive east.
/// @param[in]	pitch		nose-up positive.
/// @param[in]	roll		right-wing-down positive.
/// @return		the body-to-ECEF orientation quaternion.
//----------------------------------------------------------------------------------------------------------------------
static rotation::Quaternion nedToEcefAttitude(const Lla& position, degrees<> yaw, degrees<> pitch, degrees<> roll)
{
	// NED axes expressed in ECEF, obtained by converting local-frame unit vectors through the frame graph.
	const VectorNED<Wgs> northNed(1.0_m, 0.0_m, 0.0_m, position), eastNed(0.0_m, 1.0_m, 0.0_m, position), downNed(0.0_m, 0.0_m, 1.0_m, position);
	const VectorECEF<Wgs> n(northNed), e(eastNed), d(downNed);
	const rotation::RotationMatrix nedToEcef(std::get<0>(n.vector()) / m, std::get<0>(e.vector()) / m, std::get<0>(d.vector()) / m,
	                                         std::get<1>(n.vector()) / m, std::get<1>(e.vector()) / m, std::get<1>(d.vector()) / m,
	                                         std::get<2>(n.vector()) / m, std::get<2>(e.vector()) / m, std::get<2>(d.vector()) / m);
	return rotation::toQuaternion(nedToEcef) * rotation::toQuaternion(rotation::EulerAngles{yaw, pitch, roll});
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: maneuver [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		The commanded attitude at a phase along the S-track: yaw sweeps sinusoidally, roll follows the
///				yaw rate (a coordinated turn), and pitch oscillates gently. This is the maneuver the example flies.
/// @param[in]	phase		the maneuver phase (two full turns span the track).
/// @param[in]	angularRate	the phase's angular frequency, so the roll cue tracks the yaw's rate of change.
/// @return		the commanded yaw/pitch/roll at that phase.
//----------------------------------------------------------------------------------------------------------------------
static FlightAttitude maneuver(radians<> phase, hertz<> angularRate)
{
	const degrees<> yawAmplitude{40.0};
	const auto      yaw = degrees<>(270.0) + yawAmplitude * sin(phase);
	// Coordinated-turn roll cue: proportional to the yaw's rate of change (its analytic derivative), clamped.
	const degrees_per_second<> yawRate = yawAmplitude * cos(phase) * angularRate;
	const auto                 roll    = std::clamp(degrees<>(12.0_s * yawRate), degrees<>(-45.0), degrees<>(45.0));
	const auto                 pitch   = degrees<>(-4.0) + degrees<>(6.0) * sin(phase + radians<>(1.0));
	return {yaw, pitch, roll};
}

/// Map an F-35 body-axis point (nose +x, right +y, down +z, meters) to a screen pixel about the nadir: the nose
/// maps to screen (-cos yaw, +sin yaw), starboard to (sin yaw, cos yaw). A bank rotates the body y-z plane about
/// the forward axis, so the lateral extent is `y*cos(roll) - z*sin(roll)` -- the standing vstabs (z up) sweep into
/// the plan view while turning and collapse onto the fuselage wings-level. A top-down map projection at
/// `pixelsPerMeter` scale.
static Pixel projectBody(const CartesianTuple& body, Pixel nadir, degrees<> yaw, degrees<> roll, double pixelsPerMeter)
{
	const double forward = (body.x() / m) * pixelsPerMeter;
	const double lateral = ((body.y() / m) * cos(roll).value() - (body.z() / m) * sin(roll).value()) * pixelsPerMeter;
	const double row = nadir.row + forward * -cos(yaw).value() + lateral * sin(yaw).value();
	const double col = nadir.column + forward * sin(yaw).value() + lateral * cos(yaw).value();
	return Pixel{(int) std::lround(row), (int) std::lround(col)};
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: drawAircraftGlyph [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Stroke the simplified F-35 planform glyph at the nadir pixel and return the pod pixel the sensor ray
///				marches from.
/// @details	Draws the library's canonical F-35 planform parts -- the fuselage/wing/tail outline and the two
///				vertical stabilizers (`f35::outline()`, `f35::finLeft()`, `f35::finRight()`, the single shape source
///				shared with the compile-time build and the perspective renderer; the canopy and intakes are omitted
///				for a clean map symbol) -- as a top-down glyph: each body-axis vertex is projected about the nadir
///				through the yaw heading with the bank foreshortening the lateral extent, then each closed loop is
///				stroked two pixels thick. The pod rides its body-frame wing station, so it tracks heading and bank
///				with the airframe.
/// @param[in,out]	frame		the image to draw the glyph onto.
/// @param[in]		nadir		the aircraft's projected ground pixel.
/// @param[in]		yaw			the aircraft heading.
/// @param[in]		roll		the aircraft bank.
/// @param[in]		podStation	the pod's body-axis mount offset (the wing station it rides).
/// @return		the pod pixel (the wing-mounted sensor's screen position).
//----------------------------------------------------------------------------------------------------------------------
static Pixel drawAircraftGlyph(Image& frame, Pixel nadir, degrees<> yaw, degrees<> roll, const CartesianTuple& podStation)
{
	const double pixelsPerMeter = 1.6;    // sizes the 15.7 m airframe to a legible map glyph
	const Color  white{255, 255, 255};

	const auto strokeLoop = [&](const CartesianVector& part) {
		const auto toPixel = [&](const CartesianTuple& v) { return projectBody(v, nadir, yaw, roll, pixelsPerMeter); };
		// Each edge joins vertex i to the next, the last wrapping back to the first, closing the loop.
		for (const std::size_t i : std::views::iota(std::size_t{0}, part.size()))
		{
			const Pixel a = toPixel(part[i]), b = toPixel(part[(i + 1) % part.size()]);
			frame.line(a, b, white);
			frame.line({a.row + 1, a.column}, {b.row + 1, b.column}, white);
		}
	};

	// At this map scale the airframe is only a few dozen pixels, so draw the hand-reduced ~13-point silhouette
	// (shape-defining vertices only) rather than the full 47-point outline -- the F-35 reads the same, uncluttered.
	strokeLoop(f35::silhouette());
	strokeLoop(f35::finLeft());
	strokeLoop(f35::finRight());

	const Pixel podPixel = projectBody(podStation, nadir, yaw, roll, pixelsPerMeter);
	frame.disc(podPixel, 2, Color{40, 160, 255});    // cyan pod
	return podPixel;
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: main
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Fly an aircraft entity across the Nevada tile on a maneuvering S-track carrying a rigidly-attached
///				wing sensor pod (a child entity), march the pod's ray to the terrain each step, and draw the path,
///				rays, and hit track on the hillshade so the geometry can be verified by eye.
/// @param[in]	argc	argument count.
/// @param[in]	argv	[tilePath] [outPath] [frameDir] -- the DTED tile, the still-image output, and an optional
///						directory for per-step video frames.
/// @return		0 on success.
//----------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	const std::string           tilePath = (argc > 1) ? argv[1] : "test/resources/w115_n37.dt2";
	const std::string           outPath  = (argc > 2) ? argv[2] : "sensorTerrainHit.ppm";
	const std::filesystem::path frameDir = (argc > 3) ? argv[3] : "";

	DTEDTile tile(tilePath);
	tile.load();

	HillshadeCanvas    canvas(&tile, 9.0_arcsec);
	const TileMetadata meta = tile.metadata();
	const degrees<>    swLat = meta.southwestLatitude(), neLat = meta.northeastLatitude();
	const degrees<>    swLon = meta.southwestLongitude(), neLon = meta.northeastLongitude();

	// The hard-mounted wing pod: half-way out the right wing (2.5 m), boresight 55 deg below the forward axis
	// and canted 30 deg right of the nose. As a CHILD entity's mount, the offset places the pod and the mount
	// rotation aims its forward (+x) axis along that body-axis boresight, so `pod.ray()` looks down-and-right.
	const degrees<>            depression{55.0}, cant{30.0};
	const CartesianTuple       boresightBody(m * cos(depression) * cos(cant), m * cos(depression) * sin(cant), m * sin(depression));
	const rotation::Quaternion mountAim = rotation::Quaternion::fromTwoVectors({1.0, 0.0, 0.0}, boresightBody.normalized());
	const Entity<Wgs>::Mount   podMount{CartesianTuple(0.0_m, 2.5_m, 0.0_m), Pose(CartesianTuple(0.0_m, 0.0_m, 0.0_m), mountAim)};

	// Flight: entering from the east heading west, maneuvering across the tile.
	const Lla               startLla(37.35_deg, -114.15_deg, 6000.0_m);
	const meters_per_second<> speed{220.0};
	const seconds<>         dt{0.5};
	const int               steps = 260;

	std::cout << std::format("=== Sensor -> terrain flythrough (colour{}) ===\n", frameDir.empty() ? "" : ", video frames");

	Image path  = canvas.blank();    // persistent flight path + hit trail
	Lla   acLla = startLla;
	int   hits = 0, misses = 0, frameNo = 0;
	Pixel prevNad{-1, -1};

	// The maneuver spans two full turns of the phase across the track; the angular frequency drives the roll cue.
	const radians<> perStep     = radians<>(2.0 * (2.0 * std::numbers::pi) / steps);
	const hertz<>   angularRate = perStep / dt / 1.0_rad;

	for (const int i : std::views::iota(0, steps))
	{
		const FlightAttitude att = maneuver(perStep * static_cast<double>(i), angularRate);

		// Advance the aircraft along its heading over the ellipsoid with the direct-geodesic solver.
		acLla = geodesicDirect(acLla, att.yaw, speed * dt).destination();
		acLla.setAltitude(6000.0_m);
		const degrees<> lat = acLla.latitude().to<degrees<>>(), lon = acLla.longitude().to<degrees<>>();
		if (lat < swLat || lat > neLat || lon < swLon || lon > neLon)
			break;

		// The aircraft entity at this pose, with the pod attached as a rigid child. The pod's ray resolves to
		// world through the parent chain -- no hand-rolled body->wing->ECEF composition.
		Entity<Wgs>  aircraft(PositionECEF<Wgs>(acLla), Pose(PositionECEF<Wgs>(acLla).point(), nedToEcefAttitude(acLla, att.yaw, att.pitch, att.roll)));
		Entity<Wgs>& pod = aircraft.attach(podMount);
		const auto   hit = terrainIntersection(pod.ray());

		const Pixel nad = canvas.project(acLla);
		if (prevNad.row >= 0)
			path.line(prevNad, nad, Color{0, 220, 255});    // persistent flight path, cyan
		prevNad = nad;

		hit.has_value() ? ++hits : ++misses;

		// The pod's mount aims +x along the boresight, so a cone of half-angle `coneHalfAngle` about +x is the set
		// of body directions (cos a, sin a cos t, sin a sin t) for clock angle t. March each to the terrain through
		// the pod's world pose and project the hit: the ring is the FOV footprint on the ground.
		const degrees<>       coneHalfAngle{4.0};
		const dimensionless<> sinA = sin(coneHalfAngle), cosA = cos(coneHalfAngle);
		std::vector<Pixel>    footprint;
		for (const int k : std::views::iota(0, 48))
		{
			const radians<>      clock   = radians<>(2.0 * std::numbers::pi * k / 48);
			const CartesianTuple edge    = {m * cosA, m * sinA * cos(clock), m * sinA * sin(clock)};
			const auto           edgeHit = terrainIntersection(Ray<Entity<Wgs>::frame_type>::fromPose(pod.pose(), edge));
			if (edgeHit.has_value())
				footprint.push_back(canvas.project(edgeHit->hit));
		}

		// Stamp the footprint ring onto the PERSISTENT layer each step: the overlapping rings accumulate into a
		// swept-coverage band tracing where the sensor has looked over the whole track (dim orange).
		for (const std::size_t k : std::views::iota(std::size_t{0}, footprint.size()))
			path.line(footprint[k], footprint[(k + 1) % footprint.size()], Color{150, 90, 30});

		// Per-frame overlay: the aircraft glyph, the two cone-bound rays, and the current bright footprint.
		if (!frameDir.empty())
		{
			Image       frame    = path;
			const Pixel podPixel = drawAircraftGlyph(frame, nad, att.yaw, att.roll, podMount.offset);

			for (const std::size_t k : std::views::iota(std::size_t{0}, footprint.size()))    // current footprint, bright orange
				frame.line(footprint[k], footprint[(k + 1) % footprint.size()], Color{255, 150, 40});

			// The two roll-plane bound rays (clock +/-90 deg), pod -> footprint edge, yellow.
			for (const double side : {1.0, -1.0})
			{
				const auto boundHit = terrainIntersection(Ray<Entity<Wgs>::frame_type>::fromPose(pod.pose(), CartesianTuple(m * cosA, 0.0_m, m * sinA * side)));
				if (boundHit.has_value())
					frame.line(podPixel, canvas.project(boundHit->hit), Color{255, 240, 0});
			}

			canvas.writePpm(frame, (frameDir / std::format("frame_{:04}.ppm", frameNo++)).string());
		}
	}

	// Final still: a bold aircraft marker at the start, over the path + swept coverage band.
	path.disc(canvas.project(startLla), 4, Color{255, 255, 255});
	path.disc(canvas.project(startLla), 1, Color{0, 0, 0});
	canvas.writePpm(path, outPath);

	std::cout << std::format("Terrain hits: {}  misses: {}\n", hits, misses);
	std::cout << "Colours: cyan = flight path, yellow = cone-bound rays, orange = swept sensor coverage, white = aircraft.\n";
	std::cout << std::format("Still: {}{}\n", outPath, frameDir.empty() ? "" : "   (frames in the given dir)");
	return 0;
}
