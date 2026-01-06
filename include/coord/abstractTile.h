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

#ifndef AbstractTile_h__
#define AbstractTile_h__

//------------------------
//	INCLUDES
//------------------------
#include "tileMetadata.h"

namespace coord
{
	/**
	* @brief		namespace for implementation details relating to topography
	*/
	namespace topography
	{

		//	----------------------------------------------------------------------------
		//	CLASS		AbstractTile
		//  ----------------------------------------------------------------------------
		///	@brief		Abstract interface for tile-manager classes.
		///	@details	Tile classes are the primary interface for classes
		///				implementing the topography concept to access external terrain
		///				data stored in multiple files. There is an implicit assumption
		///				that the tiles are square (in terms of lat/lon). Additionally, it
		///				is assumed that a higher-level AbstractTileManager class will handle
		///				coordinate access to a collection of tiles.
		//  ----------------------------------------------------------------------------
		class AbstractTile
		{
		public:

			using degree_t = units::angle::degrees<double>;
			using meters = units::length::meters<double>;

		public:

			virtual ~AbstractTile() {};

			/**
			* @brief		returns metadata about the tile object containing <i>latitude</i> and <i>longitude</i>
			* @details
			* @returns		TileMetadata object with information about the tile containing <i>latitude</i> and <i>longitude</i>
			*/
			virtual TileMetadata metadata() const = 0;

			/**
			* @brief		checks whether the tile object containing <i>latitude</i> and <i>longitude</i> is in memory
			* @details
			* @returns		true if the tile is already loaded into memory, false otherwise.
			*/
			virtual bool isLoaded() const = 0;

			/**
			 * @brief		load tile data from disc.
			 * @details		Explicitly forces tile data to load from disc. This should not need to be
			 *				called by users since the other tile functions like `metadata()` and `elevation` 
			 *				will load the data if required.
			 * @returns		returns true on success, false on failure or if the tile does not have a
			 *				valid data source (i.e. if it was default constructed)
			 */
			virtual bool load() const = 0;

			/**
			 * @brief		clear tile data from memory.
			 * @details		Explicitly forces tile data to free any allocated memory. This should not
			 *				need to be called by users since the tile manage their own memory automatically.
			 * @returns		returns true on success, false on failure. If the tile had no allocated 
			 *				memory when clear is called, it will return true.
			 */
			virtual bool clear() const = 0;

			/**
			* @brief		return elevation at the given  <i>latitude</i> and <i>longitude</i>
			* @details		The tile manager takes care of the details of loading and querying the
			*				correct tile for the given point.
			* @param[in]	latitude	latitude at which to find the elevation
			* @param[in]	longitude	longitude at which to find the elevation
			* @throws		std::runtime_error if the lat/lon are out of bounds of the tile.
			* @returns		elevation of ground level at the given coordinates in meters
			*/
			virtual meters elevation(const degree_t& latitude, const degree_t& longitude) const = 0;

			/**
			 * @brief		returns elevation at the given indices
			 * @details		throws tile_out_of_bounds if the indices are not valid. This method does
			 *				not perform any interpolation.
			 * @param[in]	latitudeIndex	index in data corresponding to latitude axis
			 * @param[in]	longitudeIndex	index in data corresponding to longitude axis
			 * @returns		elevation (meters) at the indices provided.
			 */
			virtual meters elevation(size_t latitudeIndex, size_t longitudeIndex) const = 0;

			/**
			 * @brief		returns the width of the tile in degrees.
			 * @details		The extent of the titles longitude (width) in decimal degrees. This represents
			 *				the full extent of the tile, but does *not* include overlap in tiles. I.e.
			 *				if a tile is 1.08333 degrees wide from from 12.91777 to 14.083333, the width
			 *				would be reported as 1 degree. The main intended use of this function is
			 *				to help tile manager classes determine how many tiles need to be allocated.
			 * @note		when deriving from AbstractTile, not that `width()` must return a correct
			 *				value even if the tile does not exist on disc or has not been loaded into
			 *				memory. 
			 * @returns		extent of tile longitude in degrees.
			 */
			virtual degree_t width() const = 0;

			/**
			* @brief		returns the height of the tile in degrees.
			* @details		The extent of the titles latitude (height) in decimal degrees. This represents
			*				the full extent of the tile, but does *not* include overlap in tiles. I.e.
			*				if a tile is 1.08333 degrees tall from from 12.91777 to 14.083333, the height
			*				would be reported as 1 degree. The main intended use of this function is
			*				to help tile manager classes determine how many tiles need to be allocated.
			* @note			when deriving from AbstractTile, not that `height()` must return a correct
			*				value even if the tile does not exist on disc or has not been loaded into
			*				memory.
			* @returns		extent of tile longitude in degrees.
			*/
			virtual degree_t height() const = 0;

		};
	}
}


#endif // AbstractTile_h__