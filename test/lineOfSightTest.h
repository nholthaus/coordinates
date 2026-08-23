#pragma once

//--------------------------------------------------------------------------------------------------
//
//  LineOfSight tests
//
//  Goals:
//  - Provide a deterministic, file-free terrain source (synthetic tile).
//  - Verify terrain LOS uses ray/terrain intersection (no range-bin stepping).
//  - Generate a viewshed overlay PPM and assert tinting behavior.
//
//--------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include "abstractTile.h"
#include "datum.h"
#include "lineOfSight.h"
#include "tileMetadata.h"
#include "topography.h"

#include <regex>

inline namespace coordinates
{
	namespace test
	{
		/**
		 * @brief Simple in-memory synthetic tile.
		 *
		 * Tile layout:
		 * - Lat:  [latSW, latNE]
		 * - Lon:  [lonSW, lonNE]
		 * - A tall "ridge" wall at a fixed longitude creates blocked regions.
		 */
		class SyntheticTile final : public AbstractTile
		{
		public:
			SyntheticTile(const degrees<> latSW,
			              const degrees<> lonSW,
			              const degrees<> latNE,
			              const degrees<> lonNE,
			              const degrees<> resolution,
			              const degrees<> ridgeLon,
			              const meters<>  ridgeHeight)
			    : m_loaded(true)
			    , m_latSW(latSW)
			    , m_lonSW(lonSW)
			    , m_latNE(latNE)
			    , m_lonNE(lonNE)
			    , m_res(resolution)
			    , m_ridgeLon(ridgeLon)
			    , m_ridgeHeight(ridgeHeight)
			{
				// Build metadata
				m_meta.setSouthwestLatitude(latSW);
				m_meta.setSouthwestLongitude(lonSW);
				m_meta.setNortheastLatitude(latNE);
				m_meta.setNortheastLongitude(lonNE);
				m_meta.setLatitudeResolution(resolution);
				m_meta.setLongitudeResolution(resolution);

				const int width  = ((latNE - latSW) / resolution).to<int>() + 1;
				const int height = ((lonNE - lonSW) / resolution).to<int>() + 1;
				m_meta.setNumLatitudeLines(static_cast<size_t>(width));
				m_meta.setNumLongitudeLines(static_cast<size_t>(height));

				// Conservative ranges
				m_meta.setMinElevation(0.0_m);
				m_meta.setMaxElevation(ridgeHeight);
				m_meta.setMeanElevation((ridgeHeight / 10));
			}

			[[nodiscard]] TileMetadata metadata() const override { return m_meta; }
			[[nodiscard]] bool         isLoaded() const override { return m_loaded; }
			[[nodiscard]] bool         load() const override
			{
				m_loaded = true;
				return true;
			}
			[[nodiscard]] bool clear() const override
			{
				m_loaded = false;
				return true;
			}

			[[nodiscard]] meters<> elevation(const degrees<>& latitude, const degrees<>& longitude) const override
			{
				if (latitude < m_latSW || latitude > m_latNE || longitude < m_lonSW || longitude > m_lonNE)
					throw std::runtime_error("SyntheticTile::elevation: out of bounds");

				// A vertical ridge wall centered at m_ridgeLon. Make it ~1 cell thick.
				const auto half = m_res * 0.5;
				if (std::abs((longitude - m_ridgeLon).to<double>()) <= half.to<double>())
					return m_ridgeHeight;

				return 0.0_m;
			}

			[[nodiscard]] meters<> elevation(size_t latitudeIndex, size_t longitudeIndex) const override
			{
				const int width  = ((m_latNE - m_latSW) / m_res).to<int>() + 1;
				const int height = ((m_lonNE - m_lonSW) / m_res).to<int>() + 1;
				if (latitudeIndex >= static_cast<size_t>(width) || longitudeIndex >= static_cast<size_t>(height))
					throw std::runtime_error("SyntheticTile::elevation(i,j): out of bounds");

				const degrees<> lat = m_latNE - m_res * static_cast<int>(latitudeIndex);
				const degrees<> lon = m_lonSW + m_res * static_cast<int>(longitudeIndex);
				return elevation(lat, lon);
			}

			[[nodiscard]] degrees<> width() const override { return (m_lonNE - m_lonSW); }
			[[nodiscard]] degrees<> height() const override { return (m_latNE - m_latSW); }

		private:
			mutable bool m_loaded;
			degrees<>    m_latSW;
			degrees<>    m_lonSW;
			degrees<>    m_latNE;
			degrees<>    m_lonNE;
			degrees<>    m_res;
			degrees<>    m_ridgeLon;
			meters<>     m_ridgeHeight;
			TileMetadata m_meta;
		};

		/**
		 * @brief Minimal topography model used by LineOfSight for tests.
		 */
		struct SyntheticTopography : public Topography<geoids::NULL_GEOID>
		{
			static const AbstractTile* s_tile;

			static meters<> orthometricHeight(const degrees<> lat, const degrees<> lon)
			{
				if (s_tile == nullptr)
					return 0.0_m;
				return s_tile->elevation(lat, lon);
			}
		};

		inline const AbstractTile* SyntheticTopography::s_tile = nullptr;

		/**
		 * @brief Read a binary PPM (P6) into an RGB buffer.
		 */
		static void readPpmP6(const std::filesystem::path& path, std::size_t& width, std::size_t& height, std::vector<std::uint8_t>& rgb)
		{
			std::ifstream in(path, std::ios::binary);
			ASSERT_TRUE(static_cast<bool>(in)) << "Failed to open PPM: " << path.string();

			std::string magic;
			in >> magic;
			ASSERT_EQ(magic, "P6");

			int w    = 0;
			int h    = 0;
			int maxv = 0;
			in >> w >> h >> maxv;
			ASSERT_GT(w, 0);
			ASSERT_GT(h, 0);
			ASSERT_EQ(maxv, 255);

			// Consume single whitespace after header.
			in.get();

			width  = static_cast<std::size_t>(w);
			height = static_cast<std::size_t>(h);
			rgb.resize(width * height * 3);
			in.read(reinterpret_cast<char*>(rgb.data()), static_cast<std::streamsize>(rgb.size()));
			ASSERT_TRUE(in.good() || in.eof());
		}

		static std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>
		pixelAt(const std::vector<std::uint8_t>& rgb, std::size_t width, std::size_t row, std::size_t col)
		{
			const std::size_t idx = (row * width + col) * 3;
			return {rgb[idx + 0], rgb[idx + 1], rgb[idx + 2]};
		}

	}    // namespace test

	TEST(LineOfSightTerrain, terrainLOS_and_viewshedOverlayPPM)
	{
		using Datum = datums::WGS84_G1674;
		using LOS   = LineOfSight<Datum, test::SyntheticTopography, double>;

		// Synthetic tile: 0.10 x 0.10 degrees at 0.01 degree resolution (11x11 samples).
		const degrees<> latSW = 0.00_deg;
		const degrees<> lonSW = 0.00_deg;
		const degrees<> latNE = 0.10_deg;
		const degrees<> lonNE = 0.10_deg;
		const degrees<> res   = 0.01_deg;

		// Ridge wall at lon=0.05, height 200m.
		test::SyntheticTile tile(latSW, lonSW, latNE, lonNE, res, 0.05_deg, 200.0_m);
		test::SyntheticTopography::s_tile = &tile;

		// Observer west of ridge, elevated 50m MSL.
		const PositionGeodetic<Datum> observer(0.05_deg, 0.02_deg, 50.0_m);

		LineOfSightOptions opt;
		opt.maxRange           = 50000.0_m;
		opt.initialBracketStep = 50.0_m;
		opt.bracketGrowth      = 1.5;
		opt.refineIterations   = 18;
		opt.threadCount        = 1;        // deterministic
		opt.useEarthCurvature  = false;    // keep the test local/simple
		opt.preferFastEarlyOut = false;
		opt.overlayTint        = 0.40;

		LOS los(observer, opt);

		// Target before ridge: should be visible.
		{
			const auto                    ground = test::SyntheticTopography::orthometricHeight(0.05_deg, 0.03_deg);
			const PositionGeodetic<Datum> tgt(0.05_deg, 0.03_deg, ground);
			EXPECT_TRUE(los.lineOfSightTerrain(tgt));
		}

		// Target behind ridge: should be blocked.
		{
			const auto                    ground = test::SyntheticTopography::orthometricHeight(0.05_deg, 0.08_deg);
			const PositionGeodetic<Datum> tgt(0.05_deg, 0.08_deg, ground);
			EXPECT_FALSE(los.lineOfSightTerrain(tgt));
		}

		// Generate and validate overlay image.
		const auto outPath = std::filesystem::temp_directory_path() / "coordinates_los_viewshed.ppm";
		std::cout << "Test file saved to: " << outPath << std::endl;
		if (std::filesystem::exists(outPath))
			std::filesystem::remove(outPath);

		los.saveViewshedOverlayPPM(&tile, res, outPath.string());
		ASSERT_TRUE(std::filesystem::exists(outPath));
		ASSERT_GT(std::filesystem::file_size(outPath), 0u);

		std::size_t               w = 0;
		std::size_t               h = 0;
		std::vector<std::uint8_t> rgb;
		test::readPpmP6(outPath, w, h, rgb);

		// Expect 11x11.
		EXPECT_EQ(w, 11u);
		EXPECT_EQ(h, 11u);

		// Pick a pixel west of ridge at lat=0.05, lon=0.03 => visible => greenish (G > R).
		{
			const std::size_t row = static_cast<std::size_t>(((latNE - 0.05_deg) / res).to<int>());
			const std::size_t col = static_cast<std::size_t>(((0.03_deg - lonSW) / res).to<int>());
			auto [r, g, b]        = test::pixelAt(rgb, w, row, col);
			EXPECT_GT(g, r);
			EXPECT_GE(g, b);
		}

		// Pick a pixel east of ridge at lat=0.05, lon=0.08 => blocked => reddish (R > G).
		{
			const std::size_t row = static_cast<std::size_t>(((latNE - 0.05_deg) / res).to<int>());
			const std::size_t col = static_cast<std::size_t>(((0.08_deg - lonSW) / res).to<int>());
			auto [r, g, b]        = test::pixelAt(rgb, w, row, col);
			EXPECT_GT(r, g);
			EXPECT_GE(r, b);
		}
	}

#if defined(COORDINATES_ENABLE_DTED) && COORDINATES_ENABLE_DTED
	TEST(LineOfSightDTED, Viewshed_w115_n37)
	{
		// Fixed tile selection
		constexpr degrees tileLat = 37.0566666667_deg;      // N37
		constexpr degrees tileLon = -115.0_deg;    // W115

		// Load tile
		DTEDTileManager& mgr  = DTEDTileManager::instance();
		auto             tile = mgr.tile(tileLat, tileLon);
		ASSERT_TRUE(tile) << "Failed to load DTED tile w115_n37.dt2";

		// Observer: center of tile, slightly above ground
		const PositionGeodetic<WGS84_G1674> observer(37.0566666667_deg, -114.5633333333_deg, 762.0_m);

		LineOfSight<WGS84_G1674> los(observer);

		// Output path
		std::filesystem::path out = std::filesystem::temp_directory_path() / "coordinates_los_w115_n37.ppm";

		std::cout << "Writing test image to: " << out << std::endl;

		// Generate viewshed overlay
		los.saveViewshedOverlayPPM(tile, 0.01_deg, out.string());

		ASSERT_TRUE(std::filesystem::exists(out)) << "Viewshed image was not written";

		// Read back PPM (binary P6)
		std::ifstream in(out, std::ios::binary);
		ASSERT_TRUE(in.good());

		std::string magic;
		int         width = 0, height = 0, maxval = 0;
		in >> magic >> width >> height >> maxval;
		in.get();    // consume single whitespace/newline

		ASSERT_EQ(magic, "P6");
		ASSERT_GT(width, 0);
		ASSERT_GT(height, 0);
		ASSERT_EQ(maxval, 255);

		std::vector<unsigned char> pixels(width * height * 3);
		in.read(reinterpret_cast<char*>(pixels.data()), pixels.size());

		bool sawGreenish = false;
		bool sawReddish  = false;

		for (size_t i = 0; i < pixels.size(); i += 3)
		{
			unsigned char r = pixels[i + 0];
			unsigned char g = pixels[i + 1];

			if (g > r + 5)
				sawGreenish = true;
			if (r > g + 5)
				sawReddish = true;

			if (sawGreenish && sawReddish)
				break;
		}

		EXPECT_TRUE(sawGreenish) << "Expected at least one visible (greenish) pixel";
		EXPECT_TRUE(sawReddish) << "Expected at least one blocked (reddish) pixel";
	}
#endif    // COORDINATES_ENABLE_DTED
}    // namespace coordinates
