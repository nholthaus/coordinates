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
// An 8-bit-per-channel RGB color, plus `channel`, which converts a [0, 1] intensity to a clamped 8-bit value.
//
//--------------------------------------------------------------------------------------------------

#ifndef color_h
#define color_h

#include <algorithm>
#include <cmath>
#include <cstdint>

inline namespace coordinates
{
	inline namespace topography
	{
		//	----------------------------------------------------------------------------
		//	STRUCT		Color
		//  ----------------------------------------------------------------------------
		///	@brief		An 8-bit-per-channel RGB color for canvas drawing.
		//  ----------------------------------------------------------------------------
		struct Color
		{
			std::uint8_t r{0}, g{0}, b{0};
		};

		/// A [0,1] intensity as an 8-bit channel value (clamped).
		[[nodiscard]] inline std::uint8_t channel(double intensity)
		{
			return static_cast<std::uint8_t>(std::lround(255.0 * std::clamp(intensity, 0.0, 1.0)));
		}
	}    // namespace topography
}    // namespace coordinates

#endif    // color_h
