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
#include "color.h"
#include "image.h"
#include "pixel.h"
#include "tileMetadata.h"

inline namespace coordinates
{
	inline namespace topography
	{
		using namespace units;
		using namespace units::literals;

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
			explicit HillshadeCanvas(const AbstractTile* tile, degrees<> resolution = 0.0_deg)
			    : m_meta(tile ? tile->metadata() : TileMetadata{})
			{
				if (tile == nullptr)
					throw std::invalid_argument("HillshadeCanvas: tile is null");

				const auto shade = coordinates::hillshade(tile, resolution);
				m_rows           = static_cast<int>(shade.size());
				m_columns        = m_rows > 0 ? static_cast<int>(shade[0].size()) : 0;

				m_southwestLatitude  = m_meta.southwestLatitude();
				m_northeastLatitude  = m_meta.northeastLatitude();
				m_southwestLongitude = m_meta.southwestLongitude();
				m_northeastLongitude = m_meta.northeastLongitude();

				// Elevation range for the color ramp, seeded from the first sample.
				meters<> low = elevationAt(tile, 0, 0), high = low;
				for (int r = 0; r < m_rows; ++r)
					for (int c = 0; c < m_columns; ++c)
					{
						const meters<> z = elevationAt(tile, r, c);
						low              = units::min(low, z);
						high             = units::max(high, z);
					}

				// Green valley to brown ridge, modulated by the hillshade. `f` is the normalized elevation.
				m_base = Image(m_rows, m_columns);
				for (int r = 0; r < m_rows; ++r)
					for (int c = 0; c < m_columns; ++c)
					{
						const meters<> z  = elevationAt(tile, r, c);
						const double   f  = (high > low) ? ((z - low) / (high - low)).value() : 0.0;
						const double   sh = shade[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)] / 255.0;
						m_base.plot(Pixel{r, c}, Color{channel((0.30 + 0.70 * f) * sh), channel((0.45 + 0.35 * f) * sh), channel((0.25 + 0.20 * f) * sh)});
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

			/// Map a geodetic latitude/longitude to a raster pixel. Row 0 is the north edge, column 0 the west
			/// edge; the projection is equirectangular over the tile bounds (the simple map projection).
			[[nodiscard]] Pixel project(degrees<> latitude, degrees<> longitude) const
			{
				const dimensionless<> rowFraction = (m_northeastLatitude - latitude) / (m_northeastLatitude - m_southwestLatitude);
				const dimensionless<> colFraction = (longitude - m_southwestLongitude) / (m_northeastLongitude - m_southwestLongitude);
				const int             row = static_cast<int>(std::lround(rowFraction.value() * (m_rows - 1)));
				const int             col = static_cast<int>(std::lround(colFraction.value() * (m_columns - 1)));
				return Pixel{row, col};
			}

			/// Map a geodetic POINT to a raster pixel -- a position projects directly, no accessor unwrap needed.
			template<class GeodeticPoint>
			    requires requires(const GeodeticPoint& p) { p.latitude(); p.longitude(); }
			[[nodiscard]] Pixel project(const GeodeticPoint& point) const
			{
				return project(point.latitude().template to<degrees<>>(), point.longitude().template to<degrees<>>());
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
			//	FUNCTION: elevationAt [private]
			//  ----------------------------------------------------------------------------
			///	@brief		The tile elevation (meters) at a raster (row, column), via the equirectangular mapping.
			///	@param[in]	tile	the tile to sample.
			///	@param[in]	row		the raster row (0 = north edge).
			///	@param[in]	column	the raster column (0 = west edge).
			///	@return		the ground elevation in meters at that pixel's lat/lon.
			//  ----------------------------------------------------------------------------
			[[nodiscard]] units::length::meters<> elevationAt(const AbstractTile* tile, int row, int column) const
			{
				const units::dimensionless<> rowFraction = units::dimensionless<>(static_cast<double>(row) / (m_rows - 1));
				const units::dimensionless<> colFraction = units::dimensionless<>(static_cast<double>(column) / (m_columns - 1));
				const units::angle::degrees<> lat = m_northeastLatitude - (m_northeastLatitude - m_southwestLatitude) * rowFraction;
				const units::angle::degrees<> lon = m_southwestLongitude + (m_northeastLongitude - m_southwestLongitude) * colFraction;
				return tile->elevation(lat, lon);
			}

			TileMetadata            m_meta;
			Image                   m_base;
			int                     m_rows{0};
			int                     m_columns{0};
			units::angle::degrees<> m_southwestLatitude{0.0};
			units::angle::degrees<> m_northeastLatitude{0.0};
			units::angle::degrees<> m_southwestLongitude{0.0};
			units::angle::degrees<> m_northeastLongitude{0.0};
		};
	}    // namespace topography
}    // namespace coordinates

#endif    // hillshadeCanvas_h
