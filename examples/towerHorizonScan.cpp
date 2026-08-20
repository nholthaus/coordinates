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
// a full 360 deg azimuth turn. Each azimuth the site emits a level ray (`site.ray(az, 0)`), the ray is marched
// to the terrain (`terrainIntersection(ray)`), and the first intersection is a radio line-of-sight / horizon-
// mask point. The yellow ray TERMINATES at the terrain hit when a ridge occludes it, and runs to the tile edge
// when the beam clears the terrain; the red hits ACCUMULATE into the RF horizon footprint. The scan is a full
// 360 deg over a footprint painted before any frame is drawn, so the video loops seamlessly.
//
// Every geometric and rendering step is a library call: `PositionGeodetic::ray` (a look-angle ray from the
// site), the free `terrainIntersection` (the geodesic terrain march), and `topography::HillshadeCanvas` (the
// colorized hillshade, the lat/lon->pixel projection, and the ray/dot drawing).
//
//--------------------------------------------------------------------------------------------------

#include <cstdio>
#include <string>
#include <vector>

#include "coordinates.h"
#include "dtedTile.h"

using namespace coordinates;
using namespace coordinates::topography;
using namespace coordinates::datums;
using namespace units::literals;

using Wgs = WGS84_G1674;
using Lla = PositionGeodetic<Wgs>;

//----------------------------------------------------------------------------------------------------------------------
//	FUNCTION: main
//----------------------------------------------------------------------------------------------------------------------
/// @brief		Stand a short tower on a hilltop, sweep a 0-deg-elevation beam through a full 360 deg turn, march
///				each beam to the terrain, and render the moving yellow ray plus the accumulating red terrain-hit
///				(RF horizon) footprint on the tile's colorized hillshade.
//----------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	const std::string tilePath = (argc > 1) ? argv[1] : "test/resources/w115_n37.dt2";
	const std::string outPath  = (argc > 2) ? argv[2] : "towerHorizonScan.ppm";
	const std::string frameDir = (argc > 3) ? argv[3] : "";

	DTEDTile tile(tilePath);
	tile.load();

	HillshadeCanvas canvas(&tile, 9.0_arcsec);

	// The tower: a 10 ft mast on a hilltop. tile.elevation() is a typed length (meters); adding a feet mast
	// just works -- units converts feet<->meters and the sum is the antenna's MSL altitude, no manual math.
	const units::angle::degrees<> siteLat{37.75500}, siteLon{-114.90500};
	const units::length::feet<>   towerHeight{10.0};
	const auto                    floorElev = tile.elevation(siteLat, siteLon);
	const Lla                     antenna(siteLat, siteLon, floorElev + towerHeight);
	const Pixel                   antennaPixel = canvas.project(siteLat, siteLon);

	std::printf("=== Tower RF horizon scan ===\n");
	std::printf("Tower: %.4f, %.4f  floor %.0f m + %.0f ft antenna\n", siteLat.value(), siteLon.value(), floorElev.value(), towerHeight.value());
	std::printf("Beam: 0 deg elevation (level), azimuth sweeping a full 360 deg turn, 0 roll.\n");

	const int steps = 360;    // one full turn -- the last frame's ray meets the first frame's, so the video loops seamlessly

	// Resolve every azimuth once: the ray endpoint pixel (the terrain hit if blocked, else a far point the canvas
	// clips to the tile edge) and, when blocked, the red hit pixel. Painting the WHOLE footprint before any frame
	// makes the footprint identical on every frame, so only the yellow ray moves -- a seamless loop.
	struct Beam { Pixel ray; Pixel hit; bool blocked; };
	std::vector<Beam> beams(static_cast<std::size_t>(steps));
	int               hits = 0, misses = 0;
	for (int i = 0; i < steps; ++i)
	{
		const auto beam = ray(antenna, units::angle::degrees<>(static_cast<double>(i)), 0.0_deg);    // level, azimuth i
		const auto hit  = terrainIntersection(beam);

		Beam b{antennaPixel, antennaPixel, hit.has_value()};
		if (b.blocked)
		{
			b.hit = canvas.project(hit->hit);    // the terrain-hit point projects directly
			b.ray = b.hit;                       // the ray terminates at the terrain hit
			++hits;
		}
		else
		{
			// Open horizon: draw the beam far along its direction; the canvas clips the line to the tile edge.
			b.ray = canvas.project(Lla(beam.pointAt(300000.0_m)));
			++misses;
		}
		beams[static_cast<std::size_t>(i)] = b;
	}

	// The base every frame draws over: the finished red footprint + the white antenna marker.
	Image footprint = canvas.blank();
	for (const Beam& b : beams)
		if (b.blocked)
			footprint.plot(b.hit, Color{255, 40, 40});
	footprint.disc(antennaPixel, 3, Color{255, 255, 255});
	footprint.disc(antennaPixel, 1, Color{0, 0, 0});

	if (!frameDir.empty())
		for (int i = 0; i < steps; ++i)
		{
			const Beam& b     = beams[static_cast<std::size_t>(i)];
			Image       frame = footprint;
			frame.line(antennaPixel, b.ray, Color{255, 240, 0});    // the swept beam, yellow
			if (b.blocked)
				frame.disc(b.hit, 1, Color{255, 40, 40});           // terrain-hit terminus, red, over the ray
			frame.disc(antennaPixel, 3, Color{255, 255, 255});      // antenna, white (over any ray pixels)
			frame.disc(antennaPixel, 1, Color{0, 0, 0});
			char name[512];
			std::snprintf(name, sizeof(name), "%s/frame_%04d.ppm", frameDir.c_str(), i);
			canvas.writePpm(frame, name);
		}

	canvas.writePpm(footprint, outPath);

	std::printf("Beam hits (blocked by terrain): %d   open (ran off-tile): %d\n", hits, misses);
	std::printf("White = antenna, yellow = swept beam, red = accumulated terrain line-of-sight footprint.\n");
	std::printf("Still: %s%s\n", outPath.c_str(), frameDir.empty() ? "" : "   (frames in the given dir)");
	return 0;
}
