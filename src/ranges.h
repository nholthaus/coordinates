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
// Strongly-typed distances. A slant range (straight line from an observer to a target), a geodesic
// distance (along the curved surface of the ellipsoid), and a Euclidean distance (the 3-D straight-line
// magnitude of a difference vector) are all lengths, yet they measure different paths and must not be
// silently interchanged -- a slant range is not a surface distance, and neither is a coordinate height.
// Each is a `units::kind` tag so the type system keeps them apart: mixing two different range kinds is a
// compile error. The tags are boundary-permissive: a plain length constructs into a range implicitly and
// `.to<PlainUnit>()` unwraps.
//
//--------------------------------------------------------------------------------------------------

#ifndef ranges_h
#define ranges_h

//------------------------
//	INCLUDES
//------------------------

#include <units.h>
#include <units/kind.h>

inline namespace coordinates
{
	namespace ranges
	{
		/// A slant range: the straight-line distance from an observer origin to a target (the range
		/// component of an azimuth/elevation/range position).
		using Slant = units::kind<"slant_range", units::length::meters<double>>;

		/// A geodesic distance: the shortest distance along the curved surface of the reference ellipsoid
		/// between two points (the result of an inverse geodesic solve).
		using Geodesic = units::kind<"geodesic_distance", units::length::meters<double>>;

		/// A Euclidean distance: the 3-D straight-line magnitude of the difference between two Cartesian
		/// positions (the result of `distance` / `magnitude`).
		using Euclidean = units::kind<"euclidean_distance", units::length::meters<double>>;
	}    // namespace ranges
}    // namespace coordinates

#endif    // ranges_h
