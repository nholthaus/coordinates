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
Backed by the Phase-R rotation lib. **Design decision (locked): per-frame body data lives in the frame
TYPE, not the single runtime `FrameData`.** The dispatcher threads ONE `FrameData` up the whole recursion
(same `f` at every level), so distinct per-level offsets for nested bodies must come from the type: each
`BodyFrame` resolves its OWN compile-time-tagged offset+orientation during the type-driven recursion, so
`BodyFrame<Wingtip>` and `BodyFrame<PlaneBody>` compose correctly with zero dispatcher changes — the purest
form of the library's compile-time thesis. **Runtime-varying pose** (a moving vehicle) is carried as the
DATA a `Pose` coordinate VALUE holds and composed with the rotation lib, NOT baked into a frame type.

- **`BodyFrame<Parent, Transform>`** — a frame node whose `convertToBaseFrame` rotate+translates by its
  type-tagged offset AND orientation (a compile-time `Transform` policy / static registry), so a body frame
  sits anywhere relative to its parent at any attitude. **Nests to arbitrary depth** (the LCA recursion
  walks any depth):
  - `BodyFrame<NEDFrame<Datum>, ...>` = a vehicle body carried in local NED.
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

### Phase 1 — bugs + packaging + CI + docs (no API change)  `[x]` (merged into integration/2.0.0)
- [x] `datum.h` `IGS08_MSL` → `IGS08` (no test pinned the old value).
- [x] `positionAER.h` `sphericalTuple` → `SphericalTuple`; added a `scalarOriginConstructor` test that
      instantiates the previously-uncovered ctor (the typo compiled only because it was never instantiated).
- [x] Strip false `constexpr` from the three LUT-backed `geoid::undulation` (extern-const LUT is never a
      constant expression); kept the valid `constexpr` on the ellipsoidal `return 0_m` overload.
- [x] `frameOfReference.h`: removed the bogus longitude wrap — `atan2` already yields (-180, 180], so the
      line was a wrong no-op (SSOT: don't handle a case that can't occur). Truth suite unaffected.
- [x] `src/CMakeLists.txt` `PUBLIC_HEADERS` complete (coordinates.h, coordinates_fwd.h, intersection.h,
      geodesic*Result.h, vector*.h, tileException.h).
- [x] Root VERSION → 1.2.0; `coordinatesConfig.cmake.in` → `find_dependency(units 3.5.1 CONFIG)`.
- [x] **Extra packaging bug found + fixed:** linked the bare `units` target, which only resolved under
      FetchContent; against an installed units the imported target is `units::units`, so the bare name fell
      through to a raw `-lunits` link failure. Now links `units::units` (works both ways). Also set
      `LINKER_LANGUAGE CXX` so the library links with all optional sources off.
- [x] CI: `-Werror`/`/WX`, Debug configs, ASan+UBSan job, feature-toggle job (true all-off), install-smoke
      consumer.
- [x] Docs: MIT `LICENSE` (© 2016 Nic Holthaus), README sync, `CHANGELOG.md`, `Doxyfile`.
- [x] STR-header scrub: every STR file header replaced with the repo's MIT header (7 were self-inflicted
      this session by following the wrong repo's convention; `cacheTest.h` pre-existing). Scrubbed from the
      two feature branches' history via rebase + force-push.
- LOS remains WIP/quarantined (not gated behind a new option this phase; deferred to Phase 6).

### Branch model (locked): one integration branch
All phases merge into `integration/2.0.0` (off `main`); shared fixes live in one place instead of being
threaded across a stack. Open PR: **#6 integration/2.0.0 → main**. The earlier per-phase PRs (#3/#4/#5) were
closed in favor of it. Phases 0, 1, R, 4 are merged; 2/3/5 will merge in as they land.

### Phase R — lib/ rotation + quaternion math  `[~]`
- [x] `lib/quaternion.h` + `lib/rotation.h`: Quaternion / EulerAngles / RotationMatrix / AxisAngle, fully
      interconvertible, `constexpr`-marked, runtime-capable. CMake: `lib/` on the target's public include
      path + installed as a `lib_headers` file-set.
- [x] Ops: compose / inverse / conjugate / normalize / rotate(vector, unit-preserving) / dot /
      fromTwoVectors / slerp / identity.
- [x] `lib/rotationDetail.h`: `if consteval` constexpr-trig fallback (`sin`/`cos`/`sqrt`/`asin`/`atan`/
      `atan2`), machine-precision accurate vs `std::` (~5e-16 over full ranges), `std::` at runtime.
- [x] Full gtest suite (`test/rotationTest.h`, 90 tests, 7 `Rotation*` suites) + `static_assert` round-trip
      proofs; Z-Y-X (intrinsic Tait-Bryan) convention pinned + convention-locked in tests.
- [x] Defect caught by the suite + fixed: `toEulerAngles` gimbal-lock (|pitch|=90) recovered a wrong
      yaw/roll split for coupled inputs. Root-caused (both yaw/roll terms scale by cos(pitch)→0, so the
      generic atan2 is singular), derived the observable-angle closed form (`yaw = 2*atan2(z,w)`, roll=0 —
      valid for both ±90), fixed. Coupled ±90 sweep (882 cases) drift now ~4.5e-8; the two tests upgraded
      from skip to hard rotation-equivalence assertions.
- [x] Full `ctest` green: **419/419** on Linux g++15, zero skips (329 existing + 90 rotation).
- [ ] Windows MSVC build + `ctest` green (running).
- [ ] Doxygen audit of the three lib headers; commit as its own MR.

### Phase 2 — concepts + DRY foundations (behavior-identical)  `[ ]`
- [ ] `void_t` traits → `requires`-constrained specializations.
- [ ] `typedef` → `using`.
- [ ] Delete dead `coordinate_traits.h` logic traits + `cpow` (→ `units::pow`) + `positionXYZ.h`.
- [ ] Re-express ECEF↔ENU/NED rotations onto `lib/rotation.h` `RotationMatrix`.

### Phase 3 — Coordinate<Frame,Tuple> template (collapse duplication)  `[ ]`
- [ ] `coordinate.h` + `is_coordinate`; six positions + three vectors as aliases; delete pure-virtual
      `Point`. Land frame-family by frame-family (ECEF+Geodetic → ENU/NED → AER), each re-running its truth
      test. Bump VERSION to 2.0.0 (alias shim preserves callers).
- [ ] **`PositionXYZ<BodyFrame<Parent, Transform>>` as the FIRST citizen of the new `Coordinate` template**
      (owner decision): a body-axis Cartesian point defined relative to another point + orientation — the
      wingtip-of-an-airplane use case (`PositionXYZ<Wingtip>{3_ft, 0_ft, 0_ft}` = 3 ft out the wingtip's X).
      Aerospace body convention (X forward, Y right, Z down), consistent with `BodyFrame`/`Pose`; converts to
      ECEF/LLA/NED like any position via the frame graph. This is what the deleted-empty `positionXYZ.h`
      always intended (a body-relative point, never the arbitrary-origin unitless sketch it actually held).
- [ ] Fix header self-containment as the position/vector headers are rewritten (`vectorENU.h`/`vectorNED.h`
      currently reference `VectorECEF` without including it — resolved by the collapse, not patched earlier).

### Phase 4 — BodyFrame + Orientation + Pose (nested, offset)  `[~]`
- [x] `src/bodyFrame.h`: `BodyTransform<OffsetX,Y,Z, Yaw,Pitch,Roll>` (offset+orientation as unit-typed
      NTTPs — units 3.5.1 makes unit quantities valid NTTPs, matching the datum-coefficient idiom), aliases
      `Offset<>` (pure translation) + `Attitude<>` (pure rotation), and `BodyFrame<Parent, Transform>` — a
      Cartesian frame-graph node that rotate+translates via the rotation lib. Slots into the LCA solver
      with ZERO dispatcher changes.
- [x] Nested, offsetable body frames PROVEN: `BodyFrame<BodyFrame<NEDFrame<Datum>, Wingtip>, CameraAft>`
      (camera-on-a-wingtip pointing aft). `convert<CameraAft, NEDFrame>` and its inverse round-trip; a
      point 10m ahead of the aft camera maps through camera→wingtip→body correctly.
- [x] `src/pose.h`: `Pose` (6-DOF runtime VALUE — translation + Quaternion). `transformPoint` (rotate then
      translate), `inverse`, `operator*` compose (parentFromLocal = parentFromMid * midFromLocal),
      Euler/Quaternion ctors, `identity`. All constexpr-capable.
- [x] SSOT collapse: a standalone `Orientation` frame node is redundant — a rotation-only body frame is
      `BodyFrame<Parent, Attitude<...>>` and a rotation-only pose is `Pose(zero, quat)`. One mechanism
      covers position, orientation-only, and full 6-DOF; no separate `Orientation` type.
- [x] Two-model consistency PROVEN: the same camera-on-wingtip scenario as nested `BodyFrame`s and as
      composed `Pose`s maps a test point to identical parent coordinates (compile-time frames for static
      mounting, runtime poses for a moving body).
- [x] `bodyFrame.h`/`pose.h` wired into PUBLIC_HEADERS + umbrella `coordinates.h` (umbrella compiles).
- [x] Made `FrameData` a literal type (`constexpr` constructors) so `BodyFrame::convertToBaseFrame`/
      `convertFromBaseFrame` are constexpr-invocable and round-trip at compile time. Additive, no runtime
      change; full suite green after (no regression on the shared type). The end-to-end `convert()` free
      function + dispatchers gain `constexpr` in Phase 5.
- [x] Exhaustive gtest suite (`test/bodyFrameTest.h`, 37 tests, 10 `BodyFrame*`/`Pose*` suites) incl. the
      camera-on-wingtip nesting, nested↔composed-pose equivalence, and `static_assert` compile-time proofs.
- [x] Full `ctest` green on Linux g++15: **456/456**, zero skips (329 + 90 rotation + 37 body/pose).
- [ ] Windows MSVC build + `ctest` green (running).
- [ ] Commit Phase 4 as its own MR.

### Phase 5 — constexpr/consteval sweep + compile-time proofs  `[ ]`
Goal (owner-sharpened): **PARTIAL compile-time folding within a chain** — even when the end-to-end transform
can't be a constant expression, every leg that CAN fold should. Today NONE of the dispatchers or node
transforms are `constexpr`, so nothing folds; fixing that lets a chain fold up to its first genuine runtime
barrier (the extern geoid LUT, or a runtime origin) and no further.
- [ ] Mark the whole dispatch pipeline `constexpr`: `convert<>`, `convertToBase`/`convertFromBase`
      dispatchers, and every node's `convertToBaseFrame`/`convertFromBaseFrame`.
- [ ] **Route the trig-bearing geodesy legs through `lib/rotation.h`'s `if consteval` trig fallback**
      (ECEF↔Geodetic `frameOfReference.h:284-322`, ENU↔ECEF `:395-422`, AER↔NED `:495`): constant-evaluable
      `sin`/`cos`/`atan2`/`asin` when constant-evaluated, `std::` at runtime — so these legs fold when
      constant-evaluated while the runtime path stays bit-for-bit `std::`. (units' own trig is not constexpr.)
- [ ] Leg foldability map — GUARANTEED foldable (pure algebra): Helmert (`helmert.h`), NED↔ENU swap,
      BodyFrame rotate+translate. FOLDABLE VIA FALLBACK: ECEF↔Geodetic, ENU↔ECEF, AER. GENUINE RUNTIME
      BARRIER (never folds): geoid-undulation / Geodetic3D height (extern LUT + throw), a runtime `FrameData`
      origin.
- [ ] `static_assert` proofs of PARTIAL folding: each foldable leg in isolation AND a partial SUB-CHAIN that
      folds even though a full end-to-end wouldn't — e.g. `PositionXYZ<Wingtip> → NED → ECEF → ITRF2014`
      constant-evaluates (BodyFrame + NED↔ENU + Helmert legs), and a chain that hits the geodetic leg folds
      up to it. These fail to compile if a leg silently regresses to runtime — the hard guard.

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
