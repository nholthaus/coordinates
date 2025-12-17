#ifndef ellipsoid_h__
#define ellipsoid_h__

//------------------------
//	INCLUDES
//------------------------
#include <type_traits>
#include <cmath>

#include <units.h>
#include "coordinate_traits.h"

using namespace units::literals;

namespace coord
{
	//	----------------------------------------------------------------------------
	//	CLASS		ellipsoid
	//  ----------------------------------------------------------------------------
	///	@brief		Interface class for reference ellipsoids for geodetic coordinates
	///	@details
	///
	/// NOTE: not all ellipsoids have the same defined/derived parameters, so we can't just code the following
	/// formulas. We'll make sure that the ellipsoid returns all these values, based on whichever parameters
	/// are defined:
	///
	/// semi - major axis			(a)			typically defined
	/// semi - minor axis			(b)			= a(1 - f)
	/// flattening					(f)			= (a - b) / a
	/// flattening inverse			(f^-1)		= (1 / f)
	/// eccentricity				(e)			= sqrt(1 - (b^2 / a^2))
	/// eccentricity squared		(e^2)		= (a^2 - b^2) / a^2
	//  ----------------------------------------------------------------------------

	//----------------------------------
	//	ELLIPSOID CONCEPT
	//----------------------------------

	/**
	 * @brief		ellipsoid concept definition
	 * @details		Defines the ellipsoid concept. An object is an ellipsoid if:
	 *				- It is default constructible.
	 *				- It has all the proper members returning its constant values, which are:
	 *					- a (semi-major axis)
	 *					- b (semi-minor axis)
	 *					- f (flattening)
	 *					- invf (inverse flattening)
	 *					- e (eccentricity)
	 *					- e2 (eccentricity squared)
	 *				- those constant values are returned as floating point (float, double, long double, etc)
	 *				  types.
	 */
	template <class T>
	concept ellipsoid =
		std::is_default_constructible_v<T> &&
		requires
		{
			{ T::a() } -> std::convertible_to<units::length::meters<>>;
			{ T::b() } -> std::convertible_to<units::length::meters<>>;
			{ T::f() } -> std::convertible_to<units::dimensionless<>>;
			{ T::invf() } -> std::convertible_to<units::dimensionless<>>;
			{ T::e() } -> std::convertible_to<units::dimensionless<>>;
			{ T::e2() } -> std::convertible_to<units::dimensionless<>>;
		};

	//----------------------------------
	//	ELLIPSOID BASE
	//----------------------------------

	//	----------------------------------------------------------------------------
	//	CLASS		Ellipsoid
	//  ----------------------------------------------------------------------------
	///	@brief		Convenience base class for ellipsoids.
	///	@details	Notice: you don't *have* to inherit from this class to create
	///				a compatible ellipsoid. All you have to do is match the concept.
	///				This class is just provided for convenience. If your ellipsoid
	///				isn't defined in terms of `a` and `invf`, just implement your
	///				own with the member functions required by `is_ellipsoid`.
	/// @tparam		SemiMajorAxisRatio_m		semi-major axis length, in meters, defined
	///											by a `std::ratio` type.
	/// @tparam		InverseFlatteningRatio_m	inverse flattening, defined by a `std::ratio` type.
	//  ----------------------------------------------------------------------------
	//	template <class SemiMajorAxisRatio, class InverseFlatteningRatio>
	template <units::length::meters SemiMajorAxis, units::dimensionless InverseFlattening>
	class Ellipsoid
	{
	protected:

	//	using meters = units::length::meters<>;
	//	using scalar = units::dimensionless<>;

	public:

	//	static consteval auto A = SemiMajorAxis;
	//	static consteval auto INVF = InverseFlattening;
		// using F = units::unit_value_t<typename InverseFlatteningRatio::unit_type, InverseFlatteningRatio::ratio::den, InverseFlatteningRatio::ratio::num>;
		// using B = units::unit_value_subtract<SemiMajorAxisRatio, units::unit_value_multiply<SemiMajorAxisRatio, F>>;
		// using E2 = units::unit_value_subtract<units::unit_value_t<scalar, 1>, units::unit_value_power<units::unit_value_divide<B, SemiMajorAxisRatio>, 2>>;
		// using ENUM = units::ratio_sqrt<std::ratio<E2::ratio::num>>; // use intermediate products to reduce chances of overflow
		// using EDEN = units::ratio_sqrt<std::ratio<E2::ratio::den>>;
		// using E = units::unit_value_t<scalar, ENUM::num * EDEN::den, ENUM::den * EDEN::num>;

	public:
		//
		// /**
		// * @brief		semi-major axis.
		// * @details		Also referred to as the Equatorial radius. Typically a defined parameter.
		// * @returns		returns the semi-major axis value in units of meters
		// */
		// static constexpr auto a() -> decltype(SemiMajorAxis)
		// {
		// 	return SemiMajorAxis;
		// }
		//
		// /**
		// * @brief		semi-minor axis.
		// * @details		Also referred to as the Polar radius. May be a defined or derived parameter.
		// * @returns		returns the semi-minor axis value in units of <i>OutputUnits</i>
		// */
		// static constexpr auto b() -> decltype(B::value())
		// {
		// 	return B::value();
		// }
		//
		// /**
		// * @brief		flatting coefficient
		// * @details		Flattening is a measure of the compression of a circle or sphere along a
		// * 				diameter to form an ellipse or an ellipsoid of revolution (spheroid) respectively.
		// * 				Other terms used are ellipticity, or oblateness. The usual notation for flattening
		// * 				is f and its definition in terms of the semi-axes of the resulting ellipse or
		// * 				ellipsoid is f = (a - b) / a. Flattening is a dimensionless constant, and so it
		// * 				does not have a unit template.
		// * @returns		flattening coefficient with <i>T</i> precision.
		// */
		// static constexpr auto f() -> decltype(F::value())
		// {
		// 	return F::value();
		// }
		//
		// /**
		// * @brief		inverse flattening coefficient
		// * @details		inverse of the flattening coefficient, f(). May be a defined or derived parameter.
		// * 				Inverse Flattening is a dimensionless constant, and so it does not have a unit
		// * 				template.
		// * @sa			f()
		// * @return		inverse flattening coefficient with <i>T</i> precision.
		// */
		// static constexpr auto invf() -> decltype(InverseFlattening)
		// {
		// 	return InverseFlattening;
		// }
		//
		// /**
		// * @brief		eccentricity coefficient
		// * @details		eccentricity can be thought of as a measure of how much the ellipse deviates from
		// * 				being circular. It's definition in terms of the semi-axes of the ellipse is
		// * 				sqrt(1 - (b^2 / a^2)). Eccentricity is a dimensionless constant, and so it does
		// * 				not have a unit template.
		// * @return		eccentricity coefficient with <i>T</i> precision.
		// */
		// static constexpr auto e() -> decltype(E::value())
		// {
		// 	return E::value();
		// }
		//
		// /**
		// * @brief		eccentricity coefficient squared
		// * @details		first eccentricity of the ellipsoid, squared.
		// * @sa			e()
		// * @return		eccentricity coefficient squared with <i>T</i> precision.
		// */
		// static constexpr auto e2() -> decltype(E2::value())
		// {
		// 	return E2::value();
		// }
	};

	//----------------------------------
	//	COMMON ELLIPSOIDS
	//----------------------------------

	namespace ellipsoids
	{
		//	----------------------------------------------------------------------------
		//	CLASS		WGS84_Ellipsoid
		//  ----------------------------------------------------------------------------
		///	@brief		Reference Ellipsoid for the WGS84 Datum
		///	@details	Not to be confused with the EMG96 geoid.
		//  ----------------------------------------------------------------------------
		class WGS84 : public Ellipsoid<6378137_m, 298.257223563>
		{
		};

		//	----------------------------------------------------------------------------
		//	CLASS		GRS80
		//  ----------------------------------------------------------------------------
		///	@brief		Reference Ellipsoid for the NAD83 Datum and many others
		///	@details
		//  ----------------------------------------------------------------------------
		class GRS80 : public Ellipsoid<6378137_m, 298.257222101>
		{
		};
	}
}

#endif // ellipsoid_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: