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
// A ground RF site: a short tower stands on a hilltop and sweeps a horizontal beam (0 deg elevation) through
// a full 360 deg azimuth turn. Each azimuth, the beam is resolved as a level ECEF ray from the antenna and marched
// to the terrain surface; the first terrain intersection is a radio line-of-sight / horizon-mask point.
// As the beam rotates, the yellow ray sweeps the full azimuth -- drawn to the tile edge every frame -- and a
// red terrain-hit dot drops where a ridge occludes the beam. The red dots ACCUMULATE into the RF horizon
// footprint: close where a ridge blocks the beam, absent where the beam clears the tile.
//
// The geometry is unit-typed: the beam direction is built from a units::angle azimuth at 0 deg elevation in
// the local NED frame, rotated to ECEF, and the antenna sits a units::length::feet mast above the hilltop.
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

using Wgs = WGS84_G1674;
using Ecef = PositionECEF<Wgs>;
using Lla  = PositionGeodetic<Wgs>;

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: nedToEcefRotation [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		The rotation taking a vector expressed in the local NED frame at a geodetic point into ECEF.
/// @details	The NED axes in ECEF are the columns of the direction-cosine matrix from the geodetic latitude
///				and longitude: North = (-sinφcosλ, -sinφsinλ, cosφ), East = (-sinλ, cosλ, 0),
///				Down = (-cosφcosλ, -cosφsinλ, -sinφ). It lets a level, azimuth-defined beam in NED be expressed
///				as an ECEF ray so it can be marched against the terrain.
/// @param[in]	latitude	the geodetic latitude.
/// @param[in]	longitude	the geodetic longitude.
/// @return		the NED-to-ECEF rotation quaternion.
//----------------------------------------------------------------------------------------------------------------------
static rotation::Quaternion nedToEcefRotation(units::angle::degrees<> latitude, units::angle::degrees<> longitude)
{
	const double phi = units::angle::radians<>(latitude).value();
	const double lam = units::angle::radians<>(longitude).value();
	const double sp = std::sin(phi), cp = std::cos(phi), sl = std::sin(lam), cl = std::cos(lam);
	const rotation::RotationMatrix m(units::dimensionless<>(-sp * cl), units::dimensionless<>(-sl), units::dimensionless<>(-cp * cl),
	                                 units::dimensionless<>(-sp * sl), units::dimensionless<>(cl), units::dimensionless<>(-cp * sl),
	                                 units::dimensionless<>(cp), units::dimensionless<>(0.0), units::dimensionless<>(-sp));
	return rotation::toQuaternion(m);
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: marchToTerrain [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		March an ECEF ray from a start point until it first crosses below the DTED terrain, returning
///				the geodetic terrain-intersection point.
/// @details	Steps the ray, converts each sample to geodetic, and compares the sample height to the terrain
///				elevation at its lat/lon; the first below-terrain sample is bracketed and refined by bisection.
///				This is the terrain line-of-sight primitive: the ray's first occlusion by terrain.
/// @param[in]	tile		the DTED tile providing terrain elevation.
/// @param[in]	startEcef	the ray origin in ECEF (the antenna).
/// @param[in]	dirEcef		the ray direction in ECEF (need not be normalized).
/// @param[out]	hitLla		the geodetic terrain-intersection (valid only if the function returns true).
/// @param[out]	edgeLla		the last on-tile point along the ray (always valid) -- the ray's tile-edge exit,
///							used to draw the swept beam full length regardless of whether it was occluded.
/// @return		true if the ray hit terrain within the tile, false if it left the tile first.
//----------------------------------------------------------------------------------------------------------------------
static bool marchToTerrain(const DTEDTile& tile, const Ecef& startEcef, const CartesianTuple& dirEcef, Lla& hitLla, Lla& edgeLla)
{
	const double dx = std::get<0>(dirEcef).value(), dy = std::get<1>(dirEcef).value(), dz = std::get<2>(dirEcef).value();
	const double dn = std::sqrt(dx * dx + dy * dy + dz * dz);
	if (dn == 0.0)
		return false;
	const double ux = dx / dn, uy = dy / dn, uz = dz / dn;
	const double x0 = startEcef.x().value(), y0 = startEcef.y().value(), z0 = startEcef.z().value();

	const double swLat = tile.metadata().southwestLatitude().value(), neLat = tile.metadata().northeastLatitude().value();
	const double swLon = tile.metadata().southwestLongitude().value(), neLon = tile.metadata().northeastLongitude().value();

	auto heightAboveTerrain = [&](double t, Lla& lla, bool& inBounds) -> double {
		Ecef p(units::length::meters<>(x0 + ux * t), units::length::meters<>(y0 + uy * t), units::length::meters<>(z0 + uz * t));
		lla = Lla(p);
		const double lat = lla.latitude().template to<units::angle::degrees<>>().value();
		const double lon = lla.longitude().template to<units::angle::degrees<>>().value();
		inBounds         = (lat >= swLat && lat <= neLat && lon >= swLon && lon <= neLon);
		if (!inBounds)
			return 1.0;
		const double h       = lla.altitude().template to<units::length::meters<>>().value();
		const double terrain = tile.elevation(units::angle::degrees<>(lat), units::angle::degrees<>(lon)).value();
		return h - terrain;
	};

	Lla    lla;
	Lla    lastInBounds;    // the last sample still on the tile -- the ray's geometric tile-edge exit
	bool   inBounds = true;
	bool   hit      = false;
	double prevT    = 0.0;
	// March the whole ray to the tile boundary so edgeLla is ALWAYS the geometric tile-edge exit (the swept
	// beam is drawn full length regardless of occlusion); record the FIRST terrain crossing as the hit but
	// keep marching so a close ridge does not cut the visible ray short.
	// Start a little out from the antenna so the tower's own cell is not counted as an immediate hit.
	for (double t = 50.0; t <= 200000.0; t += 5.0)
	{
		const double above = heightAboveTerrain(t, lla, inBounds);
		if (!inBounds)
			break;    // left the tile; lastInBounds already holds the tile-edge exit
		lastInBounds = lla;
		if (!hit && above <= 0.0)
		{
			double lo = prevT, hi = t;
			for (int i = 0; i < 40; ++i)
			{
				const double mid = 0.5 * (lo + hi);
				bool         ib;
				(heightAboveTerrain(mid, lla, ib) > 0.0 ? lo : hi) = mid;
			}
			bool ib;
			heightAboveTerrain(0.5 * (lo + hi), hitLla, ib);
			hit = true;
		}
		prevT = t;
	}
	edgeLla = lastInBounds;    // always the far tile-edge point along the ray, so the caller can draw the full beam
	return hit;
}

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: latLonToPixel [static]
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Map a geodetic latitude/longitude to a (row, col) pixel in the hillshade raster (row 0 = north,
///				col 0 = west).
/// @param[in]	lat		latitude, degrees.
/// @param[in]	lon		longitude, degrees.
/// @param[in]	meta	tile metadata (bounds).
/// @param[in]	rows	raster row count.
/// @param[in]	cols	raster column count.
/// @param[out]	row		resulting pixel row.
/// @param[out]	col		resulting pixel column.
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
/// @brief		Stand a short tower on a hilltop, sweep a 0-deg-elevation beam through a full 360 deg turn,
///				march each beam to the terrain, and render the moving yellow ray plus the accumulating red
///				terrain-hit (RF horizon) footprint on the tile's colorized hillshade.
//----------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	const std::string tilePath = (argc > 1) ? argv[1] : "test/resources/w115_n37.dt2";
	const std::string outPath  = (argc > 2) ? argv[2] : "towerHorizonScan.ppm";
	const std::string frameDir = (argc > 3) ? argv[3] : "";

	DTEDTile tile(tilePath);
	tile.load();
	const TileMetadata meta = tile.metadata();

	const auto shade = coordinates::hillshade(&tile, units::angle::arcseconds<>(9.0));
	const int  rows  = static_cast<int>(shade.size());
	const int  cols  = rows > 0 ? static_cast<int>(shade[0].size()) : 0;

	// Colorized hillshade base (green valley -> brown ridge, modulated by shade).
	std::vector<uint8_t> base(static_cast<std::size_t>(rows) * cols * 3, 0);
	const double swLat = meta.southwestLatitude().value(), neLat = meta.northeastLatitude().value();
	const double swLon = meta.southwestLongitude().value(), neLon = meta.northeastLongitude().value();
	double zlo = 1e9, zhi = -1e9;
	for (int r = 0; r < rows; ++r)
		for (int c = 0; c < cols; ++c)
		{
			const double lat = neLat - (neLat - swLat) * r / (rows - 1);
			const double lon = swLon + (neLon - swLon) * c / (cols - 1);
			const double z   = tile.elevation(units::angle::degrees<>(lat), units::angle::degrees<>(lon)).value();
			zlo = std::min(zlo, z);
			zhi = std::max(zhi, z);
		}
	for (int r = 0; r < rows; ++r)
		for (int c = 0; c < cols; ++c)
		{
			const double lat = neLat - (neLat - swLat) * r / (rows - 1);
			const double lon = swLon + (neLon - swLon) * c / (cols - 1);
			const double z   = tile.elevation(units::angle::degrees<>(lat), units::angle::degrees<>(lon)).value();
			const double f   = (zhi > zlo) ? (z - zlo) / (zhi - zlo) : 0.0;
			const double sh  = shade[r][c] / 255.0;
			const std::size_t i = (static_cast<std::size_t>(r) * cols + c) * 3;
			base[i + 0] = static_cast<uint8_t>(std::lround(255.0 * std::min(1.0, (0.30 + 0.70 * f) * sh)));
			base[i + 1] = static_cast<uint8_t>(std::lround(255.0 * std::min(1.0, (0.45 + 0.35 * f) * sh)));
			base[i + 2] = static_cast<uint8_t>(std::lround(255.0 * std::min(1.0, (0.25 + 0.20 * f) * sh)));
		}

	std::vector<uint8_t> img = base;    // accumulates the red hit footprint + the antenna marker
	auto put = [&](int r, int c, uint8_t R, uint8_t G, uint8_t B) {
		if (r < 0 || r >= rows || c < 0 || c >= cols) return;
		const std::size_t i = (static_cast<std::size_t>(r) * cols + c) * 3;
		img[i + 0] = R; img[i + 1] = G; img[i + 2] = B;
	};
	auto disc = [&](std::vector<uint8_t>& buf, int r, int c, int radius, uint8_t R, uint8_t G, uint8_t B) {
		for (int dr = -radius; dr <= radius; ++dr)
			for (int dc = -radius; dc <= radius; ++dc)
				if (dr * dr + dc * dc <= radius * radius && r + dr >= 0 && r + dr < rows && c + dc >= 0 && c + dc < cols)
				{
					const std::size_t i = (static_cast<std::size_t>(r + dr) * cols + (c + dc)) * 3;
					buf[i + 0] = R; buf[i + 1] = G; buf[i + 2] = B;
				}
	};
	auto writePpm = [&](const std::string& path, const std::vector<uint8_t>& buf) {
		std::ofstream out(path, std::ios::binary);
		out << "P6\n" << cols << " " << rows << "\n255\n";
		out.write(reinterpret_cast<const char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
	};

	// --- The tower: a 100 ft antenna on a local high point that still sits below the surrounding ridges.
	//     tile.elevation() already returns a typed length (meters); adding a feet tower height just works --
	//     units converts feet<->meters implicitly and the sum is a length, so the antenna altitude is the
	//     ground elevation plus 100 ft with no manual conversion. ---
	const units::angle::degrees<> siteLat{37.65639}, siteLon{-114.32944};
	const units::length::feet<>   towerHeight{10.0};
	const auto                    floorElev = tile.elevation(siteLat, siteLon);    // meters<>, typed
	const Lla                     antenna(siteLat, siteLon, floorElev + towerHeight);
	const Ecef                    antennaEcef(antenna);
	const rotation::Quaternion    ned2ecef = nedToEcefRotation(siteLat, siteLon);

	int antR, antC;
	latLonToPixel(siteLat.value(), siteLon.value(), meta, rows, cols, antR, antC);

	std::printf("=== Tower RF horizon scan ===\n");
	std::printf("Tower: %.4f, %.4f  floor %.0f m + %.0f ft antenna\n", siteLat.value(), siteLon.value(), floorElev.value(), towerHeight.value());
	std::printf("Beam: 0 deg elevation (level), azimuth sweeping a full 360 deg turn, 0 roll.\n");

	const int steps = 360;    // one full turn -- the last frame's ray meets the first frame's, so the video loops seamlessly

	// Resolve every azimuth once: the ray endpoint (terrain hit if blocked, else the tile edge) and whether it
	// was blocked. Painting the WHOLE red footprint before any frame is drawn makes the footprint identical on
	// every frame, so the only thing that moves is the yellow ray -- the wrap from 359 deg back to 0 deg is a
	// single 1 deg step with no popping footprint, i.e. a seamless loop.
	struct Beam { int rayR, rayC, hitR, hitC; };    // hitR<0 => open (ran to the tile edge)
	std::vector<Beam> beams(static_cast<std::size_t>(steps));
	int               hits = 0, misses = 0;
	for (int i = 0; i < steps; ++i)
	{
		const units::angle::degrees<> azimuth{static_cast<double>(i)};    // from north, +east

		// Level beam in local NED: elevation 0 => no vertical (down) component; azimuth az from north.
		const double         az = units::angle::radians<>(azimuth).value();
		const CartesianTuple beamNed(units::length::meters<>(std::cos(az)),    // north
		                             units::length::meters<>(std::sin(az)),    // east
		                             0.0_m);                                   // down = 0 (0 deg elevation)
		const CartesianTuple beamEcef = ned2ecef.rotate(beamNed);             // NED -> ECEF direction

		Lla        hitLla, edgeLla;
		Beam       beam{-1, -1, -1, -1};
		const bool blocked = marchToTerrain(tile, antennaEcef, beamEcef, hitLla, edgeLla);
		// The yellow ray is the beam's line of sight: it TERMINATES at the terrain hit when a ridge occludes it
		// (drawn to the red dot, never past it), and runs to the tile edge only when the beam clears the terrain.
		if (blocked)
		{
			latLonToPixel(hitLla.latitude().to<units::angle::degrees<>>().value(),
			              hitLla.longitude().to<units::angle::degrees<>>().value(), meta, rows, cols, beam.hitR, beam.hitC);
			put(beam.hitR, beam.hitC, 255, 40, 40);    // paint the red terrain hit into the shared footprint
			beam.rayR = beam.hitR; beam.rayC = beam.hitC;
			++hits;
		}
		else
		{
			latLonToPixel(edgeLla.latitude().to<units::angle::degrees<>>().value(),
			              edgeLla.longitude().to<units::angle::degrees<>>().value(), meta, rows, cols, beam.rayR, beam.rayC);
			++misses;
		}
		beams[static_cast<std::size_t>(i)] = beam;
	}

	// The base for every frame: the finished footprint + the antenna marker. Frames differ ONLY by the moving ray.
	std::vector<uint8_t> footprint = img;
	disc(footprint, antR, antC, 3, 255, 255, 255);
	disc(footprint, antR, antC, 1, 0, 0, 0);

	if (!frameDir.empty())
		for (int i = 0; i < steps; ++i)
		{
			const Beam&          beam  = beams[static_cast<std::size_t>(i)];
			std::vector<uint8_t> frame = footprint;
			// The swept beam: Bresenham from the antenna to the ray endpoint (terrain hit, or tile edge), yellow.
			int r0 = antR, c0 = antC, r1 = beam.rayR, c1 = beam.rayC;
			int dr = std::abs(r1 - r0), dc = std::abs(c1 - c0), sr = r0 < r1 ? 1 : -1, sc = c0 < c1 ? 1 : -1, err = dc - dr;
			while (true)
			{
				const std::size_t k = (static_cast<std::size_t>(r0) * cols + c0) * 3;
				if (r0 >= 0 && r0 < rows && c0 >= 0 && c0 < cols) { frame[k + 0] = 255; frame[k + 1] = 240; frame[k + 2] = 0; }
				if (r0 == r1 && c0 == c1) break;
				int e2 = 2 * err;
				if (e2 > -dr) { err -= dr; c0 += sc; }
				if (e2 < dc)  { err += dc; r0 += sr; }
			}
			if (beam.hitR >= 0)
				disc(frame, beam.hitR, beam.hitC, 1, 255, 40, 40);    // terrain-hit terminus, red, over the ray
			disc(frame, antR, antC, 3, 255, 255, 255);    // antenna, white (redrawn over any ray pixels)
			disc(frame, antR, antC, 1, 0, 0, 0);
			char name[512];
			std::snprintf(name, sizeof(name), "%s/frame_%04d.ppm", frameDir.c_str(), i);
			writePpm(name, frame);
		}

	// Final still: the full red horizon footprint + the antenna.
	disc(img, antR, antC, 3, 255, 255, 255);
	disc(img, antR, antC, 1, 0, 0, 0);
	writePpm(outPath, img);

	std::printf("Beam hits (blocked by terrain): %d   open (ran off-tile): %d\n", hits, misses);
	std::printf("White = antenna, yellow = swept beam, red = accumulated terrain line-of-sight footprint.\n");
	std::printf("Still: %s%s\n", outPath.c_str(), frameDir.empty() ? "" : "   (frames in the given dir)");
	return 0;
}
