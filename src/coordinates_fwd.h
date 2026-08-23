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

#ifndef COORDINATES_FWD_H
#define COORDINATES_FWD_H

//------------------------
//	INCLUDES
//------------------------

#include <tuple>

#include <units.h>

#include "frameOfReference.h"

//------------------------
//	FORWARD DECLARATIONS
//------------------------

inline namespace coordinates
{
	using namespace units::length;
	using namespace units::angle;

	// The one primary template every position type is an alias of. Forward-declared here so a consumer that
	// only names a position by value (algorithm.h, the vectors, the tests) needs no definition; the body
	// lives in coordinate.h. An alias template to a forward-declared class template is well-formed C++.
	template<class Frame, class Tuple, class FrameDataType = FrameData>
	class Coordinate;

	/// A geodetic (latitude, longitude, altitude) position in `Datum`, stored in the chosen angle/length units.
	template<class Datum, template<class> class AngleUnits = degrees, template<class> class HeightUnits = meters>
	using PositionGeodetic =
	        Coordinate<coordinateFrames::Geodetic3DFrame<Datum>, std::tuple<AngleUnits<double>, AngleUnits<double>, HeightUnits<double>>, FrameData>;

	/// An Earth-centered, Earth-fixed (x, y, z) position in `Datum`, stored in the chosen length unit.
	template<class Datum, template<class> class Units = meters>
	using PositionECEF =
	        Coordinate<coordinateFrames::ECEFFrame<typename traits::datum_traits<Datum>::horizontal_datum>, std::tuple<Units<double>, Units<double>, Units<double>>, FrameData>;

	/// A locally-level East-North-Up position in `Datum`, pinned to a geodetic origin, in the chosen length unit.
	template<class Datum, template<class> class DistanceUnits = meters>
	using PositionENU =
	        Coordinate<coordinateFrames::ENUFrame<Datum>, std::tuple<DistanceUnits<double>, DistanceUnits<double>, DistanceUnits<double>>, FrameData>;

	/// A locally-level North-East-Down position in `Datum`, pinned to a geodetic origin, in the chosen length unit.
	template<class Datum, template<class> class DistanceUnits = meters>
	using PositionNED =
	        Coordinate<coordinateFrames::NEDFrame<Datum>, std::tuple<DistanceUnits<double>, DistanceUnits<double>, DistanceUnits<double>>, FrameData>;

	/// A locally-level azimuth-elevation-range look-angle position in `Datum`, pinned to a geodetic origin.
	template<class Datum, template<class> class AzElUnits = degrees, template<class> class RangeUnits = meters>
	using PositionAER =
	        Coordinate<coordinateFrames::AERFrame<Datum>, std::tuple<AzElUnits<double>, AzElUnits<double>, RangeUnits<double>>, FrameData>;
}    // namespace coordinates

#endif    // COORDINATES_FWD_H