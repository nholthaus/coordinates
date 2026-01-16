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
///	@file			dtedTile.h
///	@brief			Implementation of DTED 1x1 tiles
/// @details
//
//--------------------------------------------------------------------------------------------------

#ifndef dtedTile_h
#define dtedTile_h

//------------------------
//	INCLUDES
//------------------------

#include "abstractTile.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

inline namespace coordinates
{
	inline namespace topography
	{
		//	----------------------------------------------------------------------------
		//	CLASS		DTEDTile
		//  ----------------------------------------------------------------------------
		///	@brief		implementation of a DTED data tile
		///	@details	1 degree x 1 degree extent
		//  ----------------------------------------------------------------------------
		class DTEDTile : public AbstractTile
		{
		public:

			DTEDTile(std::filesystem::path filePath = {});
			explicit DTEDTile(std::string path);
			explicit DTEDTile(const char* path);

			/**
			 * @brief		Access tile metadata
			 * @details		loads the tile from disc if it isn't already in memory.
			 * @returns		metadata for this tile
			 */
			TileMetadata metadata() const override;

			/**
			 * @brief		tile is loaded into memory
			 * @details
			 * @returns		true if loaded in memory, false otherwise.
			 */
			bool isLoaded() const override;

			/**
			 * @brief		load tile data from disc.
			 * @details		Explicitly forces tile data to load from disc. This should not need to be
			 *				called by users since the other tile functions like `metadata()` and `elevation`
			 *				will load the data if required.
			 * @returns		returns true on success, false on failure or if the tile does not have a
			 *				valid data source (i.e. if it was default constructed)
			 */
			bool load() const override;

			/**
			 * @brief		clear tile data from memory.
			 * @details		Explicitly forces tile data to free any allocated memory. This should not
			 *				need to be called by users since the tile manage their own memory automatically.
			 * @returns		returns true on success, false on failure. If the tile had no allocated
			 *				memory when clear is called, it will return true.
			 */
			bool clear() const override;

			/**
			 * @brief		elevation at location
			 * @details
			 * @param[in]	latitude	latitude of location at which to find terrain elevation.
			 * @param[in]	longitude	longitude of location at which to find terrain elevation.
			 * @returns		elevation, in meters, at the given lat/lon coordinate
			 */
			units::meters<> elevation(const units::degrees<>& latitude, const units::degrees<>& longitude) const override;

			/**
			 * @brief		returns elevation at the given indices
			 * @details		throws tile_out_of_bounds if the indices are not valid. This method does
			 *				not perform any interpolation.
			 * @param[in]	latitudeIndex	index in data corresponding to latitude axis
			 * @param[in]	longitudeIndex	index in data corresponding to longitude axis
			 * @returns		elevation (meters) at the indices provided.
			 */
			units::meters<> elevation(size_t latitudeIndex, size_t longitudeIndex) const override;

			/**
			* @brief		returns the width of the tile in degrees.
			* @details		For DTED data, this is always 1 degree.
			* @returns		extent of tile longitude in degrees.
			*/
			units::degrees<> width() const override;

			/**
			* @brief		returns the height of the tile in degrees.
			* @details		For DTED data, this is always 1 degree.
			* @returns		extent of tile longitude in degrees.
			*/
			units::degrees<> height() const override;

			/**
			 * @brief		Equality
			 */
			friend bool operator==(const DTEDTile& lhs, const DTEDTile& rhs)
			{
				return (lhs.m_filePath == rhs.m_filePath);
			}

			/**
			 * @brief		Inequality
			 */
			friend bool operator!=(const DTEDTile& lhs, const DTEDTile& rhs)
			{
				return !(lhs == rhs);
			}

		protected:

			/**
			 * @brief		creates decimal degree object from a DTED formated string [DDMMSSH, DDDMMSSH, or DDDMMSS.SH]
			 * @details
			 * @param[in]	ptr		location of the start of the ascii byte array
			 * @param[in]	length	size of the ascii byte array
			 * @returns		value in degrees if it matches the Regex, NAN otherwise
			 */
			virtual units::degrees<> decimalDegreeFromDtedDegree(const char ptr[], int length) const;

			/**
			 * @brief		creates decimal degree object from a DTED formatted string [SSSS]
			 * @details
			 * @param[in]	ptr		location of the start of the ascii byte array
			 * @param[in]	length	size of the ascii byte array
			 * @returns		value in degrees if it matches the regex, NAN otherwise
			 */
			virtual units::degrees<> decimalDegreeFromDtedTenthsec(const char ptr[], int length) const;

			/**
			 * @brief		returns elevation values (meters) from a pair of DTED file indices
			 * @details
			 * @param[in]	latitudeIndex	index of the latitude in the DTED file.
			 * @param[in]	longitudeIndex	index of the longitude in the DTED file.
			 * @returns		elevation (meters)
			 */
			template <typename DTEDFileMask>
			int16_t elevationFromIndex(size_t latitudeIndex, size_t longitudeIndex) const
			{
				// table look-up
				const auto&   be  = reinterpret_cast<DTEDFileMask*>(m_data.data())->dataRecord[longitudeIndex].elevation[latitudeIndex];
				const auto*   raw = reinterpret_cast<const std::uint8_t*>(&be);
				const int16_t e   = static_cast<int16_t>((static_cast<std::uint16_t>(raw[0]) << 8u) | static_cast<std::uint16_t>(raw[1]));

				// elevation is stored as 16-bit big-endian int, where MSB is a sign bit (0 = positive, 1 = negative)
				const bool signBit = static_cast<bool>(e & 0x8000);
				// true is guaranteed == 1 by the c++ standard: http://stackoverflow.com/questions/2725044/can-i-assume-booltrue-int1-for-any-c-compiler
				const int16_t elevation_m = (e & 0x7FFF);
				if (signBit)
					return -elevation_m;

				return elevation_m;
			}

			/**
			 * @brief		untemplated overload of `elevationFromIndex()`
			 * @details
			 * @param[in]	latitudeIndex	index of the latitude in the DTED file.
			 * @param[in]	longitudeIndex	index of the longitude in the DTED file.
			 * @returns		elevation (meters)
			 */
			int16_t elevationFromIndex(size_t latitudeIndex, size_t longitudeIndex) const;

		protected:

			const std::filesystem::path    m_filePath;
			mutable std::vector<std::byte> m_data; ///< The raw dted data directly from the file. Mutable since it is effectively a cache.
			mutable TileMetadata           m_metadata;
		};
	}
}

#endif // dtedTile_h