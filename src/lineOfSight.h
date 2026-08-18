#pragma once

//--------------------------------------------------------------------------------------------------
//
//  LineOfSight: Ellipsoid + Terrain line-of-sight utilities (DTED/topography-backed)
//
//  Design goals:
//  - No hard-coded point types in the public API (use templates + requires).
//  - Do not accept AGL as input; derive it when/if needed.
//  - Terrain LOS and viewsheds are ray/intersection based (no range-bin resolution).
//  - Viewshed overlays can be written as binary PPM (P6) using algorithm::topography
//    hillshade as the grayscale base, with a green/red tint for visible/blocked.
//
//--------------------------------------------------------------------------------------------------

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <future>
#include <limits>
#include <numbers>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "algorithm.h"
#include "positionECEF.h"
#include "positionGeodetic.h"
#include "topography.h"

inline namespace coordinates
{
	/**
	 * @brief Options controlling terrain LOS and viewshed generation.
	 */
	struct LineOfSightOptions
	{
		meters<double> maxRange = meters{200000.0};

		/// Initial step used to bracket a terrain intersection along the ray.
		meters<double> initialBracketStep = meters{100.0};

		/// Growth factor for bracketing steps (must be > 1).
		double bracketGrowth = 1.35;

		/// Number of binary-refinement iterations once a bracket is found.
		std::size_t refineIterations = 24;

		/// 0 = auto (hardware_concurrency)
		std::size_t threadCount = 0;

		/// Apply ellipsoid blocking as an early-out before any terrain work.
		bool useEarthCurvature = true;

		/// When true, do the cheap ellipsoid test first.
		bool preferFastEarlyOut = true;

		/// Tint strength for overlay images (0..1). Typical: 0.35
		double overlayTint = 0.35;
	};

	/**
	 * @brief Result of a terrain ray intersection.
	 */
	template<typename T>
	struct TerrainIntersection
	{
		LLA dummy;    // never used (work around older tooling)
	};

	/**
	 * @brief Stateful LOS engine supporting ellipsoid and terrain (DTED/topography) LOS and viewsheds.
	 *
	 * @tparam Datum Datum type for geodesy (e.g., datums::WGS84_G1674)
	 * @tparam TopographyModel Topography provider (defaults to topography::DTED)
	 * @tparam T Numeric type (defaults to double)
	 */
	template<class Datum, class TopographyModel = DTED, typename T = double>
	class LineOfSight
	{
	public:
		//----------------------------------
		//  TYPES
		//----------------------------------

		/**
		 * @brief Result of a terrain ray intersection.
		 */
		struct TerrainHit
		{
			PositionGeodetic<Datum> hit;          ///< Hit point in geodetic (height interpreted as MSL)
			meters<T>               range;        ///< Along-ray range to the hit point (horizontal surface distance)
			meters<T>               rayHeight;    ///< Ray height (MSL) at intersection
			meters<T>               ground;       ///< Terrain height (MSL) at intersection
		};

		//----------------------------------
		//  CTORS
		//----------------------------------

		/**
		 * @brief Construct a LOS engine with an observer position.
		 *
		 * @tparam Point Point type satisfying point traits and convertible to PositionGeodetic<Datum>
		 * @param[in] observer Observer position (height interpreted as MSL)
		 * @param[in] options  Sampling and map options
		 */
		template<class Point>
		    requires(traits::is_point<Point> && traits::is_convertible_point<Point, PositionGeodetic<Datum>>)
		explicit LineOfSight(const Point& observer, const LineOfSightOptions& options = {})
		    : m_observerGeodetic(PositionGeodetic<Datum>(observer))
		    , m_opt(options)
		{
		}

		//----------------------------------
		//  CONFIG
		//----------------------------------

		/**
		 * @brief Set the observer position.
		 *
		 * @tparam Point Point type satisfying point traits and convertible to PositionGeodetic<Datum>
		 * @param[in] observer Observer position (height interpreted as MSL)
		 */
		template<class Point>
		    requires(traits::is_point<Point> && traits::is_convertible_point<Point, PositionGeodetic<Datum>>)
		void setObserver(const Point& observer)
		{ m_observerGeodetic = PositionGeodetic<Datum>(observer); }

		/**
		 * @brief Set options controlling LOS and map generation.
		 * @param[in] options New options
		 */
		void setOptions(const LineOfSightOptions& options) { m_opt = options; }

		/**
		 * @brief Get current options.
		 * @return Options (copy)
		 */
		[[nodiscard]] LineOfSightOptions options() const { return m_opt; }

		/**
		 * @brief Observer position (geodetic).
		 */
		[[nodiscard]] const PositionGeodetic<Datum>& observer() const noexcept { return m_observerGeodetic; }

		/**
		 * @brief Observer AGL (derived) at the observer location.
		 */
		[[nodiscard]] meters<T> observerAGL() const
		{
			const auto ground = TopographyModel::orthometricHeight(m_observerGeodetic.latitude(), m_observerGeodetic.longitude()).template to<units::length::meters<T>>();
			const auto lat    = degrees<T>{m_observerGeodetic.latitude().value()};
			const auto lon    = degrees<T>{m_observerGeodetic.longitude().value()};
			const auto h      = m_observerGeodetic.altitude().to<meters<T>>();
			return toOrthometricMSL_(lat, lon, h) - ground;
		}

		//----------------------------------
		//  FLAVOR 1: ELLIPSOID LOS
		//----------------------------------

		/**
		 * @brief Ellipsoid-only line-of-sight between two points (ignores terrain).
		 *
		 * @tparam Point Point type satisfying point traits
		 * @param[in] target point from the observer to check line of sight to.
		 * @return True if unobstructed by the ellipsoid; false if blocked.
		 */
		template<class Point>
		    requires(traits::is_point<Point> && traits::is_convertible_point<LLA, Point>)
		[[nodiscard]] bool lineOfSightEllipsoid(const Point& target) const
		{ return coordinates::isLineOfSight<Datum>(m_observerGeodetic, target); }

		//----------------------------------
		//  FLAVOR 2: TERRAIN LOS (RAY/INTERSECTION)
		//----------------------------------

		/**
		 * @brief Terrain-aware line-of-sight from observer to target.
		 *
		 * This does NOT do range-bin stepping. Instead it casts the observer->target line
		 * as an infinite ray and checks whether the ray intersects terrain before reaching
		 * the target.
		 *
		 * Target altitude is interpreted as MSL.
		 *
		 * @tparam Point Target point type convertible to PositionGeodetic<Datum>
		 * @param[in] target Target position
		 * @return True if visible considering terrain; false if blocked.
		 */
		template<class Point>
		    requires(traits::is_point<Point> && traits::is_convertible_point<Point, PositionGeodetic<Datum>>)
		[[nodiscard]] bool lineOfSightTerrain(const Point& target) const
		{
			const PositionGeodetic<Datum> tgt(target);

			// Optional cheap early-out: if the ellipsoid blocks it, terrain can't help.
			if (m_opt.useEarthCurvature && m_opt.preferFastEarlyOut)
			{
				const PositionECEF<Datum> e0(m_observerGeodetic);
				const PositionECEF<Datum> e1(tgt);
				if (!coordinates::isLineOfSight<Datum>(e0, e1))
					return false;
			}

			const auto r = rangeTo_(tgt);
			if (r <= meters<T>{0})
				return true;

			const auto targetAER = PositionAER<Datum>(tgt, m_observerGeodetic);
			const auto azi       = targetAER.azimuth();
			const auto el        = targetAER.elevation();

			auto hit = terrainIntersection(azi, el);
			if (!hit.has_value())
				return true;

			// If the first terrain hit is beyond the target, then terrain does not block the LOS.
			// if (hit->surfaceRange >= r)
			// 	return true;
			// If the first terrain intersection is strictly before the target, it's blocked.
			const T eps = static_cast<T>(0.25);    // 25 cm
			return (hit->range.value() >= r.value() - eps);
		}

		//----------------------------------
		//  FLAVOR 3: TERRAIN INTERSECTION (RAY CAST)
		//----------------------------------

		/**
		 * @brief Intersect an infinite ray with the terrain, returning the first hit.
		 *
		 * The direction vector is ENU (east,north,up) in meters; only its direction matters.
		 * The returned range is the along-surface distance used in geodesicDirect.
		 *
		 * @param[in] enuDirectionMeters Direction in ENU meters (east,north,up)
		 * @param[in] maxRange           Max range to search along the ray
		 * @return Hit record if found; std::nullopt otherwise.
		 */

		/**
		 * @brief Intersect an infinite ray with the terrain, returning the first hit.
		 *
		 * The ray is specified by local azimuth/elevation at the observer.
		 * Azimuth convention: atan2(E, N) with 0=North, 90=East.
		 * Elevation is above the local horizon (0=level, +90=straight up).
		 *
		 * The returned range is the along-surface distance used in geodesicDirect.
		 *
		 * @param[in] azimuth   Azimuth (0=North, 90=East)
		 * @param[in] elevation Elevation above local horizon
		 * @return Hit record if found; std::nullopt otherwise.
		 */
		[[nodiscard]] std::optional<TerrainHit> terrainIntersection(const degrees<T> azimuth, const degrees<T> elevation) const
		{ return terrainIntersectionAngles_(azimuth, elevation); }

		/**
		 * @brief Intersect an infinite ray with the terrain up to a caller-specified maximum range.
		 *
		 * This overload is used internally for observer->target tests (maxRange = target range).
		 *
		 * @param[in] azimuth   Azimuth (0=North, 90=East)
		 * @param[in] elevation Elevation above local horizon
		 * @param[in] maxRange  Maximum along-surface range to search
		 * @return Hit record if found; std::nullopt otherwise.
		 */
		[[nodiscard]] std::optional<TerrainHit> terrainIntersection(const degrees<T> azimuth, const degrees<T> elevation, const meters<T> maxRange) const
		{
			auto hit = terrainIntersectionAngles_(azimuth, elevation);
			if (!hit.has_value())
				return std::nullopt;
			if (hit->surfaceRange > maxRange)
				return std::nullopt;
			return hit;
		}    //----------------------------------
		//  VIEWSHED OVERLAY IMAGE
		//----------------------------------

		/**
		 * @brief Save a tile-sized viewshed overlay image.
		 *
		 * Produces a binary PPM (P6) where the base grayscale is algorithm::topography::hillshade(tile, resolution)
		 * and each pixel is tinted:
		 *  - greenish if that ground pixel is visible from the observer
		 *  - reddish if blocked
		 *
		 * This uses a single terrain-intersection test per pixel (no range-bin stepping).
		 *
		 * @param[in] tile       Tile to render
		 * @param[in] resolution Pixel angular resolution (use tile->metadata().latitudeResolution() for native)
		 * @param[in] filename   Output filename (.ppm recommended)
		 */
		void saveViewshedOverlayPPM(const AbstractTile* tile, degrees<> resolution, const std::string& filename) const
		{
			if (tile == nullptr)
				throw std::invalid_argument("LineOfSight::saveViewshedOverlayPPM: tile is null");
			if (resolution == 0_deg)
				resolution = tile->metadata().latitudeResolution();
			if (!(m_opt.overlayTint >= 0.0 && m_opt.overlayTint <= 1.0))
				throw std::invalid_argument("LineOfSight::saveViewshedOverlayPPM: overlayTint must be in [0,1]");

			const auto shade = coordinates::topography::hillshade(tile, resolution);
			if (shade.empty() || shade.front().empty())
				throw std::runtime_error("LineOfSight::saveViewshedOverlayPPM: hillshade returned empty image");

			const std::size_t height = shade.size();
			const std::size_t width  = shade.front().size();

			// Precompute per-row latitude; lon changes per column.
			const auto latNE = tile->metadata().northeastLatitude();
			const auto lonSW = tile->metadata().southwestLongitude();

			std::vector<std::uint8_t> rgb;
			rgb.resize(width * height * 3);

			const std::size_t threads = (m_opt.threadCount == 0) ? std::max<std::size_t>(1, std::thread::hardware_concurrency()) : m_opt.threadCount;

			auto rowTask = [this, tile, resolution, width, height, latNE, lonSW, &shade, &rgb](std::size_t row)
			{
				const degrees<> lat = latNE - resolution * static_cast<int>(row);
				for (std::size_t col = 0; col < width; ++col)
				{
					const degrees<> lon = lonSW + resolution * static_cast<int>(col);
					const auto      gnd = TopographyModel::orthometricHeight(lat, lon).template to<units::length::meters<T>>();

					// Target is the ground pixel (MSL).
					const LLA  tgt(lat, lon, gnd);
					const bool visible = lineOfSightTerrain(tgt);

					const std::uint8_t gray = shade[row][col];

					// Apply tint without destroying terrain contrast.
					const double t = m_opt.overlayTint;
					std::uint8_t r = gray;
					std::uint8_t g = gray;
					std::uint8_t b = gray;

					if (visible)
					{
						// greenish: lift green, slightly suppress red
						g = static_cast<std::uint8_t>(std::clamp(gray + t * (255.0 - gray), 0.0, 255.0));
						r = static_cast<std::uint8_t>(std::clamp(gray * (1.0 - 0.5 * t), 0.0, 255.0));
					}
					else
					{
						// reddish: lift red, slightly suppress green
						r = static_cast<std::uint8_t>(std::clamp(gray + t * (255.0 - gray), 0.0, 255.0));
						g = static_cast<std::uint8_t>(std::clamp(gray * (1.0 - 0.5 * t), 0.0, 255.0));
					}

					const std::size_t idx = (row * width + col) * 3;
					rgb[idx + 0]          = r;
					rgb[idx + 1]          = g;
					rgb[idx + 2]          = b;
				}
			};

			// Simple static partitioning by row.
			std::vector<std::future<void>> jobs;
			jobs.reserve(height);

			if (threads <= 1)
			{
				for (std::size_t row = 0; row < height; ++row)
					rowTask(row);
			}
			else
			{
				std::atomic<std::size_t> nextRow{0};
				auto                     worker = [&]()
				{
					for (;;)
					{
						const std::size_t row = nextRow.fetch_add(1);
						if (row >= height)
							return;
						rowTask(row);
					}
				};

				for (std::size_t i = 0; i < threads; ++i)
					jobs.emplace_back(std::async(std::launch::async, worker));
				for (auto& j : jobs)
					j.get();
			}

			// Mark the observer location for visual sanity: solid blue crosshair.
			{
				const auto obsLat = m_observerGeodetic.latitude();
				const auto obsLon = m_observerGeodetic.longitude();

				// Compute nearest pixel index in the rendered grid.
				const auto dRow = static_cast<long long>(std::llround(((latNE - obsLat) / resolution).to<double>()));
				const auto dCol = static_cast<long long>(std::llround(((obsLon - lonSW) / resolution).to<double>()));

				const auto clampLL = [](long long v, long long lo, long long hi) -> long long
				{
					return (v < lo) ? lo : (v > hi) ? hi : v;
				};

				const long long row0 = clampLL(dRow, 0, static_cast<long long>(height) - 1);
				const long long col0 = clampLL(dCol, 0, static_cast<long long>(width) - 1);

				// 5-pixel cross centered at (row0,col0)
				for (long long dr = 0; dr <= 0; ++dr)
				{
					const long long rr = row0 + dr;
					if (rr < 0 || rr >= static_cast<long long>(height))
						continue;

					const std::size_t idx = (static_cast<std::size_t>(rr) * width + static_cast<std::size_t>(col0)) * 3;
					rgb[idx + 0]          = 0;
					rgb[idx + 1]          = 0;
					rgb[idx + 2]          = 255;
				}
				for (long long dc = 0; dc <= 0; ++dc)
				{
					const long long cc = col0 + dc;
					if (cc < 0 || cc >= static_cast<long long>(width))
						continue;

					const std::size_t idx = (static_cast<std::size_t>(row0) * width + static_cast<std::size_t>(cc)) * 3;
					rgb[idx + 0]          = 0;
					rgb[idx + 1]          = 0;
					rgb[idx + 2]          = 255;
				}
			}

			writePPM_(filename, width, height, rgb);
		}

	private:
		PositionGeodetic<Datum> m_observerGeodetic;
		LineOfSightOptions      m_opt;

		//----------------------------------
		//  INTERNAL HELPERS
		//----------------------------------

		/**
		 * @brief Convert a height in the current Datum's vertical datum to ellipsoidal height.
		 *
		 * @param[in] lat Latitude
		 * @param[in] lon Longitude
		 * @param[in] h   Height in Datum's vertical datum
		 * @return Ellipsoidal height (meters)
		 */
		[[nodiscard]] meters<T> toEllipsoidHeight_(const degrees<T> lat, const degrees<T> lon, const meters<T> h) const
		{
			using vdatum = typename traits::datum_traits<Datum>::vertical_datum;
			return coordinates::convertToEllipsoidHeight<vdatum>(lat, lon, h);
		}

		/**
		 * @brief Convert an ellipsoidal height to orthometric height (MSL) using the TopographyModel reference geoid.
		 *
		 * @param[in] lat     Latitude
		 * @param[in] lon     Longitude
		 * @param[in] hEllip  Ellipsoidal height
		 * @return Orthometric height (MSL, meters)
		 */
		[[nodiscard]] meters<T> ellipsoidToOrthometric_(const degrees<T> lat, const degrees<T> lon, const meters<T> hEllip) const
		{
			using ref_geoid = typename traits::topography_traits<TopographyModel>::reference_geoid;
			return coordinates::convertFromEllipsoidHeight<ref_geoid>(lat, lon, hEllip);
		}

		/**
		 * @brief Convert a height in the current Datum's vertical datum to orthometric height (MSL) for comparison with terrain.
		 *
		 * @param[in] lat Latitude
		 * @param[in] lon Longitude
		 * @param[in] h   Height in Datum's vertical datum
		 * @return Orthometric height (MSL, meters)
		 */
		[[nodiscard]] meters<T> toOrthometricMSL_(const degrees<T> lat, const degrees<T> lon, const meters<T> h) const
		{ return ellipsoidToOrthometric_(lat, lon, toEllipsoidHeight_(lat, lon, h)); }

		[[nodiscard]] meters<T> observerMSL_() const { return m_observerGeodetic.altitude(); }

		[[nodiscard]] meters<T> rangeTo_(const PositionGeodetic<Datum>& tgt) const
		{
			const auto inv = coordinates::geodesicInverse(m_observerGeodetic, tgt);
			return inv.distance();
		}

		[[nodiscard]] std::tuple<meters<T>, meters<T>, meters<T>> directionENUTo_(const PositionGeodetic<Datum>& tgt) const
		{
			// Convert both to ECEF, then rotate delta into the observer's ENU frame.
			const PositionECEF<Datum> eObs(m_observerGeodetic);
			const PositionECEF<Datum> eTgt(tgt);

			const auto xo = std::get<0>(eObs.point());
			const auto yo = std::get<1>(eObs.point());
			const auto zo = std::get<2>(eObs.point());

			const auto xt = std::get<0>(eTgt.point());
			const auto yt = std::get<1>(eTgt.point());
			const auto zt = std::get<2>(eTgt.point());

			const auto dx = xt - xo;
			const auto dy = yt - yo;
			const auto dz = zt - zo;

			const radians<T> lat = m_observerGeodetic.latitude();
			const radians<T> lon = m_observerGeodetic.longitude();

			const auto sLat = sin(lat);
			const auto cLat = cos(lat);
			const auto sLon = sin(lon);
			const auto cLon = cos(lon);

			// Standard ECEF->ENU rotation (right-handed ENU).
			const auto east  = (-sLon) * dx + (cLon) *dy;
			const auto north = (-sLat * cLon) * dx + (-sLat * sLon) * dy + (cLat) *dz;
			const auto up    = (cLat * cLon) * dx + (cLat * sLon) * dy + (sLat) *dz;

			return {east, north, up};
		}

		/**
		 * @brief Compute the maximum along-surface range for an az/el ray before it hits the reference ellipsoid.
		 *
		 * We compute an ECEF ray from the observer in the specified local ENU direction, intersect it with the
		 * datum reference ellipsoid, convert the hit point back to geodetic, then compute surface distance to it.
		 *
		 * @param[in] azimuth   Azimuth (0=North, 90=East)
		 * @param[in] elevation Elevation above local horizon
		 * @return Surface range to ellipsoid hit, or std::nullopt if no hit occurs.
		 */
		[[nodiscard]] std::optional<meters<T>> ellipsoidCapRange_(const degrees<T> azimuth, const degrees<T> elevation) const
		{
			// Choose any arbitrary range (scale doesn't matter for ray direction)
			const meters<T> kRange{1};

			// Build an AER "point" at (az, el, range) from the observer origin.
			// (No manual radians conversion needed if your azimuth/elevation are already angle units.)
			const PositionAER<Datum> aer(azimuth, elevation, kRange, m_observerGeodetic);

			// Convert that AER offset into an ENU offset (still relative to the same origin)
			const PositionENU<Datum> enu(aer);

			// Convert observer and ENU-tip into ECEF
			const PositionECEF<Datum> eObs(m_observerGeodetic);
			PositionECEF<Datum>       eTip(enu);

			// Direction in ECEF is just (tip - origin)
			eTip -= eObs;

			// If intersectRay wants a tuple<meters, meters, meters>
			const auto dir = eTip.point(); // (dx, dy, dz) in meters
			const Intersection<Datum> isect = eObs.intersectRay(dir);

			if (!isect.hitEllipsoid())
				return std::nullopt;

			// Hit -> geodetic -> geodesic inverse as before
			const PositionECEF<Datum>     eHit(isect.ellipsoidECEF());
			const PositionGeodetic<Datum> gHit(eHit);
			const auto                    inv = coordinates::geodesicInverse(m_observerGeodetic, gHit);
			return inv.distance();
		}

		/**
		 * @brief Cast a local azimuth/elevation ray and find the first terrain intersection.
		 *
		 * Uses ellipsoid intersection to cap the search range (ray cannot hit terrain beyond the ellipsoid).
		 *
		 * @param[in] azimuth   Azimuth (0=North, 90=East)
		 * @param[in] elevation Elevation above local horizon
		 * @param[in] maxRange  Maximum along-surface range to search (will be clamped by ellipsoid hit)
		 * @return First terrain hit, or std::nullopt if no terrain blocks the ray within range.
		 */
		[[nodiscard]] std::optional<TerrainHit> terrainIntersectionAngles_(const degrees<T> azimuth, const degrees<T> elevation) const
		{
			// Ellipsoid-capped max surface range for this ray.
			const auto sCap = ellipsoidCapRange_(azimuth, elevation);
			if (!sCap.has_value())
				return std::nullopt;

			const auto maxMeters = min(*sCap, m_opt.maxRange);
			if (maxMeters <= 0.0_m)
				return std::nullopt;

			// Convert elevation to vertical slope per meter of surface distance.
			const T slope = tan(elevation);

			const auto obsMSL = observerMSL_();

			auto step = max(1.0_m, m_opt.initialBracketStep);
			if (!(m_opt.bracketGrowth > 1.0))
				step = std::max(step, 100.0_m);

			auto s0 = 0.0_m;

			// March outward with increasing step until we bracket (ray-ground) crossing.
			for (auto s1 = step; s1 <= maxMeters;)
			{
				const auto direct = coordinates::geodesicDirect(m_observerGeodetic, azimuth, s1);
				const auto p      = direct.destination();

				const auto ground = TopographyModel::orthometricHeight(p.latitude(), p.longitude()).template to<units::length::meters<T>>();
				const auto rayH   = obsMSL + s1 * slope;
				const T    f1     = (rayH - ground).value();

				if (f1 <= 0)
				{
					// Bracket found [s0, s1]. Refine.
					return refineTerrainHit_(azimuth, slope, s0, s1);
				}

				s0 = s1;

				// Grow step.
				s1   = min(maxMeters, s1 + step);
				step = step * m_opt.bracketGrowth;
				step = min(step, 5000.0_m);
				if (s1 == maxMeters)
					break;
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional<TerrainHit> refineTerrainHit_(const degrees<T> azi, const T slope, meters<T> sLo, meters<T> sHi) const
		{
			const auto obsMSL = observerMSL_();

			for (std::size_t i = 0; i < m_opt.refineIterations; ++i)
			{
				const meters<T> sMid{(sLo.value() + sHi.value()) * static_cast<T>(0.5)};
				const auto      direct = coordinates::geodesicDirect(m_observerGeodetic, azi, sMid);
				const auto      p      = direct.destination();

				const auto ground = TopographyModel::orthometricHeight(p.latitude(), p.longitude()).template to<units::length::meters<T>>();
				const auto rayH   = obsMSL + units::length::meters<T>{sMid.value() * slope};
				const T    fMid   = (rayH - ground).value();

				if (fMid <= static_cast<T>(0))
					sHi = sMid;
				else
					sLo = sMid;
			}

			const auto direct = coordinates::geodesicDirect(m_observerGeodetic, azi, sHi);
			const auto p      = direct.destination();
			const auto ground = TopographyModel::orthometricHeight(p.latitude(), p.longitude()).template to<units::length::meters<T>>();
			const auto rayH   = obsMSL + units::length::meters<T>{sHi.value() * slope};

			TerrainHit hit;
			hit.hit = p;
			hit.hit.setAltitude(rayH);    // MSL height at intersection
			hit.range     = sHi;
			hit.rayHeight = rayH;
			hit.ground    = ground;
			return hit;
		}

		static void writePPM_(const std::string& filename, std::size_t width, std::size_t height, const std::vector<std::uint8_t>& rgb)
		{
			if (rgb.size() != width * height * 3)
				throw std::invalid_argument("LineOfSight::writePPM_: rgb size mismatch");

			std::ofstream out(filename, std::ios::binary);
			if (!out)
				throw std::runtime_error("LineOfSight::writePPM_: failed to open output file");

			out << "P6\n" << width << " " << height << "\n255\n";
			out.write(reinterpret_cast<const char*>(rgb.data()), static_cast<std::streamsize>(rgb.size()));

			if (!out)
				throw std::runtime_error("LineOfSight::writePPM_: failed while writing file");
		}
	};
}    // namespace coordinates
