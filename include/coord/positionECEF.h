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

#ifndef pointECEF_h__
#define pointECEF_h__

//------------------------
//	INCLUDES
//------------------------

#include <iostream>

#include <units.h>
#include "frameOfReference.h"
#include "point.h"
#include "algorithm.h"

namespace coord
{
	//	----------------------------------------------------------------------------
	//	CLASS		PositionECEF
	//  ----------------------------------------------------------------------------
	///	@brief		A 3D point in the Earth-centered, Earth-fixed frame of reference
	///	@details	For practical purposes, an ECEF from an XYZ only it that it has defined units. However,
	///				since they represent (potentially) different frames of references, they have a
	///				different type to avoid being implicitly convertible.
	///
	///				https://en.wikipedia.org/wiki/ECEF
	///
	///				ECEF ("Earth-Centered, Earth-Fixed"), also known as ECR ("Earth Centered
	///				Rotational"), is a geographic coordinate system and Cartesian coordinate system, and is sometimes
	///				known as a "conventional terrestrial" system.[1] It represents positions as an X, Y, and Z
	///				coordinate. The point (0,0,0) is defined as the center of mass of the Earth,[2] hence the name
	///				Earth-Centered. Its axes are aligned with the International Reference Pole (IRP) and International
	///				Reference Meridian (IRM) that are fixed with respect to the surface of the Earth,[3][4] hence the
	///				name Earth-Fixed. This term can cause confusion since the Earth does not rotate about the z-axis
	///				(unlike an inertial system such as ECI), and is therefore alternatively called ECR.
	///
	///				The z-axis is pointing towards the north but it does not coincide exactly with the instantaneous
	///				Earth rotational axis.[3] The slight "wobbling" of the rotational axis is known as polar motion.[5]
	///				The x-axis intersects the sphere of the Earth at 0� latitude (Equator) and 0� longitude (Greenwich).
	///				This means that ECEF rotates with the earth and therefore, coordinates of a point fixed on the
	///				surface of the earth do not change. Conversion from a WGS84 Datum to ECEF can be used as an
	///				intermediate step in converting velocities to the North East Down coordinate system.
	//  ----------------------------------------------------------------------------
	template<class Datum, class Units = units::length::meter, typename T = double>
	class PositionECEF : public coord::Point<coord::coordinateFrames::ECEFFrame<typename coord::traits::datum_traits<Datum>::horizontal_datum>, coord::cartesianTuple, coord::FrameData>
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
		using year_t = units::time::year_t;
		
		using tuple_type = typename coord::Point<coord::coordinateFrames::ECEFFrame<typename coord::traits::datum_traits<Datum>::horizontal_datum>, coord::cartesianTuple, coord::FrameData>::tuple_type;
		using frame_data_type = typename coord::Point<coord::coordinateFrames::ECEFFrame<typename coord::traits::datum_traits<Datum>::horizontal_datum>, coord::cartesianTuple, coord::FrameData>::frame_data_type;
		using reference_frame = typename coord::Point<coord::coordinateFrames::ECEFFrame<typename coord::traits::datum_traits<Datum>::horizontal_datum>, coord::cartesianTuple, coord::FrameData>::reference_frame;
		
		using datum_type = Datum;

	public:

		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/**
		* @brief		default constructor
		* @details		Creates a point object with the value (0,0,0).
		*/
		explicit PositionECEF() :
			m_x(0), m_y(0), m_z(0), m_frameData(Datum::epoch())
		{

		}

		/**
		* @brief		constructor
		* @details		constructs a new Point object, performing unit conversions if necessary.
		* @param[in]	x	x-value of the point.
		* @param[in]	y	y-value of the point.
		* @param[in]	z	z-value of the point.
		* @param[in]	dateOfObservation	date, in decimal years, when the measurement was taken. This
		*					can be used to account for tectonic plate shift in the coordinate conversion
		*					if necessary. It is safe to omit if no datum conversion will be performed,
		*					or if the time-dependent correction is undesirable.
		*/
		explicit PositionECEF(unit_type x, unit_type y, unit_type z, year_t dateOfObservation = Datum::epoch()) :
			m_x(x), m_y(y), m_z(z), m_frameData(dateOfObservation)
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
		explicit PositionECEF(tuple_type point, year_t dateOfObservation = Datum::epoch()) :
			m_frameData(dateOfObservation)
		{
			m_x = std::get<0>(point);
			m_y = std::get<1>(point);
			m_z = std::get<2>(point);
		}

		/**
		* @brief		copy constructor
		* @details		creates a copy of <i>other</i>, performing unit conversions if necessary.
		* @param[in]	other PointECEF to copy
		* @returns		copy of <i>other</i>, with appropriate unit conversions performed.
		*/
		PositionECEF(const PositionECEF& other) :
			m_x(other.m_x), m_y(other.m_y), m_z(other.m_z), m_frameData(other.m_frameData)
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
		template<class P, class = typename std::enable_if<coord::traits::is_point<P>::value && !std::is_convertible<P, PositionECEF>::value>::type>
		PositionECEF(const P& point) :
			m_frameData(point.frameData())
		{
			coord::convert(point, *this);
		}

		/**
		 * @brief		assignment operator
		 * @details		performs unit conversions if necessary
		 * @param[in]	other Point value to assign to this point
		 */
		PositionECEF& operator=(const PositionECEF& other)
		{
			m_x = other.m_x;
			m_y = other.m_y;
			m_z = other.m_z;
			m_frameData = other.m_frameData;
			return *this;
		}

		/**
		 * @brief		implicit conversion assignment
		 * @details		assigns a PointECEF from another point, invoking all necessary unit conversions
		 * 				and coordinate frame conversions.
		 * @param[in]	point point to construct from
		 * @returns		copy of *this
		 */
		template<class Point, class = typename std::enable_if<coord::traits::is_point<Point>::value && !std::is_convertible<Point, PositionECEF>::value>::type>
		PositionECEF& operator=(const Point& point)
		{
			m_frameData = point.frameData();
			coord::convert(point, *this);
			return *this;
		}

		virtual ~PositionECEF() {}

		// friend other specializations of the template.
		template<class D, class Ty, class DU>
		friend class PositionECEF;

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
		 * @brief		set X value
		 * @details		overwrites the x-value with the value provided, performing a unit conversion
		 *				if necessary.
		 * @param[in]	x new value of <i>x</i>
		 */
		void setX(unit_type x)
		{
			m_x = x;
		}

		/**
		 * @brief		set Y value
		 * @details		overwrites the y-value with the value provided, performing a unit conversion
		 *				if necessary.
		 * @param[in]	y new value of <i>x</i>
		 */
		void setY(unit_type y)
		{
			m_y = y;
		}

		/**
		* @brief		set Z value
		* @details		overwrites the z-value with the value provided, performing a unit conversion
		*				if necessary.
		* @param[in]	z new value of <i>z</i>
		*/
		void setZ(unit_type z)
		{
			m_z = z;
		}

		/**
		 * @brief		x-value
		 * @details		returns the x-value of the point, performing a unit-conversion to <i>UnitsTo</i> if
		 *				necessary.
		 * @returns		x-value of the Point.
		 */
		unit_type x() const
		{
			// remember, convert automatically uses the most efficient algorithm, including to/fromBase
			// and directly returning values if they are the same.
			return m_x;
		}

		/**
		 * @brief		y-value
		 * @details		returns the y-value of the point, performing a unit-conversion to <i>UnitsTo</i> if
		 *				necessary.
		 * @returns		y-value of the Point.
		 */
		unit_type y() const
		{
			return m_y;
		}

		/**
		 * @brief		z-value
		 * @details		returns the z-value of the point, performing a unit-conversion to <i>UnitsTo</i> if
		 *				necessary.
		 * @returns		z-value of the Point.
		 */
		unit_type z() const
		{
			return m_z;
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
		PositionECEF& operator*=(units::dimensionless::scalar_t factor) 
		{
			m_x = m_x * factor;
			m_y = m_y * factor;
			m_z = m_z * factor;
			return *this;
		}

		/**
		 * @brief 		addition operator
		 * @details		adds the given point to this point and returns a reference to this point.
		 * @sa			operator-=
		 * @param[in] 	p	point to add to this point.
		 * @return 		reference to this point.
		 */
		PositionECEF& operator+=(const PositionECEF& p)
		{
			m_x = m_x + p.x();
			m_y = m_y + p.y();
			m_z = m_z + p.z();
			return *this;
		}

		/**
		 * @brief 		subtraction operator
		 * @details		subtracts the given point from this point and returns a reference to this point.
		 * @sa			operator+=
		 * @param[in]	p	point to subtract from this point.
		 * @return 		reference to this point.
		 */
		PositionECEF& operator-=(const PositionECEF& p)
		{
			m_x = m_x - p.x();
			m_y = m_y - p.y();
			m_z = m_z - p.z();
			return *this;
		}

		/**
		 * @brief 		division operator
		 * @details		divides each coordinate of the point by <i>divisor</i> and returns a reference
		 * 				to this point.
		 * @param[in]	divisor divisor for the division operation
		 * @return 		reference to this point.
		 */
		PositionECEF& operator/=(units::dimensionless::scalar_t divisor)
		{
			m_x = m_x / divisor;
			m_y = m_y / divisor;
			m_z = m_z / divisor;
			return *this;
		}

		/**
		 * @brief		print value
		 * @details		Prints the point value in the form "(x, y, z)"
		 * @param[in]	os	output stream 
		 * @param[in]	p	point to print
		 * @returns		reference to the output stream.
		 */
		friend std::ostream& operator<<(std::ostream& os, const PositionECEF& p)
		{
			return os << "(" << p.m_x << ", " << p.m_y << ", " << p.m_z << ")";
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
			return tuple_type(m_x, m_y, m_z);
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
			m_x = std::get<0>(point);
			m_y = std::get<1>(point);
			m_z = std::get<2>(point);
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
			m_x = std::move(std::get<0>(point));
			m_y = std::move(std::get<1>(point));
			m_z = std::move(std::get<2>(point));
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

		unit_type	m_x;
		unit_type	m_y;
		unit_type	m_z;

		frame_data_type	m_frameData;
	};

}

#endif // pointECEF_h__