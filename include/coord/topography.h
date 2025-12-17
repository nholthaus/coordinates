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

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: