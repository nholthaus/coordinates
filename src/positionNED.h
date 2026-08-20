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
// `PositionNED<Datum, DistanceUnits>` is an alias template for `Coordinate<NEDFrame<Datum>, tuple<...>>`: a
// locally-level North-East-Down position pinned to a geodetic origin. The north/east/down accessors come
// from the NED `AxisAccessors` specialization; the origin-carrying constructors, `origin()`/`setOrigin()`,
// and the anchored-vector arithmetic come from the shared `Coordinate` body's local-frame surface (guarded
// by `is_local_frame`). This header exists so consumers can `#include "positionNED.h"` and get the type.
//
//--------------------------------------------------------------------------------------------------

#ifndef positionNED_h__
#define positionNED_h__

//------------------------
//	INCLUDES
//------------------------

#include "coordinate.h"
#include "coordinates_fwd.h"

#endif    // positionNED_h__
