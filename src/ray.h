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
//
// A geometric ray: an origin position plus a direction, both expressed in the same coordinate frame. It is
// the natural input to a terrain/ellipsoid intersection query and to a sensor line-of-sight test: a sensor
// aims a ray from its position along a boresight, and the ray is marched until it meets terrain. A `Ray` pairs
// a `Coordinate<Frame,...>` origin with a `Vector<Frame>` direction, and offers `pointAt(range)` (the point a
// given distance along the ray) plus factories that build the ray from the two natural sources of a pointing:
// a `Pose` with a body-axis direction (a slewing sensor's boresight rotated into the parent frame), and a
// local azimuth/elevation look-angle at a geodetic origin (a level or depressed beam). The direction is stored
// normalized; scaling it does not change the ray.
//
//--------------------------------------------------------------------------------------------------

#ifndef ray_h
#define ray_h

//------------------------
//	INCLUDES
//------------------------

#include <cmath>
#include <tuple>

#include <units.h>

#include "coordinate.h"
#include "frameOfReference.h"
#include "pose.h"
#include "vector.h"

inline namespace coordinates
{
	using namespace units;
	using namespace units::literals;

	//	----------------------------------------------------------------------------
	//	CLASS		Ray
	//  ----------------------------------------------------------------------------
	///	@brief		A directed half-line: an origin position and a normalized direction in one coordinate frame.
	///	@details	Pairs a `Coordinate<Frame,...>` origin with a `Vector<Frame>` direction. The direction is kept
	///				normalized, so `pointAt(range)` walks a metric distance along the ray. A `Ray` is the input a
	///				terrain/ellipsoid intersector marches, and the geometric object a sensor aims: the boresight
	///				factory carries a fixed body-axis direction through a `Pose` into the parent frame, and the
	///				look-angle factory builds a ray from an azimuth/elevation at a local origin.
	///	@tparam		Frame	the coordinate frame the origin and direction are expressed in (e.g. an ECEF frame).
	//  ----------------------------------------------------------------------------
	template<class Frame>
	class Ray
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using reference_frame = Frame;
		using origin_type     = Coordinate<Frame, CartesianTuple>;
		using direction_type  = Vector<Frame>;

		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		constexpr Ray() = default;

		/// From an origin and a direction; the direction is normalized on construction.
		constexpr Ray(const origin_type& origin, const direction_type& direction)
		    : m_origin(origin)
		    , m_direction(normalized(direction))
		{
		}

		//////////////////////////////////////////////////////////////////////////
		//		GETTERS
		//////////////////////////////////////////////////////////////////////////

		[[nodiscard]] constexpr const origin_type&    origin() const { return m_origin; }       ///< the ray's start point
		[[nodiscard]] constexpr const direction_type& direction() const { return m_direction; } ///< the ray's unit direction

		//////////////////////////////////////////////////////////////////////////
		//		OPERATIONS
		//////////////////////////////////////////////////////////////////////////

		/// The point a metric distance `range` along the ray from the origin.
		[[nodiscard]] constexpr origin_type pointAt(meters<> range) const
		{
			const auto d = m_direction.vector();
			origin_type p;
			p.setFrameData(m_origin.frameData());
			p.setPoint(std::get<0>(m_origin.point()) + std::get<0>(d) * range.value(),
			           std::get<1>(m_origin.point()) + std::get<1>(d) * range.value(),
			           std::get<2>(m_origin.point()) + std::get<2>(d) * range.value());
			return p;
		}

		//////////////////////////////////////////////////////////////////////////
		//		FACTORIES
		//////////////////////////////////////////////////////////////////////////

		/// A ray from a sensor pose along a body-axis direction: the body direction is rotated into the parent
		/// frame by the pose, and the ray starts at the pose's translation. The parent frame IS `Frame`, so a
		/// pose whose translation is in `Frame` (e.g. an aircraft-in-ECEF pose) yields a `Ray<Frame>`.
		[[nodiscard]] static constexpr Ray fromPose(const Pose& pose, const CartesianTuple& bodyDirection)
		{
			origin_type origin;
			origin.setPoint(pose.translation());
			const CartesianTuple worldDirection = pose.rotateDirection(bodyDirection);
			direction_type       dir(std::get<0>(worldDirection), std::get<1>(worldDirection), std::get<2>(worldDirection));
			return Ray(origin, dir);
		}

		/// A ray from an azimuth/elevation look-angle at a geodetic origin: azimuth (0=North, +East) and
		/// elevation (0=level, +up) define a direction in the origin's local ENU tangent frame, which is rotated
		/// into `Frame`. This is the natural "aim a beam at a bearing and elevation from a site" constructor -- a
		/// level scan is `elevation == 0`. `Frame` must be the ECEF frame the origin's datum lives in (see `RayECEF`).
		template<class OriginPoint>
		[[nodiscard]] static Ray fromAzimuthElevation(const OriginPoint& originPoint, degrees<> azimuth, degrees<> elevation)
		{
			// The local ENU direction of the look-angle: east = cos(el) sin(az), north = cos(el) cos(az),
			// up = sin(el). Anchor it to the origin as an ENU vector, then convert to `Frame` by the rotation-
			// only vector frame change -- the frame graph rotates the local direction into ECEF.
			const double az = radians<>(azimuth).value();
			const double el = radians<>(elevation).value();
			const double ce = std::cos(el);
			using EnuVector = Vector<coordinateFrames::ENUFrame<typename OriginPoint::reference_frame::datum_type>>;
			EnuVector localDirection(meters<>(ce * std::sin(az)),    // east
			                         meters<>(ce * std::cos(az)),    // north
			                         meters<>(std::sin(el)),         // up
			                         originPoint);
			direction_type worldDirection(localDirection);    // rotation-only frame change into Frame

			origin_type origin;
			origin.setPoint(Coordinate<Frame, CartesianTuple>(originPoint).point());
			return Ray(origin, worldDirection);
		}

		//////////////////////////////////////////////////////////////////////////
		//		OPERATORS
		//////////////////////////////////////////////////////////////////////////

		/// Streams as `origin -> (dx, dy, dz)`.
		friend std::ostream& operator<<(std::ostream& os, const Ray& r)
		{
			const auto d = r.m_direction.vector();
			return os << "(" << std::get<0>(r.m_origin.point()) << ", " << std::get<1>(r.m_origin.point()) << ", "
			          << std::get<2>(r.m_origin.point()) << ") -> (" << std::get<0>(d) << ", " << std::get<1>(d) << ", "
			          << std::get<2>(d) << ")";
		}

	private:
		//	----------------------------------------------------------------------------
		//	FUNCTION: normalized [static, private]
		//  ----------------------------------------------------------------------------
		///	@brief		Return the direction scaled to unit length (a zero vector is returned unchanged).
		///	@param[in]	direction	the direction to normalize.
		///	@return		the unit-length direction, sharing the input's frame data.
		//  ----------------------------------------------------------------------------
		static constexpr direction_type normalized(const direction_type& direction)
		{
			const auto   v   = direction.vector();
			const double dx  = std::get<0>(v).value(), dy = std::get<1>(v).value(), dz = std::get<2>(v).value();
			const double len = units::sqrt(dx * dx + dy * dy + dz * dz);
			if (len == 0.0)
				return direction;
			direction_type unit(meters<>(dx / len), meters<>(dy / len), meters<>(dz / len));
			unit.setFrameData(direction.frameData());
			return unit;
		}

		origin_type    m_origin;       ///< the ray's start point in `Frame`
		direction_type m_direction;    ///< the ray's unit direction in `Frame`
	};

	//======================================================
	//	FREE FUNCTIONS
	//======================================================

	//	----------------------------------------------------------------------------
	//	FUNCTION: ray [free]
	//  ----------------------------------------------------------------------------
	///	@brief		A ray FROM a site at a local azimuth/elevation look-angle -- the point emits the ray.
	///	@details	The natural "aim a beam from this site at a bearing and elevation" call: the site is the
	///				origin (nothing re-passed), azimuth is 0=North/+East and elevation 0=level/+up in the site's
	///				local tangent frame. A level scan is `elevation == 0`. Returns a ray in the site's ECEF frame.
	///	@tparam		OriginPoint	a geodetic/ECEF position type (the ray's origin).
	///	@param[in]	site		the position the ray emanates from.
	///	@param[in]	azimuth		the look-angle azimuth (0=North, +East).
	///	@param[in]	elevation	the look-angle elevation (0=level, +up).
	///	@return		the ray from `site` along the look-angle, in the site's ECEF frame.
	//  ----------------------------------------------------------------------------
	template<class OriginPoint>
	[[nodiscard]] auto ray(const OriginPoint& site, degrees<> azimuth, degrees<> elevation)
	{
		using Frame = coordinateFrames::ECEFFrame<typename OriginPoint::reference_frame::datum_type>;
		return Ray<Frame>::fromAzimuthElevation(site, azimuth, elevation);
	}

	//======================================================
	//	ALIASES
	//======================================================

	/// A ray in a datum's Earth-centred Earth-fixed frame -- the frame terrain/ellipsoid intersection runs in.
	template<class Datum>
	using RayECEF = Ray<coordinateFrames::ECEFFrame<typename traits::datum_traits<Datum>::horizontal_datum>>;
}    // namespace coordinates

#endif    // ray_h
