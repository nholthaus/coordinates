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
// A raster pixel address: a (row, column) integer coordinate into an image, row 0 at the north/top edge and
// column 0 at the west/left edge.
//
//--------------------------------------------------------------------------------------------------

#ifndef pixel_h
#define pixel_h

#include <ratio>

#include <units.h>

// A `pixels` unit in its own dimension, so raster measurements (a focal length, an image extent) are a typed
// quantity rather than a bare double. Defined at namespace `units` scope, as every unit is.
namespace units
{
	namespace dimension
	{
		struct pixel_tag
		{
			static constexpr auto name         = "pixel";
			static constexpr auto abbreviation = "px";
		};
		using pixel = make_dimension<pixel_tag>;
	}    // namespace dimension

	// Compose the unit from the sub-macros the type and the `_px` literal need, omitting UNIT_ADD_CONSTANT: that
	// piece emits a `constexpr px{1.0}` convenience constant nothing references, which a warnings-as-errors build
	// flags as unused.
	UNIT_ADD_STRONG_CONVERSION_FACTOR(raster, pixels, conversion_factor<std::ratio<1>, dimension::pixel>)
	UNIT_ADD_UNIT_DEFINITION(raster, pixels, conversion_factor<std::ratio<1>, dimension::pixel>)
	UNIT_ADD_NAME(raster, pixels, px)
	UNIT_REGISTER_NAMED_CLASS(raster, pixels)
	UNIT_ADD_LITERALS(raster, pixels, px)
	UNIT_ADD_DIMENSION_TRAIT(pixel, Pixels)
}    // namespace units

inline namespace coordinates
{
	inline namespace topography
	{
		//	----------------------------------------------------------------------------
		//	STRUCT		Pixel
		//  ----------------------------------------------------------------------------
		///	@brief		A raster pixel address (row 0 = north edge, column 0 = west edge).
		//  ----------------------------------------------------------------------------
		struct Pixel
		{
			int row{0}, column{0};
		};
	}    // namespace topography
}    // namespace coordinates

#endif    // pixel_h
