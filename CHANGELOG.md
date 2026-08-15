# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

Modernization on `units` 3.5.1: a reusable rotation-math library, plus latent
numeric and packaging fixes, CI hardening, and licensing/documentation.

### Added

- **Rotation-math library** in `lib/` (`quaternion.h`, `rotation.h`): four fully
  interconvertible, `constexpr`-capable rotation representations — `Quaternion`
  (canonical, Hamilton convention, active rotation), `EulerAngles` (intrinsic Z-Y-X
  Tait-Bryan yaw/pitch/roll), `RotationMatrix` (3x3 direction-cosine), and `AxisAngle`.
  Operations include compose (`operator*`), `conjugate`/`inverse`, `normalized`,
  `rotate` (a vector), `fromTwoVectors`, `slerp`, `identity`, and the `toQuaternion`/
  `toEulerAngles`/`toRotationMatrix`/`toAxisAngle` cross-conversions. Depends only on
  `units`, with a constant-evaluable trig fallback so trig-bearing conversions remain
  usable in a constant-expression context.

### Changed

- Bumped the project version to 1.2.0.
- CI hardened: warnings-as-errors, Debug configurations, an AddressSanitizer +
  UndefinedBehaviorSanitizer job, a feature-toggle job, and an install-smoke consumer
  that verifies `find_package(coordinates)` resolves against the installed package.

### Fixed

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
