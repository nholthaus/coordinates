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
///	@file			dtedTileManager.h
///	@brief			Tile manager specific to loading/reading DTED files
/// @details
//
//--------------------------------------------------------------------------------------------------

#ifndef dtedTileManager_h
#define dtedTileManager_h

//------------------------
//	INCLUDES
//------------------------

#include "abstractTileManager.h"
#include "dtedTile.h"

#include <cache.h>

#include <filesystem>
#include <list>
#include <string>
#include <vector>

inline namespace coordinates
{
	inline namespace topography
	{
		//	----------------------------------------------------------------------------
		//	CLASS		DTEDTileManager
		//  ----------------------------------------------------------------------------
		///	@brief		Tile manager standard tiles.
		///	@details	This tile manager can support any type of tiles which are:\n\n
		///				1) Derived from the `AbstractTile` interface.\n
		///				2) Are default constructible.\n
		///				3) Are rectangular.\n
		///				4) Are all the same size (height/width)\n\n
		///				DTED is an example of tiled data which meets all of these criteria.
		///
		//  ----------------------------------------------------------------------------
		class DTEDTileManager : public AbstractTileManager
		{
		public:
			using list_type           = std::list<std::pair<size_t, size_t>>;
			using iterator_type       = list_type::iterator;
			using const_iterator_type = list_type::const_iterator;
			using tile_type           = DTEDTile;

			struct tileRecord
			{
				tileRecord() = default;

				std::filesystem::path filePath{};    ///< Full path to tile data on disk (empty if not present)
				int                   level{-1};     ///< DTED level (0..2). -1 => unset
			};

		public:
			static DTEDTileManager& instance();
			DTEDTileManager(const DTEDTileManager&) = delete;
			DTEDTileManager(DTEDTileManager&&)      = delete;
			~DTEDTileManager() override;

			bool exists(const units::degrees<>& latitude, const units::degrees<>& longitude) const override;
			bool isLoaded(const units::degrees<>& latitude, const units::degrees<>& longitude) const override;

			units::meters<> elevation(const units::degrees<>& latitude, const units::degrees<>& longitude) const override;
			AbstractTile*   tile(const units::degrees<>& latitude, const units::degrees<>& longitude) const override;

			void   setTileLimit(size_t limit) override;
			size_t tileLimit() const override;
			size_t numTiles() const override;
			size_t numLoadedTiles() const override;
			size_t maxNumTiles() const override;

		protected:
			static std::filesystem::path resolve_dted_root();

			static std::vector<std::filesystem::path> searchRecursivelyForFiles(const std::filesystem::path&    rootSearchDir,
			                                                             const std::vector<std::string>& extensions);

		private:
			explicit DTEDTileManager();

			static size_t indexFromLatLon(units::degrees<> latitude, units::degrees<> longitude);

		protected:
			//----------------------------------
			//	PROTECTED MEMBERS
			//----------------------------------

			mutable std::vector<tileRecord> m_tileGrid;
			mutable Cache<size_t, DTEDTile> m_tileCache;
			size_t                          m_numTiles;    ///< Number of tiles found on disc by the tile manager recursive directory search.
		};
	}    // namespace topography
}    // namespace coordinates

#endif    // dtedTileManager_h