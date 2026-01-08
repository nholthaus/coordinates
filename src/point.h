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

#ifndef point_h__
#define point_h__

//------------------------
//	INCLUDES
//------------------------
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <type_traits>
#include <initializer_list>
#include <tuple>

#include "units.h"
#include "frameOfReference.h"
#include "coordinate_traits.h"

namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		Point
	//  ----------------------------------------------------------------------------
	///	@brief		Convenience class for implementing the `point` concept
	///	@details	
	//  ----------------------------------------------------------------------------
	template<typename ReferenceFrame, typename TupleType, typename FrameDataType>
	class Point
	{
	public:

		using reference_frame = ReferenceFrame;
		using tuple_type = TupleType;
		using frame_data_type = FrameDataType;

		virtual tuple_type point() const = 0;
		virtual frame_data_type frameData() const = 0;

		virtual void setPoint(const tuple_type& point) = 0;
		virtual void setPoint(tuple_type&& point) = 0;
		virtual void setFrameData(const frame_data_type& frameData) = 0;
		virtual void setFrameData(frame_data_type&& frameData) = 0;

	};

	//----------------------------------
	//	POINT TRAITS CLASS
	//----------------------------------
	namespace traits
	{
		/**
		 * @brief		Traits class defining properties of Units.
		 * @details
		 */
		namespace detail
		{
			template<class Point, typename = void>
			struct point_traits
			{
				typedef void reference_frame;
				typedef void tuple_type;
				typedef void frame_data_type;
			};

			template<class Point>
			struct point_traits<Point, typename void_type<
				typename Point::reference_frame,
				typename Point::tuple_type,
				typename Point::frame_data_type>::type>
			{
				typedef typename Point::reference_frame	reference_frame;
				typedef typename Point::tuple_type		tuple_type;
				typedef typename Point::frame_data_type	frame_data_type;
			};
		}

		template<class Point>
		struct point_traits : detail::point_traits<Point> {};

		//----------------------------------
		//	POINT CONCEPT CHECKERS
		//----------------------------------

		/**
		 * @brief		Tests that a class has an `reference_frame` typedef that represents a frame of reference.
		 */
		template <typename T, template<class> class Traits = point_traits>
		struct has_reference_frame : coordinates::traits::is_frame_of_reference<typename Traits<T>::reference_frame>::type {};

		/**
		 * @brief		Trait which tests that a class has a `frame_data_type` typedef, and that it refers to a
		 *				type which is inherited from `coordinates::frameData`.
		 */
		template <typename T, template<class> class Traits = point_traits>
		struct has_frame_data_type : std::is_base_of<coordinates::FrameData, typename Traits<T>::frame_data_type>::type {};

		namespace detail
		{
			/// implementation of the has_tuple concept checker.
			template <typename T>
			struct has_point_impl
			{
				template<typename U>
				static auto test(const U* const p) -> decltype(p->point());
				template<typename U>
				static auto test(...)->std::false_type;

				using type = typename std::is_same<decltype(test<T>(0)), typename point_traits<T>::tuple_type>::type;
			};
		}

		/**
		 * @brief		Trait which tests that a class has a `point()` member function to retrieve its
		 *				position data as a 3-tuple of units.
		 * @details		The requirements are:
		 * 				- takes no arguments
		 * 				- const
		 * 				- is non-static
		 * 				- returns a tuple of the same type as the classes `tuple_type`.
		 */
		template <typename T>
		struct has_point : detail::has_point_impl<T>::type {};

		namespace detail
		{
			/// implementation of the has_set_tuple concept checker.
			template <typename T>
			struct has_setPoint_impl
			{
				template<typename U>
				static auto test(U* p) -> decltype(p->setPoint(typename point_traits<T>::tuple_type()));
				template<typename U>
				static auto test(...)->std::false_type;

				using type = typename std::is_void<decltype(test<T>(0))>::type;
			};
		}

		/**
		 * @brief		Tests that a class has a `setPoint(const tuple_type& t)` member function which takes a tuple argument.
		 * @details		The requirements are:
		 * 				- takes a const tuple_type& argument
		 * 				- returns void
		 * 				- is non-static
		 * 				- has template parameters for the input units of distance, angle, and time, respectively.
		 */
		template <typename T>
		struct has_setPoint : detail::has_setPoint_impl<T>::type {};

		namespace detail
		{
			template <typename T>
			struct has_frameData_impl
			{
				template<typename U>
				static auto test(const U* const p) -> decltype(p->frameData());
				template<typename U>
				static auto test(...)->std::false_type;

				using type = typename std::is_same<decltype(test<T>(0)), typename point_traits<T>::frame_data_type>::type;
			};
		}

		/**
		* @brief		Trait which tests that a class has a `frameData()` member function to retrieve its
		*				position data as a 3-tuple of units.
		* @details		The requirements are:
		* 				- takes no arguments
		* 				- const
		* 				- is non-static
		* 				- returns a class of the same type as the classes `frame_data_type`.
		*/
		template <typename T>
		struct has_frameData : detail::has_frameData_impl<T>::type {};

		namespace detail
		{
			template <typename T>
			struct has_setFrameData_impl
			{
				template<typename U>
				static auto test(U* p) -> decltype(p->setFrameData(typename point_traits<U>::frame_data_type()));
				template<typename U>
				static auto test(...)->std::false_type;

				using type = typename std::is_void<decltype(test<T>(0))>::type;
			};
		}

		/**
		* @brief		Tests that a class has a `setFrameData(const frame_data_type& t)` member function.
		* @details		The requirements are:
		* 				- takes a const frame_data_type& argument
		* 				- returns void
		* 				- is non-static
		*/
		template <typename T>
		struct has_setFrameData : detail::has_setFrameData_impl<T>::type {};

		/**
		 * @brief		Trait which tests whether a class satisfies the `point` concept.
		 * @details		To satisfy the `point` concept, a class must:
		 *				- be default constructible.
		 *				- have a `reference_frame` typedef (which itself must satisfy the `frame_of_reference` concept).
		 *				- have a `tuple_type` typedef.
		 *				- have a `frame_data_type` typedef (which must represent a `frameData` class, or a class inherited from it).
		 *				- have a `tuple_type point() const` member function.
		 *				- have a `void setPoint(const tuple_type&)` member function.
		 *				- have a `frame_data_type frameData() const` member function.
		 *				- have a `void setFrameData(const frame_data_type&)` member function.

		 */
		template <typename T>
		struct is_point : std::integral_constant<bool,
			std::is_default_constructible<T>::value &&
			coordinates::traits::has_reference_frame<T>::value &&
			coordinates::traits::has_tuple_type<T, point_traits>::value &&
			coordinates::traits::has_frame_data_type<T>::value &&
			coordinates::traits::has_point<T>::value &&
			coordinates::traits::has_setPoint<T>::value &&
			coordinates::traits::has_frameData<T>::value &&
			coordinates::traits::has_setFrameData<T>::value>
		{};

		/**
		 * @brief		Trait which tests whether two point are convertible to each other
		 * @details		To be convertible, both types must satisfy the `point` concept, and both of
		 *				their frames of reference must be convertible.
		 */
		template<typename P1, typename P2>
		struct is_convertible_point : std::integral_constant<bool,
			coordinates::traits::is_point<P1>::value &&
			coordinates::traits::is_point<P2>::value &&
			coordinates::traits::is_convertible_frame<typename coordinates::traits::point_traits<P1>::reference_frame, typename coordinates::traits::point_traits<P2>::reference_frame>::value>
		{};

		/**
		 * @brief		Trait which tests whether a point has a Cartesian frame of reference.
		 */
		template <typename T>
		struct is_cartesian_point : std::integral_constant<bool,
			coordinates::traits::is_point<T>::value &&
			coordinates::traits::is_cartesian_frame<typename coordinates::traits::point_traits<T>::reference_frame>::value>
		{};
	}

	//----------------------------------
	//	CONVERSION FUNCTIONS
	//----------------------------------

	template <class PointFrom, class PointTo,
	class enable_if_are_points = typename std::enable_if<coordinates::traits::is_point<PointTo>::value && coordinates::traits::is_point<PointFrom>::value>::type>
		void convert(const PointFrom& pointFrom, PointTo& pointTo)
	{
		using FrameFrom = typename coordinates::traits::point_traits<PointFrom>::reference_frame;
		using FrameTo = typename coordinates::traits::point_traits<PointTo>::reference_frame;

		static_assert(coordinates::traits::is_convertible_frame<FrameFrom, FrameTo>::value, "No known conversions between `FrameFrom` and `FrameTo`. It may be possible to diagnose this error using the coordinates::traits::least_common_ancestor trait.");

		pointTo.setPoint(coordinates::convert<FrameFrom, FrameTo>(pointFrom.point(), pointFrom.frameData(), pointTo.frameData()));
	}
}

//----------------------------------
//	POINT ARITHMETIC
//----------------------------------

template<class PointLhs, class PointRhs, typename = typename std::enable_if<coordinates::traits::is_point<PointLhs>::value && coordinates::traits::is_point<PointRhs>::value>::type>
bool operator==(const PointLhs& lhs, const PointRhs& rhs)
{
	static_assert(coordinates::traits::is_convertible_point<PointLhs, PointRhs>::value, "No known conversion between types `PointLhs` and `PointRhs`.");

	using namespace units::math;

	// convert both points to the nearest frame of reference which is Cartesian.
	coordinates::cartesianTuple l;
	coordinates::cartesianTuple r;

	// if the frame data is the same, use the nearest cartesian reference
	if (lhs.frameData() == rhs.frameData())
	{
		using LCA = typename coordinates::traits::least_common_cartesian_ancestor<typename coordinates::traits::point_traits<PointLhs>::reference_frame, typename coordinates::traits::point_traits<PointRhs>::reference_frame>::type;

		l = coordinates::convert<typename coordinates::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = coordinates::convert<typename coordinates::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}
	// otherwise, use the base reference to make sure all necessary conversions are performed.
	else
	{
		using LCA = typename coordinates::traits::lowest_base_frame<typename coordinates::traits::point_traits<PointLhs>::reference_frame>::type;

		l = coordinates::convert<typename coordinates::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = coordinates::convert<typename coordinates::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}

	auto x0 = std::get<0>(l);
	auto y0 = std::get<1>(l);
	auto z0 = std::get<2>(l);
	auto x1 = std::get<0>(r);
	auto y1 = std::get<1>(r);
	auto z1 = std::get<2>(r);

	using namespace units::math;
	return (x0 == x1 && y0 == y1 && z0 == z1);
}

template<class PointLhs, class PointRhs, class = typename std::enable_if<coordinates::traits::is_point<PointLhs>::value && coordinates::traits::is_point<PointRhs>::value>::type>
bool operator!=(const PointLhs& lhs, const PointRhs& rhs)
{
	using namespace units::math;

	// convert both points to the nearest frame of reference which is Cartesian.
	coordinates::cartesianTuple l;
	coordinates::cartesianTuple r;

	// if the frame data is the same, use the nearest cartesian reference
	if (lhs.frameData() == rhs.frameData())
	{
		using LCA = typename coordinates::traits::least_common_cartesian_ancestor<typename coordinates::traits::point_traits<PointLhs>::reference_frame, typename coordinates::traits::point_traits<PointRhs>::reference_frame>::type;

		l = coordinates::convert<typename coordinates::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = coordinates::convert<typename coordinates::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}
	// otherwise, use the base reference to make sure all necessary conversions are performed.
	else
	{
		using LCA = typename coordinates::traits::lowest_base_frame<typename coordinates::traits::point_traits<PointLhs>::reference_frame>::type;

		l = coordinates::convert<typename coordinates::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = coordinates::convert<typename coordinates::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}

	return !(l == r);
}

template<class Point, class = typename std::enable_if<coordinates::traits::is_cartesian_point<Point>::value>::type>
Point operator*(const Point& p, units::dimensionless::scalar_t factor)
{
	auto x = std::get<0>(p.point());
	auto y = std::get<1>(p.point());
	auto z = std::get<2>(p.point());

	Point ret;
	ret.setPoint(x * factor, y * factor, z * factor);
	ret.setFrameData(p.frameData());
	return ret;
}

template<class Point, class = typename std::enable_if<coordinates::traits::is_cartesian_point<Point>::value>::type>
Point operator*(units::dimensionless::scalar_t factor, const Point& p)
{
	auto x = std::get<0>(p.point());
	auto y = std::get<1>(p.point());
	auto z = std::get<2>(p.point());

	Point ret;
	ret.setPoint(x * factor, y * factor, z * factor);
	ret.setFrameData(p.frameData());
	return ret;
}

template<class PointLhs, class PointRhs, class = typename std::enable_if<coordinates::traits::is_cartesian_point<PointLhs>::value && coordinates::traits::is_cartesian_point<PointRhs>::value>::type>
PointLhs operator+(const PointLhs& lhs, const PointRhs& rhs)
{
	static_assert(coordinates::traits::is_convertible_point<PointLhs, PointRhs>::value, "No known conversion between types `PointLhs` and `PointRhs`.");

	using namespace units::math;

	// convert both points to the nearest frame of reference which is Cartesian.
	coordinates::cartesianTuple l;
	coordinates::cartesianTuple r;

	// if the frame data is the same, use the nearest cartesian reference
	if (lhs.frameData() == rhs.frameData())
	{
		using LCA = typename coordinates::traits::least_common_cartesian_ancestor<typename coordinates::traits::point_traits<PointLhs>::reference_frame, typename coordinates::traits::point_traits<PointRhs>::reference_frame>::type;

		l = coordinates::convert<typename coordinates::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = coordinates::convert<typename coordinates::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}
	// otherwise, convert to the lhs reference frame/origin
	else
	{
		using Base = typename coordinates::traits::lowest_base_frame<typename coordinates::traits::point_traits<PointRhs>::reference_frame>::type;
		
		l = lhs.point();
		auto intermediate = coordinates::convert<typename coordinates::traits::point_traits<PointRhs>::reference_frame, Base>(rhs.point(), rhs.frameData(), rhs.frameData());
		r = coordinates::convert<Base, typename coordinates::traits::point_traits<PointLhs>::reference_frame>(intermediate, rhs.frameData(), lhs.frameData());
	}

	auto x0 = std::get<0>(l);
	auto y0 = std::get<1>(l);
	auto z0 = std::get<2>(l);
	auto x1 = std::get<0>(r);
	auto y1 = std::get<1>(r);
	auto z1 = std::get<2>(r);

	PointLhs ret;
	ret.setPoint(x0 + x1, y0 + y1, z0 + z1);
	ret.setFrameData(lhs.frameData());
	return ret;
}

template<class Point, class = typename std::enable_if<coordinates::traits::is_cartesian_point<Point>::value>::type>
Point operator+(const Point& p)
{
	return p;
}

template<class PointLhs, class PointRhs, class = typename std::enable_if<coordinates::traits::is_cartesian_point<PointLhs>::value && coordinates::traits::is_cartesian_point<PointRhs>::value>::type>
PointLhs operator-(const PointLhs& lhs, const PointRhs& rhs)
{
	static_assert(coordinates::traits::is_convertible_point<PointLhs, PointRhs>::value, "No known conversion between types `PointLhs` and `PointRhs`.");

	using namespace units::math;

	// convert both points to the nearest frame of reference which is Cartesian.
	coordinates::cartesianTuple l;
	coordinates::cartesianTuple r;

	// if the frame data is the same, use the nearest cartesian reference
	if (lhs.frameData() == rhs.frameData())
	{
		using LCA = typename coordinates::traits::least_common_cartesian_ancestor<typename coordinates::traits::point_traits<PointLhs>::reference_frame, typename coordinates::traits::point_traits<PointRhs>::reference_frame>::type;

		l = coordinates::convert<typename coordinates::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = coordinates::convert<typename coordinates::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}
	// otherwise, convert to the lhs reference frame/origin
	else
	{
		using Base = typename coordinates::traits::lowest_base_frame<typename coordinates::traits::point_traits<PointRhs>::reference_frame>::type;

		l = lhs.point();
		auto intermediate = coordinates::convert<typename coordinates::traits::point_traits<PointRhs>::reference_frame, Base>(rhs.point(), rhs.frameData(), rhs.frameData());
		r = coordinates::convert<Base, typename coordinates::traits::point_traits<PointLhs>::reference_frame>(intermediate, rhs.frameData(), lhs.frameData());
	}

	auto x0 = std::get<0>(l);
	auto y0 = std::get<1>(l);
	auto z0 = std::get<2>(l);
	auto x1 = std::get<0>(r);
	auto y1 = std::get<1>(r);
	auto z1 = std::get<2>(r);

	PointLhs ret;
	ret.setPoint(x0 - x1, y0 - y1, z0 - z1);
	ret.setFrameData(lhs.frameData());
	return ret;
}

template<class Point, class = typename std::enable_if<coordinates::traits::is_cartesian_point<Point>::value>::type>
Point operator-(const Point& p)
{
	auto x = std::get<0>(p.point());
	auto y = std::get<1>(p.point());
	auto z = std::get<2>(p.point());

	Point ret;
	ret.setPoint(-x, -y, -z);
	ret.setFrameData(p.frameData());
	return ret;
}

template<class Point, class = typename std::enable_if<coordinates::traits::is_cartesian_point<Point>::value>::type>
Point operator/(const Point& p, units::dimensionless::scalar_t divisor)
{
	auto x = std::get<0>(p.point());
	auto y = std::get<1>(p.point());
	auto z = std::get<2>(p.point());

	Point ret;
	ret.setPoint(x / divisor, y / divisor, z / divisor);
	ret.setFrameData(p.frameData());
	return ret;
}

#endif // point_h__