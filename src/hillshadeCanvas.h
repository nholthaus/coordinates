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
// A drawable colorized-hillshade canvas over a DTED tile: the simple equirectangular map projection plus the
// draw primitives a terrain visualization needs. `HillshadeCanvas` renders a tile to a green-valley-to-brown-
// ridge base image modulated by `algorithm::topography::hillshade`, projects a geodetic latitude/longitude to a
// raster pixel (`project`), and offers point/disc/line drawing and a P6 PPM writer on an RGB `Image`. It carries
// no domain logic beyond the tile and the projection; a caller renders a footprint into the base and clones it
// per frame for transient overlays (a moving ray), which is exactly what the terrain demos need. The projection
// is the library's simple map projection (equirectangular over the tile bounds); it is the seam a richer
// projection would later slot behind.
//
//--------------------------------------------------------------------------------------------------

#ifndef hillshadeCanvas_h
#define hillshadeCanvas_h

//------------------------
//	INCLUDES
//------------------------

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <units.h>

#include "abstractTile.h"
#include "algorithm.h"
#include "tileMetadata.h"

inline namespace coordinates
{
	namespace topography
	{
		//	----------------------------------------------------------------------------
		//	STRUCT		Color
		//  ----------------------------------------------------------------------------
		///	@brief		An 8-bit-per-channel RGB color for canvas drawing.
		//  ----------------------------------------------------------------------------
		struct Color
		{
			std::uint8_t r{0}, g{0}, b{0};
		};

		//	----------------------------------------------------------------------------
		//	STRUCT		Pixel
		//  ----------------------------------------------------------------------------
		///	@brief		A raster pixel address (row 0 = north edge, column 0 = west edge).
		//  ----------------------------------------------------------------------------
		struct Pixel
		{
			int row{0}, column{0};
		};

		//	----------------------------------------------------------------------------
		//	CLASS		Image
		//  ----------------------------------------------------------------------------
		///	@brief		A mutable RGB raster: the drawable surface a `HillshadeCanvas` produces and writes.
		///	@details	Row-major, three bytes per pixel. Draw primitives clip to the bounds. Cheap to copy, so a
		///				caller clones the base once per frame and overlays transient content.
		//  ----------------------------------------------------------------------------
		class Image
		{
		public:
			Image() = default;
			Image(int rows, int columns) : m_rows(rows), m_columns(columns), m_rgb(static_cast<std::size_t>(rows) * columns * 3, 0) {}

			[[nodiscard]] int rows() const { return m_rows; }
			[[nodiscard]] int columns() const { return m_columns; }
			[[nodiscard]] const std::vector<std::uint8_t>& rgb() const { return m_rgb; }

			/// Set a single pixel (clipped).
			void plot(Pixel p, Color c)
			{
				if (p.row < 0 || p.row >= m_rows || p.column < 0 || p.column >= m_columns)
					return;
				const std::size_t i = (static_cast<std::size_t>(p.row) * m_columns + p.column) * 3;
				m_rgb[i + 0] = c.r; m_rgb[i + 1] = c.g; m_rgb[i + 2] = c.b;
			}

			/// Fill a filled disc of the given pixel radius centered at `p` (clipped).
			void disc(Pixel p, int radius, Color c)
			{
				for (int dr = -radius; dr <= radius; ++dr)
					for (int dc = -radius; dc <= radius; ++dc)
						if (dr * dr + dc * dc <= radius * radius)
							plot(Pixel{p.row + dr, p.column + dc}, c);
			}

			/// Draw a straight line between two pixels (Bresenham, clipped per pixel).
			void line(Pixel a, Pixel b, Color c)
			{
				int r0 = a.row, c0 = a.column, r1 = b.row, c1 = b.column;
				int dr = std::abs(r1 - r0), dc = std::abs(c1 - c0);
				int sr = r0 < r1 ? 1 : -1, sc = c0 < c1 ? 1 : -1, err = dc - dr;
				while (true)
				{
					plot(Pixel{r0, c0}, c);
					if (r0 == r1 && c0 == c1)
						break;
					const int e2 = 2 * err;
					if (e2 > -dr) { err -= dr; c0 += sc; }
					if (e2 < dc)  { err += dc; r0 += sr; }
				}
			}

		private:
			int                       m_rows{0};
			int                       m_columns{0};
			std::vector<std::uint8_t> m_rgb;
		};

		//	----------------------------------------------------------------------------
		//	CLASS		HillshadeCanvas
		//  ----------------------------------------------------------------------------
		///	@brief		A colorized-hillshade base image over a DTED tile plus the simple map projection and a
		///				PPM writer -- the drawable canvas terrain visualizations render onto.
		///	@details	Builds the base once at construction (elevation ramp green->brown, modulated by hillshade).
		///				`project(lat, lon)` maps geodetic degrees to a raster pixel (equirectangular over the tile
		///				bounds). `base()` is the rendered background; `blank`/draw on an `Image` and `writePpm`
		///				emit frames. The intended pattern: draw a persistent footprint into a copy of `base()`,
		///				then clone that per frame for a moving overlay.
		//  ----------------------------------------------------------------------------
		class HillshadeCanvas
		{
		public:
			//----------------------------------
			//	CONSTRUCTORS
			//----------------------------------

			/// Render `tile` to a colorized hillshade at the given angular resolution (0 = the tile's native).
			explicit HillshadeCanvas(const AbstractTile* tile, units::angle::degrees<> resolution = units::angle::degrees<>(0.0))
			    : m_meta(tile ? tile->metadata() : TileMetadata{})
			{
				if (tile == nullptr)
					throw std::invalid_argument("HillshadeCanvas: tile is null");

				const auto shade = coordinates::hillshade(tile, resolution);
				m_rows           = static_cast<int>(shade.size());
				m_columns        = m_rows > 0 ? static_cast<int>(shade[0].size()) : 0;

				m_swLat = m_meta.southwestLatitude().value();
				m_neLat = m_meta.northeastLatitude().value();
				m_swLon = m_meta.southwestLongitude().value();
				m_neLon = m_meta.northeastLongitude().value();

				// Elevation range for the ramp.
				double zlo = 1e30, zhi = -1e30;
				for (int r = 0; r < m_rows; ++r)
					for (int c = 0; c < m_columns; ++c)
					{
						const double z = elevationAt_(tile, r, c);
						zlo            = std::min(zlo, z);
						zhi            = std::max(zhi, z);
					}

				m_base = Image(m_rows, m_columns);
				for (int r = 0; r < m_rows; ++r)
					for (int c = 0; c < m_columns; ++c)
					{
						const double z  = elevationAt_(tile, r, c);
						const double f  = (zhi > zlo) ? (z - zlo) / (zhi - zlo) : 0.0;
						const double sh = shade[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)] / 255.0;
						const Color  col{
						        static_cast<std::uint8_t>(std::lround(255.0 * std::min(1.0, (0.30 + 0.70 * f) * sh))),
						        static_cast<std::uint8_t>(std::lround(255.0 * std::min(1.0, (0.45 + 0.35 * f) * sh))),
						        static_cast<std::uint8_t>(std::lround(255.0 * std::min(1.0, (0.25 + 0.20 * f) * sh)))};
						m_base.plot(Pixel{r, c}, col);
					}
			}

			//----------------------------------
			//	GETTERS
			//----------------------------------

			[[nodiscard]] int          rows() const { return m_rows; }
			[[nodiscard]] int          columns() const { return m_columns; }
			[[nodiscard]] const Image& base() const { return m_base; }    ///< the rendered hillshade background
			[[nodiscard]] Image        blank() const { return m_base; }   ///< a fresh copy of the base to draw a frame on

			//----------------------------------
			//	PROJECTION
			//----------------------------------

			/// Map a geodetic latitude/longitude (degrees) to a raster pixel. Row 0 is the north edge, column 0
			/// the west edge; the projection is equirectangular over the tile bounds (the simple map projection).
			[[nodiscard]] Pixel project(units::angle::degrees<> latitude, units::angle::degrees<> longitude) const
			{
				const double lat = latitude.value(), lon = longitude.value();
				const int    row = static_cast<int>(std::lround((m_neLat - lat) / (m_neLat - m_swLat) * (m_rows - 1)));
				const int    col = static_cast<int>(std::lround((lon - m_swLon) / (m_neLon - m_swLon) * (m_columns - 1)));
				return Pixel{row, col};
			}

			//----------------------------------
			//	OUTPUT
			//----------------------------------

			/// Write an image as a binary P6 PPM.
			void writePpm(const Image& image, const std::string& path) const
			{
				std::ofstream out(path, std::ios::binary);
				if (!out)
					throw std::runtime_error("HillshadeCanvas::writePpm: cannot open " + path);
				out << "P6\n" << image.columns() << " " << image.rows() << "\n255\n";
				out.write(reinterpret_cast<const char*>(image.rgb().data()), static_cast<std::streamsize>(image.rgb().size()));
			}

		private:
			//	----------------------------------------------------------------------------
			//	FUNCTION: elevationAt_ [private]
			//  ----------------------------------------------------------------------------
			///	@brief		The tile elevation (meters) at a raster (row, column), via the equirectangular mapping.
			///	@param[in]	tile	the tile to sample.
			///	@param[in]	row		the raster row (0 = north edge).
			///	@param[in]	column	the raster column (0 = west edge).
			///	@return		the ground elevation in meters at that pixel's lat/lon.
			//  ----------------------------------------------------------------------------
			[[nodiscard]] double elevationAt_(const AbstractTile* tile, int row, int column) const
			{
				const double lat = m_neLat - (m_neLat - m_swLat) * row / (m_rows - 1);
				const double lon = m_swLon + (m_neLon - m_swLon) * column / (m_columns - 1);
				return tile->elevation(units::angle::degrees<>(lat), units::angle::degrees<>(lon)).value();
			}

			TileMetadata m_meta;
			Image        m_base;
			int          m_rows{0};
			int          m_columns{0};
			double       m_swLat{0.0}, m_neLat{0.0}, m_swLon{0.0}, m_neLon{0.0};
		};
	}    // namespace topography
}    // namespace coordinates

#endif    // hillshadeCanvas_h
