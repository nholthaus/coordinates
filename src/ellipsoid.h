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
/// @file	ellipsoid.h
/// @brief	Definition of Ellipsoid class and concepts
//
//--------------------------------------------------------------------------------------------------

#ifndef ellipsoid_h__
#define ellipsoid_h__

//------------------------
//	INCLUDES
//------------------------

#include "coordinate_traits.h"

#include <units/length.h>

//------------------------
//	NAMESPACE: COORD
//------------------------
inline namespace coord
{
    //------------------------
    //	USING
    //------------------------

    using namespace units::literals;

    //	----------------------------------------------------------------------------
    //	CLASS		ellipsoid
    //  ----------------------------------------------------------------------------
    ///	@brief		Interface class for reference ellipsoids for geodetic coordinates
    ///	@details
    ///
    /// NOTE: not all ellipsoids have the same defined/derived parameters, so we can't just code the following
    /// formulas. We'll make sure that the ellipsoid returns all these values, based on whichever parameters
    /// are defined:
    ///
    /// semi - major axis			(a)			typically defined
    /// semi - minor axis			(b)			= a(1 - f)
    /// flattening					(f)			= (a - b) / a
    /// flattening inverse			(f^-1)		= (1 / f)
    /// eccentricity				(e)			= sqrt(1 - (b^2 / a^2))
    /// eccentricity squared		(e^2)		= (a^2 - b^2) / a^2
    //  ----------------------------------------------------------------------------

    //----------------------------------
    //	ELLIPSOID CONCEPT
    //----------------------------------

    namespace traits
    {
        /**
        * @brief		ellipsoid concept definition
        * @details		Defines the ellipsoid concept. An object is an ellipsoid if:
        *				- It is default constructible.
        *				- It has all the proper members returning its constant values, which are:
        *					- a (semi-major axis)
        *					- b (semi-minor axis)
        *					- f (flattening)
        *					- invf (inverse flattening)
        *					- e (eccentricity)
        *					- e2 (eccentricity squared)
        *				- those constant values are returned as floating point (float, double, long double, etc)
        *				  types.
        */
        template <class T>
        concept is_ellipsoid =
            requires
        {
            { T::a() } -> std::convertible_to<units::length::meters<>>;
            { T::b() } -> std::convertible_to<units::length::meters<>>;
            { T::f() } -> std::convertible_to<units::dimensionless<>>;
            { T::invf() } -> std::convertible_to<units::dimensionless<>>;
            { T::e() } -> std::convertible_to<units::dimensionless<>>;
            { T::e2() } -> std::convertible_to<units::dimensionless<>>;

            // Constexpr-ness checks (these will fail if any function stops being constexpr)
            typename traits::require_constexpr_value<T::a()>;
            typename traits::require_constexpr_value<T::b()>;
            typename traits::require_constexpr_value<T::f()>;
            typename traits::require_constexpr_value<T::invf()>;
            typename traits::require_constexpr_value<T::e()>;
            typename traits::require_constexpr_value<T::e2()>;
        };
    }

    //----------------------------------
    //	ELLIPSOID BASE
    //----------------------------------

    //	----------------------------------------------------------------------------
    //	CLASS		Ellipsoid
    //  ----------------------------------------------------------------------------
    ///	@brief		Convenience base class for ellipsoids.
    ///	@details	Notice: you don't *have* to inherit from this class to create
    ///				a compatible ellipsoid. All you have to do is match the concept.
    ///				This class is just provided for convenience. If your ellipsoid
    ///				isn't defined in terms of `a` and `invf`, just implement your
    ///				own with the member functions required by `is_ellipsoid`.
    /// @tparam		SemiMajorAxis		semi-major axis length, in meters, defined
    ///									by a `meters` type.
    /// @tparam		InverseFlattening	inverse flattening, defined by a floating point type.
    //  ----------------------------------------------------------------------------
    //	template <class SemiMajorAxisRatio, class InverseFlatteningRatio>
    template <units::length::meters SemiMajorAxis, units::dimensionless InverseFlattening>
    class Ellipsoid
    {
        static_assert(SemiMajorAxis > 0.0_m, "Ellipsoid: semi-major axis must be positive");
        static_assert(InverseFlattening > 1.0, "Ellipsoid: invf must be > 1");

    public:
        /**
        * @brief		semi-major axis.
        * @details		Also referred to as the Equatorial radius. Typically, a defined parameter.
        * @returns		returns the semi-major axis value in units of meters
        */
        static constexpr auto a() -> decltype(SemiMajorAxis)
        {
            return SemiMajorAxis;
        }

        /**
        * @brief		semi-minor axis.
        * @details		Also referred to as the Polar radius. May be a defined or derived parameter.
        * @returns		returns the semi-minor axis value in units of <i>OutputUnits</i>
        */
        static constexpr auto b()
        {
            return a() * (units::dimensionless(1.0) - f());
        }

        /**
        * @brief		flatting coefficient
        * @details		Flattening is a measure of the compression of a circle or sphere along a
        * 				diameter to form an ellipse or an ellipsoid of revolution (spheroid) respectively.
        * 				Other terms used are ellipticity, or oblateness. The usual notation for flattening
        * 				is f and its definition in terms of the semi-axes of the resulting ellipse or
        * 				ellipsoid is f = (a - b) / a. Flattening is a dimensionless constant, and so it
        * 				does not have a unit template.
        * @returns		flattening coefficient with <i>T</i> precision.
        */
        static constexpr auto f()
        {
            return units::dimensionless(1.0) / InverseFlattening;
        }

        /**
        * @brief		inverse flattening coefficient
        * @details		inverse of the flattening coefficient, f(). May be a defined or derived parameter.
        * 				Inverse Flattening is a dimensionless constant, and so it does not have a unit
        * 				template.
        * @sa			f()
        * @return		inverse flattening coefficient with <i>T</i> precision.
        */
        static constexpr auto invf()
        {
            return InverseFlattening;
        }

        /**
        * @brief		eccentricity coefficient
        * @details		eccentricity can be thought of as a measure of how much the ellipse deviates from
        * 				being circular. It's definition in terms of the semi-axes of the ellipse is
        * 				sqrt(1 - (b^2 / a^2)). Eccentricity is a dimensionless constant, and so it does
        * 				not have a unit template.
        * @return		eccentricity coefficient with <i>T</i> precision.
        */
        static constexpr auto e()
        {
            return units::sqrt(units::dimensionless(1.0) - (units::pow<2>(b()) / units::pow<2>(a())));
        }

        /**
        * @brief		eccentricity coefficient squared
        * @details		first eccentricity of the ellipsoid, squared.
        * @sa			e()
        * @return		eccentricity coefficient squared with <i>T</i> precision.
        */
        static constexpr auto e2()
        {
            return (units::pow<2>(a()) - units::pow<2>(b())) / units::pow<2>(a());
        }
    };

    //----------------------------------
    //	COMMON ELLIPSOIDS
    //----------------------------------

    inline namespace ellipsoids
    {
        //	----------------------------------------------------------------------------
        //	CLASS		WGS84_Ellipsoid
        //  ----------------------------------------------------------------------------
        ///	@brief		Reference Ellipsoid for the WGS84 Datum
        ///	@details	Not to be confused with the EMG96 geoid.
        //  ----------------------------------------------------------------------------
        class WGS84 : public Ellipsoid<6378137.0_m, 298.257223563>
        {
        };

        //	----------------------------------------------------------------------------
        //	CLASS		GRS80
        //  ----------------------------------------------------------------------------
        ///	@brief		Reference Ellipsoid for the NAD83 Datum and many others
        ///	@details
        //  ----------------------------------------------------------------------------
        class GRS80 : public Ellipsoid<6378137.0_m, 298.257222101>
        {
        };
    }
}

#endif // ellipsoid_h__