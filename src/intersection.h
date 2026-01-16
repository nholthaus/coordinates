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

#ifndef intersection_h
#define intersection_h

//------------------------
//	INCLUDES
//------------------------

#include "frameOfReference.h"
#include "point.h"

#include <units.h>

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		Intersection
	//  ----------------------------------------------------------------------------
	///	@brief		Represents an intersection result for a ray or segment test.
	///	@details	This is primarily used for ellipsoid intersection (horizon checks, line-of-sight, etc.).
	///				The primary intersection is represented as an ECEF point (Cartesian tuple). An
	///				optional secondary intersection is intended for terrain/topography refinement.
	///
	///				Because this is a generic result type, conversions to concrete position classes are
	///				provided via the as<PointType>() template, which uses the library's normal conversion
	///				pipeline (coordinates::convert).
	///
	/// @tparam		Datum	3-dimensional datum of the intersection points.
	//  ----------------------------------------------------------------------------
	template<class Datum>
	class Intersection
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//		STATIC ERROR CHECKING
		//////////////////////////////////////////////////////////////////////////

		static_assert(traits::is_datum<Datum>, "`Datum` template parameter does not satisfy the datum concept.");

	public:
		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using datum_type      = Datum;
		using ecef_frame_type = coordinateFrames::ECEFFrame<Datum>;
		using tuple_type      = CartesianTuple;
		using frame_data_type = FrameData;

	private:
		using ecef_point_type = Point<ecef_frame_type, tuple_type, frame_data_type>;

	public:
		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		default constructor
		 * @details		Creates a non-hit intersection.
		 */
		explicit Intersection()
		    : m_hitEllipsoid(false)
		    , m_hitTerrain(false)
		    , m_t(0.0_m)
		    , m_ellipsoid(0.0_m, 0.0_m, 0.0_m)
		    , m_terrain(0.0_m, 0.0_m, 0.0_m)
		    , m_date(Datum::epoch())
		{
		}

		/**
		 * @brief		constructs an ellipsoid hit intersection
		 * @param[in]	t			Distance along the ray direction to the intersection point.
		 * @param[in]	pointECEF	Intersection point in ECEF.
		 * @param[in]	dateOfObservation	Date of observation (decimal years).
		 */
		explicit Intersection(units::length::meters<> t, const tuple_type& pointECEF, years<> dateOfObservation = Datum::epoch())
		    : m_hitEllipsoid(true)
		    , m_hitTerrain(false)
		    , m_t(t)
		    , m_ellipsoid(pointECEF)
		    , m_terrain(0.0_m, 0.0_m, 0.0_m)
		    , m_date(dateOfObservation)
		{
		}

	public:
		//////////////////////////////////////////////////////////////////////////
		//		STATE
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		Returns whether any intersection occurred.
		 * @return		true if ellipsoid hit occurred, false otherwise.
		 */
		bool hit() const
		{
			return m_hitEllipsoid;
		}

		/**
		 * @brief		Returns whether an ellipsoid intersection occurred.
		 * @return		true if ellipsoid hit, false otherwise.
		 */
		bool hitEllipsoid() const
		{
			return m_hitEllipsoid;
		}

		/**
		 * @brief		Returns whether a terrain intersection is available.
		 * @details		Terrain intersection is optional and can be set by calling setTerrainHit().
		 * @return		true if terrain hit is available, false otherwise.
		 */
		bool hitTerrain() const
		{
			return m_hitTerrain;
		}

		/**
		 * @brief		Returns the distance along the ray to the ellipsoid intersection.
		 * @return		Distance along the ray to the ellipsoid intersection point.
		 */
		units::length::meters<> t() const
		{
			return m_t;
		}

		/**
		 * @brief		Returns the observation date.
		 * @return		Date in decimal years.
		 */
		years<> date() const
		{
			return m_date;
		}

	public:
		//////////////////////////////////////////////////////////////////////////
		//		RAW POINT GETTERS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		Returns the ellipsoid intersection point as an ECEF tuple.
		 * @return		ECEF Cartesian tuple in meters.
		 */
		const tuple_type& ellipsoidECEF() const
		{
			return m_ellipsoid;
		}

		/**
		 * @brief		Sets an optional terrain intersection point.
		 * @details		This allows higher-level algorithms to refine the ellipsoid hit to a terrain hit.
		 * @param[in]	pointECEF	Terrain intersection point in ECEF.
		 */
		void setTerrainHit(const tuple_type& pointECEF)
		{
			m_hitTerrain = true;
			m_terrain    = pointECEF;
		}

		/**
		 * @brief		Returns the terrain intersection point as an ECEF tuple.
		 * @details		Only valid if hitTerrain() is true.
		 * @return		ECEF Cartesian tuple in meters.
		 */
		const tuple_type& terrainECEF() const
		{
			return m_terrain;
		}

	public:
		//////////////////////////////////////////////////////////////////////////
		//		CONVENIENCE CONVERSIONS
		//////////////////////////////////////////////////////////////////////////

		/**
		 * @brief		Converts the ellipsoid intersection to an arbitrary point type.
		 * @details		The destination's frameData controls origin/date behavior for local frames.
		 * @tparam		PointType	Destination point type.
		 * @param[in]	frameDataTo	Destination frame data (origin, date, orientation).
		 * @return		Intersection point converted to <i>PointType</i>.
		 */
		template<class PointType>
		PointType as(const frame_data_type& frameDataTo) const
		{
			static_assert(coordinates::is_point<PointType>, "Template parameter `PointType` does not satisfy the `point` concept.");
			static_assert(coordinates::traits::is_convertible_point<ecef_point_type, PointType>, "No known conversion between ECEF and the requested point type.");

			ecef_point_type src;
			src.setPoint(m_ellipsoid);
			src.setFrameData(frame_data_type(frameDataTo.origin, frameDataTo.orientation, m_date));

			PointType dst;
			dst.setFrameData(frameDataTo);

			coordinates::convert(src, dst);

			return dst;
		}

	private:
		bool m_hitEllipsoid;
		bool m_hitTerrain;

		units::length::meters<> m_t;
		tuple_type m_ellipsoid;
		tuple_type m_terrain;
		years<> m_date;
	};
}

#endif
