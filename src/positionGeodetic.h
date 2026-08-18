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

#ifndef positionGeodetic_h
#define positionGeodetic_h

//------------------------
//	INCLUDES
//------------------------

#include "algorithm.h"
#include "angles.h"
#include "frameOfReference.h"
#include "heights.h"
#include "point.h"
#include "ranges.h"
#include <units.h>

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		PositionGeodetic
	//  ----------------------------------------------------------------------------
	///	@brief		A 3D point in the Earth-centered, Earth-fixed frame of reference represented by
	///				a latitude, longitude, and height.
	///	@details
	/// @tparam		Datum		3-dimensional datum of the point. See coordinates::datums.
	/// @tparam		LatLonUnits	units of latitude/longitude. Should be a unit of angle, or a derivative
	///							of a unit of angle.
	/// @tparam		HeightUnits	units of altitude. Should be a unit of length, or a derivative of a unit
	///							of length.
	/// @tparam		T			underlying storage type for the point values. Should be an arithmetic type,
	///							such as double.
	//  ----------------------------------------------------------------------------
	template<class Datum, template<class> class LatLonUnits, template<class> class HeightUnits, typename T>
	class PositionGeodetic : public Point<Geodetic3DFrame<Datum>, SphericalTuple, FrameData>
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//		STATIC ERROR CHECKING
		//////////////////////////////////////////////////////////////////////////

		static_assert(units::traits::is_angle_unit_v<LatLonUnits<T>>, "Template parameter `LatLonUnits` template parameter must be a unit type.");
		static_assert(units::traits::is_length_unit_v<HeightUnits<T>>, "Template parameter `HeightUnits` template parameter must be a unit type.");
		static_assert(traits::is_datum<Datum>, "`Datum` template parameter does not satisfy the datum concept.");
		static_assert(std::is_arithmetic_v<T>, "`T` template parameter must be an arithmetic type.");

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using tuple_type       = Point<Geodetic3DFrame<Datum>, SphericalTuple, FrameData>::tuple_type;
		using frame_data_type  = Point<Geodetic3DFrame<Datum>, SphericalTuple, FrameData>::frame_data_type;
		using datum_type       = Datum;
		using angle_unit_type  = LatLonUnits<T>;
		using height_unit_type = HeightUnits<T>;
		using height_kind      = heights::kind_for<Datum>;    ///< the tagged height kind this datum measures (HAE vs MSL)

	public:
		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		default constructor
		 * @details		Creates a point object with the value (0,0,0).
		 */
		explicit PositionGeodetic()
		    : m_latitude(0)
		    , m_longitude(0)
		    , m_altitude(0)
		    , m_frameData(Datum::epoch())
		{
		}

		/**
		 * @brief		constructor
		 * @details		constructs a new point object, performing unit conversions if necessary.
		 * @param[in]	latitude	latitude of the point in the chosen <i>Datum</i>.
		 * @param[in]	longitude	longitude of the point in the chosen <i>Datum</i>.
		 * @param[in]	altitude	altitude of the point in the chosen <i>Datum</i>.
		 * @param[in]	dateOfObservation	date, in decimal years, when the measurement was taken. This
		 *					can be used to account for tectonic plate shift in the coordinate conversion
		 *					if necessary. It is safe to omit if no datum conversion will be performed,
		 *					or if the time-dependent correction is undesirable.
		 */
		explicit PositionGeodetic(angle_unit_type latitude, angle_unit_type longitude, height_unit_type altitude, years<> dateOfObservation = Datum::epoch())
		    : m_latitude(latitude)
		    , m_longitude(longitude)
		    , m_altitude(altitude)
		    , m_frameData(dateOfObservation)
		{
		}

		/**
		 * @brief		constructs a Point object from a tuple of values
		 * @details		constructs a new Point object, performing unit conversions if necessary.
		 * @param[in]	point	tuple containing the lat/lon/alt position of the point.
		 * @param[in]	dateOfObservation	date, in decimal years, when the measurement was taken. This
		 *					can be used to account for tectonic plate shift in the coordinate conversion
		 *					if necessary. It is safe to omit if no datum conversion will be performed,
		 *					or if the time-dependent correction is undesirable.
		 */
		explicit PositionGeodetic(tuple_type point, years<> dateOfObservation = Datum::epoch())
		    : m_latitude(std::get<0>(point))
		    , m_longitude(std::get<1>(point))
		    , m_altitude(std::get<2>(point))
		    , m_frameData(dateOfObservation)
		{
		}

		/**
		 * @brief		copy constructor
		 * @details		creates a copy of <i>other</i>, performing unit conversions if necessary.
		 * @param[in]	other PositionGeodetic to copy
		 * @returns		copy of <i>other</i>, with appropriate unit conversions performed.
		 */
		PositionGeodetic(const PositionGeodetic& other)
		    : m_latitude(other.m_latitude)
		    , m_longitude(other.m_longitude)
		    , m_altitude(other.m_altitude)
		    , m_frameData(other.m_frameData)
		{
		}

		/**
		 * @brief		implicit conversion constructor
		 * @details		constructs a PointECEF from another point, invoking all necessary unit conversions
		 * 				and coordinate frame conversions.
		 * @param[in]	point point to construct from
		 */
		template<class P>
		    requires(traits::is_point<P> && !std::same_as<std::remove_cvref_t<P>, PositionGeodetic>)
		PositionGeodetic(const P& point)
		    : m_frameData(point.frameData())
		{ coordinates::convert(point, *this); }

		/**
		 * @brief		assignment operator
		 * @details		performs unit conversions if necessary
		 * @param[in]	other Point value to assign to this point
		 */
		PositionGeodetic& operator=(const PositionGeodetic& other)
		{
			m_latitude  = other.m_latitude;
			m_longitude = other.m_longitude;
			m_altitude  = other.m_altitude;
			m_frameData = other.m_frameData;
			return *this;
		}

		/**
		 * @brief		implicit conversion assignment
		 * @details		assigns a PositionGeodetic from another point, invoking all necessary unit conversions
		 * 				and coordinate frame conversions.
		 * @param[in]	point point to construct from
		 * @returns		copy of *this
		 */
		template<class Point>
		    requires(traits::is_point<Point> && !std::is_convertible_v<Point, PositionGeodetic>)
		PositionGeodetic& operator=(const Point& point)
		{
			m_frameData = point.frameData();
			coordinates::convert(point, *this);
			return *this;
		}

		~PositionGeodetic() override = default;

		// friend other specializations of the template.
		template<class D, template<class> class LLATy, template<class> class HTy, class DU>
		friend class PositionGeodetic;

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC MEMBERS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		is point null
		 * @details		returns true if the point is (0,0,0).
		 * @returns		true if null, false otherwise.
		 */
		[[nodiscard]] bool isNull() const { return coordinates::isNull(*this); }

		/**
		 * @brief		compares two points
		 * @details		This function compares two points for equality, using the given tolerance.
		 * 				This is primarily intended for floating point types. For a tolerance of 0,
		 * 				operator== is more efficient. Appropriate unit conversions
		 *				will be performed, and two points representing the same point in space but
		 *				with different units *will* compare equal.
		 * @sa			operator==
		 * @param[in]	p			point to compare to. May have different units than this point
		 * @param[in]	tolerance	acceptable difference between the two points which will still
		 * 							compare equal, in units of <i>unit_type</i> (i.e. the same units as this
		 *							instance of the class).
		 * @returns		true if the values are equal within the tolerance, false otherwise.
		 */
		template<is_point Point>
		[[nodiscard]] bool isSame(const Point& p, height_unit_type tolerance = height_unit_type{0.0}) const
		{ return coordinates::isSame<height_unit_type>(*this, p, tolerance); }

		/**
		 * @brief		compares two points
		 * @details		This function compares two points for equality, using the tolerances in
		 *				<i>tolerance</i> for x, y, and z individually. Appropriate unit conversions
		 *				will be performed, and two points representing the same point in space but
		 *				with different units *will* compare equal.
		 * 				This is primarily intended for floating point types. For a tolerance of 0,
		 * 				operator== is more efficient.
		 * @sa			operator==
		 * @param[in]	p			point to compare to. May have different units than this point
		 * @param[in]	tolerance	acceptable difference between the two points which will still
		 * 							compare equal. The tolerance point must have the same type and
		 *							units as this instance of the class.
		 * @returns		true if the values are equal within the tolerance, false otherwise.
		 */
		template<is_point Point, is_point PointTol>
		[[nodiscard]] bool isSame(const Point& p, const PointTol& tolerance) const
		{ return coordinates::isSame(*this, p, tolerance); }

		/**
		 * @brief		calculates the distance between two points.
		 * @details		calculates the Euclidean distance between this point and point <i>p</i>
		 * @note			This is the straight-line distance, *not* the great-circle distance.
		 * @tparam		Point	Point type of the input.
		 * @param[in]	p	point to calculate distance to.
		 * @returns		distance between this Point and <i>p</i> in units of distance_units.
		 */
		template<is_point Point>
		[[nodiscard]] ranges::Euclidean distance(const Point& p) const
		{ return ranges::Euclidean(coordinates::distance(*this, p)); }

		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		set latitude value
		 * @details		overwrites the latitude-value with the value provided, performing a unit conversion
		 *				if necessary.
		 * @param[in]	latitude new value of <i>latitude</i>
		 */
		void setLatitude(angle_unit_type latitude) { m_latitude = latitude; }

		/**
		 * @brief		set longitude value
		 * @details		overwrites the longitude-value with the value provided, performing a unit conversion
		 *				if necessary.
		 * @param[in]	longitude new value of <i>longitude</i>
		 */
		void setLongitude(angle_unit_type longitude) { m_longitude = longitude; }

		/**
		 * @brief		set altitude value
		 * @details		overwrites the altitude-value with the value provided.
		 * @param[in]	altitude new value of <i>altitude</i>
		 */
		void setAltitude(height_unit_type altitude) { m_altitude = altitude; }

		/**
		 * @brief		latitude-value
		 * @details		returns the latitude-value of the point.
		 * @returns		latitude-value of the Point.
		 */
		[[nodiscard]] angles::Latitude latitude() const
		{
			// remember, convert automatically uses the most efficient algorithm, including to/fromBase
			// and directly returning values if they are the same.
			return angles::Latitude(m_latitude);
		}

		/**
		 * @brief		longitude-value
		 * @details		returns the longitude-value of the point.
		 * @returns		longitude-value of the Point.
		 */
		[[nodiscard]] angles::Longitude longitude() const { return angles::Longitude(m_longitude); }

		/**
		 * @brief		altitude-value
		 * @details		returns the altitude of the point, tagged with the height kind the datum measures: a datum
		 *				referenced to a bare ellipsoid yields an `heights::Ellipsoidal` (HAE), a datum referenced to
		 *				a geoid or topography yields an `heights::Orthometric` (MSL). The tag makes the reference
		 *				surface part of the type, so an HAE and an MSL height can never be silently interchanged.
		 * @returns		altitude of the Point, as `heights::kind_for<Datum>`.
		 */
		[[nodiscard]] height_kind altitude() const { return height_kind(m_altitude); }

		/**
		 * @brief		this point's height above the reference ellipsoid (HAE).
		 * @details		converts the stored altitude -- which is measured in the datum's own vertical reference --
		 *				to an ellipsoidal height, adding the geoid undulation at this point's latitude/longitude
		 *				when the datum is geoid-referenced, or returning it unchanged when the datum is already
		 *				ellipsoid-referenced. The result is tagged `heights::Ellipsoidal` so it can never be
		 *				mistaken for an orthometric height.
		 * @returns		height above the ellipsoid, as `heights::Ellipsoidal`.
		 */
		[[nodiscard]] heights::Ellipsoidal toEllipsoidHeight() const
		{
			return coordinates::convertToEllipsoidHeight<typename traits::datum_traits<Datum>::vertical_datum>(
			        m_latitude, m_longitude, m_altitude);
		}

		/**
		 * @brief		this point's height above the geoid (orthometric / MSL height).
		 * @details		converts the stored altitude to an orthometric height: it is first raised to an
		 *				ellipsoidal height (`toEllipsoidHeight`), then the datum's undulation is subtracted back
		 *				off, so a geoid-referenced datum round-trips to its stored value and an ellipsoid-referenced
		 *				datum yields the MSL height below its stored HAE. The result is tagged `heights::Orthometric`
		 *				so it can never be mistaken for an ellipsoidal height.
		 * @returns		height above the geoid, as `heights::Orthometric`.
		 */
		[[nodiscard]] heights::Orthometric toOrthometricHeight() const
		{
			return coordinates::convertFromEllipsoidHeight<typename traits::datum_traits<Datum>::vertical_datum>(
			        m_latitude, m_longitude, toEllipsoidHeight());
		}

		/**
		 * @brief		Date of observation.
		 * @details		This parameter is only relevant when converting between datums.
		 * @returns		Date, in decimal years, when the position was measured.
		 */
		[[nodiscard]] years<> date() const { return m_frameData.date; }

		/**
		 * @brief		print value
		 * @details		Prints the point value in the form "(lat, lon, alt)"
		 * @param[in]	os	output stream
		 * @param[in]	p	point to print
		 * @returns		reference to the output stream.
		 */
		friend std::ostream& operator<<(std::ostream& os, const PositionGeodetic& p)
		{ return os << "(" << p.m_latitude << ", " << p.m_longitude << ", " << p.m_altitude << ")"; }

		//////////////////////////////////////////////////////////////////////////
		//		POINT INTERFACE
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		returns value of the point as a tuple.
		 * @details		This is primarily intended for use by reference frame conversion methods.
		 * @returns		point as a tuple.
		 * @note		This function is intended for use by implicit conversion routines. Library
		 *				users should generally not need to call this function.
		 */
		[[nodiscard]] tuple_type point() const override { return tuple_type(m_latitude, m_longitude, m_altitude); }

		/**
		 * @brief		Returns the frame data.
		 * @details		Frame data is the ancillary data about the relationship between the frame of
		 *				reference and its base frame. This could include offsets in time or space or
		 *				axis rotation.
		 * @returns		ancillary frame data.
		 * @note		This function is intended for use by implicit conversion routines. Library
		 *				users should generally not need to call this function.
		 */
		[[nodiscard]] frame_data_type frameData() const override { return m_frameData; }

		/**
		 * @brief		set value of point.
		 * @details		Performs unit conversions as necessary. Does not affect the ancillary data about
		 *				the frame.
		 * @param[in]	point	position of the point as a tuple.
		 * @note		This function is intended for use by implicit conversion routines. Library
		 *				users should generally not need to call this function.
		 */
		void setPoint(const tuple_type& point) override
		{
			m_latitude  = std::get<0>(point);
			m_longitude = std::get<1>(point);
			m_altitude  = std::get<2>(point);
		}

		/**
		 * @brief		set value of point.
		 * @details		Performs unit conversions as necessary. Does not affect the ancillary data about
		 *				the frame.
		 * @param[in]	point	position of the point as a tuple.
		 * @note			This function is intended for use by implicit conversion routines. Library
		 *				users should generally not need to call this function.
		 */
		void setPoint(tuple_type&& point) override
		{
			m_latitude  = std::move(std::get<0>(point));
			m_longitude = std::move(std::get<1>(point));
			m_altitude  = std::move(std::get<2>(point));
		}

		/**
		 * @brief		set value of point.
		 * @details		Performs unit conversions as necessary. Does not affect the ancillary data about
		 *				the frame.
		 * @param[in]	args	values to be forwarded to the tuple constructor.
		 */
		template<class... Args>
		    requires (sizeof...(Args) > 0) && std::constructible_from<tuple_type, Args...>
		void setPoint(Args&&... args)
		{
			this->setPoint(tuple_type{std::forward<Args>(args)...});
		}

		/**
		 * @brief		set the ancillary frame data.
		 * @details		Frame data is required to support certain frame of reference conversions.
		 * @param[in]	frameData	value of the frame data.
		 * @note		This function is intended for use by implicit conversion routines. Library
		 *				users should generally not need to call this function.
		 */
		void setFrameData(const frame_data_type& frameData) override { m_frameData = frameData; }

		/**
		 * @brief		set the ancillary frame data.
		 * @details		Frame data is required to support certain frame of reference conversions.
		 * @param[in]	frameData	value of the frame data.
		 * @note		This function is intended for use by implicit conversion routines. Library
		 *				users should generally not need to call this function.
		 */
		void setFrameData(frame_data_type&& frameData) override { m_frameData = std::move(frameData); }


	public:
		//////////////////////////////////////////////////////////////////////////
		//		GEODESIC CONVENIENCE
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		Computes the inverse geodesic solution to another geodetic point.
		 * @details		Uses the datum's reference ellipsoid.
		 * @param[in]	other	Other geodetic point.
		 * @return		GeodesicInverseResult containing distance and bearings.
		 */
		GeodesicInverseResult inverseTo(const PositionGeodetic& other) const
		{
			return geodesicInverse<Datum>(*this, other);
		}

		/**
		 * @brief		Computes the geodesic surface distance to another geodetic point.
		 * @param[in]	other	Other geodetic point.
		 * @return		Surface distance along the ellipsoid.
		 */
		ranges::Geodesic distanceTo(const PositionGeodetic& other) const
		{
			return ranges::Geodesic(geodesicDistance<Datum>(*this, other));
		}

		/**
		 * @brief		Computes the initial bearing to another geodetic point.
		 * @param[in]	other	Other geodetic point.
		 * @return		Initial bearing at this point, normalized to [0, 360).
		 */
		angles::Azimuth initialBearingTo(const PositionGeodetic& other) const
		{
			return angles::Azimuth(initialBearing<Datum>(*this, other));
		}

		/**
		 * @brief		Computes the final bearing to another geodetic point.
		 * @param[in]	other	Other geodetic point.
		 * @return		Final bearing at the destination point, normalized to [0, 360).
		 */
		angles::Azimuth finalBearingTo(const PositionGeodetic& other) const
		{
			return angles::Azimuth(finalBearing<Datum>(*this, other));
		}

		/**
		 * @brief		Computes a destination point given an initial bearing and distance.
		 * @details		Uses the datum's reference ellipsoid.
		 * @param[in]	azimuth		Initial bearing.
		 * @param[in]	distance	Surface distance to travel.
		 * @return		GeodesicDirectResult containing the destination and final bearing.
		 */
		GeodesicDirectResult<PositionGeodetic> destination(degrees<> azimuth, meters<> distance) const
		{
			return geodesicDirect<Datum>(*this, azimuth, distance);
		}

	private:
		angle_unit_type  m_latitude;
		angle_unit_type  m_longitude;
		height_unit_type m_altitude;

		frame_data_type m_frameData;
	};

}    // namespace coordinates

#endif    // positionGeodetic_h