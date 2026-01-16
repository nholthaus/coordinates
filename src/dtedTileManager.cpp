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

#include "dtedTileManager.h"
#include "tileException.h"

#include <cmath>
#include <regex>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <vector>

using namespace units::literals;

// --------------------------------------------------------------------------------
// 	FUNCTION: instance (public )
// --------------------------------------------------------------------------------
DTEDTileManager& DTEDTileManager::instance()
{
	static DTEDTileManager instance;
	return instance;
}

// --------------------------------------------------------------------------------
// 	FUNCTION: DTEDTileManager (public )
// --------------------------------------------------------------------------------
DTEDTileManager::DTEDTileManager() : m_numTiles(0)
{
	// make 360x180 tile grid
	auto tileHeight = tile_type().height();
	auto tileWidth = tile_type().width();

	double height = 180.0_deg / tileHeight;
	double width = 360.0_deg / tileWidth;

	const auto gridSize = static_cast<size_t>(std::ceil(height * width));
	m_tileGrid.resize(gridSize);
	m_tileCache.reserve(gridSize);

	auto tiles = searchRecursivelyForFiles(
		std::filesystem::path(resolve_dted_root()),
		std::vector<std::string>{"dt0", "dt1", "dt2"});

	m_numTiles = tiles.size();

	const std::regex rx(R"(.*[/\\]([WwEe])(\d{1,3})[/\\_]([NnSs])(\d{1,3})\.dt([0-2]))");

	for (const auto& p : tiles)
	{
		std::smatch m;
		const std::string s = p.generic_string();
		if (!std::regex_match(s, m, rx) || m.size() != 6)
			continue;

		units::degrees longitude(std::stod(m[2].str()));
		if (m[1].str() == "W" || m[1].str() == "w")
			longitude = -1 * longitude;

		units::degrees latitude(std::stod(m[4].str()));
		if (m[3].str() == "S" || m[3].str() == "s")
			latitude = -1 * latitude;

		const int level = (m[5].str().empty()) ? -1 : (m[5].str()[0] - '0');
		const size_t idx = indexFromLatLon(latitude, longitude);
		auto& rec = m_tileGrid[idx];

		// Choose the best (highest level) available tile for this 1x1 degree cell.
		if (rec.filePath.empty() || level > rec.level)
		{
			rec.filePath = p;
			rec.level = level;
		}
	}
}

// --------------------------------------------------------------------------------
// 	FUNCTION: ~DTEDTileManager (public )
// --------------------------------------------------------------------------------
DTEDTileManager::~DTEDTileManager()
{

}

// --------------------------------------------------------------------------------
// 	FUNCTION: isInMemory (public )
// --------------------------------------------------------------------------------
bool DTEDTileManager::isLoaded(const units::degrees<>& latitude, const units::degrees<>& longitude) const
{
	auto t = tile(latitude, longitude);
	if (!t)
		return false;

	return t->isLoaded();
}

// --------------------------------------------------------------------------------
// 	FUNCTION: elevation (public )
// --------------------------------------------------------------------------------
units::meters<> DTEDTileManager::elevation(const units::degrees<>& latitude, const units::degrees<>& longitude) const
{
	if (!exists(latitude, longitude))
		return 0.0_m;

	return tile(latitude, longitude)->elevation(latitude, longitude);
}

// --------------------------------------------------------------------------------
// 	FUNCTION: tile (public )
// --------------------------------------------------------------------------------
AbstractTile* DTEDTileManager::tile(const units::degrees<>& latitude, const units::degrees<>& longitude) const
{
	const size_t index = indexFromLatLon(latitude, longitude);

	// if the tile isn't already cached, than cache it.
	if (!m_tileCache.count(index))
	{
		// find its location on disc
		if (!m_tileGrid[index].filePath.empty() && std::filesystem::is_regular_file(m_tileGrid[index].filePath))
		{
			// load it into the cache. Once the cache is full, this will automatically remove
			// the oldest one.
			m_tileCache.emplace(index, m_tileGrid[index].filePath);
		} 
		// uh-oh, we don't have this tile anywhere
		else
		{
			throw tile_invalid("The requested DTED tile could not be found", __FILE__, __LINE__);
		}
	}

	return &m_tileCache.at(index);
}

// --------------------------------------------------------------------------------
// 	FUNCTION: searchRecursivelyForFiles (public )
// --------------------------------------------------------------------------------
std::vector<std::filesystem::path> DTEDTileManager::searchRecursivelyForFiles(
	const std::filesystem::path& rootSearchDir,
	const std::vector<std::string>& extensions)
{
	std::vector<std::filesystem::path> matchingFiles;
	if (rootSearchDir.empty() || !std::filesystem::exists(rootSearchDir))
		return matchingFiles;

	auto toLower = [](std::string v) {
		std::ranges::transform(v, v.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return v;
	};

	std::vector<std::string> extLower;
	extLower.reserve(extensions.size());
	for (const auto& e : extensions)
		extLower.push_back(toLower(e));

	for (const auto& entry : std::filesystem::recursive_directory_iterator(rootSearchDir))
	{
		if (!entry.is_regular_file())
			continue;

		std::string ext = entry.path().extension().string();
		if (!ext.empty() && ext[0] == '.')
			ext.erase(ext.begin());
		ext = toLower(ext);

		if (std::ranges::find(extLower, ext) != extLower.end())
			matchingFiles.push_back(entry.path());
	}

	return matchingFiles;
}

// --------------------------------------------------------------------------------
// 	FUNCTION: exists (public )
// --------------------------------------------------------------------------------
bool DTEDTileManager::exists(const units::degrees<>& latitude, const units::degrees<>& longitude) const
{
	return (!m_tileGrid[indexFromLatLon(latitude, longitude)].filePath.empty());
}

// --------------------------------------------------------------------------------
// 	FUNCTION: indexFromLongitude ()
// --------------------------------------------------------------------------------
size_t DTEDTileManager::indexFromLatLon(units::degrees<> latitude, units::degrees<> longitude)
{
	return static_cast<size_t>(std::floor((90.0 - std::floor(latitude.to<long double>()) - 1.0))) * 360u
		+ static_cast<size_t>(std::floor((180_deg + longitude).to<long double>()));
}

// --------------------------------------------------------------------------------
// 	FUNCTION: setTileLimit (public )
// --------------------------------------------------------------------------------
void DTEDTileManager::setTileLimit(size_t limit)
{
	if (limit < m_tileCache.capacity())
	{
		m_tileCache.shrink(limit);
	} 
	else
	{
		m_tileCache.reserve(limit);
	}
}

// --------------------------------------------------------------------------------
// 	FUNCTION: tileLimit (public )
// --------------------------------------------------------------------------------
size_t DTEDTileManager::tileLimit() const 
{
	return m_tileCache.capacity();
}

// --------------------------------------------------------------------------------
// 	FUNCTION: numTiles (public )
// --------------------------------------------------------------------------------
size_t DTEDTileManager::numTiles() const
{
	return m_numTiles;
}

// --------------------------------------------------------------------------------
// 	FUNCTION: numLoadedTiles (public )
// --------------------------------------------------------------------------------
size_t DTEDTileManager::numLoadedTiles() const
{
	return m_tileCache.size();
}

// --------------------------------------------------------------------------------
// 	FUNCTION: maxNumTiles (public )
// --------------------------------------------------------------------------------
size_t DTEDTileManager::maxNumTiles() const
{
	return m_tileGrid.size();
}

// --------------------------------------------------------------------------------
// 	FUNCTION: resolve_dted_root (protected )
// --------------------------------------------------------------------------------
std::filesystem::path DTEDTileManager::resolve_dted_root()
{
	if (const char* env = std::getenv("DTED_ROOT_DIR"))
	{
		if (*env != '\0')
		{
			return std::filesystem::path(env);
		}
	}

	// Portable, standard fallback
	return std::filesystem::current_path();
}