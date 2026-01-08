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

#ifndef verticalDatum_h__
#define verticalDatum_h__

//------------------------
//	INCLUDES
//------------------------

#include <type_traits>
#include <units.h>

#include "ellipsoid.h"
#include "geoid.h"
#include "topography.h"

namespace coord
{	
	namespace traits
	{

		/**
		 * @brief		Traits which tests whether a type represents a vertical datum
		 */
		template<typename T>
		struct is_vertical_datum : std::integral_constant<bool,
			coord::traits::is_ellipsoid<T>::value ||
			coord::traits::is_geoid<T>::value ||
			coord::traits::is_topography<T>::value>::type
		{};		

		/**
		 * @brief		Traits class which provides a common interface to different types of vertical datums
		 * @details		has overloads for ellipsoids, geoids, and topography models.
		 */
		template<class T, typename = void>
		struct vertical_datum_traits
		{
			typedef void base_datum;
			static units::length::meter_t correctionValue(units::angle::degree_t lat, units::angle::degree_t lon)
			{
				static_assert(is_vertical_datum<T>::value, "Type T is not a vertical datum.");
			}
		};

		template<class T>
		struct vertical_datum_traits<T, typename std::enable_if<is_ellipsoid<T>::value>::type>
		{
			typedef T base_datum;
			typedef T reference_ellipsoid;
			static units::length::meter_t correctionValue(units::angle::degree_t lat, units::angle::degree_t lon)
			{
				return 0_m;
			}
		};

		template<class T>
		struct vertical_datum_traits<T, typename std::enable_if<is_geoid<T>::value>::type>
		{
			typedef typename geoid_traits<T>::reference_ellipsoid base_datum;
			typedef typename geoid_traits<T>::reference_ellipsoid reference_ellipsoid;
			static units::length::meter_t correctionValue(units::angle::degree_t lat, units::angle::degree_t lon)
			{
				return T::undulation(lat, lon);
			}
		};

		template<class T>
		struct vertical_datum_traits<T, typename std::enable_if<is_topography<T>::value>::type>
		{
			typedef typename topography_traits<T>::reference_geoid base_datum;
			typedef typename geoid_traits<base_datum>::reference_ellipsoid reference_ellipsoid;
			static units::length::meter_t correctionValue(units::angle::degree_t lat, units::angle::degree_t lon)
			{
				return T::orthometricHeight(lat, lon);
			}
		};
	}

	namespace detail
	{
		/// Overload for if the height is already in an ellipsoid datum
		template<typename VerticalDatum, typename std::enable_if<traits::is_ellipsoid<VerticalDatum>::value, int>::type = 0>
		static units::length::meter_t convertToEllipsoidHeight(units::angle::degree_t latitude, units::angle::degree_t longitude, units::length::meter_t height)
		{
			return height;
		}

		/// Overload for if the height is already in an ellipsoid datum
		template<typename VerticalDatum, typename std::enable_if<!traits::is_ellipsoid<VerticalDatum>::value, int>::type = 0>
		static units::length::meter_t convertToEllipsoidHeight(units::angle::degree_t latitude, units::angle::degree_t longitude, units::length::meter_t height)
		{
			return convertToEllipsoidHeight<typename coord::traits::vertical_datum_traits<VerticalDatum>::base_datum>(latitude, longitude, 
				height + coord::traits::vertical_datum_traits<VerticalDatum>::correctionValue(latitude, longitude));
		}

		/// Overload for if the height is already in an ellipsoid datum
		template<typename VerticalDatum, typename std::enable_if<traits::is_ellipsoid<VerticalDatum>::value, int>::type = 0>
		static units::length::meter_t convertFromEllipsoidHeight(units::angle::degree_t latitude, units::angle::degree_t longitude, units::length::meter_t height)
		{
			return height;
		}

		/// Overload for if the height is already in an ellipsoid datum
		template<typename VerticalDatum, typename std::enable_if<!traits::is_ellipsoid<VerticalDatum>::value, int>::type = 0>
		static units::length::meter_t convertFromEllipsoidHeight(units::angle::degree_t latitude, units::angle::degree_t longitude, units::length::meter_t height)
		{
			return convertFromEllipsoidHeight<typename coord::traits::vertical_datum_traits<VerticalDatum>::base_datum>(latitude, longitude,
				height - coord::traits::vertical_datum_traits<VerticalDatum>::correctionValue(latitude, longitude));
		}
	}

	template<typename VerticalDatum, typename AngleUnits, typename LengthUnits, class enable = typename std::enable_if<traits::is_vertical_datum<VerticalDatum>::value>::type>
	static units::length::meter_t convertToEllipsoidHeight(AngleUnits latitude, AngleUnits longitude, LengthUnits height)
	{
		static_assert(units::traits::is_angle_unit<AngleUnits>::value, "Type of input argument `latitude` and `longitude` must be a unit of angle.");
		static_assert(units::traits::is_length_unit<LengthUnits>::value, "Type of input argument `height` must be a unit of length.");

		return detail::convertToEllipsoidHeight<VerticalDatum>(latitude, longitude, height);
	}

	template<typename VerticalDatum, typename AngleUnits, typename LengthUnits, class enable = typename std::enable_if<traits::is_vertical_datum<VerticalDatum>::value>::type>
	static units::length::meter_t convertFromEllipsoidHeight(AngleUnits latitude, AngleUnits longitude, LengthUnits height)
	{
		static_assert(units::traits::is_angle_unit<AngleUnits>::value, "Type of input argument `latitude` and `longitude` must be a unit of angle.");
		static_assert(units::traits::is_length_unit<LengthUnits>::value, "Type of input argument `height` must be a unit of length.");

		return detail::convertFromEllipsoidHeight<VerticalDatum>(latitude, longitude, height);
	}
}

#endif // verticalDatum_h__