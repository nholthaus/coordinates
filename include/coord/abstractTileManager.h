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

#ifndef abstractTileManager_h__
#define abstractTileManager_h__

//------------------------
//	INCLUDES
//------------------------
#include "abstractTile.h"

namespace coord
{
	/**
	 * @brief		namespace for implementation details relating to topography
	 */
	namespace topography
	{
		//	----------------------------------------------------------------------------
		//	CLASS		AbstractTileManager
		//  ----------------------------------------------------------------------------
		///	@brief		Abstract interface for tile-manager classes.
		///	@details	Tile manager classes are the primary interface for classes
		///				implementing the topography concept to access external terrain
		///				data stored in multiple files. There is an implicit assumption
		///				that the tiles are square (in terms of lat/lon).
		//  ----------------------------------------------------------------------------
		class AbstractTileManager
		{
		public:

			using degrees = units::angle::degrees<double>;
			using meters = units::length::meters<double>;

		public:

			virtual ~AbstractTileManager() {};

			/**
			 * @brief		checks whether the tile object containing <i>latitude</i> and <i>longitude</i> exists.
			 * @details		An existing tile may or may not be loaded into memory. A tile exists if
			 *				it's data can be found by the tile manager (most likely by searching a
			 *				location on disc). If data for a particular tile cannot be found or accessed
			 *				by the tile manager, then it will return false.
			 * @param[in]	latitude	latitude inside the tile of interest.
			 * @param[in]	longitude	longitude inside the tile or interest.
			 * @returns		true if the tile exists, false otherwise.
			 */
			virtual bool exists(const degrees& latitude, const degrees& longitude) const = 0;
			/**
			 * @brief		checks whether the tile object containing <i>latitude</i> and <i>longitude</i> is in memory
			 * @details		Not all tiles that exist will be loaded into memory at any given time.
			 * @param[in]	latitude	latitude inside the tile of interest.
			 * @param[in]	longitude	longitude inside the tile or interest.
			 * @returns		true if the tile is already loaded into memory, false otherwise.
			 */
			virtual bool isLoaded(const degrees& latitude, const degrees& longitude) const = 0;

			/**
			 * @brief		return elevation at the given  <i>latitude</i> and <i>longitude</i>
			 * @details		The tile manager takes care of the details of loading and querying the
			 *				correct tile for the given point. If no elevation data exists for the
			 *				coordinate pair, sea level (0m) will be returned.
			 * @param[in]	latitude	latitude at which to find the elevation
			 * @param[in]	longitude	longitude at which to find the elevation
			 * @returns		elevation of ground level at the given coordinates in meters
			 */
			virtual meters elevation(const degrees& latitude, const degrees& longitude) const = 0;

			/**
			 * @brief		pointer to tile at given coordinates
			 * @details		returns a pointer to the underlying tile that contains the given coordinates.
			 *				Tile metadata can be accessed through the tile pointer.
			 * @param[in]	latitude	latitude in tile
			 * @param[in]	longitude	longitude in tile
			 * @returns		pointer to the tile containing [lat, lon].
			 */
			virtual AbstractTile* tile(const degrees& latitude, const degrees& longitude) const = 0;

			/**
			 * @brief		Number of tiles found.
			 * @returns		The number of tiles that the tile manager was able to find.
			 */
			virtual size_t numTiles() const = 0;

			/**
			 * @brief		Number of tiles currently loaded.
			 * @returns		The number of tiles that the tile manager currently has loaded into memory.
			 */
			virtual size_t numLoadedTiles() const = 0;

			/**
			 * @brief		Maximum number of tiles.
			 * @details		The number of tiles that it would take to represent the entire planet, based
			 *				on the tile extent.
			 * @returns		Maximum number of tiles.
			 */
			virtual size_t maxNumTiles() const = 0;

			/**
			 * @brief		set maximum number of tiles that can be loaded at once.
			 * @details		This function can be used to control the maximum amount of memory that
			 *				the tile manager will use. Reducing the tile limit may cause tiles currently
			 *				in memory to be unloaded.
			 * @note		setting this value too low may cause severe performance degradation.
			 * @param[in]	limit	maximum number of tiles.
			 */
			virtual void setTileLimit(size_t limit) = 0;

			/**
			 * @brief		Tile limit.
			 * @details		defaults to `std::numeric_limits<size_t>::max()`.
			 * @returns		max number of tiles that can be loaded at a given time.
			 */
			virtual size_t tileLimit() const = 0;

		};
	}
}


#endif // abstractTileManager_h__