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
// Conversion between latitude kinds, routed by the same least-common-ancestor machinery that routes frame
// conversions. Geodetic latitude (the angle to the ellipsoid normal, what GPS reports) and geocentric
// latitude (the angle to the ellipsoid centre) are both latitudes but differ by the ellipsoid's flattening;
// they are distinct `units::kind` tags that must not be silently mixed. Each is a node in a small kind graph
// whose parent edge is given by `kind_parent`; geodetic latitude is the root, and geocentric latitude derives
// from it via `tan(geocentric) = (1 - e^2) * tan(geodetic)`. `convertLatitude<To, Ellipsoid>(from)` walks the
// graph with `traits::detail::least_common_ancestor<..., kind_parent>` -- the generic algorithm the frame
// graph also uses -- so adding a third latitude kind (conformal, reduced/parametric) is one more node with no
// dispatcher change. The per-hop physics needs the ellipsoid (for e^2), which rides as the conversion context
// the way `FrameData` rides a frame conversion.
//
//--------------------------------------------------------------------------------------------------

#ifndef latitudeConversion_h
#define latitudeConversion_h

//------------------------
//	INCLUDES
//------------------------

#include <cmath>

#include <units.h>
#include <units/kind.h>

#include "angles.h"
#include "ellipsoid.h"
#include "frameOfReference.h"

inline namespace coordinates
{
	using namespace units;

	namespace latitudes
	{
		//	----------------------------------------------------------------------------
		//	Latitude kind-graph nodes
		//  ----------------------------------------------------------------------------
		//	Each node names the latitude kind it carries (`kind_type`) and its parent node (`parent_type`), and
		//	supplies the per-hop physics `toParent` / `fromParent`. Geodetic latitude is the root (its parent is
		//	itself). The nodes are types only; a value is a plain `angles::*` kind, and the ellipsoid rides as
		//	the conversion context.

		/// Geodetic latitude node -- the graph root (parent is itself, no correction to apply).
		struct GeodeticLatitude
		{
			using kind_type   = angles::Latitude;
			using parent_type = GeodeticLatitude;
		};

		/// Geocentric latitude node -- derives from geodetic via the ellipsoid eccentricity.
		struct GeocentricLatitude
		{
			using kind_type   = angles::Geocentric;
			using parent_type = GeodeticLatitude;

			//------------------------------------------------------------------------------------------------------
			/// @brief		Convert this geocentric latitude up to its parent geodetic latitude.
			/// @tparam		Ellipsoid	the reference ellipsoid supplying the eccentricity.
			/// @param[in]	geocentric	the geocentric latitude.
			/// @return		the geodetic latitude: `atan( tan(geocentric) / (1 - e^2) )`.
			//------------------------------------------------------------------------------------------------------
			template<class Ellipsoid>
			static angles::Latitude toParent(angles::Geocentric geocentric) noexcept
			{
				const double e2      = Ellipsoid::e2().template to<double>();
				const double phiGeoc = angle::radians<>(geocentric.template to<angle::degrees<>>()).template to<double>();
				return angles::Latitude(angle::degrees<>(angle::radians<>(std::atan2(std::tan(phiGeoc), 1.0 - e2))));
			}

			//------------------------------------------------------------------------------------------------------
			/// @brief		Convert a parent geodetic latitude down to this geocentric latitude.
			/// @tparam		Ellipsoid	the reference ellipsoid supplying the eccentricity.
			/// @param[in]	geodetic	the geodetic latitude.
			/// @return		the geocentric latitude: `atan( (1 - e^2) * tan(geodetic) )`.
			//------------------------------------------------------------------------------------------------------
			template<class Ellipsoid>
			static angles::Geocentric fromParent(angles::Latitude geodetic) noexcept
			{
				const double e2      = Ellipsoid::e2().template to<double>();
				const double phiGeod = angle::radians<>(geodetic.template to<angle::degrees<>>()).template to<double>();
				return angles::Geocentric(angle::degrees<>(angle::radians<>(std::atan2((1.0 - e2) * std::tan(phiGeod), 1.0))));
			}
		};

		//	----------------------------------------------------------------------------
		//	kind_parent accessor + node predicate for the generic LCA
		//  ----------------------------------------------------------------------------

		/// Parent-accessor for a latitude node, in the shape the generic `least_common_ancestor` expects.
		template<class Node>
		struct kind_parent
		{
			using type = typename Node::parent_type;
		};

		/// Every latitude node is an accepted ancestor (the graph is a single chain to the geodetic root).
		template<class Node>
		struct is_latitude_node : std::true_type
		{
		};

		/// The node that carries a given latitude kind, so a conversion can deduce its source node from the
		/// value's kind (each kind is carried by exactly one node).
		template<class Kind>
		struct node_for;
		template<>
		struct node_for<angles::Latitude>
		{
			using type = GeodeticLatitude;
		};
		template<>
		struct node_for<angles::Geocentric>
		{
			using type = GeocentricLatitude;
		};

		//	----------------------------------------------------------------------------
		//	Dispatchers -- climb to the ancestor, then descend, mirroring the frame graph
		//  ----------------------------------------------------------------------------

		namespace latitudeDetail
		{
			/// Climb `From` up to the intermediate ancestor, applying each node's `toParent` (needs the
			/// ellipsoid); terminates when `From` reaches the ancestor.
			template<class From, class Ancestor, class Ellipsoid>
			static constexpr auto climbToAncestor(typename From::kind_type value) noexcept
			{
				if constexpr (std::is_same_v<From, Ancestor>)
					return value;
				else
					return climbToAncestor<typename From::parent_type, Ancestor, Ellipsoid>(From::template toParent<Ellipsoid>(value));
			}

			/// Descend the ancestor down to `To`, applying each node's `fromParent` on the way back out
			/// (root-to-leaf order), mirroring the frame graph's `convertFromBase`.
			template<class Ancestor, class To, class Ellipsoid>
			static constexpr auto descendToTarget(typename Ancestor::kind_type value) noexcept
			{
				if constexpr (std::is_same_v<Ancestor, To>)
					return value;
				else
					return To::template fromParent<Ellipsoid>(descendToTarget<Ancestor, typename To::parent_type, Ellipsoid>(value));
			}
		}    // namespace latitudeDetail

		//------------------------------------------------------------------------------------------------------
		/// @brief		Convert a latitude from one kind to another over the given ellipsoid.
		/// @details	Routes through the least common ancestor of the two latitude nodes using the same generic
		///				`least_common_ancestor` the frame graph uses (here over the `kind_parent` graph). For the
		///				geodetic/geocentric pair the ancestor is geodetic latitude, so a geocentric-to-geocentric
		///				conversion (should a third kind ever sit between them) still routes correctly. Mixing the
		///				two kinds without this bridge is a compile error.
		/// @tparam		ToNode		the destination latitude node (e.g. `GeocentricLatitude`).
		/// @tparam		Ellipsoid	the reference ellipsoid whose eccentricity drives the conversion.
		/// @tparam		FromKind	the source latitude kind (deduced from the argument); its node is `node_for`.
		/// @param[in]	from	the latitude value in the source kind.
		/// @return		the latitude value in the destination kind.
		//------------------------------------------------------------------------------------------------------
		template<class ToNode, class Ellipsoid, class FromKind>
		    requires(coordinates::traits::is_ellipsoid<Ellipsoid>)
		static typename ToNode::kind_type convertLatitude(FromKind from) noexcept
		{
			using FromNode = typename node_for<FromKind>::type;
			using Ancestor = typename coordinates::traits::detail::least_common_ancestor<FromNode, ToNode, is_latitude_node, kind_parent>::type;
			return latitudeDetail::descendToTarget<Ancestor, ToNode, Ellipsoid>(latitudeDetail::climbToAncestor<FromNode, Ancestor, Ellipsoid>(from));
		}
	}    // namespace latitudes

	// Surface the latitude-graph entry points + nodes at the coordinates namespace for ergonomic use.
	using latitudes::convertLatitude;
	using latitudes::GeocentricLatitude;
	using latitudes::GeodeticLatitude;
}    // namespace coordinates

#endif    // latitudeConversion_h
