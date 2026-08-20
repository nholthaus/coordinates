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

#ifndef point_h
#define point_h

//------------------------
//	INCLUDES
//------------------------

#include <tuple>
#include <type_traits>

#include "frameOfReference.h"
#include "units.h"

inline namespace coordinates
{
	//----------------------------------
	//	POINT TRAITS CLASS
	//----------------------------------
	inline namespace traits
	{
		/**
		 * @brief		Traits class defining properties of Units.
		 * @details
		 */
		namespace detail
		{
			template<class, typename = void>
			struct point_traits
			{
				typedef void reference_frame;
				typedef void tuple_type;
				typedef void frame_data_type;
			};

			template<class Point>
			struct point_traits<Point, std::void_t<typename Point::reference_frame, typename Point::tuple_type, typename Point::frame_data_type>>
			{
				typedef Point::reference_frame reference_frame;
				typedef Point::tuple_type      tuple_type;
				typedef Point::frame_data_type frame_data_type;
			};
		}    // namespace detail

		template<class Point>
		struct point_traits : detail::point_traits<Point>
		{
		};

		//----------------------------------
		//	POINT CONCEPT CHECKERS
		//----------------------------------

		/**
		 * @brief		Tests that a class has an `reference_frame` typedef that represents a frame of reference.
		 */
		template<typename T, template<class> class Traits = point_traits>
		concept has_reference_frame = is_frame_of_reference<typename Traits<T>::reference_frame>;

		/**
		 * @brief		Trait which tests that a class has a `frame_data_type` typedef, and that it refers to a
		 *				type which is inherited from `frameData`.
		 */
		template<typename T, template<class> class Traits = point_traits>
		concept has_frame_data_type = std::is_base_of_v<FrameData, typename Traits<T>::frame_data_type>;

		/**
		 * @brief		Trait which tests that a class has a `point()` member function to retrieve its
		 *				position data as a 3-tuple of units.
		 * @details		The requirements are:
		 * 				- takes no arguments
		 * 				- const
		 * 				- is non-static
		 * 				- returns a tuple of the same type as the classes `tuple_type`.
		 */
		template<typename T>
		concept has_point = requires(T p) {
			{ p.point() } -> std::same_as<typename point_traits<T>::tuple_type>;
		};

		/**
		 * @brief		Tests that a class has a `setPoint(const tuple_type& t)` member function which takes a tuple argument.
		 * @details		The requirements are:
		 * 				- takes a const tuple_type& argument
		 * 				- returns void
		 * 				- is non-static
		 * 				- has template parameters for the input units of distance, angle, and time, respectively.
		 */
		template<typename T>
		concept has_setPoint = requires(T p) {
			{ p.setPoint(typename point_traits<T>::tuple_type{}) } -> std::same_as<void>;
		};

		/**
		 * @brief		Trait which tests that a class has a `frameData()` member function to retrieve its
		 *				position data as a 3-tuple of units.
		 * @details		The requirements are:
		 * 				- takes no arguments
		 * 				- const
		 * 				- is non-static
		 * 				- returns a class of the same type as the classes `frame_data_type`.
		 */
		template<typename T>
		concept has_frameData = requires(T p) {
			{ p.frameData() } -> std::same_as<typename point_traits<T>::frame_data_type>;
		};

		/**
		 * @brief		Tests that a class has a `setFrameData(const frame_data_type& t)` member function.
		 * @details		The requirements are:
		 * 				- takes a const frame_data_type& argument
		 * 				- returns void
		 * 				- is non-static
		 */
		template<typename T>
		concept has_setFrameData = requires(T p) {
			{ p.setFrameData(typename point_traits<T>::frame_data_type()) } -> std::same_as<void>;
		};

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
		template<typename T>
		concept is_coordinate = std::is_default_constructible_v<T> && has_reference_frame<T> && has_tuple_type<T, point_traits> && has_frame_data_type<T> &&
		                        has_point<T> && has_setPoint<T> && has_frameData<T> && has_setFrameData<T>;

		/// `is_point` is the structural coordinate contract. It carries no base-class / virtual requirement:
		/// any type providing the frame/tuple/frame-data typedefs and the point/frameData interface satisfies
		/// it. Retained as the established spelling; `is_coordinate` is the same contract under the evolved name.
		template<typename T>
		concept is_point = is_coordinate<T>;

		/**
		 * @brief		Trait which tests whether two point are convertible to each other
		 * @details		To be convertible, both types must satisfy the `point` concept, and both of
		 *				their frames of reference must be convertible.
		 */
		template<typename P1, typename P2>
		concept is_convertible_point =
		        is_point<P1> && is_point<P2> && is_convertible_frame<typename point_traits<P1>::reference_frame, typename point_traits<P2>::reference_frame>;

		/**
		 * @brief		Trait which tests whether a point has a Cartesian frame of reference.
		 */
		template<typename T>
		concept is_cartesian_point = is_point<T> && is_cartesian_frame<typename point_traits<T>::reference_frame>;

		//----------------------------------
		//	VECTOR TRAITS + CONCEPT CHECKERS
		//----------------------------------

		namespace detail
		{
			/**
			 * @brief		vector traits implementation for classes which are not vectors.
			 */
			template<class, typename = void>
			struct vector_traits
			{
				typedef void tuple_type;
				typedef void frame_data_type;
				typedef void reference_frame;
			};

			/**
			 * @brief		Traits class defining the properties of a vector.
			 * @details		A vector is distinct from a point: it represents a directed quantity and
			 *					may or may not be anchored to an origin depending on the reference frame.
			 */
			template<class V>
			struct vector_traits<V, std::void_t<typename V::reference_frame, typename V::tuple_type, typename V::frame_data_type, typename V::vector_tag>>
			{
				typedef typename V::reference_frame reference_frame;
				typedef typename V::tuple_type      tuple_type;
				typedef typename V::frame_data_type frame_data_type;
			};
		}    // namespace detail

		/**
		 * @brief		Traits class defining the properties of a vector.
		 */
		template<class V>
		struct vector_traits : detail::vector_traits<V>
		{
		};

		/**
		 * @brief		Tests that a class has a `vector()` member function.
		 * @details		The requirements are:
		 *					- takes no arguments
		 *					- const
		 *					- is non-static
		 *					- returns a tuple of the same type as the classes `tuple_type`.
		 */
		template<typename T>
		concept has_vector = requires(T v) {
			{ v.vector() } -> std::same_as<typename vector_traits<T>::tuple_type>;
		};

		/**
		 * @brief		Tests that a class has a `setVector(const tuple_type&)` member function.
		 */
		template<typename T>
		concept has_setVector = requires(T v) {
			{ v.setVector(typename vector_traits<T>::tuple_type{}) } -> std::same_as<void>;
		};

		/**
		 * @brief		Trait which tests whether a class satisfies the `vector` concept.
		 * @details		To satisfy the `vector` concept, a class must:
		 *					- be default constructible.
		 *					- have a `reference_frame` typedef.
		 *					- have a `tuple_type` typedef.
		 *					- have a `frame_data_type` typedef.
		 *					- have a `vector_tag` typedef.
		 *					- have a `tuple_type vector() const` member function.
		 *					- have a `void setVector(const tuple_type&)` member function.
		 */
		template<typename T>
		concept is_vector = std::is_default_constructible_v<T> && has_reference_frame<T, vector_traits> && has_tuple_type<T, vector_traits> && has_vector<T> &&
		                    has_setVector<T>;
	}    // namespace traits

	//----------------------------------
	//	CONVERSION FUNCTIONS
	//----------------------------------

	template<is_point PointFrom, is_point PointTo>
	void convert(const PointFrom& pointFrom, PointTo& pointTo)
	{
		using FrameFrom = point_traits<PointFrom>::reference_frame;
		using FrameTo   = point_traits<PointTo>::reference_frame;

		static_assert(
		        is_convertible_frame<FrameFrom, FrameTo>,
		        "No known conversions between `FrameFrom` and `FrameTo`. It may be possible to diagnose this error using the least_common_ancestor trait.");

		pointTo.setPoint(convert<FrameFrom, FrameTo>(pointFrom.point(), pointFrom.frameData(), pointTo.frameData()));
	}
}    // namespace coordinates

//----------------------------------
//	POINT ARITHMETIC
//----------------------------------

template<is_point PointLhs, is_point PointRhs>
bool operator==(const PointLhs& lhs, const PointRhs& rhs)
{
	static_assert(is_convertible_point<PointLhs, PointRhs>, "No known conversion between types `PointLhs` and `PointRhs`.");

	// convert both points to the nearest frame of reference which is Cartesian.
	CartesianTuple l;
	CartesianTuple r;

	// if the frame data is the same, use the nearest cartesian reference
	if (lhs.frameData() == rhs.frameData())
	{
		using LCA = least_common_cartesian_ancestor<typename point_traits<PointLhs>::reference_frame, typename point_traits<PointRhs>::reference_frame>::type;

		l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}
	// otherwise, use the base reference to make sure all necessary conversions are performed.
	else
	{
		using LCA = lowest_base_frame<typename point_traits<PointLhs>::reference_frame>::type;

		l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}

	const auto x0 = std::get<0>(l);
	const auto y0 = std::get<1>(l);
	const auto z0 = std::get<2>(l);
	const auto x1 = std::get<0>(r);
	const auto y1 = std::get<1>(r);
	const auto z1 = std::get<2>(r);

	return (x0 == x1 && y0 == y1 && z0 == z1);
}

template<is_point PointLhs, is_point PointRhs>
bool operator!=(const PointLhs& lhs, const PointRhs& rhs)
{
	// convert both points to the nearest frame of reference which is Cartesian.
	CartesianTuple l;
	CartesianTuple r;

	// if the frame data is the same, use the nearest cartesian reference
	if (lhs.frameData() == rhs.frameData())
	{
		using LCA = least_common_cartesian_ancestor<typename point_traits<PointLhs>::reference_frame, typename point_traits<PointRhs>::reference_frame>::type;

		l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}
	// otherwise, use the base reference to make sure all necessary conversions are performed.
	else
	{
		using LCA = lowest_base_frame<typename point_traits<PointLhs>::reference_frame>::type;

		l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}

	return l != r;
}

template<is_cartesian_point Point>
Point operator*(const Point& p, dimensionless<> factor)
{
	auto x = std::get<0>(p.point());
	auto y = std::get<1>(p.point());
	auto z = std::get<2>(p.point());

	Point ret;
	ret.setPoint(x * factor, y * factor, z * factor);
	ret.setFrameData(p.frameData());
	return ret;
}

template<is_cartesian_point Point>
Point operator*(dimensionless<> factor, const Point& p)
{
	auto x = std::get<0>(p.point());
	auto y = std::get<1>(p.point());
	auto z = std::get<2>(p.point());

	Point ret;
	ret.setPoint(x * factor, y * factor, z * factor);
	ret.setFrameData(p.frameData());
	return ret;
}

template<is_cartesian_point PointLhs, is_cartesian_point PointRhs>
PointLhs operator+(const PointLhs& lhs, const PointRhs& rhs)
{
	static_assert(is_convertible_point<PointLhs, PointRhs>, "No known conversion between types `PointLhs` and `PointRhs`.");

	// convert both points to the nearest frame of reference which is Cartesian.
	CartesianTuple l;
	CartesianTuple r;

	// if the frame data is the same, use the nearest cartesian reference
	if (lhs.frameData() == rhs.frameData())
	{
		using LCA = least_common_cartesian_ancestor<typename point_traits<PointLhs>::reference_frame, typename point_traits<PointRhs>::reference_frame>::type;

		l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}
	// otherwise, convert to the lhs reference frame/origin
	else
	{
		using Base = lowest_base_frame<typename point_traits<PointRhs>::reference_frame>::type;

		l                 = lhs.point();
		auto intermediate = convert<typename point_traits<PointRhs>::reference_frame, Base>(rhs.point(), rhs.frameData(), rhs.frameData());
		r                 = convert<Base, typename point_traits<PointLhs>::reference_frame>(intermediate, rhs.frameData(), lhs.frameData());
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

template<is_cartesian_point Point>
Point operator+(const Point& p)
{ return p; }

template<is_cartesian_point PointLhs, is_cartesian_point PointRhs>
PointLhs operator-(const PointLhs& lhs, const PointRhs& rhs)
{
	static_assert(is_convertible_point<PointLhs, PointRhs>, "No known conversion between types `PointLhs` and `PointRhs`.");

	// convert both points to the nearest frame of reference which is Cartesian.
	CartesianTuple l;
	CartesianTuple r;

	// if the frame data is the same, use the nearest cartesian reference
	if (lhs.frameData() == rhs.frameData())
	{
		using LCA = least_common_cartesian_ancestor<typename point_traits<PointLhs>::reference_frame, typename point_traits<PointRhs>::reference_frame>::type;

		l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
		r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
	}
	// otherwise, convert to the lhs reference frame/origin
	else
	{
		using Base = lowest_base_frame<typename point_traits<PointRhs>::reference_frame>::type;

		l                 = lhs.point();
		auto intermediate = convert<typename point_traits<PointRhs>::reference_frame, Base>(rhs.point(), rhs.frameData(), rhs.frameData());
		r                 = convert<Base, typename point_traits<PointLhs>::reference_frame>(intermediate, rhs.frameData(), lhs.frameData());
	}

	const auto x0 = std::get<0>(l);
	const auto y0 = std::get<1>(l);
	const auto z0 = std::get<2>(l);
	const auto x1 = std::get<0>(r);
	const auto y1 = std::get<1>(r);
	const auto z1 = std::get<2>(r);

	PointLhs ret;
	ret.setPoint(x0 - x1, y0 - y1, z0 - z1);
	ret.setFrameData(lhs.frameData());
	return ret;
}

template<is_cartesian_point Point>
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

template<is_cartesian_point Point>
Point operator/(const Point& p, dimensionless<> divisor)
{
	auto x = std::get<0>(p.point());
	auto y = std::get<1>(p.point());
	auto z = std::get<2>(p.point());

	Point ret;
	ret.setPoint(x / divisor, y / divisor, z / divisor);
	ret.setFrameData(p.frameData());
	return ret;
}

#endif    // point_h