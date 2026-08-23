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
// Polygon simplification by the Visvalingam-Whyatt algorithm: repeatedly remove the vertex whose triangle with
// its two neighbours has the smallest area (contributes least to the shape) until a target vertex count remains.
// Ranking by effective area keeps the corners that define the silhouette and drops filler, so a coarse rendering
// of a shape (an aircraft outline as a small map glyph) reads like the full-resolution one with a fraction of the
// vertices.
//
//--------------------------------------------------------------------------------------------------

#ifndef simplify_h
#define simplify_h

#include <array>
#include <cstddef>
#include <vector>

#include <units.h>

#include "frameOfReference.h"

inline namespace coordinates
{
	//----------------------------------------------------------------------------------------------------------------------
	//	FUNCTION: simplify [consteval]
	//----------------------------------------------------------------------------------------------------------------------
	/// @brief		Reduce a closed polygon to its `KeepCount` most significant vertices by the Visvalingam-Whyatt
	///				algorithm, at compile time.
	/// @details	Treats `polygon` as a closed loop and repeatedly removes the vertex whose triangle with its two
	///				current neighbours has the least area -- the one that changes the enclosed area the least --
	///				until `KeepCount` remain, preserving order. The working set is a transient `std::vector` built
	///				and consumed inside this one constant evaluation; the result is copied into a fixed
	///				`std::array<CartesianTuple, KeepCount>` that PERSISTS as a `constexpr` constant, so a caller
	///				writes `constexpr auto reduced = simplify<16>(shape)` and the reduction happens once, at compile
	///				time. The z component is carried through unchanged; significance uses the x-y (plan) area.
	/// @tparam		KeepCount	the number of vertices to keep (must be >= 3 and <= the input size).
	/// @param[in]	polygon		the closed loop of points, in order.
	/// @return		the `KeepCount` most significant vertices, in the original order.
	//----------------------------------------------------------------------------------------------------------------------
	template<std::size_t KeepCount>
	consteval std::array<CartesianTuple, KeepCount> simplify(const CartesianVector& polygon)
	{
		// The plan-area of the triangle a-b-c (half the cross product of the two edges), in square meters.
		const auto triangleArea = [](const CartesianTuple& a, const CartesianTuple& b, const CartesianTuple& c) {
			const auto abx = b.x() - a.x(), aby = b.y() - a.y();
			const auto acx = c.x() - a.x(), acy = c.y() - a.y();
			return units::abs(abx * acy - acx * aby) / 2.0;
		};

		std::vector<CartesianTuple> kept = polygon;
		while (kept.size() > KeepCount)
		{
			std::size_t least     = 0;
			auto        leastArea = triangleArea(kept.back(), kept.front(), kept[1]);
			for (std::size_t i = 1; i < kept.size(); ++i)
			{
				const auto area = triangleArea(kept[i - 1], kept[i], kept[(i + 1) % kept.size()]);
				if (area < leastArea)
				{
					leastArea = area;
					least     = i;
				}
			}
			kept.erase(kept.begin() + static_cast<std::ptrdiff_t>(least));
		}

		std::array<CartesianTuple, KeepCount> reduced{};
		for (std::size_t i = 0; i < KeepCount; ++i)
			reduced[i] = kept[i];
		return reduced;
	}
}    // namespace coordinates

#endif    // simplify_h
