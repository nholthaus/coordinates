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

#ifndef helmert_h
#define helmert_h

//------------------------
//	INCLUDES
//------------------------

#include <type_traits>
#include <units.h>

//----------------------------------
//	ADDITIONAL UNIT DEFINITIONS
//----------------------------------

namespace units
{
	inline namespace velocity
	{
		using meters_per_year      = decltype(m / yr);
		using millimeters_per_year = decltype(mm / yr);
		using ppm_per_year         = decltype(ppm / yr);
		using ppb_per_year         = decltype(ppb / yr);
	}    // namespace velocity

	inline namespace angular_velocity
	{
		using mas_per_year = decltype(mas / yr);
	}
}    // namespace units

inline namespace coordinates
{
	using namespace units;

	//----------------------------------
	//	CONCEPTS
	//----------------------------------

	inline namespace traits
	{
		/**
		 * @brief	Detects whether T exposes a static member function named `tx()`.
		 */
		template<typename T>
		concept has_tx = requires { T::tx(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `ty()`.
		 */
		template<typename T>
		concept has_ty = requires { T::ty(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `tz()`.
		 */
		template<typename T>
		concept has_tz = requires { T::tz(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `s()`.
		 */
		template<typename T>
		concept has_s = requires { T::s(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `rx()`.
		 */
		template<typename T>
		concept has_rx = requires { T::rx(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `ry()`.
		 */
		template<typename T>
		concept has_ry = requires { T::ry(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `rz()`.
		 */
		template<typename T>
		concept has_rz = requires { T::rz(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `epoch()`.
		 */
		template<typename T>
		concept has_epoch = requires { T::epoch(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `dtx()`.
		 */
		template<typename T>
		concept has_dtx = requires { T::dtx(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `dty()`.
		 */
		template<typename T>
		concept has_dty = requires { T::dty(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `dtz()`.
		 */
		template<typename T>
		concept has_dtz = requires { T::dtz(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `ds()`.
		 */
		template<typename T>
		concept has_ds = requires { T::ds(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `drx()`.
		 */
		template<typename T>
		concept has_drx = requires { T::drx(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `dry()`.
		 */
		template<typename T>
		concept has_dry = requires { T::dry(); };

		/**
		 * @brief	Detects whether T exposes a static member function named `drz()`.
		 */
		template<typename T>
		concept has_drz = requires { T::drz(); };

		/**
		 * @brief	7-parameter Helmert coefficient set.
		 * @details	This checks for the required accessors and also constrains their
		 *				return types to be convertible to the canonical units used by
		 *				positionVectorTransform.
		 */
		template<typename T>
		concept Helmert7Coefficients = has_tx<T> && has_ty<T> && has_tz<T> && has_s<T> && has_rx<T> && has_ry<T> && has_rz<T> && requires {
			{ T::tx() } -> std::convertible_to<meters<double>>;
			{ T::ty() } -> std::convertible_to<meters<double>>;
			{ T::tz() } -> std::convertible_to<meters<double>>;
			{ T::s() } -> std::convertible_to<dimensionless<double>>;
			{ T::rx() } -> std::convertible_to<radians<double>>;
			{ T::ry() } -> std::convertible_to<radians<double>>;
			{ T::rz() } -> std::convertible_to<radians<double>>;
		};

		/**
		 * @brief	14-parameter (time-dependent) Helmert coefficient set.
		 */
		template<typename T>
		concept Helmert14Coefficients =
		        Helmert7Coefficients<T> && has_epoch<T> && has_dtx<T> && has_dty<T> && has_dtz<T> && has_ds<T> && has_drx<T> && has_dry<T> && has_drz<T>;

		/**
		 * @brief		7-parameter Helmert Concept.
		 * @details		Tests whether a class is a 7-parameter Helmert coefficient set.
		 */
		template<typename T>
		struct is_helmert_7param_coefficients : std::bool_constant<Helmert7Coefficients<T>>
		{
		};

		/**
		 * @brief		14-parameter (time dependent) Helmert Concept.
		 * @details		Tests whether a class is a 14-parameter Helmert coefficient set.
		 */
		template<typename T>
		struct is_helmert_14param_coefficients : std::bool_constant<Helmert14Coefficients<T>>
		{
		};
	}    // namespace traits

	//----------------------------------
	//	COMMON HORIZONTAL DATUMS
	//----------------------------------

	/**
	 * @brief
	 * @details
	 */
	template<meters<double>            Tx,
	         meters<double>            Ty,
	         meters<double>            Tz,
	         parts_per_billion<double> ScaleFactor,
	         milliarcseconds<double>   Rx,
	         milliarcseconds<double>   Ry,
	         milliarcseconds<double>   Rz,
	         years<double>             Epoch = years{2000.0},
	         meters_per_year           Dtx   = meters_per_year{0.0},
	         meters_per_year           Dty   = meters_per_year{0.0},
	         meters_per_year           Dtz   = meters_per_year{0.0},
	         ppb_per_year              Ds    = ppb_per_year{0.0},
	         mas_per_year              Drx   = mas_per_year{0.0},
	         mas_per_year              Dry   = mas_per_year{0.0},
	         mas_per_year              Drz   = mas_per_year{0.0}>
	class Helmert
	{
	public:
		/// epoch t0
		static constexpr auto epoch() { return Epoch; }

		/// X-axis translation
		static constexpr auto tx() { return Tx; }

		/// Y-axis translation
		static constexpr auto ty() { return Ty; }

		/// Z-axis translation
		static constexpr auto tz() { return Tz; }

		/// Scale factor
		static constexpr auto s() { return ScaleFactor; }

		/// Rotation about the X-axis
		static constexpr auto rx() { return Rx; }

		/// Rotation about the Y-axis
		static constexpr auto ry() { return Ry; }

		/// Rotation about the Z-axis
		static constexpr auto rz() { return Rz; }

		/// Rate of change of  X-axis translation
		static constexpr auto dtx() { return Dtx; }

		/// Rate of change of  Y-axis translation
		static constexpr auto dty() { return Dty; }

		/// Rate of change of  Z-axis translation
		static constexpr auto dtz() { return Dtz; }

		/// Rate of change of  Scale factor
		static constexpr auto ds() { return Ds; }

		/// Rate of change of  Rotation about the X-axis
		static constexpr auto drx() -> decltype(Drx) { return Drx; }

		/// Rate of change of  Rotation about the Y-axis
		static constexpr auto dry() { return Dry; }

		/// Rate of change of  Rotation about the Z-axis
		static constexpr auto drz() { return Drz; }
	};

	//----------------------------------
	//	INVERSE COEFFICIENTS
	//----------------------------------

	template<class Coefficients>
	    requires coordinates::traits::Helmert7Coefficients<Coefficients>
	struct inverse_coefficients
	{
		/// epoch
		template<typename C = Coefficients>
		    requires coordinates::traits::has_epoch<C>
		static constexpr auto epoch() -> decltype(C::epoch())
		{ return C::epoch(); }

		/// Inverse X-axis translation
		static constexpr auto tx() -> decltype(Coefficients::tx()) { return -Coefficients::tx(); }

		/// Inverse Y-axis translation
		static constexpr auto ty() -> decltype(Coefficients::ty()) { return -Coefficients::ty(); }

		/// Inverse Z-axis translation
		static constexpr auto tz() -> decltype(Coefficients::tz()) { return -Coefficients::tz(); }

		/// Inverse Scale factor
		static constexpr auto s() -> decltype(Coefficients::s()) { return -Coefficients::s(); }

		/// Inverse Rotation about the X-axis
		static constexpr auto rx() -> decltype(Coefficients::rx()) { return -Coefficients::rx(); }

		/// Inverse Rotation about the Y-axis
		static constexpr auto ry() -> decltype(Coefficients::ry()) { return -Coefficients::ry(); }

		/// Inverse Rotation about the Z-axis
		static constexpr auto rz() -> decltype(Coefficients::rz()) { return -Coefficients::rz(); }

		/// Rate of change of Inverse X-axis translation
		template<typename C = Coefficients>
		    requires coordinates::traits::has_dtx<C>
		static constexpr auto dtx() -> decltype(C::dtx())
		{ return -C::dtx(); }

		/// Rate of change of Inverse Y-axis translation
		template<typename C = Coefficients>
		    requires coordinates::traits::has_dty<C>
		static constexpr auto dty() -> decltype(C::dty())
		{ return -C::dty(); }

		/// Rate of change of Inverse Z-axis translation
		template<typename C = Coefficients>
		    requires coordinates::traits::has_dtz<C>
		static constexpr auto dtz() -> decltype(C::dtz())
		{ return -C::dtz(); }

		/// Rate of change of Inverse Scale factor
		template<typename C = Coefficients>
		    requires coordinates::traits::has_ds<C>
		static constexpr auto ds() -> decltype(C::ds())
		{ return -C::ds(); }

		/// Rate of change of Inverse Rotation about the X-axis
		template<typename C = Coefficients>
		    requires coordinates::traits::has_drx<C>
		static constexpr auto drx() -> decltype(C::drx())
		{ return -C::drx(); }

		/// Rate of change of Inverse Rotation about the Y-axis
		template<typename C = Coefficients>
		    requires coordinates::traits::has_dry<C>
		static constexpr auto dry() -> decltype(C::dry())
		{ return -C::dry(); }

		/// Rate of change of Inverse Rotation about the Z-axis
		template<typename C = Coefficients>
		    requires coordinates::traits::has_drz<C>
		static constexpr auto drz() -> decltype(C::drz())
		{ return -C::drz(); }
	};

	//----------------------------------
	//	POSITION VECTOR TRANSFORMATION
	//----------------------------------

	//----------------------------------
	//	!!!!!		NOTE		!!!!!
	//
	//	This library uses the Position-
	//	vector transform convention. That
	//	means any transformation parameters
	//	taken from EPSG which are in
	//	coordinate-transform notation will
	//	have to have the signs on the rotation
	//	parameters inverted!
	//
	//	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//----------------------------------

	/**
	 * @brief		Performs a Position Vector Transformation.
	 * @details		Transforms (x,y,z) to (X,Y,Z) using the given coefficients. This can be
	 *				useful to convert ECEF frames of reference to between datums. Currently,
	 *				the coefficients can be either 7-parameter helmert or 14-parameter helmert. The
	 *				correct algorithm will be automatically chosen depending on the parameter set.
	 * @tparam		Coefficients	coefficients to use for the transformation
	 * @param[in]	x x-coordinate of the 'from' datum in units of DistanceUnitsFrom
	 * @param[in]	y y-coordinate of the 'from' datum in units of DistanceUnitsFrom
	 * @param[in]	z z-coordinate of the 'from' datum in units of DistanceUnitsFrom
	 * @returns		tuple of (X,Y,Z) in the resulting 'to' datum in units of DistanceUnitsTo
	 */
	template<class Coefficients, class LengthUnits>
	    requires(coordinates::traits::Helmert7Coefficients<Coefficients> && !coordinates::traits::Helmert14Coefficients<Coefficients> &&
	             units::traits::is_length_unit_v<LengthUnits>)
	std::tuple<meters<>, meters<>, meters<>> positionVectorTransform(const LengthUnits& x, const LengthUnits& y, const LengthUnits& z)
	{
		// Helmert 7-parameter implementation
		meters        H_tx = Coefficients::tx();
		meters        H_ty = Coefficients::ty();
		meters        H_tz = Coefficients::tz();
		dimensionless H_s  = Coefficients::s();
		radians       H_rx = Coefficients::rx();
		radians       H_ry = Coefficients::ry();
		radians       H_rz = Coefficients::rz();

		std::tuple<meters<>, meters<>, meters<>> output;
		auto&                                    X_b = std::get<0>(output);
		auto&                                    Y_b = std::get<1>(output);
		auto&                                    Z_b = std::get<2>(output);

		meters X_a = x;
		meters Y_a = y;
		meters Z_a = z;

		X_b = (H_tx + ((1.0 + (H_s)) * (X_a - (H_rz.template to<double>() * Y_a) + (H_ry.template to<double>() * Z_a))));
		Y_b = (H_ty + ((1.0 + (H_s)) * ((H_rz.template to<double>() * X_a) + Y_a - (H_rx.template to<double>() * Z_a))));
		Z_b = (H_tz + ((1.0 + (H_s)) * ((-H_ry.template to<double>() * X_a) + (H_rx.template to<double>() * Y_a) + Z_a)));

		return output;
	}

	template<class Coefficients, class LengthUnits>
	    requires(coordinates::traits::Helmert14Coefficients<Coefficients> && units::traits::is_length_unit_v<LengthUnits>)
	std::tuple<meters<>, meters<>, meters<>> positionVectorTransform(const LengthUnits& x,
	                                                                 const LengthUnits& y,
	                                                                 const LengthUnits& z,
	                                                                 const years<>&     dateOfMeasurement = Coefficients::epoch())
	{
		// Helmert 14-parameter implementation
		meters        H_tx = (Coefficients::tx() + (Coefficients::dtx() * (dateOfMeasurement - Coefficients::epoch())));
		meters        H_ty = (Coefficients::ty() + (Coefficients::dty() * (dateOfMeasurement - Coefficients::epoch())));
		meters        H_tz = (Coefficients::tz() + (Coefficients::dtz() * (dateOfMeasurement - Coefficients::epoch())));
		dimensionless H_s  = (Coefficients::s() + (Coefficients::ds() * (dateOfMeasurement - Coefficients::epoch())));
		radians       H_rx = (Coefficients::rx() + (Coefficients::drx() * (dateOfMeasurement - Coefficients::epoch())));
		radians       H_ry = (Coefficients::ry() + (Coefficients::dry() * (dateOfMeasurement - Coefficients::epoch())));
		radians       H_rz = (Coefficients::rz() + (Coefficients::drz() * (dateOfMeasurement - Coefficients::epoch())));

		std::tuple<meters<>, meters<>, meters<>> output;
		auto&                                    X_b = std::get<0>(output);
		auto&                                    Y_b = std::get<1>(output);
		auto&                                    Z_b = std::get<2>(output);

		meters X_a = x;
		meters Y_a = y;
		meters Z_a = z;

		X_b = (H_tx + ((1.0 + (H_s)) * (X_a - (H_rz.template to<double>() * Y_a) + (H_ry.template to<double>() * Z_a))));
		Y_b = (H_ty + ((1.0 + (H_s)) * ((H_rz.template to<double>() * X_a) + Y_a - (H_rx.template to<double>() * Z_a))));
		Z_b = (H_tz + ((1.0 + (H_s)) * ((-H_ry.template to<double>() * X_a) + (H_rx.template to<double>() * Y_a) + Z_a)));

		return output;
	}

	template<class Coefficients, class LengthUnits>
	    requires(coordinates::traits::Helmert7Coefficients<Coefficients> && !coordinates::traits::Helmert14Coefficients<Coefficients> &&
	             units::traits::is_length_unit_v<LengthUnits>)
	std::tuple<meters<>, meters<>, meters<>> positionVectorTransform(const std::tuple<LengthUnits, LengthUnits, LengthUnits>& input)
	{ return positionVectorTransform<Coefficients, LengthUnits>(std::get<0>(input), std::get<1>(input), std::get<2>(input)); }

	template<class Coefficients, class LengthUnits>
	    requires(coordinates::traits::Helmert7Coefficients<Coefficients> && !coordinates::traits::Helmert14Coefficients<Coefficients> &&
	             units::traits::is_length_unit_v<LengthUnits>)
	std::tuple<meters<>, meters<>, meters<>> inversePositionVectorTransform(const LengthUnits& x, const LengthUnits& y, const LengthUnits& z)
	{ return positionVectorTransform<inverse_coefficients<Coefficients>, LengthUnits>(x, y, z); }

	template<class Coefficients, class LengthUnits>
	    requires(coordinates::traits::Helmert7Coefficients<Coefficients> && !coordinates::traits::Helmert14Coefficients<Coefficients> &&
	             units::traits::is_length_unit_v<LengthUnits>)
	std::tuple<meters<>, meters<>, meters<>> inversePositionVectorTransform(const std::tuple<LengthUnits, LengthUnits, LengthUnits>& input)
	{ return positionVectorTransform<inverse_coefficients<Coefficients>, LengthUnits>(std::get<0>(input), std::get<1>(input), std::get<2>(input)); }

	template<class Coefficients, class LengthUnits>
	    requires(coordinates::traits::Helmert14Coefficients<Coefficients> && units::traits::is_length_unit_v<LengthUnits>)
	std::tuple<meters<>, meters<>, meters<>> positionVectorTransform(const std::tuple<LengthUnits, LengthUnits, LengthUnits>& input,
	                                                                 const years<>& dateOfMeasurement = Coefficients::epoch())
	{ return positionVectorTransform<Coefficients, LengthUnits>(std::get<0>(input), std::get<1>(input), std::get<2>(input), dateOfMeasurement); }

	template<class Coefficients, class LengthUnits>
	    requires(coordinates::traits::Helmert14Coefficients<Coefficients> && units::traits::is_length_unit_v<LengthUnits>)
	std::tuple<meters<>, meters<>, meters<>> inversePositionVectorTransform(const LengthUnits& x,
	                                                                        const LengthUnits& y,
	                                                                        const LengthUnits& z,
	                                                                        const years<>&     dateOfMeasurement = Coefficients::epoch())
	{ return positionVectorTransform<inverse_coefficients<Coefficients>, LengthUnits>(x, y, z, dateOfMeasurement); }

	template<class Coefficients, class LengthUnits>
	    requires(coordinates::traits::Helmert14Coefficients<Coefficients> && units::traits::is_length_unit_v<LengthUnits>)
	std::tuple<meters<>, meters<>, meters<>> inversePositionVectorTransform(const std::tuple<LengthUnits, LengthUnits, LengthUnits>& input,
	                                                                        const years<>& dateOfMeasurement = Coefficients::epoch())
	{
		return positionVectorTransform<inverse_coefficients<Coefficients>, LengthUnits>(std::get<0>(input),
		                                                                                std::get<1>(input),
		                                                                                std::get<2>(input),
		                                                                                dateOfMeasurement);
	}
}    // end namespace coordinates

#endif    // helmert_h