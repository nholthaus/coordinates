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
///	@file			GEOID12ALUT.h
///	@brief			Look-up table for the USGS GEOID12A Hybrid Geoid corrections.
/// @details		This is just the g2012au0.asc file from http://www.ngs.noaa.gov/GEOID/GEOID12A/GEOID12A_data.shtml
///					reformated from a data file into a vector. This geoid is used to convert from NAD83
///					to NAVD88.
//
//--------------------------------------------------------------------------------------------------

#ifndef COORDINATES_GEOID12ALUT_H
#define COORDINATES_GEOID12ALUT_H

//------------------------
//	INCLUDES
//------------------------
#include <array>

//------------------------------
//  EXTERN DECLARATIONS
//------------------------------

/// Precision, in decimal degrees, of the look-up table
extern const double GEOID12ALUT_PRECISION_DEG;

/// Table entries are stored as ints. To get the real value, divide each by the divisor.
extern const double GEOID12ALUT_DIVISOR;

/// Geoid Look-up Table
extern const std::array<std::array<int32_t, 4201>, 2041> GEOID12ALUT;

#endif //COORDINATES_GEOID12ALUT_H