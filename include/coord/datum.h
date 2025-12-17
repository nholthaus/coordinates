#ifndef datum_h__
#define datum_h__

//------------------------
//	INCLUDES
//------------------------

#include "horizontalDatum.h"
#include "verticalDatum.h"

namespace coord
{
	//	----------------------------------------------------------------------------
	//	CLASS		Datum
	//  ----------------------------------------------------------------------------
	///	@brief		base class for creating datum definitions
	///	@details	
	/// @note		Inheritance from this class is not required to fulfill the datum
	///				concept. See `is_datum` for the concept requirements.
	//  ----------------------------------------------------------------------------
	template<class Horizontal, class Vertical = typename coord::traits::horizontal_datum_traits<Horizontal>::reference_ellipsoid>
	struct Datum : public Horizontal, public Vertical
	{
		static_assert(traits::is_horizontal_datum<Horizontal>::value, "Template parameter `Horizontal` must be a horizontal datum type.");
		static_assert(traits::is_vertical_datum<Vertical>::value, "Template parameter `Vertical` must be a vertical datum type.");
		static_assert(std::is_same<typename traits::horizontal_datum_traits<Horizontal>::reference_ellipsoid, typename traits::vertical_datum_traits<Vertical>::reference_ellipsoid>::value,
			"The `Horizontal` and `Vertical` Datum components must refer to the same ellipsoid.");

		typedef Horizontal																	horizontal_datum;
		typedef Vertical																	vertical_datum;
		typedef typename traits::horizontal_datum_traits<Horizontal>::reference_ellipsoid	reference_ellipsoid;
		typedef typename traits::horizontal_datum_traits<Horizontal>::reference_frame		reference_frame;
	};

	//----------------------------------
	//	COMMON DATUMS
	//----------------------------------
	namespace datums
	{
		//	----------------------------------------------------------------------------
		//	CLASS		WGS84_G1674
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct WGS84_G1674 : public Datum<coord::horizontalDatums::WGS84_G1674>{};

		//	----------------------------------------------------------------------------
		//	CLASS		WGS84_G1674_MSL
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct WGS84_G1674_MSL : public Datum<coord::horizontalDatums::WGS84_G1674, coord::geoids::EGM96>{};

		//	----------------------------------------------------------------------------
		//	CLASS		NAD83
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		/// @note		Only suitable for use within CONUS (24-58N, 130-60W)
		//  ----------------------------------------------------------------------------
		struct NAD83 : public Datum<coord::horizontalDatums::NAD83>{};

		//	----------------------------------------------------------------------------
		//	CLASS		NAD83_NAVD88
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details
		/// @note		Only suitable for use within CONUS (24-58N, 130-60W)
		//  ----------------------------------------------------------------------------
		struct NAD83_NAVD88 : public Datum<coord::horizontalDatums::NAD83, coord::geoids::GEOID12A>{};

		//	----------------------------------------------------------------------------
		//	CLASS		NAD83_MSL
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		/// @note		Only suitable for use within CONUS (24-58N, 130-60W)
		//  ----------------------------------------------------------------------------
		struct NAD83_MSL : public Datum<coord::horizontalDatums::NAD83, coord::geoids::USGG2012>{};

		//	----------------------------------------------------------------------------
		//	CLASS		IGS08
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct IGS08 : public Datum<coord::horizontalDatums::IGS08>{};

		//	----------------------------------------------------------------------------
		//	CLASS		IGS08_MSL
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		/// @note		Only suitable for use within CONUS (24-58N, 130-60W)
		//  ----------------------------------------------------------------------------
		struct IGS08_MSL : public Datum<coord::horizontalDatums::NAD83, coord::geoids::USGG2012>{};

		//	----------------------------------------------------------------------------
		//	CLASS		GDA94
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		/// @note		Only suitable for use in and about Australia
		//  ----------------------------------------------------------------------------
		struct GDA94 : public Datum<coord::horizontalDatums::GDA94>{};

		//	----------------------------------------------------------------------------
		//	CLASS		ETRS89
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		/// @note		Only suitable for use in and about Europe
		//  ----------------------------------------------------------------------------
		struct ETRS89 : public Datum<coord::horizontalDatums::ETRS89>{};

		//	----------------------------------------------------------------------------
		//	CLASS		ITRS2008
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct ITRS2008 : public Datum<coord::horizontalDatums::ITRS2008>{};

		//	----------------------------------------------------------------------------
		//	CLASS		ITRS2000
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct ITRS2000 : public Datum<coord::horizontalDatums::ITRS2000>{};

		//	----------------------------------------------------------------------------
		//	CLASS		ITRS88
		//  ----------------------------------------------------------------------------
		///	@brief		
		///	@details	
		/// @note		Suitable for world-wide use.
		//  ----------------------------------------------------------------------------
		struct ITRS88 : public Datum<coord::horizontalDatums::ITRS88>{};
	}
	
	//----------------------------------
	//	TYPE TRAITS
	//----------------------------------
	namespace traits
	{
		/**
		 * @brief		geoid traits implementation for classes which are not datums.
		 */
		template<class T, typename = void>
		struct datum_traits
		{
			typedef void horizontal_datum;
			typedef void vertical_datum;
			typedef void reference_ellipsoid;
			typedef void reference_frame;
		};

		/**
		 * @brief		Traits class defining the properties of a datum.
		 */
		template<class T>
		struct datum_traits<T, typename coord::traits::void_type<
			typename T::horizontal_datum,
			typename T::vertical_datum,
			typename T::reference_frame,
			typename T::reference_ellipsoid
		>::type>
		{
			typedef typename T::horizontal_datum horizontal_datum;									///< Horizontal component of the datum
			typedef typename T::vertical_datum vertical_datum;										///< Vertical component of the datum
			typedef typename T::reference_ellipsoid reference_ellipsoid;							///< Ellipsoid component of the datum
			typedef typename T::reference_frame reference_frame;									///< Reference (ITRF) frame of the datum
		};

		/**
		 * @brief		Traits which tests whether a class satisfies the datum concept
		 */
		template<typename T>
		struct is_datum : std::integral_constant<bool,
			coord::traits::is_horizontal_datum<typename coord::traits::datum_traits<T>::horizontal_datum>::value &&
			coord::traits::is_vertical_datum<typename coord::traits::datum_traits<T>::vertical_datum>::value &&
			coord::traits::is_ellipsoid<typename coord::traits::datum_traits<T>::reference_ellipsoid>::value
		>::type
		{};
	}

}

#endif // datum_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: