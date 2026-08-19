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

#include <ostream>
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
	template<class Frame, class Tuple, class FrameDataType = FrameData>
	class Coordinate : public Point<Frame, Tuple, FrameDataType>,
	                   public traits::AxisAccessors<Frame, Coordinate<Frame, Tuple, FrameDataType>, Tuple>
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

		/// From the three axis components (forwarded into the tuple) at an optional observation date.
		template<class... Components>
		    requires(sizeof...(Components) == std::tuple_size_v<Tuple>)
		explicit Coordinate(Components... components)
		    : m_point(std::move(components)...)
		    , m_frameData(datum_type::epoch())
		{
		}

		/// From a tuple value at an optional observation date.
		explicit Coordinate(const Tuple& point, years<> dateOfObservation = datum_type::epoch())
		    : m_point(point)
		    , m_frameData(dateOfObservation)
		{
		}

		Coordinate(const Coordinate&)            = default;
		Coordinate(Coordinate&&)                 = default;
		Coordinate& operator=(const Coordinate&) = default;
		Coordinate& operator=(Coordinate&&)      = default;
		~Coordinate() override                   = default;

		/// Converting constructor: build this frame's coordinate from any other point, running the conversion
		/// pipeline. Delegates to the converting assignment so the two share one implementation.
		template<class P>
		    requires(traits::is_point<P> && !std::same_as<std::remove_cvref_t<P>, Coordinate>)
		Coordinate(const P& point)
		    : Coordinate()
		{
			*this = point;
		}

		/// Converting assignment: the single conversion implementation. A global frame (ECEF, geodetic)
		/// converts in one hop. A local (origin-carrying) frame adopts the source's origin, then takes a
		/// closed-form shortcut when `convert_fast_path` provides one for the source/destination pair (e.g.
		/// the NED<->ENU axis swap), else routes through an ECEF intermediate.
		template<class P>
		    requires(traits::is_point<P> && !std::same_as<std::remove_cvref_t<P>, Coordinate>)
		Coordinate& operator=(const P& point)
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

		//////////////////////////////////////////////////////////////////////////
		//		POINT INTERFACE
		//////////////////////////////////////////////////////////////////////////

		[[nodiscard]] tuple_type      point() const override { return m_point; }
		[[nodiscard]] frame_data_type frameData() const override { return m_frameData; }

		void setPoint(const tuple_type& point) override { m_point = point; }
		void setPoint(tuple_type&& point) override { m_point = std::move(point); }

		void setFrameData(const frame_data_type& frameData) override { m_frameData = frameData; }
		void setFrameData(frame_data_type&& frameData) override { m_frameData = std::move(frameData); }

		//////////////////////////////////////////////////////////////////////////
		//		ACCESSORS (frame-agnostic)
		//////////////////////////////////////////////////////////////////////////

		/// Date of observation (relevant only when converting between datums).
		[[nodiscard]] years<> date() const { return m_frameData.date; }

		//////////////////////////////////////////////////////////////////////////
		//		QUERIES / MEASUREMENTS (shared forwarders over the free-function engine)
		//////////////////////////////////////////////////////////////////////////

		/// True if this is the zero value of its representation.
		[[nodiscard]] bool isNull() const { return coordinates::isNull(*this); }

		/// Whether another point equals this one within a scalar tolerance.
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
