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
// The tagged height kinds, alone. An ellipsoidal (HAE) and an orthometric (MSL) height are both lengths but
// are semantically distinct and must not be silently interchanged; they differ by the geoid undulation. Each
// is a `units::kind` tag so mixing them is a compile error. These live in their own header, depending only on
// `units`, because they sit at the very bottom of the include graph -- both the topography/geoid producers
// (which return an orthometric height) and the vertical-datum machinery (which converts between them) need the
// tags, and neither may pull in the heavier datum/frame headers. The datum-derived selector `heights::kind_for`
// lives in `heights.h`, above `datum.h`.
//
//--------------------------------------------------------------------------------------------------

#ifndef heightKinds_h
#define heightKinds_h

//------------------------
//	INCLUDES
//------------------------

#include <units.h>
#include <units/kind.h>

inline namespace coordinates
{
	/// Type-tagged vertical-height values. Both an ellipsoidal (HAE) and an orthometric (MSL) height are
	/// lengths, so nothing at the type level stops them being added or interchanged -- yet doing so without the
	/// geoid-undulation correction is a datum error. These `units::kind` tags make the two distinct types:
	/// mixing them, or using one where the other is expected, is a compile error, and the ONLY bridge is
	/// `convertToEllipsoidHeight` / `convertFromEllipsoidHeight`. (Distinct from the `traits::OrthometricHeight`
	/// concept, which describes a *type that can produce* an orthometric height.)
	namespace heights
	{
		/// A height above the reference ellipsoid (HAE / geometric height, e.g. what GPS reports).
		using Ellipsoidal = units::kind<"ellipsoidal_height", units::length::meters<double>>;

		/// A height above the geoid (orthometric / mean-sea-level height, e.g. what a map or DTED reports).
		using Orthometric = units::kind<"orthometric_height", units::length::meters<double>>;
	}    // namespace heights
}    // namespace coordinates

#endif    // heightKinds_h
