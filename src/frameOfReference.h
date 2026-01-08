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

#ifndef framesOfReference_h__
#define framesOfReference_h__

//------------------------
//	INCLUDES
//------------------------

#include <units.h>
#include <type_traits>
#include <iostream>

#include "geoid.h"
#include "datum.h"
#include "horizontalDatum.h"
#include "coordinate_traits.h"

namespace coord
{
	//----------------------------------
	//	BASE FRAME TYPE
	//----------------------------------

	/**
	 * @brief		base Frame class
	 * @details		Frame types may be derived from this class, but need not be. A frame class must
	 *				define the `frame_category` and `base_frame_type` typedefs.
	 * @note		inheritance from `frame` class is not sufficient to satisfy the frame concept, because
	 *				the static functions `convertToBaseFrame` and `convertFromBaseFrame` are also required,
	 *				and static functions cannot be virtual.
	 */
	template <class Datum, class Base, class Tuple>
	struct frameOfReference
	{
		typedef Datum										datum_type;
		typedef Base										base_frame_type;
		typedef Tuple										tuple_type;								// tuple type that the frame can conver from
	};

	//----------------------------------
	//	TYPEDEFS
	//----------------------------------

	using cartesianTuple = std::tuple<units::length::meter_t, units::length::meter_t, units::length::meter_t>;
	using sphericalTuple = std::tuple<units::angle::degree_t, units::angle::degree_t, units::length::meter_t>;
	using orientationTuple = std::tuple<units::angle::degree_t, units::angle::degree_t, units::angle::degree_t>;

	/**
	 * @brief		Frame metadata
	 * @details		Some frames of reference have time-varying definitions, such as
	 *				vehicle carried NED, or body-axis frames. `frameData` contains the
	 *				additional metadata required to perform such conversions.
	 * @note		when subclassing, it's advisable to create constructors for each valid permutation
	 *				of required data for a single conversion. This way, the point can call the convert
	 *				function with just the data required, 
	 */
	struct FrameData
	{
		using meter_t = units::length::meter_t;
		using degree_t = units::angle::degree_t;
		using year_t = units::time::year_t;

		FrameData() : origin(0.0_deg, 0.0_deg, 0.0_m), orientation(0.0_deg, 0.0_deg, 0.0_deg), date(0) {};
		FrameData(sphericalTuple origin) : origin(origin), orientation(0.0_deg, 0.0_deg, 0.0_deg), date(0) {};
		FrameData(orientationTuple orientation) : origin(0.0_deg, 0.0_deg, 0.0_m), orientation(orientation), date(0){};
		FrameData(year_t date) : origin(0.0_deg, 0.0_deg, 0.0_m), orientation(0.0_deg, 0.0_deg, 0.0_deg), date(date) {};
		FrameData(sphericalTuple origin, orientationTuple orientation) : origin(origin), orientation(orientation), date(0) {};
		FrameData(sphericalTuple origin, year_t date) : origin(origin), orientation(0.0_deg, 0.0_deg, 0.0_deg), date(date) {};
		FrameData(sphericalTuple origin, orientationTuple orientation, year_t date) : origin(origin), orientation(orientation), date(date) {};

		sphericalTuple		origin;			///< Origin of the frame, with respect to the base frame.
		orientationTuple	orientation;	///< Orientation of the frame, with respect to the base frame.
		year_t				date;			///< Date of observation.
	};

	bool operator==(const FrameData& lhs, const FrameData& rhs)
	{
		return (lhs.origin == rhs.origin && lhs.orientation == rhs.orientation && lhs.date == rhs.date);
	}

	bool operator!=(const FrameData& lhs, const FrameData& rhs)
	{
		return !(lhs == rhs);
	}

	std::ostream& operator<<(std::ostream& os, const FrameData& f)
	{
		os << "{ (";
		os << std::get<0>(f.origin) << ",";
		os << std::get<1>(f.origin) << ",";
		os << std::get<2>(f.origin);
		os << ") , (";
		os << std::get<0>(f.orientation) << ",";
		os << std::get<1>(f.orientation) << ",";
		os << std::get<2>(f.orientation);
		os << ") , ";
		os << f.date;
		os << " }";
		return os;
	}

	//----------------------------------
	//	COORDINATE FRAMES OF REFERENCE
	//----------------------------------

	namespace coordinateFrames
	{
		/**
		 * @brief		Earth-centered, Earth-fixed frame of reference
		 * @details		https://en.wikipedia.org/wiki/ECEF
		 *				ECEF ("Earth-Centered, Earth-Fixed"), also known as ECR ("Earth Centered Rotational"),
		 *				is a geographic coordinate system and Cartesian coordinate system, and is
		 *				sometimes known as a "conventional terrestrial" system. It represents positions
		 *				as an X, Y, and Z coordinate. The point (0,0,0) is defined as the center of
		 *				mass of the Earth, hence the name Earth-Centered. Its axes are aligned with
		 *				the International Reference Pole (IRP) and International Reference Meridian
		 *				(IRM) that are fixed with respect to the surface of the Earth, hence the name
		 *				Earth-Fixed. This term can cause confusion since the Earth does not rotate
		 *				about the z-axis (unlike an inertial system such as ECI), and is therefore
		 *				alternatively called ECR.\n\n
		 *				The z-axis is pointing towards the north but it does not coincide exactly with
		 *				the instantaneous Earth rotational axis. The slight "wobbling" of the rotational
		 *				axis is known as polar motion. The x-axis intersects the sphere of the Earth
		 *				at 0� latitude (Equator) and 0� longitude (Greenwich). This means that ECEF
		 *				rotates with the earth and therefore, coordinates of a point fixed on the
		 *				surface of the earth do not change. Conversion from a WGS84 Datum to ECEF
		 *				can be used as an intermediate step in converting velocities to the North
		 *				East Down coordinate system.
		 * @tparam		HorizontalDatum	2-dimensional horizontal datum the ECEF frame is based on
		 */
		template<class HorizontalDatum>
		struct ECEFFrame : frameOfReference<
			HorizontalDatum,
			ECEFFrame<typename coord::traits::horizontal_datum_traits<HorizontalDatum>::reference_frame>,
			cartesianTuple
		>
		{
			using tuple_type = typename frameOfReference<HorizontalDatum, ECEFFrame<typename coord::traits::horizontal_datum_traits<HorizontalDatum>::reference_frame>, cartesianTuple>::tuple_type;
			using base_tuple_type = typename frameOfReference<HorizontalDatum, ECEFFrame<typename coord::traits::horizontal_datum_traits<HorizontalDatum>::reference_frame>, cartesianTuple>::base_frame_type::tuple_type;
			using meter_t = units::length::meter_t;
			using year_t = units::time::year_t;

			/**
			 * @brief		convert ellipsoidal ecef frame to an ITRF frame.
			 * @details		Uses a time-dependent 14-parameter helmert transform for the conversion.
			 * @tparam		Args	arguments not used by this conversion, but which may be passed
			 *						down from the conversion dispatcher.
			 * @param[in]	p			position vector in ECEF coordinates
			 * @param[in]	frameData	additional metadata about the frame of reference for the conversion.
			 * @returns		position vector in ITRF coordinates
			 */
			template<class... Args>
			static base_tuple_type convertToBaseFrame(const tuple_type& p, const FrameData& f, Args...)
			{
				if (f.date != 0_yr)
				{
					return coord::inversePositionVectorTransform<HorizontalDatum>(p, f.date);
				}
				else
					return coord::inversePositionVectorTransform<HorizontalDatum>(p);
			}

			/**
			 * @brief		convert ITRF frame to an ellipsoidal ecef frame.
			 * @details		Uses a time-dependent 14-parameter helmert transform for the conversion.
			 * @tparam		Args	arguments not used by this conversion, but which may be passed
			 *						down from the conversion dispatcher.
			 * @param[in]	p			position vector in ECEF coordinates
			 * @param[in]	frameData	additional metadata about the frame of reference for the conversion.
			 * @returns		position vector in ECEF coordinates
			 */
			template<class... Args>
			static tuple_type convertFromBaseFrame(const base_tuple_type& p, const FrameData& f, Args...)
			{
				if (f.date != 0_yr)
				{
					return coord::positionVectorTransform<HorizontalDatum>(p, f.date);
				}
				else
					return coord::positionVectorTransform<HorizontalDatum>(p);
			}
		};

		/**
		 * @brief		Geodetic coordinates, using the ellipsoid as the reference for height
		 * @details		https://en.wikipedia.org/wiki/Reference_ellipsoid#Coordinates
		 */
		template<class HorizontalDatum>
		struct Geodetic2DFrame : frameOfReference<
			HorizontalDatum,
			ECEFFrame<HorizontalDatum>,
			sphericalTuple
		>
		{
			
			using tuple_type = typename frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, sphericalTuple>::tuple_type;
			using base_tuple_type = typename frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, sphericalTuple>::base_frame_type::tuple_type;

			using meter_t = units::length::meter_t;
			using scalar_t = units::dimensionless::scalar_t;
			using degree_t = units::angle::degree_t;
			using radian_t = units::angle::radian_t;
			
			/**
			 * @brief		converts Geodetic coordinates to ECEF coordinates
			 * @details		Source: IOPG Geomatics Guidance Not 7.2 "Coordinate Conversions and Transformations Including Formulae" pg. 94
			 *				This is the authoritative source per: http://gis.stackexchange.com/questions/176089/exact-conversion-of-ecef-to-geodetic-coordinates
			 * @note		All altitudes are referenced to the ellipsoid.
			 * @param[in]	p tuple containing (in order):
			 *				latitude, -90 to 90 degrees. Equivalent angles of other units are also accepted.
			 *				longitude, -180 to 180. Equivalent angles of other units are also accepted.
			 *				height, in meters, referenced to the vertical datum of `Datum`.
			 * @returns		tuple of (x,y,z) ECEF values, in meters.
			 */
			template<class... Args>
			static base_tuple_type convertToBaseFrame(const tuple_type& point, Args...)
			{			
				radian_t phi(std::get<0>(point));		// latitude
				radian_t lambda(std::get<1>(point));	// longitude
				meter_t h(std::get<2>(point));			// height

				meter_t a = coord::traits::horizontal_datum_traits<HorizontalDatum>::reference_ellipsoid::a();
				scalar_t e2 = coord::traits::horizontal_datum_traits<HorizontalDatum>::reference_ellipsoid::e2();

				meter_t x, y, z;

				auto N_phi = a / units::math::sqrt(1.0 - e2*units::math::pow<2>(units::math::sin(phi)));

				x = (N_phi + h)*units::math::cos(phi)*units::math::cos(lambda);
				y = (N_phi + h)*units::math::cos(phi)*units::math::sin(lambda);
				z = (N_phi*(1.0 - e2) + h)*units::math::sin(phi);

				return base_tuple_type(x, y, z);
			}

			/**
			* @brief		converts ECEF coordinates to geodetic coordinates
			* @details		Source: IOPG Geomatics Guidance Not 7.2 "Coordinate Conversions and Transformations Including Formulae" pg. 94
			*				This is the authoritative source per: http://gis.stackexchange.com/questions/176089/exact-conversion-of-ecef-to-geodetic-coordinates
			* @note			All altitudes are referenced to the ellipsoid.
			* @param[in]	p tuple containing (in order):
			*				x, in meters
			*				y, in meters
			*				z, in meters
			* @returns		tuple of (lat, long, alt) values, in degrees/meters relative to the ellipsoid.
			*/
			template<class... Args>
			static tuple_type convertFromBaseFrame(const base_tuple_type& point, Args...)
			{		
				meter_t X(std::get<0>(point));
				meter_t Y(std::get<1>(point));
				meter_t Z(std::get<2>(point));

				meter_t a = coord::traits::horizontal_datum_traits<HorizontalDatum>::reference_ellipsoid::a();
				meter_t b = coord::traits::horizontal_datum_traits<HorizontalDatum>::reference_ellipsoid::b();
				scalar_t e2 = coord::traits::horizontal_datum_traits<HorizontalDatum>::reference_ellipsoid::e2();

				scalar_t epsilon = e2 / (1.0 - e2);
				meter_t p = units::math::sqrt(units::math::cpow<2>(X) + units::math::cpow<2>(Y));
				radian_t q = units::math::atan2((Z * a) , (p * b));
			
				radian_t phi = units::math::atan2((Z + epsilon*b*units::math::pow<3>(units::math::sin(q))), (p - e2*a*units::math::pow<3>(units::math::cos(q))));
				radian_t lambda = units::math::atan2(Y, X);
				meter_t v = a / units::math::sqrt(1.0 - e2*units::math::pow<2>(units::math::sin(phi)));
				meter_t h = (p /(units::math::cos(phi))) - v;

				degree_t lat_deg(phi);
				degree_t lon_deg(lambda);

				lon_deg = ((lon_deg > 180_deg) ? 180_deg - lon_deg : lon_deg);

				return tuple_type(lat_deg, lon_deg, h);
			}
		};

		/**
		 * @brief		Geodetic coordinates, using the provided vertical datum as the reference for height
		 * @details		https://en.wikipedia.org/wiki/Reference_ellipsoid#Coordinates
		 */
		template<class Datum>
		struct Geodetic3DFrame : frameOfReference<
			Datum,
			Geodetic2DFrame<typename coord::traits::datum_traits<Datum>::horizontal_datum>,
			sphericalTuple
		>
		{
			using tuple_type = typename frameOfReference<Datum,Geodetic2DFrame<typename coord::traits::datum_traits<Datum>::horizontal_datum>,sphericalTuple>::tuple_type;
			using base_tuple_type = typename frameOfReference<Datum,Geodetic2DFrame<typename coord::traits::datum_traits<Datum>::horizontal_datum>,sphericalTuple>::base_frame_type::tuple_type;
			
			using meter_t = units::length::meter_t;
			using scalar_t = units::dimensionless::scalar_t;
			using degree_t = units::angle::degree_t;
			using radian_t = units::angle::radian_t;
			
			template<class... Args>
			static base_tuple_type convertToBaseFrame(const tuple_type& point, Args...)
			{
				radian_t phi(std::get<0>(point));		// latitude
				radian_t lambda(std::get<1>(point));	// longitude
				meter_t	h(std::get<2>(point));			// height

				h = coord::convertToEllipsoidHeight<typename coord::traits::datum_traits<Datum>::vertical_datum>(phi, lambda, h);

				return base_tuple_type(phi, lambda, h);
			}

			template<class... Args>
			static tuple_type convertFromBaseFrame(const base_tuple_type& point, Args...)
			{
				radian_t phi(std::get<0>(point));		// latitude
				radian_t lambda(std::get<1>(point));	// longitude
				meter_t	h(std::get<2>(point));			// height

				h = coord::convertFromEllipsoidHeight<typename coord::traits::datum_traits<Datum>::vertical_datum>(phi, lambda, h);

				return tuple_type(phi, lambda, h);
			}
		};

		/**
		 * @brief		East-North-Up frame of reference
		 * @details		
		 */
		template<class HorizontalDatum>
		struct ENUFrame : frameOfReference<
			HorizontalDatum,
			ECEFFrame<HorizontalDatum>,
			cartesianTuple
		>
		{			
			using tuple_type = typename frameOfReference<HorizontalDatum,ECEFFrame<HorizontalDatum>,cartesianTuple>::tuple_type;
			using base_tuple_type = typename frameOfReference<HorizontalDatum,ECEFFrame<HorizontalDatum>,cartesianTuple>::base_frame_type::tuple_type;
			
			using meter_t = units::length::meter_t;
			using scalar_t = units::dimensionless::scalar_t;
			using degree_t = units::angle::degree_t;
			using radian_t = units::angle::radian_t;
			
			template<class... Args>
			static base_tuple_type convertToBaseFrame(const tuple_type& point, const FrameData& f, Args... args)
			{
				// Source: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_ENU
				// NOTE: the origin is assumed to be in the same datum as the point.
				using namespace units::math;

				meter_t E = std::get<0>(point);
				meter_t N = std::get<1>(point);
				meter_t U = std::get<2>(point);

				radian_t phi = std::get<0>(f.origin);
				radian_t lambda = std::get<1>(f.origin);
				meter_t h = std::get<2>(f.origin);

				cartesianTuple originECEF = Geodetic2DFrame<HorizontalDatum>::convertToBaseFrame(f.origin);
				meter_t Xt = std::get<0>(originECEF);
				meter_t Yt = std::get<1>(originECEF);
				meter_t Zt = std::get<2>(originECEF);

				meter_t x = Xt + -E * sin(lambda) - N * sin(phi) * cos(lambda) + U * cos(phi) * cos(lambda);
				meter_t y = Yt + E * cos(lambda) - N * sin(phi) * sin(lambda) + U * cos(phi) * sin(lambda);
				meter_t z = Zt + N * cos(phi) + U * sin(phi);

				return cartesianTuple(x, y, z);
			}

			template<class... Args>
			static tuple_type convertFromBaseFrame(const base_tuple_type& point, const FrameData& f, Args... args)
			{
				// Source: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_ENU
				// NOTE: the origin is assumed to be in the same datum as the point.
				using namespace units::math;

				meter_t X = std::get<0>(point);
				meter_t Y = std::get<1>(point);
				meter_t Z = std::get<2>(point);

				radian_t phi = std::get<0>(f.origin);
				radian_t lambda = std::get<1>(f.origin);
				meter_t h = std::get<2>(f.origin);

				cartesianTuple originECEF = Geodetic2DFrame<HorizontalDatum>::convertToBaseFrame(f.origin);
				meter_t Xt = std::get<0>(originECEF);
				meter_t Yt = std::get<1>(originECEF);
				meter_t Zt = std::get<2>(originECEF);

				meter_t E = -(X - Xt)*sin(lambda) + (Y - Yt)*cos(lambda);
				meter_t N = -(X - Xt)*sin(phi)*cos(lambda) - (Y - Yt)*sin(phi)*sin(lambda) + (Z - Zt)*cos(phi);
				meter_t U = (X - Xt)*cos(phi)*cos(lambda) + (Y - Yt)*cos(phi)*sin(lambda) + (Z - Zt)*sin(phi);

				return cartesianTuple(E, N, U);
			}
		};

		/**
		* @brief		North-East-down frame of reference
		* @details
		*/
		template<class HorizontalDatum>
		struct NEDFrame : frameOfReference<
			HorizontalDatum,
			ENUFrame<HorizontalDatum>,
			cartesianTuple
		>
		{
			using tuple_type = typename frameOfReference<HorizontalDatum, ENUFrame<HorizontalDatum>, cartesianTuple>::tuple_type;
			using base_tuple_type = typename frameOfReference<HorizontalDatum, ENUFrame<HorizontalDatum>, cartesianTuple>::base_frame_type::tuple_type;

			using meter_t = units::length::meter_t;
			using scalar_t = units::dimensionless::scalar_t;
			using degree_t = units::angle::degree_t;
			using radian_t = units::angle::radian_t;

			template<class... Args>
			static base_tuple_type convertToBaseFrame(const tuple_type& point, const FrameData& f, Args... args)
			{
				// Source: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_ENU
				// NOTE: the origin is assumed to be in the same datum as the point.

				meter_t N = std::get<0>(point);
				meter_t E = std::get<1>(point);
				meter_t D = std::get<2>(point);

				return cartesianTuple(E, N, -D);
			}

			template<class... Args>
			static tuple_type convertFromBaseFrame(const base_tuple_type& point, const FrameData& f, Args... args)
			{
				// Source: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_ENU
				// NOTE: the origin is assumed to be in the same datum as the point.

				meter_t E = std::get<0>(point);
				meter_t N = std::get<1>(point);
				meter_t U = std::get<2>(point);

				return cartesianTuple(N, E, -U);
			}
		};

		/**
		 * @brief		Azimuth, Elevation, and Range
		 * @details		Presents a location as an Azimuth angle (from North), an elevation angle (from horizontal),
		 *				and a slant range from an arbitrary observation point (geodetic), in the locally-level
		 *				North-East-Down frame of the observer.
		 */
		template<class HorizontalDatum>
		struct AERFrame : frameOfReference<
			HorizontalDatum,
			NEDFrame<HorizontalDatum>,
			sphericalTuple
		>
		{
			using tuple_type = typename frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, sphericalTuple>::tuple_type;
			using base_tuple_type = typename frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, sphericalTuple>::base_frame_type::tuple_type;

			using meter_t = units::length::meter_t;
			using scalar_t = units::dimensionless::scalar_t;
			using degree_t = units::angle::degree_t;
			using radian_t = units::angle::radian_t;

			/**
			* @brief		converts Az/El/Range to ECEF
			* @details
			* @param[in]	point	Point to convert in Azimuth, Elevation, Range format from the local
			*						horizontal plane of the observer.
			* @param[in]	f		frame data, which must include the geodetic origin of the 'observer'
			* @returns		equivalent ECEF coordinate
			*/
			template<class... Args>
			static base_tuple_type convertToBaseFrame(const tuple_type& point, const FrameData& f, Args...)
			{
				// source: http://gis.stackexchange.com/questions/58923/calculate-view-angle/
				using namespace units::math;

				degree_t azimuth = std::get<0>(point);
				degree_t elevation = std::get<1>(point) + 90_deg;
				meter_t range = std::get<2>(point);

				// find the local NED cartesian coordinates
				meter_t n = range * cos(azimuth) * sin(elevation);
				meter_t e = range * sin(azimuth) * sin(elevation);
				meter_t d = range * cos(elevation);

				return base_tuple_type(n, e, d);
			}

			/**
			* @brief		converts ECEF coordinates (relative to an arbitrary observer) to azimuth, elevation,
			*				and range relative to the observers local NED frame.
			* @details
			* @param[in]	point	Point to convert as an ECEF tuple.
			* @param[in]	f		frame data, which must include the geodetic origin of the 'observer'
			* @returns		equivalent az/el/range from observer
			*/
			template<class... Args>
			static tuple_type convertFromBaseFrame(const base_tuple_type& point, const FrameData& f, Args...)
			{
				// source: http://mathworld.wolfram.com/SphericalCoordinates.html
				using namespace units::math;

				meter_t x = std::get<0>(point);
				meter_t y = std::get<1>(point);
				meter_t z = std::get<2>(point);

				// calculate spherical coordinates
				meter_t range = sqrt(cpow<2>(x) + cpow<2>(y) + cpow<2>(z));
				degree_t az = atan2(y, x);
				degree_t el = (range != 0_m) ? degree_t(acos(z / range) - 90_deg) : 0_deg;

				// positive az
				az = az < 0_deg ? az + 360_deg : az;

				return tuple_type(az, el, range);
			}
		};
	}

	//------------------------------------------
	//	FRAME OF REFERNCE TRAITS CLASS
	//------------------------------------------

	namespace traits
	{
		namespace detail
		{
			/**
			 * @brief		Traits class defining properties of Units.
			 * @details
			 */
			template<class Frame, typename = void>
			struct frame_traits
			{
				typedef void base_frame_type;
				typedef void datum_type;
				typedef void tuple_type;
			};
		
			template<class Frame>
			struct frame_traits<Frame, typename void_type<
				typename Frame::base_frame_type,
				typename Frame::datum_type,
				typename Frame::tuple_type>::type>
			{
				typedef typename Frame::base_frame_type			base_frame_type;
				typedef typename Frame::datum_type				datum_type;
				typedef typename Frame::tuple_type				tuple_type;
			};
		}

		template<class Frame>
		struct frame_traits : detail::frame_traits<Frame> {};
	}
	
	//------------------------------------------
	//	FRAME OF REFERENCE TYPE TRAITS
	//------------------------------------------
	
	namespace traits
	{
		/**
		 * @brief		Trait which tests that a class has an `base_frame_type` typedef which represents the base frame used in coordinate conversions.
		 * @details		Since `base_frame_type` needs to be a `frame_of_reference`, but is used in the definition, the best we can do is
		 *				check that it's not void, or we'd end up in a chicken/egg situation.
		 */
		template <typename T, template<class> class Traits = frame_traits>
		struct has_base_frame_type : std::integral_constant<bool, !std::is_same<typename Traits<T>::base_frame_type, void>::value>::type {};
		
		/**
		 * @brief		Trait which tests that a class has a `datum_type` typedef which represents the datum used by the coordinate frame
		 * @details		`datum_type` may represent a horizontal datum, or a 3D datum, but `is_horizontal_datum` will return true for both,
		 *				so that's the test we'll use.
		 */
		template <typename T, template<class> class Traits = frame_traits>
		struct has_datum_type : coord::traits::is_horizontal_datum<typename Traits<T>::datum_type>::type {};
	
		/**
		 * @brief		Trait which tests that a class has a `tuple_type` typedef which represents the type of data the frame converts from
		 */
		template <typename T, template<class> class Traits = frame_traits>
		struct has_tuple_type : is_specialization_of<std::tuple, typename Traits<T>::tuple_type>::type {};
	
		/// implementation of the has_convertToBaseFrame concept checker.
		template <class T>
		struct has_convertToBaseFrame_impl
		{
			template<typename U>
			static auto test(U* p) -> decltype(U::convertToBaseFrame(typename frame_traits<T>::tuple_type(), typename coord::FrameData()));
			template<typename U>
			static std::false_type test(...);
	
			using type = typename std::is_same<decltype(test<T>(0)), typename frame_traits<typename frame_traits<T>::base_frame_type>::tuple_type>::type;
		};
	
		/**
		 * @brief		Tests that a class has a `convertToBaseFrame` static function.
		 * @details		The requirements are:
		 * 				- takes in inputs of const std::tuple<T,T,T>&, where T is a unit_t type.
		 * 				- returns std::tuple<T,T,T>, where T is a unit_t type.
		 * 				- is static
		 */
		template <typename T>
		struct has_convertToBaseFrame : has_convertToBaseFrame_impl<T>::type{};
		
		/// implementation of the has_convertFromBaseFrame concept checker.
		template <typename T>
		struct has_convertFromBaseFrame_impl
		{
			template<typename U>
			static auto test(U* p) -> decltype(U::convertFromBaseFrame(typename frame_traits<typename frame_traits<T>::base_frame_type>::tuple_type(), typename coord::FrameData()));
			template<typename U>
			static std::false_type test(...);
	
			using type = typename std::is_same<decltype(test<T>(0)), typename frame_traits<T>::tuple_type>::type;
	
		};
	
		/**
		 * @brief		Tests that a class has a `convertFromBaseFrame` static function.
		 * @details		The requirements are:
		 * 				- takes in inputs of const std::tuple<T,T,T>&, where T is a unit_t type.
		 * 				- returns std::tuple<T,T,T>, where T is a unit_t type.
		 * 				- is static
		 */
		template <typename T>
		struct has_convertFromBaseFrame : has_convertFromBaseFrame_impl<T>::type {};
		
		/**
		 * @brief		Tests whether a type represents a frame of reference.
		 * @details		A frame of reference must:
		 * 				- be default constructible
		 * 				- have a `base_frame_type` typedef, representing the base coordinate frame used 
		 * 				  in conversion.
		 * 				- has a `datum_type` typedef, representing the datum of the frame of reference.
		 * 				- has a `tuple_type` typedef, representing the tuple of values the frame can
		 *				  convert from.
		 * 				- has a `convertToBaseFrame` or `convertToBase` static member. See 
		 *				  has_convertToBaseFrame for function requirements.
		 * 				- has a `convertFromBaseFrame` or `convertFromBase` static member. 
		 *				  See has_convertToBase for function requirements.
		 */
		template <typename T>
		struct is_frame_of_reference : std::integral_constant<bool,
			std::is_default_constructible<T>::value &&
			has_base_frame_type<T>::value &&	
			has_datum_type<T>::value &&
			has_tuple_type<T>::value &&
			has_convertToBaseFrame<T>::value && 
			has_convertFromBaseFrame<T>::value>::type {};
	
		/**
		 * @brief		Tests whether a frame of reference type is a base frame
		 * @details		a base frame is defined as any frame whose base_frame_type is itself, e.g. ECEFFrame.
		 */
		template <typename T>
		struct is_base_frame :
			std::integral_constant<bool,
				is_frame_of_reference<T>::value &&
				// has the same type as it's base unit, i.e. is the base unit
				std::is_same<typename std::decay<T>, typename std::decay<typename frame_traits<T>::base_frame_type>>::value>
		{};
	
		/**
		 * @brief		Returns the lowest base frame type of a frame of reference
		 * @details		Recursively searches down the base_frame_type tree until the fundamental base frame
		 *				is found.
		 */
		template<typename T, class U = typename std::conditional<is_frame_of_reference<T>::value, void, std::true_type>::type>
		struct lowest_base_frame
		{
			typedef typename lowest_base_frame<typename frame_traits<T>::base_frame_type, typename is_base_frame<T>::type>::type type;
		};
	
		template<typename T>
		struct lowest_base_frame<T, std::true_type>
		{
			typedef T type;
		};
	
		/**
		 * @brief		Trait which tests if two frames of reference are convertible to each other.
		 * @details		To be convertible, both types must be frames of reference, and they must have the
		 *				same lowest base frame.
		 */
		template<typename Frame1, typename Frame2>
		struct is_convertible_frame : std::integral_constant<bool,
			is_frame_of_reference<Frame1>::value &&
			is_frame_of_reference<Frame2>::value &&
			std::is_same<typename std::decay<typename lowest_base_frame<typename frame_traits<Frame1>::base_frame_type>::type>::type,
			typename std::decay<typename lowest_base_frame<typename frame_traits<Frame2>::base_frame_type>::type>::type>::value>
		{};
		
		/**
		 * @brief		Trait which tests whether two reference frames are identical
		 * @details		
		 */
		template<typename Frame1, typename Frame2>
		struct is_same_frame : std::integral_constant<bool,
			is_frame_of_reference<Frame1>::value &&
			is_frame_of_reference<Frame2>::value &&
			std::is_same<typename std::decay<Frame1>::type, typename std::decay<Frame2>::type>::value>
		{};

		/**
		 * @brief		Trait which tests whether a frame of reference is Cartesian
		 * @details
		 */
		template<typename T>
		struct is_cartesian_frame : std::is_same<coord::cartesianTuple, typename std::decay<typename coord::traits::frame_traits<T>::tuple_type>::type>::type {};

		namespace detail
		{
			template <typename U, typename = typename frame_traits<U>::base_frame_type>
			struct depth 
			{
				static const int value = depth<typename frame_traits<U>::base_frame_type>::value + 1;
			};
	
			template <typename U>
			struct depth<U, U> 
			{
				static const int value = 0;
			};
	
			template <typename U, typename V, template<class> class Trait, typename Enabler = void>
			struct least_common_ancestor;
	
			template <typename U, template<class> class Trait>
			struct least_common_ancestor<U, U, Trait, typename std::enable_if<Trait<U>::value>::type> 
			{
				using type = U;
			};
	
			template <typename U, template<class> class Trait>
			struct least_common_ancestor<U, U, Trait, typename std::enable_if<!Trait<U>::value>::type>
			{
				using type = typename least_common_ancestor<typename frame_traits<U>::base_frame_type, typename frame_traits<U>::base_frame_type, Trait>::type;
			};

			template <typename U, typename V, template<class> class Trait>
			struct least_common_ancestor < U, V, Trait,
				typename std::enable_if<(depth<U>::value < depth<V>::value)>::type> 
			{
				using type = typename least_common_ancestor<U, typename frame_traits<V>::base_frame_type, Trait>::type;
			};
	
			template <typename U, typename V, template<class> class Trait>
			struct least_common_ancestor < U, V, Trait,
				typename std::enable_if<(depth<V>::value < depth<U>::value)>::type> 
			{
				using type = typename least_common_ancestor<V, typename frame_traits<U>::base_frame_type, Trait>::type;
			};
	
			template <typename U, typename V, template<class> class Trait>
			struct least_common_ancestor<U, V, Trait,
				typename std::enable_if<!std::is_same<U, V>::value && (depth<V>::value == depth<U>::value)>::type> 
			{
				using type = typename least_common_ancestor<typename frame_traits<V>::base_frame_type, typename frame_traits<U>::base_frame_type, Trait>::type;
			};
		}

		/**
		 * @brief		Trait which determines the least common ancestor of two frames of reference.
		 * @details		The ancestor is referenced by the `type` member typedef.
		 */
		template<typename T1, typename T2, template<class> class Trait = is_frame_of_reference>
		struct least_common_ancestor : detail::least_common_ancestor<T1, T2, Trait> {};

		/**
		 * @brief		Trait which determines the least common ancestor of two frames of reference which itself is a Cartesian Frame.
		 * @details		The ancestor is referenced by the `type` member typedef.	
		 */
		template<typename T1, typename T2, template<class> class Trait = is_cartesian_frame>
		struct least_common_cartesian_ancestor : detail::least_common_ancestor<T1, T2, Trait> {};


	}

	//----------------------------------
	//	DISPATCHERS - DON'T CALL THESE
	//----------------------------------
	
	namespace detail
	{
		/**
		 * @brief		Conversion overload for when the two frames are the same.
		 * @details		Returns the original frame, performing unit conversions if necessary.
		 */
		template <class FrameFrom, class FrameTo, class FrameData,
			typename std::enable_if<traits::is_same_frame<FrameFrom, FrameTo>::value, int>::type = 0>
		typename traits::frame_traits<FrameTo>::tuple_type convertToBase(const typename traits::frame_traits<FrameFrom>::tuple_type& p, const FrameData& f)
		{
			return typename coord::traits::frame_traits<FrameTo>::tuple_type(std::get<0>(p), std::get<1>(p), std::get<2>(p));	// this should handle unit conversions if necessary
		};	

		/**
		 * @brief		Conversion overload for when the two frames are different.
		 * @details		Recursively converts `FrameFrom` towards `FrameTo`.
		 */
		template <class FrameFrom, class FrameTo, class FrameData,
			typename std::enable_if<!traits::is_same_frame<FrameFrom, FrameTo>::value, int>::type = 0>
		typename traits::frame_traits<FrameTo>::tuple_type convertToBase(const typename traits::frame_traits<FrameFrom>::tuple_type& p, const FrameData& f)
		{
			typedef typename coord::traits::frame_traits<FrameFrom>::base_frame_type NextFrameFrom;
			return convertToBase<NextFrameFrom, FrameTo>(FrameFrom::convertToBaseFrame(p, f), f);
		};

		/**
		* @brief		Conversion overload for when the two frames are the same.
		* @details		Returns the original frame, performing unit conversions if necessary.
		*/
		template <class FrameFrom, class FrameTo, class FrameData,
			typename std::enable_if<traits::is_same_frame<FrameFrom, FrameTo>::value, int>::type = 0>
		typename traits::frame_traits<FrameTo>::tuple_type convertFromBase(const typename traits::frame_traits<FrameFrom>::tuple_type& p, const FrameData& f)
		{
			return typename coord::traits::frame_traits<FrameTo>::tuple_type(std::get<0>(p), std::get<1>(p), std::get<2>(p));	// this should handle unit conversions if necessary
		};

		/**
		 * @brief		Conversion overload for when the two frames are different.
		 * @details		Recursively converts `FrameFrom` towards `FrameTo`.
		 */
		template <class FrameFrom, class FrameTo, class FrameData,
			typename std::enable_if<!traits::is_same_frame<FrameFrom, FrameTo>::value, int>::type = 0>
		typename traits::frame_traits<FrameTo>::tuple_type convertFromBase(const typename traits::frame_traits<FrameFrom>::tuple_type& p, const FrameData& f)
		{
			typedef typename coord::traits::frame_traits<FrameTo>::base_frame_type NextFrameTo;
			return FrameTo::convertFromBaseFrame(convertFromBase<FrameFrom, NextFrameTo>(p, f), f);
		};
	}
	
	//----------------------------------
	//	CONVERSION FUNCTIONS
	//----------------------------------
	
	/**
	 * @brief		
	 * @details		
	 * @tparam		FrameFrom
	 * @tparam		FrameTo
	 * @tparam		FrameDataFrom	Frame metadata, as required by `FrameFrom::convertToBaseFrame`. Can be any class derived from `frameData`.
	 * @tparam		FrameDataTo
	 */
	template <class FrameFrom, class FrameTo, class FrameDataFrom = FrameData, class FrameDataTo = FrameData, class = typename std::enable_if<
		traits::is_frame_of_reference<typename std::decay<FrameFrom>::type>::value && 
		traits::is_frame_of_reference<typename std::decay<FrameTo>::type>::value>::type>
	typename traits::frame_traits<FrameTo>::tuple_type convert(
	const typename traits::frame_traits<FrameFrom>::tuple_type& p, const FrameDataFrom& frameDataFrom = FrameDataFrom(), const FrameDataTo& frameDataTo = FrameDataTo())
	{		
		typedef typename coord::traits::least_common_ancestor<FrameFrom, FrameTo>::type IntermediateFrame;

		return detail::convertFromBase<IntermediateFrame, FrameTo>(detail::convertToBase<FrameFrom, IntermediateFrame>(p, frameDataFrom), frameDataTo);
	};
	
}

#endif // framesOfReference_h__