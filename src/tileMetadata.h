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

#ifndef tileMetadata_h
#define tileMetadata_h

//------------------------
//	INCLUDES
//------------------------
#include <string>
#include <units.h>

inline namespace coordinates
{
	inline namespace topography
	{
		//	----------------------------------------------------------------------------
		//	CLASS		TileMetadata
		//  ----------------------------------------------------------------------------
		///	@brief		contains meta-data about a given topography tile
		///	@details
		//  ----------------------------------------------------------------------------
		class TileMetadata
		{
		public:
			TileMetadata()
			    : m_southwestLatitude(0.0)
			    , m_southwestLongitude(0.0)
			    , m_northeastLatitude(0.0)
			    , m_northeastLongitude(0.0)
			    , m_latitudeResolution(0.0)
			    , m_longitudeResolution(0.0)
			    , m_minElevation(NAN)
			    , m_maxElevation(NAN)
			    , m_meanElevation(NAN)
			    , m_numLatitudeLines(0)
			    , m_numLongitudeLines(0) {};

			TileMetadata(const TileMetadata& other) = default;

			TileMetadata(TileMetadata&& other) noexcept
			    : m_filePath(std::move(other.m_filePath))
			    , m_dataType(std::move(other.m_dataType))
			    , m_southwestLatitude(other.m_southwestLatitude)
			    , m_southwestLongitude(other.m_southwestLongitude)
			    , m_northeastLatitude(other.m_northeastLatitude)
			    , m_northeastLongitude(other.m_northeastLongitude)
			    , m_latitudeResolution(other.m_latitudeResolution)
			    , m_longitudeResolution(other.m_longitudeResolution)
			    , m_minElevation(other.m_minElevation)
			    , m_maxElevation(other.m_maxElevation)
			    , m_meanElevation(other.m_meanElevation)
			    , m_numLatitudeLines(other.m_numLatitudeLines)
			    , m_numLongitudeLines(other.m_numLongitudeLines)
			{
			}

			TileMetadata& operator=(const TileMetadata& other) = default;

			TileMetadata& operator=(TileMetadata&& other) noexcept
			{
				m_filePath            = std::move(other.m_filePath);
				m_dataType            = std::move(other.m_dataType);
				m_southwestLatitude   = other.m_southwestLatitude;
				m_southwestLongitude  = other.m_southwestLongitude;
				m_northeastLatitude   = other.m_northeastLatitude;
				m_northeastLongitude  = other.m_northeastLongitude;
				m_latitudeResolution  = other.m_latitudeResolution;
				m_longitudeResolution = other.m_longitudeResolution;
				m_minElevation        = other.m_minElevation;
				m_maxElevation        = other.m_maxElevation;
				m_meanElevation       = other.m_meanElevation;
				m_numLatitudeLines    = other.m_numLatitudeLines;
				m_numLongitudeLines   = other.m_numLongitudeLines;
				return *this;
			}

			[[nodiscard]] std::string dataType() const { return m_dataType; }
			[[nodiscard]] std::string filePath() const { return m_filePath; }

			[[nodiscard]] units::degrees<double> northeastLatitude() const { return m_northeastLatitude; }
			[[nodiscard]] units::degrees<double> northeastLongitude() const { return m_northeastLongitude; }
			[[nodiscard]] units::degrees<double> latitudeResolution() const { return m_latitudeResolution; }
			[[nodiscard]] units::degrees<double> longitudeResolution() const { return m_longitudeResolution; }
			[[nodiscard]] units::degrees<double> southwestLatitude() const { return m_southwestLatitude; }
			[[nodiscard]] units::degrees<double> southwestLongitude() const { return m_southwestLongitude; }
			[[nodiscard]] units::meters<double>  maxElevation() const { return m_maxElevation; }
			[[nodiscard]] units::meters<double>  meanElevation() const { return m_meanElevation; }
			[[nodiscard]] units::meters<double>  minElevation() const { return m_minElevation; }

			[[nodiscard]] size_t numLatitudeLines() const { return m_numLatitudeLines; }
			[[nodiscard]] size_t numLongitudeLines() const { return m_numLongitudeLines; }

			void setDataType(const std::string& val) { m_dataType = val; }
			void setDataType(std::string&& val) { m_dataType = std::move(val); }
			void setFilePath(const std::string& val) { m_filePath = val; }
			void setFilePath(std::string&& val) { m_filePath = std::move(val); }

			void setMaxElevation(const units::length::meters<double>& val) { m_maxElevation = val; }
			void setMeanElevation(const units::length::meters<double>& val) { m_meanElevation = val; }
			void setMinElevation(const units::length::meters<double>& val) { m_minElevation = val; }
			void setNortheastLatitude(const units::angle::degrees<double>& val) { m_northeastLatitude = val; }
			void setNortheastLongitude(const units::angle::degrees<double>& val) { m_northeastLongitude = val; }
			void setLatitudeResolution(const units::angle::degrees<double>& val) { m_latitudeResolution = val; }
			void setLongitudeResolution(const units::angle::degrees<double>& val) { m_longitudeResolution = val; }
			void setSouthwestLatitude(const units::angle::degrees<double>& val) { m_southwestLatitude = val; }
			void setSouthwestLongitude(const units::angle::degrees<double>& val) { m_southwestLongitude = val; }

			void setNumLatitudeLines(const size_t& val) { m_numLatitudeLines = val; }
			void setNumLongitudeLines(const size_t& val) { m_numLongitudeLines = val; }

		protected:
			std::string m_filePath;    ///< Path to the file which contains the terrain data for this tile.
			std::string m_dataType;    ///< Human-readable string containing the type of data the tile represents, e.g. "DTED Level-2". Beware: these strings
			                           ///< may not be consistent and shouldn't be used for indexing.

			units::angle::degrees<double> m_southwestLatitude;     ///< Latitude (decimal degrees, datum unknown) of the tile's southwest corner.
			units::angle::degrees<double> m_southwestLongitude;    ///< Longitude (decimal degrees, datum unknown) of the tile's southwest corner.
			units::angle::degrees<double> m_northeastLatitude;     ///< Latitude (decimal degrees, datum unknown) of the tile's northeast corner.
			units::angle::degrees<double> m_northeastLongitude;    ///< Longitude (decimal degrees, datum unknown) of the tile's northeast corner.

			units::angle::degrees<double> m_latitudeResolution;     ///< angle (decimal degrees) between latitude elevation data points in the tile. Values <= 0
			                                                        ///< or NAN represent unknown resolutions.
			units::angle::degrees<double> m_longitudeResolution;    ///< angle (decimal degrees) between longitude elevation data points in the tile. Values <=
			                                                        ///< 0 or NAN represent unknown resolutions.

			units::length::meters<double> m_minElevation;     ///< Minimum elevation within the tile. NAN values represent invalid/unknown.
			units::length::meters<double> m_maxElevation;     ///< Maximum elevation within the tile. NAN values represent invalid/unknown.
			units::length::meters<double> m_meanElevation;    ///< Average elevation value of the tile. NAN values represent invalid/unknown.

			size_t m_numLatitudeLines;     ///< Number of latitude lines (rows or columns) in the data.
			size_t m_numLongitudeLines;    ///< Number of longitude lines (rows or columns) in the data.
		};
	}    // namespace topography
}    // namespace coordinates

#endif    // tileMetadata_h