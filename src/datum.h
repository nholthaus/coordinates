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

#ifndef datum_h
#define datum_h

//------------------------
//	INCLUDES
//------------------------

#include "horizontalDatum.h"
#include "verticalDatum.h"

#include <type_traits>

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		Datum
	//  ----------------------------------------------------------------------------
	///	@brief		base class for creating datum definitions
	///	@details
	/// @note		Inheritance from this class is not required to fulfill the datum
	///				concept. See `is_datum` for the concept requirements.
	//  ----------------------------------------------------------------------------
	template<class Horizontal, class Vertical = horizontal_datum_traits<Horizontal>::reference_ellipsoid>
	struct Datum : Horizontal, Vertical
	{
		static_assert(traits::is_horizontal_datum<Horizontal>, "Template parameter `Horizontal` must be a horizontal datum type.");
		static_assert(traits::is_vertical_datum<Vertical>, "Template parameter `Vertical` must be a vertical datum type.");
		static_assert(std::is_same_v<typename horizontal_datum_traits<Horizontal>::reference_ellipsoid,
		                             typename vertical_datum_traits<Vertical>::reference_ellipsoid>,
		              "The `Horizontal` and `Vertical` Datum components must refer to the same ellipsoid.");

		using horizontal_datum    = Horizontal;
		using vertical_datum      = Vertical;
		using reference_ellipsoid = horizontal_datum_traits<Horizontal>::reference_ellipsoid;
		using reference_frame     = horizontal_datum_traits<Horizontal>::reference_frame;
	};

	//----------------------------------
	//	COMMON DATUMS
	//----------------------------------
	inline namespace datums
	{
		//	----------------------------------------------------------------------------
		//	CLASS		WGS84_G1674
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct WGS84_G1674 : Datum<horizontalDatums::WGS84_G1674>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		WGS84_G1674_MSL
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct WGS84_G1674_MSL : Datum<horizontalDatums::WGS84_G1674, geoids::EGM96>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		NAD83
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Only suitable for use within CONUS (24-58N, 130-60W)
		//  ----------------------------------------------------------------------------
		struct NAD83 : Datum<horizontalDatums::NAD83>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		NAD83_NAVD88
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Only suitable for use within CONUS (24-58N, 130-60W)
		//  ----------------------------------------------------------------------------
		struct NAD83_NAVD88 : Datum<horizontalDatums::NAD83, geoids::GEOID12A>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		NAD83_MSL
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Only suitable for use within CONUS (24-58N, 130-60W)
		//  ----------------------------------------------------------------------------
		struct NAD83_MSL : Datum<horizontalDatums::NAD83, geoids::USGG2012>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		IGS08
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct IGS08 : Datum<horizontalDatums::IGS08>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		IGS08_MSL
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Only suitable for use within CONUS (24-58N, 130-60W)
		//  ----------------------------------------------------------------------------
		struct IGS08_MSL : Datum<horizontalDatums::IGS08, geoids::USGG2012>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		GDA94
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Only suitable for use in and about Australia
		//  ----------------------------------------------------------------------------
		struct GDA94 : Datum<horizontalDatums::GDA94>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		GDA2020
		//  ----------------------------------------------------------------------------
		///	@brief		Geodetic Datum of Australia (2020).
		///	@details	The plate-fixed modern Australian datum, tied to ITRF2014 with the plate-motion
		///				(time-dependent) transform. See the horizontal datum for the validated parameters.
		/// @note		Only suitable for use in and about Australia.
		//  ----------------------------------------------------------------------------
		struct GDA2020 : Datum<horizontalDatums::GDA2020>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		ETRS89
		//  ----------------------------------------------------------------------------
		///	@brief		European Terrestrial Reference System 1989 (legacy zero-transform realization).
		///	@details	This realization ties ETRS89 to ITRF89 with a zero transform, which is only correct
		///				at the 1989.0 reference epoch. It does NOT model the accumulated Eurasia plate motion,
		///				so it is off by decimetres at modern epochs. Use `ETRF2000` or `ETRF2014` for accurate
		///				modern European work.
		/// @note		Only suitable for use in and about Europe, and only near epoch 1989.0.
		//  ----------------------------------------------------------------------------
		struct [[deprecated("ETRS89 is a zero-transform tie valid only at epoch 1989.0; use ETRF2000 or ETRF2014 for modern epochs")]] ETRS89
		    : Datum<horizontalDatums::ETRS89>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		ETRF2000
		//  ----------------------------------------------------------------------------
		///	@brief		European Terrestrial Reference Frame 2000.
		///	@details	The modern, plate-motion-aware European realization. Prefer this (or `ETRF2014`) over
		///				the legacy `ETRS89` zero-transform tie.
		/// @note		Only suitable for use in and about Europe.
		//  ----------------------------------------------------------------------------
		struct ETRF2000 : Datum<horizontalDatums::ETRF2000>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		ETRF2014
		//  ----------------------------------------------------------------------------
		///	@brief		European Terrestrial Reference Frame 2014.
		///	@details	The latest European realization tied to ITRF2014. Constants from the primary EPSG
		///				record (EPSG:8366); validated end-to-end against PROJ to sub-mm.
		/// @note		Only suitable for use in and about Europe.
		//  ----------------------------------------------------------------------------
		struct ETRF2014 : Datum<horizontalDatums::ETRF2014>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		ITRS2008
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct ITRS2008 : Datum<horizontalDatums::ITRS2008>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		ITRS2014
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct ITRS2014 : Datum<horizontalDatums::ITRS2014>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		ITRS2000
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct ITRS2000 : Datum<horizontalDatums::ITRS2000>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		ITRS88
		//  ----------------------------------------------------------------------------
		///	@brief
		///	@details
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct ITRS88 : Datum<horizontalDatums::ITRS88>
		{
		};
	}    // namespace datums

	//----------------------------------
	//  TYPE TRAITS (C++23)
	//----------------------------------

	inline namespace traits
	{
		/**
		 * @brief datum traits implementation for classes which are not datums.
		 */
		template<class T, typename = void>
		struct datum_traits
		{
			using horizontal_datum    = void;
			using vertical_datum      = void;
			using reference_ellipsoid = void;
			using reference_frame     = void;
		};

		/**
		 * @brief Traits class defining the properties of a datum.
		 */
		template<class T>
		    requires requires {
			    typename T::horizontal_datum;
			    typename T::vertical_datum;
			    typename T::reference_frame;
			    typename T::reference_ellipsoid;
		    }
		struct datum_traits<T, void>
		{
			using horizontal_datum    = T::horizontal_datum;       ///< Horizontal component of the datum
			using vertical_datum      = T::vertical_datum;         ///< Vertical component of the datum
			using reference_ellipsoid = T::reference_ellipsoid;    ///< Ellipsoid component of the datum
			using reference_frame     = T::reference_frame;        ///< Reference (ITRF) frame of the datum
		};

		//--------------------------------------------------------------------------
		//  detail concepts (private): keep public names stable, but use concepts for clarity
		//--------------------------------------------------------------------------

		namespace detail
		{
			template<typename T>
			concept HasDatumTypedefs =
			        requires {
				        typename datum_traits<T>::horizontal_datum;
				        typename datum_traits<T>::vertical_datum;
				        typename datum_traits<T>::reference_ellipsoid;
				        typename datum_traits<T>::reference_frame;
			        } && !std::is_same_v<typename datum_traits<T>::horizontal_datum, void> &&
			        !std::is_same_v<typename datum_traits<T>::vertical_datum, void> &&
			        !std::is_same_v<typename datum_traits<T>::reference_ellipsoid, void> &&
			        !std::is_same_v<typename datum_traits<T>::reference_frame, void>;

		}    // namespace detail

		/**
		 * @brief Traits which tests whether a class satisfies the datum concept
		 */
		template<typename T>
		concept is_datum = detail::HasDatumTypedefs<T> && coordinates::traits::is_horizontal_datum<typename datum_traits<T>::horizontal_datum> &&
		                   coordinates::traits::is_vertical_datum<typename datum_traits<T>::vertical_datum> &&
		                   coordinates::traits::is_ellipsoid<typename datum_traits<T>::reference_ellipsoid>;

	}    // namespace traits

}    // namespace coordinates

#endif    // datum_h