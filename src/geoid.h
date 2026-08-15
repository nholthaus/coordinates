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
/// @file	geoid.h
/// @brief	Definition of Geoid class and concepts
//
//--------------------------------------------------------------------------------------------------

#ifndef geoid_h
#define geoid_h

//------------------------
//	INCLUDES
//------------------------

#include <units/length.h>
#include <units/angle.h>

#include <interpolate.h>

#include "ellipsoid.h"
#include "EGM96LUT.h"
#include "USGG2012LUT.h"
#include "GEOID12ALUT.h"

inline namespace coordinates
{
    using namespace units::literals;

    //----------------------------------
    //	BASE GEOID
    //----------------------------------

    template <typename ReferenceEllipsoid>
    struct Geoid
    {
        using reference_ellipsoid = ReferenceEllipsoid;
    };

    namespace geoids
    {
        //----------------------------------
        //	HELPER FUNCTIONS
        //----------------------------------

        constexpr int wrap_periodic(int idx, int period) noexcept
        {
            idx %= period;
            if (idx < 0) idx += period;
            return idx;
        }

        constexpr int reflect_lat(int idx, int nrows) noexcept
        {
            // reflect repeatedly until in range
            while (idx < 0 || idx >= nrows)
            {
                if (idx < 0) idx = -idx;                     // reflect at +90
                else         idx = 2*(nrows - 1) - idx;      // reflect at -90
            }
            return idx;
        }

        constexpr int clamp_index(int idx, int lo, int hi) noexcept
        {
            return (idx < lo) ? lo : (idx > hi) ? hi : idx;
        }

        //	----------------------------------------------------------------------------
        //	CLASS		NULL_GEOID
        //  ----------------------------------------------------------------------------
        ///	@brief		null-value geoid
        ///	@details	Can be used for datums which don't required or define a geoid,
        ///				or for cases in which the geoid code is unimplemented.
        //  ----------------------------------------------------------------------------
        class NULL_GEOID : public Geoid<void>
        {
        public:
            /**
            * @brief		always returns 0
            * @param[in]	latitude latitude to find the undulation at, +90N to -90S.
            * @param[in]	longitude longitude to find the undulation at, either in -180W to 180E, or 0E to 360E
            * @returns		0 meters for all lat/lon combinations.
            */
            static constexpr meters<> undulation(degrees<> latitude, degrees<> longitude)
            {
                return 0_m;
            }
        };

        //	----------------------------------------------------------------------------
        //	CLASS		EGM96
        //  ----------------------------------------------------------------------------
        ///	@brief		Earth Gravitational Model 1996
        ///	@details	EGM96 is the geoid reference used by the GPS (WGS84) system
        //  ----------------------------------------------------------------------------
        class EGM96 : public Geoid<WGS84>
        {
        public:
            /**
            * @brief		calculates the undulation of the geoid at a given latitude/longitude.
            * @details		Undulation of the geoid is the mathematical process of determining the height in
            *				meters above the geoid (relative to the mean sea level) from the height provided
            *				by the GPS system which uses the (WGS84) ellipsoid as reference. In maps and common
            *				use the height over the mean sea level is used to indicate the height of elevations
            *				while the ellipsoidal height results from the restrictions that apply for the GPS system.
            *				EGM96 uses a built-in undulation lookup table to determine the height above sea level.
            *				The deviation \zeta between the ellipsoidal height h and the orthometric height
            *				H can be calculated by \n\n
            *
            *				N=h-H	\n\n
            *
            *				Likewise, the deviation \zeta between the ellipsoidal height h and the normal height H_N can be calculated by \n\n
            *
            *				\zeta=h-H_N
            * @note			this geoid interpolation is believed to be accurate to +/- 1cm of the accepted
            *				NGA interpolation values.
            * @param[in]	latitude latitude to find the undulation at, +90N to -90S.
            * @param[in]	longitude longitude to find the undulation at, either in -180W to 180E, or 0E to 360E
            * @returns		geoid undulation at (latitude, longitude) relative to the WGS84 ellipsoid.
            */
            static meters<> undulation(degrees<> latitude, degrees<> longitude)
            {
                // convert from -180  180 to 0 360 if need be.
                longitude = longitude < 0.0_deg ? longitude + 360.0_deg : longitude;

                // convert the lat/lon to the closest indices in the EGM96 look-up table.
                // The LUT layout is :
                //
                //   90.00 N + ------------------ +
                //           |                    |
                //           |   15' spacing N/S  |
                //           |                    |
                //           |                    |
                //           |   15' spacing E/W  |
                //           |                    |
                // 	-90.00 N + ------------------ +
                //          0.00 E           360.00 E
                //
                //---------------------------------------------------

                const double precision = EGM96LUT_PRECISION_DEG;
            	const double divisor = EGM96LUT_DIVISOR;

                // round to nearest 0.25 towards +90.0 (index 0)
                auto lat = ceil(latitude * dimensionless{1.0} / precision) / (dimensionless{1.0} / precision);
                int latIndex = static_cast<int>((lat - 90.0_deg) / -precision);
                // convert to 0 = North, -180 = South. gives values where 0 = North and 720 = South

                // round to nearest 0.25 towards 0.0 (index 0)
                auto lon = floor(longitude * 1.0 / precision) / (1.0 / precision);
                // round to the nearest quarter. The LUT has increments of 15 arcsec = 0.25 degrees
                int lonIndex = static_cast<int>(lon / precision); // gives values where 0 = East and 1440 = West

                auto xVal = (lat - latitude) / precision;
                // distance from nearest point in LUT to desired point, normalized on a scale of 0-1. latitude is always > lat.
                auto yVal = (longitude - lon) / precision;
                // distance from nearest point in LUT to desired point, normalized on a scale of 0-1. longitude is always > lon.

                // create surface of points to interpolate with
                interpolationMatrix<double> surface;
                constexpr int interpolationDimension = surface.size();

                constexpr int nrows  = EGM96LUT.size();
                constexpr int ncols  = EGM96LUT[0].size();
                constexpr int period = ncols - 1; // assume last column duplicates first

                for (int i = 0; i < interpolationDimension; ++i)
                {
                    const int row = reflect_lat(latIndex + (i - 1), nrows);

                    for (int j = 0; j < interpolationDimension; ++j)
                    {
                        const int col = wrap_periodic(lonIndex + (j - 1), period);
                        surface[i][j] = EGM96LUT[row][col] / divisor;
                    }
                }

                return meters(bicubicInterpolate(surface, xVal.to<double>(), yVal.to<double>()));
            }
        };

        //	----------------------------------------------------------------------------
        //	CLASS		USGG2012
        //  ----------------------------------------------------------------------------
        ///	@brief		US Gravimetric geoid model 2012 (NGS)
        ///	@details	GEOID12A is the geoid reference used by the NAD83 system
        //  ----------------------------------------------------------------------------
        class USGG2012 : public Geoid<GRS80>
        {
        public:
            /**
             * @brief		calculates the undulation of the geoid at a given latitude/longitude.
             * @details		Undulation of the geoid is the mathematical process of determining the height in
             *				meters above the geoid (relative to the mean sea level) from the height provided
             *				by an ellipsoidal datum reference. \n\n
             *				http://www.ngs.noaa.gov/GEOID/USGG2012/ \n\n
             *				USGG2012 is a refined gravimetric model of the geoid in the United States
             *				and other territories, which supersedes the previous models USGG2009
             *				and USGG2003.\n\n
             *				For USGG2012 in CONUS regions, heights range from a low of -52.53 meters
             *				in the Atlantic Ocean to a high of 3.47 meters in the Labrador Strait.\n\n
             *				USGG2012 refers to a GRS 80 ellipsoid, centered in the IGS08 (2005.00)
             *				reference frame, and is the best geopotential surface that approximates
             *				Mean Sea Level (MSL).\n\n
             *				USGG2012 is NOT for converting between NAD83 and NAVD88.\n\n
             *				USGG2012 is suitable for use with WGS84 and scientific applications.\n\n
             * @note			this geoid interpolation is believed to be accurate to +/- 1cm of the accepted
             *				USGS interpolation values.
             * @param[in]	latitude latitude to find the undulation at, +58N to +24N.
             * @param[in]	longitude longitude to find the undulation at, either in -130W to -60W, or 230E to 300E
             * @returns		geoid undulation at (latitude, longitude) relative to the GRS80 ellipsoid.
             */
            static meters<> undulation(degrees<> latitude, degrees<> longitude)
            {
                // convert from -180  180 to 0 360 if need be.
                longitude = longitude < 0.0_deg ? longitude + 360.0_deg : longitude;

                if (latitude > 58_deg || latitude < 24_deg)
                    throw std::runtime_error("Latitude out of bounds while calculating GEOID12A undulation.");
                if (longitude > 300_deg || longitude < 230_deg)
                    throw std::runtime_error("Longitude out of bounds while calculating GEOID12A undulation.");

                // convert the lat/lon to the closest indices in the EGM96 look-up table.
                // The LUT layout is :
                //
                //  58 N + ----------------- +
                //       |                   |
                //       |   1' spacing N/S  |
                //       |                   |
                //       |                   |
                //       |   1' spacing E/W  |
                //       |                   |
                // 	24 N + ----------------- +
                //      130 W               60 W
                //
                //---------------------------------------------------

                const degrees resolution(USGG2012LUT_PRECISION_DEG);
            	const double divisor = USGG2012LUT_DIVISOR;

                // get latitude index.toDouble()s (with decimals)
                auto latX = (latitude - 24_deg) / resolution;
                auto lonY = (longitude - 230_deg) / resolution;

                // find the lat/lon index (integer)
                unsigned int latIndex =floor(latX);
                unsigned int lonIndex = floor(lonY);

                // interpolation x,y
                long double xVal = (latX - latIndex);
                long double yVal = (lonY - lonIndex);

                // create surface of points to interpolate with. Use nearest-neighbor extrapolation on the edges
                interpolationMatrix<long double> elevationSurface;
                constexpr std::size_t interpolationDimension = elevationSurface.size();

                constexpr int numLatitudeLines  = USGG2012LUT.size();
                constexpr int numLongitudeLines = USGG2012LUT[0].size();

                for (int i = 0; i < static_cast<int>(interpolationDimension); ++i)
                {
                    const int row = clamp_index(latIndex + (i - 1), 0, numLatitudeLines - 1);

                    for (int j = 0; j < static_cast<int>(interpolationDimension); ++j)
                    {
                        const int column = clamp_index(lonIndex + (j - 1), 0, numLongitudeLines - 1);

                        // query the LUT (cast to size_t to keep std::array indexing happy)
                        elevationSurface[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] =
                            USGG2012LUT[static_cast<std::size_t>(row)][static_cast<std::size_t>(column)] / divisor;
                    }
                }

                return meters(bicubicInterpolate(elevationSurface, xVal, yVal));
            }
        };

        //	----------------------------------------------------------------------------
        //	CLASS		GEOID12A
        //  ----------------------------------------------------------------------------
        ///	@brief		Earth Gravitational Model Geoid 12A (NGS)
        ///	@details	GEOID12A is the geoid reference used to convert between NAD83 and
        ///				NAVD88.
        //  ----------------------------------------------------------------------------
        class GEOID12A : public Geoid<GRS80>
        {
        public:
            /**
             * @brief		calculates the undulation of the geoid at a given latitude/longitude.
             * @details		Undulation of the geoid is the mathematical process of determining the height in
             *				meters above the geoid (relative to the mean sea level) from the height provided
             *				by the ellipsoid as reference.
             *				http://www.ngs.noaa.gov/GEOID/GEOID12A/ \n\n
             *				GEOID12 is a refined hybrid model of the geoid in the United States,
             *				which supersedes the previous models GEOID09, GEOID06, GEOID03, GEOID99,
             *				GEOID96, GEOID93, and GEOID90.\n\n
             *				This model is intended for converting between the NAD83 ellipsoid
             *				reference frame resulting from the National Adjustment of 2011 and vertical
             *				datum NAVD88.\n\n
             *				For GEOID12 in CONUS regions, heights range from a low of -50.71 meters
             *				in the Atlantic Ocean to a high of 3.31 meters in the Labrador Strait.
             *				However, these geoid heights are only reliable within CONUS due to the
             *				limited extents of the data used to compute it.\n\n
             * @note			this geoid interpolation is believed to be accurate to +/- 1cm of the accepted
             *				NGA interpolation values.
             * @param[in]	latitude latitude to find the undulation at, +58N to +24N.
             * @param[in]	longitude longitude to find the undulation at, either in -130W to -60W, or 230E to 300E
             * @returns		geoid undulation at (latitude, longitude) relative to the NAVD88 Datum.
             */
            static meters<> undulation(degrees<> latitude, degrees<> longitude)
            {
                // convert from -180  180 to 0 360 if need be.
                longitude = longitude < 0.0_deg ? longitude + 360.0_deg : longitude;

                if (latitude > 58_deg || latitude < 24_deg)
                    throw std::runtime_error("Latitude out of bounds while calculating GEOID12A undulation.");
                if (longitude > 300_deg || longitude < 230_deg)
                    throw std::runtime_error("Longitude out of bounds while calculating GEOID12A undulation.");

                // convert the lat/lon to the closest indices in the EGM96 look-up table.
                // The LUT layout is :
                //
                //  58 N + ----------------- +
                //       |                   |
                //       |   1' spacing N/S  |
                //       |                   |
                //       |                   |
                //       |   1' spacing E/W  |
                //       |                   |
                // 	24 N + ----------------- +
                //      130 W               60 W
                //
                //---------------------------------------------------

                const degrees resolution(GEOID12ALUT_PRECISION_DEG);
            	const double divisor = GEOID12ALUT_DIVISOR;

                // get latitude index.toDouble()s (with decimals)
                const auto latX = (latitude - 24.0_deg) / resolution;
                const auto lonY = (longitude - 230.0_deg) / resolution;

                // find the lat/lon index (integer)
                const auto latIndex = static_cast<unsigned int>(floor(latX));
                const auto lonIndex = static_cast<unsigned int>(floor(lonY));

                // interpolation x,y
                long double xVal = latX - latIndex;
                long double yVal = lonY - lonIndex;

                // create surface of points to interpolate with. Use nearest-neighbor extrapolation on the edges
                interpolationMatrix<long double> elevationSurface;

                constexpr size_t interpolationDimension = elevationSurface.size();
                constexpr auto numLatitudeLines  = static_cast<int>(GEOID12ALUT.size());
                constexpr auto numLongitudeLines = static_cast<int>(GEOID12ALUT[0].size());

                for (int i = 0; i < interpolationDimension; ++i)
                {
                    const int row = clamp_index(latIndex + (i - 1), 0, numLatitudeLines - 1);

                    for (int j = 0; j < interpolationDimension; ++j)
                    {
                        const int col = clamp_index(lonIndex + (j - 1), 0, numLongitudeLines - 1);
                        elevationSurface[i][j] = GEOID12ALUT[row][col] / divisor;
                    }
                }

                return meters(bicubicInterpolate(elevationSurface, xVal, yVal));
            }
        };
    }

    //----------------------------------
    //	GEOID TRAITS
    //----------------------------------

    inline namespace traits
    {
        /**
         * @brief		geoid traits implementation for classes which are not geoids.
         */
        template <class T, typename = void>
        struct geoid_traits
        {
            using reference_ellipsoid = void;
        };

        /**
         * @brief		Traits class defining the properties of a geoid.
         */
        template <class T>
            requires requires { typename T::reference_ellipsoid; }
        struct geoid_traits<T, void>
        {
            using reference_ellipsoid = T::reference_ellipsoid;
            ///< Ellipsoid that the geoid model is referenced to.
        };
    }

    //----------------------------------
    //  GEOID CONCEPTS
    //----------------------------------

    inline namespace traits
    {
        // Helper alias
        template <class T>
        using geoid_reference_ellipsoid_t = geoid_traits<T>::reference_ellipsoid;

        /**
         * @brief   has_undulation concept
         * @details Requires a static undulation(lat, lon) callable with degree arguments
         *          returning a length (units length quantity).
         */
        template <class T>
        concept has_undulation =
            requires
        {
            // If your units library uses degree_t or degree<>, keep this aligned.
            { T::undulation(degrees{1.0}, degrees{1.0}) }
            -> std::convertible_to<meters<>>;  // assumes this is a concept; see note below
        };

        /**
         * @brief   is_geoid concept
         * @details A geoid is either the NULL_GEOID, or:
         *          - provides static undulation(lat, lon) returning length
         *          - has a reference_ellipsoid type that satisfies is_ellipsoid
         *          - is default constructible (optional; keep if you still want it)
         */
        template <class T>
        concept Geoid =
            std::same_as<T, geoids::NULL_GEOID> ||
            (
                has_undulation<T> &&
                coordinates::traits::is_ellipsoid<geoid_reference_ellipsoid_t<T>> &&
                std::is_default_constructible_v<T>
            );

        template <class T>
        inline constexpr bool is_geoid = Geoid<T>;

    }
}

#endif // geoid_h