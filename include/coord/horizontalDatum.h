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

#ifndef horizontalDatum_h__
#define horizontalDatum_h__

//------------------------
//	INCLUDES
//------------------------

#include "helmert.h"
#include "ellipsoid.h"

namespace coord
{
	//	----------------------------------------------------------------------------
	//	CLASS		HoriztontalDatum
	//  ----------------------------------------------------------------------------
	///	@brief		Represents a horizontal geodetic datum
	///	@details	A horizontal datum details how to convert from a standardized International 
	///				Terrestrial Reference frame to an arbitrary ellipsoid to  using a position 
	///				vector transformation.\n\n
	///				To create your own horizontal datum
	/// @note		
	//  ----------------------------------------------------------------------------
	template<
		class ReferenceFrame,		///< ITRF Frame that the parameters convert _from_
		class ReferenceEllipsoid,	///< Ellipsoid (or ITRF) that the parameters convert _to_
		class Tx,					///< Helmert Tx parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Ty,					///< Helmert Ty parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Tz,					///< Helmert Tz parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class S,					///< Helmert S parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Rx,					///< Helmert Rx parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Ry,					///< Helmert Ry parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Rz,					///< Helmert Rz parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Epoch = void,			///< Helmert Epoch parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Dtx = void,			///< Helmert Dtx parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Dty = void,			///< Helmert Dty parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Dtz = void,			///< Helmert Dtz parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Ds = void,			///< Helmert Ds parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Drx = void,			///< Helmert Drx parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Dry = void,			///< Helmert Dry parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
		class Drz = void			///< Helmert Drz parameter (position vector notation) in terms of _ReferenceFrame to ReferenceEllipsoid_
	>
	class HorizontalDatum : public Helmert<Tx, Ty, Tz, S, Rx, Ry, Rz, Epoch, Dtx, Dty, Dtz, Ds, Drx, Dry, Drz>
	{
	public:
		typedef ReferenceFrame		reference_frame;
		typedef ReferenceEllipsoid	reference_ellipsoid;
	};
}

//----------------------------------
//	ADDITIONAL UNIT DEFINITIONS
//----------------------------------

namespace units
{
	namespace velocity
	{
		using meters_per_year = units::compound_unit<units::length::meters, units::inverse<units::time::year>>;
		using meters_per_year_t = units::unit_t<meters_per_year>;
		using millimeters_per_year = units::compound_unit<units::length::millimeters, units::inverse<units::time::year>>;
		using millimeters_per_year_t = units::unit_t<millimeters_per_year>;
		using ppm_per_year = units::compound_unit<units::concentration::ppm, units::inverse<units::time::year>>;
		using ppm_per_year_t = units::unit_t<ppm_per_year>;
		using ppb_per_year = units::compound_unit<units::concentration::ppb, units::inverse<units::time::year>>;
		using ppb_per_year_t = units::unit_t<ppb_per_year>;
	}

	namespace angle
	{
		using mas_t = units::unit_t<units::angle::milliarcseconds>;
	}

	namespace angular_velocity
	{
		using mas_per_year = units::compound_unit<units::angle::mas, units::inverse<units::time::year>>;
		using mas_per_year_t = units::unit_t<mas_per_year>;
	}
}


namespace coord
{
	//----------------------------------
	//	ITRF TRANSFORMATIONS
	//----------------------------------

	//**************************************************************************************************
	//	NOTE:	ITRF2008 is used as the basis-realization for the coord library, however, derived 
	//			vertical datums can be specified in terms of any of the included ITRF realizations.
	//**************************************************************************************************

	namespace horizontalDatums
	{
		namespace detail
		{ 
			struct _ITRF_t {}; 
		}
	
		/**
		 * @brief		ITRF 2008 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF2008 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF2008,
			units::unit_value_t<units::length::meters, 0>,											// Tx = 0.0 m
			units::unit_value_t<units::length::meters, 0>,											// Ty = 0.0 m
			units::unit_value_t<units::length::meters, 0>,											// Tz = 0.0 m
			units::unit_value_t<units::concentration::ppb, 0>,										// S = 0.0 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.0 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.0 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rz = 0.0 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2005 
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dtx = 0.0 m/yr
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dty = 0.0 m/yr
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dtz = 0.0 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 0>,									// S = 0.0 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.0 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.0 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>							// Drz = 0.0 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 2014 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF2014 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF2014,
			units::unit_value_t<units::length::meters, -1, 625>,									// Tx = -0.00160 m
			units::unit_value_t<units::length::meters, -3, 1579>,									// Ty = -0.00190 m
			units::unit_value_t<units::length::meters, -3, 1250>,									// Tz = -0.00240 m
			units::unit_value_t<units::concentration::ppb, 1, 50>,									// S = 0.02000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rz = 0.00000 mas
			units::unit_value_t<units::time::year, 2010>,											// Epoch = 2010 
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dtx = 0.00000 m/yr
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dty = 0.00000 m/yr
			units::unit_value_t<units::velocity::meters_per_year, 1, 10000>,						// Dtz = 0.00010 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, -3, 100>,							// S = -0.03000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>							// Drz = 0.00000 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 2005 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF2005 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF2005,
			units::unit_value_t<units::length::millimeters, -2>,									// Tx = -0.00200 m
			units::unit_value_t<units::length::millimeters, -9, 10>,								// Ty = -0.00090 m
			units::unit_value_t<units::length::millimeters, -47, 10>,								// Tz = -0.00470 m
			units::unit_value_t<units::concentration::ppb, 47, 50>,									// S = 0.94000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rz = 0.00000 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000 
			units::unit_value_t<units::velocity::millimeters_per_year, 3, 10>,						// Dtx = 0.00030 m/yr
			units::unit_value_t<units::velocity::millimeters_per_year, 0>,							// Dty = 0.00000 m/yr
			units::unit_value_t<units::velocity::millimeters_per_year, 0>,							// Dtz = 0.00000 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 0>,									// S = 0.00000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>							// Drz = 0.00000 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 2000 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF2000 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF2000,
			units::unit_value_t<units::length::millimeters, -19, 10>,								// Tx = -0.00190 m
			units::unit_value_t<units::length::millimeters, -17, 10>,								// Ty = -0.00170 m
			units::unit_value_t<units::length::millimeters, -105, 10>,								// Tz = -0.01050 m
			units::unit_value_t<units::concentration::ppb, 67, 50>,									// S = 1.34000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rz = 0.00000 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000 
			units::unit_value_t<units::velocity::millimeters_per_year, 1, 10>,						// Dtx = 0.00010 m/yr
			units::unit_value_t<units::velocity::millimeters_per_year, 1, 10>,						// Dty = 0.00010 m/yr
			units::unit_value_t<units::velocity::millimeters_per_year, -18, 10>,					// Dtz = -0.00180 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 2, 25>,								// S = 0.08000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>							// Drz = 0.00000 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 97 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF97 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF97,
			units::unit_value_t<units::length::meters, 3, 625>,										// Tx = 0.00480 m
			units::unit_value_t<units::length::meters, 3, 1154>,									// Ty = 0.00260 m
			units::unit_value_t<units::length::meters, -25, 753>,									// Tz = -0.03320 m
			units::unit_value_t<units::concentration::ppb, 73, 25>,									// S = 2.92000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 3, 50>,								// Rz = 0.06000 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000 
			units::unit_value_t<units::velocity::meters_per_year, 1, 10000>,						// Dtx = 0.00010 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -1, 2000>,						// Dty = -0.00050 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -2, 625>,							// Dtz = -0.00320 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 9, 100>,								// S = 0.09000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 1, 50>						// Drz = 0.02000 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 96 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF96 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF96,
			units::unit_value_t<units::length::meters, 3, 625>,										// Tx = 0.00480 m
			units::unit_value_t<units::length::meters, 3, 1154>,									// Ty = 0.00260 m
			units::unit_value_t<units::length::meters, -25, 753>,									// Tz = -0.03320 m
			units::unit_value_t<units::concentration::ppb, 73, 25>,									// S = 2.92000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 3, 50>,								// Rz = 0.06000 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000 
			units::unit_value_t<units::velocity::meters_per_year, 1, 10000>,						// Dtx = 0.00010 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -1, 2000>,						// Dty = -0.00050 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -2, 625>,							// Dtz = -0.00320 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 9, 100>,								// S = 0.09000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 1, 50>						// Drz = 0.02000 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 94 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF94 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF94,
			units::unit_value_t<units::length::meters, 3, 625>,										// Tx = 0.00480 m
			units::unit_value_t<units::length::meters, 3, 1154>,									// Ty = 0.00260 m
			units::unit_value_t<units::length::meters, -25, 753>,									// Tz = -0.03320 m
			units::unit_value_t<units::concentration::ppb, 73, 25>,									// S = 2.92000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 3, 50>,								// Rz = 0.06000 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000 
			units::unit_value_t<units::velocity::meters_per_year, 1, 10000>,						// Dtx = 0.00010 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -1, 2000>,						// Dty = -0.00050 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -2, 625>,							// Dtz = -0.00320 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 9, 100>,								// S = 0.09000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 1, 50>						// Drz = 0.02000 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 93 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF93 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF93,
			units::unit_value_t<units::length::meters, -3, 125>,									// Tx = -0.02400 m
			units::unit_value_t<units::length::meters, 3, 1250>,									// Ty = 0.00240 m
			units::unit_value_t<units::length::meters, -43, 1114>,									// Tz = -0.03860 m
			units::unit_value_t<units::concentration::ppb, 341, 100>,								// S = 3.41000 ppb
			units::unit_value_t<units::angle::milliarcseconds, -171, 100>,							// Rx = -1.71000 mas
			units::unit_value_t<units::angle::milliarcseconds, -37, 25>,							// Ry = -1.48000 mas
			units::unit_value_t<units::angle::milliarcseconds, -3, 10>,								// Rz = -0.30000 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000 
			units::unit_value_t<units::velocity::meters_per_year, -7, 2500>,						// Dtx = -0.00280 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -1, 10000>,						// Dty = -0.00010 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -3, 1250>,						// Dtz = -0.00240 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 9, 100>,								// S = 0.09000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, -11, 100>,					// Drx = -0.11000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, -19, 100>,					// Dry = -0.19000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 7, 100>						// Drz = 0.07000 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 92 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF92 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF92,
			units::unit_value_t<units::length::meters, 8, 625>,										// Tx = 0.01280 m
			units::unit_value_t<units::length::meters, 5, 1087>,									// Ty = 0.00460 m
			units::unit_value_t<units::length::meters, -103, 2500>,									// Tz = -0.04120 m
			units::unit_value_t<units::concentration::ppb, 221, 100>,								// S = 2.21000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 3, 50>,								// Rz = 0.06000 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000 
			units::unit_value_t<units::velocity::meters_per_year, 1, 10000>,						// Dtx = 0.00010 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -1, 2000>,						// Dty = -0.00050 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -2, 625>,							// Dtz = -0.00320 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 9, 100>,								// S = 0.09000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 1, 50>						// Drz = 0.02000 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 91 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF91 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF91,
			units::unit_value_t<units::length::meters, 31, 1250>,									// Tx = 0.02480 m
			units::unit_value_t<units::length::meters, 17, 914>,									// Ty = 0.01860 m
			units::unit_value_t<units::length::meters, -59, 1250>,									// Tz = -0.04720 m
			units::unit_value_t<units::concentration::ppb, 361, 100>,								// S = 3.61000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 3, 50>,								// Rz = 0.06000 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000 
			units::unit_value_t<units::velocity::meters_per_year, 1, 10000>,						// Dtx = 0.00010 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -1, 2000>,						// Dty = -0.00050 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -2, 625>,							// Dtz = -0.00320 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 9, 100>,								// S = 0.09000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 1, 50>						// Drz = 0.02000 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 90 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF90 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF90,
			units::unit_value_t<units::length::meters, 57, 2500>,									// Tx = 0.02280 m
			units::unit_value_t<units::length::meters, 73, 5000>,									// Ty = 0.01460 m
			units::unit_value_t<units::length::meters, -79, 1250>,									// Tz = -0.06320 m
			units::unit_value_t<units::concentration::ppb, 391, 100>,								// S = 3.91000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 3, 50>,								// Rz = 0.06000 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000 
			units::unit_value_t<units::velocity::meters_per_year, 1, 10000>,						// Dtx = 0.00010 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -1, 2000>,						// Dty = -0.00050 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -2, 625>,							// Dtz = -0.00320 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 9, 100>,								// S = 0.09000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 1, 50>						// Drz = 0.02000 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 89 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF89 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF89,
			units::unit_value_t<units::length::meters, 35, 1259>,									// Tx = 0.02780 m
			units::unit_value_t<units::length::meters, 43, 1114>,									// Ty = 0.03860 m
			units::unit_value_t<units::length::meters, -59, 583>,									// Tz = -0.10120 m
			units::unit_value_t<units::concentration::ppb, 731, 100>,								// S = 7.31000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 3, 50>,								// Rz = 0.06000 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000 
			units::unit_value_t<units::velocity::meters_per_year, 1, 10000>,						// Dtx = 0.00010 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -1, 2000>,						// Dty = -0.00050 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -2, 625>,							// Dtz = -0.00320 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 9, 100>,								// S = 0.09000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 1, 50>						// Drz = 0.02000 mas/yr
		> {};
	
		/**
		 * @brief		ITRF 88 Realization
		 * @details		The International Terrestrial Reference System (ITRS) describes procedures for
		 *				creating reference frames suitable for use with measurements on or near the Earth's
		 *				surface. This is done in much the same way that a physical standard might be described
		 *				as a set of procedures for creating a realization of that standard. The ITRS defines
		 *				a geocentric system of coordinates using the SI system of measurement.\n\n
		 *				An International Terrestrial Reference Frame (ITRF) is a realization of the ITRS.
		 *				New ITRF solutions are produced every few years, using the latest mathematical and
		 *				surveying techniques to attempt to realize the ITRS as precisely as possible.
		 *				Due to experimental error, any given ITRF will differ very slightly from any other
		 *				realization of the ITRF.\n\n
		 *				Practical navigation systems are in general referenced to a specific ITRF solution,
		 *				or to their own coordinate systems which are then referenced to an ITRF solution.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct ITRF88 : detail::_ITRF_t, HorizontalDatum<
			ITRF2008,
			ITRF88,
			units::unit_value_t<units::length::meters, 57, 2500>,									// Tx = 0.02280 m
			units::unit_value_t<units::length::meters, 3, 1154>,									// Ty = 0.00260 m
			units::unit_value_t<units::length::meters, -78, 623>,									// Tz = -0.12520 m
			units::unit_value_t<units::concentration::ppb, 1041, 100>,								// S = 10.41000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 1, 10>,								// Rx = 0.10000 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.00000 mas
			units::unit_value_t<units::angle::milliarcseconds, 3, 50>,								// Rz = 0.06000 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000 
			units::unit_value_t<units::velocity::meters_per_year, 1, 10000>,						// Dtx = 0.00010 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -1, 2000>,						// Dty = -0.00050 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -2, 625>,							// Dtz = -0.00320 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 9, 100>,								// S = 0.09000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.00000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 1, 50>						// Drz = 0.02000 mas/yr
		> {};
	
	
		//----------------------------------
		//	COMMON HORIZONTAL DATUMS
		//----------------------------------
	
		/**
		 * @brief		WGS84 Horizontal Datum for realization G1674 (ITR2008)
		 * @details		WGS84 is an Earth-centered, Earth-fixed terrestrial reference system and geodetic
		 *				datum. WGS84 is based on a consistent set of constants and model parameters that
		 *				describe the Earth's size, shape, and gravity and geomagnetic fields. WGS84 is
		 *				the standard U.S. Department of Defense definition of a global reference system
		 *				for geospatial information and is the reference system for the Global Positioning
		 *				System (GPS). It is compatible with the International Terrestrial Reference System
		 *				(ITRS). The current realization WGS84 (G1674) follows the criteria outlined in
		 *				the International Earth Rotation Service (IERS) Technical Note 21 (TN 21).
		 *				The responsible organization is the National Geospatial-Intelligence Agency (NGA).
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855173
		 *				Section: Transformation parameters.
		 */
		struct WGS84_G1674 : HorizontalDatum<
			ITRF2008,
			ellipsoids::WGS84,
			units::unit_value_t<units::length::meters, 0>,											// Tx = 0 m
			units::unit_value_t<units::length::meters, 0>,											// Ty = 0 m
			units::unit_value_t<units::length::meters, 0>,											// Tz = 0 m
			units::unit_value_t<units::concentration::ppb, 0>,										// S = 0 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rz = 0 mas
			units::unit_value_t<units::time::year, 2005>,											// Epoch = 0 
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dtx = 0 m/yr
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dty = 0 m/yr
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dtz = 0 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 0>,									// S = 0 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>							// Drz = 0 mas/yr
		> {};
	
		/**
		 * @brief		North American Datum 1983 (CORS2011)
		 * @details		The North American Datum (NAD) is the datum now used to define the geodetic 
		 *				network in North America. A datum is a formal description of the shape of the 
		 *				Earth along with an "anchor" point for the coordinate system. In surveying, 
		 *				cartography, and land-use planning, two North American Datums are in use: the 
		 *				North American Datum of 1927 (NAD27) and the North American Datum of 1983 (NAD83). 
		 *				Both are geodetic reference systems based on slightly different assumptions and 
		 *				measurements.
		 *				Helmert Coefficients for an ITRF2008 to NAD83 (1997) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		 */
		struct NAD83 : HorizontalDatum<
			ITRF2008,
			ellipsoids::GRS80,
			units::unit_value_t<units::length::meters, 756, 761>,									// Tx = 0.99343 m
			units::unit_value_t<units::length::meters, -2185, 1148>,								// Ty = -1.90331 m
			units::unit_value_t<units::length::meters, -1775, 3371>,								// Tz = -0.52655 m
			units::unit_value_t<units::concentration::ppb, 650, 379>,								// S = 1.71504 ppb
			units::unit_value_t<units::angle::milliarcseconds, -17311, 668>,						// Rx = -25.91467 mas
			units::unit_value_t<units::angle::milliarcseconds, -6985, 741>,							// Ry = -9.42645 mas
			units::unit_value_t<units::angle::milliarcseconds, -24985, 2154>,						// Rz = -11.59935 mas
			units::unit_value_t<units::time::year, 1997>,											// Epoch = 1997 
			units::unit_value_t<units::velocity::meters_per_year, 1, 1266>,							// Dtx = 0.00079 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -1, 1667>,						// Dty = -0.00060 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -1, 746>,							// Dtz = -0.00134 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, -66, 647>,							// S = -0.10201 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, -1333, 19994>,				// Drx = -0.06667 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 509, 672>,					// Dry = 0.75744 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 27, 526>						// Drz = 0.05133 mas/yr
		> {};
	
		/**
		* @brief		IGS 2008 Realization
		* @details		GPS satellite ephemerides produced by US NGS and other International GNSS 
		*				Service (IGS) analysis centers are expressed in this CRS. 
		*				For practical purposes this is coincident with 
		*				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		*				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (NAD83 sheet)".
		*/
		struct IGS08 : HorizontalDatum<
			ITRF2008,
			ellipsoids::GRS80,
			units::unit_value_t<units::length::meters, 0>,											// Tx = 0.0 m
			units::unit_value_t<units::length::meters, 0>,											// Ty = 0.0 m
			units::unit_value_t<units::length::meters, 0>,											// Tz = 0.0 m
			units::unit_value_t<units::concentration::ppb, 0>,										// S = 0.0 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0.0 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0.0 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rz = 0.0 mas
			units::unit_value_t<units::time::year, 2005>,											// Epoch = 2005 
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dtx = 0.0 m/yr
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dty = 0.0 m/yr
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dtz = 0.0 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 0>,									// S = 0.0 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0.0 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0.0 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>							// Drz = 0.0 mas/yr
		> {};

		/**
		 * @brief		Geodetic Datum of Australia (1994)
		 * @details		Source: http://www.epsg-registry.org/ (Code:6276)
		 * @note			All EPSG helmert parameters are given in the coordinate-frame rotation notation, while
		 *				this library uses the position-vector frame notation. Thus, all of the rotation
		 *				and delta-rotation parameters have had their signs reversed compared to the database
		 *				parameters.
		 */
		struct GDA94 : HorizontalDatum<
			ITRF2008,
			ellipsoids::GRS80,
			units::unit_value_t<units::length::millimeter, -2117, 25>,								// Tx = -84.68 mm
			units::unit_value_t<units::length::millimeter, -971, 50>,								// Ty = -19.42 mm
			units::unit_value_t<units::length::millimeter, 3201, 100>,								// Tz = 32.01 mm
			units::unit_value_t<units::concentration::ppm, 75, 7724>,								// S = 0.00971 ppm
			units::unit_value_t<units::angle::milliarcseconds, 479, 1126>,							// Rx = 0.4254 mas
			units::unit_value_t<units::angle::milliarcseconds, -1375, 609>,							// Ry = -2.2578 mas
			units::unit_value_t<units::angle::milliarcseconds, -1280, 533>,							// Rz = -2.4015 mas
			units::unit_value_t<units::time::year, 1994>,											// Epoch = 1994
			units::unit_value_t<units::velocity::millimeters_per_year, 71, 50>,						// Dtx = 1.42 mm/yr
			units::unit_value_t<units::velocity::millimeters_per_year, 67, 50>,						// Dty = 1.34 mm/yr
			units::unit_value_t<units::velocity::millimeters_per_year, 9, 10>,						// Dtz = 0.90 mm/yr
			units::unit_value_t<units::velocity::ppm_per_year, 1, 9174>,							// S = 0.000109 ppm/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, -218, 141>,					// Drx = -1.5461 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, -591, 500>,					// Dry = -1.1820 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, -2197, 1902>					// Drz = -1.1551 mas/yr
		> {};
	
		/**
		 * @brief		European Terrestrial Reference System 1989
		 * @details		The European Terrestrial Reference System 1989 (ETRS89) is an ECEF (Earth-Centered, 
		 *				Earth-Fixed) geodetic Cartesian reference frame, in which the Eurasian Plate as 
		 *				a whole is static. The coordinates and maps in Europe based on ETRS89 are not
		 *				subject to change due to the continental drift.
		 *				Helmert Coefficients for an ITRF2008 to ETRS89 (2000) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (ETRS89 sheet)".
		 */
		struct ETRS89 : HorizontalDatum<
			ITRF89,
			ellipsoids::GRS80,
			units::unit_value_t<units::length::meters, 0>,											// Tx = 0 m
			units::unit_value_t<units::length::meters, 0>,											// Ty = 0 m
			units::unit_value_t<units::length::meters, 0>,											// Tz = 0 m
			units::unit_value_t<units::concentration::ppb, 0>,										// S = 0 ppb
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rx = 0 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Ry = 0 mas
			units::unit_value_t<units::angle::milliarcseconds, 0>,									// Rz = 0 mas
			units::unit_value_t<units::time::year, 1989>,											// Epoch = 1989.00 
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dtx = 0 m/yr
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dty = 0 m/yr
			units::unit_value_t<units::velocity::meters_per_year, 0>,								// Dtz = 0 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 0>,									// S = 0 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Drx = 0 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,							// Dry = 0 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>							// Drz = 0 mas/yr
		> {};
	
		/**
		 * @brief		European Terrestrial Reference Frame 2000
		 * @details		The European Terrestrial Reference System 1989 (ETRS89) is an ECEF (Earth-Centered,
		 *				Earth-Fixed) geodetic Cartesian reference frame, in which the Eurasian Plate as
		 *				a whole is static. The coordinates and maps in Europe based on ETRS89 are not
		 *				subject to change due to the continental drift.
		 *				Helmert Coefficients for an ITRF2008 to ETRS89 (2000) conversion
		 *				Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855153#NorthAmericanDatum1983%28NAD83%29-Transformationparameters
		 *				Section: Transformation parameters, link to "ITRF Transformation Parameters.xlsx (ETRS89 sheet)".
		 */
		struct ETRF2000 : HorizontalDatum<
			ITRF2008,
			ellipsoids::GRS80,
			units::unit_value_t<units::length::meters, 31, 595>,									// Tx = 0.05210 m
			units::unit_value_t<units::length::meters, 81, 1643>,									// Ty = 0.04930 m
			units::unit_value_t<units::length::meters, -32, 547>,									// Tz = -0.05850 m
			units::unit_value_t<units::concentration::ppb, 67, 50>,									// S = 1.34000 ppb
			units::unit_value_t<units::angle::milliarcseconds, 891, 1000>,							// Rx = 0.89100 mas
			units::unit_value_t<units::angle::milliarcseconds, 539, 100>,							// Ry = 5.39000 mas
			units::unit_value_t<units::angle::milliarcseconds, -1089, 125>,							// Rz = -8.71200 mas
			units::unit_value_t<units::time::year, 2000>,											// Epoch = 2000.00 
			units::unit_value_t<units::velocity::meters_per_year, 1, 10000>,						// Dtx = 0.00010 m/yr
			units::unit_value_t<units::velocity::meters_per_year, 1, 10000>,						// Dty = 0.00010 m/yr
			units::unit_value_t<units::velocity::meters_per_year, -2, 1111>,						// Dtz = -0.00180 m/yr
			units::unit_value_t<units::velocity::ppb_per_year, 2, 25>,								// S = 0.08000 ppb/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 81, 1000>,					// Drx = 0.08100 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, 49, 100>,					// Dry = 0.49000 mas/yr
			units::unit_value_t<units::angular_velocity::mas_per_year, -99, 125>					// Drz = -0.79200 mas/yr
		> {};

		/**
		* @brief		International Terrestrial Reference System 2008
		* @details		Realization of the ITRF2008 Frame using the GRS80 ellipsoid.
		*/
		struct ITRS2008 : HorizontalDatum<
			ITRF2008,
			ellipsoids::GRS80,
			units::unit_value_t<units::length::meters, 0>,
			units::unit_value_t<units::length::meters, 0>,
			units::unit_value_t<units::length::meters, 0>,
			units::unit_value_t<units::concentration::ppb, 0>,
			units::unit_value_t<units::angle::milliarcseconds, 0>,
			units::unit_value_t<units::angle::milliarcseconds, 0>,
			units::unit_value_t<units::angle::milliarcseconds, 0>,
			units::unit_value_t<units::time::year, 2000>,
			units::unit_value_t<units::velocity::meters_per_year, 0>,
			units::unit_value_t<units::velocity::meters_per_year, 0>,
			units::unit_value_t<units::velocity::meters_per_year, 0>,
			units::unit_value_t<units::velocity::ppb_per_year, 0>,
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>
		> {};

		/**
		 * @brief		International Terrestrial Reference System 2000
		 * @details		Realization of the ITRF2000 Frame using the GRS80 ellipsoid.
		 */
		struct ITRS2000 : HorizontalDatum<
			ITRF2000,
			ellipsoids::GRS80,
			units::unit_value_t<units::length::meters, 0>,									
			units::unit_value_t<units::length::meters, 0>,									
			units::unit_value_t<units::length::meters, 0>,									
			units::unit_value_t<units::concentration::ppb, 0>,								
			units::unit_value_t<units::angle::milliarcseconds, 0>,							
			units::unit_value_t<units::angle::milliarcseconds, 0>,							
			units::unit_value_t<units::angle::milliarcseconds, 0>,							
			units::unit_value_t<units::time::year, 2000>,									
			units::unit_value_t<units::velocity::meters_per_year, 0>,						
			units::unit_value_t<units::velocity::meters_per_year, 0>,						
			units::unit_value_t<units::velocity::meters_per_year, 0>,						
			units::unit_value_t<units::velocity::ppb_per_year, 0>,							
			units::unit_value_t<units::angular_velocity::mas_per_year,0>,					
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,					
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>					
		> {};

		/**
		* @brief		International Terrestrial Reference System 1988
		* @details		Realization of the ITRF2000 Frame using the GRS80 ellipsoid.
		*/
		struct ITRS88 : HorizontalDatum<
			ITRF88,
			ellipsoids::GRS80,
			units::unit_value_t<units::length::meters, 0>,
			units::unit_value_t<units::length::meters, 0>,
			units::unit_value_t<units::length::meters, 0>,
			units::unit_value_t<units::concentration::ppb, 0>,
			units::unit_value_t<units::angle::milliarcseconds, 0>,
			units::unit_value_t<units::angle::milliarcseconds, 0>,
			units::unit_value_t<units::angle::milliarcseconds, 0>,
			units::unit_value_t<units::time::year, 2000>,
			units::unit_value_t<units::velocity::meters_per_year, 0>,
			units::unit_value_t<units::velocity::meters_per_year, 0>,
			units::unit_value_t<units::velocity::meters_per_year, 0>,
			units::unit_value_t<units::velocity::ppb_per_year, 0>,
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>,
			units::unit_value_t<units::angular_velocity::mas_per_year, 0>
		> {};
	}

	//----------------------------------
	//	TYPE TRAITS
	//----------------------------------

	namespace traits
	{
		template<class T, typename = void>
		struct horizontal_datum_traits
		{
			typedef void reference_frame;
			typedef void reference_ellipsoid;
		};
	
		template<class T>
		struct horizontal_datum_traits
			<T, typename void_type<
			typename T::reference_frame,
			typename T::reference_ellipsoid>::type>
		{
			typedef typename T::reference_frame reference_frame;									///< Unit type that the unit was derived from. May be a `base_unit` or another `unit`. Use the `base_unit_of` trait to find the SI base unit type. This will be `void` if type `T` is not a unit.
			typedef typename T::reference_ellipsoid reference_ellipsoid;							///< `std::ratio` representing a datum translation to the base unit (i.e. degrees C to degrees F conversion). This will be `void` if type `T` is not a unit.
		};
	
		/**
		 * @brief		Trait which tests is type `T` is an ITRF datum
		 * @details		To be an ITRF, an object must:
		 *				- be derived from `detail::_ITRF_t`
		 *				- be a 7 (or 14) parameter helmert transform
		 *				- have a reference_frame typedef which is another ITRF
		 *				- have a reference_ellipsoid typedef which is another ITRF
		 *
		 */
		template<typename T>
		struct is_itrf : std::integral_constant<bool,
			std::is_base_of<coord::horizontalDatums::detail::_ITRF_t, T>::value &&
			coord::traits::is_helmert_7param_coefficients<T>::value &&
			!std::is_same<typename coord::traits::horizontal_datum_traits<T>::reference_frame, void>::value &&
			!std::is_same<typename coord::traits::horizontal_datum_traits<T>::reference_ellipsoid, void>::value>
		{};
	
		/**
		 * @brief		Type trait which tests if `T` is a horizontal datum
		 * @details		To be a horizontal datum, and object must:
		 *				-	have a `reference_ellipsoid` typedef which contains the datums ellipsoid
		 *					definition.
		 *				-	have a `reference_frame` typedef which contains the datum's reference ITRF
		 *					frame.
		 *				-	be a 7 (or 14) parameter helmert transform, whose coefficients represent the
		 *					transformation from `reference_frame` to `reference_ellipsoid`.
		 */
		template<typename T>
		struct is_horizontal_datum : std::integral_constant<bool,
			coord::traits::is_helmert_7param_coefficients<T>::value &&
			is_itrf<typename coord::traits::horizontal_datum_traits<T>::reference_frame>::value &&
			(traits::is_ellipsoid<typename traits::horizontal_datum_traits<T>::reference_ellipsoid>::value ||
			is_itrf<typename traits::horizontal_datum_traits<T>::reference_ellipsoid>::value)
		> {};
	}
}

#endif // horizontalDatum_h__