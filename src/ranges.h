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
// Strongly-typed distances -- but ONLY where the reference genuinely differs. A straight-line distance
// through 3-space (the magnitude of a difference vector, whether you call it a Euclidean distance or a
// slant range from an observer) is just a length: it carries no reference surface, so a slant range and a
// Euclidean distance are the SAME kind and are not distinguished. A geodesic distance is different -- it is
// measured ALONG the curved surface of the ellipsoid, so a 1000 m arc is not the same measurement as a
// 1000 m chord; the surface is an implicit reference the chord does not have. Only that genuine difference
// is a distinct `units::kind`, so mixing a surface distance with a straight-line distance is a compile
// error while two straight-line distances interoperate freely. The tags are boundary-permissive: a plain
// length constructs into a range implicitly and `.to<PlainUnit>()` unwraps.
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
		/// A straight-line distance through 3-space: the magnitude of the difference between two positions.
		/// A "slant range" from an observer to a target is exactly this quantity -- it carries no reference
		/// surface, so it is not a distinct kind from a Euclidean distance.
		using Euclidean = units::kind<"euclidean_distance", units::length::meters<double>>;

		/// A geodesic distance: the shortest distance ALONG the curved surface of the reference ellipsoid
		/// between two points (the result of an inverse geodesic solve). Distinct from a straight-line
		/// distance because it measures an arc, not a chord -- the surface is its implicit reference.
		using Geodesic = units::kind<"geodesic_distance", units::length::meters<double>>;
	}    // namespace ranges
}    // namespace coordinates

#endif    // ranges_h
