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

#ifndef verticalDatum_h
#define verticalDatum_h

//------------------------
//	INCLUDES
//------------------------

#include <units.h>
#include <units/kind.h>

#include "ellipsoid.h"
#include "geoid.h"
#include "topography.h"

#include <verticalDatum.h>

inline namespace coordinates
{
	/// Type-tagged vertical-height values. Both an ellipsoidal (HAE) and an orthometric (MSL) height are lengths,
	/// so nothing at the type level stops them being added or interchanged today -- yet doing so without the
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

		/// The geoid undulation N: the signed separation between the ellipsoid and the geoid at a point
		/// (`N = ellipsoidal - orthometric`). Tagged distinct from a height so it can only enter a height
		/// through the sanctioned `convertToEllipsoidHeight` / `convertFromEllipsoidHeight` bridge, never be
		/// mistaken for an altitude.
		using Undulation = units::kind<"geoid_undulation", units::length::meters<double>>;
	} // namespace heights

	inline namespace traits
	{

		template<typename T>
		concept VerticalDatum = coordinates::traits::is_ellipsoid<T> || coordinates::traits::is_geoid<T> || coordinates::traits::is_topography<T>;

		/**
		 * @brief		Traits which tests whether a type represents a vertical datum
		 */
		template<typename T>
		inline constexpr bool is_vertical_datum = VerticalDatum<T>;

		/**
		 * @brief		Traits class which provides a common interface to different types of vertical datums
		 * @details		has overloads for ellipsoids, geoids, and topography models.
		 */
		template<class T, typename = void>
		struct vertical_datum_traits
		{
			using base_datum = void;

			static meters<> correctionValue(degrees<> /*lat*/, degrees<> /*lon*/)
			{
				static_assert(is_vertical_datum<T>,
				              "Type T is not a vertical datum. "
				              "A vertical datum must be an ellipsoid, geoid, or topography model.");
				return 0.0_m;    // keeps return type well-formed even in error paths
			}
		};

		/// Ellipsoid Specialization
		template<class T>
		    requires is_ellipsoid<T>
		struct vertical_datum_traits<T, void>
		{
			using base_datum          = T;
			using reference_ellipsoid = T;

			static meters<> correctionValue(degrees<> /*lat*/, degrees<> /*lon*/) { return 0.0_m; }
		};

		/// Geoid Specialization
		template<class T>
		    requires is_geoid<T>
		struct vertical_datum_traits<T, void>
		{
			using base_datum          = geoid_traits<T>::reference_ellipsoid;
			using reference_ellipsoid = geoid_traits<T>::reference_ellipsoid;

			static meters<> correctionValue(degrees<> lat, degrees<> lon) { return T::undulation(lat, lon); }
		};

		/// Topography Specialization
		template<class T>
		    requires is_topography<T>
		struct vertical_datum_traits<T, void>
		{
			using base_datum          = topography_traits<T>::reference_geoid;
			using reference_ellipsoid = geoid_traits<base_datum>::reference_ellipsoid;

			static meters<> correctionValue(degrees<> lat, degrees<> lon) { return T::orthometricHeight(lat, lon); }
		};
	}    // namespace traits

	inline namespace traits
	{
		namespace detail
		{
			/// Overload for if the height is already in an ellipsoid datum
			template<typename VerticalDatum>
			    requires(is_ellipsoid<VerticalDatum>)
			static meters<> convertToEllipsoidHeight(degrees<>, degrees<>, meters<> height)
			{ return height; }

			/// Overload for if the height is already in an ellipsoid datum
			template<typename VerticalDatum>
			    requires(!is_ellipsoid<VerticalDatum>)
			static meters<> convertToEllipsoidHeight(degrees<> latitude, degrees<> longitude, meters<> height)
			{
				return convertToEllipsoidHeight<typename vertical_datum_traits<VerticalDatum>::base_datum>(
				        latitude,
				        longitude,
				        height + vertical_datum_traits<VerticalDatum>::correctionValue(latitude, longitude));
			}

			/// Overload for if the height is already in an ellipsoid datum
			template<typename VerticalDatum>
			    requires(is_ellipsoid<VerticalDatum>)
			static meters<> convertFromEllipsoidHeight(degrees<>, degrees<>, meters<> height)
			{ return height; }

			/// Overload for if the height is already in an ellipsoid datum
			template<typename VerticalDatum>
			    requires(!is_ellipsoid<VerticalDatum>)
			static meters<> convertFromEllipsoidHeight(degrees<> latitude, degrees<> longitude, meters<> height)
			{
				return convertFromEllipsoidHeight<typename vertical_datum_traits<VerticalDatum>::base_datum>(
				        latitude,
				        longitude,
				        height - vertical_datum_traits<VerticalDatum>::correctionValue(latitude, longitude));
			}
		}    // namespace detail
	}    // namespace traits

	/// Convert an orthometric (MSL) height to an ellipsoidal (HAE) height for the given vertical datum, adding
	/// the geoid undulation. The height is an `OrthometricHeight` and the result an `EllipsoidalHeight`, so the
	/// two cannot be confused: passing an already-ellipsoidal height here, or using the result where an
	/// orthometric height is expected, is a compile error. A plain `meters<>` still constructs into the
	/// `OrthometricHeight` argument, so `convertToEllipsoidHeight<D>(lat, lon, 12.0_m)` remains valid.
	template<VerticalDatum Datum, typename AngleUnits>
	static heights::Ellipsoidal convertToEllipsoidHeight(AngleUnits latitude, AngleUnits longitude, heights::Orthometric height)
	{
		static_assert(units::traits::is_angle_unit_v<AngleUnits>, "Type of input argument `latitude` and `longitude` must be a unit of angle.");

		return heights::Ellipsoidal(detail::convertToEllipsoidHeight<Datum>(latitude, longitude, height.template to<meters<>>()));
	}

	/// Convert an ellipsoidal (HAE) height to an orthometric (MSL) height for the given vertical datum,
	/// subtracting the geoid undulation. Inverse of `convertToEllipsoidHeight`; the `EllipsoidalHeight` input and
	/// `OrthometricHeight` result are tagged distinct so the two heights cannot be silently interchanged.
	template<VerticalDatum Datum, typename AngleUnits>
	static heights::Orthometric convertFromEllipsoidHeight(AngleUnits latitude, AngleUnits longitude, heights::Ellipsoidal height)
	{
		static_assert(units::traits::is_angle_unit_v<AngleUnits>, "Type of input argument `latitude` and `longitude` must be a unit of angle.");

		return heights::Orthometric(detail::convertFromEllipsoidHeight<Datum>(latitude, longitude, height.template to<meters<>>()));
	}
}    // namespace coordinates

#endif    // verticalDatum_h