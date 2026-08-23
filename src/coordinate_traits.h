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

#ifndef coordinate_traits_h
#define coordinate_traits_h

//------------------------
//	INCLUDES
//------------------------
#include <units.h>
#include <type_traits>

inline namespace coordinates
{
    inline namespace traits
    {
        /// Trait used to test for constexpr
        template <auto>
        struct require_constexpr_value
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
    }
}

#endif // coordinate_traits_h
