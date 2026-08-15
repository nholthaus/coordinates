# Roadmap — `coordinates` Modernization / Rearchitect (units 3.5.1 + C++23)

This is the phased, checklist-driven source of truth for modernizing `coordinates`. It drives several
PRs, not one change. Each phase leaves `ctest` green and keeps the public aliases (`ECEF`/`LLA`/`ENU`/
`NED`/`AER`/`Vec*`) compiling for callers.

## Context — why

`coordinates` is a generic, mostly-compile-time, type-driven geodesy/coordinate-transform library layered
on `units`. The 2.3→3.0 port left the good type-level design (a recursive frame graph + least-common-
ancestor solver + a single generic `convert<From,To>`, no N² pairs) sitting on a substrate that
contradicts the library's own thesis:

- A pure-virtual `Point` base (`src/point.h`) puts vtables + virtual dispatch on every conversion in a
  "compile-time generic" library, while an `is_point` concept already captures the identical contract.
- The transform pipeline is `constexpr`-typed but runtime-computed — no `constexpr`/`noexcept` even where
  every input (ellipsoid params, Helmert coefficients, datums) is a compile-time constant.
- ~2,500 lines of duplication across six `position*.h` (`positionXYZ.h` is almost entirely dead-commented)
  and parallel `vector*.h`.
- Concepts are half-migrated (modern `requires` alongside `void_t` SFINAE); dead logic traits; a
  hand-rolled `cpow` duplicating `units::pow`.
- Latent defects + broken packaging: `IGS08_MSL` uses `NAD83`; `sphericalTuple()` called but undefined;
  false `constexpr` on `geoid::undulation`; suspect longitude wrap; `install()` omits headers so
  `find_package` is broken; `VERSION 0.1.0` vs git tags; LOS is WIP/commented out; CI lacks
  `-Werror`/sanitizers/Debug; no `LICENSE`.

**Outcome:** a clean C++23 generic library that keeps the conversion model, replaces the virtual base with
the concept, collapses the position/vector duplication into one `Coordinate<Frame,Tuple>` template, grows
body-local frames + 6-DOF pose + a full rotation-math library, is as `constexpr` as the math allows, and
fixes the bugs/packaging/CI/docs. The numeric truth suite (MATLAB / GeographicLib / EPSG / VDatum) is the
immovable safety net.

## Core design

### `Coordinate<Frame, Tuple, FrameData>` — one template, no base
Shared behavior (ctors, conversion ctor/assignment, arithmetic, geometry, `point()/setPoint/frameData/
setFrameData`) lives once. **No base class, no CRTP.** `is_coordinate` (evolved from `is_point`, minus the
virtual requirement) is the sole contract; `is_point`/`is_vector` become aliases for back-compat. The six
positions + three vectors become alias templates; `coordinates.h` public aliases re-alias unchanged;
`positionXYZ.h` is deleted. Named accessors (`x()`/`latitude()`/`azimuth()`) come from a frame-constrained
free-function / `frame_axes<Frame>` layer, not per-class methods.

### Rotation-math library — `lib/quaternion.h` + `lib/rotation.h`
A first-class, reusable rotation library living in its own `lib/` seam inside coordinates (depends only on
`units`; extractable to its own repo later). Four fully interconvertible representations, all
`constexpr`-marked and runtime-capable:

- **Quaternion** — canonical internal representation (unit quaternion of `dimensionless<>`); no gimbal
  lock, cheap composition, SLERP-ready.
- **EulerAngles** — yaw/pitch/roll (`OrientationTuple`, degrees×3), **intrinsic Z-Y-X (Tait-Bryan)**
  convention pinned in docs + `static_assert`. This is what `FrameData.orientation` already stores.
- **RotationMatrix** — 3×3 direction-cosine matrix of `dimensionless<>`; the bridge the ECEF↔ENU/NED
  rotations re-express onto.
- **AxisAngle** — unit axis + `radians<>` angle; the log/exp bridge to/from quaternion.

Operations (all `constexpr`-marked): compose (`*`), inverse/conjugate, normalize, `rotate(vector)`,
from/to every representation, `fromTwoVectors`, `slerp`, identity.

### Orientation / Pose / BodyFrame — falls out of the frame graph
`FrameData` already stores `origin + orientation + date`; no frame *consumes* `orientation` yet. Body/
attitude/pose are that hook finally used, backed by the rotation lib. `FrameData.orientation` stays the
Euler `OrientationTuple` (API/back-compat); internally it converts to a `Quaternion`.

- **`BodyFrame<Parent>`** — a frame node whose `convertToBaseFrame` rotate+translates by its `FrameData`'s
  per-frame offset (`origin`) AND orientation, so a body frame sits anywhere relative to its parent at any
  attitude. **Nests to arbitrary depth** (the LCA `depth<>` recursion already walks any depth):
  - `BodyFrame<NEDFrame<Datum>>` = a vehicle body carried in local NED.
  - `BodyFrame<BodyFrame<NEDFrame<Datum>>>` = **camera on a wingtip pointing aft** — plane body relative to
    NED, then camera offset to the wingtip and rotated aft relative to the plane body. Camera-frame points
    convert down through wingtip→plane→NED by composing each leg's rotate+translate.
- **`Orientation<Parent>`** — a rotate-only frame node (composes/decomposes rotation, zero translation).
- **`Pose<Parent>` (6-DOF)** — `PoseTuple = tuple<CartesianTuple, OrientationTuple>`; the position leg
  rotate+translates, the orientation leg composes only, via two internal `convert` calls.

### Type-driven dispatch — preserved and strengthened
`convert<From,To>` → `least_common_ancestor` → `convertToBase`/`convertFromBase` stays verbatim (algorithm
chosen from frame *types*, the analog of `units::convert` selecting its branch from `Ratio`/`PiRatio`/
`Translation`). Strengthened with concept-gating, `constexpr` dispatchers/node transforms, and de-`void_t`
frame traits. New nodes (Body/Attitude/Pose) join with zero dispatcher changes.

### constexpr — as constexpr/consteval as we can, runtime never sacrificed
Compile-verified (g++15/C++23, units 3.5.1): `units::sqrt` **is** `constexpr`; `units::cos/sin/tan/atan2`
are **not** (they wrap `std::` and are `noexcept` only). Every type/function is `constexpr`-MARKED —
constant-evaluated when its inputs and operations allow, runtime otherwise (constexpr-capable, never
constexpr-only).

- **Guaranteed compile-time-provable (algebra only):** quaternion/matrix ops, `rotate(vector)`, `sqrt`
  normalization, Helmert transforms, NED↔ENU swap, rotate-only Attitude/Pose composition — get
  `static_assert` round-trip proofs.
- **constexpr-marked, runtime today (trig-bearing):** Euler↔quaternion, axis-angle↔quaternion, ECEF↔
  Geodetic. The rotation lib ships a `constexpr` trig fallback used only in a constant-evaluated context
  (`if consteval`), degrading to `std::` trig at runtime — so these gain compile-time proofs too, with the
  runtime path bit-for-bit `std::`.
- **Honestly runtime (no false constexpr):** Vincenty (iterative), geoid LUT lookups (extern data +
  `throw`), DTED/topography (I/O + threads). The false `constexpr` on `geoid::undulation` is removed.

## Checklist (the SSOT — keep accurate at all times)

Legend: `[ ]` todo · `[~]` in progress · `[x]` done.

### Phase 0 — units 3.5.1 bump + helmert deduction fix  `[~]`
- [x] Pin units 3.5.1 in `CMakeLists.txt` + README.
- [x] Fix `helmert.h` base-class deduction break: concept-constrain the length inputs
      (`template<class L> requires units::traits::is_length_unit_v<L>`), take `L`/`tuple<L,L,L>`, keep the
      output `std::tuple<meters<>,meters<>,meters<>>`. Root cause confirmed by compile repro: under units
      3.4.0+ `meters<>` derives from (is not) `unit<LengthUnits>`, so `tuple<unit<LengthUnits>,...>` can't
      deduce. Verified the fix deduces for `meters<>` AND `feet<>`.
- [x] units 3.5.1 namespace migration: unit-name sub-namespaces are `inline` (`units::length` etc.), so a
      `using namespace units;` inside `inline namespace coordinates` surfaces `meters<>`/`degrees<>` again
      (added to `frameOfReference.h` + `helmert.h`). Aligned the two non-inline `namespace traits` decls
      (`helmert.h`, `horizontalDatum.h`) to `inline` to match the rest.
- [x] Verify units constexpr surface: `units::sqrt` IS constexpr; `units::cos/sin/tan/atan2` are NOT (wrap
      `std::`, `noexcept` only) — compile-proven. Drives the "constexpr-marked + runtime-capable + `if
      consteval` trig fallback" design.
- [x] Full `ctest` green under units 3.5.1: **329/329 pass** (Linux g++15 compile-check build; GEOIDS+DTED
      ON).
- [x] Windows MSVC build + `ctest` green (the canonical gate): **329/329 pass** (cl.exe, Ninja, Release,
      GEOIDS+DTED ON).
- Note (deferred to Phase 2): `vectorENU.h`/`vectorNED.h` aren't self-contained — they reference
  `VectorECEF` without including `vectorECEF.h`; they compile only via the umbrella's include order. Not a
  3.5.1 regression. Make headers self-contained in the DRY sweep.

### Phase 1 — bugs + packaging + CI + docs (no API change)  `[ ]`
- [ ] `datum.h` `IGS08_MSL` → `IGS08` (confirm geoid pairing with owner).
- [ ] `positionAER.h` `sphericalTuple` → `SphericalTuple`/`make_tuple`.
- [ ] Strip false `constexpr` on `geoid::undulation`.
- [ ] `frameOfReference.h` longitude wrap → `wrap180(lon)` (guard against the truth suite; escalate, never
      edit a truth value).
- [ ] Delete dead code (`algorithm.h` locals, dead-commented blocks).
- [ ] `src/CMakeLists.txt` `PUBLIC_HEADERS` complete (add `coordinates.h`, `coordinates_fwd.h`,
      `intersection.h`, `geodesic*Result.h`, `vector*.h`, `tileException.h`; LOS header only when enabled).
- [ ] Root `CMakeLists.txt` VERSION corrected; `coordinatesConfig.cmake.in` → `find_dependency(units 3.5.1
      CONFIG)`.
- [ ] `option(COORDINATES_ENABLE_LOS OFF)` gating `lineOfSight.h` compile/install + the two LOS test files.
- [ ] CI: `-Werror`/`/WX`, Debug config, ASan+UBSan job, feature-toggle job, install-smoke consumer.
- [ ] Docs: `LICENSE` (reconcile the STR header in `cacheTest.h` with owner), README sync, `CHANGELOG.md`,
      `Doxyfile`.

### Phase R — lib/ rotation + quaternion math  `[ ]`
- [ ] `lib/quaternion.h` + `lib/rotation.h`: Quaternion / EulerAngles / RotationMatrix / AxisAngle, fully
      interconvertible, `constexpr`-marked, runtime-capable.
- [ ] Ops: compose / inverse / conjugate / normalize / rotate(vector) / fromTwoVectors / slerp / identity.
- [ ] `if consteval` constexpr-trig fallback (`sin`/`cos`/`atan2`/`asin`/`acos`) — constexpr path in a
      constant-evaluated context, `std::` at runtime.
- [ ] Full gtest suite + `static_assert` round-trip proofs (Z-Y-X Tait-Bryan convention pinned).

### Phase 2 — concepts + DRY foundations (behavior-identical)  `[ ]`
- [ ] `void_t` traits → `requires`-constrained specializations.
- [ ] `typedef` → `using`.
- [ ] Delete dead `coordinate_traits.h` logic traits + `cpow` (→ `units::pow`) + `positionXYZ.h`.
- [ ] Re-express ECEF↔ENU/NED rotations onto `lib/rotation.h` `RotationMatrix`.

### Phase 3 — Coordinate<Frame,Tuple> template (collapse duplication)  `[ ]`
- [ ] `coordinate.h` + `is_coordinate`; six positions + three vectors as aliases; delete pure-virtual
      `Point`. Land frame-family by frame-family (ECEF+Geodetic → ENU/NED → AER), each re-running its truth
      test. Bump VERSION to 2.0.0 (alias shim preserves callers).

### Phase 4 — BodyFrame + Orientation + Pose (nested, offset)  `[ ]`
- [ ] New frame-graph nodes consuming `FrameData.orientation` via the rotation lib; camera-on-wingtip
      nesting works; pose (rotate+translate) vs orientation-only (compose) verified.
- [ ] Tests: body-in-NED round-trips, nesting, pose vs orientation-only, `static_assert` round-trips.

### Phase 5 — constexpr/consteval sweep + compile-time proofs  `[ ]`
- [ ] Mark the honest-constexpr surface; assemble the `static_assert` round-trip suite; type-driven
      dispatch becomes literally compile-time-evaluable where the math allows.

### Phase 6 (stretch) — LOS  `[ ]`
- [ ] Finish the terrain intersector, or keep quarantined behind the flag.

## Verification
- Every phase: full `ctest` green (the MATLAB/GeographicLib/EPSG/VDatum truth suite is immovable), on the
  hardened CI matrix (Release+Debug × gcc/clang/msvc, `-Werror`, ASan/UBSan, feature-toggle, install-smoke).
- Rearchitect phases: per-frame-family numeric diff vs pre-migration output; grep-verify no `virtual`/vtable
  remains on the coordinate types.
- New capability: Body/Orientation/Pose round-trip tests + `static_assert` compile-time round-trips.
- Packaging: the install-smoke consumer (`find_package(coordinates)` + `#include <coordinates.h>` + one
  `convert`) builds against the installed package.

## Risks (hardest first)
1. Pose-through-frame-graph semantics through arbitrary LCA routes + nested `BodyFrame` — pin the rotation
   convention (Z-Y-X Tait-Bryan, active vs passive; cf. the Helmert convention note in `helmert.h`) in docs
   + asserts; validate vs hand-computed + MATLAB attitude round-trips before trusting nesting.
2. Collapsing ~2,500 lines without behavior drift — per-frame-family migration gated on each truth test;
   numeric-diff generated conversions vs pre-migration output.
3. constexpr trig — resolved: everything `constexpr`-marked + runtime-capable; algebra core is
   compile-time-provable; trig-bearing math gains proofs via the `if consteval` fallback; runtime stays
   bit-for-bit `std::`.
4. Not breaking the numeric truth suite — a "bug fix" that shifts a truth value escalates to the owner,
   never edits the test.
5. Accessor ergonomics vs single storage — frame-constrained free functions + thin alias wrappers.
6. License contradiction — MIT headers vs the STR claim in `cacheTest.h` vs no `LICENSE`; resolve with
   owner before publishing.
