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
// A render target bound to a camera: a `View` pairs a `Camera` with an `Image` sized to its frame, so a drawing
// call takes the pairing once instead of an image and a camera separately. `Image` stays a pure standalone
// raster; `View` is the one place the image-and-camera relationship lives.
//
//--------------------------------------------------------------------------------------------------

#ifndef view_h
#define view_h

#include "camera.h"
#include "color.h"
#include "image.h"

inline namespace coordinates
{
	inline namespace topography
	{
		//	----------------------------------------------------------------------------
		//	CLASS		View
		//  ----------------------------------------------------------------------------
		///	@brief		A camera paired with an image sized to its frame -- the target a projection draws onto.
		///	@details	Holds the `Camera` and an `Image` of the camera's dimensions, so a draw call needs only the
		///				view (not a separate image and camera). The image is a blank canvas (default white).
		//  ----------------------------------------------------------------------------
		class View
		{
		public:
			/// A view onto `camera`, with a fresh image of its dimensions filled with `background` (default white).
			explicit View(const Camera& camera, Color background = {255, 255, 255}) : m_camera(camera), m_image(camera, background) {}

			[[nodiscard]] const Camera& camera() const { return m_camera; }    ///< the projecting camera
			[[nodiscard]] Image&        image() { return m_image; }            ///< the drawable canvas
			[[nodiscard]] const Image&  image() const { return m_image; }

		private:
			Camera m_camera;    ///< the view's camera (copied; a view owns its projection)
			Image  m_image;     ///< the canvas, sized to the camera
		};
	}    // namespace topography
}    // namespace coordinates

#endif    // view_h
