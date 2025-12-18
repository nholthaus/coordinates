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
/// @file	coordinate_traits.h
/// @brief	Various type traits used by this library
//
//--------------------------------------------------------------------------------------------------

#ifndef coordinate_traits_h__
#define coordinate_traits_h__

//------------------------
//	INCLUDES
//------------------------
#include <units.h>
#include <type_traits>

inline namespace coord
{
    namespace traits
    {
        /// Trait used to test for constexpr
        template <auto>
        struct require_constexpr_value
        {
        };

        /**
        * @brief		parameter pack for boolean arguments.
        */
        template <bool...>
        struct bool_pack
        {
        };

        /**
        * @brief		Trait which tests that a set of other traits are all true.
        */
        template <bool... Args>
        struct all_true : std::is_same<bool_pack<true, Args...>, bool_pack<Args..., true>>
        {
        };

        /**
         * @brief		determines if a type is a specialization of another type
         * @details		http://stackoverflow.com/questions/11251376/how-can-i-check-if-a-type-is-an-instantiation-of-a-given-class-template#comment14786989_11251408
         */
        template <template <typename...> class Template, typename T>
        struct is_specialization_of : std::false_type
        {
        };

        template <template <typename...> class Template, typename... Args>
        struct is_specialization_of<Template, Template<Args...>> : std::true_type
        {
        };

        //----------------------------------
        //	LOGICAL TRAITS
        //----------------------------------

        // adapted from: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0013r0.html

        template <class...>
        struct and_type; // not defined

        template <>
        struct and_type<> : std::true_type
        {
        };

        template <class B1>
        struct and_type<B1> : B1
        {
        };

        template <class B1, class B2>
        struct and_type<B1, B2>
            : std::conditional<B1::value, B2, B1>
        {
        };

        template <class B1, class B2, class B3, class... Bn>
        struct and_type<B1, B2, B3, Bn...>
            : std::conditional<B1::value, and_type<B2, B3, Bn...>, B1>
        {
        };

        template <class...>
        struct or_type; // not defined

        template <>
        struct or_type<> : std::false_type
        {
        };

        template <class B1>
        struct or_type<B1> : B1
        {
        };

        template <class B1, class B2>
        struct or_type<B1, B2>
            : std::conditional<B1::value, B1, B2>
        {
        };

        template <class B1, class B2, class B3, class... Bn>
        struct or_type<B1, B2, B3, Bn...>
            : std::conditional<B1::value, B1, or_type<B2, B3, Bn...>>
        {
        };
    }
}

#endif // coordinate_traits_h__
