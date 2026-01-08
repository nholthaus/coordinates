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

#ifndef positionGeodetic_h__
#define positionGeodetic_h__

//------------------------
//	INCLUDES
//------------------------

#include <iostream>

#include <units.h>
#include "frameOfReference.h"
#include "point.h"
#include "algorithm.h"

namespace coordinates
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
	template<class Datum, class LatLonUnits = units::angle::degrees, class HeightUnits = units::length::meters, typename T = double>
	class PositionGeodetic : public coordinates::Point<coordinates::coordinateFrames::Geodetic3DFrame<Datum>, coordinates::sphericalTuple, coordinates::FrameData>
	{
	public:

		//////////////////////////////////////////////////////////////////////////
		//		STATIC ERROR CHECKING
		//////////////////////////////////////////////////////////////////////////

		static_assert(units::traits::is_unit<LatLonUnits>::value, "Template parameter `LatLonUnits` template parameter must be a unit type.");
		static_assert(units::traits::is_unit<HeightUnits>::value, "Template parameter `HeightUnits` template parameter must be a unit type.");
		static_assert(coordinates::traits::is_datum<Datum>::value, "`Datum` template parameter does not satisfy the datum concept.");
		static_assert(std::is_arithmetic<T>::value, "`T` template parameter must be an arithmetic type.");

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using tuple_type = typename coordinates::Point<coordinates::coordinateFrames::Geodetic3DFrame<Datum>, coordinates::sphericalTuple, coordinates::FrameData>::tuple_type;
		using frame_data_type = typename coordinates::Point<coordinates::coordinateFrames::Geodetic3DFrame<Datum>, coordinates::sphericalTuple, coordinates::FrameData>::frame_data_type;

		using datum_type = Datum;

		using lat_lon_unit_type = units::unit_t<LatLonUnits, T>;
		using height_unit_type = units::unit_t<HeightUnits, T>;
		using year_t = units::time::year_t;

	public:

		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/**
		* @brief		default constructor
		* @details		Creates a point object with the value (0,0,0).
		*/
		explicit PositionGeodetic() :
			m_latitude(0), m_longitude(0), m_altitude(0), m_frameData(Datum::epoch())
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
		explicit PositionGeodetic(lat_lon_unit_type latitude, lat_lon_unit_type longitude, height_unit_type altitude, year_t dateOfObservation = Datum::epoch()) :
			m_latitude(latitude), m_longitude(longitude), m_altitude(altitude), m_frameData(dateOfObservation)
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
		explicit PositionGeodetic(tuple_type point, year_t dateOfObservation = Datum::epoch()) :
			m_latitude(std::get<0>(point)), m_longitude(std::get<1>(point)), m_altitude(std::get<2>(point)), m_frameData(dateOfObservation)
		{

		}

		/**
		* @brief		copy constructor
		* @details		creates a copy of <i>other</i>, performing unit conversions if necessary.
		* @param[in]	other PositionGeodetic to copy
		* @returns		copy of <i>other</i>, with appropriate unit conversions performed.
		*/
		PositionGeodetic(const PositionGeodetic& other) :
			m_latitude(other.m_latitude), m_longitude(other.m_longitude), m_altitude(other.m_altitude), m_frameData(other.m_frameData)
		{

		}

		/**
		* @brief		implicit conversion constructor
		* @details		constructs a PointECEF from another point, invoking all necessary unit conversions
		* 				and coordinate frame conversions.
		* @param[in]	point point to construct from
		* @param[in]	dateOfObservation	date, in decimal years, when the measurement was taken. This
		*					can be used to account for tectonic plate shift in the coordinate conversion
		*					if necessary. It is safe to omit if no datum conversion will be performed,
		*					or if the time-dependent correction is undesirable.
		*/
		template<class P, class = typename std::enable_if<coordinates::traits::is_point<P>::value && !std::is_convertible<P, PositionGeodetic>::value>::type>
		PositionGeodetic(const P& point) :
			m_frameData(point.frameData())
		{
			coordinates::convert(point, *this);
		}

		/**
		* @brief		assignment operator
		* @details		performs unit conversions if necessary
		* @param[in]	other Point value to assign to this point
		*/
		PositionGeodetic& operator=(const PositionGeodetic& other)
		{
			m_latitude = other.m_latitude;
			m_longitude = other.m_longitude;
			m_altitude = other.m_altitude;
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
		template<class Point, class = typename std::enable_if<coordinates::traits::is_point<Point>::value && !std::is_convertible<Point, PositionGeodetic>::value>::type>
		PositionGeodetic& operator=(const Point& point)
		{
			m_frameData = point.frameData();
			coordinates::convert(point, *this);
			return *this;
		}

		virtual ~PositionGeodetic() {}

		// friend other specializations of the template.
		template<class D, class LLATy, class HTy, class DU>
		friend class PositionGeodetic;

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC MEMBERS
		//////////////////////////////////////////////////////////////////////////

	public:

		/**
		* @brief		is point null
		* @details		returns true if the point is (0,0,0).
		* @returns		true if null, false otherwise.
		*/
		bool isNull() const
		{
			return coordinates::isNull(*this);
		}

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
		template<class Point>
		bool isSame(const Point& p, height_unit_type tolerance = height_unit_type(0)) const
		{
			return coordinates::isSame<height_unit_type>(*this, p, tolerance);
		}

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
		template<class Point, class PointTol, class = typename std::enable_if<coordinates::traits::is_point<PointTol>::value>::type>
		bool isSame(const Point& p, const PointTol& tolerance) const
		{
			return coordinates::isSame(*this, p, tolerance);
		}

		/**
		* @brief		calculates the distance between two points.
		* @details		calculates the euclidean distance between this point and point <i>p</i>
		* @note			This is the straight-line distance, *not* the great-circle distance.
		* @tparam		UnitType	desired units of the output. Defaults to <i>unit_type</i>,
		*				i.e. the same units as this instance of the class. Ordinarily this parameter
		*				does not need to be changed, since `unit` types are implicitly convertible
		*				to one another, but setting it may save unnecessary conversions in some
		*				circumstances.
		* @param[in]	p	point to calculate distance to.
		* @returns		distance between this Point and <i>p</i> in units of distance_units.
		*/
		template<class Point, typename UnitType = height_unit_type>
		UnitType distance(const Point& p) const
		{
			return coordinates::distance(*this, p);
		}

		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS
		//////////////////////////////////////////////////////////////////////////

		/**
		* @brief		set latitude value
		* @details		overwrites the latitude-value with the value provided, performing a unit conversion
		*				if necessary.
		* @param[in]	latitude new value of <i>latitude</i>
		*/
		void setLatitude(lat_lon_unit_type latitude)
		{
			m_latitude = latitude;
		}

		/**
		* @brief		set longitude value
		* @details		overwrites the longitude-value with the value provided, performing a unit conversion
		*				if necessary.
		* @param[in]	longitude new value of <i>longitude</i>
		*/
		void setLongitude(lat_lon_unit_type longitude)
		{
			m_longitude = longitude;
		}

		/**
		* @brief		set altitude value
		* @details		overwrites the altitude-value with the value provided.
		* @param[in]	altitude new value of <i>altitude</i>
		*/
		void setAltitude(height_unit_type altitude)
		{
			m_altitude = altitude;
		}

		/**
		* @brief		latitude-value
		* @details		returns the latitude-value of the point.
		* @returns		latitude-value of the Point.
		*/
		lat_lon_unit_type latitude() const
		{
			// remember, convert automatically uses the most efficient algorithm, including to/fromBase
			// and directly returning values if they are the same.
			return m_latitude;
		}

		/**
		* @brief		longitude-value
		* @details		returns the longitude-value of the point.
		* @returns		longitude-value of the Point.
		*/
		lat_lon_unit_type longitude() const
		{
			return m_longitude;
		}

		/**
		* @brief		altitude-value
		* @details		returns the altitude-value of the point, performing a unit-conversion to <i>UnitsTo</i> if
		*				necessary.
		* @returns		altitude-value of the Point.
		*/
		height_unit_type altitude() const
		{
			return m_altitude;
		}

		/**
		* @brief		Date of observation.
		* @details		This parameter is only relevant when converting between datums.
		* @returns		Date, in decimal years, when the position was measured.
		*/
		year_t date() const
		{
			return m_frameData.date;
		}

		/**
		 * @brief		print value
		 * @details		Prints the point value in the form "(lat, lon, alt)"
		 * @param[in]	os	output stream
		 * @param[in]	p	point to print
		 * @returns		reference to the output stream.
		 */
		friend std::ostream& operator<<(std::ostream& os, const PositionGeodetic& p)
		{
			return os << "(" << p.m_latitude << ", " << p.m_longitude << ", " << p.m_altitude << ")";
		}

	public:

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
		virtual tuple_type point() const override
		{
			return tuple_type(m_latitude, m_longitude, m_altitude);
		}

		/**
		* @brief		Returns the frame data.
		* @details		Frame data is the ancillary data about the relationship between the frame of
		*				reference and its base frame. This could include offsets in time or space or
		*				axis rotation.
		* @returns		ancillary frame data.
		* @note		This function is intended for use by implicit conversion routines. Library
		*				users should generally not need to call this function.
		*/
		virtual frame_data_type frameData() const override
		{
			return m_frameData;
		}

		/**
		* @brief		set value of point.
		* @details		Performs unit conversions as necessary. Does not affect the ancillary data about
		*				the frame.
		* @param[in]	point	position of the point as a tuple.
		* @note		This function is intended for use by implicit conversion routines. Library
		*				users should generally not need to call this function.
		*/
		virtual void setPoint(const tuple_type& point) override
		{
			m_latitude = std::get<0>(point);
			m_longitude = std::get<1>(point);
			m_altitude = std::get<2>(point);
		}

		/**
		* @brief		set value of point.
		* @details		Performs unit conversions as necessary. Does not affect the ancillary data about
		*				the frame.
		* @param[in]	point	position of the point as a tuple.
		* @note			This function is intended for use by implicit conversion routines. Library
		*				users should generally not need to call this function.
		*/
		virtual void setPoint(tuple_type&& point) override
		{
			m_latitude = std::move(std::get<0>(point));
			m_longitude = std::move(std::get<1>(point));
			m_altitude = std::move(std::get<2>(point));
		}

		/**
		 * @brief		set value of point.
		 * @details		Performs unit conversions as necessary. Does not affect the ancillary data about
		 *				the frame.
		 * @param[in]	args	values to be forwarded to the tuple constructor.
		 */
		template<class... Args>
		void setPoint(Args... args)
		{
			this->setPoint(std::make_tuple<Args...>(std::forward<Args>(args)...));
		}

		/**
		* @brief		set the ancillary frame data.
		* @details		Frame data is required to support certain frame of reference conversions.
		* @param[in]	frameData	value of the frame data.
		* @note		This function is intended for use by implicit conversion routines. Library
		*				users should generally not need to call this function.
		*/
		virtual void setFrameData(const frame_data_type& frameData) override
		{
			m_frameData = frameData;
		}

		/**
		* @brief		set the ancillary frame data.
		* @details		Frame data is required to support certain frame of reference conversions.
		* @param[in]	frameData	value of the frame data.
		* @note		This function is intended for use by implicit conversion routines. Library
		*				users should generally not need to call this function.
		*/
		virtual void setFrameData(frame_data_type&& frameData) override
		{
			m_frameData = std::move(frameData);
		}

	private:

		lat_lon_unit_type	m_latitude;
		lat_lon_unit_type	m_longitude;
		height_unit_type	m_altitude;

		frame_data_type		m_frameData;
	};

}

#endif // positionGeodetic_h__