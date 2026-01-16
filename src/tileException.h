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
///	@file			tileException.h
///	@brief			Exception definitions for terrain tile handling errors
/// @details		All exceptions are derived from std::runtime_error
//
//--------------------------------------------------------------------------------------------------

#ifndef tileException_h
#define tileException_h

//------------------------
//	INCLUDES
//------------------------

#include <stdexcept>
#include <string>

inline namespace coordinates
{
	inline namespace topography
	{
		//	----------------------------------------------------------------------------
		//	CLASS		tile_out_of_bounds
		//  ----------------------------------------------------------------------------
		///	@brief		thrown when information is requested from a tile which is outside
		///				of the boundaries of the data stored by the tile.
		///	@details	e.g. The lat/lon requested is outside the tile.
		//  ----------------------------------------------------------------------------
		class tile_out_of_bounds : public std::runtime_error
		{
		public:

			explicit tile_out_of_bounds(const std::string& msg, std::string, unsigned int)
			: 
			std::runtime_error(msg) {};

		};

		//	----------------------------------------------------------------------------
		//	CLASS		tile_invalid
		//  ----------------------------------------------------------------------------
		///	@brief		Thrown when the data for the requested tile is corrupted or doesn't
		///				exist.
		///	@details	e.g. Requested tile file has the wrong size, or requested tile
		///				file cannot be found on disc
		//  ----------------------------------------------------------------------------
		class tile_invalid : public std::runtime_error
		{
		public:

			explicit tile_invalid(const std::string& msg, const std::string&, unsigned int)
			: 
			std::runtime_error(msg) {};

		};

		//	----------------------------------------------------------------------------
		//	CLASS		tile_exception
		//  ----------------------------------------------------------------------------
		///	@brief		generic errors which do not fit other exception categories
		///	@details	e.g. unexpected value parsed from a DTED file
		//  ----------------------------------------------------------------------------
		class tile_exception : public std::runtime_error
		{
		public:

			explicit tile_exception(const std::string& msg, const std::string&, unsigned int)
			: 
			std::runtime_error(msg) {};

		};
	}
}

#endif // tileException_h