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
// Drawing a body-frame polyline through a pose and a camera onto an image. `drawPolyline` takes a closed loop of
// body-axis points, applies a rigid pose (the body's attitude/position in the world), projects each point through
// the camera, and strokes the loop onto the image -- the generic wireframe primitive any posed-shape renderer
// (an aircraft outline, a sensor footprint, a bounding box) draws with.
//
//--------------------------------------------------------------------------------------------------

#ifndef wireframe_h
#define wireframe_h

#include <ranges>

#include "camera.h"
#include "color.h"
#include "frameOfReference.h"
#include "image.h"
#include "pose.h"
#include "view.h"

inline namespace coordinates
{
	inline namespace topography
	{
		//----------------------------------------------------------------------------------------------------------------------
		//	FUNCTION: drawPolyline [free]
		//----------------------------------------------------------------------------------------------------------------------
		/// @brief		Draw a body-frame polyline, posed into the world, through a camera onto an image as a closed loop.
		/// @details	Each body point is posed by `pose` and projected to a pixel; consecutive projected pixels -- and
		///				the last back to the first -- are stroked (two pixels thick for crispness), skipping any edge
		///				with an endpoint behind the lens. The shape is defined once in body axes; drawing it at any
		///				attitude is just a different `pose`, so the projection foreshortens and rotates it for free. The
		///				edges are the pixels zipped with themselves rotated by one, which closes the loop.
		/// @param[in,out]	image	the image to draw onto.
		/// @param[in]		camera	the camera projecting world points to pixels.
		/// @param[in]		pose	the rigid transform placing the body-frame polyline in the world.
		/// @param[in]		polyline	the closed loop of body-axis points, in draw order.
		/// @param[in]		color	the stroke color.
		//----------------------------------------------------------------------------------------------------------------------
		inline void drawPolyline(Image& image, const Camera& camera, const Pose& pose, const CartesianVector& polyline, Color color)
		{
			const auto pixels = polyline | std::views::transform([&](const CartesianTuple& p) { return camera.project(pose.transformPoint(p)); })
			                  | std::ranges::to<std::vector>();

			// Each edge joins pixel i to the next, the last wrapping back to the first, closing the loop.
			for (const std::size_t i : std::views::iota(std::size_t{0}, pixels.size()))
			{
				const Pixel a = pixels[i];
				const Pixel b = pixels[(i + 1) % pixels.size()];
				if (Camera::sees(a) && Camera::sees(b))
				{
					image.line(a, b, color);
					image.line({a.row + 1, a.column}, {b.row + 1, b.column}, color);
				}
			}
		}

		/// Draw a body-frame polyline onto a `View` -- the view supplies both its camera and its image, so a
		/// caller need not pass them separately.
		inline void drawPolyline(View& view, const Pose& pose, const CartesianVector& polyline, Color color = {})
		{
			drawPolyline(view.image(), view.camera(), pose, polyline, color);
		}
	}    // namespace topography
}    // namespace coordinates

#endif    // wireframe_h
