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
// A mutable RGB raster with the basic draw primitives (`plot`, `disc`, `line`). Row-major, three bytes per
// pixel; every primitive clips to the bounds. Cheap to copy, so a caller clones a base image once per frame and
// overlays transient content.
//
//--------------------------------------------------------------------------------------------------

#ifndef image_h
#define image_h

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <vector>

#include "color.h"
#include "pixel.h"

inline namespace coordinates
{
	inline namespace topography
	{
		//	----------------------------------------------------------------------------
		//	CLASS		Image
		//  ----------------------------------------------------------------------------
		///	@brief		A mutable RGB raster with clipped `plot`/`disc`/`line` primitives.
		///	@details	Row-major, three bytes per pixel. Draw primitives clip to the bounds. Cheap to copy, so a
		///				caller clones the base once per frame and overlays transient content.
		//  ----------------------------------------------------------------------------
		class Image
		{
		public:
			Image() = default;

			/// A `rows` x `columns` image filled with `background` (default white -- a blank canvas).
			Image(int rows, int columns, Color background = {255, 255, 255}) : m_rows(rows), m_columns(columns), m_rgb(static_cast<std::size_t>(rows) * columns * 3)
			{
				fill(background);
			}

			/// An image sized to a camera's view (anything with `.height()`/`.width()`), filled with `background`.
			template<class Camera>
			    requires requires(const Camera& c) { c.height(); c.width(); }
			explicit Image(const Camera& camera, Color background = {255, 255, 255}) : Image(camera.height(), camera.width(), background)
			{
			}

			[[nodiscard]] int                             rows() const { return m_rows; }
			[[nodiscard]] int                             columns() const { return m_columns; }
			[[nodiscard]] const std::vector<std::uint8_t>& rgb() const { return m_rgb; }

			/// Fill the whole image with a single color.
			void fill(Color c)
			{
				for (std::size_t i = 0; i < m_rgb.size(); i += 3)
				{
					m_rgb[i + 0] = c.r;
					m_rgb[i + 1] = c.g;
					m_rgb[i + 2] = c.b;
				}
			}

			/// Set a single pixel (clipped).
			void plot(Pixel p, Color c)
			{
				if (p.row < 0 || p.row >= m_rows || p.column < 0 || p.column >= m_columns)
					return;
				const std::size_t i = (static_cast<std::size_t>(p.row) * m_columns + p.column) * 3;
				m_rgb[i + 0]        = c.r;
				m_rgb[i + 1]        = c.g;
				m_rgb[i + 2]        = c.b;
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
	}    // namespace topography
}    // namespace coordinates

#endif    // image_h
