#ifndef coordinate_traits_h__
#define coordinate_traits_h__

//------------------------
//	INCLUDES
//------------------------
#include <units.h>
#include <type_traits>

namespace coord
{

	namespace traits
	{
		/**
		* @brief		dummy type.
		* @details		used by multiple typedef concept checkers.
		* @TODO			replace all instances with C++17 'void_t'
		*/
		template<class...>
		struct void_type {
			typedef void type;
		};
	
		/**
		* @brief		parameter pack for boolean arguments.
		*/
		template<bool...> struct bool_pack {};
	
		/**
		* @brief		Trait which tests that a set of other traits are all true.
		*/
		template<bool... Args>
		struct all_true : std::is_same<bool_pack<true, Args...>, bool_pack<Args..., true>> {};
	
		/**
		 * @brief		determines if a type is a specialization of another type
		 * @details		http://stackoverflow.com/questions/11251376/how-can-i-check-if-a-type-is-an-instantiation-of-a-given-class-template#comment14786989_11251408
		 */
		template < template <typename...> class Template, typename T >
		struct is_specialization_of : std::false_type {};
	
		template < template <typename...> class Template, typename... Args >
		struct is_specialization_of< Template, Template<Args...> > : std::true_type{};

		//----------------------------------
		//	LOGICAL TRAITS
		//----------------------------------
	
		// adapted from: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0013r0.html
	
		template<class...> struct and_type; // not defined
	
		template<> struct and_type<> : std::true_type{};
	
		template<class B1> struct and_type<B1> : B1{};
	
		template<class B1, class B2>
		struct and_type<B1, B2>
			: std::conditional<B1::value, B2, B1>
		{};
	
		template<class B1, class B2, class B3, class... Bn>
		struct and_type<B1, B2, B3, Bn...>
			: std::conditional<B1::value, and_type<B2, B3, Bn...>, B1>
		{};
	
		template<class...> struct or_type; // not defined
	
		template<> struct or_type<> : std::false_type{};
	
		template<class B1> struct or_type<B1> : B1{};
	
		template<class B1, class B2>
		struct or_type<B1, B2>
			: std::conditional<B1::value, B1, B2>
		{};
	
		template<class B1, class B2, class B3, class... Bn>
		struct or_type<B1, B2, B3, Bn...>
			: std::conditional<B1::value, B1, or_type<B2, B3, Bn...>>
		{};
	}
}

#endif // coordinate_traits_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: