#ifndef helmert_h__
#define helmert_h__

//------------------------
//	INCLUDES
//------------------------
#include <units.h>
#include <cmath>
#include <functional>
#include <type_traits>

#include "coordinate_traits.h"

namespace coord
{

	//----------------------------------
	//	TRAIT FORWARD DECLARATIONS
	//----------------------------------

	namespace traits
	{
		template<typename T> struct	has_tx;																///< Has static tx member function.
		template<typename T> struct	has_ty;																///< Has static ty member function.
		template<typename T> struct	has_tz;																///< Has static tz member function.
		template<typename T> struct	has_s;																///< Has static s member function.
		template<typename T> struct	has_rx;																///< Has static rx member function.
		template<typename T> struct	has_ry;																///< Has static ry member function.
		template<typename T> struct	has_rz;																///< Has static rz member function.
		template<typename T> struct	has_epoch;															///< Has static epoch member function.
		template<typename T> struct	has_dtx;															///< Has static dtx member function.
		template<typename T> struct	has_dty;															///< Has static dty member function.
		template<typename T> struct	has_dtz;															///< Has static dtz member function.
		template<typename T> struct	has_ds;																///< Has static ds member function.
		template<typename T> struct	has_drx;															///< Has static drx member function.
		template<typename T> struct	has_dry;															///< Has static dry member function.
		template<typename T> struct	has_drz;															///< Has static drz member function.
	}

	//----------------------------------
	//	CONCEPTS
	//----------------------------------

	namespace traits
	{
		/**
		 * @brief		7-parameter Helmert Concept.
		 * @details		Tests whether a class is a 7-parameter Helmert coefficient set.
		 */
		template<typename T>
		struct is_helmert_7param_coefficients : std::integral_constant<bool,
			coord::traits::has_tx<T>::value &&
			coord::traits::has_ty<T>::value &&
			coord::traits::has_tz<T>::value &&
			coord::traits::has_s<T>::value &&
			coord::traits::has_rx<T>::value &&
			coord::traits::has_ry<T>::value &&
			coord::traits::has_rz<T>::value>
		{};
	
		/**
		 * @brief		14-parameter (time dependent) Helmert Concept.
		 * @details		Tests whether a class is a 14-parameter Helmert coefficient set.
		 */
		template<typename T>
		struct is_helmert_14param_coefficients : std::integral_constant<bool,
			coord::traits::is_helmert_7param_coefficients<T>::value &&
			coord::traits::has_epoch<T>::value &&
			coord::traits::has_dtx<T>::value &&
			coord::traits::has_dty<T>::value &&
			coord::traits::has_dtz<T>::value &&
			coord::traits::has_ds<T>::value &&
			coord::traits::has_drx<T>::value &&
			coord::traits::has_dry<T>::value &&
			coord::traits::has_drz<T>::value>
		{};
	}

	//----------------------------------
	//	COMMON HORIZTONAL DATUMS
	//----------------------------------

	/**
	 * @brief		
	 * @details		
	 */
	template<	class Tx,
				class Ty,
				class Tz,
				class ScaleFactor,
				class Rx,
				class Ry,
				class Rz,
				class Epoch = void,
				class Dtx	= void,
				class Dty	= void,
				class Dtz	= void,
				class Ds	= void,
				class Drx	= void,
				class Dry	= void,
				class Drz	= void>
	class Helmert
	{
	public:

		using TX = Tx;
		using TY = Ty;
		using TZ = Tz;
		using S = ScaleFactor;
		using RX = Rx;
		using RY = Ry;
		using RZ = Rz;
		using EPOCH = Epoch;
		using DTX = Dtx;
		using DTY = Dty;
		using DTZ = Dtz;
		using DS = Ds;
		using DRX = Drx;
		using DRY = Dry;
		using DRZ = Drz;

		/// epoch t0
		static constexpr auto epoch() -> decltype(Epoch::value())
		{
			return Epoch::value();
		}

		/// X-axis translation
		static constexpr auto tx() -> decltype(TX::value())
		{
			return TX::value();
		}

		/// Y-axis translation
		static constexpr auto ty() -> decltype(TY::value())
		{
			return TY::value();
		}

		/// Z-axis translation
		static constexpr auto tz() -> decltype(TZ::value())
		{
			return TZ::value();
		}

		/// Scale factor
		static constexpr auto s() -> decltype(S::value())
		{
			return S::value();
		}

		/// Rotation about the X-axis
		static constexpr auto rx() -> decltype(RX::value())
		{
			return RX::value();
		}

		/// Rotation about the Y-axis
		static constexpr auto ry() -> decltype(RY::value())
		{
			return RY::value();
		}

		/// Rotation about the Z-axis
		static constexpr auto rz() -> decltype(RZ::value())
		{
			return RZ::value();
		}

		/// Rate of change of  X-axis translation
		template<class = typename std::enable_if<!std::is_void<DTX>::value>::type>
		static constexpr auto dtx() -> decltype(DTX::value())
		{
			return DTX::value();
		}

		/// Rate of change of  Y-axis translation
		template<class = typename std::enable_if<!std::is_void<DTX>::value>::type>
		static constexpr auto dty() -> decltype(DTY::value())
		{
			return DTY::value();
		}

		/// Rate of change of  Z-axis translation
		template<class = typename std::enable_if<!std::is_void<DTX>::value>::type>
		static constexpr auto dtz() -> decltype(DTZ::value())
		{
			return DTZ::value();
		}

		/// Rate of change of  Scale factor
		template<class = typename std::enable_if<!std::is_void<DTX>::value>::type>
		static constexpr auto ds() -> decltype(DS::value())
		{
			return DS::value();
		}

		/// Rate of change of  Rotation about the X-axis
		template<class = typename std::enable_if<!std::is_void<DTX>::value>::type>
		static constexpr auto drx() -> decltype(DRX::value())
		{
			return DRX::value();
		}

		/// Rate of change of  Rotation about the Y-axis
		template<class = typename std::enable_if<!std::is_void<DTX>::value>::type>
		static constexpr auto dry() -> decltype(DRY::value())
		{
			return DRY::value();
		}

		/// Rate of change of  Rotation about the Z-axis
		template<class = typename std::enable_if<!std::is_void<DTX>::value>::type>
		static constexpr auto drz() -> decltype(DRZ::value())
		{
			return DRZ::value();
		}

		// Check types
		static_assert(units::traits::is_unit_value_t_category<units::category::length_unit, Tx>::value,
			"Template parameter `Tx` must be a `unit_value_t` representing a length");
		static_assert(units::traits::is_unit_value_t_category<units::category::length_unit, Ty>::value,
			"Template parameter `Ty` must be a `unit_value_t` representing a length");
		static_assert(units::traits::is_unit_value_t_category<units::category::length_unit, Tz>::value,
			"Template parameter `Tz` must be a `unit_value_t` representing a length");
		static_assert(units::traits::is_unit_value_t_category<units::category::scalar_unit, S>::value,
			"Template parameter `S` must be a `unit_value_t` representing a concentration");
		static_assert(units::traits::is_unit_value_t_category<units::category::angle_unit, Rx>::value,
			"Template parameter `Rx` must be a `unit_value_t` representing an angle");
		static_assert(units::traits::is_unit_value_t_category<units::category::angle_unit, Ry>::value,
			"Template parameter `Ry` must be a `unit_value_t` representing an angle");
		static_assert(units::traits::is_unit_value_t_category<units::category::angle_unit, Rz>::value,
			"Template parameter `Rz` must be a `unit_value_t` representing an angle");
		static_assert(units::traits::is_unit_value_t_category<units::category::velocity_unit, Dtx>::value,
			"Template parameter `Dtx` must be a `unit_value_t` representing a  velocity");
		static_assert(units::traits::is_unit_value_t_category<units::category::velocity_unit, Dty>::value,
			"Template parameter `Dty` must be a `unit_value_t` representing a velocity");
		static_assert(units::traits::is_unit_value_t_category<units::category::velocity_unit, Dtz>::value,
			"Template parameter `Dtz` must be a `unit_value_t` representing a velocity");
		static_assert(units::traits::is_unit_value_t_category<units::category::frequency_unit, Ds>::value,
			"Template parameter `Ds` must be a `unit_value_t` representing a change in concentration");
		static_assert(units::traits::is_unit_value_t_category<units::category::angular_velocity_unit, Drx>::value,
			"Template parameter `Drx` must be a `unit_value_t` representing an angular velocity");
		static_assert(units::traits::is_unit_value_t_category<units::category::angular_velocity_unit, Dry>::value,
			"Template parameter `Dry` must be a `unit_value_t` representing an angular velocity");
		static_assert(units::traits::is_unit_value_t_category<units::category::angular_velocity_unit, Drz>::value,
			"Template parameter `Drz` must be a `unit_value_t` representing an angular velocity");
	};

	//----------------------------------
	//	INVERSE COEFFICIENTS
	//----------------------------------

	template<class Coefficients, class = typename std::enable_if<coord::traits::is_helmert_7param_coefficients<Coefficients>::value>::type>
	struct inverse_coefficients
	{
		/// epoch
		template<typename C = Coefficients, typename = typename std::enable_if<coord::traits::has_epoch<C>::value>::type>
		static constexpr auto epoch() -> decltype(C::epoch())
		{
			return C::epoch();
		}

		/// Inverse X-axis translation
		static constexpr auto tx() -> decltype(Coefficients::tx())
		{
			return -Coefficients::tx();
		}

		/// Inverse Y-axis translation
		static constexpr auto ty() -> decltype(Coefficients::ty())
		{
			return -Coefficients::ty();
		}

		/// Inverse Z-axis translation
		static constexpr auto tz() -> decltype(Coefficients::tz())
		{
			return -Coefficients::tz();
		}

		/// Inverse Scale factor
		static constexpr auto s() -> decltype(Coefficients::s())
		{
			return -Coefficients::s();
		}

		/// Inverse Rotation about the X-axis
		static constexpr auto rx()  -> decltype(Coefficients::rx())
		{
			return -Coefficients::rx();
		}

		/// Inverse Rotation about the Y-axis
		static constexpr auto ry()  -> decltype(Coefficients::ry())
		{
			return -Coefficients::ry();
		}

		/// Inverse Rotation about the Z-axis
		static constexpr auto rz() -> decltype(Coefficients::rz())
		{
			return -Coefficients::rz();
		}

		/// Rate of change of Inverse X-axis translation
		template<typename C = Coefficients, typename = typename std::enable_if<coord::traits::has_dtx<C>::value>::type>
		static constexpr auto dtx() -> decltype(C::dtx())
		{
			return -C::dtx();
		}

		/// Rate of change of Inverse Y-axis translation
		template<typename C = Coefficients, typename = typename std::enable_if<coord::traits::has_dty<C>::value>::type>
		static constexpr auto dty() -> decltype(C::dty())
		{
			return -C::dty();
		}

		/// Rate of change of Inverse Z-axis translation
		template<typename C = Coefficients, typename = typename std::enable_if<coord::traits::has_dtz<C>::value>::type>
		static constexpr auto dtz() -> decltype(C::dtz())
		{
			return -C::dtz();
		}

		/// Rate of change of Inverse Scale factor
		template<typename C = Coefficients, typename = typename std::enable_if<coord::traits::has_ds<C>::value>::type>
		static constexpr auto ds() -> decltype(C::ds())
		{
			return -C::ds();
		}

		/// Rate of change of Inverse Rotation about the X-axis
		template<typename C = Coefficients, typename = typename std::enable_if<coord::traits::has_drx<C>::value>::type>
		static constexpr auto drx()  -> decltype(C::drx())
		{
			return -C::drx();
		}

		/// Rate of change of Inverse Rotation about the Y-axis
		template<typename C = Coefficients, typename = typename std::enable_if<coord::traits::has_dry<C>::value>::type>
		static constexpr auto dry()  -> decltype(C::dry())
		{
			return -C::dry();
		}

		/// Rate of change of Inverse Rotation about the Z-axis
		template<typename C = Coefficients, typename = typename std::enable_if<coord::traits::has_drz<C>::value>::type>
		static constexpr auto drz() -> decltype(C::drz())
		{
			return -C::drz();
		}
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
	template<class Coefficients, class LengthUnits, typename std::enable_if<traits::is_helmert_7param_coefficients<Coefficients>::value && !coord::traits::is_helmert_14param_coefficients<Coefficients>::value, int>::type = 0>
	std::tuple<units::length::meter_t, units::length::meter_t, units::length::meter_t>
		positionVectorTransform(const units::unit_t<LengthUnits>& x, const units::unit_t<LengthUnits>& y, const units::unit_t<LengthUnits>& z)
	{
		// Helmert 7-parameter implementation
		static_assert(units::traits::is_length_unit<LengthUnits>::value, "`LengthUnits` template parameter must be a unit of length.");

		units::length::meter_t H_tx = Coefficients::tx();
		units::length::meter_t H_ty = Coefficients::ty();
		units::length::meter_t H_tz = Coefficients::tz();
		units::dimensionless::scalar_t H_s = Coefficients::s();
		units::angle::radian_t H_rx = Coefficients::rx();
		units::angle::radian_t H_ry = Coefficients::ry();
		units::angle::radian_t H_rz = Coefficients::rz();

		std::tuple<units::length::meter_t, units::length::meter_t, units::length::meter_t> output;
		units::length::meter_t& X_b = std::get<0>(output);
		units::length::meter_t& Y_b = std::get<1>(output);
		units::length::meter_t& Z_b = std::get<2>(output);

		units::length::meter_t X_a = x;
		units::length::meter_t Y_a = y;
		units::length::meter_t Z_a = z;

		X_b = (H_tx + ((1.0 + (H_s))*(X_a - (H_rz.to<double>()*Y_a) + (H_ry.to<double>()*Z_a))));
		Y_b = (H_ty + ((1.0 + (H_s))*((H_rz.to<double>()*X_a) + Y_a - (H_rx.to<double>()*Z_a))));
		Z_b = (H_tz + ((1.0 + (H_s))*((-H_ry.to<double>()*X_a) + (H_rx.to<double>()*Y_a) + Z_a)));

		return output;
	}

	template<class Coefficients, class LengthUnits, typename std::enable_if<traits::is_helmert_14param_coefficients<Coefficients>::value, int>::type = 0>
	std::tuple<units::length::meter_t, units::length::meter_t, units::length::meter_t>
		positionVectorTransform(const units::unit_t<LengthUnits>& x, const units::unit_t<LengthUnits>& y, const units::unit_t<LengthUnits>& z, const units::time::year_t& dateOfMeasurement = Coefficients::epoch())
	{
		// Helmert 14-parameter implementation
		static_assert(units::traits::is_length_unit<LengthUnits>::value, "`LengthUnits` template parameter must be a unit of length.");

		units::length::meter_t H_tx = (Coefficients::tx() + (Coefficients::dtx()*(dateOfMeasurement - Coefficients::epoch())));
		units::length::meter_t H_ty = (Coefficients::ty() + (Coefficients::dty()*(dateOfMeasurement - Coefficients::epoch())));
		units::length::meter_t H_tz = (Coefficients::tz() + (Coefficients::dtz()*(dateOfMeasurement - Coefficients::epoch())));
		units::dimensionless::scalar_t H_s = (Coefficients::s() + (Coefficients::ds() *(dateOfMeasurement - Coefficients::epoch())));
		units::angle::radian_t H_rx = (Coefficients::rx() + (Coefficients::drx()*(dateOfMeasurement - Coefficients::epoch())));
		units::angle::radian_t H_ry = (Coefficients::ry() + (Coefficients::dry()*(dateOfMeasurement - Coefficients::epoch())));
		units::angle::radian_t H_rz = (Coefficients::rz() + (Coefficients::drz()*(dateOfMeasurement - Coefficients::epoch())));

		std::tuple<units::length::meter_t, units::length::meter_t, units::length::meter_t> output;
		units::length::meter_t& X_b = std::get<0>(output);
		units::length::meter_t& Y_b = std::get<1>(output);
		units::length::meter_t& Z_b = std::get<2>(output);

		units::length::meter_t X_a = x;
		units::length::meter_t Y_a = y;
		units::length::meter_t Z_a = z;

		X_b = (H_tx + ((1.0 + (H_s))*(X_a - (H_rz.to<double>()*Y_a) + (H_ry.to<double>()*Z_a))));
		Y_b = (H_ty + ((1.0 + (H_s))*((H_rz.to<double>()*X_a) + Y_a - (H_rx.to<double>()*Z_a))));
		Z_b = (H_tz + ((1.0 + (H_s))*((-H_ry.to<double>()*X_a) + (H_rx.to<double>()*Y_a) + Z_a)));

		return output;
	}

	template<class Coefficients, class LengthUnits, typename std::enable_if<!coord::traits::is_helmert_14param_coefficients<Coefficients>::value, int>::type = 0>
	std::tuple<units::length::meter_t, units::length::meter_t, units::length::meter_t>
		positionVectorTransform(std::tuple<units::unit_t<LengthUnits>, units::unit_t<LengthUnits>, units::unit_t<LengthUnits>> input)
	{
		return positionVectorTransform<Coefficients, LengthUnits>
			(std::get<0>(input), std::get<1>(input), std::get<2>(input));
	}

	template<class Coefficients, class LengthUnits, typename std::enable_if<!coord::traits::is_helmert_14param_coefficients<Coefficients>::value, int>::type = 0>
	std::tuple<units::length::meter_t, units::length::meter_t, units::length::meter_t>
		inversePositionVectorTransform(const units::unit_t<LengthUnits>& x, const units::unit_t<LengthUnits>& y, const units::unit_t<LengthUnits>& z)
	{
		return positionVectorTransform<coord::inverse_coefficients<Coefficients>, LengthUnits>(x, y, z);
	}

	template<class Coefficients, class LengthUnits, typename std::enable_if<!coord::traits::is_helmert_14param_coefficients<Coefficients>::value, int>::type = 0>
	std::tuple<units::length::meter_t, units::length::meter_t, units::length::meter_t>
		inversePositionVectorTransform(std::tuple<units::unit_t<LengthUnits>, units::unit_t<LengthUnits>, units::unit_t<LengthUnits>> input)
	{
		return positionVectorTransform<coord::inverse_coefficients<Coefficients>, LengthUnits>
			(std::get<0>(input), std::get<1>(input), std::get<2>(input));
	}

	template<class Coefficients, class LengthUnits, typename std::enable_if<coord::traits::is_helmert_14param_coefficients<Coefficients>::value, int>::type = 0>
	std::tuple<units::length::meter_t, units::length::meter_t, units::length::meter_t>
		positionVectorTransform(std::tuple<units::unit_t<LengthUnits>, units::unit_t<LengthUnits>, units::unit_t<LengthUnits>> input, const units::time::year_t& dateOfMeasurement = Coefficients::epoch())
	{
		return positionVectorTransform<Coefficients, LengthUnits>
			(std::get<0>(input), std::get<1>(input), std::get<2>(input), dateOfMeasurement);
	}

	template<class Coefficients, class LengthUnits, typename std::enable_if<coord::traits::is_helmert_14param_coefficients<Coefficients>::value, int>::type = 0>
	std::tuple<units::length::meter_t, units::length::meter_t, units::length::meter_t>
		inversePositionVectorTransform(const units::unit_t<LengthUnits>& x, const units::unit_t<LengthUnits>& y, const units::unit_t<LengthUnits>& z, const units::time::year_t& dateOfMeasurement = Coefficients::epoch())
	{
		return positionVectorTransform<coord::inverse_coefficients<Coefficients>, LengthUnits>(x, y, z, dateOfMeasurement);
	}

	template<class Coefficients, class LengthUnits, typename std::enable_if<coord::traits::is_helmert_14param_coefficients<Coefficients>::value, int>::type = 0>
	std::tuple<units::length::meter_t, units::length::meter_t, units::length::meter_t>
		inversePositionVectorTransform(std::tuple<units::unit_t<LengthUnits>, units::unit_t<LengthUnits>, units::unit_t<LengthUnits>> input, const units::time::year_t& dateOfMeasurement = Coefficients::epoch())
	{
		return positionVectorTransform<coord::inverse_coefficients<Coefficients>, LengthUnits>
			(std::get<0>(input), std::get<1>(input), std::get<2>(input), dateOfMeasurement);
	}

	//----------------------------------
	//	TYPE TRAITS
	//----------------------------------

	namespace traits
	{
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_tx_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::tx());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_length_unit<typename std::decay<decltype(test<T>(0))>::type>::type; // checks return type is some kind of floating point
			};
		}
	
		/**
		* @brief		Tests that a class has a static `tx` member
		* @details		The `tx` member returns some type of length unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_tx : detail::has_tx_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_ty_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::ty());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_length_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `tx` member
		* @details		The `tx` member returns some type of length unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_ty : detail::has_ty_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_tz_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::tz());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_length_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `tz` member
		* @details		The `tz` member returns some type of length unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_tz : detail::has_tz_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_s_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::s());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_concentration_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `s` member
		* @details		The `s` member returns some type of concentration unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_s : detail::has_s_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_rx_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::rx());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_angle_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `rx` member
		* @details		The `rx` member returns some type of angle unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_rx : detail::has_rx_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_ry_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::ry());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_angle_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `ry` member
		* @details		The `ry` member returns some type of angle unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_ry : detail::has_ry_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_rz_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::rz());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_angle_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `rz` member
		* @details		The `rz` member returns some type of angle unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_rz : detail::has_rz_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_dtx_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::dtx());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_velocity_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `dtx` member
		* @details		The `dtx` member returns some type of velocity unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_dtx : detail::has_dtx_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_dty_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::dty());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_velocity_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `dty` member
		* @details		The `dty` member returns some type of velocity unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_dty : detail::has_dty_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_dtz_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::dtz());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_velocity_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `dtz` member
		* @details		The `dtz` member returns some type of velocity unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_dtz : detail::has_dtz_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_ds_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::ds());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename std::is_same<units::traits::base_unit_of<typename units::traits::unit_t_traits<typename std::decay<decltype(test<T>(0))>::type>::unit_type>, units::detail::inverse_base<units::category::time_unit>>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `ds` member
		* @details		The `ds` member returns some type of concentration per time unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_ds : detail::has_ds_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_drx_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::drx());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_angular_velocity_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `drx` member
		* @details		The `drx` member returns some type of angular velocity unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_drx : detail::has_drx_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_dry_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::dry());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_angular_velocity_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `dry` member
		* @details		The `dry` member returns some type of angular velocity unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_dry : detail::has_dry_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_drz_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::drz());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_angular_velocity_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class has a static `drz` member
		* @details		The `drz` member returns some type of angular velocity unit type. It takes no arguments.
		*/
		template <typename T>
		struct has_drz : detail::has_drz_impl<T>::type {};
	
		/// implementation
		namespace detail
		{
			template <typename T>
			struct has_epoch_impl
			{
				template<typename U>
				static constexpr auto test(U* p) -> decltype(U::epoch());	// checks function existence
				template<typename U>
				static constexpr auto test(...)->std::false_type;
	
				using type = typename units::traits::is_time_unit<typename std::decay<decltype(test<T>(0))>::type>::type;
			};
		}
	
		/**
		* @brief		Tests that a class can be defined with an integer `epoch`, representing the epoch
		*				in which measurements were taken.
		*/
		template <typename T>
		struct has_epoch : detail::has_epoch_impl<T>::type {};
	}

}	// end namespace coord

#endif // helmert_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: