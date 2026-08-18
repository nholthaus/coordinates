# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

Modernization on `units` 3.6.1: a reusable rotation-math library, plus latent
numeric and packaging fixes, CI hardening, and licensing/documentation.

### Added

- **Strongly-typed distances** (`src/ranges.h`): a slant range (observer to target), a geodesic distance
  (along the ellipsoid surface), and a Euclidean distance (3-D straight-line magnitude) are now distinct
  `units::kind` types in the `ranges::` namespace. All lengths but measuring different paths, so mixing them
  is a compile error. `PositionAER::range()` → `ranges::Slant`; `distanceTo()` / `GeodesicInverseResult::distance()`
  → `ranges::Geodesic`; the `distance()` / `magnitude()` accessors on every position → `ranges::Euclidean`.
- **Strongly-typed angles** (`src/angles.h`): latitude, longitude, azimuth, elevation, and the three
  orientation angles (yaw/pitch/roll) are now distinct `units::kind` types in the `angles::` namespace.
  They are all `degrees<>` but semantically incommensurable, so mixing them is a compile error — most
  importantly, a latitude and an azimuth (which previously shared both a type and a `SphericalTuple` slot)
  can no longer be silently confused. `PositionGeodetic::latitude()`/`longitude()`,
  `PositionAER::azimuth()`/`elevation()`, and every geodesic bearing accessor now return their tagged kind;
  AER azimuth, geodesic bearings, and any North-referenced heading unify as one `angles::Azimuth`.
- **Strongly-typed heights** (`src/heights.h`, `src/verticalDatum.h`): ellipsoidal (HAE) and orthometric
  (MSL) heights are now distinct `units::kind` types (`heights::Ellipsoidal`, `heights::Orthometric`),
  joined by a `heights::Undulation` kind for the geoid separation. Both are lengths but mixing them in
  arithmetic or comparison is a compile error; the only bridge is an explicit conversion. A plain length
  still constructs into a height implicitly, so existing construction is unaffected. `heights::kind_for<Datum>`
  deduces which height a datum measures from its vertical reference.
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

- Bumped the project version to 1.2.0.
- Position accessors now return tagged geodesy kinds instead of bare `units` quantities:
  `latitude()`/`longitude()` → `angles::Latitude`/`Longitude`, `azimuth()`/`elevation()` →
  `angles::Azimuth`/`Elevation`, the geodesic bearing accessors → `angles::Azimuth`, `range()` →
  `ranges::Slant`, `distanceTo()` → `ranges::Geodesic`, and `distance()`/`magnitude()` → `ranges::Euclidean`.
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
