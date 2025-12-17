#ifndef algorithm_h__
#define algorithm_h__

//------------------------
//	INCLUDES
//------------------------

#include <vector>
#include <future>
#include <atomic>
#include <numeric>

#include <units.h>

#include "point.h"

namespace coord
{
	using namespace units::literals;

	/**
	 * @brief		Tests whether a point is null
	 * @details		A point is null if all of it's values are (0,0,0)
	 * @param[in]	point	Point to test
	 * @returns		true if <i>point</i> is null, false otherwise.
	 */
	template<class Point>
	bool isNull(const Point& point)
	{
		static_assert(coord::traits::is_point<Point>::value, "Template parameter `Point` does not satisfy the `point` concept.");

		auto x = std::get<0>(point.point());
		auto y = std::get<1>(point.point());
		auto z = std::get<2>(point.point());
		auto x0 = std::get<0>(point.frameData().origin);
		auto y0 = std::get<1>(point.frameData().origin);
		auto z0 = std::get<2>(point.frameData().origin);

		return (x == decltype(x)(0) && y == decltype(y)(0) && z == decltype(z)(0) && x0 == decltype(x0)(0) && y0 == decltype(y0)(0) && z0 == decltype(z0)(0));
	}

	template<class Units, class PointLhs, class PointRhs, class = typename std::enable_if<units::traits::is_unit_t<Units>::value>::type>
	bool isSame(const PointLhs& lhs, const PointRhs& rhs, Units tolerance = Units(0))
	{
		static_assert(coord::traits::is_point<PointLhs>::value, "Template parameter `PointLhs` does not satisfy the `point` concept.");
		static_assert(coord::traits::is_point<PointRhs>::value, "Template parameter `PointRhs` does not satisfy the `point` concept.");
		static_assert(coord::traits::is_convertible_point<PointLhs, PointRhs>::value, "No known conversion between types `PointLhs` and `PointRhs`.");

		// if the origins match, perform a direct comparison
		if (lhs.frameData() == rhs.frameData())
		{
			// convert both points to the nearest frame of reference which is Cartesian.
			using LCA = typename coord::traits::least_common_cartesian_ancestor<typename coord::traits::point_traits<PointLhs>::reference_frame, typename coord::traits::point_traits<PointRhs>::reference_frame>::type;
			using namespace units::math;

			auto l = coord::convert<typename coord::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = coord::convert<typename coord::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);

			using namespace units::math;
			return ((abs(x0 - x1) <= tolerance) && (abs(y0 - y1) <= tolerance) && (abs(z0 - z1) <= tolerance));
		}
		// otherwise, convert to ECEF
		else
		{
			using BaseFrame = typename coord::traits::lowest_base_frame<typename coord::traits::point_traits<PointLhs>::reference_frame>::type;

			coord::cartesianTuple l = coord::convert<typename coord::traits::point_traits<PointLhs>::reference_frame, BaseFrame>(lhs.point(), lhs.frameData(), lhs.frameData());
			coord::cartesianTuple r = coord::convert<typename coord::traits::point_traits<PointRhs>::reference_frame, BaseFrame>(rhs.point(), rhs.frameData(), rhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);
			
			using namespace units::math;
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
	* @param[in]	p			point to compare to. May have different units than this point
	* @param[in]	tolerance	acceptable difference between the two points which will still
	* 							compare equal. The tolerance point must have the same type and
	*							units as this instance of the class.
	* @returns		true if the values are equal within the tolerance, false otherwise.
	*/
	template<class PointLhs, class PointRhs, class PointTol>
	bool isSame(const PointLhs& lhs, const PointRhs& rhs, const PointTol& tolerance)
	{
		static_assert(coord::traits::is_point<PointLhs>::value, "Template parameter `PointLhs` does not satisfy the `point` concept.");
		static_assert(coord::traits::is_point<PointRhs>::value, "Template parameter `PointRhs` does not satisfy the `point` concept.");
		static_assert(coord::traits::is_convertible_point<PointLhs, PointRhs>::value, "No known conversion between types `PointLhs` and `PointRhs`.");
		static_assert(coord::traits::is_convertible_point<PointLhs, PointTol>::value, "No known conversion between types `PointTol` and `PointLhs`.");

		// convert both points to the nearest frame of reference which is Cartesian.
		using namespace units::math;

		// if the origins match, perform a direct comparison
		if (lhs.frameData() == rhs.frameData() && rhs.frameData() == tolerance.frameData())
		{
			using LCAtemp = typename coord::traits::least_common_cartesian_ancestor<typename coord::traits::point_traits<PointLhs>::reference_frame, typename coord::traits::point_traits<PointRhs>::reference_frame>::type;
			using LCA = typename coord::traits::least_common_cartesian_ancestor<LCAtemp, typename coord::traits::point_traits<PointTol>::reference_frame>::type;

			auto l = coord::convert<typename coord::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = coord::convert<typename coord::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
			auto t = coord::convert<typename coord::traits::point_traits<PointTol>::reference_frame, LCA>(tolerance.point(), tolerance.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);
			auto xTol = std::get<0>(t);
			auto yTol = std::get<1>(t);
			auto zTol = std::get<2>(t);
			
			using namespace units::math;
			return ((abs(x0 - x1) <= xTol) && (abs(y0 - y1) <= yTol) && (abs(z0 - z1) <= zTol));
		}
		// otherwise, convert to ECEF
		else
		{
			using LCA = typename coord::traits::lowest_base_frame<typename coord::traits::point_traits<PointTol>::reference_frame>::type;

			auto l = coord::convert<typename coord::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = coord::convert<typename coord::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());
			auto t = coord::convert<typename coord::traits::point_traits<PointTol>::reference_frame, LCA>(tolerance.point(), tolerance.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);
			auto xTol = std::get<0>(t);
			auto yTol = std::get<1>(t);
			auto zTol = std::get<2>(t);
			
			using namespace units::math;
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
	template<class PointLhs, class PointRhs, class distance_unit = units::length::meter_t>
	distance_unit distance(const PointLhs& lhs, const PointRhs& rhs)
	{
		static_assert(coord::traits::is_point<PointLhs>::value, "Template parameter `PointLhs` does not satisfy the `point` concept.");
		static_assert(coord::traits::is_point<PointRhs>::value, "Template parameter `PointRhs` does not satisfy the `point` concept.");
		static_assert(coord::traits::is_convertible_point<PointLhs, PointRhs>::value, "No known conversion between types `PointLhs` and `PointRhs`.");

		// if the origins match, perform a direct comparison
		if (lhs.frameData() == rhs.frameData())
		{
			// convert both points to the nearest frame of reference which is Cartesian.
			using LCA = typename coord::traits::least_common_cartesian_ancestor<typename coord::traits::point_traits<PointLhs>::reference_frame, typename coord::traits::point_traits<PointRhs>::reference_frame>::type;
			using namespace units::math;

			auto l = coord::convert<typename coord::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = coord::convert<typename coord::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			distance_unit x0 = std::get<0>(l);
			distance_unit y0 = std::get<1>(l);
			distance_unit z0 = std::get<2>(l);
			distance_unit x1 = std::get<0>(r);
			distance_unit y1 = std::get<1>(r);
			distance_unit z1 = std::get<2>(r);
			
			// 3-D distance formula
			return sqrt(cpow<2>(x1 - x0) + cpow<2>(y1 - y0) + cpow<2>(z1 - z0));
		}
		// otherwise, convert to ECEF
		else
		{
			// convert both points to the nearest frame of reference which is Cartesian.
			using LCA = typename coord::traits::lowest_base_frame<typename coord::traits::point_traits<PointLhs>::reference_frame>::type;
			using namespace units::math;

			auto l = coord::convert<typename coord::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = coord::convert<typename coord::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			distance_unit x0 = std::get<0>(l);
			distance_unit y0 = std::get<1>(l);
			distance_unit z0 = std::get<2>(l);
			distance_unit x1 = std::get<0>(r);
			distance_unit y1 = std::get<1>(r);
			distance_unit z1 = std::get<2>(r);
			
			// 3-D distance formula
			return sqrt(cpow<2>(x1 - x0) + cpow<2>(y1 - y0) + cpow<2>(z1 - z0));
		}
	}

	/**
	 * @brief		Calculates the magnitude of a point/vector
	 * @details		
	 * @param[in]	point	Point to calculate the magnitude of
	 * @returns		magnitude in units of 
	 */
	template<class Point>
	auto magnitude(const Point& point) -> typename std::decay<decltype(std::get<2>(point.point()))>::type
	{
		static_assert(coord::traits::is_point<Point>::value, "Template parameter `Point` does not satisfy the `point` concept.");

		// convert both points to the nearest frame of reference which is Cartesian.
		using LCA = typename coord::traits::least_common_cartesian_ancestor<typename coord::traits::point_traits<Point>::reference_frame, typename coord::traits::point_traits<Point>::reference_frame>::type;
		using namespace units::math;

		auto p = coord::convert<typename coord::traits::point_traits<Point>::reference_frame, LCA>(point.point(), point.frameData());

		auto x = std::get<0>(p);
		auto y = std::get<1>(p);
		auto z = std::get<2>(p);

		// 3-D distance formula
		return sqrt(pow<2>(x) +pow<2>(y) +pow<2>(z));
	}

	/**
	 * @brief		Calculates the dot product between two points.
	 * @details		https://en.wikipedia.org/wiki/Dot_product
	 * @param[in]	lhs	left-hand side point.
	 * @param[in]	rhs right-hand side point.
	 * @returns		dot product of lhs and rhs (wrt to the origin of lhs).
	 */
	template<class PointLhs, class PointRhs>
	auto dotProduct(const PointLhs& lhs, const PointRhs& rhs) -> decltype(units::math::pow<2>(std::get<2>(lhs.point())))
	{
		static_assert(coord::traits::is_point<PointLhs>::value, "Template parameter `PointLhs` does not satisfy the `point` concept.");
		static_assert(coord::traits::is_point<PointRhs>::value, "Template parameter `PointRhs` does not satisfy the `point` concept.");
		static_assert(coord::traits::is_convertible_point<PointLhs, PointRhs>::value, "No known conversion between types `PointLhs` and `PointRhs`.");

		// if the origins match, perform a direct comparison
		if (lhs.frameData() == rhs.frameData())
		{
			// convert both points to the nearest frame of reference which is Cartesian.
			using LCA = typename coord::traits::least_common_cartesian_ancestor<typename coord::traits::point_traits<PointLhs>::reference_frame, typename coord::traits::point_traits<PointRhs>::reference_frame>::type;
			using namespace units::math;

			auto l = coord::convert<typename coord::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = coord::convert<typename coord::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);

			// dot product formula
			return x0*x1 + y0*y1 + z0*z1;
		}
		// otherwise, convert to lhs frame (through an intermediary to ensure the origin is translated)
		else
		{
			using LCA = typename coord::traits::least_common_cartesian_ancestor<typename coord::traits::point_traits<PointLhs>::reference_frame, typename coord::traits::point_traits<PointRhs>::reference_frame>::type;
			using Base = typename coord::traits::lowest_base_frame<typename coord::traits::point_traits<PointLhs>::reference_frame>::type;
			using namespace units::math;

			auto l = coord::convert<typename coord::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto rIntermediate = coord::convert<typename coord::traits::point_traits<PointRhs>::reference_frame, Base>(rhs.point(), rhs.frameData(), rhs.frameData());
			auto r = coord::convert<Base, LCA>(rIntermediate, FrameData(), lhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);

			// dot product formula
			return x0*x1 + y0*y1 + z0*z1;
		}
	}

	/**
	 * @brief		Calculates the cross product of two points
	 * @details		https://en.wikipedia.org/wiki/Cross_product
	 * @param[in]	lhs	left-hand side point.
	 * @param[in]	rhs right-hand side point.
	 * @returns		doct product of lhs and rhs.
	 */
	template<class PointLhs, class PointRhs>
	auto crossProduct(const PointLhs& lhs, const PointRhs& rhs) -> std::tuple<decltype(units::math::pow<2>(std::get<2>(lhs.point()))), decltype(units::math::pow<2>(std::get<2>(lhs.point()))), decltype(units::math::pow<2>(std::get<2>(lhs.point())))>
	{
		static_assert(coord::traits::is_point<PointLhs>::value, "Template parameter `PointLhs` does not satisfy the `point` concept.");
		static_assert(coord::traits::is_point<PointRhs>::value, "Template parameter `PointRhs` does not satisfy the `point` concept.");
		static_assert(coord::traits::is_convertible_point<PointLhs, PointRhs>::value, "No known conversion between types `PointLhs` and `PointRhs`.");

		// convert both points to the nearest frame of reference which is Cartesian.
		using cross_product_type = std::tuple<decltype(units::math::pow<2>(std::get<0>(lhs.point()))), decltype(units::math::pow<2>(std::get<1>(lhs.point()))), decltype(units::math::pow<2>(std::get<2>(lhs.point())))>;
		// if the origins match, perform a direct comparison
		if (lhs.frameData() == rhs.frameData())
		{
			using LCA = typename coord::traits::least_common_cartesian_ancestor<typename coord::traits::point_traits<PointLhs>::reference_frame, typename coord::traits::point_traits<PointRhs>::reference_frame>::type;
			using namespace units::math;

			auto l = coord::convert<typename coord::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = coord::convert<typename coord::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);

			// dot product formula
			return cross_product_type(y0*z1 - z0*y1, z0*x1 - x0*z1, x0*y1 - y0*x1);
		}
		// otherwise, convert to ECEF
		else
		{
			using LCA = typename coord::traits::lowest_base_frame<typename coord::traits::point_traits<PointLhs>::reference_frame>::type;
			using namespace units::math;

			auto l = coord::convert<typename coord::traits::point_traits<PointLhs>::reference_frame, LCA>(lhs.point(), lhs.frameData(), lhs.frameData());
			auto r = coord::convert<typename coord::traits::point_traits<PointRhs>::reference_frame, LCA>(rhs.point(), rhs.frameData(), rhs.frameData());

			auto x0 = std::get<0>(l);
			auto y0 = std::get<1>(l);
			auto z0 = std::get<2>(l);
			auto x1 = std::get<0>(r);
			auto y1 = std::get<1>(r);
			auto z1 = std::get<2>(r);

			// dot product formula
			return cross_product_type(y0*z1 - z0*y1, z0*x1 - x0*z1, x0*y1 - y0*x1);

		}
	}

	namespace topography
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
		static std::vector<std::vector<int8_t>> image(const AbstractTile* tile, units::angle::degree_t resolution)
		{
			if (tile == nullptr)
			{
				return std::vector<std::vector<int8_t>>();
			}

			ThreadPool& pool = ThreadPool::instance();

			int width = ((tile->metadata().northeastLatitude() - tile->metadata().southwestLatitude()) / resolution).to<int>() + 1;
			int height = ((tile->metadata().northeastLongitude() - tile->metadata().southwestLongitude()) / resolution).to<int>() + 1;

			std::atomic<int> min;
			std::atomic<int> max;
			min.store(std::numeric_limits<int>::max());
			max.store(std::numeric_limits<int>::min());

			std::vector<std::vector<int>> elevations;
			elevations.resize(height);
			for (auto itr = elevations.begin(); itr != elevations.end(); ++itr)
			{
				itr->resize(width);
			}

			// lambda for calculating all the elevation values for a line (row) of latitude
			auto calculateRow = [tile, resolution, height, width, &elevations, &min, &max](int row)
			{
				units::angle::degree_t lat(tile->metadata().northeastLatitude() - resolution * row);

				for (int col = 0; col < width; ++col)
				{
					units::angle::degree_t lon = tile->metadata().southwestLongitude() + resolution * col;
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
			for (auto itr = data.begin(); itr != data.end(); ++itr)
			{
				itr->resize(width);
			}

			// synchronize with calculateRow
			for (auto itr = rowDone.begin(); itr != rowDone.end(); ++itr)
			{
				// wait for ALL rows before doing any scaling b/c we need the global min/max to be correct
				itr->get();
			}

			// avoid divide by 0's
			if (max - min == 0)
			{
				return std::vector<std::vector<int8_t>>();
			}

			// function to color-scale a line (row) of latitude
			auto scaleRow = [height, width, &data, &elevations, &min, &max](int row)
			{
				for (int col = 0; col < width; ++col)
				{
					data[row][col] = (int8_t)(255 * (elevations[row][col] - min) / (max - min));
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
		static units::length::meter_t z_factor(units::angle::degree_t latitude)
		{
			using meter_t = units::length::meter_t;

			// See: http://webhelp.esri.com/arcgisdesktop/9.3/index.cfm?TopicName=Applying%20a%20z-factor
			int val = (int)abs(latitude.to<double>());
			if (val >= 0 && val < 10)
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

		static std::vector<std::vector<uint8_t>> hillshade(const AbstractTile* tile, units::angle::degree_t resolution = 0_deg, 
			units::angle::degree_t sunAltitude = 45_deg, units::angle::degree_t sunAzimuth = 315_deg)
		{
			// see: http://edndoc.esri.com/arcobjects/9.2/net/shared/geoprocessing/spatial_analyst_tools/how_hillshade_works.htm
			using degree_t = units::angle::degree_t;
			using radian_t = units::angle::radian_t;
			using meter_t = units::length::meter_t;
			using scalar_t = units::dimensionless::scalar_t;

			if (resolution == 0_deg)
			{
				resolution = tile->metadata().latitudeResolution();
			}

			ThreadPool& pool = ThreadPool::instance();

			int width = ((tile->metadata().northeastLatitude() - tile->metadata().southwestLatitude()) / resolution).to<int>() + 1;
			int height = ((tile->metadata().northeastLongitude() - tile->metadata().southwestLongitude()) / resolution).to<int>() + 1;

			std::vector<std::vector<uint8_t>> shade;
			shade.resize(height);
			for (auto itr = shade.begin(); itr != shade.end(); ++itr)
			{
				itr->resize(width);
			}

			units::angle::radian_t azimuth_r = 360_deg - sunAzimuth + 90_deg;
			units::angle::radian_t zenith_r = 90_deg - sunAltitude;

			// aprrox cellsize
			meter_t cellsize((1.0_nmi / 60) * resolution.convert<units::angle::arcsecond>().to<long double>());

			// lambda for calculating all the elevation values for a line (row) of latitude
			auto calculateRow = [&, tile, cellsize](int row)
			{
				units::angle::degree_t lat(tile->metadata().northeastLatitude() - resolution * row);

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
					units::angle::degree_t lon = tile->metadata().southwestLongitude() + resolution * col;

					meter_t a = tile->elevation(lat + resolution, lon - resolution);
					meter_t b = tile->elevation(lat + resolution, lon);
					meter_t c = tile->elevation(lat + resolution, lon + resolution);
					meter_t d = tile->elevation(lat, lon - resolution);
					meter_t e = tile->elevation(lat, lon);						
					meter_t f = tile->elevation(lat, lon + resolution);			 
					meter_t g = tile->elevation(lat - resolution, lon - resolution);
					meter_t h = tile->elevation(lat - resolution, lon);		
					meter_t i = tile->elevation(lat - resolution, lon + resolution);

					scalar_t dz_dx = ((c + 2*f + i) - (a + 2*d + g)) / (8 * cellsize);
					scalar_t dz_dy = ((g + 2 * h + i) - (a + 2*b + c)) / (8 * cellsize);
					
					scalar_t slope = atan(sqrt(std::pow(dz_dx(), 2) + std::pow(dz_dy(), 2)));
					scalar_t aspect = 0;
					if (dz_dx != 0)
					{
						aspect = atan2(dz_dy(), -1 * dz_dx());
						if (aspect() < 0)
							aspect =  aspect + units::constants::pi * 2;
					}
					else if (dz_dx == 0)
					{
						if (dz_dy > 0)
						{
							aspect = units::constants::pi / 2;
						}
						else if (dz_dy < 0)
						{
							aspect = 2 * units::constants::pi - units::constants::pi / 2;
						}
						else
						{
							aspect = 0;
						}
					}
					
					shade[row][col] = (uint8_t)abs(255.0 * ((cos(zenith_r()) * cos(slope())) + (sin(zenith_r()) * sin(slope()) * cos(azimuth_r() - aspect()))));
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
			for (auto itr = rowDone.begin(); itr != rowDone.end(); ++itr)
			{
				// wait for ALL rows before doing any scaling b/c we need the global min/max to be correct
				itr->get();
			}

			return shade;
		}

	}
}

#endif // algorithm_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: