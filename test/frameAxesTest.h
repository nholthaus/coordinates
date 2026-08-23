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

#ifndef frameAxesTest_h__
#define frameAxesTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <type_traits>

#include <frameAxes.h>

using namespace coordinates;
using namespace coordinates::coordinateFrames;
using namespace units;
using namespace units::length;

namespace
{
	class FrameAxesTest : public ::testing::Test
	{
	protected:
		FrameAxesTest() {}
		~FrameAxesTest() override {}
		void SetUp() override {}
		void TearDown() override {}
	};

	//------------------------------------------------------------------------------------------------------
	//	C-0 scaffolding proof: the per-frame axis table encodes the right accessor return types and the
	//	local-ness the Coordinate collapse will consume. Compile-time only -- no behaviour yet.
	//------------------------------------------------------------------------------------------------------

	using WgsHoriz = horizontalDatums::WGS84_G1674;
	using Wgs      = datums::WGS84_G1674;

	// is_local: ECEF/Geodetic are global; ENU/NED/AER are tangent frames carrying an origin.
	static_assert(!coordinates::traits::frame_axes<ECEFFrame<WgsHoriz>>::is_local);
	static_assert(!coordinates::traits::frame_axes<Geodetic3DFrame<Wgs>>::is_local);
	static_assert(coordinates::traits::frame_axes<ENUFrame<WgsHoriz>>::is_local);
	static_assert(coordinates::traits::frame_axes<NEDFrame<WgsHoriz>>::is_local);
	static_assert(coordinates::traits::frame_axes<AERFrame<WgsHoriz>>::is_local);

	// is_local_frame free trait agrees with the table.
	static_assert(!coordinates::traits::is_local_frame<ECEFFrame<WgsHoriz>>);
	static_assert(coordinates::traits::is_local_frame<ENUFrame<WgsHoriz>>);
	static_assert(coordinates::traits::is_local_frame<NEDFrame<WgsHoriz>>);
	static_assert(coordinates::traits::is_local_frame<AERFrame<WgsHoriz>>);

	// ECEF axes are raw lengths.
	static_assert(std::is_same_v<coordinates::traits::frame_axes<ECEFFrame<WgsHoriz>>::component<0>, meters<double>>);
	static_assert(std::is_same_v<coordinates::traits::frame_axes<ECEFFrame<WgsHoriz>>::component<2>, meters<double>>);

	// Geodetic axes are the tagged angle kinds + the datum's height kind (WGS84_G1674 is ellipsoid-referenced).
	static_assert(std::is_same_v<coordinates::traits::frame_axes<Geodetic3DFrame<Wgs>>::component<0>, angles::Latitude>);
	static_assert(std::is_same_v<coordinates::traits::frame_axes<Geodetic3DFrame<Wgs>>::component<1>, angles::Longitude>);
	static_assert(std::is_same_v<coordinates::traits::frame_axes<Geodetic3DFrame<Wgs>>::component<2>, heights::Ellipsoidal>);

	// A geoid-referenced datum's altitude axis is an orthometric height (datum-dependent return type).
	static_assert(std::is_same_v<coordinates::traits::frame_axes<Geodetic3DFrame<datums::NAD83_NAVD88>>::component<2>, heights::Orthometric>);

	// AER axes are the tagged azimuth/elevation + a straight-line range.
	static_assert(std::is_same_v<coordinates::traits::frame_axes<AERFrame<WgsHoriz>>::component<0>, angles::Azimuth>);
	static_assert(std::is_same_v<coordinates::traits::frame_axes<AERFrame<WgsHoriz>>::component<1>, angles::Elevation>);
	static_assert(std::is_same_v<coordinates::traits::frame_axes<AERFrame<WgsHoriz>>::component<2>, ranges::Euclidean>);

	// convert_fast_path defaults to no shortcut (the pairwise shortcuts arrive with the Coordinate body).
	static_assert(!coordinates::traits::convert_fast_path<ENUFrame<WgsHoriz>, ECEFFrame<WgsHoriz>>::has_shortcut);

	TEST_F(FrameAxesTest, tablePresent)
	{
		// The static_asserts above are the real proof; this keeps a runtime test case in the suite.
		SUCCEED();
	}
}

#endif    // frameAxesTest_h__
