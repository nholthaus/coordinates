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

#ifndef topography_h__
#define topography_h__

//------------------------
//	INCLUDES
//------------------------
#include <units.h>

#include "geoid.h"

namespace coord
{
	using namespace units::literals;

	//----------------------------------
	//	BASE TOPOGRAPHY
	//----------------------------------

	template<typename ReferenceGeoid>
	struct Topography
	{
		typedef ReferenceGeoid reference_geoid;
	};

	namespace topography
	{
		//	----------------------------------------------------------------------------
		//	CLASS		NULL_TOPOGRAPHY
		//  ----------------------------------------------------------------------------
		///	@brief		Always return 0m elevation
		///	@details	Useful for datums in which no topographical data exists or the
		///				topography is unimplemented.
		//  ----------------------------------------------------------------------------
		class NULL_TOPOGRAPHY : public Topography<coord::geoids::NULL_GEOID>
		{
		public:

			using meter_t = units::length::meter_t;
			using degree_t = units::angle::degree_t;

			/**
			 * @brief		always returns 0m
			 * @details		The orthometric height is the height of ground level (terrain) above mean-sea
			 *				level, as represented by the topography's reference geoid.
			 * @param[in]	latitude	latitude at which to calculate the orthometric height
			 * @param[in]	longitude	longitude at which to calculate the orthometric height
			 * @returns		0 meters for all inputs.
			 */
			static inline meter_t orthometricHeight(const degree_t& latitude, const degree_t& longitude)
			{
				return 0_m;
			}
		};
	}

	//----------------------------------
	//	TOPOGRAPHY TRAITS
	//----------------------------------

	namespace traits
	{
		/**
		* @brief		geoid traits implementation for classes which are not geoids.
		*/
		template<class T, typename = void>
		struct topography_traits
		{
			typedef void reference_geoid;
		};

		/**
		* @brief		Traits class defining the properties of a geoid.
		*/
		template<class T>
		struct topography_traits<T, typename coord::traits::void_type<
			typename T::reference_geoid >::type>
		{
			typedef typename T::reference_geoid reference_geoid;									///< Geoid that the topographic model is referenced to.
		};
	}

	//----------------------------------
	//	TOPOGRAPHY CONCEPT
	//----------------------------------

	namespace traits
	{
		namespace detail
		{
			/// implementation of the has_undulation concept checker.
			template <typename T>
			struct has_orthometricHeight_impl
			{
				using d = units::angle::degree_t;

				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::orthometricHeight(d(1.0), d(1.0)));
				template<typename U>
				static constexpr auto test(...)->std::false_type;

				using type = typename units::traits::is_length_unit<decltype(test<T>(0))>::type;
			};
		}

		/**
		* @brief		Tests that a class has a static `undulation` member
		* @details		The undulation member has template parameters for angle units, distance units, and
		*				type respectively, and returns the same type. `undulation` must be a static function.
		*				Type must be a variety of floating point, e.g. float, double, long double, etc.
		*/
		template <typename T>
		struct has_orthometricHeight : detail::has_orthometricHeight_impl<T>::type {};

		/**
		* @brief		describes the topography concept
		* @details		a class is a topography model if it:
		*				- provides the appropriate static member `orthometricHeight`.
		*				- is default constructible.
		*				- contains a typedef for the geoid that it is referenced to.
		*/
		template <typename T>
		struct is_topography :
			std::integral_constant<bool,
			std::is_same<T, topography::NULL_TOPOGRAPHY>::value ||
			(has_orthometricHeight<T>::value &&
			coord::traits::is_geoid<typename topography_traits<T>::reference_geoid>::value &&
			std::is_default_constructible<T>::value)>
		{};
	}

}


#endif // topography_h__