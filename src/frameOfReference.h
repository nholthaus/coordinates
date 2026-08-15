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

#include <type_traits>
#include <units.h>
#include <utility>

#include "coordinate_traits.h"
#include "datum.h"
#include "horizontalDatum.h"

inline namespace coordinates
{
	using namespace units;

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
	template<class Datum, class Base, class Tuple>
	struct frameOfReference
	{
		typedef Datum datum_type;
		typedef Base  base_frame_type;
		typedef Tuple tuple_type;    // tuple type that the frame can conver from
	};

	//----------------------------------
	//	TYPEDEFS
	//----------------------------------

	using CartesianTuple   = std::tuple<meters<>, meters<>, meters<>>;
	using SphericalTuple   = std::tuple<degrees<>, degrees<>, meters<>>;
	using OrientationTuple = std::tuple<degrees<>, degrees<>, degrees<>>;

	constexpr auto NULL_POINT       = CartesianTuple(0.0_m, 0.0_m, 0.0_m);
	constexpr auto NULL_ORIGIN      = SphericalTuple(0.0_deg, 0.0_deg, 0.0_m);
	constexpr auto NULL_ORIENTATION = OrientationTuple(0.0_deg, 0.0_deg, 0.0_deg);

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
		FrameData()
		    : origin(0.0_deg, 0.0_deg, 0.0_m)
		    , orientation(0.0_deg, 0.0_deg, 0.0_deg)
		    , date(0) {};

		FrameData(SphericalTuple origin)
		    : origin(std::move(origin))
		    , orientation(0.0_deg, 0.0_deg, 0.0_deg)
		    , date(0) {};

		FrameData(OrientationTuple orientation)
		    : origin(0.0_deg, 0.0_deg, 0.0_m)
		    , orientation(std::move(orientation))
		    , date(0) {};

		FrameData(years<> date)
		    : origin(0.0_deg, 0.0_deg, 0.0_m)
		    , orientation(0.0_deg, 0.0_deg, 0.0_deg)
		    , date(date) {};

		FrameData(SphericalTuple origin, OrientationTuple orientation)
		    : origin(std::move(origin))
		    , orientation(std::move(orientation))
		    , date(0) {};

		FrameData(SphericalTuple origin, years<> date)
		    : origin(std::move(origin))
		    , orientation(0.0_deg, 0.0_deg, 0.0_deg)
		    , date(date) {};

		FrameData(const SphericalTuple& origin, const OrientationTuple& orientation, years<> date)
		    : origin(origin)
		    , orientation(orientation)
		    , date(date) {};

		// Accept any (angle, angle, length) tuple origin and normalize to canonical units.
		template<typename Angle0, typename Angle1, typename Length>
		    requires(units::traits::is_angle_unit_v<std::remove_cvref_t<Angle0>> && units::traits::is_angle_unit_v<std::remove_cvref_t<Angle1>> &&
		             units::traits::is_length_unit_v<std::remove_cvref_t<Length>>)
		FrameData(const std::tuple<Angle0, Angle1, Length>& originIn)
		    : origin(std::get<0>(originIn), std::get<1>(originIn), std::get<2>(originIn))
		    , orientation(0.0_deg, 0.0_deg, 0.0_deg)
		    , date(0){};

		SphericalTuple   origin;         ///< Origin of the frame, with respect to the base frame.
		OrientationTuple orientation;    ///< Orientation of the frame, with respect to the base frame.
		years<>          date;           ///< Date of observation.
	};

	inline bool operator==(const FrameData& lhs, const FrameData& rhs)
	{ return (lhs.origin == rhs.origin && lhs.orientation == rhs.orientation && lhs.date == rhs.date); }

	inline bool operator!=(const FrameData& lhs, const FrameData& rhs)
	{ return !(lhs == rhs); }

	inline std::ostream& operator<<(std::ostream& os, const FrameData& f)
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
	//	ASSERT HELPER
	//----------------------------------

	inline void requireSameFrameData(const FrameData& lhs, const FrameData& rhs, const char* message)
	{
		if (!(lhs == rhs))
			throw std::logic_error(message);
	}

	//----------------------------------
	//	COORDINATE FRAMES OF REFERENCE
	//----------------------------------

	inline namespace coordinateFrames
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
		struct ECEFFrame : frameOfReference<HorizontalDatum, ECEFFrame<typename horizontal_datum_traits<HorizontalDatum>::reference_frame>, CartesianTuple>
		{
			using tuple_type      = frameOfReference<HorizontalDatum,
			                                         ECEFFrame<typename horizontal_datum_traits<HorizontalDatum>::reference_frame>,
			                                         CartesianTuple>::tuple_type;
			using base_tuple_type = frameOfReference<HorizontalDatum,
			                                         ECEFFrame<typename horizontal_datum_traits<HorizontalDatum>::reference_frame>,
			                                         CartesianTuple>::base_frame_type::tuple_type;

			/**
			 * @brief		convert ellipsoidal ecef frame to an ITRF frame.
			 * @details		Uses a time-dependent 14-parameter helmert transform for the conversion.
			 * @tparam		Args	arguments not used by this conversion, but which may be passed
			 *						down from the conversion dispatcher.
			 * @param[in]	p		position vector in ECEF coordinates
			 * @param[in]	f		additional metadata about the frame of reference for the conversion.
			 * @returns		position vector in ITRF coordinates
			 */
			template<class... Args>
			static base_tuple_type convertToBaseFrame(const tuple_type& p, const FrameData& f, Args...)
			{
				if (f.date != 0_yr)
				{
					return coordinates::inversePositionVectorTransform<HorizontalDatum>(p, f.date);
				}
				else
					return coordinates::inversePositionVectorTransform<HorizontalDatum>(p);
			}

			/**
			 * @brief		convert ITRF frame to an ellipsoidal ecef frame.
			 * @details		Uses a time-dependent 14-parameter helmert transform for the conversion.
			 * @tparam		Args	arguments not used by this conversion, but which may be passed
			 *						down from the conversion dispatcher.
			 * @param[in]	p		position vector in ECEF coordinates
			 * @param[in]	f		additional metadata about the frame of reference for the conversion.
			 * @returns		position vector in ECEF coordinates
			 */
			template<class... Args>
			static tuple_type convertFromBaseFrame(const base_tuple_type& p, const FrameData& f, Args...)
			{
				if (f.date != 0_yr)
				{
					return coordinates::positionVectorTransform<HorizontalDatum>(p, f.date);
				}

				return coordinates::positionVectorTransform<HorizontalDatum>(p);
			}
		};

		/**
		 * @brief		Geodetic coordinates, using the ellipsoid as the reference for height
		 * @details		https://en.wikipedia.org/wiki/Reference_ellipsoid#Coordinates
		 */
		template<class HorizontalDatum>
		struct Geodetic2DFrame : frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, SphericalTuple>
		{
			using tuple_type      = frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, SphericalTuple>::tuple_type;
			using base_tuple_type = frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, SphericalTuple>::base_frame_type::tuple_type;

			/**
			 * @brief		converts Geodetic coordinates to ECEF coordinates
			 * @details		Source: IOPG Geomatics Guidance Not 7.2 "Coordinate Conversions and Transformations Including Formulae" pg. 94
			 *				This is the authoritative source per: http://gis.stackexchange.com/questions/176089/exact-conversion-of-ecef-to-geodetic-coordinates
			 * @note		All altitudes are referenced to the ellipsoid.
			 * @param[in]	point tuple containing (in order):
			 *				latitude, -90 to 90 degrees. Equivalent angles of other units are also accepted.
			 *				longitude, -180 to 180. Equivalent angles of other units are also accepted.
			 *				height, in meters, referenced to the vertical datum of `Datum`.
			 * @returns		tuple of (x,y,z) ECEF values, in meters.
			 */
			template<class... Args>
			static base_tuple_type convertToBaseFrame(const tuple_type& point, Args...)
			{
				auto phi(std::get<0>(point));       // latitude
				auto lambda(std::get<1>(point));    // longitude
				auto h(std::get<2>(point));         // height

				auto a  = traits::horizontal_datum_traits<HorizontalDatum>::reference_ellipsoid::a();
				auto e2 = traits::horizontal_datum_traits<HorizontalDatum>::reference_ellipsoid::e2();

				auto N_phi = a / sqrt(1.0 - e2 * pow<2>(sin(phi)));

				auto x = (N_phi + h) * cos(phi) * cos(lambda);
				auto y = (N_phi + h) * cos(phi) * sin(lambda);
				auto z = (N_phi * (1.0 - e2) + h) * sin(phi);

				return base_tuple_type(x, y, z);
			}

			/**
			 * @brief		converts ECEF coordinates to geodetic coordinates
			 * @details		Source: IOPG Geomatics Guidance Not 7.2 "Coordinate Conversions and Transformations Including Formulae" pg. 94
			 *				This is the authoritative source per: http://gis.stackexchange.com/questions/176089/exact-conversion-of-ecef-to-geodetic-coordinates
			 * @note			All altitudes are referenced to the ellipsoid.
			 * @param[in]	point tuple containing (in order):
			 *				x, in meters
			 *				y, in meters
			 *				z, in meters
			 * @returns		tuple of (lat, long, alt) values, in degrees/meters relative to the ellipsoid.
			 */
			template<class... Args>
			static tuple_type convertFromBaseFrame(const base_tuple_type& point, Args...)
			{
				auto X(std::get<0>(point));
				auto Y(std::get<1>(point));
				auto Z(std::get<2>(point));

				auto a  = traits::horizontal_datum_traits<HorizontalDatum>::reference_ellipsoid::a();
				auto b  = traits::horizontal_datum_traits<HorizontalDatum>::reference_ellipsoid::b();
				auto e2 = traits::horizontal_datum_traits<HorizontalDatum>::reference_ellipsoid::e2();

				auto epsilon = e2 / (1.0 - e2);
				auto p       = sqrt(pow<2>(X) + pow<2>(Y));
				auto q       = atan2((Z * a), (p * b));

				auto phi    = atan2((Z + epsilon * b * pow<3>(sin(q))), (p - e2 * a * pow<3>(cos(q))));
				auto lambda = atan2(Y, X);
				auto v      = a / sqrt(1.0 - e2 * pow<2>(sin(phi)));
				auto h      = (p / (cos(phi))) - v;

				auto      lat_deg(phi);
				degrees<> lon_deg(lambda);

				lon_deg = ((lon_deg > 180.0_deg) ? 180.0_deg - lon_deg : lon_deg);

				return tuple_type(lat_deg, lon_deg, h);
			}
		};

		/**
		 * @brief		Geodetic coordinates, using the provided vertical datum as the reference for height
		 * @details		https://en.wikipedia.org/wiki/Reference_ellipsoid#Coordinates
		 */
		template<class Datum>
		struct Geodetic3DFrame : frameOfReference<Datum, Geodetic2DFrame<typename traits::datum_traits<Datum>::horizontal_datum>, SphericalTuple>
		{
			using tuple_type = frameOfReference<Datum, Geodetic2DFrame<typename traits::datum_traits<Datum>::horizontal_datum>, SphericalTuple>::tuple_type;
			using base_tuple_type = frameOfReference<Datum, Geodetic2DFrame<typename traits::datum_traits<Datum>::horizontal_datum>, SphericalTuple>::
			        base_frame_type::tuple_type;

			template<class... Args>
			static base_tuple_type convertToBaseFrame(const tuple_type& point, Args...)
			{
				auto phi(std::get<0>(point));       // latitude
				auto lambda(std::get<1>(point));    // longitude
				auto h(std::get<2>(point));         // height

				h = coordinates::convertToEllipsoidHeight<typename datum_traits<Datum>::vertical_datum>(phi, lambda, h);

				return base_tuple_type(phi, lambda, h);
			}

			template<class... Args>
			static tuple_type convertFromBaseFrame(const base_tuple_type& point, Args...)
			{
				auto phi(std::get<0>(point));       // latitude
				auto lambda(std::get<1>(point));    // longitude
				auto h(std::get<2>(point));         // height

				h = coordinates::convertFromEllipsoidHeight<typename datum_traits<Datum>::vertical_datum>(phi, lambda, h);

				return tuple_type(phi, lambda, h);
			}
		};

		/**
		 * @brief		East-North-Up frame of reference
		 * @details
		 */
		template<class HorizontalDatum>
		struct ENUFrame : frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, CartesianTuple>
		{
			using tuple_type      = frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, CartesianTuple>::tuple_type;
			using base_tuple_type = frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, CartesianTuple>::base_frame_type::tuple_type;

			template<class... Args>
			static base_tuple_type convertToBaseFrame(const tuple_type& point, const FrameData& f, Args... args)
			{
				// Source: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_ENU
				// NOTE: the origin is assumed to be in the same datum as the point.

				auto E = std::get<0>(point);
				auto N = std::get<1>(point);
				auto U = std::get<2>(point);

				auto phi    = std::get<0>(f.origin);
				auto lambda = std::get<1>(f.origin);

				const CartesianTuple originECEF = Geodetic2DFrame<HorizontalDatum>::convertToBaseFrame(f.origin);
				auto                 Xt         = std::get<0>(originECEF);
				auto                 Yt         = std::get<1>(originECEF);
				auto                 Zt         = std::get<2>(originECEF);

				auto x = Xt + -E * sin(lambda) - N * sin(phi) * cos(lambda) + U * cos(phi) * cos(lambda);
				auto y = Yt + E * cos(lambda) - N * sin(phi) * sin(lambda) + U * cos(phi) * sin(lambda);
				auto z = Zt + N * cos(phi) + U * sin(phi);

				return CartesianTuple(x, y, z);
			}

			template<class... Args>
			static tuple_type convertFromBaseFrame(const base_tuple_type& point, const FrameData& f, Args... args)
			{
				// Source: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_ENU
				// NOTE: the origin is assumed to be in the same datum as the point.

				auto X = std::get<0>(point);
				auto Y = std::get<1>(point);
				auto Z = std::get<2>(point);

				auto phi    = std::get<0>(f.origin);
				auto lambda = std::get<1>(f.origin);

				CartesianTuple originECEF = Geodetic2DFrame<HorizontalDatum>::convertToBaseFrame(f.origin);
				auto           Xt         = std::get<0>(originECEF);
				auto           Yt         = std::get<1>(originECEF);
				auto           Zt         = std::get<2>(originECEF);

				auto E = -(X - Xt) * sin(lambda) + (Y - Yt) * cos(lambda);
				auto N = -(X - Xt) * sin(phi) * cos(lambda) - (Y - Yt) * sin(phi) * sin(lambda) + (Z - Zt) * cos(phi);
				auto U = (X - Xt) * cos(phi) * cos(lambda) + (Y - Yt) * cos(phi) * sin(lambda) + (Z - Zt) * sin(phi);

				return CartesianTuple(E, N, U);
			}
		};

		/**
		 * @brief		North-East-down frame of reference
		 * @details
		 */
		template<class HorizontalDatum>
		struct NEDFrame : frameOfReference<HorizontalDatum, ENUFrame<HorizontalDatum>, CartesianTuple>
		{
			using tuple_type      = frameOfReference<HorizontalDatum, ENUFrame<HorizontalDatum>, CartesianTuple>::tuple_type;
			using base_tuple_type = frameOfReference<HorizontalDatum, ENUFrame<HorizontalDatum>, CartesianTuple>::base_frame_type::tuple_type;

			template<class... Args>
			static base_tuple_type convertToBaseFrame(const tuple_type& point, const FrameData& f, Args... args)
			{
				// Source: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_ENU
				// NOTE: the origin is assumed to be in the same datum as the point.

				auto N = std::get<0>(point);
				auto E = std::get<1>(point);
				auto D = std::get<2>(point);

				return CartesianTuple(E, N, -D);
			}

			template<class... Args>
			static tuple_type convertFromBaseFrame(const base_tuple_type& point, const FrameData& f, Args... args)
			{
				// Source: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_ENU
				// NOTE: the origin is assumed to be in the same datum as the point.

				auto E = std::get<0>(point);
				auto N = std::get<1>(point);
				auto U = std::get<2>(point);

				return CartesianTuple(N, E, -U);
			}
		};

		/**
		 * @brief		Azimuth, Elevation, and Range
		 * @details		Presents a location as an Azimuth angle (from North), an elevation angle (from horizontal),
		 *				and a slant range from an arbitrary observation point (geodetic), in the locally-level
		 *				North-East-Down frame of the observer.
		 */
		template<class HorizontalDatum>
		struct AERFrame : frameOfReference<HorizontalDatum, NEDFrame<HorizontalDatum>, SphericalTuple>
		{
			using tuple_type      = frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, SphericalTuple>::tuple_type;
			using base_tuple_type = frameOfReference<HorizontalDatum, ECEFFrame<HorizontalDatum>, SphericalTuple>::base_frame_type::tuple_type;

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

				auto azimuth   = std::get<0>(point);
				auto elevation = std::get<1>(point) + 90_deg;
				auto range     = std::get<2>(point);

				// find the local NED cartesian coordinates
				auto n = range * cos(azimuth) * sin(elevation);
				auto e = range * sin(azimuth) * sin(elevation);
				auto d = range * cos(elevation);

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

				meters<> x = std::get<0>(point);
				meters<> y = std::get<1>(point);
				meters<> z = std::get<2>(point);

				// calculate spherical coordinates
				meters<>  range = sqrt(pow<2>(x) + pow<2>(y) + pow<2>(z));
				degrees<> az    = atan2(y, x);
				degrees<> el    = (range != 0_m) ? degrees(acos(z / range) - 90_deg) : 0_deg;

				// positive az
				az = az < 0.0_deg ? az + 360.0_deg : az;

				return tuple_type(az, el, range);
			}
		};
	}    // namespace coordinateFrames

	//------------------------------------------
	//	FRAME OF REFERNCE TRAITS CLASS
	//------------------------------------------

	inline namespace traits
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
				using base_frame_type = void;
				using datum_type      = void;
				using tuple_type      = void;
			};

			template<class Frame>
			struct frame_traits<Frame, std::void_t<typename Frame::base_frame_type, typename Frame::datum_type, typename Frame::tuple_type>>
			{
				using base_frame_type = Frame::base_frame_type;
				using datum_type      = Frame::datum_type;
				using tuple_type      = Frame::tuple_type;
			};
		}    // namespace detail

		template<class Frame>
		struct frame_traits : detail::frame_traits<Frame>
		{
		};
	}    // namespace traits

	//------------------------------------------
	//	FRAME OF REFERENCE TYPE TRAITS
	//------------------------------------------

	inline namespace traits
	{
		/**
		 * @brief		Trait which tests that a class has an `base_frame_type` typedef which represents the base frame used in coordinate conversions.
		 * @details		Since `base_frame_type` needs to be a `frame_of_reference`, but is used in the definition, the best we can do is
		 *				check that it's not void, or we'd end up in a chicken/egg situation.
		 */
		template<typename T, template<class> class Traits = frame_traits>
		inline constexpr bool has_base_frame_type = !std::is_same_v<typename Traits<T>::base_frame_type, void>;

		/**
		 * @brief		Trait which tests that a class has a `datum_type` typedef which represents the datum used by the coordinate frame
		 * @details		`datum_type` may represent a horizontal datum, or a 3D datum, but `is_horizontal_datum` will return true for both,
		 *				so that's the test we'll use.
		 */
		template<typename T, template<class> class Traits = frame_traits>
		concept has_datum_type = is_horizontal_datum<typename Traits<T>::datum_type>;

		/**
		 * @brief		Trait which tests that a class has a `tuple_type` typedef which represents the type of data the frame converts from
		 */
		template<typename T, template<class> class Traits = frame_traits>
		concept has_tuple_type = is_specialization_of<std::tuple, typename Traits<T>::tuple_type>::value;

		/**
		 * @brief		Tests that a class has a `convertToBaseFrame` static function.
		 * @details		The requirements are:
		 * 				- takes in inputs of const std::tuple<T,T,T>&, where T is a unit_t type.
		 * 				- returns std::tuple<T,T,T>, where T is a unit_t type.
		 * 				- is static
		 */
		template<class Frame>
		concept has_convertToBaseFrame = requires(const typename frame_traits<Frame>::tuple_type& p, const FrameData& f) {
			{ Frame::convertToBaseFrame(p, f) } -> std::same_as<typename frame_traits<typename frame_traits<Frame>::base_frame_type>::tuple_type>;
		};

		/**
		 * @brief		Tests that a class has a `convertFromBaseFrame` static function.
		 * @details		The requirements are:
		 * 				- takes in inputs of const std::tuple<T,T,T>&, where T is a unit_t type.
		 * 				- returns std::tuple<T,T,T>, where T is a unit_t type.
		 * 				- is static
		 */
		template<class Frame>
		concept has_convertFromBaseFrame =
		        requires(const typename frame_traits<typename frame_traits<Frame>::base_frame_type>::tuple_type& p, const FrameData& f) {
			        { Frame::convertFromBaseFrame(p, f) } -> std::same_as<typename frame_traits<Frame>::tuple_type>;
		        };

		template<class Frame, class FD>
		concept has_convertToBaseFrame_with = requires(const typename frame_traits<Frame>::tuple_type& p, const FD& f) {
			{ Frame::convertToBaseFrame(p, f) } -> std::same_as<typename frame_traits<typename frame_traits<Frame>::base_frame_type>::tuple_type>;
		};

		template<class Frame, class FD>
		concept has_convertFromBaseFrame_with =
		        requires(const typename frame_traits<typename frame_traits<Frame>::base_frame_type>::tuple_type& p, const FD& f) {
			        { Frame::convertFromBaseFrame(p, f) } -> std::same_as<typename frame_traits<Frame>::tuple_type>;
		        };

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
		template<class T>
		concept is_frame_of_reference = std::is_default_constructible_v<T> && has_base_frame_type<T> && has_datum_type<T> && has_tuple_type<T> &&
		                                has_convertToBaseFrame<T> && has_convertFromBaseFrame<T>;

		/**
		 * @brief		Tests whether a frame of reference type is a base frame
		 * @details		a base frame is defined as any frame whose base_frame_type is itself, e.g. ECEFFrame.
		 */
		template<class T>
		concept is_base_frame = is_frame_of_reference<T> &&
		                        // has the same type as it's base unit, i.e. is the base unit
		                        std::is_same_v<std::decay_t<T>, std::decay_t<typename frame_traits<T>::base_frame_type>>;

		/**
		 * @brief		Returns the lowest base frame type of a frame of reference
		 * @details		Recursively searches down the base_frame_type tree until the fundamental base frame
		 *				is found.
		 */
		template<typename T>
		    requires(is_frame_of_reference<T>)
		struct lowest_base_frame
		{
			typedef lowest_base_frame<typename frame_traits<T>::base_frame_type>::type type;
		};

		template<typename T>
		    requires(is_frame_of_reference<T> && is_base_frame<T>)
		struct lowest_base_frame<T>
		{
			typedef T type;
		};

		/**
		 * @brief		Trait which tests if two frames of reference are convertible to each other.
		 * @details		To be convertible, both types must be frames of reference, and they must have the
		 *				same lowest base frame.
		 */
		template<typename Frame1, typename Frame2>
		concept is_convertible_frame = is_frame_of_reference<Frame1> && is_frame_of_reference<Frame2> &&
		                               std::is_same_v<std::decay_t<typename lowest_base_frame<typename frame_traits<Frame1>::base_frame_type>::type>,
		                                              std::decay_t<typename lowest_base_frame<typename frame_traits<Frame2>::base_frame_type>::type>>;

		/**
		 * @brief		Trait which tests whether two reference frames are identical
		 * @details
		 */
		template<typename Frame1, typename Frame2>
		concept is_same_frame = is_frame_of_reference<Frame1> && is_frame_of_reference<Frame2> && std::is_same_v<std::decay_t<Frame1>, std::decay_t<Frame2>>;

		/**
		 * @brief		Trait which tests whether a frame of reference is Cartesian
		 * @details
		 */
		template<typename T>
		concept is_cartesian_frame = std::is_same_v<CartesianTuple, std::decay_t<typename frame_traits<T>::tuple_type>>;

		// Adapters for concept predicates used where a trait class template is required.
		template<class T>
		struct is_frame_of_reference_trait : std::bool_constant<is_frame_of_reference<T>>
		{
		};

		template<typename T1, typename T2>
		struct is_same_frame_trait : std::bool_constant<is_same_frame<T1, T2>>
		{
		};

		template<class T>
		struct is_cartesian_frame_trait : std::bool_constant<is_cartesian_frame<T>>
		{
		};

		namespace detail
		{
			template<typename U>
			struct depth
			{
				static_assert(is_frame_of_reference<U>, "traits::detail::depth<U>: U must satisfy is_frame_of_reference.");
			};

			// base case: base_frame_type == U
			template<typename U>
			    requires(is_frame_of_reference<U> && std::same_as<typename frame_traits<U>::base_frame_type, U>)
			struct depth<U>
			{
				static constexpr int value = 0;
			};

			// recursive case: climb base_frame_type
			template<typename U>
			    requires(is_frame_of_reference<U> && !std::same_as<typename frame_traits<U>::base_frame_type, U>)
			struct depth<U>
			{
				using base_t = frame_traits<U>::base_frame_type;
				static_assert(is_frame_of_reference<base_t>, "frame_traits<U>::base_frame_type must itself satisfy is_frame_of_reference.");
				static constexpr int value = depth<base_t>::value + 1;
			};

			/**
			 * @brief		Computes the least common ancestor of two frame types.
			 * @details		This is purely a type-level computation. The `Trait<T>::value` predicate
			 *				determines which nodes in the inheritance tree are considered "valid" ancestors.
			 */
			template<typename U, typename V, template<class> class Trait>
			struct least_common_ancestor
			{
			private:
				template<typename A, typename B>
				struct impl;    // primary

				//-------------------------------------------------------------------------
				// Case 1a: A == B and Trait<A>::value => A is the answer
				//-------------------------------------------------------------------------
				template<typename A>
				    requires Trait<A>::value
				struct impl<A, A>
				{
					using type = A;
				};

				//-------------------------------------------------------------------------
				// Case 1b: A == B and Trait<A>::value == false => climb
				//-------------------------------------------------------------------------
				template<typename A>
				    requires(!Trait<A>::value)
				struct impl<A, A>
				{
					using A_base = frame_traits<A>::base_frame_type;
					using type   = impl<A_base, A_base>::type;
				};

				//-------------------------------------------------------------------------
				// Case 2: depths differ, climb the deeper one
				//-------------------------------------------------------------------------
				template<typename A, typename B>
				    requires(depth<A>::value < depth<B>::value)
				struct impl<A, B>
				{
					using B_base = frame_traits<B>::base_frame_type;
					using type   = impl<A, B_base>::type;
				};

				template<typename A, typename B>
				    requires(depth<B>::value < depth<A>::value)
				struct impl<A, B>
				{
					using A_base = frame_traits<A>::base_frame_type;
					using type   = impl<A_base, B>::type;
				};

				//-------------------------------------------------------------------------
				// Case 3: same depth, not equal: climb both
				//-------------------------------------------------------------------------
				template<typename A, typename B>
				    requires(!std::is_same_v<A, B> && (depth<A>::value == depth<B>::value))
				struct impl<A, B>
				{
					using A_base = frame_traits<A>::base_frame_type;
					using B_base = frame_traits<B>::base_frame_type;
					using type   = impl<A_base, B_base>::type;
				};

			public:
				using type = impl<U, V>::type;
			};
		}    // namespace detail

		/**
		 * @brief		Trait which determines the least common ancestor of two frames of reference.
		 * @details		The ancestor is referenced by the `type` member typedef.
		 */
		template<typename T1, typename T2, template<class> class Trait = is_frame_of_reference_trait>
		struct least_common_ancestor : detail::least_common_ancestor<T1, T2, Trait>
		{
		};

		/**
		 * @brief		Trait which determines the least common ancestor of two frames of reference which itself is a Cartesian Frame.
		 * @details		The ancestor is referenced by the `type` member typedef.
		 */
		template<typename T1, typename T2, template<class> class Trait = is_cartesian_frame_trait>
		struct least_common_cartesian_ancestor : detail::least_common_ancestor<T1, T2, Trait>
		{
		};
	}    // namespace traits

	//----------------------------------
	//	DISPATCHERS - DON'T CALL THESE
	//----------------------------------

	namespace dispatchers
	{
		/**
		 * @brief		Conversion overload for when the two frames are the same.
		 * @details		Returns the original frame, performing unit conversions if necessary.
		 */
		template<class FrameFrom, class FrameTo, class FrameData>
		    requires is_same_frame<FrameFrom, FrameTo>
		frame_traits<FrameTo>::tuple_type convertToBase(const typename frame_traits<FrameFrom>::tuple_type& p, const FrameData& f)
		{
			return typename frame_traits<FrameTo>::tuple_type(std::get<0>(p), std::get<1>(p), std::get<2>(p));
			// this should handle unit conversions if necessary
		};

		/**
		 * @brief		Conversion overload for when the two frames are different.
		 * @details		Recursively converts `FrameFrom` towards `FrameTo`.
		 */
		template<class FrameFrom, class FrameTo, class FrameData>
		    requires(!is_same_frame<FrameFrom, FrameTo>) && has_convertToBaseFrame_with<FrameFrom, FrameData>
		frame_traits<FrameTo>::tuple_type convertToBase(const typename frame_traits<FrameFrom>::tuple_type& p, const FrameData& f)
		{
			typedef typename frame_traits<FrameFrom>::base_frame_type NextFrameFrom;
			return convertToBase<NextFrameFrom, FrameTo>(FrameFrom::convertToBaseFrame(p, f), f);
		};

		/**
		 * @brief		Conversion overload for when the two frames are the same.
		 * @details		Returns the original frame, performing unit conversions if necessary.
		 */
		template<class FrameFrom, class FrameTo, class FrameData>
		    requires is_same_frame<FrameFrom, FrameTo>
		frame_traits<FrameTo>::tuple_type convertFromBase(const typename frame_traits<FrameFrom>::tuple_type& p, const FrameData& f)
		{
			return typename frame_traits<FrameTo>::tuple_type(std::get<0>(p), std::get<1>(p), std::get<2>(p));
			// this should handle unit conversions if necessary
		};

		/**
		 * @brief		Conversion overload for when the two frames are different.
		 * @details		Recursively converts `FrameFrom` towards `FrameTo`.
		 */
		template<class FrameFrom, class FrameTo, class FrameData>
		    requires(!is_same_frame<FrameFrom, FrameTo>) && has_convertFromBaseFrame_with<FrameTo, FrameData>
		frame_traits<FrameTo>::tuple_type convertFromBase(const typename frame_traits<FrameFrom>::tuple_type& p, const FrameData& f)
		{
			typedef typename frame_traits<FrameTo>::base_frame_type NextFrameTo;
			return FrameTo::convertFromBaseFrame(convertFromBase<FrameFrom, NextFrameTo>(p, f), f);
		};
	}    // namespace dispatchers

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
	template<is_frame_of_reference FrameFrom, is_frame_of_reference FrameTo, class FrameDataFrom = FrameData, class FrameDataTo = FrameData>
	frame_traits<FrameTo>::tuple_type
	convert(const typename frame_traits<FrameFrom>::tuple_type& p, const FrameDataFrom& frameDataFrom, const FrameDataTo& frameDataTo)
	{
		using IntermediateFrame = least_common_ancestor<FrameFrom, FrameTo>::type;

		auto toIntermediate = dispatchers::convertToBase<FrameFrom, IntermediateFrame>(p, frameDataFrom);

		return dispatchers::convertFromBase<IntermediateFrame, FrameTo>(toIntermediate, frameDataTo);
	}

	template<is_frame_of_reference FrameFrom, is_frame_of_reference FrameTo, class FrameDataFrom = FrameData, class FrameDataTo = FrameData>
	frame_traits<FrameTo>::tuple_type convert(const typename frame_traits<FrameFrom>::tuple_type& p)
	{ return convert<FrameFrom, FrameTo, FrameDataFrom, FrameDataTo>(p, FrameDataFrom{}, FrameDataTo{}); }

	template<is_frame_of_reference FrameFrom, is_frame_of_reference FrameTo, class FrameDataFrom = FrameData, class FrameDataTo = FrameData>
	    requires(std::is_constructible_v<FrameDataTo, FrameDataFrom>)
	frame_traits<FrameTo>::tuple_type convert(const typename frame_traits<FrameFrom>::tuple_type& p, const FrameDataFrom& frameDataFrom)
	{ return convert<FrameFrom, FrameTo, FrameDataFrom, FrameDataTo>(p, frameDataFrom, FrameDataTo(frameDataFrom)); }

}    // namespace coordinates

#endif    // framesOfReference_h__