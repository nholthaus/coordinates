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
// The single coordinate value template. `Coordinate<Frame, Tuple>` holds -- once -- the behaviour that the
// five position classes (ECEF, geodetic, ENU, NED, AER) previously copy-pasted: it stores the frame's tuple
// plus its frame data, exposes the point/frame-data interface the conversion machinery needs, and provides one
// converting constructor that turns any convertible point into this frame. What differs per frame -- the axis
// accessor names and return types, and whether the frame is a local (origin-carrying) one -- comes from
// `frame_axes<Frame>`, so this body never mentions x/latitude/azimuth. The friendly names (ECEF, LLA, ...) are
// alias templates of this. Storage IS the tuple (not named scalars), which is what makes the body uniform
// across frames whose components are heterogeneous (a geodetic point is angle/angle/length, an AER point is
// angle/angle/range).
//
//--------------------------------------------------------------------------------------------------

#ifndef coordinate_h
#define coordinate_h

//------------------------
//	INCLUDES
//------------------------

#include <cassert>
#include <ostream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include <units.h>

#include "algorithm.h"
#include "axisAccessors.h"
#include "frameAxes.h"
#include "frameOfReference.h"
#include "point.h"

inline namespace coordinates
{
	//	----------------------------------------------------------------------------
	//	CLASS		Coordinate
	//  ----------------------------------------------------------------------------
	///	@brief		A position in a frame of reference: the one template every position type is an alias of.
	///	@details	Stores the frame's `Tuple` value plus its `FrameData`. Converting from any other point runs
	///				the shared conversion machinery, branching on whether `Frame` is a local (origin-carrying)
	///				frame. Named axis accessors live in the frame-constrained free functions that read
	///				`frame_axes<Frame>`, not here.
	///	@tparam		Frame		the frame of reference this coordinate is expressed in.
	///	@tparam		Tuple		the frame's tuple type (e.g. `CartesianTuple`, `SphericalTuple`).
	///	@tparam		FrameDataType	the ancillary frame data type (defaults to `FrameData`).
	//  ----------------------------------------------------------------------------
	template<class Frame, class Tuple, class FrameDataType>
	class Coordinate : public traits::AxisAccessors<Frame, Coordinate<Frame, Tuple, FrameDataType>, Tuple>
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//		PUBLIC TYPES
		//////////////////////////////////////////////////////////////////////////

		using reference_frame = Frame;
		using tuple_type      = Tuple;
		using frame_data_type = FrameDataType;
		using datum_type      = typename traits::frame_traits<Frame>::datum_type;

		//////////////////////////////////////////////////////////////////////////
		//		CONSTRUCTORS
		//////////////////////////////////////////////////////////////////////////

		/// Default: the zero point at the datum epoch.
		Coordinate()
		    : m_point{}
		    , m_frameData(datum_type::epoch())
		{
		}

		/// From the axis components (forwarded into the tuple) at the datum epoch, or from the axis components
		/// plus a trailing observation date. Accepts exactly `tuple_size` component values, or `tuple_size`
		/// values followed by a `years<>` date; the date-carrying case splits the last argument off the pack.
		/// The first argument must itself be a component (convertible to slot 0), so a `(wholePoint, origin,
		/// date)` local-frame call is NOT mistaken for three components and instead resolves to the tuple-origin
		/// constructor.
		template<class First, class... Rest>
		    requires((sizeof...(Rest) + 1 == std::tuple_size_v<Tuple>
		              || (sizeof...(Rest) + 1 == std::tuple_size_v<Tuple> + 1
		                  && std::same_as<std::remove_cvref_t<std::tuple_element_t<sizeof...(Rest), std::tuple<First, Rest...>>>, years<>>))
		             && std::convertible_to<First, std::tuple_element_t<0, Tuple>>)
		explicit Coordinate(First first, Rest... rest)
		{
			auto packed = std::forward_as_tuple(std::move(first), std::move(rest)...);
			if constexpr (sizeof...(Rest) + 1 == std::tuple_size_v<Tuple>)
			{
				m_point     = std::make_from_tuple<Tuple>(std::move(packed));
				m_frameData = frame_data_type(datum_type::epoch());
			}
			else
			{
				m_point     = [&]<std::size_t... I>(std::index_sequence<I...>) { return Tuple(std::get<I>(std::move(packed))...); }(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
				m_frameData = frame_data_type(std::get<std::tuple_size_v<Tuple>>(std::move(packed)));
			}
		}

		/// From a tuple value at an optional observation date.
		explicit Coordinate(const Tuple& point, years<> dateOfObservation = datum_type::epoch())
		    : m_point(point)
		    , m_frameData(dateOfObservation)
		{
		}

		//======================================================
		//	LOCAL-FRAME CONSTRUCTORS (origin-carrying: ENU/NED/AER)
		//======================================================
		//	A local (tangent) frame's coordinate carries the geodetic origin its axes are pinned to, stored in
		//	`m_frameData.origin`. These constructors set both the local components and that origin; they exist
		//	only for local frames (`is_local_frame<Frame>`), so a global frame never gains an origin argument.

		/// The stored type of axis `I` (from the frame's `Tuple`, which is complete here).
		template<std::size_t I>
		using slot_type = std::tuple_element_t<I, Tuple>;

		/// The geodetic origin type a local frame is pinned to.
		using local_origin_type = PositionGeodetic<datum_type>;

		/// Local components plus the origin's geodetic (latitude, longitude, altitude) and an optional date.
		/// The components are the frame's three axis slot types, so the trailing arguments are ordinary,
		/// fully-deducible parameters (no pack in a non-deduced position).
		template<class Frame_ = Frame>
		    requires(traits::is_local_frame<Frame_>)
		Coordinate(slot_type<0> c0, slot_type<1> c1, slot_type<2> c2, degrees<> latitude0, degrees<> longitude0, meters<> altitude0, years<> dateOfObservation = datum_type::epoch())
		    : m_point(c0, c1, c2)
		    , m_frameData(SphericalTuple(latitude0, longitude0, altitude0), dateOfObservation)
		{
		}

		/// Local components plus the origin (any point convertible to the geodetic origin) and an optional date.
		template<traits::is_point OriginPoint>
		    requires(traits::is_local_frame<Frame>)
		Coordinate(slot_type<0> c0, slot_type<1> c1, slot_type<2> c2, const OriginPoint& origin, years<> dateOfObservation = datum_type::epoch())
		    : m_point(c0, c1, c2)
		    , m_frameData(local_origin_type(origin).point(), dateOfObservation)
		{
		}

		/// A local coordinate tuple plus the origin (any point convertible to the geodetic origin) and a date.
		template<traits::is_point OriginPoint>
		    requires(traits::is_local_frame<Frame>)
		Coordinate(const Tuple& point, const OriginPoint& origin, years<> dateOfObservation = datum_type::epoch())
		    : m_point(point)
		    , m_frameData(local_origin_type(origin).point(), dateOfObservation)
		{
		}

		/// A source point converted into this local frame, anchored at the given origin, at a given date. The
		/// origin and date are those the caller supplied here (this coordinate's frame), NOT the source's:
		/// re-projecting `point` into this frame leaves the anchor/date exactly as constructed.
		template<traits::is_point SourcePoint, traits::is_point OriginPoint>
		    requires(traits::is_local_frame<Frame>)
		Coordinate(const SourcePoint& point, const OriginPoint& origin, years<> dateOfObservation = datum_type::epoch())
		    : m_point{}
		    , m_frameData(local_origin_type(origin).point(), dateOfObservation)
		{
			assignReprojecting(point);
			// Re-projection may adopt the source's date; the caller pinned origin + date here, so restore them.
			m_frameData = frame_data_type(local_origin_type(origin).point(), dateOfObservation);
		}

		/// The zero local coordinate at a given geodetic origin and an optional date. Takes exactly the
		/// geodetic origin type (not a template) so a single geodetic argument resolves here as an exact match,
		/// while a single argument of any OTHER point type resolves to the converting constructor.
		template<class Frame_ = Frame>
		    requires(traits::is_local_frame<Frame_>)
		explicit Coordinate(const local_origin_type& origin, years<> dateOfObservation = datum_type::epoch())
		    : m_point{}
		    , m_frameData(origin.point(), dateOfObservation)
		{
		}

		Coordinate(const Coordinate&) = default;
		Coordinate(Coordinate&&)      = default;
		~Coordinate()                 = default;

		/// Copy assignment. A global frame copies members directly. A local (origin-carrying) frame preserves
		/// its OWN origin: if the source shares it, this is a plain copy; if it differs, the source is
		/// re-projected into this coordinate's frame (so `enuAtBoston = enuAtLexington` translates rather than
		/// silently adopting Lexington). Same behavior the per-class bodies had before the collapse.
		Coordinate& operator=(const Coordinate& other)
		{
			if (this == &other)
				return *this;
			if constexpr (traits::is_local_frame<Frame>)
			{
				if (m_frameData.origin == NULL_ORIGIN || m_frameData.origin == other.m_frameData.origin)
				{
					m_point     = other.m_point;
					m_frameData = other.m_frameData;
					return *this;
				}
				return assignReprojecting(other);
			}
			else
			{
				m_point     = other.m_point;
				m_frameData = other.m_frameData;
				return *this;
			}
		}

		Coordinate& operator=(Coordinate&& other) noexcept
		{
			*this = static_cast<const Coordinate&>(other);
			return *this;
		}

		/// Converting constructor: build this frame's coordinate from any other point, running the conversion
		/// pipeline. Delegates to the converting assignment so the two share one implementation.
		template<class P>
		    requires(traits::is_point<P> && !std::same_as<std::remove_cvref_t<P>, Coordinate>)
		Coordinate(const P& point)
		    : Coordinate()
		{
			*this = point;
		}

		/// Converting assignment from any other point type: the single conversion implementation.
		template<class P>
		    requires(traits::is_point<P> && !std::same_as<std::remove_cvref_t<P>, Coordinate>)
		Coordinate& operator=(const P& point)
		{
			return assignReprojecting(point);
		}

	private:
		/// The one conversion implementation, shared by the converting assignment and the local-frame copy
		/// assignment. A global frame (ECEF, geodetic) converts in one hop. A local (origin-carrying) frame
		/// adopts the source's origin only if it has none of its own, then takes a closed-form shortcut when
		/// `convert_fast_path` provides one for the source/destination frame pair (e.g. the NED<->ENU axis
		/// swap), else routes through an ECEF intermediate at this coordinate's origin.
		template<class P>
		    requires(traits::is_point<P>)
		Coordinate& assignReprojecting(const P& point)
		{
			if constexpr (traits::is_local_frame<Frame>)
			{
				// Adopt the source's origin only if this coordinate has no origin of its own yet.
				if (m_frameData.origin == NULL_ORIGIN)
					m_frameData = point.frameData();
				else
					m_frameData.date = point.frameData().date;

				using SourceFrame = typename traits::point_traits<P>::reference_frame;
				using FastPath    = traits::convert_fast_path<SourceFrame, Frame>;
				if constexpr (FastPath::has_shortcut)
				{
					if (point.frameData().origin == m_frameData.origin)
					{
						m_point = FastPath::apply(point.point());
						return *this;
					}
				}

				// General path: convert through an ECEF intermediate at this coordinate's date. A local frame's
				// datum_type is its horizontal datum, so the intermediate is `ECEFFrame<datum_type>`.
				using ecef_type = Coordinate<coordinateFrames::ECEFFrame<datum_type>, CartesianTuple>;
				ecef_type intermediate;
				intermediate.setFrameData(frame_data_type(m_frameData.date));
				coordinates::convert(point, intermediate);
				coordinates::convert(intermediate, *this);
			}
			else
			{
				m_frameData = point.frameData();
				coordinates::convert(point, *this);
			}
			return *this;
		}

		/// The components of `p` expressed in THIS coordinate's frame. For a local frame with a different
		/// origin, `p` is re-projected into this coordinate's origin first; otherwise its tuple is returned
		/// as-is. Used by the same-frame `operator+=`/`-=` so anchored-point arithmetic is origin-correct.
		template<class P>
		    requires(traits::is_point<P>)
		[[nodiscard]] Tuple sameFrameComponents(const P& p) const
		{
			if constexpr (traits::is_local_frame<Frame>)
			{
				if (p.frameData().origin == m_frameData.origin)
					return Tuple(p.point());
				Coordinate reprojected;
				reprojected.setFrameData(m_frameData);
				reprojected.assignReprojecting(p);
				return reprojected.point();
			}
			else
			{
				return Tuple(p.point());
			}
		}

	public:

		//////////////////////////////////////////////////////////////////////////
		//		POINT INTERFACE
		//////////////////////////////////////////////////////////////////////////

		[[nodiscard]] tuple_type      point() const { return m_point; }
		[[nodiscard]] frame_data_type frameData() const { return m_frameData; }

		void setPoint(const tuple_type& point) { m_point = point; }
		void setPoint(tuple_type&& point) { m_point = std::move(point); }

		/// Set the point from its individual axis components, converting each into the tuple's stored unit.
		template<class... Components>
		    requires(sizeof...(Components) == std::tuple_size_v<Tuple>)
		void setPoint(Components... components) { m_point = Tuple(std::move(components)...); }

		void setFrameData(const frame_data_type& frameData) { m_frameData = frameData; }
		void setFrameData(frame_data_type&& frameData) { m_frameData = std::move(frameData); }

		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS (frame-agnostic)
		//////////////////////////////////////////////////////////////////////////

		/// Date of observation (relevant only when converting between datums).
		[[nodiscard]] years<> date() const { return m_frameData.date; }

		/// The geodetic origin this local (tangent) frame's axes are pinned to.
		template<class Frame_ = Frame>
		    requires(traits::is_local_frame<Frame_>)
		[[nodiscard]] local_origin_type origin() const { return local_origin_type(m_frameData.origin, m_frameData.date); }

		/// Move the origin while preserving this point's absolute location (round-trips through ECEF). To move
		/// the origin WITHOUT altering the local components (a moving platform), set the frame data directly.
		template<class Frame_ = Frame>
		    requires(traits::is_local_frame<Frame_>)
		void setOrigin(const local_origin_type& origin)
		{
			using ecef_type = Coordinate<coordinateFrames::ECEFFrame<datum_type>, CartesianTuple>;
			ecef_type intermediate;
			intermediate.setFrameData(m_frameData);
			coordinates::convert(*this, intermediate);
			m_frameData.origin = origin.point();
			coordinates::convert(intermediate, *this);
		}

		//////////////////////////////////////////////////////////////////////////
		//		QUERIES / MEASUREMENTS (shared forwarders over the free-function engine)
		//////////////////////////////////////////////////////////////////////////

		/// True if this is the zero value of its representation.
		[[nodiscard]] bool isNull() const { return coordinates::isNull(*this); }

		/// Whether another point is the same as this one exactly (zero tolerance).
		template<traits::is_point P>
		[[nodiscard]] bool isSame(const P& p) const
		{ return coordinates::isSame(*this, p, units::length::meters<>{0.0}); }

		/// Whether another point equals this one within a scalar (length) or per-axis point tolerance.
		template<traits::is_point P, class Tol>
		[[nodiscard]] bool isSame(const P& p, const Tol& tolerance) const
		{ return coordinates::isSame(*this, p, tolerance); }

		/// Straight-line (Euclidean) distance from this point to another.
		template<traits::is_point P>
		[[nodiscard]] ranges::Euclidean distance(const P& p) const
		{ return ranges::Euclidean(coordinates::distance(*this, p)); }

		/// Magnitude of this point as a vector from its origin.
		[[nodiscard]] ranges::Euclidean magnitude() const
		{ return ranges::Euclidean(coordinates::magnitude(*this)); }

		/// Dot product with another point (an area).
		template<traits::is_point P>
		[[nodiscard]] auto dotProduct(const P& p) const { return coordinates::dotProduct(*this, p); }

		//////////////////////////////////////////////////////////////////////////
		//		ARITHMETIC (Cartesian frames)
		//////////////////////////////////////////////////////////////////////////

		/// Add a vector's components to this point.
		template<class Vector>
		    requires(traits::is_vector<Vector> && requires(const Vector& v) { v.x(); v.y(); v.z(); })
		Coordinate& operator+=(const Vector& v)
		{
			std::get<0>(m_point) = std::get<0>(m_point) + v.x();
			std::get<1>(m_point) = std::get<1>(m_point) + v.y();
			std::get<2>(m_point) = std::get<2>(m_point) + v.z();
			return *this;
		}

		/// Subtract a vector's components from this point.
		template<class Vector>
		    requires(traits::is_vector<Vector> && requires(const Vector& v) { v.x(); v.y(); v.z(); })
		Coordinate& operator-=(const Vector& v)
		{
			std::get<0>(m_point) = std::get<0>(m_point) - v.x();
			std::get<1>(m_point) = std::get<1>(m_point) - v.y();
			std::get<2>(m_point) = std::get<2>(m_point) - v.z();
			return *this;
		}

		/// Add a same-frame anchored (local) vector's components to this point, requiring the vector share
		/// this point's frame data (origin/date). A frame mismatch is a logic error: it asserts and throws.
		template<class Vector>
		    requires(traits::is_local_frame<Frame> && traits::is_vector<Vector>
		             && std::same_as<typename traits::point_traits<Vector>::reference_frame, Frame>
		             && requires(const Vector& v) { v.vector(); v.frameData(); })
		Coordinate& operator+=(const Vector& v)
		{
			assert(m_frameData == v.frameData());
			if (m_frameData != v.frameData())
				throw std::logic_error("Coordinate frame mismatch in operator+= (anchored vector)");
			const auto& o        = v.vector();
			std::get<0>(m_point) = std::get<0>(m_point) + std::get<0>(o);
			std::get<1>(m_point) = std::get<1>(m_point) + std::get<1>(o);
			std::get<2>(m_point) = std::get<2>(m_point) + std::get<2>(o);
			return *this;
		}

		/// Subtract a same-frame anchored (local) vector's components from this point, requiring the vector
		/// share this point's frame data. A frame mismatch is a logic error: it asserts and throws.
		template<class Vector>
		    requires(traits::is_local_frame<Frame> && traits::is_vector<Vector>
		             && std::same_as<typename traits::point_traits<Vector>::reference_frame, Frame>
		             && requires(const Vector& v) { v.vector(); v.frameData(); })
		Coordinate& operator-=(const Vector& v)
		{
			assert(m_frameData == v.frameData());
			if (m_frameData != v.frameData())
				throw std::logic_error("Coordinate frame mismatch in operator-= (anchored vector)");
			const auto& o        = v.vector();
			std::get<0>(m_point) = std::get<0>(m_point) - std::get<0>(o);
			std::get<1>(m_point) = std::get<1>(m_point) - std::get<1>(o);
			std::get<2>(m_point) = std::get<2>(m_point) - std::get<2>(o);
			return *this;
		}

		/// Add another same-frame point's components. For a local (origin-carrying) frame the other point is
		/// first re-projected into THIS coordinate's origin (so adding a point anchored elsewhere is
		/// meaningful), then its components are added; for a global frame the components add directly.
		template<traits::is_point P>
		    requires(std::same_as<typename traits::point_traits<P>::reference_frame, Frame> && !traits::is_vector<P>)
		Coordinate& operator+=(const P& p)
		{
			const auto o = sameFrameComponents(p);
			std::get<0>(m_point) = std::get<0>(m_point) + std::get<0>(o);
			std::get<1>(m_point) = std::get<1>(m_point) + std::get<1>(o);
			std::get<2>(m_point) = std::get<2>(m_point) + std::get<2>(o);
			return *this;
		}

		/// Subtract another same-frame point's components (re-projected into this origin for a local frame).
		template<traits::is_point P>
		    requires(std::same_as<typename traits::point_traits<P>::reference_frame, Frame> && !traits::is_vector<P>)
		Coordinate& operator-=(const P& p)
		{
			const auto o = sameFrameComponents(p);
			std::get<0>(m_point) = std::get<0>(m_point) - std::get<0>(o);
			std::get<1>(m_point) = std::get<1>(m_point) - std::get<1>(o);
			std::get<2>(m_point) = std::get<2>(m_point) - std::get<2>(o);
			return *this;
		}

		/// Scale each component.
		Coordinate& operator*=(units::dimensionless<> factor)
		{
			std::get<0>(m_point) = std::get<0>(m_point) * factor;
			std::get<1>(m_point) = std::get<1>(m_point) * factor;
			std::get<2>(m_point) = std::get<2>(m_point) * factor;
			return *this;
		}

		/// Divide each component.
		Coordinate& operator/=(units::dimensionless<> divisor)
		{
			std::get<0>(m_point) = std::get<0>(m_point) / divisor;
			std::get<1>(m_point) = std::get<1>(m_point) / divisor;
			std::get<2>(m_point) = std::get<2>(m_point) / divisor;
			return *this;
		}

		/// Stream as `(c0, c1, c2)`, appending ` @ (lat, lon, alt)` for a local (origin-carrying) frame.
		friend std::ostream& operator<<(std::ostream& os, const Coordinate& c)
		{
			os << "(" << std::get<0>(c.m_point) << ", " << std::get<1>(c.m_point) << ", " << std::get<2>(c.m_point) << ")";
			if constexpr (traits::is_local_frame<Frame>)
				os << " @ (" << std::get<0>(c.m_frameData.origin) << ", " << std::get<1>(c.m_frameData.origin) << ", "
				   << std::get<2>(c.m_frameData.origin) << ")";
			return os;
		}

	private:
		Tuple           m_point;        ///< the frame's tuple value (the storage, uniform across frames)
		frame_data_type m_frameData;    ///< ancillary frame data (origin, orientation, date)
	};
}    // namespace coordinates

#endif    // coordinate_h
