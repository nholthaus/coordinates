#ifndef geoid_h__
#define geoid_h__

//------------------------
//	INCLUDES
//------------------------
#include <units.h>
#include <cmath>
#include <interpolate.h>
#include <functional>

#include "ellipsoid.h"
#include "EGM96LUT.h"
#include "GEOID12ALUT.h"
#include "USGG2012LUT.h"
#include "coordinate_traits.h"

namespace coord
{
	using namespace units::literals;

	//----------------------------------
	//	BASE GEOID
	//----------------------------------

	template<typename ReferenceEllipsoid>
	struct Geoid
	{
		typedef ReferenceEllipsoid reference_ellipsoid;
	};

	namespace geoids
	{
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
	
			using degree_t = units::angle::degree_t;
			using meter_t = units::length::meter_t;
	
			/**
			* @brief		always returns 0
			* @param[in]	latitude latitude to find the undulation at, +90N to -90S.
			* @param[in]	longitude longitude to find the undulation at, either in -180W to 180E, or 0E to 360E
			* @returns		0 meters for all lat/lon combinations.
			*/
			static meter_t undulation(const degree_t& latitude, const degree_t& longitude)
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
		class EGM96 : public Geoid<coord::ellipsoids::WGS84>
		{
		public:
	
			using degree_t = units::angle::degree_t;
			using meter_t = units::length::meter_t;
	
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
			static meter_t undulation(const degree_t& latitude, const degree_t& longitude)
			{
				// convert lat/long to degrees
				degree_t latitude_deg = latitude;
				degree_t longitude_deg = longitude;
	
				// convert from -180  180 to 0 360 if need be.
				longitude_deg = longitude_deg < 0.0_deg ? longitude_deg + 360.0_deg : longitude_deg;
	
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
	
				const double prec = coord::EGM96LUT_PRECISION_DEG;
	
				double lat = latitude_deg();
				// round to nearest 0.25 towards +90.0 (index 0)
				lat = ceil(lat * 1 / prec) / (1 / prec);
				int latIndex = (int)((lat - 90.0) / -prec);												// convert to 0 = North, -180 = South. gives values where 0 = North and 720 = South
	
				double lon = longitude_deg();
				// round to nearest 0.25 towards 0.0 (index 0)
				lon = floor(lon * 1 / prec) / (1 / prec);												// round to the nearest quarter. The LUT has increments of 15 arcsec = 0.25 degrees
				int lonIndex = (int)(lon / prec);														// gives values where 0 = East and 1440 = West
	
				double xVal = (lat - latitude_deg()) / prec;											// distance from nearest point in LUT to desired point, normalized on a scale of 0-1. latitude is always > lat.
				double yVal = (longitude_deg() - lon) / prec;											// distance from nearest point in LUT to desired point, normalized on a scale of 0-1. longitude is always > lon.
	
				// create surface of points to interpolate with
				interpolationMatrix<double> surface;
				const int interpolationDimension = (int)surface.size();
	
				for (int i = 0; i < interpolationDimension; ++i)
				{
					int row = latIndex + (i - 1);
					row = ((row < 0) ? abs(row) : row);														// if the row is less than 0, reflect it because latitude values are symmetrical, i.e. indices [-1] is the same value as index [1] since they both represent 89.75 degrees lat.
					row = ((row >= (int)coord::EGM96LUT.size()) ? row % (int)coord::EGM96LUT.size() : row);	// if the row is less than 0, reflect it because latitude values are symmetrical, i.e. indices [-1] is the same value as index [1] since they both represent 89.75 degrees lat.
	
					for (int j = 0; j < interpolationDimension; ++j)
					{
						int column = lonIndex + (j - 1);
						column = ((column < 0) ? (int)coord::EGM96LUT[0].size() - 2 : column);				// if the index is negative, use the right side values of the lookup, since 0E and 260E are actually adjacent.
						column = ((column == (int)coord::EGM96LUT[0].size()) ? 1 : column);					// symmetry again for edge case. Not 0, because 0 and 1440 represent the same point in space!
						column = ((column >	 (int)coord::EGM96LUT[0].size()) ? 2 : column);					// symmetry again for edge case. Not 1, because 0 and 1440 represent the same point in space!
	
						// query the LUT
						surface[i][j] = coord::EGM96LUT[row][column];
					}
				}
	
				return meter_t(bicubicInterpolate(surface, xVal, yVal));
			}
		};
	
		//	----------------------------------------------------------------------------
		//	CLASS		USGG2012
		//  ----------------------------------------------------------------------------
		///	@brief		US Gravimetric geoid model 2012 (NGS)
		///	@details	GEOID12A is the geoid reference used by the NAD83 system
		//  ----------------------------------------------------------------------------
		class USGG2012 : public Geoid<coord::ellipsoids::GRS80>
		{
		public:
	
			using degree_t = units::angle::degree_t;
			using meter_t = units::length::meter_t;
	
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
			static meter_t undulation(const degree_t& latitude, const degree_t& longitude)
			{
				// convert lat/long to degrees
				degree_t latitude_deg = latitude;
				degree_t longitude_deg = longitude;

				// convert from -180  180 to 0 360 if need be.
				longitude_deg = longitude_deg < 0.0_deg ? longitude_deg + 360.0_deg : longitude_deg;

				if (latitude_deg > 58_deg || latitude_deg < 24_deg)
					throw std::runtime_error("Latitude out of bounds while calculating GEOID12A undulation.");
				if (longitude_deg > 300_deg || longitude_deg < 230_deg)
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

				const degree_t resolution(coord::USGG2012LUT_PRECISION_DEG);

				// get latitude index.toDouble()s (with decimals)
				long double latX = ((latitude_deg - 24_deg) / resolution()).to<long double>();
				long double lonY = ((longitude_deg - 230_deg) / resolution()).to<long double>();

				// find the lat/lon index (integer)
				unsigned int latIndex = (unsigned int)floor(latX);
				unsigned int lonIndex = (unsigned int)floor(lonY);

				// interpolation x,y
				long double xVal = (latX - latIndex);
				long double yVal = (lonY - lonIndex);

				// create surface of points to interpolate with. Use nearest-neighbor extrapolation on the edges
				interpolationMatrix<long double> elevationSurface;
				const size_t interpolationDimension = elevationSurface.size();
				int numLatitudeLines = (int)USGG2012LUT.size();
				int numLongitudeLines = (int)USGG2012LUT[0].size();

				for (int i = 0; i < interpolationDimension; ++i)
				{
					int row = latIndex + (i - 1);
					row = ((row < 0) ? 0 : row);													// if the row is less than 0, copy the nearest neighbor (i.e. repeat 0)
					row = ((row >= numLatitudeLines) ? numLatitudeLines - 1 : row);					// if the row is on the right edge, copy the right pixel

					for (int j = 0; j < interpolationDimension; ++j)
					{
						int column = lonIndex + (j - 1);
						column = ((column < 0) ? 0 : column);										// if the index is negative, repeat the left-edge pixel
						column = ((column >= numLongitudeLines) ? numLongitudeLines - 1 : column);	// on the right edge, repeat the right-edge pixel

						// query the LUT
						elevationSurface[i][j] = USGG2012LUT[row][column];
					}
				}

				return meter_t(bicubicInterpolate(elevationSurface, xVal, yVal));
			}
		};
	
		//	----------------------------------------------------------------------------
		//	CLASS		GEOID12A
		//  ----------------------------------------------------------------------------
		///	@brief		Earth Gravitational Model Geoid 12A (NGS)
		///	@details	GEOID12A is the geoid reference used to convert between NAD83 and
		///				NAVD88.
		//  ----------------------------------------------------------------------------
		class GEOID12A : public Geoid<coord::ellipsoids::GRS80>
		{
		public:
	
			using degree_t = units::angle::degree_t;
			using meter_t = units::length::meter_t;
	
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
			static meter_t undulation(const degree_t& latitude, const degree_t& longitude)
			{
				// convert lat/long to degrees
				degree_t latitude_deg = latitude;
				degree_t longitude_deg = longitude;

				// convert from -180  180 to 0 360 if need be.
				longitude_deg = longitude_deg < 0.0_deg ? longitude_deg + 360.0_deg : longitude_deg;

				if (latitude_deg > 58_deg || latitude_deg < 24_deg)
					throw std::runtime_error("Latitude out of bounds while calculating GEOID12A undulation.");
				if (longitude_deg > 300_deg || longitude_deg < 230_deg)
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

				const degree_t resolution(coord::GEOID12ALUT_PRECISION_DEG);

				// get latitude index.toDouble()s (with decimals)
				long double latX = ((latitude_deg - 24_deg) / resolution()).to<long double>();
				long double lonY = ((longitude_deg - 230_deg) / resolution()).to<long double>();

				// find the lat/lon index (integer)
				unsigned int latIndex = (unsigned int)floor(latX);
				unsigned int lonIndex = (unsigned int)floor(lonY);

				// interpolation x,y
				long double xVal = (latX - latIndex);
				long double yVal = (lonY - lonIndex);

				// create surface of points to interpolate with. Use nearest-neighbor extrapolation on the edges
				interpolationMatrix<long double> elevationSurface;
				const size_t interpolationDimension = elevationSurface.size();
				int numLatitudeLines = (int)GEOID12ALUT.size();
				int numLongitudeLines = (int)GEOID12ALUT[0].size();

				for (int i = 0; i < interpolationDimension; ++i)
				{
					int row = latIndex + (i - 1);
					row = ((row < 0) ? 0 : row);													// if the row is less than 0, copy the nearest neighbor (i.e. repeat 0)
					row = ((row >= numLatitudeLines) ? numLatitudeLines - 1 : row);					// if the row is on the right edge, copy the right pixel

					for (int j = 0; j < interpolationDimension; ++j)
					{
						int column = lonIndex + (j - 1);
						column = ((column < 0) ? 0 : column);										// if the index is negative, repeat the left-edge pixel
						column = ((column >= numLongitudeLines) ? numLongitudeLines - 1 : column);	// on the right edge, repeat the right-edge pixel

						// query the LUT
						elevationSurface[i][j] = GEOID12ALUT[row][column];
					}
				}

				return meter_t(bicubicInterpolate(elevationSurface, xVal, yVal));
			}
		};
	}

	//----------------------------------
	//	GEOID TRAITS
	//----------------------------------

	namespace traits
	{
		/**
		 * @brief		geoid traits implementation for classes which are not geoids.
		 */
		template<class T, typename = void>
		struct geoid_traits
		{
			typedef void reference_ellipsoid;
		};
	
		/**
		 * @brief		Traits class defining the properties of a geoid.
		 */
		template<class T>
		struct geoid_traits<T, typename coord::traits::void_type<
				typename T::reference_ellipsoid >::type>
		{
			typedef typename T::reference_ellipsoid reference_ellipsoid;							///< Ellipsoid that the geoid model is referenced to.
		};
	}

	//----------------------------------
	//	GEOID CONCEPT
	//----------------------------------

	namespace traits
	{
		namespace detail
		{
			/// implementation of the has_undulation concept checker.
			template <typename T>
			struct has_undulation_impl
			{
				using d = units::angle::degree_t;
		
				template<typename U>
				static auto test(U* p) -> decltype(U::undulation(d(1.0), d(1.0)));
				template<typename U>
				static auto test(...)->std::false_type;
		
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
		struct has_undulation : detail::has_undulation_impl<T>::type {};
	
		/**
		 * @brief		describes the geoid concept
		 * @details		a class is a geoid if it:
		 *				- provides the appropriate static member `undulation`
		 *				- is default constructible
		 */
		template <typename T>
		struct is_geoid :
				std::integral_constant<bool,
				std::is_same<T, geoids::NULL_GEOID>::value ||
		(has_undulation<T>::value &&
		coord::traits::is_ellipsoid<typename geoid_traits<T>::reference_ellipsoid>::value &&
		std::is_default_constructible<T>::value)>
		{};
	}

}

#endif // geoid_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: