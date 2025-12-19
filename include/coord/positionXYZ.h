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

#ifndef pointXYZ_h__
#define pointXYZ_h__

//------------------------
//	INCLUDES
//------------------------
#include "point.h"

namespace coord
{
//	----------------------------------------------------------------------------
//	CLASS		PointXYZ
//  ----------------------------------------------------------------------------
///	@brief		represents a point in Cartesian space.
///	@details	The origin of an XYZ point is arbitrarily (0,0,0). It is up to
///				the user to ensure that the origin of two XYZ points represents
///				the same thing. XYZ points are unit-less.
//  ----------------------------------------------------------------------------
// 	template<typename T>
// 	class PointXYZ : public Point<T>
// 	{
// 	public:
//
// 		//////////////////////////////////////////////////////////////////////////
// 		//		CONSTRUCTORS
// 		//////////////////////////////////////////////////////////////////////////
//
// 		/**
// 		* @brief		default constructor
// 		* @details		Creates a point object with the value (0,0,0).
// 		*/
// 		PointXYZ()
// 			:
// 			m_x(0),
// 			m_y(0),
// 			m_z(0)
// 		{
//
// 		}
//
// 		/**
// 		* @brief		constructor
// 		* @details		constructs a new Point object
// 		* @param[in]	x	x-value of the point.
// 		* @param[in]	y	y-value of the point.
// 		* @param[in]	z	z-value of the point.
// 		*/
// 		PointXYZ(T x, T y, T z)
// 			:
// 			m_x(x),
// 			m_y(y),
// 			m_z(z)
// 		{
//
// 		}
//
// 		/**
// 		* @brief		constructs a Point object from an initializer_list
// 		* @details		the initializer_list must have exactly 3 elements.
// 		* @param[in]	il	initializer_list containing the x,y,z value of the point.
// 		*/
// 		PointXYZ(std::initializer_list<T> il)
// 		{
// 			if (il.size() != 3)
// 				throw std::out_of_range("Initializer lists for the Point class must contain exactly 3 elements.");
//
// 			m_x = il.begin()[0];
// 			m_y = il.begin()[1];
// 			m_z = il.begin()[2];
// 		}
//
// 		virtual ~PointXYZ() {}
//
// 		//////////////////////////////////////////////////////////////////////////
// 		//		STATIC PUBLIC MEMBERS
// 		//////////////////////////////////////////////////////////////////////////
//
// 		/**
// 		* @brief		calculate dot product
// 		* @details		calculates the dot product of two points
// 		* @param[in]	p1	left-hand side Point.
// 		* @param[in]	p2	right-hand side Point.
// 		* @returns		dot product of p1 and p2.
// 		*/
// 		static T dotProduct(const PointXYZ& p1, const PointXYZ& p2) { return (p1.m_x*p2.m_x + p1.m_y*p2.m_y + p1.m_z*p2.m_z); }
//
// 		//////////////////////////////////////////////////////////////////////////
// 		//		PUBLIC MEMBERS
// 		//////////////////////////////////////////////////////////////////////////
//
// 		/**
// 		* @brief		returns true if x,y, and z are all 0, otherwise returns false.
// 		*/
// 		virtual bool isNull() const { return (m_x == m_y && m_y == m_z && m_z == 0); }
//
// 		/**
// 		* @brief		compares two points
// 		* @details		This function compares two points for equality, using the given tolerance.
// 		* 				This is primarily intended for floating point types. For a tolerance of 0,
// 		* 				operator== is more efficient.
// 		* @sa			operator==
// 		* @param[in]	p			point to compare to
// 		* @param[in]	tolerance	acceptable difference between the two points which will still
// 		* 							compare equal.
// 		* @returns		true if the values are equal within the tolerance, false otherwise.
// 		*/
// 		virtual bool isSame(const PointXYZ& p, T tolerance = 0) const
// 		{
// 			return ((std::abs(m_x - p.m_x) <= tolerance) && (std::abs(m_y - p.m_y) <= tolerance) && (std::abs(m_z - p.m_z) <= tolerance));
// 		}
//
// 		/**
// 		* @brief		compares two points
// 		* @details		This function compares two points for equality, using the tolerances in
// 		*				<i>tolerance</i> for x, y, and z individually.
// 		* 				This is primarily intended for floating point types. For a tolerance of 0,
// 		* 				operator== is more efficient.
// 		* @sa			operator==
// 		* @param[in]	p
// 		* @param[in]	tolerance
// 		* @returns		bool
// 		*/
// 		virtual bool isSame(const PointXYZ& p, const PointXYZ& tolerance) const
// 		{
// 			return ((std::abs(m_x - p.m_x) <= tolerance.m_x) && (std::abs(m_y - p.m_y) <= tolerance.m_y) && (std::abs(m_z - p.m_z) <= tolerance.m_z));
// 		}
//
// 		/**
// 		* @brief		calculates the distance between two points.
// 		* @details		calculates the euclidean distance between this point and point <i>p</i>
// 		* @param[in]	p	point to calculate distance to.
// 		* @returns		distance between this Point and p.
// 		*/
// 		virtual T distance(const PointXYZ& p) const { return sqrt((p.m_x - m_x)*(p.m_x - m_x) + (p.m_y - m_y)*(p.m_y - m_y) + (p.m_z - m_z)*(p.m_z - m_z)); }
//
// 		/**
// 		* @brief		calculate dot-product
// 		* @details		calculates the dot-product of this point with the Point <i>p</i>
// 		* @param[in]	p point to calculate dot-product with.
// 		* @returns		dot product of this point and p.
// 		*/
// 		virtual T dotProduct(const PointXYZ& p) const { return (m_x*p.m_x + m_y*p.m_y + m_z*p.m_z); }
//
// 		/**
// 		* @brief		returns origin of the frame of reference of the point.
// 		* @details		For a Point, this is always (0,0,0).
// 		* @returns		frame-of-reference origin.
// 		*/
// 		virtual const PointXYZ<T> origin() { return PointXYZ<T>(0, 0, 0); }
//
// 		//////////////////////////////////////////////////////////////////////////
// 		//		ACCESSORS
// 		//////////////////////////////////////////////////////////////////////////
//
// 		/**
// 		* @brief		set X value
// 		* @details		overwrites the x-value with the value provided.
// 		* @param[in]	x new value of <i>x</i>
// 		*/
// 		virtual void setX(T x) { m_x = x; }
//
// 		/**
// 		* @brief		set Y value
// 		* @details		overwrites the y-value with the value provided.
// 		* @param[in]	y new value of <i>y</i>
// 		*/
// 		virtual void setY(T y) { m_y = y; }
//
// 		/**
// 		* @brief		set Z value
// 		* @details		overwrites the z-value with the value provided.
// 		* @param[in]	z new value of <i>z</i>
// 		*/
// 		virtual void setZ(T z) { m_z = z; }
//
// 		/**
// 		* @brief		x-value
// 		* @returns		x-value of the Point.
// 		*/
// 		virtual T x() const { return m_x; }
//
// 		/**
// 		* @brief		y-value
// 		* @returns		y-value of the Point.
// 		*/
// 		virtual T y() const { return m_y; }
//
// 		/**
// 		* @brief		z-value
// 		* @returns		z-value of the Point.
// 		*/
// 		virtual T z() const { return m_z; }
//
// 		/**
// 		* @brief		reference to x
// 		* @details		Allows for direct manipulation of the 'x' value, e.g. p.rx()++;
// 		* @returns		mutable reference to the x-value of the Point.
// 		*/
// 		virtual T& rx() { return m_x; }
//
// 		/**
// 		* @brief		reference to y
// 		* @details		Allows for direct manipulation of the 'y' value, e.g. p.ry()--;
// 		* @returns		mutable reference to the y-value of the Point.
// 		*/
// 		virtual T& ry() { return m_y; }
//
// 		/**
// 		* @brief		reference to z
// 		* @details		Allows for direct manipulation of the 'z' value, e.g. p.rz()++;
// 		* @returns		mutable reference to the z-value of the Point.
// 		*/
// 		virtual T& rz() { return m_z; }
//
// 		//////////////////////////////////////////////////////////////////////////
// 		//		OPERATORS
// 		//////////////////////////////////////////////////////////////////////////
//
// 		/**
// 		* @brief		multiplication operator
// 		* @details		multiplies this point's coordinates by the given <i>factor</i>, and returns
// 		* 				a reference to this point.
// 		* @sa			operator/=
// 		* @param[in]	factor multiplication factor
// 		* @returns		reference to this point.
// 		*/
// 		template<typename Ty, class = typename std::enable_if<std::is_arithmetic<Ty>::value>::type>
// 		PointXYZ& operator*=(Ty factor) { m_x *= factor; m_y *= factor; m_z *= factor; return *this; }
//
// 		/**
// 		* @brief 		addition operator
// 		* @details		adds the given point to this point and returns a reference to this point.
// 		* @sa			operator-=
// 		* @param[in] 	p	point to add to this point.
// 		* @return 		reference to this point.
// 		*/
// 		PointXYZ& operator+=(const PointXYZ& p) { m_x += p.m_x; m_y += p.m_y; m_z += p.m_z; return *this; }
//
// 		/**
// 		* @brief 		subtraction operator
// 		* @details		subtracts the given point from this point and returns a reference to this point.
// 		* @sa			operator+=
// 		* @param[in]	p	point to subtract from this point.
// 		* @return 		reference to this point.
// 		*/
// 		PointXYZ& operator-=(const PointXYZ& p) { m_x -= p.m_x; m_y -= p.m_y; m_z -= p.m_z; return *this; }
//
// 		/**
// 		* @brief 		division operator
// 		* @details		divides each coordinate of the point by <i>divisor</i> and returns a reference
// 		* 				to this point.
// 		* @param[in]	divisor divisor for the division operation
// 		* @return 		reference to this point.
// 		*/
// 		template<typename Ty, class = typename std::enable_if<std::is_arithmetic<Ty>::value>::type>
// 		PointXYZ& operator/=(Ty divisor) { m_x /= divisor; m_y /= divisor; m_z /= divisor; return *this; }
//
// 		/**
// 		* @brief		equality operator
// 		* @details		compares <i>p1</i> and <i>p2</i> for equality.
// 		* @sa			isSame()
// 		* @param[in]	p1	left-hand side point.
// 		* @param[in]	p2	right-hand side point.
// 		* @returns		true if all coordinates are an exact match, false otherwise.
// 		*/
// 		template<typename Ty>
// 		friend bool operator==(const PointXYZ<Ty>& p1, const PointXYZ<Ty>& p2);
//
// 		/**
// 		* @brief		inequality operator
// 		* @details		compares <i>p1</i> and <i>p2</i> for inequality.
// 		* @sa			isSame()
// 		* @param[in]	p1	left-hand side point.
// 		* @param[in]	p2	right-hand side point.
// 		* @returns		false if all coordinates are an exact match, true otherwise.
// 		*/
// 		template<typename Ty>
// 		friend bool operator!=(const PointXYZ<Ty>& p1, const PointXYZ<Ty>& p2);
//
// 		/**
// 		* @brief 		output stream operator
// 		* @details		prints the point to an output stream in the form "(x,y,z)"
// 		* @param[in] 	os 	stream to print to.
// 		* @param[in] 	p 	point to print.
// 		* @returns		reference to the stream object.
// 		*/
// 		template<typename Ty>
// 		friend std::ostream& operator<<(std::ostream& os, const PointXYZ<Ty>& p);
//
// 		/**
// 		* @brief 		multiplication operator
// 		* @details		multiplies each coordinate of point <i>p</i> by <i>factor</i>
// 		* @param[in] 	p point to multiply by <i>factor</i>
// 		* @param[in] 	factor factor to multiply <i>p</i> by.
// 		* @return 		a copy of the given point multiplied by <i>factor</i>
// 		*/
// 		template<typename Ty>
// 		friend const PointXYZ<Ty> operator*(const PointXYZ<Ty>& p, T factor);
//
// 		/**
// 		* @brief 		multiplication operator
// 		* @details		multiplies each coordinate of point <i>p</i> by <i>factor</i>
// 		* @param		factor factor to multiply <i>p</i> by.
// 		* @param		p point to multiply by <i>factor</i>
// 		* @return 		a copy of the given point multiplied by <i>factor</i>
// 		*/
// 		template<typename Ty>
// 		friend const PointXYZ<Ty> operator*(T factor, const PointXYZ<Ty>& p);
//
// 		/**
// 		* @brief 		addition operator
// 		* @details		adds to points together
// 		* @param[in]	p1 left-hand side point
// 		* @param[in]	p2 right-hand side point
// 		* @returns		a copy of the point created by the addition of <i>p1</i> and <i>p2</i>
// 		*/
// 		template<typename Ty>
// 		friend const PointXYZ<Ty> operator+(const PointXYZ<Ty>& p1, const PointXYZ<Ty>& p2);
//
// 		/**
// 		* @brief 		addition operator
// 		* @param p 	point
// 		* @returns 	<i>p</i> copy of p, unchanged.
// 		*/
// 		template<typename Ty>
// 		friend const PointXYZ<Ty> operator+(const PointXYZ<Ty>& p);
//
// 		/**
// 		* @brief 		subtraction operator
// 		* @details		subtracts two points from each other
// 		* @param[in]	p1 left-hand side point.
// 		* @param[in]	p2 right-hand side point.
// 		* @returns 	a copy of <i>p1</i> - <i>p2</i>
// 		*/
// 		template<typename Ty>
// 		friend const PointXYZ<Ty> operator-(const PointXYZ<Ty>& p1, const PointXYZ<Ty>& p2);
//
// 		/**
// 		* @brief 		negative operator
// 		* @details		inverts the sign of each coordinate of the given point. Equivalent to
// 		* 				Point(0,0,0) - p.
// 		* @param[in]	p point to change sign of.
// 		* @return 		copy of the point that is formed by changing the sign of all the components
// 		* 				of <i>p</i>
// 		*/
// 		template<typename Ty>
// 		friend const PointXYZ<Ty> operator-(const PointXYZ<Ty>& p);
//
// 		/**
// 		* @brief 		division operator
// 		* @details		divides each coordinate of <i>p</i> by <i>divisor</i>
// 		* @param[in]	p point to divide.
// 		* @param[in]	divisor divisor to use.
// 		* @returns 	copy of the point created by dividing <i>p</i> by <i>divisor</i>
// 		*/
// 		template<typename Ty>
// 		friend const PointXYZ<Ty> operator/(const PointXYZ<Ty>& p, T divisor);
//
// 	protected:
//
// 		T	m_x;					///< 'x' value of the point.
// 		T	m_y;					///< 'y' value of the point.
// 		T	m_z;					///< 'z' value of the point.
//
// 	};
//
// 	//////////////////////////////////////////////////////////////////////////
// 	//		FRIEND FUNCTION DEFINITIONS
// 	//////////////////////////////////////////////////////////////////////////
//
// 	template <typename T>
// 	bool operator==(const PointXYZ<T>& p1, const PointXYZ<T>& p2)
// 	{
// 		return (p1.m_x == p2.m_x && p1.m_y == p2.m_y && p1.m_z == p2.m_z);
// 	}
//
// 	template <typename T>
// 	bool operator!=(const PointXYZ<T>& p1, const PointXYZ<T>& p2)
// 	{
// 		return !(p1 == p2);
// 	}
//
// 	template <typename T>
// 	std::ostream& operator<<(std::ostream& os, const PointXYZ<T>& p)
// 	{
// 		os << '(' << p.m_x << ',' << p.m_y << ',' << p.m_z << ')';
// 		return os;
// 	}
//
// 	template <typename T>
// 	const PointXYZ<T> operator*(const PointXYZ<T>& p, T factor)
// 	{
// 		return PointXYZ<T>(p.m_x*factor, p.m_y*factor, p.m_z*factor);
// 	}
//
// 	template <typename T>
// 	const PointXYZ<T> operator*(T factor, const PointXYZ<T>& p)
// 	{
// 		return (p*factor);
// 	}
//
// 	template <typename T>
// 	const PointXYZ<T> operator+(const PointXYZ<T>& p1, const PointXYZ<T>& p2)
// 	{
// 		return PointXYZ<T>(p1.m_x + p2.m_x, p1.m_y + p2.m_y, p1.m_z + p2.m_z);
// 	}
//
// 	template <typename T>
// 	const PointXYZ<T> operator+(const PointXYZ<T>& p)
// 	{
// 		return PointXYZ<T>(p);
// 	}
//
// 	template <typename T>
// 	const PointXYZ<T> operator-(const PointXYZ<T>& p1, const PointXYZ<T>& p2)
// 	{
// 		return PointXYZ<T>(p1.m_x - p2.m_x, p1.m_y - p2.m_y, p1.m_z - p2.m_z);
// 	}
//
// 	template <typename T>
// 	const PointXYZ<T> operator-(const PointXYZ<T>& p)
// 	{
// 		return PointXYZ<T>(-p.m_x, -p.m_y, -p.m_z);
// 	}
//
// 	template <typename T>
// 	const PointXYZ<T> operator/(const PointXYZ<T>& p, T divisor)
// 	{
// 		return PointXYZ<T>(p.m_x / divisor, p.m_y / divisor, p.m_z / divisor);
// 	}
}


#endif // pointXYZ_h__