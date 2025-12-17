#ifndef positionNED_h__
#define positionNED_h__

//------------------------
//	INCLUDES
//------------------------

#include "frameOfReference.h"
#include "point.h"
#include "positionGeodetic.h"

namespace coord
{
	//	----------------------------------------------------------------------------
	//	CLASS		PositionNED
	//  ----------------------------------------------------------------------------
	///	@brief		Position vector in the locally-level east-north-Down frame of reference.
	///	@details	A east-north-Down frame of reference is a Cartesian frame, whose axes
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
	template<class Datum, class Units = units::length::meter, typename T = double>
	class PositionNED : public coord::Point<coord::coordinateFrames::NEDFrame<Datum>, coord::cartesianTuple, coord::FrameData>
	{
	public:

		//////////////////////////////////////////////////////////////////////////
		//		STATIC ERROR CHECKING
		//////////////////////////////////////////////////////////////////////////

		static_assert(units::traits::is_unit<Units>::value, "Template parameter `Units` template parameter must be a unit type.");
		static_assert(coord::traits::is_datum<Datum>::value, "`Datum` template parameter does not satisfy the datum concept.");
		static_assert(std::is_arithmetic<T>::value, "`T` template parameter must be an arithmetic type.");

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using unit_type = units::unit_t<Units, T>;
		using degree_t = units::angle::degree_t;
		using year_t = units::time::year_t;

		using origin_type = coord::PositionGeodetic<Datum>;
		using tuple_type = typename coord::Point<coord::coordinateFrames::NEDFrame<Datum>, coord::cartesianTuple, coord::FrameData>::tuple_type;
		using frame_data_type = typename coord::Point<coord::coordinateFrames::NEDFrame<Datum>, coord::cartesianTuple, coord::FrameData>::frame_data_type;
		using reference_frame = typename coord::Point<coord::coordinateFrames::NEDFrame<Datum>, coord::cartesianTuple, coord::FrameData>::reference_frame;

		using datum_type = Datum;

	public:

		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/**
		* @brief		default constructor
		* @details		Creates a point object with the value (0,0,0) @ (0,0,0).
		*/
		PositionNED() :
			m_north(0), m_east(0), m_down(0), m_frameData(Datum::epoch())
		{

		}

		/**
		* @brief		Constructor.
		* @details
		* @param[in]	north				northward component of the position.
		* @param[in]	east				eastward component of the position.
		* @param[in]	down				downward component of the position.
		* @param[in]	latitude0			geodetic latitude of the origin.
		* @param[in]	longitude0			geodetic longitude of the origin.
		* @param[in]	altitude0			geodetic altitude of the origin.
		* @param[in]	dateOfObservation	Date the observation was made. This field is necessary if
		*									the position value will be converted between different
		*									datums.
		*/
		PositionNED(unit_type north, unit_type east, unit_type down, degree_t latitude0, degree_t longitude0, unit_type altitude0, year_t dateOfObservation = Datum::epoch())
			:
			m_north(north),
			m_east(east),
			m_down(down),
			m_frameData(coord::sphericalTuple(latitude0, longitude0, altitude0), dateOfObservation)
		{

		}

		/**
		* @brief		Constructor.
		* @details
		* @param[in]	north				northward component of the position.
		* @param[in]	east				eastward component of the position.
		* @param[in]	down					downward component of the position.
		* @param[in]	origin				A geodetic point, in the same datum, representing the origin
		*									of this NED coordinate system. May be implicitly converted
		*									from other point types and datums.
		* @param[in]	dateOfObservation	Date the observation was made. This field is necessary if
		*									the position value will be converted between different
		*									datums.
		*/
		PositionNED(unit_type north, unit_type east, unit_type down, const origin_type& origin, year_t dateOfObservation = Datum::epoch())
			:
			m_north(north),
			m_east(east),
			m_down(down),
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
		PositionNED(const tuple_type& point, const origin_type& origin, year_t dateOfObservation = Datum::epoch())
			:
			m_north(std::get<0>(point)),
			m_east(std::get<1>(point)),
			m_down(std::get<2>(point)),
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
		explicit PositionNED(const origin_type& origin, year_t dateOfObservation = Datum::epoch())
			:
			m_north(unit_type(0)),
			m_east(unit_type(0)),
			m_down(unit_type(0)),
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
		PositionNED(const PositionNED& other) :
			m_north(other.m_north),
			m_east(other.m_east),
			m_down(other.m_down),
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
		PositionNED(const P& point, const origin_type& origin, year_t dateOfObservation = Datum::epoch())
			:
			m_frameData(origin.point(), dateOfObservation)
		{
			// convert the point to an intermediate value, so that two NED's with different origins 
			// (which type-wise are otherwise identical) will properly be converted.
			origin_type intermediate;
			intermediate.setFrameData(dateOfObservation);

			coord::convert(point, intermediate);
			coord::convert(intermediate, *this);
		}

		/**
		* @brief		assignment operator
		* @details		performs unit conversions if necessary
		* @param[in]	other Point value to assign to this point
		*/
		PositionNED& operator=(const PositionNED& other)
		{
			if (this->isNull())
			{
				m_north = other.m_north;
				m_east = other.m_east;
				m_down = other.m_down;
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
		template<class Point/*, class = typename std::enable_if<coord::traits::is_point<Point>::value && !std::is_convertible<Point, PositionNED>::value>::type*/>
		PositionNED& operator=(const Point& point)
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

		virtual ~PositionNED() {}

		// friend other specializations of the template.
		template<class D, class Ty, class DU>
		friend class PositionNED;

	public:

		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC MEMBERS
		//////////////////////////////////////////////////////////////////////////

		/**
		* @brief		is point null
		* @details		returns true if the point is (0,0,0).
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
		bool isSame(const Point& p, unit_type tolerance = unit_type(0)) const
		{
			return coord::isSame<unit_type>(*this, p, tolerance);
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
		template<class Point, typename UnitType = unit_type>
		UnitType distance(const Point& p) const
		{
			return coord::distance(*this, p);
		}

		/**
		* @brief		Calculates the dot product of two points.
		* @details		https://en.wikipedia.org/wiki/Dot_product
		* @param[in]	p Point to calculate the dot product with this point
		* @returns		dot product between this point and point <i>p</i>.
		*/
		template<class Point>
		auto dotProduct(const Point& p) const -> decltype(units::math::pow<2>(unit_type()))
		{
			return coord::dotProduct(*this, p);
		}

		/**
		* @brief		Calculates the magnitude of the point (from the origin)
		* @details
		* @param[in]	p Point to calculate the magnitude of
		* @returns		magnitude of the point vector wrt its origin.
		*/
		unit_type magnitude() const
		{
			return coord::magnitude(*this);
		}

		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS
		//////////////////////////////////////////////////////////////////////////

		/**
		* @brief		set north value
		* @details		overwrites the north-value with the value provided, performing a unit conversion
		*				if necessary.
		* @param[in]	north new value of <i>north</i>
		*/
		void setNorth(unit_type north)
		{
			m_north = north;
		}

		/**
		* @brief		set east value
		* @details		overwrites the east-value with the value provided, performing a unit conversion
		*				if necessary.
		* @param[in]	east new value of <i>x</i>
		*/
		void setEast(unit_type east)
		{
			m_east = east;
		}

		/**
		* @brief		set down value
		* @details		overwrites the down-value with the value provided, performing a unit conversion
		*				if necessary.
		* @param[in]	down new value of <i>down</i>
		*/
		void setDown(unit_type down)
		{
			m_down = down;
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
		* @brief		north-value
		* @details		returns the north-value of the point, performing a unit-conversion to <i>UnitsTo</i> if
		*				necessary.
		* @returns		north-value of the Point.
		*/
		unit_type north() const
		{
			// remember, convert automatically uses the most efficient algorithm, including to/fromBase
			// and directly returning values if they are the same.
			return m_north;
		}

		/**
		* @brief		east-value
		* @details		returns the east-value of the point, performing a unit-conversion to <i>UnitsTo</i> if
		*				necessary.
		* @returns		east-value of the Point.
		*/
		unit_type east() const
		{
			return m_east;
		}

		/**
		* @brief		down-value
		* @details		returns the down-value of the point, performing a unit-conversion to <i>UnitsTo</i> if
		*				necessary.
		* @returns		down-value of the Point.
		*/
		unit_type down() const
		{
			return m_down;
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
		friend std::ostream& operator<<(std::ostream& os, const PositionNED& p)
		{
			return os << "(" << p.m_north << ", " << p.m_east << ", " << p.m_down << ") @ (" <<
				std::get<0>(p.m_frameData.origin) << ", " << std::get<1>(p.m_frameData.origin) << ", " <<
				std::get<2>(p.m_frameData.origin) << ")";
		}

	public:

		//////////////////////////////////////////////////////////////////////////
		//		ARITHMETIC OPERATORS
		//////////////////////////////////////////////////////////////////////////

		/**
		* @brief		multiplication operator
		* @details		multiplies this point's coordinates by the given <i>factor</i>, and returns
		* 				a reference to this point.
		* @sa			operator/=
		* @param[in]	factor multiplication factor
		* @returns		reference to this point.
		*/
		PositionNED& operator*=(units::dimensionless::scalar_t factor)
		{
			m_north = m_north * factor;
			m_east = m_east * factor;
			m_down = m_down * factor;
			return *this;
		}

		/**
		* @brief 		addition operator
		* @details		adds the given point to this point and returns a reference to this point.
		* @sa			operator-=
		* @param[in] 	point	point to add to this point.
		* @return 		reference to this point.
		*/
		template<typename D, typename U, typename Ty>
		PositionNED& operator+=(const PositionNED<D, U, Ty>& point)
		{
			PositionNED p(point);

			if (m_frameData != p.m_frameData)
			{
				PositionECEF<Datum, Units, T> temp;
				temp.setFrameData(m_frameData.date);

				coord::convert(p, temp);
				p.setFrameData(m_frameData);
				coord::convert(temp, p);
			}

			m_north = m_north + p.north();
			m_east = m_east + p.east();
			m_down = m_down + p.down();

			return *this;
		}

		/**
		* @brief 		subtraction operator
		* @details		subtracts the given point from this point and returns a reference to this point.
		* @sa			operator+=
		* @param[in]	point	point to subtract from this point.
		* @return 		reference to this point.
		*/
		template<typename D, typename U, typename Ty>
		PositionNED& operator-=(const PositionNED<D, U, Ty>& point)
		{
			PositionNED p(point);

			if (m_frameData != p.m_frameData)
			{
				PositionECEF<Datum, Units, T> temp;
				temp.setFrameData(m_frameData.date);

				coord::convert(p, temp);
				p.setFrameData(m_frameData);
				coord::convert(temp, p);
			}

			m_north = m_north - p.north();
			m_east = m_east - p.east();
			m_down = m_down - p.down();

			return *this;
		}

		/**
		* @brief 		division operator
		* @details		divides each coordinate of the point by <i>divisor</i> and returns a reference
		* 				to this point.
		* @param[in]	divisor divisor for the division operation
		* @return 		reference to this point.
		*/
		PositionNED& operator/=(units::dimensionless::scalar_t divisor)
		{
			m_north = m_north / divisor;
			m_east = m_east / divisor;
			m_down = m_down / divisor;
			return *this;
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
			return tuple_type(m_north, m_east, m_down);
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
			m_north = std::get<0>(point);
			m_east = std::get<1>(point);
			m_down = std::get<2>(point);
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
			m_north = std::move(std::get<0>(point));
			m_east = std::move(std::get<1>(point));
			m_down = std::move(std::get<2>(point));
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

		unit_type			m_north;
		unit_type			m_east;
		unit_type			m_down;

		frame_data_type		m_frameData;


	};
}


#endif // positionNED_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: