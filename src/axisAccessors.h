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
// The per-frame named axis accessors, injected into `Coordinate` by inheritance. `Coordinate<Frame, Tuple>`
// is one body, but a caller still writes `ecef.x()`, `lla.latitude()`, `aer.azimuth()` -- member syntax, a
// different name and return type per frame. `AxisAccessors<Frame, Derived>` supplies exactly those members
// for `Frame`, reading/writing the derived coordinate's tuple through `point()`/`setPoint()`. It is a
// CRTP-lite mixin: the primary template is empty (a frame with no named axes adds nothing), and each frame
// specializes it. This keeps the accessor names -- the only genuinely per-frame code -- out of the shared
// `Coordinate` body while preserving the exact member-call surface every caller already uses.
//
//--------------------------------------------------------------------------------------------------

#ifndef axisAccessors_h
#define axisAccessors_h

//------------------------
//	INCLUDES
//------------------------

#include <tuple>

#include <units.h>

#include "angles.h"
#include "frameAxes.h"
#include "frameOfReference.h"
#include "heights.h"
#include "latitudeConversion.h"
#include "ranges.h"
#include "verticalDatum.h"

inline namespace coordinates
{
	using namespace units;

	inline namespace traits
	{
		//	----------------------------------------------------------------------------
		//	AxisAccessors<Frame, Derived>
		//  ----------------------------------------------------------------------------
		///	@brief		The named axis accessors a coordinate in `Frame` exposes, mixed in by inheritance.
		///	@details	Empty by default; each frame specializes it to add its `x()/y()/z()`,
		///				`latitude()/longitude()/altitude()`, etc. `Derived` is the `Coordinate` that owns the
		///				tuple; accessors read `self().point()` and write `self().setPoint(...)`.
		///	@tparam		Frame	the frame of reference.
		///	@tparam		Derived	the CRTP-derived coordinate type.
		//  ----------------------------------------------------------------------------
		template<class Frame, class Derived>
		struct AxisAccessors
		{
		};

		//------------------------------------------------------------------------------------------------------
		//	Shared CRTP plumbing: derive an accessor mixin from AxisAccessorBase to get self()/get<I>()/set<I>().
		//------------------------------------------------------------------------------------------------------
		template<class Derived>
		struct AxisAccessorBase
		{
		protected:
			[[nodiscard]] const Derived& self() const { return static_cast<const Derived&>(*this); }

			template<std::size_t I>
			[[nodiscard]] auto slot() const
			{
				return std::get<I>(self().point());
			}

			template<std::size_t I, class Value>
			void setSlot(const Value& value)
			{
				auto point       = static_cast<Derived&>(*this).point();
				std::get<I>(point) = value;
				static_cast<Derived&>(*this).setPoint(point);
			}
		};

		//------------------------------------------------------------------------------------------------------
		//	ECEF: x / y / z (raw lengths).
		//------------------------------------------------------------------------------------------------------
		template<class HorizontalDatum, class Derived>
		struct AxisAccessors<coordinateFrames::ECEFFrame<HorizontalDatum>, Derived> : AxisAccessorBase<Derived>
		{
			using base = AxisAccessorBase<Derived>;
			[[nodiscard]] auto x() const { return base::template slot<0>(); }
			[[nodiscard]] auto y() const { return base::template slot<1>(); }
			[[nodiscard]] auto z() const { return base::template slot<2>(); }
			void               setX(const meters<>& v) { base::template setSlot<0>(v); }
			void               setY(const meters<>& v) { base::template setSlot<1>(v); }
			void               setZ(const meters<>& v) { base::template setSlot<2>(v); }
		};

		//------------------------------------------------------------------------------------------------------
		//	Geodetic: latitude / longitude / altitude (tagged), plus the geocentric + ortho/ellipsoidal verbs.
		//------------------------------------------------------------------------------------------------------
		template<class Datum, class Derived>
		struct AxisAccessors<coordinateFrames::Geodetic3DFrame<Datum>, Derived> : AxisAccessorBase<Derived>
		{
			using base = AxisAccessorBase<Derived>;

			[[nodiscard]] angles::Latitude  latitude() const { return angles::Latitude(base::template slot<0>()); }
			[[nodiscard]] angles::Latitude  geodeticLatitude() const { return angles::Latitude(base::template slot<0>()); }
			[[nodiscard]] angles::Longitude longitude() const { return angles::Longitude(base::template slot<1>()); }
			[[nodiscard]] heights::kind_for<Datum> altitude() const { return heights::kind_for<Datum>(base::template slot<2>()); }

			void setLatitude(const degrees<>& v) { base::template setSlot<0>(v); }
			void setLongitude(const degrees<>& v) { base::template setSlot<1>(v); }
			void setAltitude(const meters<>& v) { base::template setSlot<2>(v); }

			/// This point's geocentric latitude, via the datum's reference ellipsoid.
			[[nodiscard]] angles::Geocentric geocentricLatitude() const
			{
				return convertLatitude<GeocentricLatitude, typename datum_traits<Datum>::reference_ellipsoid>(latitude());
			}

			/// This point's height above the ellipsoid (HAE), converting from the datum's vertical reference.
			[[nodiscard]] heights::Ellipsoidal toEllipsoidHeight() const
			{
				return coordinates::convertToEllipsoidHeight<typename datum_traits<Datum>::vertical_datum>(
				        base::template slot<0>(), base::template slot<1>(), base::template slot<2>());
			}

			/// This point's height above the geoid (MSL), converting from the datum's vertical reference.
			[[nodiscard]] heights::Orthometric toOrthometricHeight() const
			{
				return coordinates::convertFromEllipsoidHeight<typename datum_traits<Datum>::vertical_datum>(
				        base::template slot<0>(), base::template slot<1>(), toEllipsoidHeight());
			}
		};

		//------------------------------------------------------------------------------------------------------
		//	ENU: east / north / up (raw lengths).
		//------------------------------------------------------------------------------------------------------
		template<class HorizontalDatum, class Derived>
		struct AxisAccessors<coordinateFrames::ENUFrame<HorizontalDatum>, Derived> : AxisAccessorBase<Derived>
		{
			using base = AxisAccessorBase<Derived>;
			[[nodiscard]] auto east() const { return base::template slot<0>(); }
			[[nodiscard]] auto north() const { return base::template slot<1>(); }
			[[nodiscard]] auto up() const { return base::template slot<2>(); }
			void               setEast(const meters<>& v) { base::template setSlot<0>(v); }
			void               setNorth(const meters<>& v) { base::template setSlot<1>(v); }
			void               setUp(const meters<>& v) { base::template setSlot<2>(v); }
		};

		//------------------------------------------------------------------------------------------------------
		//	NED: north / east / down (raw lengths).
		//------------------------------------------------------------------------------------------------------
		template<class HorizontalDatum, class Derived>
		struct AxisAccessors<coordinateFrames::NEDFrame<HorizontalDatum>, Derived> : AxisAccessorBase<Derived>
		{
			using base = AxisAccessorBase<Derived>;
			[[nodiscard]] auto north() const { return base::template slot<0>(); }
			[[nodiscard]] auto east() const { return base::template slot<1>(); }
			[[nodiscard]] auto down() const { return base::template slot<2>(); }
			void               setNorth(const meters<>& v) { base::template setSlot<0>(v); }
			void               setEast(const meters<>& v) { base::template setSlot<1>(v); }
			void               setDown(const meters<>& v) { base::template setSlot<2>(v); }
		};

		//------------------------------------------------------------------------------------------------------
		//	AER: azimuth / elevation / range (tagged).
		//------------------------------------------------------------------------------------------------------
		template<class HorizontalDatum, class Derived>
		struct AxisAccessors<coordinateFrames::AERFrame<HorizontalDatum>, Derived> : AxisAccessorBase<Derived>
		{
			using base = AxisAccessorBase<Derived>;
			[[nodiscard]] angles::Azimuth   azimuth() const { return angles::Azimuth(base::template slot<0>()); }
			[[nodiscard]] angles::Elevation elevation() const { return angles::Elevation(base::template slot<1>()); }
			[[nodiscard]] ranges::Euclidean range() const { return ranges::Euclidean(base::template slot<2>()); }
			void                            setAzimuth(const degrees<>& v) { base::template setSlot<0>(v); }
			void                            setElevation(const degrees<>& v) { base::template setSlot<1>(v); }
			void                            setRange(const meters<>& v) { base::template setSlot<2>(v); }
		};
	}    // namespace traits
}    // namespace coordinates

#endif    // axisAccessors_h
