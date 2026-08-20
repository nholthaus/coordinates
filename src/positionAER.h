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
// `PositionAER<Datum, AzElUnits, RangeUnits>` is an alias template for `Coordinate<AERFrame<Datum>,
// tuple<...>>`: a locally-level azimuth/elevation/range look-angle position pinned to a geodetic origin. The
// tagged azimuth/elevation/range accessors and the `fromObserver` factory come from the AER `AxisAccessors`
// specialization; the origin-carrying constructors, `origin()`/`setOrigin()`, and the ENU/NED->AER trig fast
// paths come from the shared `Coordinate` body's local-frame surface and the `convert_fast_path` hook. This
// header exists so consumers can `#include "positionAER.h"` and get the type.
//
//--------------------------------------------------------------------------------------------------

#ifndef positionAER_h__
#define positionAER_h__

//------------------------
//	INCLUDES
//------------------------

#include "coordinate.h"
#include "coordinates_fwd.h"

#endif    // positionAER_h__
