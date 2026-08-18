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

#ifndef algorithm_h
#define algorithm_h

//------------------------
//	INCLUDES
//------------------------

#include <atomic>
#include <cmath>
#include <future>
#include <limits>
#include <numbers>
#include <vector>

#include <units.h>

#include "CAS.h"
#include "abstractTile.h"
#include "point.h"
#include "threadPool.h"

#include "ellipsoid.h"
#include "geodesicDirectResult.h"
#include "geodesicInverseResult.h"
#include "intersection.h"

#include "coordinates_fwd.h"

//------------------------
//	ALGORITHMS
//------------------------
inline namespace coordinates
{

	using namespace units::literals;

	/**
	 * @brief		Tests whether a point is null
	 * @details		A point is null if all of its values are (0,0,0)
	 * @param[in]	point	Point to test
	 * @returns		true if <i>point</i> is null, false otherwise.
	 */
	template<class Point>
	bool isNull(const Point& point)
	{
		static_assert(coordinates::is_point<Point>, "Template parameter `Point` does not satisfy the `point` concept.");

		auto x  = std::get<0>(point.point());
		auto y  = std::get<1>(point.point());
		auto z  = std::get<2>(point.point());
		auto x0 = std::get<0>(point.frameData().origin);
		auto y0 = std::get<1>(point.frameData().origin);
		auto z0 = std::get<2>(point.frameData().origin);

		return (x == decltype(x)(0) && y == decltype(y)(0) && z == decltype(z)(0) && x0 == decltype(x0)(0) && y0 == decltype(y0)(0) && z0 == decltype(z0)(0));
	}

	template<class Units, class PointLhs, class PointRhs>
	    requires units::traits::is_unit_v<Units>
	bool isSame(const PointLhs& lhs, const PointRhs& rhs, Units tolerance = Units{0.0})
	{
		static_assert(coordinates::is_point<PointLhs>, "Template parameter `PointLhs` does not satisfy the `point` concept.");
		static_assert(coordinates::is_point<PointRhs>, "Template parameter `PointRhs` does not satisfy the `point` concept.");
		static_assert(coordinates::is_convertible_point<PointLhs, PointRhs>, "No known conversion between types `PointLhs` and `PointRhs`.");

		// if the origins match, perform a direct comparison
		if (lhs.frameData() == rhs.frameData())
		{
			// convert both points to the nearest frame of reference which is Cartesian.
			using LCA =
			        least_common_cartesian_ancestor<typename point_traits<PointLhs>::reference_frame, typename point_traits<PointRhs>::reference_frame>::type;

			auto l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);

			return ((abs(x0 - x1) <= tolerance) && (abs(y0 - y1) <= tolerance) && (abs(z0 - z1) <= tolerance));
		}
		// otherwise, convert to ECEF
		else
		{
			using BaseFrame = lowest_base_frame<typename point_traits<PointLhs>::reference_frame>::type;

			CartesianTuple l = convert<typename point_traits<PointLhs>::reference_frame, BaseFrame>(lhs.point(), lhs.frameData(), lhs.frameData());
			CartesianTuple r = convert<typename point_traits<PointRhs>::reference_frame, BaseFrame>(rhs.point(), rhs.frameData(), rhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);

			return ((abs(x0 - x1) <= tolerance) && (abs(y0 - y1) <= tolerance) && (abs(z0 - z1) <= tolerance));
		}
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
	 * @param[in]	lhs			this point
	 * @param[in]	rhs			point to compare to. May have different units than this point
	 * @param[in]	tolerance	acceptable difference between the two points which will still
	 * 							compare equal. The tolerance point must have the same type and
	 *							units as this instance of the class.
	 * @returns		true if the values are equal within the tolerance, false otherwise.
	 */
	template<is_point PointLhs, is_point PointRhs, is_point PointTol>
	bool isSame(const PointLhs& lhs, const PointRhs& rhs, const PointTol& tolerance)
	{
		static_assert(is_point<PointLhs>, "Template parameter `PointLhs` does not satisfy the `point` concept.");
		static_assert(is_point<PointRhs>, "Template parameter `PointRhs` does not satisfy the `point` concept.");
		static_assert(is_convertible_point<PointLhs, PointRhs>, "No known conversion between types `PointLhs` and `PointRhs`.");
		static_assert(is_convertible_point<PointLhs, PointTol>, "No known conversion between types `PointTol` and `PointLhs`.");

		// convert both points to the nearest frame of reference which is Cartesian.
		// if the origins match, perform a direct comparison
		if (lhs.frameData() == rhs.frameData() && rhs.frameData() == tolerance.frameData())
		{
			using LCAtemp =
			        least_common_cartesian_ancestor<typename point_traits<PointLhs>::reference_frame, typename point_traits<PointRhs>::reference_frame>::type;
			using LCA = least_common_cartesian_ancestor<LCAtemp, typename point_traits<PointTol>::reference_frame>::type;

			auto l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
			auto t = convert<typename point_traits<PointTol>::reference_frame, LCA>(tolerance.point(), tolerance.frameData());

			auto x0   = std::get<0>(l);
			auto y0   = std::get<1>(l);
			auto z0   = std::get<2>(l);
			auto x1   = std::get<0>(r);
			auto y1   = std::get<1>(r);
			auto z1   = std::get<2>(r);
			auto xTol = std::get<0>(t);
			auto yTol = std::get<1>(t);
			auto zTol = std::get<2>(t);

			return ((abs(x0 - x1) <= xTol) && (abs(y0 - y1) <= yTol) && (abs(z0 - z1) <= zTol));
		}
		// otherwise, convert to ECEF
		else
		{
			using LCA = lowest_base_frame<typename point_traits<PointTol>::reference_frame>::type;

			auto l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
			auto t = convert<typename point_traits<PointTol>::reference_frame, LCA>(tolerance.point(), tolerance.frameData());

			auto x0   = std::get<0>(l);
			auto y0   = std::get<1>(l);
			auto z0   = std::get<2>(l);
			auto x1   = std::get<0>(r);
			auto y1   = std::get<1>(r);
			auto z1   = std::get<2>(r);
			auto xTol = std::get<0>(t);
			auto yTol = std::get<1>(t);
			auto zTol = std::get<2>(t);

			return ((abs(x0 - x1) <= xTol) && (abs(y0 - y1) <= yTol) && (abs(z0 - z1) <= zTol));
		}
	}

	/**
	 * @brief		Calculates the distance between two points.
	 * @details		The distance calculated is the straight-line slant range <i>from</i> lhs <i>to</i> rhs.
	 * @param[in]	lhs	Point the calculate the distance <i>from</i>.
	 * @param[in]	rhs	Point to calculate the distance <i>to</i>.
	 * @returns		distance between the two points, in units of `distance_unit`, which defaults to
	 *				meters.
	 */
	template<class PointLhs, class PointRhs, class distance_unit = meters<>>
	distance_unit distance(const PointLhs& lhs, const PointRhs& rhs)
	{
		static_assert(is_point<PointLhs>, "Template parameter `PointLhs` does not satisfy the `point` concept.");
		static_assert(is_point<PointRhs>, "Template parameter `PointRhs` does not satisfy the `point` concept.");
		static_assert(is_convertible_point<PointLhs, PointRhs>, "No known conversion between types `PointLhs` and `PointRhs`.");

		// if the origins match, perform a direct comparison
		if (lhs.frameData() == rhs.frameData())
		{
			// convert both points to the nearest frame of reference which is Cartesian.
			using LCA =
			        least_common_cartesian_ancestor<typename point_traits<PointLhs>::reference_frame, typename point_traits<PointRhs>::reference_frame>::type;

			auto l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			distance_unit x0 = std::get<0>(l);
			distance_unit y0 = std::get<1>(l);
			distance_unit z0 = std::get<2>(l);
			distance_unit x1 = std::get<0>(r);
			distance_unit y1 = std::get<1>(r);
			distance_unit z1 = std::get<2>(r);

			// 3-D distance formula
			return sqrt(pow<2>(x1 - x0) + pow<2>(y1 - y0) + pow<2>(z1 - z0));
		}
		// otherwise, convert to ECEF
		else
		{
			// convert both points to the nearest frame of reference which is Cartesian.
			using LCA = lowest_base_frame<typename point_traits<PointLhs>::reference_frame>::type;

			auto l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			distance_unit x0 = std::get<0>(l);
			distance_unit y0 = std::get<1>(l);
			distance_unit z0 = std::get<2>(l);
			distance_unit x1 = std::get<0>(r);
			distance_unit y1 = std::get<1>(r);
			distance_unit z1 = std::get<2>(r);

			// 3-D distance formula
			return sqrt(pow<2>(x1 - x0) + pow<2>(y1 - y0) + pow<2>(z1 - z0));
		}
	}

	/**
	 * @brief		Calculates the magnitude of a point/vector
	 * @details
	 * @param[in]	point	Point to calculate the magnitude of
	 * @returns		magnitude in units of
	 */
	template<class Point>
	auto magnitude(const Point& point) -> std::decay_t<decltype(std::get<2>(point.point()))>
	{
		static_assert(is_point<Point>, "Template parameter `Point` does not satisfy the `point` concept.");

		// convert both points to the nearest frame of reference which is Cartesian.
		using LCA = least_common_cartesian_ancestor<typename point_traits<Point>::reference_frame, typename point_traits<Point>::reference_frame>::type;

		auto p = convert<typename point_traits<Point>::reference_frame, LCA>(point.point(), point.frameData());

		auto x = std::get<0>(p);
		auto y = std::get<1>(p);
		auto z = std::get<2>(p);

		// 3-D distance formula
		return sqrt(pow<2>(x) + pow<2>(y) + pow<2>(z));
	}

	/**
	 * @brief		Calculates the dot product between two points.
	 * @details		https://en.wikipedia.org/wiki/Dot_product
	 * @param[in]	lhs	left-hand side point.
	 * @param[in]	rhs right-hand side point.
	 * @returns		dot product of lhs and rhs (wrt to the origin of lhs).
	 */
	template<is_point PointLhs, is_point PointRhs>
	auto dotProduct(const PointLhs& lhs, const PointRhs& rhs) -> decltype(pow<2>(std::get<2>(lhs.point())))
	{
		static_assert(is_convertible_point<PointLhs, PointRhs>, "No known conversion between types `PointLhs` and `PointRhs`.");

		// if the origins match, perform a direct comparison
		if (lhs.frameData() == rhs.frameData())
		{
			// convert both points to the nearest frame of reference which is Cartesian.
			using LCA =
			        least_common_cartesian_ancestor<typename point_traits<PointLhs>::reference_frame, typename point_traits<PointRhs>::reference_frame>::type;

			auto l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);

			// dot product formula
			return x0 * x1 + y0 * y1 + z0 * z1;
		}
		// otherwise, convert to lhs frame (through an intermediary to ensure the origin is translated)
		else
		{
			using LCA =
			        least_common_cartesian_ancestor<typename point_traits<PointLhs>::reference_frame, typename point_traits<PointRhs>::reference_frame>::type;
			using Base = lowest_base_frame<typename point_traits<PointLhs>::reference_frame>::type;

			auto l             = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto rIntermediate = convert<typename point_traits<PointRhs>::reference_frame, Base>(rhs.point(), rhs.frameData(), rhs.frameData());
			auto r             = convert<Base, LCA>(rIntermediate, FrameData(), lhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);

			// dot product formula
			return x0 * x1 + y0 * y1 + z0 * z1;
		}
	}

	/**
	 * @brief		Calculates the cross product of two points
	 * @details		https://en.wikipedia.org/wiki/Cross_product
	 * @param[in]	lhs	left-hand side point.
	 * @param[in]	rhs right-hand side point.
	 * @returns		doct product of lhs and rhs.
	 */
	template<is_point PointLhs, is_point PointRhs>
	auto crossProduct(const PointLhs& lhs, const PointRhs& rhs)
	        -> std::tuple<decltype(pow<2>(std::get<2>(lhs.point()))), decltype(pow<2>(std::get<2>(lhs.point()))), decltype(pow<2>(std::get<2>(lhs.point())))>
	{
		static_assert(is_convertible_point<PointLhs, PointRhs>, "No known conversion between types `PointLhs` and `PointRhs`.");

		// convert both points to the nearest frame of reference which is Cartesian.
		using cross_product_type =
		        std::tuple<decltype(pow<2>(std::get<0>(lhs.point()))), decltype(pow<2>(std::get<1>(lhs.point()))), decltype(pow<2>(std::get<2>(lhs.point())))>;
		// if the origins match, perform a direct comparison
		if (lhs.frameData() == rhs.frameData())
		{
			using LCA =
			        least_common_cartesian_ancestor<typename point_traits<PointLhs>::reference_frame, typename point_traits<PointRhs>::reference_frame>::type;

			auto l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);

			// dot product formula
			return cross_product_type(y0 * z1 - z0 * y1, z0 * x1 - x0 * z1, x0 * y1 - y0 * x1);
		}
		// otherwise, convert to ECEF
		else
		{
			using LCA = lowest_base_frame<typename point_traits<PointLhs>::reference_frame>::type;

			auto l = convert<typename point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = convert<typename point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);

			// dot product formula
			return cross_product_type(y0 * z1 - z0 * y1, z0 * x1 - x0 * z1, x0 * y1 - y0 * x1);
		}
	}

	//------------------------
	//	GEODESIC UTILITIES
	//------------------------

	/**
	 * @brief		Normalizes an angle to the range [0, 360).
	 * @param[in]	angle	Angle to normalize.
	 * @return		Normalized angle in degrees.
	 */
	inline degrees<> wrap360(degrees<> angle)
	{
		auto x = angle.to<double>();
		x      = std::fmod(x, 360.0);
		if (x < 0.0)
		{
			x += 360.0;
		}
		return units::angle::degrees<>(x);
	}

	/**
	 * @brief		Normalizes an angle to the range (-180, 180].
	 * @param[in]	angle	Angle to normalize.
	 * @return		Normalized angle in degrees.
	 */
	inline degrees<> wrap180(degrees<> angle)
	{
		auto x = angle.to<double>();
		x      = std::fmod(x, 360.0);
		if (x <= -180.0)
		{
			x += 360.0;
		}
		else if (x > 180.0)
		{
			x -= 360.0;
		}
		return units::angle::degrees<>(x);
	}

	/**
	 * @brief		Solves the inverse geodesic problem on an ellipsoid.
	 * @details		Returns the surface distance and forward azimuths using Vincenty's inverse method.
	 *				If the iterative solve fails to converge (near-antipodal cases), a spherical fallback
	 *				is used (based on the ellipsoid semi-major axis).
	 *
	 *				This function is intended for navigation and simulation use-cases; for guaranteed
	 *				convergence and nanometer-level accuracy on all pairs, consider a Karney-style method.
	 * @tparam		Datum	Datum of the input points.
	 * @tparam		EllipsoidType	Ellipsoid type (must satisfy the ellipsoid concept).
	 * @param[in]	a	First geodetic point.
	 * @param[in]	b	Second geodetic point.
	 * @return		GeodesicInverseResult containing distance, initial bearing, and final bearing.
	 */
	template<class Datum, class EllipsoidType = horizontal_datum_traits<typename datum_traits<Datum>::horizontal_datum>::reference_ellipsoid>
	GeodesicInverseResult geodesicInverse(const PositionGeodetic<Datum>& a, const PositionGeodetic<Datum>& b)
	{
		static_assert(traits::is_datum<Datum>, "`Datum` template parameter does not satisfy the datum concept.");
		static_assert(traits::is_ellipsoid<EllipsoidType>, "`EllipsoidType` template parameter does not satisfy the ellipsoid concept.");

		using units::angle::degrees;
		using units::angle::radians;
		using units::length::meters;

		// Convert inputs to radians (typed), then immediately extract scalars for trig.
		const auto phi1_q = radians<>(a.latitude());
		const auto phi2_q = radians<>(b.latitude());
		const auto L_q    = radians<>(wrap180(b.longitude() - a.longitude()));

		const double phi1 = phi1_q.template to<double>();
		const double phi2 = phi2_q.template to<double>();
		const double L    = L_q.template to<double>();

		// Ellipsoid parameters
		const double f = EllipsoidType::f();

		// These should be in meters. If EllipsoidType::a()/b() already return meters<>,
		// `.to<double>()` is the minimal boundary-crossing needed.
		const double a_m = EllipsoidType::a().template to<double>();
		const double b_m = EllipsoidType::b().template to<double>();

		// Reduced latitudes
		const double U1 = std::atan((1.0 - f) * std::tan(phi1));
		const double U2 = std::atan((1.0 - f) * std::tan(phi2));

		const double sinU1 = std::sin(U1);
		const double cosU1 = std::cos(U1);
		const double sinU2 = std::sin(U2);
		const double cosU2 = std::cos(U2);

		// Iteration
		double lambda      = L;
		double lambda_prev = 0.0;

		double sinSigma   = 0.0;
		double cosSigma   = 0.0;
		double sigma      = 0.0;
		double sinAlpha   = 0.0;
		double cos2Alpha  = 0.0;
		double cos2SigmaM = 0.0;

		constexpr int    kMaxIters  = 200;
		constexpr double kLambdaEps = 1e-14;

		bool converged = false;

		for (int iter = 0; iter < kMaxIters; ++iter)
		{
			const double sinLambda = std::sin(lambda);
			const double cosLambda = std::cos(lambda);

			const double t1 = cosU2 * sinLambda;
			const double t2 = cosU1 * sinU2 - sinU1 * cosU2 * cosLambda;

			sinSigma = std::sqrt(t1 * t1 + t2 * t2);
			if (sinSigma == 0.0)
			{
				// coincident points
				return GeodesicInverseResult(0.0_m, 0.0_deg, 0.0_deg);
			}

			cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
			sigma    = std::atan2(sinSigma, cosSigma);

			sinAlpha  = (cosU1 * cosU2 * sinLambda) / sinSigma;
			cos2Alpha = 1.0 - sinAlpha * sinAlpha;

			if (cos2Alpha != 0.0)
			{
				cos2SigmaM = cosSigma - (2.0 * sinU1 * sinU2) / cos2Alpha;
			}
			else
			{
				cos2SigmaM = 0.0;    // equatorial line
			}

			const double C = (f / 16.0) * cos2Alpha * (4.0 + f * (4.0 - 3.0 * cos2Alpha));

			lambda_prev = lambda;
			lambda      = L + (1.0 - C) * f * sinAlpha * (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)));

			if (std::abs(lambda - lambda_prev) < kLambdaEps)
			{
				converged = true;
				break;
			}
		}

		if (!converged)
		{
			// Spherical fallback on radius a (semi-major axis).
			const double dphi = phi2 - phi1;
			const double dlam = L;

			const double sin_dphi_2 = std::sin(dphi / 2.0);
			const double sin_dlam_2 = std::sin(dlam / 2.0);

			const double h = sin_dphi_2 * sin_dphi_2 + std::cos(phi1) * std::cos(phi2) * sin_dlam_2 * sin_dlam_2;

			const double c = 2.0 * std::atan2(std::sqrt(h), std::sqrt(1.0 - h));
			const double s = a_m * c;

			const double y1   = std::sin(dlam) * std::cos(phi2);
			const double x1   = std::cos(phi1) * std::sin(phi2) - std::sin(phi1) * std::cos(phi2) * std::cos(dlam);
			const double azi1 = std::atan2(y1, x1);

			const double y2   = std::sin(-dlam) * std::cos(phi1);
			const double x2   = std::cos(phi2) * std::sin(phi1) - std::sin(phi2) * std::cos(phi1) * std::cos(-dlam);
			const double azi2 = std::atan2(y2, x2);

			return GeodesicInverseResult(meters<>(s), wrap360(degrees<>(radians<>(azi1))), wrap360(degrees<>(radians<>(azi2))));
		}

		// Vincenty correction terms (dimensionless series)
		const double u2 = cos2Alpha * ((a_m * a_m - b_m * b_m) / (b_m * b_m));
		const double A  = 1.0 + (u2 / 16384.0) * (4096.0 + u2 * (-768.0 + u2 * (320.0 - 175.0 * u2)));
		const double B  = (u2 / 1024.0) * (256.0 + u2 * (-128.0 + u2 * (74.0 - 47.0 * u2)));

		const double deltaSigma =
		        B * sinSigma *
		        (cos2SigmaM + (B / 4.0) * (cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM) -
		                                   (B / 6.0) * cos2SigmaM * (-3.0 + 4.0 * sinSigma * sinSigma) * (-3.0 + 4.0 * cos2SigmaM * cos2SigmaM)));

		const double s = b_m * A * (sigma - deltaSigma);

		// Bearings (radians -> degrees) without any nested to<double>() chains
		const double sinLambda = std::sin(lambda);
		const double cosLambda = std::cos(lambda);

		const double azi1 = std::atan2(cosU2 * sinLambda, cosU1 * sinU2 - sinU1 * cosU2 * cosLambda);
		const double azi2 = std::atan2(cosU1 * sinLambda, -sinU1 * cosU2 + cosU1 * sinU2 * cosLambda);

		const auto azi1_deg = wrap360(degrees<>(radians<>(azi1)));
		const auto azi2_deg = wrap360(degrees<>(radians<>(azi2)));

		return GeodesicInverseResult(meters<>(s), azi1_deg, azi2_deg);
	}

	/**
	 * @brief		Solves the direct geodesic problem on an ellipsoid.
	 * @details		Given a start point, initial bearing, and distance, returns the destination point and the
	 *				forward azimuth at destination.
	 * @tparam		Datum			Datum of the input point.
	 * @tparam		EllipsoidType	Ellipsoid type (defaults to the datum's reference ellipsoid).
	 * @param[in]	start			Start geodetic point.
	 * @param[in]	initialBearing	Forward azimuth at start.
	 * @param[in]	distance		Surface distance to travel along the geodesic.
	 * @return		GeodesicDirectResult containing destination point and final bearing.
	 */
	template<class Datum, class EllipsoidType = horizontal_datum_traits<typename datum_traits<Datum>::horizontal_datum>::reference_ellipsoid>
	GeodesicDirectResult<PositionGeodetic<Datum>> geodesicDirect(const PositionGeodetic<Datum>& start, degrees<> initialBearing, meters<> distance)
	{
		static_assert(traits::is_datum<Datum>, "`Datum` template parameter does not satisfy the datum concept.");
		static_assert(traits::is_ellipsoid<EllipsoidType>, "`EllipsoidType` template parameter does not satisfy the ellipsoid concept.");

		// Use long double internally to reduce accumulated error for long-haul routes.
		// The public API remains double-precision via the units types.
		using fp_t = long double;

		const fp_t f   = static_cast<fp_t>(EllipsoidType::f().template to<long double>());
		const fp_t a_m = static_cast<fp_t>(EllipsoidType::a().template to<long double>());
		const fp_t b_m = static_cast<fp_t>(EllipsoidType::b().template to<long double>());

		const fp_t phi1   = units::angle::radians<>(start.latitude()).to<long double>();
		const fp_t L1     = units::angle::radians<>(start.longitude()).to<long double>();
		const fp_t alpha1 = units::angle::radians<>(wrap360(initialBearing)).to<long double>();

		const fp_t sinAlpha1 = std::sin(alpha1);
		const fp_t cosAlpha1 = std::cos(alpha1);

		const fp_t tanU1 = (static_cast<fp_t>(1.0) - f) * std::tan(phi1);
		const fp_t cosU1 = static_cast<fp_t>(1.0) / std::sqrt(static_cast<fp_t>(1.0) + tanU1 * tanU1);
		const fp_t sinU1 = tanU1 * cosU1;

		const fp_t sigma1    = std::atan2(tanU1, cosAlpha1);
		const fp_t sinAlpha  = cosU1 * sinAlpha1;
		const fp_t cos2Alpha = static_cast<fp_t>(1.0) - sinAlpha * sinAlpha;

		const fp_t u2 = cos2Alpha * ((a_m * a_m - b_m * b_m) / (b_m * b_m));
		const fp_t A  = static_cast<fp_t>(1.0) +
		               (u2 / static_cast<fp_t>(16384.0)) *
		                       (static_cast<fp_t>(4096.0) + u2 * (-static_cast<fp_t>(768.0) + u2 * (static_cast<fp_t>(320.0) - static_cast<fp_t>(175.0) * u2)));
		const fp_t B = (u2 / static_cast<fp_t>(1024.0)) *
		               (static_cast<fp_t>(256.0) + u2 * (-static_cast<fp_t>(128.0) + u2 * (static_cast<fp_t>(74.0) - static_cast<fp_t>(47.0) * u2)));

		fp_t sigma      = static_cast<fp_t>(distance.to<long double>()) / (b_m * A);
		fp_t sigmaP     = static_cast<fp_t>(2.0) * std::numbers::pi_v<fp_t>;
		fp_t cos2SigmaM = static_cast<fp_t>(0.0);
		fp_t sinSigma   = static_cast<fp_t>(0.0);
		fp_t cosSigma   = static_cast<fp_t>(0.0);
		fp_t deltaSigma = static_cast<fp_t>(0.0);

		// Tight convergence is required for sub-meter agreement in the unit tests.
		for (int iter = 0; iter < 500 && std::abs(sigma - sigmaP) > static_cast<fp_t>(1e-18L); ++iter)
		{
			cos2SigmaM = std::cos(static_cast<fp_t>(2.0) * sigma1 + sigma);
			sinSigma   = std::sin(sigma);
			cosSigma   = std::cos(sigma);

			deltaSigma = B * sinSigma *
			             (cos2SigmaM + (B / static_cast<fp_t>(4.0)) * (cosSigma * (-static_cast<fp_t>(1.0) + static_cast<fp_t>(2.0) * cos2SigmaM * cos2SigmaM) -
			                                                           (B / static_cast<fp_t>(6.0)) * cos2SigmaM *
			                                                                   (-static_cast<fp_t>(3.0) + static_cast<fp_t>(4.0) * sinSigma * sinSigma) *
			                                                                   (-static_cast<fp_t>(3.0) + static_cast<fp_t>(4.0) * cos2SigmaM * cos2SigmaM)));

			sigmaP = sigma;
			sigma  = static_cast<fp_t>(distance.to<long double>()) / (b_m * A) + deltaSigma;
		}

		const fp_t tmp = sinU1 * sinSigma - cosU1 * cosSigma * cosAlpha1;
		const fp_t phi2 =
		        std::atan2(sinU1 * cosSigma + cosU1 * sinSigma * cosAlpha1, (static_cast<fp_t>(1.0) - f) * std::sqrt(sinAlpha * sinAlpha + tmp * tmp));

		const fp_t lambda = std::atan2(sinSigma * sinAlpha1, cosU1 * cosSigma - sinU1 * sinSigma * cosAlpha1);

		const fp_t C = (f / static_cast<fp_t>(16.0)) * cos2Alpha * (static_cast<fp_t>(4.0) + f * (static_cast<fp_t>(4.0) - static_cast<fp_t>(3.0) * cos2Alpha));
		const fp_t L =
		        lambda -
		        (static_cast<fp_t>(1.0) - C) * f * sinAlpha *
		                (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-static_cast<fp_t>(1.0) + static_cast<fp_t>(2.0) * cos2SigmaM * cos2SigmaM)));

		const fp_t lon2 = L1 + L;

		const fp_t alpha2 = std::atan2(sinAlpha, -tmp);

		PositionGeodetic<Datum> dst(units::angle::degrees<>(units::angle::radians<>(static_cast<double>(phi2))),
		                            units::angle::degrees<>(wrap180(units::angle::degrees<>(units::angle::radians<>(static_cast<double>(lon2))))),
		                            start.altitude().template to<units::length::meters<>>(),
		                            start.frameData().date);

		const auto azi2_deg = wrap360(units::angle::degrees<>(static_cast<double>(alpha2 * static_cast<fp_t>(180.0) / std::numbers::pi_v<fp_t>)));

		return GeodesicDirectResult<PositionGeodetic<Datum>>(dst, azi2_deg);
	}

	/**
	 * @brief		Convenience wrapper returning only the geodesic distance.
	 * @tparam		Datum	Datum of the input points.
	 * @param[in]	a	First point.
	 * @param[in]	b	Second point.
	 * @return		Surface distance between points (Great Circle).
	 */
	template<class Datum>
	meters<> geodesicDistance(const PositionGeodetic<Datum>& a, const PositionGeodetic<Datum>& b)
	{ return geodesicInverse<Datum>(a, b).distance(); }

	/**
	 * @brief		Convenience wrapper returning the initial bearing.
	 * @tparam		Datum	Datum of the input points.
	 * @param[in]	a	First point.
	 * @param[in]	b	Second point.
	 * @return		Initial bearing at point a.
	 */
	template<class Datum>
	degrees<> initialBearing(const PositionGeodetic<Datum>& a, const PositionGeodetic<Datum>& b)
	{ return geodesicInverse<Datum>(a, b).initialBearing(); }

	/**
	 * @brief		Convenience wrapper returning the final bearing.
	 * @tparam		Datum	Datum of the input points.
	 * @param[in]	a	First point.
	 * @param[in]	b	Second point.
	 * @return		Final bearing at point b.
	 */
	template<class Datum>
	degrees<> finalBearing(const PositionGeodetic<Datum>& a, const PositionGeodetic<Datum>& b)
	{ return geodesicInverse<Datum>(a, b).finalBearing(); }

	//------------------------
	//	ELLIPSOID INTERSECTION
	//------------------------

	/**
	 * @brief		Intersects a ray with the reference ellipsoid.
	 * @details		The ray origin is specified as an ECEF point. The ray direction is given as an ECEF
	 *				Cartesian tuple. The direction does not need to be normalized.
	 * @tparam		Datum	Datum used for ellipsoid parameters.
	 * @param[in]	originECEF	Ray origin point in ECEF.
	 * @param[in]	dirECEF		Ray direction vector in ECEF.
	 * @return		Intersection result (hit flag + intersection point).
	 */
	template<class Datum, class EllipsoidType = horizontal_datum_traits<typename datum_traits<Datum>::horizontal_datum>::reference_ellipsoid>
	Intersection<Datum> intersectEllipsoid(const PositionECEF<Datum>& originECEF, const CartesianTuple& dirECEF)
	{
		static_assert(traits::is_datum<Datum>, "`Datum` template parameter does not satisfy the datum concept.");
		static_assert(traits::is_ellipsoid<EllipsoidType>, "`EllipsoidType` template parameter does not satisfy the ellipsoid concept.");

		const auto ox = std::get<0>(originECEF.point());
		const auto oy = std::get<1>(originECEF.point());
		const auto oz = std::get<2>(originECEF.point());

		const auto dx = std::get<0>(dirECEF);
		const auto dy = std::get<1>(dirECEF);
		const auto dz = std::get<2>(dirECEF);

		const auto a = EllipsoidType::a();
		const auto b = EllipsoidType::b();

		// Scale-space ellipsoid intersection: (x/a)^2 + (y/a)^2 + (z/b)^2 = 1
		const auto A = (dx * dx + dy * dy) / (a * a) + (dz * dz) / (b * b);
		const auto B = 2.0 * ((ox * dx + oy * dy) / (a * a) + (oz * dz) / (b * b));
		const auto C = (ox * ox + oy * oy) / (a * a) + (oz * oz) / (b * b) - 1.0;

		const auto disc = B * B - 4.0 * A * C;
		if (disc < 0.0 || A == 0.0)
		{
			return Intersection<Datum>();
		}

		const auto sqrtDisc = sqrt(disc);
		const auto t0       = (-B - sqrtDisc) / (2.0 * A);
		const auto t1       = (-B + sqrtDisc) / (2.0 * A);

		// Choose nearest positive intersection.
		double t = std::numeric_limits<double>::infinity();
		if (t0 > 0.0)
			t = t0;
		if (t1 > 0.0 && t1 < t)
			t = t1;

		if (!std::isfinite(t))
		{
			return Intersection<Datum>();
		}

		const auto px = units::length::meters<>(ox + t * dx);
		const auto py = units::length::meters<>(oy + t * dy);
		const auto pz = units::length::meters<>(oz + t * dz);

		return Intersection<Datum>(units::length::meters(t), CartesianTuple(px, py, pz), originECEF.frameData().date);
	}

	/**
	 * @brief		Tests whether two points have clear line-of-sight over the ellipsoid.
	 * @details		This is an ellipsoid-only test. Terrain/topography is not considered.
	 * @tparam		Datum	Datum of the points.
	 * @param[in]	observer	Observer point.
	 * @param[in]	target		Target point.
	 * @return		true if the line segment between observer and target does not intersect the ellipsoid interior.
	 */
	template<class Datum, class EllipsoidType = traits::horizontal_datum_traits<typename traits::datum_traits<Datum>::horizontal_datum>::reference_ellipsoid>
	bool isLineOfSight(const PositionECEF<Datum>& observer, const PositionECEF<Datum>& target)
	{
		static_assert(traits::is_datum<Datum>, "`Datum` template parameter does not satisfy the datum concept.");
		static_assert(traits::is_ellipsoid<EllipsoidType>, "`EllipsoidType` template parameter does not satisfy the ellipsoid concept.");

		const auto ox = std::get<0>(observer.point()).template to<double>();
		const auto oy = std::get<1>(observer.point()).template to<double>();
		const auto oz = std::get<2>(observer.point()).template to<double>();

		const auto tx = std::get<0>(target.point()).template to<double>();
		const auto ty = std::get<1>(target.point()).template to<double>();
		const auto tz = std::get<2>(target.point()).template to<double>();

		const double dx = tx - ox;
		const double dy = ty - oy;
		const double dz = tz - oz;

		const double a = EllipsoidType::a().template to<double>();
		const double b = EllipsoidType::b().template to<double>();

		const double A = (dx * dx + dy * dy) / (a * a) + (dz * dz) / (b * b);
		const double B = 2.0 * ((ox * dx + oy * dy) / (a * a) + (oz * dz) / (b * b));
		const double C = (ox * ox + oy * oy) / (a * a) + (oz * oz) / (b * b) - 1.0;

		const double disc = B * B - 4.0 * A * C;
		if (disc <= 0.0)
		{
			// No intersection or tangent only.
			return true;
		}

		const double sqrtDisc = std::sqrt(disc);
		const double t0       = (-B - sqrtDisc) / (2.0 * A);
		const double t1       = (-B + sqrtDisc) / (2.0 * A);

		// If either intersection lies strictly between 0 and 1, the segment penetrates.
		const double eps = 1e-12;
		if ((t0 > eps && t0 < 1.0 - eps) || (t1 > eps && t1 < 1.0 - eps))
		{
			return false;
		}

		// If both endpoints lie on (or extremely near) the ellipsoid surface, the segment can still
		// pass through the interior (e.g., opposite-side surface points). Detect this by testing the
		// midpoint against the implicit ellipsoid equation.
		{
			const double f0      = (ox * ox + oy * oy) / (a * a) + (oz * oz) / (b * b) - 1.0;
			const double f1      = (tx * tx + ty * ty) / (a * a) + (tz * tz) / (b * b) - 1.0;
			const double surfEps = 1e-10;
			if (std::abs(f0) <= surfEps && std::abs(f1) <= surfEps)
			{
				const double mx   = 0.5 * (ox + tx);
				const double my   = 0.5 * (oy + ty);
				const double mz   = 0.5 * (oz + tz);
				const double fmid = (mx * mx + my * my) / (a * a) + (mz * mz) / (b * b) - 1.0;
				if (fmid < -1e-10)
				{
					return false;
				}
			}
		}
		return true;
	}

	inline namespace topography
	{
		/**
		 * @brief		creates image of tile terrain data
		 * @details		returns a  black-and-white image of the elevations stored in the given tile. If
		 *				the resolution is different than the source data, it will be interpolated (bi-cubic).
		 *				White represents the highest elevation in the tile, black represents the lowest.
		 * @param[in]	tile		pointer to the tile to generate an image of.
		 * @param[in]	resolution	angular resolution for each pixel, in degrees. If you want to use
		 *				the dted tile resolution for the image resolution, set this value to
		 *				`tile->metadata().latitudeResolution()`.
		 * @returns		2D vector of bytes, representing a monochrome image.
		 */
		inline std::vector<std::vector<int8_t>> image(const AbstractTile* tile, degrees<> resolution)
		{
			if (tile == nullptr)
			{
				return {};
			}

			ThreadPool& pool = ThreadPool::instance();

			int width  = ((tile->metadata().northeastLatitude() - tile->metadata().southwestLatitude()) / resolution).to<int>() + 1;
			int height = ((tile->metadata().northeastLongitude() - tile->metadata().southwestLongitude()) / resolution).to<int>() + 1;

			std::atomic<int> min;
			std::atomic<int> max;
			min.store(std::numeric_limits<int>::max());
			max.store(std::numeric_limits<int>::min());

			std::vector<std::vector<int>> elevations;
			elevations.resize(height);
			for (auto& elevation : elevations)
			{
				elevation.resize(width);
			}
			// lambda for calculating all the elevation values for a line (row) of latitude
			auto calculateRow = [tile, resolution, width, &elevations, &min, &max](const int row)
			{
				const degrees lat(tile->metadata().northeastLatitude() - resolution * row);

				for (int col = 0; col < width; ++col)
				{
					degrees lon          = tile->metadata().southwestLongitude() + resolution * col;
					elevations[row][col] = tile->elevation(lat, lon).to<int>();
					compare_exchange_if_lessthan(min, elevations[row][col]);
					compare_exchange_if_greater(max, elevations[row][col]);
				}
				return row;
			};

			std::vector<std::future<int>> rowDone;
			rowDone.reserve(height);

			// calculate all the interpolated values
			for (int row = 0; row < height; ++row)
			{
				rowDone.emplace_back(pool.async(calculateRow, row));
			}

			// create/size data vector
			std::vector<std::vector<int8_t>> data;
			data.resize(height);
			for (auto& itr : data)
			{
				itr.resize(width);
			}

			// synchronize with calculateRow
			for (auto& itr : rowDone)
			{
				// wait for ALL rows before doing any scaling b/c we need the global min/max to be correct
				itr.get();
			}

			// avoid divide by 0's
			if (max - min == 0)
			{
				return {};
			}

			// function to color-scale a line (row) of latitude
			auto scaleRow = [width, &data, &elevations, &min, &max](int row)
			{
				for (int col = 0; col < width; ++col)
				{
					data[row][col] = static_cast<int8_t>(255 * (elevations[row][col] - min) / (max - min));
				}

				return row;
			};

			std::vector<std::future<int>> scaleDone;
			scaleDone.reserve(height);

			// scale the image. lowest elevation is black, highest is white.
			for (int row = 0; row < height; ++row)
			{
				scaleDone.emplace_back(pool.async(scaleRow, row));
			}

			// wait for scaling to finish
			for (int row = 0; row < height; ++row)
			{
				scaleDone[row].get();
			}

			return data;
		}

		/**
		 * @brief		latitude dependent factor for hill-shading
		 * @details
		 * @param[in]	latitude latitude to calculate factor for
		 * @returns		z-factor
		 */
		inline meters<> z_factor(const degrees<> latitude)
		{
			// See: http://webhelp.esri.com/arcgisdesktop/9.3/index.cfm?TopicName=Applying%20a%20z-factor
			if (const int val = static_cast<int>(abs(latitude.to<double>())); val >= 0 && val < 10)
				return 0.00000898_m;
			else if (val >= 10 && val < 20)
				return 0.00000912_m;
			else if (val >= 20 && val < 30)
				return 0.00000956_m;
			else if (val >= 30 && val < 40)
				return 0.00001036_m;
			else if (val >= 40 && val < 50)
				return 0.00001171_m;
			else if (val >= 50 && val < 60)
				return 0.00001395_m;
			else if (val >= 60 && val < 70)
				return 0.00001792_m;
			else if (val >= 70 && val < 80)
				return 0.00002619_m;
			else
				return 0.00005156_m;
		}

		/// Per-cell hillshade computation: the ESRI slope/aspect/shade math for one sample point, over its 3x3
		/// elevation window. A `constexpr`-marked free function of its inputs — no captures, no threading — so it is
		/// compile-time-provable where its operations are constant-evaluable and runs normally otherwise (the trig
		/// is `units`' runtime trig). Extracting it out of the threaded row lambda keeps that lambda a plain runtime
		/// callable: it merely calls this function, so it is never promoted to an immediate (`consteval`) function by
		/// a `consteval` `units` construct in the math, which would make it uninvocable through the thread pool.
		/// za..zi are the 3x3 elevation window (zd/zf flank the center; the center elevation is unused, matching the
		/// ESRI kernel), `cellSize` the ground sample distance, `zenith_r`/`azimuth_r` the sun geometry.
		constexpr uint8_t hillshadeCell(meters<> za, meters<> zb, meters<> zc,
		                                meters<> zd, meters<> zf,
		                                meters<> zg, meters<> zh, meters<> zi,
		                                meters<> cellSize, radians<> zenith_r, radians<> azimuth_r)
		{
			const dimensionless<> dz_dx((zc + 2 * zf + zi - (za + 2 * zd + zg)) / (8 * cellSize));
			const dimensionless<> dz_dy((zg + 2 * zh + zi - (za + 2 * zb + zc)) / (8 * cellSize));

			const dimensionless<> slope = atan(sqrt(std::pow(dz_dx, 2.0) + std::pow(dz_dy, 2.0)));
			dimensionless<>       aspect = 0.0;
			if (dz_dx != 0)
			{
				aspect = atan2(dz_dy.value(), -1.0 * dz_dx.value());
				if (aspect < 0)
					aspect = aspect + pi * 2;
			}
			else if (dz_dy > 0)
			{
				aspect = pi / 2;
			}
			else if (dz_dy < 0)
			{
				aspect = 2 * pi - pi / 2;
			}
			else
			{
				aspect = 0.0;
			}

			return static_cast<uint8_t>(abs(255.0 * ((cos(zenith_r.value()) * cos(slope.value())) +
			                                         (sin(zenith_r.value()) * sin(slope.value()) * cos(azimuth_r.value() - aspect.value())))));
		}

		inline std::vector<std::vector<uint8_t>>
		hillshade(const AbstractTile* tile, degrees<> resolution = 0.0_deg, const degrees<> sunAltitude = 45.0_deg, const degrees<> sunAzimuth = 315.0_deg)
		{
			// see: http://edndoc.esri.com/arcobjects/9.2/net/shared/geoprocessing/spatial_analyst_tools/how_hillshade_works.htm
			if (resolution == 0_deg)
			{
				resolution = tile->metadata().latitudeResolution();
			}

			ThreadPool& pool = ThreadPool::instance();

			const int width  = ((tile->metadata().northeastLatitude() - tile->metadata().southwestLatitude()) / resolution).to<int>() + 1;
			const int height = ((tile->metadata().northeastLongitude() - tile->metadata().southwestLongitude()) / resolution).to<int>() + 1;

			std::vector<std::vector<uint8_t>> shade;
			shade.resize(height);
			for (auto& itr : shade)
			{
				itr.resize(width);
			}

			const radians azimuth_r = 360.0_deg - sunAzimuth + 90.0_deg;
			const radians zenith_r  = 90.0_deg - sunAltitude;

			// aprrox cellsize
			meters cellSize((1.0_nmi / 60) * resolution.convert<arcseconds>().to<long double>());

			// lambda for calculating all the elevation values for a line (row) of latitude
			auto calculateRow = [&, tile, cellSize](const int row)
			{
				degrees lat(tile->metadata().northeastLatitude() - resolution * row);

				// Window layout:
				//	+---+---+---+
				//	| a | b | c |
				//	+---+---+---+
				//	| d | e | f |	<- e is the current lat/lon
				//	+---+---+---+
				//	| g | h | i |
				//	+---+---+---+

				for (int col = 1; col < width - 1; ++col)
				{
					degrees lon = tile->metadata().southwestLongitude() + resolution * col;

					// 3x3 elevation kernel around the sample point (z-prefixed to avoid shadowing
					// single-letter globals). Layout: za zb zc / zd (center) zf / zg zh zi.
					meters za = tile->elevation(lat + resolution, lon - resolution);
					meters zb = tile->elevation(lat + resolution, lon);
					meters zc = tile->elevation(lat + resolution, lon + resolution);
					meters zd = tile->elevation(lat, lon - resolution);
					meters zf = tile->elevation(lat, lon + resolution);
					meters zg = tile->elevation(lat - resolution, lon - resolution);
					meters zh = tile->elevation(lat - resolution, lon);
					meters zi = tile->elevation(lat - resolution, lon + resolution);

					shade[row][col] = hillshadeCell(za, zb, zc, zd, zf, zg, zh, zi, cellSize, zenith_r, azimuth_r);
				}
				return row;
			};

			std::vector<std::future<int>> rowDone;
			rowDone.reserve(height);

			// calculate all the interpolated values
			for (int row = 1; row < height - 1; ++row)
			{
				rowDone.emplace_back(pool.async(calculateRow, row));
			}

			// synchronize with calculateRow
			for (auto& itr : rowDone)
			{
				// wait for ALL rows before doing any scaling b/c we need the global min/max to be correct
				itr.get();
			}

			return shade;
		}
	}    // namespace topography
}    // namespace coordinates

#endif    // algorithm_h