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

#ifndef topographyTest_h__
#define topographyTest_h__

//------------------------
//	INCLUDES
//------------------------

#include "dted.h"
#include "tileException.h"
#include "dtedRawFileContents.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "algorithm.h"
#include "topography.h"

namespace fs = std::filesystem;

inline fs::path test_data_root()
{
	if (const char* env = std::getenv("COORDINATES_TEST_DATA_DIR"); env && *env)
		return fs::path(env);
	if (const char* env = std::getenv("DTED_ROOT_DIR"); env && *env)
		return fs::path(env);
	return fs::current_path() / "resources";
}

inline fs::path test_data_file(const std::string& name)
{ return test_data_root() / name; }

inline std::vector<std::byte> read_file_bytes(const fs::path& p)
{
	std::ifstream ifs(p, std::ios::binary);
	if (!ifs)
		throw std::runtime_error("Failed to open file: " + p.string());
	const std::vector<char> tmp{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};

	std::vector<std::byte> result(tmp.size());
	memcpy(result.data(), tmp.data(), tmp.size());

	return result;
}

using namespace coordinates;
using namespace units;
using namespace units::length;
using namespace units::angle;
using namespace units::time;

using namespace coordinates::topography;

namespace
{
	// The fixture for testing
	class TopographyTest : public ::testing::Test
	{
	protected:
		TopographyTest() = default;

		~TopographyTest() override {}

		void SetUp() override {}

		void TearDown() override {}
	};

	// The fixture for testing class Foo.
	class TileTest : public ::testing::Test
	{
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		TileTest()
		{
			// You can do set-up work for each test here.
		}

		~TileTest() override
		{
			// You can do clean-up work that doesn't throw exceptions here.
		}

		// If the constructor and destructor are not enough for setting up
		// and cleaning up each test, you can define the following methods:

		void SetUp() override
		{
			// Code here will be called immediately after the constructor (right
			// before each test).
		}

		void TearDown() override
		{
			// Code here will be called immediately after each test (right
			// before the destructor).
		}

		// Per-test-case set-up.
		// Called before the first test in this test case.
		// Use for expensive resources that should be shared between tests
		static void SetUpTestCase() {}

		static void TearDownTestCase() {}

		class DTEDTileWrapper : public DTEDTile
		{
		public:
			degrees<> decimalDegreeFromDtedDegree(const char ptr[], int length) const override { return DTEDTile::decimalDegreeFromDtedDegree(ptr, length); }
		};
	};

	/// NOTE: TileManager tests cannot be run out of order
	class TileManagerTest : public ::testing::Test
	{
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		TileManagerTest()
		{
			// You can do set-up work for each test here.
		}

		~TileManagerTest() override
		{
			// You can do clean-up work that doesn't throw exceptions here.
		}

		// If the constructor and destructor are not enough for setting up
		// and cleaning up each test, you can define the following methods:

		void SetUp() override
		{
			// Code here will be called immediately after the constructor (right
			// before each test).
		}

		void TearDown() override
		{
			// Code here will be called immediately after each test (right
			// before the destructor).
		}
	};

	class DTEDTest : public ::testing::Test
	{
	protected:
		// You can remove any or all of the following functions if its body
		// is empty.

		DTEDTest()
		{
			// You can do set-up work for each test here.
		}

		~DTEDTest() override
		{
			// You can do clean-up work that doesn't throw exceptions here.
		}

		// If the constructor and destructor are not enough for setting up
		// and cleaning up each test, you can define the following methods:

		void SetUp() override
		{
			// Code here will be called immediately after the constructor (right
			// before each test).
		}

		void TearDown() override
		{
			// Code here will be called immediately after each test (right
			// before the destructor).
		}
	};

	TEST_F(TopographyTest, has_orthometricHeight)
	{
		EXPECT_TRUE(coordinates::traits::has_orthometricHeight<topography::NULL_TOPOGRAPHY>);
		EXPECT_TRUE(coordinates::traits::has_orthometricHeight<topography::DTED>);
		EXPECT_FALSE(coordinates::traits::has_orthometricHeight<ellipsoids::GRS80>);
		EXPECT_FALSE(coordinates::traits::has_orthometricHeight<double>);
	}

	TEST_F(TopographyTest, is_topography)
	{
		EXPECT_TRUE(coordinates::traits::is_topography<topography::NULL_TOPOGRAPHY>);
		EXPECT_TRUE(coordinates::traits::is_topography<topography::DTED>);
		EXPECT_FALSE(coordinates::traits::is_topography<ellipsoids::GRS80>);
		EXPECT_FALSE(coordinates::traits::is_topography<double>);
	}

	TEST_F(TileTest, sizeofDt2File)
	{
		// This is a *decent* check that we have all the right fields defined for the dted file.
		EXPECT_EQ(80, sizeof(DTEDRawFileContents<3601, 3601>::UserHeaderLabel));
		EXPECT_EQ(648, sizeof(DTEDRawFileContents<3601, 3601>::DataSetIdentificationRecord));
		EXPECT_EQ(2700, sizeof(DTEDRawFileContents<3601, 3601>::AccuracyDescriptionRecord));
		EXPECT_EQ(7214, sizeof(DTEDRawFileContents<3601, 3601>::DataRecord));
		EXPECT_EQ(25981042, sizeof(DTEDRawFileContents<3601, 3601>));
	}

	TEST_F(TileTest, size)
	{
		DTEDTile t;
		// width/height should work on default/unloaded tiles AND loaded tiles
		EXPECT_EQ(1.0_deg, t.width());
		EXPECT_EQ(1.0_deg, t.height());

		DTEDTile t2(test_data_file("e141_s13.dt1"));
		EXPECT_EQ(1.0_deg, t2.width());
		EXPECT_EQ(1.0_deg, t2.height());
	}

	TEST_F(TileTest, loadClear)
	{
		DTEDTile tFail;

		EXPECT_THROW(tFail.load(), tile_invalid);

		DTEDTile t(test_data_file("e141_s13.dt1"));
		EXPECT_FALSE(t.isLoaded());
		t.load();
		EXPECT_TRUE(t.isLoaded());
		t.clear();
		EXPECT_FALSE(t.isLoaded());
	}

	TEST_F(TileTest, convertDegrees)
	{
		DTEDTileWrapper t;
		char            str[11] = "1161234.5W";
		auto            result  = t.decimalDegreeFromDtedDegree(str, static_cast<int>(strlen(str)));
		EXPECT_NEAR(-116.209583, result.to<double>(), 5.0e-7);
	}

	TEST_F(TileTest, metadata)
	{
		const auto                                             info = test_data_file("e141_s13.dt1");
		std::unique_ptr<AbstractTile> tile = std::make_unique<DTEDTile>(info);
		EXPECT_FALSE(tile->isLoaded());
		TileMetadata metadata = tile->metadata();
		EXPECT_TRUE(tile->isLoaded());
		EXPECT_EQ(-13.0_deg, metadata.southwestLatitude());
		EXPECT_EQ(141.0_deg, metadata.southwestLongitude());
		EXPECT_EQ(-12.0_deg, metadata.northeastLatitude());
		EXPECT_EQ(142.0_deg, metadata.northeastLongitude());
		EXPECT_STREQ("DTED1", metadata.dataType().c_str());
		EXPECT_EQ(arcseconds(3.0), metadata.latitudeResolution());
		EXPECT_EQ(arcseconds(3.0), metadata.longitudeResolution());
	}

	TEST_F(TileTest, elevation)
	{
		DTEDTile tile(test_data_file("e141_s13.dt1"));
		EXPECT_THROW((tile.elevation(37.3363844298_deg, -116.603493024_deg)), tile_out_of_bounds);

		// WHAT TO DO IF THIS TEST FAILS:
		// This test tests that the elevation/interpolation of dted work by going through an entire
		// tile and generating an image from the interpolated output. It then compares this image to a
		// saved 'golden' image I made on 3/3/16.
		// If the test fails open the image and manually make sure it looks like `image.pgm` in the
		// resources folder. If it doesn't, you have a problem. If it does, what changed?
		// - interpolation: if you changed interpolation, fixed bug, etc, the image will probably be
		// slightly different. That may be OK, and if so just save a new golden image.
		// - dtedTile: This is questionable, you probably messed it up. If you are sure you didn't, save
		// a new golden image.
		// - the resolution of the generated image: well that was dumb! Change it back or save a new golden
		// image.


		try
		{
			auto data = topography::image(&tile, arcseconds(9.0));    // write the image to disc
			std::ofstream image("./e141_s13.pgm", std::ios::binary);
			if (!image)
				throw std::runtime_error("Failed to open ./e141_s13.pgm for write");

			const std::string header = "P5\n# Generated from coordTest.exe\n" + std::to_string(data[0].size()) + " " + std::to_string(data.size()) + "\n255\n";
			image.write(header.data(), static_cast<std::streamsize>(header.size()));

			for (const auto& row : data)
			{
				image.write(reinterpret_cast<const char*>(row.data()), static_cast<std::streamsize>(row.size()));
			}
		}
		catch (...)
		{
			FAIL() << "Unexpected exception thrown";
		}

		// const auto generatedData = read_file_bytes(fs::path("./e141_s13.pgm"));
		// const auto goldenData    = read_file_bytes(test_data_file("e141_s13.pgm"));
	//	EXPECT_EQ(goldenData, generatedData);
	}

	TEST_F(TileTest, hillshade)
	{
		DTEDTile tile(test_data_file("e141_s13.dt1"));

		{
			auto          data = hillshade(&tile, arcseconds(9.0));
			std::ofstream image("./e141_s13_shade.pgm", std::ios::binary);
			if (!image)
				throw std::runtime_error("Failed to open ./e141_s13_shade.pgm for write");

			const std::string header = "P5\n# Generated from coordTest.exe\n" + std::to_string(data[0].size()) + " " + std::to_string(data.size()) + "\n255\n";
			image.write(header.data(), static_cast<std::streamsize>(header.size()));

			for (const auto& row : data)
			{
				image.write(reinterpret_cast<const char*>(row.data()), static_cast<std::streamsize>(row.size()));
			}
		}

		// compare to golden image
		EXPECT_TRUE(fs::exists(fs::path("./e141_s13_shade.pgm")));
		EXPECT_TRUE(fs::exists(test_data_file("e141_s13_shade.pgm")));

		const auto generatedData = read_file_bytes(fs::path("./e141_s13_shade.pgm"));
		const auto goldenData    = read_file_bytes(test_data_file("e141_s13_shade.pgm"));
		EXPECT_EQ(goldenData, generatedData);
	}

	// Hillshade over K2 / the Karakoram (e076_n35, real SRTM 1-arcsec data, 2.3-8.5 km of relief). The e141_s13
	// tile is nearly flat, so its shading is dominated by slope magnitude and barely exercises the aspect (slope
	// facing direction) term -- a hillshade aspect bug is invisible there. This tile has extreme relief, so the
	// aspect term dominates and the test actually covers it.
	TEST_F(TileTest, hillshadeK2)
	{
		DTEDTile tile(test_data_file("e076_n35.dt2"));

		{
			auto          data = hillshade(&tile, arcseconds(9.0));
			std::ofstream image("./e076_n35_shade.pgm", std::ios::binary);
			if (!image)
				throw std::runtime_error("Failed to open ./e076_n35_shade.pgm for write");

			const std::string header = "P5\n# Generated from coordTest.exe\n" + std::to_string(data[0].size()) + " " + std::to_string(data.size()) + "\n255\n";
			image.write(header.data(), static_cast<std::streamsize>(header.size()));

			for (const auto& row : data)
			{
				image.write(reinterpret_cast<const char*>(row.data()), static_cast<std::streamsize>(row.size()));
			}
		}

		// compare to golden image
		EXPECT_TRUE(fs::exists(fs::path("./e076_n35_shade.pgm")));
		EXPECT_TRUE(fs::exists(test_data_file("e076_n35_shade.pgm")));

		const auto generatedData = read_file_bytes(fs::path("./e076_n35_shade.pgm"));
		const auto goldenData    = read_file_bytes(test_data_file("e076_n35_shade.pgm"));
		EXPECT_EQ(goldenData, generatedData);
	}

	TEST_F(TileManagerTest, NumTiles)
	{
		const auto& TILE_MANAGER = DTEDTileManager::instance();
		EXPECT_EQ(64800, TILE_MANAGER.maxNumTiles());
		EXPECT_GE(3571, TILE_MANAGER.numTiles());    // 3571 is the number of tiles you'd have if you ONLY had the minimum DTED set required by this test.
		EXPECT_EQ(0, TILE_MANAGER.numLoadedTiles());
	}

	TEST_F(TileManagerTest, exists)
	{
		const auto& TILE_MANAGER = DTEDTileManager::instance();
		EXPECT_FALSE(TILE_MANAGER.exists(-60_deg, 0_deg));     // middle of south-south Atlantic.
		EXPECT_TRUE(TILE_MANAGER.exists(37_deg, -115_deg));    // middle of the USA
		EXPECT_TRUE(TILE_MANAGER.exists(-13_deg, 141_deg));    // Northern Australia
	}

	TEST_F(TileManagerTest, elevation)
	{
		// we know tile elevation is correct from the image testing above, so use it as truth data
		const DTEDTile tile(test_data_file("e141_s13.dt1"));

		const auto& TILE_MANAGER = DTEDTileManager::instance();
		EXPECT_EQ(0_m, TILE_MANAGER.elevation(-60_deg, 0_deg));
		EXPECT_NEAR(tile.elevation(-12.98_deg, 141.98_deg).to<double>(), TILE_MANAGER.elevation(-12.98_deg, 141.98_deg).to<double>(), 5.0e-9);
	}

	TEST_F(TileManagerTest, isLoaded)
	{
		DTEDTileManager& TILE_MANAGER = DTEDTileManager::instance();
		TILE_MANAGER.elevation(-12.98_deg, 141.98_deg);
		EXPECT_FALSE(TILE_MANAGER.isLoaded(37_deg, -115_deg));
		EXPECT_TRUE(TILE_MANAGER.isLoaded(-12.98_deg, 141.98_deg));
	}

	TEST_F(TileManagerTest, tile)
	{
		DTEDTileManager& TILE_MANAGER = DTEDTileManager::instance();

		AbstractTile* tile   = new DTEDTile(test_data_file("e141_s13.dt1"));
		auto                                   tmTile = TILE_MANAGER.tile(-12.98_deg, 141.98_deg);

		EXPECT_NEAR(tile->elevation(-12.98_deg, 141.98_deg).to<double>(), tmTile->elevation(-12.98_deg, 141.98_deg).to<double>(), 5.0e-9);

		delete tile;
	}

	TEST_F(TileManagerTest, tileLimit)
	{
		DTEDTile tile;

		DTEDTileManager& TILE_MANAGER = DTEDTileManager::instance();
		EXPECT_EQ(TILE_MANAGER.maxNumTiles(), TILE_MANAGER.tileLimit());
	}

	TEST_F(TileManagerTest, setTileLimit)
	{
		DTEDTile tile;

		DTEDTileManager& TILE_MANAGER = DTEDTileManager::instance();
		EXPECT_EQ(TILE_MANAGER.maxNumTiles(), TILE_MANAGER.tileLimit());
		TILE_MANAGER.setTileLimit(100);
		EXPECT_EQ(100, TILE_MANAGER.tileLimit());
	}

	TEST_F(DTEDTest, orthometricHeight)
	{
		// Got this truth data by querying the DTED file in QGIS. Our data is interpolated, so we dont
		// expect it to be exactly the same
		EXPECT_UNITS_NEAR(17.0_m, DTED::orthometricHeight(-12.98_deg, 141.98_deg), 0.5_m);
		EXPECT_UNITS_NEAR(35.5_m, DTED::orthometricHeight(-12.8630716549_deg, 141.792396611_deg), 1.0_m);
		EXPECT_UNITS_NEAR(19.0_m, DTED::orthometricHeight(-12.5122866197_deg, 141.767974868_deg), 1.0_m);
	}
}    // namespace
#endif    // topographyTest_h__