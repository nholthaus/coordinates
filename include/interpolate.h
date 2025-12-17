#ifndef interpolate_h__
#define interpolate_h__

//------------------------
//	INCLUDES
//------------------------
#include <array>

/// Array of function values
template<typename T>
using interpolationVector = std::array<T, 4>;

/// Matrix of function values
template<typename T>
using interpolationMatrix = std::array<std::array<T, 4>, 4>;

/**
 * @brief		interpolates a 2-dimensional line.
 * @details
 * @param[in]	p	vector of y values corresponding to the x values around the area to interpolate
 *					eg.	[f(x-1), f(x0), f(x1), f(x2)]
 *					namely, two points left of the desired interpolation value, and two points to the
 *					right.
 * @param[in]	x	x-value at which to interpolate, normalized to a 0-1 scale.
 * @returns		interpolated y-value at x, i.e. y = f(x).
 */
template <typename T, class = typename std::enable_if<std::is_floating_point<T>::value>::type>
const T cubicInterpolate(const interpolationVector<T>& p, const T x)
{
    return p[1] + 0.5 * x*(p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0])));
}

/**
 * @brief		interpolates a 3-dimensional surface
 * @details
 * @param[in]	p	vector of z values corresponding to the (x, y) values around the area to interpolate
 *					eg.	[f(x-1, y-1)][f(x0, y-1)]	[f(x1, y-1)][f(x2, y-1)]
 *						[f(x-1, y0 )][f(x0, y0 )]	[f(x1, y0 )][f(x2, y0 )]
 *												(x, y)
 *						[f(x-1, y1 )][f(x0, y1 )]	[f(x1, y1 )][f(x2, y1 )]
 *						[f(x-1, y2 )][f(x0, y2 )]	[f(x1, y2 )][f(x2, y2 )]
 * @param[in]	x	x-value at which to interpolate, normalized to a 0-1 scale.
 * @param[in]	y	y-value at which to interpolate, normalized to a 0-1 scale.
 * @returns		interpolated z-value at (x,y), i.e. z = f(x, y).
 */
template <typename T, class = typename std::enable_if<std::is_floating_point<T>::value>::type>
const T bicubicInterpolate(const interpolationMatrix<T>& p, const T x, const T y)
{
    interpolationVector<T> arr;
    arr[0] = cubicInterpolate(p[0], y);
    arr[1] = cubicInterpolate(p[1], y);
    arr[2] = cubicInterpolate(p[2], y);
    arr[3] = cubicInterpolate(p[3], y);
    return cubicInterpolate(arr, x);
}

#endif // interpolate_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: