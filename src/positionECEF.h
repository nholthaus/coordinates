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
// `PositionECEF<Datum, Units>` is an alias template for `Coordinate<ECEFFrame<horizontal_datum>,
// tuple<Units, Units, Units>>`: an Earth-centered, Earth-fixed (x, y, z) position. The x/y/z accessors and
// the ellipsoid/line-of-sight verbs (`intersectRay`, `hasLineOfSightTo`) come from the ECEF
// `AxisAccessors` specialization; everything else is the shared `Coordinate` body. This header exists so
// consumers can `#include "positionECEF.h"` and get the type plus its full body.
//
//--------------------------------------------------------------------------------------------------

#ifndef pointECEF_h__
#define pointECEF_h__

//------------------------
//	INCLUDES
//------------------------

#include "coordinate.h"
#include "coordinates_fwd.h"

#endif    // pointECEF_h__
