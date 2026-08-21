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

#ifndef hillshadeCanvasTest_h
#define hillshadeCanvasTest_h

//------------------------
//	INCLUDES
//------------------------

#include <gtest/gtest.h>

#include "hillshadeCanvas.h"

inline namespace coordinates
{
	inline namespace topography
	{
		using namespace units::literals;

		// The Image primitives clip and draw independent of any tile, so they are tested on their own.

		// plot sets exactly the addressed pixel; out-of-bounds plots are ignored.
		TEST(HillshadeCanvasImageTest, plotSetsPixelAndClips)
		{
			Image img(4, 4);
			img.plot(Pixel{1, 2}, Color{10, 20, 30});
			const auto& rgb = img.rgb();
			const std::size_t i = (static_cast<std::size_t>(1) * 4 + 2) * 3;
			EXPECT_EQ(rgb[i + 0], 10);
			EXPECT_EQ(rgb[i + 1], 20);
			EXPECT_EQ(rgb[i + 2], 30);
			img.plot(Pixel{-1, 0}, Color{255, 255, 255});    // clipped, no throw, no change elsewhere
			img.plot(Pixel{4, 4}, Color{255, 255, 255});
			EXPECT_EQ(rgb[0], 0);
		}

		// A line's two endpoints are both painted.
		TEST(HillshadeCanvasImageTest, lineHitsBothEndpoints)
		{
			Image img(8, 8);
			img.line(Pixel{0, 0}, Pixel{7, 7}, Color{1, 2, 3});
			const auto& rgb = img.rgb();
			EXPECT_EQ(rgb[0], 1);                                                   // (0,0)
			const std::size_t last = (static_cast<std::size_t>(7) * 8 + 7) * 3;
			EXPECT_EQ(rgb[last], 1);                                                // (7,7)
		}

		// A disc paints its center.
		TEST(HillshadeCanvasImageTest, discPaintsCenter)
		{
			Image img(8, 8);
			img.disc(Pixel{4, 4}, 2, Color{9, 9, 9});
			const std::size_t i = (static_cast<std::size_t>(4) * 8 + 4) * 3;
			EXPECT_EQ(img.rgb()[i], 9);
		}

#if defined(COORDINATES_ENABLE_DTED) && COORDINATES_ENABLE_DTED
		// Rendering the shared DTED tile yields a square canvas and a valid, in-bounds projection.
		TEST(HillshadeCanvasTest, rendersTileAndProjects)
		{
			DTEDTile tile("resources/w115_n37.dt2");
			ASSERT_TRUE(tile.load());

			HillshadeCanvas canvas(&tile, 120.0_arcsec);    // coarse for a fast test
			EXPECT_GT(canvas.rows(), 0);
			EXPECT_EQ(canvas.rows(), canvas.columns());

			// The tile spans lat 37..38, lon -115..-114; a mid point projects into the interior.
			const auto px = canvas.project(37.5_deg, -114.5_deg);
			EXPECT_GE(px.row, 0);
			EXPECT_LT(px.row, canvas.rows());
			EXPECT_GE(px.column, 0);
			EXPECT_LT(px.column, canvas.columns());

			// The NW corner projects to (row 0, col 0); the SE corner to the far pixel.
			const auto nw = canvas.project(38.0_deg, -115.0_deg);
			EXPECT_EQ(nw.row, 0);
			EXPECT_EQ(nw.column, 0);
			const auto se = canvas.project(37.0_deg, -114.0_deg);
			EXPECT_EQ(se.row, canvas.rows() - 1);
			EXPECT_EQ(se.column, canvas.columns() - 1);

			// The base image is non-empty (some terrain color was written).
			const auto& rgb = canvas.base().rgb();
			EXPECT_EQ(rgb.size(), static_cast<std::size_t>(canvas.rows()) * canvas.columns() * 3);
			bool anyColor = false;
			for (auto v : rgb)
				if (v != 0) { anyColor = true; break; }
			EXPECT_TRUE(anyColor);
		}
#endif    // COORDINATES_ENABLE_DTED
	}    // namespace topography
}    // namespace coordinates

#endif    // hillshadeCanvasTest_h
