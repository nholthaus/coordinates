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
// The per-frame axis table for the `Coordinate<Frame, Tuple>` collapse. A single `Coordinate` template holds
// every position's shared behaviour once; what differs per frame is only the NAMES and RETURN TYPES of the
// three axis accessors -- `x/y/z` for ECEF, `latitude/longitude/altitude` for a geodetic point,
// `azimuth/elevation/range` for AER, and so on. `frame_axes<Frame>` encodes that difference: for each axis it
// names the accessor's return type (which is NOT always the raw stored unit -- a geodetic altitude returns the
// datum's `heights::kind_for<Datum>`, an AER azimuth returns `angles::Azimuth`), and it flags whether the frame
// is local (carries an origin). The accessor NAMES are supplied by the frame-constrained accessor free
// functions that consume this trait; the trait carries the types and the local-ness that cannot be spelled as
// a plain unit. `Coordinate` reads `frame_axes<Frame>` for its accessors and `is_local` for its converting
// constructor branch.
//
//--------------------------------------------------------------------------------------------------

#ifndef frameAxes_h
#define frameAxes_h

//------------------------
//	INCLUDES
//------------------------

#include <cstddef>

#include <units.h>

#include "angles.h"
#include "frameOfReference.h"
#include "heights.h"
#include "ranges.h"

inline namespace coordinates
{
	using namespace units;

	inline namespace traits
	{
		//	----------------------------------------------------------------------------
		//	frame_axes<Frame>
		//  ----------------------------------------------------------------------------
		///	@brief		The axis return-type table + local-ness for a frame of reference.
		///	@details	The primary template is undefined; every frame that a `Coordinate` can name specializes
		///				it. Each specialization provides `component<I>` -- the return type of axis `I`'s accessor
		///				(0/1/2) -- and `is_local` (does the frame carry an origin). `T` is the underlying scalar.
		///	@tparam		Frame	the frame of reference.
		///	@tparam		T		the underlying arithmetic type (defaults to `double`).
		//  ----------------------------------------------------------------------------
		template<class Frame, typename T = double>
		struct frame_axes;

		/// ECEF: x / y / z, each a raw length in the frame's unit (defaults to meters).
		template<class HorizontalDatum, typename T>
		struct frame_axes<coordinateFrames::ECEFFrame<HorizontalDatum>, T>
		{
			static constexpr bool is_local = false;

			template<std::size_t I>
			using component = units::length::meters<T>;
		};

		/// Geodetic 3-D: latitude / longitude / altitude. Latitude and longitude are the tagged angle kinds;
		/// altitude is the height kind the datum measures (`heights::kind_for<Datum>` -- ellipsoidal or
		/// orthometric). Return types are datum-dependent, so the specialization takes the datum.
		template<class Datum, typename T>
		struct frame_axes<coordinateFrames::Geodetic3DFrame<Datum>, T>
		{
			static constexpr bool is_local = false;

			template<std::size_t I>
			using component = std::tuple_element_t<I, std::tuple<angles::Latitude, angles::Longitude, heights::kind_for<Datum>>>;
		};

		/// ENU: east / north / up, each a raw length.
		template<class HorizontalDatum, typename T>
		struct frame_axes<coordinateFrames::ENUFrame<HorizontalDatum>, T>
		{
			static constexpr bool is_local = true;

			template<std::size_t I>
			using component = units::length::meters<T>;
		};

		/// NED: north / east / down, each a raw length.
		template<class HorizontalDatum, typename T>
		struct frame_axes<coordinateFrames::NEDFrame<HorizontalDatum>, T>
		{
			static constexpr bool is_local = true;

			template<std::size_t I>
			using component = units::length::meters<T>;
		};

		/// AER: azimuth / elevation / range. Azimuth and elevation are the tagged angle kinds; range is a
		/// straight-line distance (`ranges::Euclidean`).
		template<class HorizontalDatum, typename T>
		struct frame_axes<coordinateFrames::AERFrame<HorizontalDatum>, T>
		{
			static constexpr bool is_local = true;

			template<std::size_t I>
			using component = std::tuple_element_t<I, std::tuple<angles::Azimuth, angles::Elevation, ranges::Euclidean>>;
		};

		//	----------------------------------------------------------------------------
		//	convert_fast_path<From, To>
		//  ----------------------------------------------------------------------------
		///	@brief		Optional direct conversion between two specific local frames, bypassing the ECEF
		///				intermediate.
		///	@details	Most frame pairs convert through the two-hop ECEF path in `Coordinate`'s converting
		///				constructor. A few pairs have a closed-form shortcut: NED<->ENU is a pure axis swap, and
		///				ENU/NED->AER is direct trig. Those live here, keyed on the (source, destination) frame
		///				pair, so the shortcut is isolated to this hook rather than smeared through the body. The
		///				primary template declares NO shortcut (`has_shortcut = false`); a specialization that
		///				provides one sets it `true` and implements `apply(sourceTuple, sharedOrigin)`.
		///	@tparam		From	the source frame.
		///	@tparam		To		the destination frame.
		//  ----------------------------------------------------------------------------
		template<class From, class To>
		struct convert_fast_path
		{
			static constexpr bool has_shortcut = false;
		};
	}    // namespace traits
}    // namespace coordinates

#endif    // frameAxes_h
