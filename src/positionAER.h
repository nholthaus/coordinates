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

#ifndef positionAER_h__
#define positionAER_h__

//------------------------
//	INCLUDES
//------------------------

#include "frameOfReference.h"
#include "point.h"
#include "positionGeodetic.h"

namespace coord
{
	//	----------------------------------------------------------------------------
	//	CLASS		PositionAER
	//  ----------------------------------------------------------------------------
	///	@brief		Position vector in the locally-level elevation-azimuth-Range frame of reference.
	///	@details	A elevation-azimuth-Range frame of reference is a Cartesian frame, whose axes
	///				are centered on a geodetic point at or above the surface of the Earth, 
	///				forming an x-y plane which is tangent to the surface of the Earth, and
	///				whose z-axis points away from the Earth orthogonal to that plane.\n\n
	///
	///				NED frames are useful as a reference to describe aircraft attitude, or
	///				as an origin for a spherical coordinate system. Since the frame is 
	///				Cartesian, it is easy to specify entities based on their relative
	///				positions to a known location.
	///
	/// @sa			https://en.wikipedia.org/wiki/Axes_conventions#Ground_reference_frames:_NED_and_NED
	/// @tparam		Datum	Datum this point is represented in. See coord::datums.
	/// @tparam		Units	Units of the position vector. Defaults to meters. May be possible to
	///						substitute this with m/s to attain a velocity vector.
	/// @tparam		T		underlying storage type of the point vector. Defaults to double.
	//  ----------------------------------------------------------------------------
	template<class Datum, class AzElUnits = units::angle::degrees, class RangeUnits = units::length::meters, typename T = double>
	class PositionAER : public coord::Point<coord::coordinateFrames::AERFrame<Datum>, coord::sphericalTuple, coord::FrameData>
	{
	public:

		//////////////////////////////////////////////////////////////////////////
		//		STATIC ERROR CHECKING
		//////////////////////////////////////////////////////////////////////////

		static_assert(units::traits::is_unit<AzElUnits>::value, "Template parameter `AzElUnits` template parameter must be a unit type.");
		static_assert(units::traits::is_unit<RangeUnits>::value, "Template parameter `RangeUnits` template parameter must be a unit type.");
		static_assert(coord::traits::is_datum<Datum>::value, "`Datum` template parameter does not satisfy the datum concept.");
		static_assert(std::is_arithmetic<T>::value, "`T` template parameter must be an arithmetic type.");

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using angle_unit_type = units::unit_t<AzElUnits, T>;
		using range_unit_type = units::unit_t<RangeUnits, T>;
		using degree_t = units::angle::degree_t;
		using year_t = units::time::year_t;

		using origin_type = coord::PositionGeodetic<Datum, AzElUnits, RangeUnits, T>;
		using tuple_type = typename coord::Point<coord::coordinateFrames::AERFrame<Datum>, coord::sphericalTuple, coord::FrameData>::tuple_type;
		using frame_data_type = typename coord::Point<coord::coordinateFrames::AERFrame<Datum>, coord::sphericalTuple, coord::FrameData>::frame_data_type;
		using reference_frame = typename coord::Point<coord::coordinateFrames::AERFrame<Datum>, coord::sphericalTuple, coord::FrameData>::reference_frame;

		using datum_type = Datum;

	public:

		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/**
		* @brief		default constructor
		* @details		Creates a point object with the value (0,0,0) @ (0,0,0).
		*/
		PositionAER() :
			m_azimuth(0), m_elevation(0), m_range(0), m_frameData(Datum::epoch())
		{

		}

		/**
		* @brief		Constructor.
		* @details
		* @param[in]	azimuth				Azimuth angle from origin to this position.
		* @param[in]	elevation			elevation angle from origin to this position.
		* @param[in]	range				Slant range from origin to this position.
		* @param[in]	latitude0			geodetic latitude of the origin.
		* @param[in]	longitude0			geodetic longitude of the origin.
		* @param[in]	altitude0			geodetic altitude of the origin.
		* @param[in]	dateOfObservation	Date the observation was made. This field is necessary if
		*									the position value will be converted between different
		*									datums.
		*/
		PositionAER(angle_unit_type azimuth, angle_unit_type elevation, range_unit_type range, angle_unit_type latitude0, angle_unit_type longitude0, range_unit_type altitude0, year_t dateOfObservation = Datum::epoch())
			:
			m_azimuth(azimuth),
			m_elevation(elevation),
			m_range(range),
			m_frameData(coord::sphericalTuple(latitude0, longitude0, altitude0), dateOfObservation)
		{

		}

		/**
		* @brief		Constructor.
		* @details
		* @param[in]	azimuth				azimuthward component of the position.
		* @param[in]	elevation				elevationward component of the position.
		* @param[in]	range					rangeward component of the position.
		* @param[in]	origin				A geodetic point, in the same datum, representing the origin
		*									of this NED coordinate system. May be implicitly converted
		*									from other point types and datums.
		* @param[in]	dateOfObservation	Date the observation was made. This field is necessary if
		*									the position value will be converted between different
		*									datums.
		*/
		PositionAER(angle_unit_type azimuth, angle_unit_type elevation, range_unit_type range, const origin_type& origin, year_t dateOfObservation = Datum::epoch())
			:
			m_azimuth(azimuth),
			m_elevation(elevation),
			m_range(range),
			m_frameData(origin.point(), dateOfObservation)
		{

		}

		/**
		* @brief		constructs a Point object from a tuple of values
		* @details		constructs a new Point object, performing unit conversions if necessary.
		* @param[in]	point tuple containing the xyz position of the point.
		* @param[in]	dateOfObservation	date, in decimal years, when the measurement was taken. This
		*					can be used to account for tectonic plate shift in the coordinate conversion
		*					if necessary. It is safe to omit if no datum conversion will be performed,
		*					or if the time-dependent correction is undesirable.
		*/
		PositionAER(const tuple_type& point, const origin_type& origin, year_t dateOfObservation = Datum::epoch())
			:
			m_azimuth(std::get<0>(point)),
			m_elevation(std::get<1>(point)),
			m_range(std::get<2>(point)),
			m_frameData(origin.point(), dateOfObservation)
		{

		}

		/**
		* @brief		origin-only constructor
		* @details		constructs a PointNED at offset (0,0,0) from the given origin, performing unit
		* 				and coordinate frame conversions if necessary.
		* @param[in]	origin				A geodetic point, in the same datum, representing the origin
		*									of this NED coordinate system. May be implicitly converted
		*									from other point types and datums.
		* @param[in]	dateOfObservation	date, in decimal years, when the measurement was taken. This
		*									can be used to account for tectonic plate shift in the coordinate conversion
		*									if necessary. It is safe to omit if no datum conversion will be performed,
		*									or if the time-dependent correction is undesirable.
		*/
		explicit PositionAER(const origin_type& origin, year_t dateOfObservation = Datum::epoch())
			:
			m_azimuth(angle_unit_type(0)),
			m_elevation(angle_unit_type(0)),
			m_range(range_unit_type(0)),
			m_frameData(origin.point(), dateOfObservation)
		{
			// this needs to be explicit, or else it will become the de-facto implicit conversion constructor
		}

		/**
		* @brief		copy constructor
		* @details		creates a copy of <i>other</i>, performing unit conversions if necessary.
		* @param[in]	other PointECEF to copy
		* @returns		copy of <i>other</i>, with appropriate unit conversions performed.
		*/
		PositionAER(const PositionAER& other) :
			m_azimuth(other.m_azimuth),
			m_elevation(other.m_elevation),
			m_range(other.m_range),
			m_frameData(other.m_frameData)
		{

		}

		/**
		* @brief		implicit conversion constructor
		* @details		constructs a PointECEF from another point, invoking all necessary unit conversions
		* 				and coordinate frame conversions.
		* @param[in]	point				point to construct from
		* @param[in]	origin				A geodetic point, in the same datum, representing the origin
		*									of this NED coordinate system. May be implicitly converted
		*									from other point types and datums.
		* @param[in]	dateOfObservation	date, in decimal years, when the measurement was taken. This
		*									can be used to account for tectonic plate shift in the coordinate conversion
		*									if necessary. It is safe to omit if no datum conversion will be performed,
		*									or if the time-dependent correction is undesirable.
		*/
		template<class P, class = typename std::enable_if<coord::traits::is_point<P>::value>::type>
		PositionAER(const P& point, const origin_type& origin, year_t dateOfObservation = Datum::epoch())
			:
			m_frameData(origin.point(), dateOfObservation)
		{
			if (point.isSame(origin))
			{
				// az/el angles can be really weird for points which are off by a nano or pico meter,
				// so if the origin and point meet the "is same" criteria, set the AER to all zeros.
				this->setPoint(angle_unit_type(0), angle_unit_type(0), range_unit_type(0));
			}
			else
			{
				// convert the point to an intermediate value, so that two NED's with different origins 
				// (which type-wise are otherwise identical) will properly be converted.
				origin_type intermediate;
				intermediate.setFrameData(dateOfObservation);

				coord::convert(point, intermediate);
				coord::convert(intermediate, *this);
			}
		}

		/**
		* @brief		assignment operator
		* @details		performs unit conversions if necessary
		* @param[in]	other Point value to assign to this point
		*/
		PositionAER& operator=(const PositionAER& other)
		{
			if (this->isNull())
			{
				m_azimuth = other.m_azimuth;
				m_elevation = other.m_elevation;
				m_range = other.m_range;
				m_frameData = other.m_frameData;
				return *this;
			}
			else
			{
				// take the date of observation, but DON'T blow away the origin.
				m_frameData.date = other.frameData().date;

				// convert the point to an intermediate value, so that two NED's with different origins 
				// (which type-wise are otherwise identical) will properly be converted.
				origin_type intermediate;
				intermediate.setFrameData(m_frameData.date);

				coord::convert(other, intermediate);
				coord::convert(intermediate, *this);
			}

			return *this;
		}

		/**
		* @brief		implicit conversion assignment
		* @details		assigns a PointECEF from another point, invoking all necessary unit conversions
		* 				and coordinate frame conversions.
		* @param[in]	point point to construct from
		* @returns		copy of *this
		*/
		template<class Point/*, class = typename std::enable_if<coord::traits::is_point<Point>::value && !std::is_convertible<Point, PositionAER>::value>::type*/>
		PositionAER& operator=(const Point& point)
		{
			// take the date of observation, but DON'T blow away the origin unless this is a null value.
			if (this->isNull())
			{
				m_frameData = point.frameData();
			}
			else
			{
				m_frameData.date = point.frameData().date;
			}

			// convert the point to an intermediate value, so that two NED's with different origins 
			// (which type-wise are otherwise identical) will properly be converted.
			origin_type intermediate;
			intermediate.setFrameData(m_frameData.date);

			coord::convert(point, intermediate);
			coord::convert(intermediate, *this);

			return *this;
		}

		virtual ~PositionAER() {}

		// friend other specializations of the template.
		template<class D, class ATy, class RTy, class DU>
		friend class PositionAER;

	public:

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC MEMBERS
		//////////////////////////////////////////////////////////////////////////

		/**
		* @brief		is point null
		* @details		returns true if the point is (0,0,0) @ (0,0,0).
		* @returns		true if null, false otherwise.
		*/
		bool isNull() const
		{
			return coord::isNull(*this);
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
		bool isSame(const Point& p, range_unit_type tolerance = range_unit_type(0)) const
		{
			return coord::isSame<range_unit_type>(*this, p, tolerance);
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
		template<class Point, class PointTol, class = typename std::enable_if<coord::traits::is_point<PointTol>::value>::type>
		bool isSame(const Point& p, const PointTol& tolerance) const
		{
			return coord::isSame(*this, p, tolerance);
		}

		/**
		* @brief		calculates the distance between two points.
		* @details		calculates the euclidean distance between this point and point <i>p</i>
		* @note		This is the straight-line distance, *not* the great-circle distance.
		* @tparam		UnitType	desired units of the output. Defaults to <i>unit_type</i>,
		*				i.e. the same units as this instance of the class. Ordinarily this parameter
		*				does not need to be changed, since `unit` types are implicitly convertible
		*				to one another, but setting it may save unnecessary conversions in some
		*				circumstances.
		* @param[in]	p	point to calculate distance to.
		* @returns		distance between this Point and <i>p</i> in units of distance_units.
		*/
		template<class Point, typename UnitType = range_unit_type>
		UnitType distance(const Point& p) const
		{
			return coord::distance(*this, p);
		}

		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS
		//////////////////////////////////////////////////////////////////////////

		/**
		* @brief		set azimuth value
		* @details		overwrites the azimuth-value with the value provided, performing a unit conversion
		*				if necessary.
		* @param[in]	azimuth new value of <i>azimuth</i>
		*/
		void setAzimuth(angle_unit_type azimuth)
		{
			m_azimuth = azimuth;
		}

		/**
		* @brief		set elevation value
		* @details		overwrites the elevation-value with the value provided, performing a unit conversion
		*				if necessary.
		* @param[in]	elevation new value of <i>x</i>
		*/
		void setElevation(angle_unit_type elevation)
		{
			m_elevation = elevation;
		}

		/**
		* @brief		set range value
		* @details		overwrites the range-value with the value provided, performing a unit conversion
		*				if necessary.
		* @param[in]	range new value of <i>range</i>
		*/
		void setRange(range_unit_type range)
		{
			m_range = range;
		}

		/**
		* @brief		set the Origin value
		* @details		This function moves the origin of the point, <i>while preserving the points
		*				absolute location in geodetic space</i>. To change the origin without altering
		*				the x/y/z value (i.e. for a moving plaform), use `setFrameData()`.
		* @param[in]	origin	new value of the origin
		*/
		void setOrigin(origin_type origin)
		{
			// convert the point to an intermediate value, so that two NED's with different origins 
			// (which type-wise are otherwise identical) will properly be converted.
			origin_type intermediate;
			intermediate.setFrameData(this->m_frameData);

			coord::convert(*this, intermediate);
			this->m_frameData.origin = origin.point();
			coord::convert(intermediate, *this);
		}

		/**
		* @brief		azimuth-value
		* @details		returns the azimuth-value of the point, performing a unit-conversion to <i>UnitsTo</i> if
		*				necessary.
		* @returns		azimuth-value of the Point.
		*/
		angle_unit_type azimuth() const
		{
			// remember, convert automatically uses the most efficient algorithm, including to/fromBase
			// and directly returning values if they are the same.
			return m_azimuth;
		}

		/**
		* @brief		elevation-value
		* @details		returns the elevation-value of the point, performing a unit-conversion to <i>UnitsTo</i> if
		*				necessary.
		* @returns		elevation-value of the Point.
		*/
		angle_unit_type elevation() const
		{
			return m_elevation;
		}

		/**
		* @brief		range-value
		* @details		returns the range-value of the point, performing a unit-conversion to <i>UnitsTo</i> if
		*				necessary.
		* @returns		range-value of the Point.
		*/
		range_unit_type range() const
		{
			return m_range;
		}

		/**
		* @brief		Origin
		* @details		The origin of the NED coordinate system.
		* @returns		origin value as an <i>origin_type</i>
		*/
		origin_type origin() const
		{
			return origin_type(m_frameData.origin, m_frameData.date);
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
		* @details		Prints the point value in the form "(x, y, z)"
		* @param[in]	os	output stream
		* @param[in]	p	point to print
		* @returns		reference to the output stream.
		*/
		friend std::ostream& operator<<(std::ostream& os, const PositionAER& p)
		{
			return os << "(" << p.m_azimuth << ", " << p.m_elevation << ", " << p.m_range << ") @ (" <<
				std::get<0>(p.m_frameData.origin) << ", " << std::get<1>(p.m_frameData.origin) << ", " <<
				std::get<2>(p.m_frameData.origin) << ")";
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
			return tuple_type(m_azimuth, m_elevation, m_range);
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
			m_azimuth = std::get<0>(point);
			m_elevation = std::get<1>(point);
			m_range = std::get<2>(point);
		}

		/**
		* @brief		set value of point.
		* @details		Performs unit conversions as necessary. Does not affect the ancillary data about
		*				the frame.
		* @param[in]	point	position of the point as a tuple.
		* @note		This function is intended for use by implicit conversion routines. Library
		*				users should generally not need to call this function.
		*/
		virtual void setPoint(tuple_type&& point) override
		{
			m_azimuth = std::move(std::get<0>(point));
			m_elevation = std::move(std::get<1>(point));
			m_range = std::move(std::get<2>(point));
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

		angle_unit_type			m_azimuth;
		angle_unit_type			m_elevation;
		range_unit_type			m_range;

		frame_data_type			m_frameData;


	};
}


#endif // positionAER_h__