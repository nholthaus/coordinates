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

#ifndef topography_h
#define topography_h

//------------------------
//	INCLUDES
//------------------------
#include <units.h>
#include <units/kind.h>

#include "geoid.h"
#include "heightKinds.h"

inline namespace coordinates
{
	using namespace units::literals;

	//----------------------------------
	//	BASE TOPOGRAPHY
	//----------------------------------

	template<typename ReferenceGeoid>
	struct Topography
	{
		using reference_geoid = ReferenceGeoid;
	};

	inline namespace topography
	{
		//	----------------------------------------------------------------------------
		//	CLASS		NULL_TOPOGRAPHY
		//  ----------------------------------------------------------------------------
		///	@brief		Always return 0m elevation
		///	@details	Useful for datums in which no topographical data exists or the
		///				topography is unimplemented.
		//  ----------------------------------------------------------------------------
		class NULL_TOPOGRAPHY : public Topography<coordinates::geoids::NULL_GEOID>
		{
		public:
			/**
			 * @brief		always returns 0m
			 * @details		The orthometric height is the height of ground level (terrain) above mean-sea
			 *				level, as represented by the topography's reference geoid.
			 * @returns		0 meters for all inputs.
			 */
			static heights::Orthometric orthometricHeight(const degrees<>, const degrees<>&) { return heights::Orthometric(0.0_m); }
		};
	}    // namespace topography

	//----------------------------------
	//	TOPOGRAPHY TRAITS
	//----------------------------------

	inline namespace traits
	{
		/**
		 * @brief		geoid traits implementation for classes which are not geoids.
		 */
		template<class, typename = void>
		struct topography_traits
		{
			using reference_geoid = void;
		};

		/**
		 * @brief		Traits class defining the properties of a geoid.
		 */
		template<class T>
		    requires requires { typename T::reference_geoid; }
		struct topography_traits<T, void>
		{
			using reference_geoid = T::reference_geoid;    ///< Geoid that the topographic model is referenced to.
		};
	}    // namespace traits

	//----------------------------------
	//	TOPOGRAPHY CONCEPT
	//----------------------------------

	inline namespace traits
	{
		/// A length "quantity": either a plain `units` length unit, or a `units::kind` whose wrapped unit is a
		/// length (e.g. `heights::Orthometric`). A tagged height is not itself a `units` unit, so a producer
		/// that returns one must still count as returning a length here.
		template<typename T>
		concept is_length_quantity =
		        units::traits::is_length_unit_v<T> || (units::traits::is_kind_v<T> && units::traits::is_length_unit_v<typename T::unit_type>);

		/// Concept that ensures a conforming `orthometricHeight(lat, lon)` static function is present, returning
		/// a length quantity (a plain length or a tagged height kind).
		template<typename T>
		concept OrthometricHeight = requires(degrees<> lat, degrees<> lon)
		{
			// Enforces a static call form (T::...), not an instance method, returning a length quantity.
			{ T::orthometricHeight(lat, lon) } -> is_length_quantity;
		};

		/// boolean constant to test `has_orthometricHeight`
		template<typename T>
		inline constexpr bool has_orthometricHeight = OrthometricHeight<T>;

		/**
		 * @brief		describes the topography concept
		 * @details		a class is a topography model if it:
		 *				- provides the appropriate static member `orthometricHeight`.
		 *				- is default constructible.
		 *				- contains a typedef for the geoid that it is referenced to.
		 */
		template<typename T>
		concept is_topography =
		        std::same_as<T, NULL_TOPOGRAPHY> || (has_orthometricHeight<T> && std::default_initializable<T> &&
		                                                         coordinates::traits::is_geoid<typename topography_traits<T>::reference_geoid>);
	}    // namespace traits

}    // namespace coordinates

#endif    // topography_h