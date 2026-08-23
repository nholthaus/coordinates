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
// A pinhole perspective camera that projects world-frame Cartesian points to image pixels. Setup is one call:
// construct with the image size, then `lookAt(eye, target)`; `project(worldPoint)` returns the pixel. The
// camera derives its own right/up/forward basis and its focal length from the field of view, so the caller
// never assembles a projection by hand. A point at or behind the lens projects to the off-image sentinel
// `Pixel{-1, -1}`, which the caller tests with `sees()`.
//
//--------------------------------------------------------------------------------------------------

#ifndef camera_h
#define camera_h

#include <units.h>

#include "pixel.h"
#include "vector3.h"

inline namespace coordinates
{
	inline namespace topography
	{
		using namespace units;
		using namespace units::literals;

		//	----------------------------------------------------------------------------
		//	CLASS		Camera
		//  ----------------------------------------------------------------------------
		///	@brief		A pinhole perspective camera projecting world Cartesian points to image pixels.
		///	@details	Owns the image size, the eye position, the view basis (right/up/forward), the field of view,
		///				and the derived focal length. `lookAt(eye, target)` orients the camera in one call; a
		///				sensible default field of view is set at construction. `project(point)` divides the point's
		///				camera-space lateral offsets by its forward depth, so near geometry looms larger than far.
		//  ----------------------------------------------------------------------------
		class Camera
		{
		public:
			//////////////////////////////////////////////////////////////////////////
			//		CONSTRUCTORS
			//////////////////////////////////////////////////////////////////////////

			/// A camera for a `width` x `height` image with an optional horizontal-plus-vertical field of view
			/// (default 48 deg). The camera starts at the origin looking down +x; call `lookAt` to place it.
			explicit Camera(int width, int height, degrees<> fieldOfView = 48.0_deg)
			    : m_width(width)
			    , m_height(height)
			{
				setFieldOfView(fieldOfView);
			}

			//////////////////////////////////////////////////////////////////////////
			//		SETUP
			//////////////////////////////////////////////////////////////////////////

			/// Place the camera at `eye` looking at `target`, deriving the right/up/forward basis. `worldUp`
			/// disambiguates roll (default -z, i.e. world up in the aerospace down-is-+z convention).
			Camera& lookAt(const CartesianTuple& eye, const CartesianTuple& target,
			               const Vector3<dimensionless<>>& worldUp = Vector3<dimensionless<>>(0.0, 0.0, -1.0))
			{
				m_eye     = eye;
				m_forward = (target - eye).normalized();
				m_right   = m_forward.cross(worldUp).normalized();
				m_up      = m_right.cross(m_forward);
				return *this;
			}

			/// Set the field of view; the focal length (pixels) is derived as the image half-height over the
			/// tangent of the half-angle.
			Camera& setFieldOfView(degrees<> fieldOfView)
			{
				m_focal = (m_height / 2.0) * 1.0_px / tan(fieldOfView / 2.0);
				return *this;
			}

			/// Move the eye without changing where the camera points is not offered: re-aim with `lookAt`, which
			/// keeps the eye and basis consistent.
			[[nodiscard]] const CartesianTuple& eye() const { return m_eye; }
			[[nodiscard]] int                   width() const { return m_width; }
			[[nodiscard]] int                   height() const { return m_height; }

			//////////////////////////////////////////////////////////////////////////
			//		PROJECTION
			//////////////////////////////////////////////////////////////////////////

			/// Project a world-frame point to a pixel. Returns `Pixel{-1, -1}` when the point is at or behind the
			/// lens (test with `sees`).
			[[nodiscard]] Pixel project(const CartesianTuple& world) const
			{
				const CartesianTuple relative = world - m_eye;
				const meters<>       depth    = relative.dot(m_forward);
				if (depth <= 0.0_m)
					return Pixel{-1, -1};
				const auto right = relative.dot(m_right) / depth * m_focal;
				const auto up    = relative.dot(m_up) / depth * m_focal;
				return Pixel{static_cast<int>(round(m_height / 2.0 - up / 1.0_px)),
				             static_cast<int>(round(m_width / 2.0 + right / 1.0_px))};
			}

			/// True if a projected pixel is a real (on-lens) sample rather than the behind-the-lens sentinel.
			[[nodiscard]] static bool sees(Pixel p) { return p.row >= 0; }

		private:
			int                      m_width{0};
			int                      m_height{0};
			CartesianTuple           m_eye{0.0_m, 0.0_m, 0.0_m};
			Vector3<dimensionless<>> m_forward{1.0, 0.0, 0.0};
			Vector3<dimensionless<>> m_right{0.0, 1.0, 0.0};
			Vector3<dimensionless<>> m_up{0.0, 0.0, 1.0};
			raster::pixels<>         m_focal{1.0};
		};
	}    // namespace topography
}    // namespace coordinates

#endif    // camera_h
