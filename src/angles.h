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
// Strongly-typed angles. Latitude, longitude, azimuth, elevation, and the three orientation angles are
// all plain angles, yet they are semantically distinct and must not be silently interchanged -- a latitude
// and an azimuth are both `degrees<>`, so nothing at the type level stops an azimuth of 315 degrees being
// consumed as a latitude of 315 degrees. Each is a `units::kind` tag so the type system keeps them apart:
// mixing two different angle kinds is a compile error. The tags are boundary-permissive: a plain angle
// constructs into an angle kind implicitly (so `LLA p(34_deg, -118_deg, 100_m)` still compiles) and
// `.to<PlainUnit>()` unwraps.
//
//--------------------------------------------------------------------------------------------------

#ifndef angles_h
#define angles_h

//------------------------
//	INCLUDES
//------------------------

#include <units.h>
#include <units/kind.h>

inline namespace coordinates
{
	namespace angles
	{
		/// A geodetic latitude: the angle from the equatorial plane to the ellipsoid normal, in [-90, 90].
		/// This is the latitude reported by GPS and stored in a geodetic position; it is the canonical latitude
		/// kind, from which the others are derived.
		using Latitude = units::kind<"latitude", units::angle::degrees<double>>;

		/// A geocentric latitude: the angle from the equatorial plane to the line joining the point and the
		/// centre of the ellipsoid, in [-90, 90]. It differs from the geodetic latitude by up to ~0.19 deg
		/// (mid-latitudes on Earth); the two relate through the ellipsoid's eccentricity. Convert with
		/// `convertLatitude` -- mixing a geocentric and a geodetic latitude is otherwise a compile error.
		using Geocentric = units::kind<"geocentric_latitude", units::angle::degrees<double>>;

		/// A longitude: the angle east of the prime meridian.
		using Longitude = units::kind<"longitude", units::angle::degrees<double>>;

		/// A horizontal azimuth measured clockwise from North, in [0, 360). Unifies the AER azimuth
		/// (observer to target), a geodesic forward/back bearing (path direction), and a sun azimuth --
		/// all the same physical measurement.
		using Azimuth = units::kind<"azimuth", units::angle::degrees<double>>;

		/// An elevation angle above the local horizon, in [-90, 90].
		using Elevation = units::kind<"elevation", units::angle::degrees<double>>;

		/// A yaw (heading) orientation angle about the body Z axis.
		using Yaw = units::kind<"yaw", units::angle::degrees<double>>;

		/// A pitch orientation angle about the body Y axis.
		using Pitch = units::kind<"pitch", units::angle::degrees<double>>;

		/// A roll orientation angle about the body X axis.
		using Roll = units::kind<"roll", units::angle::degrees<double>>;
	}    // namespace angles
}    // namespace coordinates

#endif    // angles_h
