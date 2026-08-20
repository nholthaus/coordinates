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
#include <cmath>
#include <numbers>
#include <cstdio>
#include <string>

#include "coordinates.h"
#include "dtedTile.h"

using namespace coordinates;
using namespace coordinates::topography;
using namespace coordinates::datums;
using namespace units::literals;

using Wgs = WGS84_G1674;
using Lla = PositionGeodetic<Wgs>;

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
	const rotation::RotationMatrix nedToEcef(std::get<0>(n.vector()) / 1.0_m, std::get<0>(e.vector()) / 1.0_m, std::get<0>(d.vector()) / 1.0_m,
	                                         std::get<1>(n.vector()) / 1.0_m, std::get<1>(e.vector()) / 1.0_m, std::get<1>(d.vector()) / 1.0_m,
	                                         std::get<2>(n.vector()) / 1.0_m, std::get<2>(e.vector()) / 1.0_m, std::get<2>(d.vector()) / 1.0_m);
	return rotation::toQuaternion(nedToEcef) * rotation::toQuaternion(rotation::EulerAngles{yaw, pitch, roll});
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: main
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Fly an aircraft entity across the Nevada tile on a maneuvering S-track carrying a rigidly-attached
///				wing sensor pod (a child entity), march the pod's ray to the terrain each step, and draw the path,
///				rays, and hit track on the hillshade so the geometry can be verified by eye.
//----------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	const std::string tilePath = (argc > 1) ? argv[1] : "test/resources/w115_n37.dt2";
	const std::string outPath  = (argc > 2) ? argv[2] : "sensorTerrainHit.ppm";
	const std::string frameDir = (argc > 3) ? argv[3] : "";

	DTEDTile tile(tilePath);
	tile.load();

	HillshadeCanvas    canvas(&tile, 9.0_arcsec);
	const TileMetadata meta = tile.metadata();
	const double       swLat = meta.southwestLatitude().value(), neLat = meta.northeastLatitude().value();
	const double       swLon = meta.southwestLongitude().value(), neLon = meta.northeastLongitude().value();

	// The hard-mounted wing pod: half-way out the right wing (2.5 m), boresight 55 deg below the forward axis
	// and canted 30 deg right of the nose. As a CHILD entity's mount, the offset places the pod and the mount
	// rotation aims its forward (+x) axis along that body-axis boresight, so `pod.ray()` looks down-and-right.
	const degrees<>      depression{55.0}, cant{30.0};
	const double         cosDep = units::cos(depression).to<double>();
	const CartesianTuple boresightBody(units::length::meters<>(cosDep * units::cos(cant).to<double>()),
	                                   units::length::meters<>(cosDep * units::sin(cant).to<double>()),
	                                   units::length::meters<>(units::sin(depression).to<double>()));
	const rotation::Quaternion mountAim = rotation::Quaternion::fromTwoVectors({1.0, 0.0, 0.0},
	                                                                           {std::get<0>(boresightBody).value(), std::get<1>(boresightBody).value(), std::get<2>(boresightBody).value()});
	const Entity<Wgs>::Mount   podMount{CartesianTuple(0.0_m, 2.5_m, 0.0_m), Pose(CartesianTuple(0.0_m, 0.0_m, 0.0_m), mountAim)};

	// Flight: entering from the east heading west, maneuvering across the tile.
	Lla                          startLla(37.35_deg, -114.15_deg, 6000.0_m);
	const meters_per_second<>    speed{220.0};
	const seconds<>              dt{0.5};
	const int                    steps = 260;

	std::printf("=== Sensor -> terrain flythrough (colour%s) ===\n", frameDir.empty() ? "" : ", video frames");

	Image     path  = canvas.blank();    // persistent flight path + hit trail
	Lla       acLla = startLla;
	int       hits = 0, misses = 0, frameNo = 0;
	Pixel     prevNad{-1, -1};

	for (int i = 0; i < steps; ++i)
	{
		// Maneuver control law: yaw/roll/pitch as smooth functions of time (the commanded angles).
		const double period = (steps * dt.value()) / 2.0;
		const double w      = 2.0 * std::numbers::pi / period;
		const double phase  = w * (i * dt.value());
		const double yawAmp = 40.0;
		const auto   yaw    = degrees<>(270.0 + yawAmp * std::sin(phase));
		const double yawRate= yawAmp * w * std::cos(phase);
		const auto   roll   = degrees<>(std::clamp(12.0 * yawRate, -45.0, 45.0));
		const auto   pitch  = degrees<>(-4.0 + 6.0 * std::sin(phase + 1.0));

		// Advance the aircraft along its heading over the ellipsoid with the direct-geodesic solver.
		acLla = geodesicDirect(acLla, yaw, meters<>(speed.value() * dt.value())).destination();
		acLla.setAltitude(6000.0_m);
		const double lat = acLla.latitude().to<degrees<>>().value(), lon = acLla.longitude().to<degrees<>>().value();
		if (lat < swLat || lat > neLat || lon < swLon || lon > neLon)
			break;

		// The aircraft entity at this pose, with the pod attached as a rigid child. The pod's ray resolves to
		// world through the parent chain -- no hand-rolled body->wing->ECEF composition.
		Entity<Wgs>  aircraft(PositionECEF<Wgs>(acLla), Pose(PositionECEF<Wgs>(acLla).point(), nedToEcefAttitude(acLla, yaw, pitch, roll)));
		Entity<Wgs>& pod = aircraft.attach(podMount);
		const auto   hit = terrainIntersection(pod.ray());

		const Pixel nad = canvas.project(acLla);
		if (prevNad.row >= 0)
			path.line(prevNad, nad, Color{0, 220, 255});    // persistent flight path, cyan
		prevNad = nad;

		Pixel hitPixel{-1, -1};
		if (hit.has_value())
		{
			hitPixel = canvas.project(hit->hit);
			path.plot(hitPixel, Color{255, 60, 60});    // persistent red hit trail
			++hits;
		}
		else
			++misses;

		// Per-frame overlay: the aircraft glyph (heading + bank) and the live sensor ray, on a copy of the path.
		if (!frameDir.empty())
		{
			Image        frame = path;
			auto         thick = [&](double r0, double c0, double r1, double c1, Color col) {
                frame.line(Pixel{(int) std::lround(r0), (int) std::lround(c0)}, Pixel{(int) std::lround(r1), (int) std::lround(c1)}, col);
                frame.line(Pixel{(int) std::lround(r0) + 1, (int) std::lround(c0)}, Pixel{(int) std::lround(r1) + 1, (int) std::lround(c1)}, col);
			};
			// Screen-space heading (row grows south, col grows east): nose (-cos yaw, +sin yaw), starboard (sin, cos).
			const double yr = units::angle::radians<>(yaw).value();
			const double hRow = -std::cos(yr), hCol = std::sin(yr), wRow = std::sin(yr), wCol = std::cos(yr);
			const double noseLen = 13.0, tailLen = 7.0, wingSpan = 13.0, tailSpan = 5.0;
			const double cphi = std::cos(units::angle::radians<>(roll).value());    // bank foreshortens the projected span
			const Color  white{255, 255, 255};
			// Fuselage (tail -> nose).
			thick(nad.row - hRow * tailLen, nad.column - hCol * tailLen, nad.row + hRow * noseLen, nad.column + hCol * noseLen, white);
			// Wing: rigidly perpendicular to the fuselage, projected half-span foreshortens as span*cos(roll).
			const double wingRow = nad.row + hRow * 2.0, wingCol = nad.column + hCol * 2.0, halfSpan = wingSpan * cphi;
			thick(wingRow, wingCol, wingRow + halfSpan * wRow, wingCol + halfSpan * wCol, white);
			thick(wingRow, wingCol, wingRow - halfSpan * wRow, wingCol - halfSpan * wCol, white);
			// Tailplane cross-bar.
			const double tailRow = nad.row - hRow * tailLen, tailCol = nad.column - hCol * tailLen;
			thick(tailRow, tailCol, tailRow + wRow * tailSpan, tailCol + wCol * tailSpan, white);
			thick(tailRow, tailCol, tailRow - wRow * tailSpan, tailCol - wCol * tailSpan, white);
			// Pod at the wing midpoint (rides heading + bank with the wing); the live ray is drawn from it.
			const Pixel pod{(int) std::lround(wingRow + 0.5 * halfSpan * wRow), (int) std::lround(wingCol + 0.5 * halfSpan * wCol)};
			if (hit.has_value())
				frame.line(pod, hitPixel, Color{255, 240, 0});    // live sensor ray, yellow
			frame.disc(pod, 2, Color{40, 160, 255});              // cyan pod
			char name[512];
			std::snprintf(name, sizeof(name), "%s/frame_%04d.ppm", frameDir.c_str(), frameNo++);
			canvas.writePpm(frame, name);
		}
	}

	// Final still: a bold aircraft marker at the start, over the path + hit trail.
	path.disc(canvas.project(startLla), 4, Color{255, 255, 255});
	path.disc(canvas.project(startLla), 1, Color{0, 0, 0});
	canvas.writePpm(path, outPath);

	std::printf("Terrain hits: %d  misses: %d\n", hits, misses);
	std::printf("Colours: cyan = flight path, yellow = live sensor ray, red = terrain-hit trail, white = aircraft.\n");
	std::printf("Still: %s%s\n", outPath.c_str(), frameDir.empty() ? "" : "   (frames in the given dir)");
	return 0;
}
