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
// End-to-end case study exercising the whole substrate: an aircraft flies a maneuvering (pitch/roll/yaw)
// track over Nevada terrain carrying a HARD-MOUNTED sensor pod out on the wing. The pod's boresight is fixed
// in the aircraft's BODY axes -- down and to the right of the nose -- so the pod rolls rigidly WITH the
// aircraft. That is the whole point: as the aircraft banks through its turns, the fixed body-axis look
// direction sweeps across the ground (its ground azimuth relative to the nose is only constant when
// wings-level; a hard bank swings it), which is exactly the complex body->local->ECEF axis composition this
// example demonstrates. Each step the pod's ray is resolved in ECEF, marched to the DTED terrain surface,
// and the geodetic (LLA) hit is recorded; the flight path and hit track are drawn on the tile's hillshade so
// the geometry can be verified by eye.
//
// The pipeline is pure composition of existing pieces: local-level (NED->ECEF) rotation * body attitude
// (yaw/pitch/roll) * wing-mount offset * fixed body-axis boresight -> ECEF ray -> terrain march -> LLA. The
// aircraft is advanced over the ellipsoid with the library's geodesicDirect solver; angles are unit-typed.
//
//--------------------------------------------------------------------------------------------------

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "coordinates.h"
#include "dtedTile.h"

using namespace coordinates;
using namespace coordinates::topography;
using namespace coordinates::datums;
using namespace units::literals;

using Wgs        = WGS84_G1674;
using WgsHoriz   = horizontalDatums::WGS84_G1674;
using EcefFrame  = coordinateFrames::ECEFFrame<WgsHoriz>;
using Ecef       = PositionECEF<Wgs>;
using Lla        = PositionGeodetic<Wgs>;

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: marchToTerrain [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		March an ECEF ray from a start point along a direction until it first crosses below the DTED
///				terrain surface, and return the geodetic position of that terrain hit.
/// @details	Samples the ray at a fixed step, converts each sample to geodetic, and compares the sample's
///				geodetic height to the terrain elevation at its latitude/longitude. The hit is bracketed at
///				the first sample whose height falls below the terrain, then refined by bisection between the
///				last-above and first-below samples. This is the terrain line-of-sight / masking primitive: the
///				ray's first occlusion by terrain.
/// @param[in]	tile		the DTED tile providing terrain elevation.
/// @param[in]	startEcef	the ray origin in ECEF (the sensor position).
/// @param[in]	dirEcef		the ray direction in ECEF (need not be normalized).
/// @param[out]	hitLla		the geodetic position of the terrain hit (valid only if the function returns true).
/// @return		true if the ray hit the terrain within the search range, false otherwise.
//----------------------------------------------------------------------------------------------------------------------
static bool marchToTerrain(const DTEDTile& tile, const Ecef& startEcef, const CartesianTuple& dirEcef, Lla& hitLla)
{
	// Normalize the direction.
	const double dx = std::get<0>(dirEcef).value(), dy = std::get<1>(dirEcef).value(), dz = std::get<2>(dirEcef).value();
	const double dn = std::sqrt(dx * dx + dy * dy + dz * dz);
	if (dn == 0.0)
		return false;
	const double ux = dx / dn, uy = dy / dn, uz = dz / dn;

	const double x0 = startEcef.x().value(), y0 = startEcef.y().value(), z0 = startEcef.z().value();

	const double step      = 5.0;         // metres per coarse step
	const double maxRange  = 200000.0;    // 200 km search
	double       prevT     = 0.0;
	double       prevAbove = 1.0;         // height-above-terrain at the previous sample (start is well above)

	const double swLatB = tile.metadata().southwestLatitude().value(), neLatB = tile.metadata().northeastLatitude().value();
	const double swLonB = tile.metadata().southwestLongitude().value(), neLonB = tile.metadata().northeastLongitude().value();

	// Returns the ray-height-minus-terrain at range t; sets `inBounds` false if the sample leaves the tile.
	auto heightAboveTerrain = [&](double t, Lla& lla, bool& inBounds) -> double {
		Ecef p(units::length::meters<>(x0 + ux * t), units::length::meters<>(y0 + uy * t), units::length::meters<>(z0 + uz * t));
		lla = Lla(p);    // ECEF -> geodetic
		const double lat = lla.latitude().template to<units::angle::degrees<>>().value();
		const double lon = lla.longitude().template to<units::angle::degrees<>>().value();
		inBounds         = (lat >= swLatB && lat <= neLatB && lon >= swLonB && lon <= neLonB);
		if (!inBounds)
			return 1.0;    // no terrain data off-tile; treat as "above" so the march ends without a false hit
		const double h       = lla.altitude().template to<units::length::meters<>>().value();
		const double terrain = tile.elevation(units::angle::degrees<>(lat), units::angle::degrees<>(lon)).value();
		return h - terrain;
	};

	Lla  lla;
	bool inBounds = true;
	for (double t = step; t <= maxRange; t += step)
	{
		const double above = heightAboveTerrain(t, lla, inBounds);
		if (!inBounds)
			return false;    // ray left the tile before hitting terrain
		if (above <= 0.0)
		{
			// Bracketed between prevT (above) and t (below): bisect to refine.
			double lo = prevT, hi = t;
			for (int i = 0; i < 40; ++i)
			{
				const double mid = 0.5 * (lo + hi);
				bool         ib;
				const double a = heightAboveTerrain(mid, lla, ib);
				if (a > 0.0)
					lo = mid;
				else
					hi = mid;
			}
			bool ib;
			heightAboveTerrain(0.5 * (lo + hi), hitLla, ib);
			return true;
		}
		prevT     = t;
		prevAbove = above;
		(void) prevAbove;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: nedToEcefRotation [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		The rotation that takes a vector expressed in the local NED frame at a geodetic point into
///				ECEF axes, as a quaternion.
/// @details	The NED axes in ECEF are the columns of the direction-cosine matrix built from the geodetic
///				latitude and longitude: North = (-sinφcosλ, -sinφsinλ, cosφ), East = (-sinλ, cosλ, 0),
///				Down = (-cosφcosλ, -cosφsinλ, -sinφ). Composing this with a body-to-NED attitude gives the
///				full body-to-ECEF rotation an aircraft pose needs, so a body "forward/down" points at the
///				local terrain rather than at the ECEF axes.
/// @param[in]	latitude	the geodetic latitude.
/// @param[in]	longitude	the geodetic longitude.
/// @return		the NED-to-ECEF rotation as a quaternion.
//----------------------------------------------------------------------------------------------------------------------
static rotation::Quaternion nedToEcefRotation(units::angle::degrees<> latitude, units::angle::degrees<> longitude)
{
	const double phi = units::angle::radians<>(latitude).value();
	const double lam = units::angle::radians<>(longitude).value();
	const double sp = std::sin(phi), cp = std::cos(phi), sl = std::sin(lam), cl = std::cos(lam);
	// Row-major DCM whose columns are [North, East, Down] expressed in ECEF (v_ecef = M * v_ned).
	const rotation::RotationMatrix m(units::dimensionless<>(-sp * cl), units::dimensionless<>(-sl), units::dimensionless<>(-cp * cl),
	                                 units::dimensionless<>(-sp * sl), units::dimensionless<>(cl), units::dimensionless<>(-cp * sl),
	                                 units::dimensionless<>(cp), units::dimensionless<>(0.0), units::dimensionless<>(-sp));
	return rotation::toQuaternion(m);
}


//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: latLonToPixel [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Map a geodetic latitude/longitude to a (row, col) pixel in the hillshade raster.
/// @details	Row 0 is the northernmost latitude, column 0 the westernmost longitude, matching how
///				`hillshade` lays out the tile.
/// @param[in]	lat		the latitude, in degrees.
/// @param[in]	lon		the longitude, in degrees.
/// @param[in]	meta	the tile metadata (bounds).
/// @param[in]	rows	the raster row count.
/// @param[in]	cols	the raster column count.
/// @param[out]	row		the resulting pixel row.
/// @param[out]	col		the resulting pixel column.
//----------------------------------------------------------------------------------------------------------------------
static void latLonToPixel(double lat, double lon, const TileMetadata& meta, int rows, int cols, int& row, int& col)
{
	const double swLat = meta.southwestLatitude().value(), neLat = meta.northeastLatitude().value();
	const double swLon = meta.southwestLongitude().value(), neLon = meta.northeastLongitude().value();
	row = static_cast<int>(std::lround((neLat - lat) / (neLat - swLat) * (rows - 1)));
	col = static_cast<int>(std::lround((lon - swLon) / (neLon - swLon) * (cols - 1)));
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: main
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Fly an aircraft across the Nevada DTED tile on a gently maneuvering (pitch/roll/yaw) S-track,
///				with a hard-mounted wing sensor pod fixed in body axes (down and right of the nose, rolling with the
///				aircraft), march the pod ray to the terrain each step, and draw the path, rays, and hit track on the
///				hillshade so the whole thing can be verified by eye.
//----------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	// Usage: sensorTerrainHit [tile] [outStill] [frameDir]
	//   tile     : DTED tile (default the Nevada test tile)
	//   outStill : final composited still image, PPM (default sensorTerrainHit.ppm)
	//   frameDir : if given, write one PPM frame per timestep here for encoding to video
	const std::string tilePath = (argc > 1) ? argv[1] : "test/resources/w115_n37.dt2";
	const std::string outPath  = (argc > 2) ? argv[2] : "sensorTerrainHit.ppm";
	const std::string frameDir = (argc > 3) ? argv[3] : "";

	DTEDTile tile(tilePath);
	tile.load();
	const TileMetadata meta = tile.metadata();

	// Colorized hillshade background: shade drives brightness, elevation drives a warm terrain hue so the
	// relief reads in colour rather than flat grey.
	const auto shade = coordinates::hillshade(&tile, units::angle::arcseconds<>(9.0));
	const int  rows  = static_cast<int>(shade.size());
	const int  cols  = rows > 0 ? static_cast<int>(shade[0].size()) : 0;

	// RGB framebuffer, row-major, 3 bytes/pixel.
	std::vector<uint8_t> base(static_cast<std::size_t>(rows) * cols * 3, 0);
	const double swLat = meta.southwestLatitude().value(), neLat = meta.northeastLatitude().value();
	const double swLon = meta.southwestLongitude().value(), neLon = meta.northeastLongitude().value();
	// Precompute an elevation range for the terrain colour ramp.
	double zlo = 1e9, zhi = -1e9;
	for (int r = 0; r < rows; ++r)
		for (int c = 0; c < cols; ++c)
		{
			const double lat = neLat - (neLat - swLat) * r / (rows - 1);
			const double lon = swLon + (neLon - swLon) * c / (cols - 1);
			const double z   = tile.elevation(units::angle::degrees<>(lat), units::angle::degrees<>(lon)).value();
			if (z < zlo) zlo = z;
			if (z > zhi) zhi = z;
		}
	for (int r = 0; r < rows; ++r)
		for (int c = 0; c < cols; ++c)
		{
			const double lat = neLat - (neLat - swLat) * r / (rows - 1);
			const double lon = swLon + (neLon - swLon) * c / (cols - 1);
			const double z   = tile.elevation(units::angle::degrees<>(lat), units::angle::degrees<>(lon)).value();
			const double f   = (zhi > zlo) ? (z - zlo) / (zhi - zlo) : 0.0;    // 0 low .. 1 high
			const double sh  = shade[r][c] / 255.0;                            // hillshade brightness
			// Low = green valley, high = brown/white ridge; modulate by shade.
			const double rr = (0.30 + 0.70 * f) * sh;
			const double gg = (0.45 + 0.35 * f) * sh;
			const double bb = (0.25 + 0.20 * f) * sh;
			const std::size_t i = (static_cast<std::size_t>(r) * cols + c) * 3;
			base[i + 0] = static_cast<uint8_t>(std::lround(255.0 * std::min(1.0, rr)));
			base[i + 1] = static_cast<uint8_t>(std::lround(255.0 * std::min(1.0, gg)));
			base[i + 2] = static_cast<uint8_t>(std::lround(255.0 * std::min(1.0, bb)));
		}

	// The live framebuffer we composite onto (starts as the terrain, overlays accumulate).
	std::vector<uint8_t> img = base;

	auto put = [&](int r, int c, uint8_t R, uint8_t G, uint8_t B) {
		if (r < 0 || r >= rows || c < 0 || c >= cols) return;
		const std::size_t i = (static_cast<std::size_t>(r) * cols + c) * 3;
		img[i + 0] = R; img[i + 1] = G; img[i + 2] = B;
	};
	auto disc = [&](int r, int c, int radius, uint8_t R, uint8_t G, uint8_t B) {
		for (int dr = -radius; dr <= radius; ++dr)
			for (int dc = -radius; dc <= radius; ++dc)
				if (dr * dr + dc * dc <= radius * radius)
					put(r + dr, c + dc, R, G, B);
	};
	auto line = [&](int r0, int c0, int r1, int c1, uint8_t R, uint8_t G, uint8_t B) {
		int dr = std::abs(r1 - r0), dc = std::abs(c1 - c0);
		int sr = r0 < r1 ? 1 : -1, sc = c0 < c1 ? 1 : -1;
		int err = dc - dr;
		while (true)
		{
			put(r0, c0, R, G, B);
			if (r0 == r1 && c0 == c1) break;
			int e2 = 2 * err;
			if (e2 > -dr) { err -= dr; c0 += sc; }
			if (e2 < dc)  { err += dc; r0 += sr; }
		}
	};
	auto writePpm = [&](const std::string& path, const std::vector<uint8_t>& buf) {
		std::ofstream out(path, std::ios::binary);
		out << "P6\n" << cols << " " << rows << "\n255\n";
		out.write(reinterpret_cast<const char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
	};

	// --- Flight: entering from the east, heading west, maneuvering (pitch/roll/yaw) across the tile. ---
	Lla                          startLla(37.35_deg, -114.15_deg, 6000.0_m);
	const units::velocity::meters_per_second<> speed{220.0};
	const units::time::seconds<>               dt{0.5};
	const int                                  steps = 260;

	// Body-to-ECEF rotation for an aircraft at geodetic point `p` with the given (typed) yaw/pitch/roll:
	// the local-level NED-to-ECEF rotation composed with the body-to-NED attitude.
	auto bodyToEcefAt = [](const Lla& p, units::angle::degrees<> yaw, units::angle::degrees<> pitch, units::angle::degrees<> roll) {
		const rotation::EulerAngles att{yaw, pitch, roll};
		const rotation::Quaternion  n2e = nedToEcefRotation(p.latitude().to<units::angle::degrees<>>(), p.longitude().to<units::angle::degrees<>>());
		return n2e * rotation::toQuaternion(att);
	};

	// Hard-mounted wing pod, FIXED in BODY axes: a pod half-way out the right wing (~2.5 m from the
	// centreline on a ~5 m half-span), boresight 55 deg down from the forward axis and canted 30 deg to the
	// right of the nose. It does NOT gimbal-stabilize -- it rolls rigidly with the aircraft, so its GROUND
	// look direction is fixed relative to the body but SWEEPS relative to the terrain as the aircraft banks:
	// the hit sits ~30 deg right of the nose only when wings-level, and swings well off that (even left of
	// the nose track at hard opposite bank) through the +/-45 deg turns. That sweep is the point -- it is the
	// full body->local-level->ECEF axis transform made visible. The 55 deg depression keeps the ground range
	// (~alt/tan(dep) ~= 4 km) modest so the hit track stays on the tile. `acPose * wingMount` also carries
	// the off-axis pod position up/down as the aircraft banks (the mid-wing station rides the bank).
	const Pose                    wingMount(CartesianTuple(0.0_m, 2.5_m, 0.0_m), rotation::Quaternion::identity());
	const units::angle::degrees<> depression{55.0};    // below the forward axis
	const units::angle::degrees<> cant{30.0};          // right of the nose
	// Body-axis boresight (x forward, y right, z down) as a direction, built with the unit-typed trig: the
	// angles are typed (units::cos/sin take a units::angle and return a dimensionless), and the resulting
	// direction cosines populate a Cartesian direction (its magnitude is arbitrary; the ray is normalized).
	const double cosDep = units::cos(depression).to<double>();
	const CartesianTuple boresightBody(units::length::meters<>(cosDep * units::cos(cant).to<double>()),
	                                   units::length::meters<>(cosDep * units::sin(cant).to<double>()),
	                                   units::length::meters<>(units::sin(depression).to<double>()));

	std::printf("=== Sensor -> terrain flythrough (colour%s) ===\n", frameDir.empty() ? "" : ", video frames");
	std::printf("Tile lat [%.1f,%.1f] lon [%.1f,%.1f] | %d steps @ %.1f s | speed %.0f m/s\n",
	            swLat, neLat, swLon, neLon, steps, dt.value(), speed.value());

	Lla acLla = startLla;
	int hits = 0, misses = 0, frameNo = 0;
	int prevNadR = -1, prevNadC = -1;

	for (int i = 0; i < steps; ++i)
	{
		// Maneuver control law: yaw/roll/pitch as smooth functions of time. These are the commanded angles a
		// control law produces, so they are plain scalars here, wrapped into typed angles for the pose.
		const units::time::seconds<> t{i * dt.value()};
		const double period = (steps * dt.value()) / 2.0;
		const double w      = 2.0 * 3.14159265358979323846 / period;
		const double phase  = w * t.value();
		const double yawAmp = 40.0;
		const auto   yaw    = units::angle::degrees<>(270.0 + yawAmp * std::sin(phase));    // heading (from north, +east)
		const double yawRate= yawAmp * w * std::cos(phase);
		const auto   roll   = units::angle::degrees<>(std::max(-45.0, std::min(45.0, 12.0 * yawRate)));
		const auto   pitch  = units::angle::degrees<>(-4.0 + 6.0 * std::sin(phase + 1.0));

		// Advance the aircraft along its heading over the ELLIPSOID using the library's direct-geodesic
		// solver -- walk `speed * dt` from the current position on bearing `yaw`. No flat-earth double math:
		// the typed geodesic API returns the new geodetic point.
		const units::length::meters<> stepDist{speed.value() * dt.value()};
		acLla = geodesicDirect(acLla, yaw, stepDist).destination();
		acLla.setAltitude(6000.0_m);    // hold altitude for the demo
		const double newLat = acLla.latitude().to<units::angle::degrees<>>().value();
		const double newLon = acLla.longitude().to<units::angle::degrees<>>().value();
		if (newLat < swLat || newLat > neLat || newLon < swLon || newLon > neLon) break;

		const Ecef acEcef(acLla);
		const Pose acPose(acEcef.point(), bodyToEcefAt(acLla, yaw, pitch, roll));
		const Pose sensorPose = acPose * wingMount;
		Ecef sensorEcef; sensorEcef.setPoint(sensorPose.translation());
		const CartesianTuple boresightEcef = sensorPose.rotateDirection(boresightBody);

		int nadR, nadC;
		latLonToPixel(newLat, newLon, meta, rows, cols, nadR, nadC);

		// Persistent flight path (cyan) drawn onto the accumulating image.
		if (prevNadR >= 0)
			line(prevNadR, prevNadC, nadR, nadC, 0, 220, 255);
		prevNadR = nadR; prevNadC = nadC;

		Lla hitLla;
		bool gotHit = marchToTerrain(tile, sensorEcef, boresightEcef, hitLla);
		int hR = -1, hC = -1;
		if (gotHit)
		{
			const double hLat = hitLla.latitude().to<units::angle::degrees<>>().value();
			const double hLon = hitLla.longitude().to<units::angle::degrees<>>().value();
			latLonToPixel(hLat, hLon, meta, rows, cols, hR, hC);
			put(hR, hC, 255, 60, 60);    // persistent red hit trail
			++hits;
		}
		else ++misses;

		// --- Per-frame overlay: current aircraft (bright) + current sensor ray (yellow), drawn on a copy so
		//     they don't persist as clutter; the path + hit trail persist in `img`. ---
		if (!frameDir.empty())
		{
			std::vector<uint8_t> frame = img;    // path + hits so far
			auto putF = [&](int r, int c, uint8_t R, uint8_t G, uint8_t B) {
				if (r < 0 || r >= rows || c < 0 || c >= cols) return;
				const std::size_t k = (static_cast<std::size_t>(r) * cols + c) * 3;
				frame[k + 0] = R; frame[k + 1] = G; frame[k + 2] = B;
			};
			auto lineF = [&](int r0, int c0, int r1, int c1, uint8_t R, uint8_t G, uint8_t B) {
				int dr = std::abs(r1 - r0), dc = std::abs(c1 - c0);
				int sr = r0 < r1 ? 1 : -1, sc = c0 < c1 ? 1 : -1, err = dc - dr;
				while (true) { putF(r0, c0, R, G, B); if (r0 == r1 && c0 == c1) break; int e2 = 2 * err;
					if (e2 > -dr) { err -= dr; c0 += sc; } if (e2 < dc) { err += dc; r0 += sr; } }
			};

			// --- Aircraft sprite: a top-down glyph reading heading and bank. On the map, row increases south
			//     and col increases east, so the screen-space heading unit is (drow, dcol) = (-cos yaw, +sin yaw)
			//     and the right-wing (starboard) unit is its +90 deg rotation (sin yaw, cos yaw). The wing stays
			//     rigidly perpendicular to the fuselage and its projected half-span foreshortens as
			//     span*cos(phi) with the bank angle phi -- both tips shrink toward the body equally, so a steep
			//     bank draws a short wing and wings-level draws the full span. ---
			auto thickLine = [&](double r0, double c0, double r1, double c1, uint8_t R, uint8_t G, uint8_t B) {
				lineF((int) std::lround(r0), (int) std::lround(c0), (int) std::lround(r1), (int) std::lround(c1), R, G, B);
				lineF((int) std::lround(r0) + 1, (int) std::lround(c0), (int) std::lround(r1) + 1, (int) std::lround(c1), R, G, B);
			};
			const double yr  = units::angle::radians<>(yaw).value();
			const double hR_ = -std::cos(yr), hC_ = std::sin(yr);    // heading (nose) unit, screen
			const double wR_ = std::sin(yr), wC_ = std::cos(yr);     // starboard (right-wing) unit, screen
			const double noseLen = 13.0, tailLen = 7.0, wingSpan = 13.0, tailSpan = 5.0;
			const double phi = units::angle::radians<>(roll).value();    // bank angle (rad), + = right wing down
			const double cphi = std::cos(phi);
			// Fuselage (tail -> nose), white.
			thickLine(nadR - hR_ * tailLen, nadC - hC_ * tailLen, nadR + hR_ * noseLen, nadC + hC_ * noseLen, 255, 255, 255);
			// The wing stays RIGIDLY PERPENDICULAR to the fuselage. Banking rotates it about the fuselage
			// axis, so on a top-down (orthographic-from-above) view the projected half-span foreshortens
			// symmetrically as span*cos(phi): both tips shrink toward the body equally, staying on the
			// perpendicular line (at 90 deg bank the wing would project to a point). The tips do NOT sweep
			// fore/aft -- that would be yaw/sweep, not roll.
			const double wingRow = nadR + hR_ * 2.0, wingCol = nadC + hC_ * 2.0;
			const double halfSpan = wingSpan * cphi;    // foreshortened projected half-span
			const double rTipR = wingRow + halfSpan * wR_, rTipC = wingCol + halfSpan * wC_;
			const double lTipR = wingRow - halfSpan * wR_, lTipC = wingCol - halfSpan * wC_;
			thickLine(wingRow, wingCol, rTipR, rTipC, 255, 255, 255);
			thickLine(wingRow, wingCol, lTipR, lTipC, 255, 255, 255);
			// Tailplane (short cross-bar) so orientation reads even wings-level.
			const double tailRow = nadR - hR_ * tailLen, tailCol = nadC - hC_ * tailLen;
			thickLine(tailRow, tailCol, tailRow + wR_ * tailSpan, tailCol + wC_ * tailSpan, 255, 255, 255);
			thickLine(tailRow, tailCol, tailRow - wR_ * tailSpan, tailCol - wC_ * tailSpan, 255, 255, 255);
			// Sensor pod at the glyph's wing MIDPOINT -- half-way from the fuselage to the (foreshortened)
			// right wingtip. It is anchored to the wing geometry, so it rides heading and bank exactly with
			// the wing: as the aircraft banks, the pod foreshortens inboard along the wing with it, staying at
			// the mid-wing station through every maneuver. The live sensor ray is drawn FROM this pod.
			const double podR = wingRow + 0.5 * halfSpan * wR_;
			const double podC = wingCol + 0.5 * halfSpan * wC_;
			if (gotHit)
				lineF((int) std::lround(podR), (int) std::lround(podC), hR, hC, 255, 240, 0);    // ray from the pod
			for (int dr = -2; dr <= 2; ++dr) for (int dc = -2; dc <= 2; ++dc)
				if (dr * dr + dc * dc <= 4) putF((int) std::lround(podR) + dr, (int) std::lround(podC) + dc, 40, 160, 255);    // cyan pod
			char name[512];
			std::snprintf(name, sizeof(name), "%s/frame_%04d.ppm", frameDir.c_str(), frameNo++);
			writePpm(name, frame);
		}
	}

	// Final composited still: draw a bold aircraft at the start and write it.
	int sR, sC;
	latLonToPixel(startLla.latitude().to<units::angle::degrees<>>().value(),
	              startLla.longitude().to<units::angle::degrees<>>().value(), meta, rows, cols, sR, sC);
	disc(sR, sC, 4, 255, 255, 255);
	disc(sR, sC, 1, 0, 0, 0);
	writePpm(outPath, img);

	std::printf("Terrain hits: %d  misses: %d\n", hits, misses);
	std::printf("Colours: cyan = flight path, yellow = live sensor ray, red = terrain-hit trail, white = aircraft.\n");
	std::printf("Still: %s%s\n", outPath.c_str(), frameDir.empty() ? "" : "   (frames in the given dir)");
	return 0;
}
