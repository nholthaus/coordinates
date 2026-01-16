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

#include "dtedTile.h"
#include "dtedRawFileContents.h"
#include "tileException.h"

#include <interpolate.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

#include <limits>

using namespace units::literals;
using namespace units::length;
using namespace units::angle;

// --------------------------------------------------------------------------------
// 	FUNCTION: DTEDTile (public )
// --------------------------------------------------------------------------------
DTEDTile::DTEDTile(std::filesystem::path filePath /*= {}*/) :
	m_filePath(std::move(filePath))
{

}

// --------------------------------------------------------------------------------
// 	FUNCTION: DTEDTile (public )
// --------------------------------------------------------------------------------
DTEDTile::DTEDTile(std::string path) :
	DTEDTile(std::filesystem::path(std::move(path)))
{

}

// --------------------------------------------------------------------------------
// 	FUNCTION: DTEDTile (public )
// --------------------------------------------------------------------------------
DTEDTile::DTEDTile(const char* path) :
	DTEDTile(path ? std::string(path) : std::string())
{

}

// --------------------------------------------------------------------------------
// 	FUNCTION: tileMetadata (public )
// --------------------------------------------------------------------------------
TileMetadata DTEDTile::metadata() const
{
	if (!isLoaded())
	{
		if (!load())
			throw tile_exception("failed to load tile.", __FILE__, __LINE__);
	}
	return m_metadata;
}

// --------------------------------------------------------------------------------
// 	FUNCTION: isInMemory (public )
// --------------------------------------------------------------------------------
bool DTEDTile::isLoaded() const
{
	return !m_data.empty();
}

// --------------------------------------------------------------------------------
// 	FUNCTION: load ()
// --------------------------------------------------------------------------------
bool DTEDTile::load() const
{
	if (m_filePath.empty())
		throw tile_invalid("Tile path is empty.", __FILE__, __LINE__);

	// Open the DTED file and read it into memory
	if (!std::filesystem::exists(m_filePath))
		throw tile_invalid("Tile \"" + m_filePath.string() + "\" does not exist.", __FILE__, __LINE__);

	std::ifstream dtedFile(m_filePath, std::ios::binary);
	if (!dtedFile)
		throw tile_invalid("Tile \"" + m_filePath.string() + "\" could not be opened.", __FILE__, __LINE__);

	dtedFile.seekg(0, std::ios::end);
	const std::streamsize fileSize = dtedFile.tellg();
	if (fileSize <= 0)
		throw tile_invalid("Tile \"" + m_filePath.string() + "\" appears to be empty.", __FILE__, __LINE__);
	dtedFile.seekg(0, std::ios::beg);

	m_data.resize(static_cast<size_t>(fileSize));
	if (!dtedFile.read(reinterpret_cast<char*>(m_data.data()), fileSize))
		throw tile_invalid("Tile \"" + m_filePath.string() + "\" could not be read.", __FILE__, __LINE__);

	// set the basic metadata parameters
	using DTEDFileMask = DTEDRawFileContents<1, 1>*;		// this mask is suitable for reading header data, but NOT elevation data!
	auto DTEDDataPtr = reinterpret_cast<DTEDFileMask>(m_data.data());
	m_metadata.setSouthwestLatitude(decimalDegreeFromDtedDegree(DTEDDataPtr->dataSetIndentificationRecord.latitideOfSWCorner,
		sizeof(DTEDDataPtr->dataSetIndentificationRecord.latitideOfSWCorner)));
	m_metadata.setSouthwestLongitude(decimalDegreeFromDtedDegree(DTEDDataPtr->dataSetIndentificationRecord.longitudeOfSWCorner,
		sizeof(DTEDDataPtr->dataSetIndentificationRecord.longitudeOfSWCorner)));
	m_metadata.setNortheastLatitude(decimalDegreeFromDtedDegree(DTEDDataPtr->dataSetIndentificationRecord.latitideOfNECorner,
		sizeof(DTEDDataPtr->dataSetIndentificationRecord.latitideOfNECorner)));
	m_metadata.setNortheastLongitude(decimalDegreeFromDtedDegree(DTEDDataPtr->dataSetIndentificationRecord.longitudeOfNECorner,
		sizeof(DTEDDataPtr->dataSetIndentificationRecord.longitudeOfNECorner)));

	m_metadata.setFilePath(m_filePath.string());

	m_metadata.setDataType(std::string(DTEDDataPtr->dataSetIndentificationRecord.nimaSeriesDesignator,
		sizeof(DTEDDataPtr->dataSetIndentificationRecord.nimaSeriesDesignator)));

	m_metadata.setLatitudeResolution(decimalDegreeFromDtedTenthsec(DTEDDataPtr->dataSetIndentificationRecord.latitudeInterval,
		sizeof(DTEDDataPtr->dataSetIndentificationRecord.latitudeInterval)));
	m_metadata.setLongitudeResolution(decimalDegreeFromDtedTenthsec(DTEDDataPtr->dataSetIndentificationRecord.longitudeInterval,
		sizeof(DTEDDataPtr->dataSetIndentificationRecord.longitudeInterval)));

	// get number of lat/lon database entries from the header data
	auto parseUll = [](const char* ptr, size_t len) -> unsigned long long
	{
		std::string s(ptr, ptr + len);
		// DTED fields are space padded; keep only digits
		s.erase(std::ranges::remove_if(s, [](unsigned char c) { return !std::isdigit(c); }).begin(), s.end());
		if (s.empty())
			return 0ull;
		try
		{
			return std::stoull(s);
		}
		catch (...)
		{
			return 0ull;
		}
	};

	m_metadata.setNumLatitudeLines(parseUll(reinterpret_cast<DTEDFileMask>(m_data.data())->dataSetIndentificationRecord.numLatitudeLines,
		sizeof(DTEDDataPtr->dataSetIndentificationRecord.numLatitudeLines)));
	m_metadata.setNumLongitudeLines(parseUll(reinterpret_cast<DTEDFileMask>(m_data.data())->dataSetIndentificationRecord.numLongitudeLines,
		sizeof(DTEDDataPtr->dataSetIndentificationRecord.numLongitudeLines)));

	return true;
}

// --------------------------------------------------------------------------------
// 	FUNCTION: clear ()
// --------------------------------------------------------------------------------
bool DTEDTile::clear() const
{
	// this frees all memory
	m_data.clear();
	return true;
}

// --------------------------------------------------------------------------------
// 	FUNCTION: elevation (public )
// --------------------------------------------------------------------------------
meters<> DTEDTile::elevation(const degrees<>& latitude, const degrees<>& longitude) const
{
	if (!isLoaded())
	{
		if (!load())
			throw tile_exception("failed to load DTED tile", __FILE__, __LINE__);
	}

	// check tile in bounds
	if (!(latitude >= m_metadata.southwestLatitude() && latitude <= m_metadata.northeastLatitude() &&
		longitude >= m_metadata.southwestLongitude() && longitude <= m_metadata.northeastLongitude()))
	{
		auto fmt = [](double v) -> std::string
		{
			std::ostringstream oss;
			oss.setf(std::ios::fixed);
			oss.precision(10);
			oss << v;
			return oss.str();
		};

		throw tile_out_of_bounds(
			"Coordinate (" + fmt(latitude.to<double>()) + ',' + fmt(longitude.to<double>()) +
			") is out of bounds of the tile with SW Corner at (" + fmt(m_metadata.southwestLatitude().to<double>()) + ',' +
			fmt(m_metadata.southwestLongitude().to<double>()) + ") and NW Corner at (" +
			fmt(m_metadata.northeastLatitude().to<double>()) + ',' +
			fmt(m_metadata.northeastLongitude().to<double>()) + ").",
			__FILE__, __LINE__);
	}

	// get latitude index.to<double>()s (with decimals)
	double latX = (latitude - m_metadata.southwestLatitude()) / m_metadata.latitudeResolution();
	double lonY = (longitude - m_metadata.southwestLongitude()) / m_metadata.longitudeResolution();

	// find the lat/lon index (integer)
	unsigned int latIndex = floor(latX);
	unsigned int lonIndex = floor(lonY);

	// interpolation x,y
	double xVal = (latX - latIndex);
	double yVal = (lonY - lonIndex);

	// create surface of points to interpolate with. Use nearest-neighbor extrapolation on the edges
	interpolationMatrix<double> elevationSurface;
	constexpr size_t interpolationDimension = elevationSurface.size();

	for (int i = 0; i < interpolationDimension; ++i)
	{
		int row = latIndex + (i - 1);
		row = ((row < 0) ? 0 : row);																// if the row is less than 0, copy the nearest neighbor (i.e. repeat 0)
		row = ((row >= static_cast<int>(m_metadata.numLatitudeLines())) ? static_cast<int>(m_metadata.numLatitudeLines()) - 1 : row);					// if the row is on the right edge, copy the right pixel

		for (int j = 0; j < interpolationDimension; ++j)
		{
			int column = lonIndex + (j - 1);
			column = ((column < 0) ? 0 : column);													// if the index is negative, repeat the left-edge pixel
			column = ((column >= static_cast<int>(m_metadata.numLongitudeLines())) ? static_cast<int>(m_metadata.numLongitudeLines()) - 1 : column);		// on the right edge, repeat the right-edge pixel

			// query the LUT
			elevationSurface[i][j] = elevationFromIndex(row, column);
		}
	}

	return meters(bicubicInterpolate(elevationSurface, xVal, yVal));
}

// --------------------------------------------------------------------------------
// 	FUNCTION: width ()
// --------------------------------------------------------------------------------
degrees<> DTEDTile::width() const
{
	return 1_deg;
}

// --------------------------------------------------------------------------------
// 	FUNCTION: height ()
// --------------------------------------------------------------------------------
degrees<> DTEDTile::height() const
{
	return 1_deg;
}

// --------------------------------------------------------------------------------
// 	FUNCTION: elevation (public )
// --------------------------------------------------------------------------------
meters<> DTEDTile::elevation(size_t latitudeIndex, size_t longitudeIndex) const
{
	if (!isLoaded())
	{
		if (!load())
			throw tile_exception("failed to load DTED tile", __FILE__, __LINE__);
	}

	// check tile in bounds
	if (latitudeIndex > m_metadata.numLatitudeLines() || longitudeIndex > m_metadata.numLongitudeLines())
	{
		auto fmt = [](double v) -> std::string
		{
			std::ostringstream oss;
			oss.setf(std::ios::fixed);
			oss.precision(10);
			oss << v;
			return oss.str();
		};

		throw tile_out_of_bounds(
			"Index (" + std::to_string(latitudeIndex) + ',' + std::to_string(longitudeIndex) +
			") is out of bounds of the tile with SW Corner at (" + fmt(m_metadata.southwestLatitude().to<double>()) + ',' +
			fmt(m_metadata.southwestLongitude().to<double>()) + ") and NW Corner at (" +
			fmt(m_metadata.northeastLatitude().to<double>()) + ',' +
			fmt(m_metadata.northeastLongitude().to<double>()) + ").",
			__FILE__, __LINE__);
	}

	return meters(elevationFromIndex(latitudeIndex, longitudeIndex));
}

// --------------------------------------------------------------------------------
// 	FUNCTION: decimalDegreeFromDtedDegree (protected )
// --------------------------------------------------------------------------------
degrees<> DTEDTile::decimalDegreeFromDtedDegree(const char ptr[], int length) const
{
	// Extract decimal-degree data from the DTED string: [DDMMSSH, DDDMMSSH, or DDDMMSS.SH]
	// Examples: "374512N", "1223015W", "1223015.3W"
	const std::string search(ptr, ptr + length);
	static const std::regex rx(R"(([0-9]{2,3})([0-9]{2})([0-9]{2})\.?([0-9]?)([NnSsWwEe]))");
	std::smatch m;
	if (!std::regex_search(search, m, rx) || m.size() != 6)
	{
		// If the field is malformed, preserve the old behavior: return NaN degrees.
		return degrees<>(std::numeric_limits<double>::quiet_NaN());
	}

	const double deg = std::stod(m[1].str());
	const double min = std::stod(m[2].str());
	const double sec = std::stod(m[3].str());
	const double tenthsec = m[4].str().empty() ? 0.0 : std::stod(m[4].str());

	double quadrant = 1.0;
	const char hemi = m[5].str().empty() ? 'N' : m[5].str()[0];
	if (hemi == 'S' || hemi == 's' || hemi == 'W' || hemi == 'w')
		quadrant = -1.0;

	return degrees(quadrant * (degrees(deg) + arcminutes(min) + arcseconds(sec) + 0.1 * arcseconds(tenthsec)));
}

// --------------------------------------------------------------------------------
// 	FUNCTION: decimalDegreeFromDtedTenthsec (protected )
// --------------------------------------------------------------------------------
degrees<> DTEDTile::decimalDegreeFromDtedTenthsec(const char ptr[], int length) const
{
	// DTED stores resolution as a 4-character ASCII integer representing 1/10 arc-seconds.
	const std::string search(ptr, ptr + length);
	static const std::regex rx(R"(([0-9]+))");
	std::smatch m;
	if (!std::regex_search(search, m, rx) || m.size() != 2)
		return degrees<>(std::numeric_limits<double>::quiet_NaN());

	const double tenthsec = std::stod(m[1].str());
	return degrees<>(0.1 * arcseconds(tenthsec));
}

// --------------------------------------------------------------------------------
// 	FUNCTION: elevationFromIndex (public )
// --------------------------------------------------------------------------------
int16_t DTEDTile::elevationFromIndex(size_t latIndex, size_t lonIndex) const
{
	// MOST COMMON TYPES FIRST TO HELP THE BRANCH PREDICTOR
	// DTED Level 2 : 0 - 50 degrees North-South
	if (m_metadata.numLatitudeLines() == 3601 && m_metadata.numLongitudeLines() == 3601)
	{
		return elevationFromIndex<DTEDRawFileContents<3601, 3601>>(latIndex, lonIndex);
	}
	// DTED Level 1 : 0 - 50 degrees North-South
	else if (m_metadata.numLatitudeLines() == 1201 && m_metadata.numLongitudeLines() == 1201)
	{
		return elevationFromIndex<DTEDRawFileContents<1201, 1201>>(latIndex, lonIndex);
	}
	// DTED Level 0 : 0 - 50 degrees North-South
	else if (m_metadata.numLatitudeLines() == 121 && m_metadata.numLongitudeLines() == 121)
	{
		return elevationFromIndex<DTEDRawFileContents<121, 121>>(latIndex, lonIndex);
	}
	// LESS COMMON TYPES
	// DTED Level 0 : 50 - 70 degrees North-South
	else if (m_metadata.numLatitudeLines() == 121 && m_metadata.numLongitudeLines() == 61)
	{
		return elevationFromIndex<DTEDRawFileContents<121, 61>>(latIndex, lonIndex);
	}
	// DTED Level 0 : 70 - 75 degrees North-South
	else if (m_metadata.numLatitudeLines() == 121 && m_metadata.numLongitudeLines() == 41)
	{
		return elevationFromIndex<DTEDRawFileContents<121, 41>>(latIndex, lonIndex);
	}
	// DTED Level 0 : 75 - 80 degrees North-South
	else if (m_metadata.numLatitudeLines() == 121 && m_metadata.numLongitudeLines() == 31)
	{
		return elevationFromIndex<DTEDRawFileContents<121, 31>>(latIndex, lonIndex);
	}
	// DTED Level 0 : 80 - 90 degrees North-South
	else if (m_metadata.numLatitudeLines() == 121 && m_metadata.numLongitudeLines() == 21)
	{
		return elevationFromIndex<DTEDRawFileContents<121, 21>>(latIndex, lonIndex);
	}
	// DTED Level 1 : 50 - 70 degrees North-South
	else if (m_metadata.numLatitudeLines() == 1201 && m_metadata.numLongitudeLines() == 601)
	{
		return elevationFromIndex<DTEDRawFileContents<1201, 601>>(latIndex, lonIndex);
	}
	// DTED Level 1 : 70 - 75 degrees North-South
	else if (m_metadata.numLatitudeLines() == 1201 && m_metadata.numLongitudeLines() == 401)
	{
		return elevationFromIndex<DTEDRawFileContents<1201, 401>>(latIndex, lonIndex);
	}
	// DTED Level 1 : 75 - 80 degrees North-South
	else if (m_metadata.numLatitudeLines() == 1201 && m_metadata.numLongitudeLines() == 301)
	{
		return elevationFromIndex<DTEDRawFileContents<1201, 301>>(latIndex, lonIndex);
	}
	// DTED Level 1 : 80 - 90 degrees North-South
	else if (m_metadata.numLatitudeLines() == 1201 && m_metadata.numLongitudeLines() == 201)
	{
		return elevationFromIndex<DTEDRawFileContents<1201, 201>>(latIndex, lonIndex);
	}
	
	// DTED Level 2 : 50 - 70 degrees North-South
	else if (m_metadata.numLatitudeLines() == 3601 && m_metadata.numLongitudeLines() == 1801)
	{
		return elevationFromIndex<DTEDRawFileContents<3601, 1801>>(latIndex, lonIndex);
	}
	// DTED Level 2 : 70 - 75 degrees North-South
	else if (m_metadata.numLatitudeLines() == 3601 && m_metadata.numLongitudeLines() == 1201)
	{
		return elevationFromIndex<DTEDRawFileContents<3601, 1201>>(latIndex, lonIndex);
	}
	// DTED Level 2 : 75 - 80 degrees North-South
	else if (m_metadata.numLatitudeLines() == 3601 && m_metadata.numLongitudeLines() == 901)
	{
		return elevationFromIndex<DTEDRawFileContents<3601, 901>>(latIndex, lonIndex);
	}
	// DTED Level 2 : 80 - 90 degrees North-South
	else if (m_metadata.numLatitudeLines() == 3601 && m_metadata.numLongitudeLines() == 601)
	{
		return elevationFromIndex<DTEDRawFileContents<3601, 601>>(latIndex, lonIndex);
	}
	else
	{
		throw tile_exception(
			"DTED File " + m_filePath.string() +
			" appears to be corrupted. Data record dimensions [" +
			std::to_string(m_metadata.numLatitudeLines()) + "," +
			std::to_string(m_metadata.numLongitudeLines()) +
			"] are invalid.",
			__FILE__, __LINE__);
	}
}