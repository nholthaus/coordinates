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
// Strongly-typed heights, and the trait that selects the right one for a datum. An ellipsoidal (HAE) height
// and an orthometric (MSL) height are both lengths but are semantically distinct and must not be silently
// interchanged -- they differ by the geoid undulation, which depends on position. Each is a `units::kind`
// tag (defined in verticalDatum.h) so the type system enforces the distinction: mixing them is a compile
// error, and the only bridge is an explicit conversion. The tags are boundary-permissive: a plain length
// constructs into a height implicitly (so `LLA p(34_deg, -118_deg, 100_m)` still compiles) and
// `.to<PlainUnit>()` unwraps. `kind_for` deduces which height a datum measures from its vertical reference.
//
//--------------------------------------------------------------------------------------------------

#ifndef heights_h
#define heights_h

//------------------------
//	INCLUDES
//------------------------

#include <type_traits>

#include <units.h>
#include <units/kind.h>

#include "datum.h"
#include "heightKinds.h"

inline namespace coordinates
{
	namespace heights
	{
		//------------------------------------------------------------------------------------------------------
		//	The tagged height kinds themselves (`Ellipsoidal`, `Orthometric`, `Undulation`) are defined in
		//	heightKinds.h, at the bottom of the include graph. `kind_for` needs `datum_traits` and so lives
		//	here, above datum.h.
		//------------------------------------------------------------------------------------------------------

		/// The height kind a position stores for a given datum, deduced from the datum's vertical reference: a
		/// datum whose vertical datum is a bare ellipsoid measures height above the ellipsoid (`Ellipsoidal`); a
		/// datum whose vertical datum is a geoid or topography measures height above that surface
		/// (`Orthometric`). This is the single source of truth for what `PositionGeodetic::altitude()` means, so
		/// the height's type always states which reference it is measured against.
		template<class Datum>
		using kind_for = std::conditional_t<
		        coordinates::traits::is_ellipsoid<typename coordinates::traits::datum_traits<Datum>::vertical_datum>,
		        Ellipsoidal,
		        Orthometric>;
	}    // namespace heights
}    // namespace coordinates

#endif    // heights_h
