# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

Modernization on `units` 3.6.1: a reusable rotation-math library, plus latent
numeric and packaging fixes, CI hardening, and licensing/documentation.

### Added

- **Ergonomic member API** (per `docs/API-form-policy.md`). Two-point measurements now read as directional
  members returning their distinctly-tagged kind: `a.euclideanDistanceTo(b)` (`ranges::Euclidean`),
  `a.slantRangeTo(b)` (`ranges::Euclidean` -- a slant range is a straight-line distance),
  `a.geodesicDistanceTo(b)` (`ranges::Geodesic`), and `a.bearingTo(b)`
  (`angles::Azimuth`), alongside the existing `distance`/`distanceTo`/`initialBearingTo`. Latitude conversion
  gains a position front door: `pos.geocentricLatitude()` / `pos.geodeticLatitude()`, which use the
  position's own datum ellipsoid (no argument).
- **Geodetic ↔ geocentric latitude conversion** (`src/latitudeConversion.h`): a new `angles::Geocentric`
  kind and `convertLatitude<ToNode, Ellipsoid>(from)` that converts between geodetic latitude (the ellipsoid
  normal, what GPS reports) and geocentric latitude (the ellipsoid centre) via the ellipsoid's eccentricity
  — a conversion the library did not previously provide. The two are distinct kinds that cannot be silently
  mixed. It is built as a small kind graph (geodetic latitude is the root, geocentric derives from it) routed
  by the same generic `least_common_ancestor` the frame graph uses, so a third latitude kind (conformal,
  reduced) would be one more node with no dispatcher change.
- **`Pose` ↔ geodesy interop**: a runtime 6-DOF pose now bridges to the position types for the
  moving-vehicle case (an aircraft centre of gravity in ECEF carrying a fixed sensor mount). `Pose::at(position,
  attitude)` places a pose at a geodesy position; `transformPoint` accepts and returns a Cartesian position
  type (e.g. a wing-mounted sensor's `ECEF` location); `rotateDirection` carries a body-axis boresight into
  the parent frame as the vehicle slews; and `Pose::from<Mount>()` lifts a compile-time `BodyTransform` mount
  so it composes onto a live vehicle pose (`sensorPose = vehiclePose * Pose::from<Mount>()`).
- **`PositionAER::fromObserver(observer, target)`**: a static factory for the observer-relative look angles
  (azimuth, elevation, range) of a target as seen from an observer — the observer-relative spelling of
  `PositionAER(target, observer)`. Previously advertised in the README but unimplemented.
- **`distanceSquared(a, b)`**: the squared straight-line distance between two points (an area), skipping the
  square root for performance-sensitive relative comparisons. Previously advertised but unimplemented.
- **Strongly-typed distances** (`src/ranges.h`): a distance is distinctly typed only where its reference
  genuinely differs. A straight-line distance through 3-space -- a Euclidean distance, or equally a slant
  range from an observer -- carries no reference surface, so it is one kind (`ranges::Euclidean`). A geodesic
  distance is measured along the ellipsoid surface (an arc, not a chord), so it is a distinct kind
  (`ranges::Geodesic`); mixing a surface distance with a straight-line distance is a compile error, while two
  straight-line distances interoperate freely. `PositionAER::range()` and the `distance()` / `magnitude()`
  accessors return `ranges::Euclidean`; `distanceTo()` / `GeodesicInverseResult::distance()` return
  `ranges::Geodesic`.
- **Strongly-typed angles** (`src/angles.h`): latitude, longitude, azimuth, elevation, and the three
  orientation angles (yaw/pitch/roll) are now distinct `units::kind` types in the `angles::` namespace.
  They are all `degrees<>` but semantically incommensurable, so mixing them is a compile error — most
  importantly, a latitude and an azimuth (which previously shared both a type and a `SphericalTuple` slot)
  can no longer be silently confused. `PositionGeodetic::latitude()`/`longitude()`,
  `PositionAER::azimuth()`/`elevation()`, and every geodesic bearing accessor now return their tagged kind;
  AER azimuth, geodesic bearings, and any North-referenced heading unify as one `angles::Azimuth`.
- **Strongly-typed heights** (`src/heightKinds.h`, `src/heights.h`, `src/verticalDatum.h`): ellipsoidal
  (HAE) and orthometric (MSL) heights are now distinct `units::kind` types (`heights::Ellipsoidal`,
  `heights::Orthometric`), joined by a `heights::Undulation` kind for the geoid separation. Both are lengths
  but mixing them in arithmetic or comparison is a compile error; the only bridge is an explicit conversion.
  A plain length still constructs into a height implicitly, so existing construction is unaffected.
  `heights::kind_for<Datum>` deduces which height a datum measures from its vertical reference. The
  topography producer side is tagged too: `DTED::orthometricHeight` / `NULL_TOPOGRAPHY::orthometricHeight`
  now return `heights::Orthometric`, and the `OrthometricHeight` concept accepts a length kind (via the new
  `is_length_quantity` predicate) as well as a plain length.
- **`PositionGeodetic::toEllipsoidHeight()` / `toOrthometricHeight()`**: convert a point's stored altitude
  between HAE and MSL through the datum's geoid, returning the tagged height kind.

- **Rotation-math library** in `lib/` (`quaternion.h`, `rotation.h`): four fully
  interconvertible, `constexpr`-capable rotation representations — `Quaternion`
  (canonical, Hamilton convention, active rotation), `EulerAngles` (intrinsic Z-Y-X
  Tait-Bryan yaw/pitch/roll), `RotationMatrix` (3x3 direction-cosine), and `AxisAngle`.
  Operations include compose (`operator*`), `conjugate`/`inverse`, `normalized`,
  `rotate` (a vector), `fromTwoVectors`, `slerp`, `identity`, and the `toQuaternion`/
  `toEulerAngles`/`toRotationMatrix`/`toAxisAngle` cross-conversions. Depends only on
  `units`, with a constant-evaluable trig fallback so trig-bearing conversions remain
  usable in a constant-expression context.
- **`BodyFrame`** (`src/bodyFrame.h`): a Cartesian frame rigidly attached to a parent
  frame at a compile-time offset and orientation, carried in the frame type via the
  `BodyTransform` policy (with `Offset` and `Attitude` convenience aliases). Body frames
  nest to arbitrary depth (e.g. a camera on a wingtip on an aircraft body in local NED)
  and join the existing frame graph with no dispatcher changes.
- **`Pose`** (`src/pose.h`): a runtime 6-DOF rigid transform (a translation plus a
  `Quaternion`) for bodies whose position and attitude vary over time. Provides
  `transformPoint`, `inverse`, composition via `operator*`, and `identity`.

### Changed

- The type-level least-common-ancestor machinery (`traits::detail::depth`,
  `traits::detail::least_common_ancestor`) is now generic over a `Parent` accessor rather than hard-wired to
  a frame's `base_frame_type`, so one algorithm serves any single-parent node graph. The frame graph is
  unchanged (it uses the default `frame_parent` accessor); the generalization is the foundation for routing
  conversions between same-dimension kinds through their own parent graph.
- Bumped the project version to 1.2.0.
- Position accessors now return tagged geodesy kinds instead of bare `units` quantities:
  `latitude()`/`longitude()` → `angles::Latitude`/`Longitude`, `azimuth()`/`elevation()` →
  `angles::Azimuth`/`Elevation`, the geodesic bearing accessors → `angles::Azimuth`, `range()` and
  `distance()`/`magnitude()` → `ranges::Euclidean`, and `distanceTo()` → `ranges::Geodesic`.
  Source-compatible for arithmetic and comparison against plain units; code that stored a result in an
  explicit `degrees<>`/`meters<>` should unwrap with `.to<...>()`.
- `PositionAER` gained a fourth template parameter for the origin's altitude unit, decoupling it from the
  slant-range unit (they were previously forced to share one `RangeUnits` parameter -- a range is not a
  height). Defaults leave the `AER` alias unchanged.
- `PositionGeodetic::altitude()` now returns the tagged height kind the datum measures
  (`heights::Ellipsoidal` for an ellipsoid-referenced datum, `heights::Orthometric` for a
  geoid-referenced one) instead of a bare `meters<>`. Source-compatible for arithmetic and comparison
  against plain lengths; code that stored the result in an explicit `meters<>` should unwrap with
  `.to<meters<>>()`.
- `FrameData` constructors are now `constexpr`, so body-frame conversions evaluate at
  compile time.
- CI hardened: warnings-as-errors, Debug configurations, an AddressSanitizer +
  UndefinedBehaviorSanitizer job, a feature-toggle job, and an install-smoke consumer
  that verifies `find_package(coordinates)` resolves against the installed package.

### Fixed

- Fixed the hillshade aspect computation, which used an integer-backed `dimensionless` accumulator and so
  truncated the aspect angle (radians) to whole integers, discarding almost all of the slope-facing-direction
  signal. Aspect is now a floating-point value. The effect is invisible on near-flat terrain (the previous
  golden tile) but corrupts the directional shading across real relief; a new `hillshadeK2` test over the
  Karakoram exercises the aspect term that the flat tile never did.
- Corrected the `IGS08_MSL` datum, which incorrectly referenced `NAD83`.
- Fixed a `PositionAER` constructor typo referencing `sphericalTuple`.
- Removed a false `constexpr` marking on the geoid-undulation lookup-table lookups.
- Corrected a bogus longitude wrap in the ECEF-to-geodetic conversion.
- Fixed broken `find_package` packaging: the install now ships the complete set of
  public headers, pins the `units` dependency version, and links the library even when
  all optional sources are disabled.

## [1.1.0]

- Second tagged release.

## [1.0.0]

- Initial tagged release.

[Unreleased]: https://github.com/nholthaus/coordinates/compare/1.1.0...HEAD
[1.1.0]: https://github.com/nholthaus/coordinates/compare/1.0.0...1.1.0
[1.0.0]: https://github.com/nholthaus/coordinates/releases/tag/1.0.0
