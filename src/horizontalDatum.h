#ifndef horizontalDatum_h
#define horizontalDatum_h

//------------------------
//	INCLUDES
//------------------------

#include "ellipsoid.h"
#include "helmert.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		HorizontalDatum
	//  ----------------------------------------------------------------------------
	///	@brief		Represents a horizontal geodetic datum
	///	@details	A horizontal datum details how to convert from a standardized International
	///				Terrestrial Reference frame to an arbitrary ellipsoid to  using a position
	///				vector transformation.\n\n
	///				To create your own horizontal datum
	/// @note
	//  ----------------------------------------------------------------------------
	template<class ReferenceFrame,        ///< ITRF Frame that the parameters convert _from_
	         class ReferenceEllipsoid,    ///< Ellipsoid (or ITRF) that the parameters convert _to_
	         meters<double>            Tx,
	         meters<double>            Ty,
	         meters<double>            Tz,
	         parts_per_billion<double> ScaleFactor,
	         milliarcseconds<double>   Rx,
	         milliarcseconds<double>   Ry,
	         milliarcseconds<double>   Rz,
	         years<double>             Epoch = years{2000.0},
	         meters_per_year           Dtx   = meters_per_year{0.0},
	         meters_per_year           Dty   = meters_per_year{0.0},
	         meters_per_year           Dtz   = meters_per_year{0.0},
	         ppb_per_year              Ds    = ppb_per_year{0.0},
	         mas_per_year              Drx   = mas_per_year{0.0},
	         mas_per_year              Dry   = mas_per_year{0.0},
	         mas_per_year              Drz   = mas_per_year{0.0}>
	class HorizontalDatum : public Helmert<Tx, Ty, Tz, ScaleFactor, Rx, Ry, Rz, Epoch, Dtx, Dty, Dtz, Ds, Drx, Dry, Drz>
	{
	public:
		using reference_frame     = ReferenceFrame;
		using reference_ellipsoid = ReferenceEllipsoid;
	};
}    // namespace coordinates

inline namespace coordinates
{
	//----------------------------------
	//	ITRF TRANSFORMATIONS
	//----------------------------------

	//**************************************************************************************************
	//	NOTE:	ITRF2008 is used as the basis-realization for the coordinates library, however, derived
	//			vertical datums can be specified in terms of any of the included ITRF realizations.
	//**************************************************************************************************

	namespace horizontalDatums
	{
		namespace detail
		{
			struct _ITRF_t
			{
			};
		}    // namespace detail

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
		 *				ITRF2008 is the library's basis frame, so this is the identity (zero) transform.
		 */
		struct ITRF2008 : detail::_ITRF_t,
		                  HorizontalDatum<ITRF2008,
		                                  ITRF2008,
		                                  0.0_m,      // Tx = 0.0 m
		                                  0.0_m,      // Ty = 0.0 m
		                                  0.0_m,      // Tz = 0.0 m
		                                  0.0_ppb,    // S  = 0.0 ppb
		                                  0.0_mas,    // Rx = 0.0 mas
		                                  0.0_mas,    // Ry = 0.0 mas
		                                  0.0_mas,    // Rz = 0.0 mas
		                                  2000_yr     // Epoch = 2000
		                                  >
		{
		};

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
		 *				Parameters below are the ITRF2008 -> ITRF2014 tie (inverse of the IGN table row).
		 *				Primary source: IERS/IGN Transfo-ITRF2014_ITRFs.txt,
		 *				https://itrf.ign.fr/docs/solutions/itrf2014/Transfo-ITRF2014_ITRFs.txt
		 *				(coordinate-frame; reference epoch 2010.0). Rotations are zero, so convention is moot.
		 */
		struct ITRF2014 : detail::_ITRF_t,
		                  HorizontalDatum<ITRF2008,
		                                  ITRF2014,
		                                  -0.00160_m,                  // Tx = -0.00160 m
		                                  -0.00190_m,                  // Ty = -0.00190 m
		                                  -0.00240_m,                  // Tz = -0.00240 m
		                                  0.02000_ppb,                 // S  = 0.02000 ppb
		                                  0.0_mas,                     // Rx = 0.00000 mas
		                                  0.0_mas,                     // Ry = 0.00000 mas
		                                  0.0_mas,                     // Rz = 0.00000 mas
		                                  2010_yr,                     // Epoch = 2010
		                                  meters_per_year{0.0},        // Dtx = 0.00000 m/yr
		                                  meters_per_year{0.0},        // Dty = 0.00000 m/yr
		                                  meters_per_year{0.00010},    // Dtz = 0.00010 m/yr
		                                  ppb_per_year{-0.03000},      // Ds  = -0.03000 ppb/yr
		                                  mas_per_year{0.0},           // Drx = 0.00000 mas/yr
		                                  mas_per_year{0.0},           // Dry = 0.00000 mas/yr
		                                  mas_per_year{0.0}            // Drz = 0.00000 mas/yr
		                                  >
		{
		};

		/**
		 * @brief		ITRF 2020 Realization
		 * @details		Realization of the ITRS released by IERS/IGN.
		 *
		 *				Parameters below are ITRF2008 -> ITRF2020 (inverse of IGN table row "ITRF2008").
		 *				Units: meters, ppb, mas; rates: m/yr, ppb/yr, mas/yr.
		 *				Primary source: IERS/IGN Transfo-ITRF2020_TRFs.txt,
		 *				https://itrf.ign.fr/docs/solutions/itrf2020/Transfo-ITRF2020_TRFs.txt
		 *				(coordinate-frame; reference epoch 2015.0). Rotations are zero, so convention is moot.
		 */
		struct ITRF2020 : detail::_ITRF_t,
		                  HorizontalDatum<ITRF2008,
		                                  ITRF2020,
		                                  -0.00020_m,                   // Tx = -0.2 mm
		                                  -0.00100_m,                   // Ty = -1.0 mm
		                                  -0.00330_m,                   // Tz = -3.3 mm
		                                  0.29000_ppb,                  // S  = +0.29 ppb   (inverse of -0.29 ppb)
		                                  0.0_mas,                      // Rx = 0.0 mas
		                                  0.0_mas,                      // Ry = 0.0 mas
		                                  0.0_mas,                      // Rz = 0.0 mas
		                                  2015_yr,                      // Epoch = 2015.0
		                                  meters_per_year{0.0},         // Dtx = -0.0 mm/yr
		                                  meters_per_year{0.00010},     // Dty = +0.1 mm/yr
		                                  meters_per_year{-0.00010},    // Dtz = -0.1 mm/yr
		                                  ppb_per_year{-0.03000},       // Ds  = -0.03 ppb/yr (inverse of +0.03)
		                                  mas_per_year{0.0},            // Drx = 0.0 mas/yr
		                                  mas_per_year{0.0},            // Dry = 0.0 mas/yr
		                                  mas_per_year{0.0}             // Drz = 0.0 mas/yr
		                                  >
		{
		};

		/**
		 * @brief		ITRF 2005 Realization
		 * @details		(see ITRF2008 block for full background)
		 */
		struct ITRF2005 : detail::_ITRF_t,
		                  HorizontalDatum<ITRF2008,
		                                  ITRF2005,
		                                  -0.00200_m,                  // Tx = -0.00200 m
		                                  -0.00090_m,                  // Ty = -0.00090 m
		                                  -0.00470_m,                  // Tz = -0.00470 m
		                                  0.94000_ppb,                 // S  = 0.94000 ppb
		                                  0.0_mas,                     // Rx = 0.00000 mas
		                                  0.0_mas,                     // Ry = 0.00000 mas
		                                  0.0_mas,                     // Rz = 0.00000 mas
		                                  2000_yr,                     // Epoch = 2000
		                                  meters_per_year{0.00030},    // Dtx = 0.00030 m/yr
		                                  meters_per_year{0.0},        // Dty = 0.00000 m/yr
		                                  meters_per_year{0.0},        // Dtz = 0.00000 m/yr
		                                  ppb_per_year{0.0},           // Ds  = 0.00000 ppb/yr
		                                  mas_per_year{0.0},           // Drx = 0.00000 mas/yr
		                                  mas_per_year{0.0},           // Dry = 0.00000 mas/yr
		                                  mas_per_year{0.0}            // Drz = 0.00000 mas/yr
		                                  >
		{
		};

		/**
		 * @brief		ITRF 2000 Realization
		 * @details		(see ITRF2008 block for full background)
		 */
		struct ITRF2000 : detail::_ITRF_t,
		                  HorizontalDatum<ITRF2008,
		                                  ITRF2000,
		                                  -0.00190_m,                   // Tx = -0.00190 m
		                                  -0.00170_m,                   // Ty = -0.00170 m
		                                  -0.01050_m,                   // Tz = -0.01050 m
		                                  1.34000_ppb,                  // S  = 1.34000 ppb
		                                  0.0_mas,                      // Rx = 0.00000 mas
		                                  0.0_mas,                      // Ry = 0.00000 mas
		                                  0.0_mas,                      // Rz = 0.00000 mas
		                                  2000_yr,                      // Epoch = 2000
		                                  meters_per_year{0.00010},     // Dtx = 0.00010 m/yr
		                                  meters_per_year{0.00010},     // Dty = 0.00010 m/yr
		                                  meters_per_year{-0.00180},    // Dtz = -0.00180 m/yr
		                                  ppb_per_year{0.08000},        // Ds  = 0.08000 ppb/yr
		                                  mas_per_year{0.0},            // Drx = 0.00000 mas/yr
		                                  mas_per_year{0.0},            // Dry = 0.00000 mas/yr
		                                  mas_per_year{0.0}             // Drz = 0.00000 mas/yr
		                                  >
		{
		};

		/**
		 * @brief		ITRF 97 Realization
		 * @details		(see ITRF2008 block for full background)
		 */
		struct ITRF97 : detail::_ITRF_t,
		                HorizontalDatum<ITRF2008,
		                                ITRF97,
		                                0.00480_m,                    // Tx = 0.00480 m
		                                0.00260_m,                    // Ty = 0.00260 m
		                                -0.03320_m,                   // Tz = -0.03320 m
		                                2.92000_ppb,                  // S  = 2.92000 ppb
		                                0.0_mas,                      // Rx = 0.00000 mas
		                                0.0_mas,                      // Ry = 0.00000 mas
		                                0.06000_mas,                  // Rz = 0.06000 mas
		                                2000_yr,                      // Epoch = 2000
		                                meters_per_year{0.00010},     // Dtx = 0.00010 m/yr
		                                meters_per_year{-0.00050},    // Dty = -0.00050 m/yr
		                                meters_per_year{-0.00320},    // Dtz = -0.00320 m/yr
		                                ppb_per_year{0.09000},        // Ds  = 0.09000 ppb/yr
		                                mas_per_year{0.0},            // Drx = 0.00000 mas/yr
		                                mas_per_year{0.0},            // Dry = 0.00000 mas/yr
		                                mas_per_year{0.02000}         // Drz = 0.02000 mas/yr
		                                >
		{
		};

		/**
		 * @brief		ITRF 96 Realization
		 * @details		(see ITRF2008 block for full background)
		 */
		struct ITRF96 : detail::_ITRF_t,
		                HorizontalDatum<ITRF2008,
		                                ITRF96,
		                                0.00480_m,                    // Tx = 0.00480 m
		                                0.00260_m,                    // Ty = 0.00260 m
		                                -0.03320_m,                   // Tz = -0.03320 m
		                                2.92000_ppb,                  // S  = 2.92000 ppb
		                                0.0_mas,                      // Rx = 0.00000 mas
		                                0.0_mas,                      // Ry = 0.00000 mas
		                                0.06000_mas,                  // Rz = 0.06000 mas
		                                2000_yr,                      // Epoch = 2000
		                                meters_per_year{0.00010},     // Dtx = 0.00010 m/yr
		                                meters_per_year{-0.00050},    // Dty = -0.00050 m/yr
		                                meters_per_year{-0.00320},    // Dtz = -0.00320 m/yr
		                                ppb_per_year{0.09000},        // Ds  = 0.09000 ppb/yr
		                                mas_per_year{0.0},            // Drx = 0.00000 mas/yr
		                                mas_per_year{0.0},            // Dry = 0.00000 mas/yr
		                                mas_per_year{0.02000}         // Drz = 0.02000 mas/yr
		                                >
		{
		};

		/**
		 * @brief		ITRF 94 Realization
		 * @details		(see ITRF2008 block for full background)
		 */
		struct ITRF94 : detail::_ITRF_t,
		                HorizontalDatum<ITRF2008,
		                                ITRF94,
		                                0.00480_m,                    // Tx = 0.00480 m
		                                0.00260_m,                    // Ty = 0.00260 m
		                                -0.03320_m,                   // Tz = -0.03320 m
		                                2.92000_ppb,                  // S  = 2.92000 ppb
		                                0.0_mas,                      // Rx = 0.00000 mas
		                                0.0_mas,                      // Ry = 0.00000 mas
		                                0.06000_mas,                  // Rz = 0.06000 mas
		                                2000_yr,                      // Epoch = 2000
		                                meters_per_year{0.00010},     // Dtx = 0.00010 m/yr
		                                meters_per_year{-0.00050},    // Dty = -0.00050 m/yr
		                                meters_per_year{-0.00320},    // Dtz = -0.00320 m/yr
		                                ppb_per_year{0.09000},        // Ds  = 0.09000 ppb/yr
		                                mas_per_year{0.0},            // Drx = 0.00000 mas/yr
		                                mas_per_year{0.0},            // Dry = 0.00000 mas/yr
		                                mas_per_year{0.02000}         // Drz = 0.02000 mas/yr
		                                >
		{
		};

		/**
		 * @brief		ITRF 93 Realization
		 * @details		(see ITRF2008 block for full background)
		 */
		struct ITRF93 : detail::_ITRF_t,
		                HorizontalDatum<ITRF2008,
		                                ITRF93,
		                                -0.02400_m,                   // Tx = -0.02400 m
		                                0.00240_m,                    // Ty = 0.00240 m
		                                -0.03860_m,                   // Tz = -0.03860 m
		                                3.41000_ppb,                  // S  = 3.41000 ppb
		                                -1.71000_mas,                 // Rx = -1.71000 mas
		                                -1.48000_mas,                 // Ry = -1.48000 mas
		                                -0.30000_mas,                 // Rz = -0.30000 mas
		                                2000_yr,                      // Epoch = 2000
		                                meters_per_year{-0.00280},    // Dtx = -0.00280 m/yr
		                                meters_per_year{-0.00010},    // Dty = -0.00010 m/yr
		                                meters_per_year{-0.00240},    // Dtz = -0.00240 m/yr
		                                ppb_per_year{0.09000},        // Ds  = 0.09000 ppb/yr
		                                mas_per_year{-0.11000},       // Drx = -0.11000 mas/yr
		                                mas_per_year{-0.19000},       // Dry = -0.19000 mas/yr
		                                mas_per_year{0.07000}         // Drz = 0.07000 mas/yr
		                                >
		{
		};

		/**
		 * @brief		ITRF 92 Realization
		 * @details		(see ITRF2008 block for full background)
		 */
		struct ITRF92 : detail::_ITRF_t,
		                HorizontalDatum<ITRF2008,
		                                ITRF92,
		                                0.01280_m,                    // Tx = 0.01280 m
		                                0.00460_m,                    // Ty = 0.00460 m
		                                -0.04120_m,                   // Tz = -0.04120 m
		                                2.21000_ppb,                  // S  = 2.21000 ppb
		                                0.0_mas,                      // Rx = 0.00000 mas
		                                0.0_mas,                      // Ry = 0.00000 mas
		                                0.06000_mas,                  // Rz = 0.06000 mas
		                                2000_yr,                      // Epoch = 2000
		                                meters_per_year{0.00010},     // Dtx = 0.00010 m/yr
		                                meters_per_year{-0.00050},    // Dty = -0.00050 m/yr
		                                meters_per_year{-0.00320},    // Dtz = -0.00320 m/yr
		                                ppb_per_year{0.09000},        // Ds  = 0.09000 ppb/yr
		                                mas_per_year{0.0},            // Drx = 0.00000 mas/yr
		                                mas_per_year{0.0},            // Dry = 0.00000 mas/yr
		                                mas_per_year{0.02000}         // Drz = 0.02000 mas/yr
		                                >
		{
		};

		/**
		 * @brief		ITRF 91 Realization
		 * @details		(see ITRF2008 block for full background)
		 */
		struct ITRF91 : detail::_ITRF_t,
		                HorizontalDatum<ITRF2008,
		                                ITRF91,
		                                0.02480_m,                    // Tx = 0.02480 m
		                                0.01860_m,                    // Ty = 0.01860 m
		                                -0.04720_m,                   // Tz = -0.04720 m
		                                3.61000_ppb,                  // S  = 3.61000 ppb
		                                0.0_mas,                      // Rx = 0.00000 mas
		                                0.0_mas,                      // Ry = 0.00000 mas
		                                0.06000_mas,                  // Rz = 0.06000 mas
		                                2000_yr,                      // Epoch = 2000
		                                meters_per_year{0.00010},     // Dtx = 0.00010 m/yr
		                                meters_per_year{-0.00050},    // Dty = -0.00050 m/yr
		                                meters_per_year{-0.00320},    // Dtz = -0.00320 m/yr
		                                ppb_per_year{0.09000},        // Ds  = 0.09000 ppb/yr
		                                mas_per_year{0.0},            // Drx = 0.00000 mas/yr
		                                mas_per_year{0.0},            // Dry = 0.00000 mas/yr
		                                mas_per_year{0.02000}         // Drz = 0.02000 mas/yr
		                                >
		{
		};

		/**
		 * @brief		ITRF 90 Realization
		 * @details		(see ITRF2008 block for full background)
		 */
		struct ITRF90 : detail::_ITRF_t,
		                HorizontalDatum<ITRF2008,
		                                ITRF90,
		                                0.02280_m,                    // Tx = 0.02280 m
		                                0.01460_m,                    // Ty = 0.01460 m
		                                -0.06320_m,                   // Tz = -0.06320 m
		                                3.91000_ppb,                  // S  = 3.91000 ppb
		                                0.0_mas,                      // Rx = 0.00000 mas
		                                0.0_mas,                      // Ry = 0.00000 mas
		                                0.06000_mas,                  // Rz = 0.06000 mas
		                                2000_yr,                      // Epoch = 2000
		                                meters_per_year{0.00010},     // Dtx = 0.00010 m/yr
		                                meters_per_year{-0.00050},    // Dty = -0.00050 m/yr
		                                meters_per_year{-0.00320},    // Dtz = -0.00320 m/yr
		                                ppb_per_year{0.09000},        // Ds  = 0.09000 ppb/yr
		                                mas_per_year{0.0},            // Drx = 0.00000 mas/yr
		                                mas_per_year{0.0},            // Dry = 0.00000 mas/yr
		                                mas_per_year{0.02000}         // Drz = 0.02000 mas/yr
		                                >
		{
		};

		/**
		 * @brief		ITRF 89 Realization
		 * @details		(see ITRF2008 block for full background)
		 */
		struct ITRF89 : detail::_ITRF_t,
		                HorizontalDatum<ITRF2008,
		                                ITRF89,
		                                0.02780_m,                    // Tx = 0.02780 m
		                                0.03860_m,                    // Ty = 0.03860 m
		                                -0.10120_m,                   // Tz = -0.10120 m
		                                7.31000_ppb,                  // S  = 7.31000 ppb
		                                0.0_mas,                      // Rx = 0.00000 mas
		                                0.0_mas,                      // Ry = 0.00000 mas
		                                0.06000_mas,                  // Rz = 0.06000 mas
		                                2000_yr,                      // Epoch = 2000
		                                meters_per_year{0.00010},     // Dtx = 0.00010 m/yr
		                                meters_per_year{-0.00050},    // Dty = -0.00050 m/yr
		                                meters_per_year{-0.00320},    // Dtz = -0.00320 m/yr
		                                ppb_per_year{0.09000},        // Ds  = 0.09000 ppb/yr
		                                mas_per_year{0.0},            // Drx = 0.00000 mas/yr
		                                mas_per_year{0.0},            // Dry = 0.00000 mas/yr
		                                mas_per_year{0.02000}         // Drz = 0.02000 mas/yr
		                                >
		{
		};

		/**
		 * @brief		ITRF 88 Realization
		 * @details		(see ITRF2008 block for full background)
		 */
		struct ITRF88 : detail::_ITRF_t,
		                HorizontalDatum<ITRF2008,
		                                ITRF88,
		                                0.02280_m,                    // Tx = 0.02280 m
		                                0.00260_m,                    // Ty = 0.00260 m
		                                -0.12520_m,                   // Tz = -0.12520 m
		                                10.41000_ppb,                 // S  = 10.41000 ppb
		                                0.10000_mas,                  // Rx = 0.10000 mas
		                                0.0_mas,                      // Ry = 0.00000 mas
		                                0.06000_mas,                  // Rz = 0.06000 mas
		                                2000_yr,                      // Epoch = 2000
		                                meters_per_year{0.00010},     // Dtx = 0.00010 m/yr
		                                meters_per_year{-0.00050},    // Dty = -0.00050 m/yr
		                                meters_per_year{-0.00320},    // Dtz = -0.00320 m/yr
		                                ppb_per_year{0.09000},        // Ds  = 0.09000 ppb/yr
		                                mas_per_year{0.0},            // Drx = 0.00000 mas/yr
		                                mas_per_year{0.0},            // Dry = 0.00000 mas/yr
		                                mas_per_year{0.02000}         // Drz = 0.02000 mas/yr
		                                >
		{
		};

		//----------------------------------
		//	COMMON HORIZONTAL DATUMS
		//----------------------------------

		/**
		 * @brief		WGS84 Horizontal Datum for realization G2296
		 * @details		WGS 84 (G2296) is aligned to ITRF2020 at epoch 2024.0 (treated as noop here).
		 */
		struct WGS84_G2296 : HorizontalDatum<ITRF2020,
		                                     WGS84,
		                                     0.0_m,
		                                     0.0_m,
		                                     0.0_m,
		                                     0.0_ppb,
		                                     0.0_mas,
		                                     0.0_mas,
		                                     0.0_mas,
		                                     2024_yr,
		                                     meters_per_year{0.0},
		                                     meters_per_year{0.0},
		                                     meters_per_year{0.0},
		                                     ppb_per_year{0.0},
		                                     mas_per_year{0.0},
		                                     mas_per_year{0.0},
		                                     mas_per_year{0.0}>
		{
		};

		/**
		 * @brief		WGS84 Horizontal Datum for realization G1674 (ITR2008)
		 * @details		Source: https://confluence.qps.nl/pages/viewpage.action?pageId=29855173
		 *				Section: Transformation parameters.
		 */
		struct WGS84_G1674 : HorizontalDatum<ITRF2008,
		                                     WGS84,
		                                     0.0_m,                   // Tx = 0 m
		                                     0.0_m,                   // Ty = 0 m
		                                     0.0_m,                   // Tz = 0 m
		                                     0.0_ppb,                 // S  = 0 ppb
		                                     0.0_mas,                 // Rx = 0 mas
		                                     0.0_mas,                 // Ry = 0 mas
		                                     0.0_mas,                 // Rz = 0 mas
		                                     2005_yr,                 // Epoch = 2005
		                                     meters_per_year{0.0},    // Dtx = 0 m/yr
		                                     meters_per_year{0.0},    // Dty = 0 m/yr
		                                     meters_per_year{0.0},    // Dtz = 0 m/yr
		                                     ppb_per_year{0.0},       // Ds  = 0 ppb/yr
		                                     mas_per_year{0.0},       // Drx = 0 mas/yr
		                                     mas_per_year{0.0},       // Dry = 0 mas/yr
		                                     mas_per_year{0.0}        // Drz = 0 mas/yr
		                                     >
		{
		};

		/**
		 * @brief		North American Datum 1983 (2011 realization)
		 * @details		The ITRF2008 -> NAD83(2011) tie (US NGS Soler/Snay, joint US + Canada).
		 *				Primary source: EPSG:7807 "ITRF2008 to NAD83(2011) (1)", https://epsg.io/7807
		 *				(Coordinate Frame rotation, reference epoch 1997.0).
		 * @note		EPSG:7807 publishes coordinate-frame parameters; this library uses the position-vector
		 *				convention, so the three static rotations AND their three rotation rates have their
		 *				signs reversed compared to EPSG (translations, scale, and their rates unchanged).
		 *				Verified to reproduce EPSG:7807 to 0 mm at epochs 1997.0/2010.0/2020.0.
		 */
		struct NAD83 : HorizontalDatum<ITRF2008,
		                               GRS80,
		                               0.99343_m,                    // Tx = 0.99343 m
		                               -1.90331_m,                   // Ty = -1.90331 m
		                               -0.52655_m,                   // Tz = -0.52655 m
		                               1.71504_ppb,                  // S  = 1.71504 ppb
		                               -25.91467_mas,                // Rx = -25.91467 mas
		                               -9.42645_mas,                 // Ry = -9.42645 mas
		                               -11.59935_mas,                // Rz = -11.59935 mas
		                               1997_yr,                      // Epoch = 1997
		                               meters_per_year{0.00079},     // Dtx = 0.00079 m/yr
		                               meters_per_year{-0.00060},    // Dty = -0.00060 m/yr
		                               meters_per_year{-0.00134},    // Dtz = -0.00134 m/yr
		                               ppb_per_year{-0.10201},       // Ds  = -0.10201 ppb/yr
		                               mas_per_year{-0.06667},       // Drx = -0.06667 mas/yr
		                               mas_per_year{0.75744},        // Dry = 0.75744 mas/yr
		                               mas_per_year{0.05133}         // Drz = 0.05133 mas/yr
		                               >
		{
		};

		/**
		 * @brief		IGS20 Realization
		 * @details		IGS20 is the IGS frame adopted for repro3; treated as aligned with ITRF2020.
		 *				(Zero 7-parameter transform here.)
		 */
		struct IGS20 : detail::_ITRF_t, HorizontalDatum<ITRF2020, IGS20, 0.0_m, 0.0_m, 0.0_m, 0.0_ppb, 0.0_mas, 0.0_mas, 0.0_mas, 2015.0_yr>
		{
		};

		/**
		 * @brief		IGS 2008 Realization
		 * @details		GPS satellite ephemerides produced by US NGS and other International GNSS
		 *				Service (IGS) analysis centers are expressed in this CRS.
		 */
		struct IGS08 : HorizontalDatum<ITRF2008,
		                               GRS80,
		                               0.0_m,                   // Tx = 0.0 m
		                               0.0_m,                   // Ty = 0.0 m
		                               0.0_m,                   // Tz = 0.0 m
		                               0.0_ppb,                 // S  = 0.0 ppb
		                               0.0_mas,                 // Rx = 0.0 mas
		                               0.0_mas,                 // Ry = 0.0 mas
		                               0.0_mas,                 // Rz = 0.0 mas
		                               2005_yr,                 // Epoch = 2005
		                               meters_per_year{0.0},    // Dtx = 0.0 m/yr
		                               meters_per_year{0.0},    // Dty = 0.0 m/yr
		                               meters_per_year{0.0},    // Dtz = 0.0 m/yr
		                               ppb_per_year{0.0},       // Ds  = 0.0 ppb/yr
		                               mas_per_year{0.0},       // Drx = 0.0 mas/yr
		                               mas_per_year{0.0},       // Dry = 0.0 mas/yr
		                               mas_per_year{0.0}        // Drz = 0.0 mas/yr
		                               >
		{
		};

		/**
		 * @brief		Geodetic Datum of Australia (1994)
		 * @details		Source: http://www.epsg-registry.org/ (Code:6276)
		 * @note			All EPSG helmert parameters are given in the coordinate-frame rotation notation, while
		 *				this library uses the position-vector frame notation. Thus, all of the rotation
		 *				and delta-rotation parameters have had their signs reversed compared to the database
		 *				parameters.
		 */
		struct GDA94 : HorizontalDatum<ITRF2008,
		                               GRS80,
		                               -0.08468_m,                  // Tx = -84.68 mm
		                               -0.01942_m,                  // Ty = -19.42 mm
		                               0.03201_m,                   // Tz = 32.01 mm
		                               9.71000_ppb,                 // S  = 0.00971 ppm = 9.71 ppb
		                               0.42540_mas,                 // Rx = 0.4254 mas
		                               -2.25780_mas,                // Ry = -2.2578 mas
		                               -2.40150_mas,                // Rz = -2.4015 mas
		                               1994_yr,                     // Epoch = 1994
		                               meters_per_year{0.00142},    // Dtx = 1.42 mm/yr
		                               meters_per_year{0.00134},    // Dty = 1.34 mm/yr
		                               meters_per_year{0.00090},    // Dtz = 0.90 mm/yr
		                               ppb_per_year{0.10900},       // Ds  = 0.000109 ppm/yr = 0.109 ppb/yr
		                               mas_per_year{-1.54610},      // Drx = -1.5461 mas/yr
		                               mas_per_year{-1.18200},      // Dry = -1.1820 mas/yr
		                               mas_per_year{-1.15510}       // Drz = -1.1551 mas/yr
		                               >
		{
		};

		/**
		 * @brief		Geodetic Datum of Australia (2020)
		 * @details		GDA2020 is a plate-fixed datum, coincident with ITRF2014 at the 2020.0 reference
		 *				epoch and held fixed on the Australian plate; the plate rotation separates them at
		 *				other epochs. This is the time-dependent (plate-motion) tie: all static terms are
		 *				zero, only the rotation rates are non-zero.
		 *				Primary source: EPSG:8049 "ITRF2014 to GDA2020", https://epsg.io/8049
		 *				(time-dependent Coordinate Frame rotation, reference epoch 2020.0),
		 *				GDA2020 Technical Manual (ICSM), derived at 109 ARGN stations.
		 * @note			EPSG:8049 publishes coordinate-frame parameters; this library uses the position-vector
		 *				convention, so the three rotation RATES have their signs reversed compared to EPSG.
		 *				Validated: the position-vector form (negated rotation rates) reproduces the EPSG
		 *				coordinate-frame transform to 0 mm at epochs 2000.0/2020.0/2030.0.
		 * @note			Tied from ITRF2014 (which GDA2020 is defined against), not the ITRF2008 basis.
		 */
		struct GDA2020 : HorizontalDatum<ITRF2014,
		                                 GRS80,
		                                 0.0_m,                          // Tx = 0 m (coincident at epoch 2020.0)
		                                 0.0_m,                          // Ty = 0 m
		                                 0.0_m,                          // Tz = 0 m
		                                 0.0_ppb,                        // S  = 0 ppb
		                                 0.0_mas,                        // Rx = 0 mas
		                                 0.0_mas,                        // Ry = 0 mas
		                                 0.0_mas,                        // Rz = 0 mas
		                                 2020_yr,                        // Epoch = 2020.00
		                                 meters_per_year{0.0},           // Dtx = 0 m/yr
		                                 meters_per_year{0.0},           // Dty = 0 m/yr
		                                 meters_per_year{0.0},           // Dtz = 0 m/yr
		                                 ppb_per_year{0.0},              // Ds  = 0 ppb/yr
		                                 mas_per_year{-1.50379},         // Drx = -1.50379 mas/yr (position-vector; EPSG:8049 CF is +1.50379)
		                                 mas_per_year{-1.18346},         // Dry = -1.18346 mas/yr
		                                 mas_per_year{-1.20716}          // Drz = -1.20716 mas/yr
		                                 >
		{
		};

		/**
		 * @brief		European Terrestrial Reference System 1989 (legacy zero-transform tie)
		 * @details		Ties ETRS89 to ITRF89 with a zero transform. This is only correct at the 1989.0
		 *				reference epoch and does NOT model Eurasia plate motion, so it is off by decimetres at
		 *				modern epochs. Prefer `ETRF2000` or `ETRF2014`.
		 *				Definitional (ETRS89 == ITRF89 at 1989.0); no primary Helmert parameter set applies.
		 */
		struct ETRS89 : HorizontalDatum<ITRF89,
		                                GRS80,
		                                0.0_m,                   // Tx = 0 m
		                                0.0_m,                   // Ty = 0 m
		                                0.0_m,                   // Tz = 0 m
		                                0.0_ppb,                 // S  = 0 ppb
		                                0.0_mas,                 // Rx = 0 mas
		                                0.0_mas,                 // Ry = 0 mas
		                                0.0_mas,                 // Rz = 0 mas
		                                1989_yr,                 // Epoch = 1989.00
		                                meters_per_year{0.0},    // Dtx = 0 m/yr
		                                meters_per_year{0.0},    // Dty = 0 m/yr
		                                meters_per_year{0.0},    // Dtz = 0 m/yr
		                                ppb_per_year{0.0},       // Ds  = 0 ppb/yr
		                                mas_per_year{0.0},       // Drx = 0 mas/yr
		                                mas_per_year{0.0},       // Dry = 0 mas/yr
		                                mas_per_year{0.0}        // Drz = 0 mas/yr
		                                >
		{
		};

		/**
		 * @brief		European Terrestrial Reference Frame 2000
		 * @details		The ITRF-to-ETRF2000 tie. Parameters are in the position-vector convention (matching
		 *				this library; no sign flip needed).
		 *				Primary source: EPSG:7941 "ITRF2000 to ETRF2000 (2)", https://epsg.io/7941
		 *				(Position Vector, reference epoch 2000.0), citing EUREF Technical Note 1
		 *				(Boucher & Altamimi). Cross-checked against the EUREF-hosted Altamimi 2016 tutorial.
		 * @note		Tied here from the ITRF2008 basis rather than ITRF2000; the rotations/rates are
		 *				identical (ITRF2000<->ITRF2008 has ~zero rotation) and only the translation absorbs
		 *				the ITRF2008->ITRF2000 offset. Confirmed to 0 mm by convention round-trip.
		 */
		struct ETRF2000 : HorizontalDatum<ITRF2008,
		                                  GRS80,
		                                  0.05210_m,                    // Tx = 0.05210 m
		                                  0.04930_m,                    // Ty = 0.04930 m
		                                  -0.05850_m,                   // Tz = -0.05850 m
		                                  1.34000_ppb,                  // S  = 1.34000 ppb
		                                  0.89100_mas,                  // Rx = 0.89100 mas
		                                  5.39000_mas,                  // Ry = 5.39000 mas
		                                  -8.71200_mas,                 // Rz = -8.71200 mas
		                                  2000_yr,                      // Epoch = 2000.00
		                                  meters_per_year{0.00010},     // Dtx = 0.00010 m/yr
		                                  meters_per_year{0.00010},     // Dty = 0.00010 m/yr
		                                  meters_per_year{-0.00180},    // Dtz = -0.00180 m/yr
		                                  ppb_per_year{0.08000},        // Ds  = 0.08000 ppb/yr
		                                  mas_per_year{0.08100},        // Drx = 0.08100 mas/yr
		                                  mas_per_year{0.49000},        // Dry = 0.49000 mas/yr
		                                  mas_per_year{-0.79200}        // Drz = -0.79200 mas/yr
		                                  >
		{
		};

		/**
		 * @brief		European Terrestrial Reference Frame 2014
		 * @details		Latest European realization, tied to ITRF2014. All static terms are zero (ETRF2014
		 *				is coincident with ITRF2014 at the 1989.0 reference epoch); only the Eurasia-plate
		 *				rotation rates are non-zero.
		 *				Primary source: EPSG:8366 "ITRF2014 to ETRF2014 (1)", https://epsg.io/8366
		 *				(Position Vector convention, reference epoch 1989.0), citing EUREF Technical Note 1.
		 * @note		The parameters are taken from the primary EPSG record and the convention matches the
		 *				library's position-vector convention (used as-is, no sign flip). Validated end-to-end
		 *				against PROJ 9.7.1 (cs2cs EPSG:7789->EPSG:8401, the EPSG:8366 tie): the library
		 *				reproduces the oracle to < 0.05 mm (see test/positionECEFTest.h).
		 * @note		Tied from ITRF2014, not the ITRF2008 basis, so it carries a single primary citation.
		 */
		struct ETRF2014 : HorizontalDatum<ITRF2014,
		                                  GRS80,
		                                  0.0_m,                        // Tx = 0 m
		                                  0.0_m,                        // Ty = 0 m
		                                  0.0_m,                        // Tz = 0 m
		                                  0.0_ppb,                      // S  = 0 ppb
		                                  0.0_mas,                      // Rx = 0 mas
		                                  0.0_mas,                      // Ry = 0 mas
		                                  0.0_mas,                      // Rz = 0 mas
		                                  1989_yr,                      // Epoch = 1989.00
		                                  meters_per_year{0.0},         // Dtx = 0 m/yr
		                                  meters_per_year{0.0},         // Dty = 0 m/yr
		                                  meters_per_year{0.0},         // Dtz = 0 m/yr
		                                  ppb_per_year{0.0},            // Ds  = 0 ppb/yr
		                                  mas_per_year{0.08500},        // Drx = +0.085 mas/yr (position-vector, EPSG:8366 native)
		                                  mas_per_year{0.53100},        // Dry = +0.531 mas/yr
		                                  mas_per_year{-0.77000}        // Drz = -0.770 mas/yr
		                                  >
		{
		};

		/**
		 * @brief		International Terrestrial Reference System 2008
		 * @details		Realization of the ITRF2008 Frame using the GRS80 ellipsoid.
		 */
		struct ITRS2008 : HorizontalDatum<ITRF2008,
		                                  GRS80,
		                                  0.0_m,
		                                  0.0_m,
		                                  0.0_m,
		                                  0.0_ppb,
		                                  0.0_mas,
		                                  0.0_mas,
		                                  0.0_mas,
		                                  2000_yr,
		                                  meters_per_year{0.0},
		                                  meters_per_year{0.0},
		                                  meters_per_year{0.0},
		                                  ppb_per_year{0.0},
		                                  mas_per_year{0.0},
		                                  mas_per_year{0.0},
		                                  mas_per_year{0.0}>
		{
		};

		/**
		 * @brief		International Terrestrial Reference System 2014
		 * @details		Realization of the ITRF2014 Frame using the GRS80 ellipsoid.
		 */
		struct ITRS2014 : HorizontalDatum<ITRF2014,
		                                  GRS80,
		                                  0.0_m,
		                                  0.0_m,
		                                  0.0_m,
		                                  0.0_ppb,
		                                  0.0_mas,
		                                  0.0_mas,
		                                  0.0_mas,
		                                  2010_yr,
		                                  meters_per_year{0.0},
		                                  meters_per_year{0.0},
		                                  meters_per_year{0.0},
		                                  ppb_per_year{0.0},
		                                  mas_per_year{0.0},
		                                  mas_per_year{0.0},
		                                  mas_per_year{0.0}>
		{
		};

		/**
		 * @brief		International Terrestrial Reference System 2000
		 * @details		Realization of the ITRF2000 Frame using the GRS80 ellipsoid.
		 */
		struct ITRS2000 : HorizontalDatum<ITRF2000,
		                                  GRS80,
		                                  0.0_m,
		                                  0.0_m,
		                                  0.0_m,
		                                  0.0_ppb,
		                                  0.0_mas,
		                                  0.0_mas,
		                                  0.0_mas,
		                                  2000_yr,
		                                  meters_per_year{0.0},
		                                  meters_per_year{0.0},
		                                  meters_per_year{0.0},
		                                  ppb_per_year{0.0},
		                                  mas_per_year{0.0},
		                                  mas_per_year{0.0},
		                                  mas_per_year{0.0}>
		{
		};

		/**
		 * @brief		International Terrestrial Reference System 1988
		 * @details		Realization of the ITRF88 Frame using the GRS80 ellipsoid.
		 */
		struct ITRS88 : HorizontalDatum<ITRF88,
		                                GRS80,
		                                0.0_m,
		                                0.0_m,
		                                0.0_m,
		                                0.0_ppb,
		                                0.0_mas,
		                                0.0_mas,
		                                0.0_mas,
		                                2000_yr,
		                                meters_per_year{0.0},
		                                meters_per_year{0.0},
		                                meters_per_year{0.0},
		                                ppb_per_year{0.0},
		                                mas_per_year{0.0},
		                                mas_per_year{0.0},
		                                mas_per_year{0.0}>
		{
		};
	}    // namespace horizontalDatums

	//----------------------------------
	//	CONCEPTS
	//----------------------------------

	inline namespace traits
	{
		//--------------------------------------------------------------------------
		//  horizontal_datum_traits
		//--------------------------------------------------------------------------
		//
		// Keep the same public name and behavior:
		// - primary template yields void/void
		// - specialization activates when T::reference_frame and T::reference_ellipsoid exist
		//

		template<class T, typename = void>
		struct horizontal_datum_traits
		{
			using reference_frame     = void;
			using reference_ellipsoid = void;
		};

		template<class T>
		    requires requires {
			    typename T::reference_frame;
			    typename T::reference_ellipsoid;
		    }
		struct horizontal_datum_traits<T, void>
		{
			using reference_frame     = T::reference_frame;
			using reference_ellipsoid = T::reference_ellipsoid;
		};

		//-------------------------------------------------------------------------
		//	CONCEPT		has_horizontal_datum_typedefs
		//-------------------------------------------------------------------------
		/// @brief		Tests if a type provides the horizontal datum typedefs.
		/// @details	Requires nested typedefs `reference_frame` and `reference_ellipsoid`.
		//-------------------------------------------------------------------------
		template<typename T>
		concept has_horizontal_datum_typedefs = requires {
			typename T::reference_frame;
			typename T::reference_ellipsoid;
		};

		//-------------------------------------------------------------------------
		//	CONCEPT		helmert_coefficients
		//-------------------------------------------------------------------------
		/// @brief		Tests if a type is a 7/14-parameter Helmert coefficients type.
		/// @details	Uses the existing trait in the codebase.
		//-------------------------------------------------------------------------
		template<typename T>
		concept helmert_coefficients = is_helmert_7param_coefficients<T>::value || is_helmert_14param_coefficients<T>::value;

		//-------------------------------------------------------------------------
		//	CONCEPT		itrf
		//-------------------------------------------------------------------------
		/// @brief		Tests if type `T` is an ITRF datum.
		/// @details	To be an ITRF, an object must:
		///				- be derived from `horizontalDatums::detail::_ITRF_t`
		///				- be a 7 (or 14) parameter helmert transform
		///				- have a reference_frame typedef
		///				- have a reference_ellipsoid typedef
		//-------------------------------------------------------------------------
		template<typename T>
		concept itrf = std::is_base_of_v<horizontalDatums::detail::_ITRF_t, T> && helmert_coefficients<T> && has_horizontal_datum_typedefs<T>;

		//-------------------------------------------------------------------------
		//	CONCEPT		horizontal_datum
		//-------------------------------------------------------------------------
		/// @brief		Tests if `T` is a horizontal datum.
		/// @details	To be a horizontal datum, an object must:
		///				- be a 7 (or 14) parameter helmert transform
		///				- have reference_frame and reference_ellipsoid typedefs
		///				- have reference_frame which is an ITRF
		///				- have reference_ellipsoid which is an ellipsoid or an ITRF
		//-------------------------------------------------------------------------
		template<typename T>
		concept HorizontalDatum = helmert_coefficients<T> && has_horizontal_datum_typedefs<T> && itrf<typename T::reference_frame> &&
		                           (is_ellipsoid<typename T::reference_ellipsoid> || itrf<typename T::reference_ellipsoid>);

		//-------------------------------------------------------------------------
		//	Optional: compatibility aliases to preserve old names
		//-------------------------------------------------------------------------
		template<typename T>
		inline constexpr bool is_itrf = itrf<T>;

		template<typename T>
		inline constexpr bool is_horizontal_datum = HorizontalDatum<T>;
	}    // namespace traits
}    // namespace coordinates

#endif    // horizontalDatum_h