# Roadmap — Strongly-Typed Geodesy Quantities (units 3.6 `kind`)

> Source-of-truth checklist for the "tag every semantically-distinct geodesy value as a
> `units::kind`" effort on `coordinates`. Ratified end-state; lands in three independently-shippable
> phases (A → B → C), each leaving `ctest` green. Repo: `/e/workspace/coordinates`, branch
> `integration/2.0.0`. Pinned on units 3.6.1 (`CMakeLists.txt:84`).

---

## Why (the audit verdict)

Six independent read-only audits of `src/` established that the library tells one systemic lie in two
dialects:

- **One `degrees<>` pretends** latitude, longitude, azimuth, elevation, yaw, pitch, and roll are the same
  type.
- **One `meters<>` pretends** ellipsoidal height, orthometric height, geoid undulation, ECEF/ENU/NED axes,
  slant range, geodesic distance, Euclidean distance, and curvature radii are the same type.
- **One `SphericalTuple` = `tuple<degrees<>, degrees<>, meters<>>` is the *same C++ type* for
  `(lat, lon, height)` AND `(azimuth, elevation, range)`** (`frameOfReference.h:74`). Slot 0 is a latitude
  in a geodetic frame and an azimuth in an AER frame; the generic dispatcher's `std::get<0>` — the chokepoint
  every conversion pivots through — cannot tell them apart. A 315° azimuth is silently consumable as a 315°
  latitude, a 40 km slant range as a 40 km altitude, with zero compiler resistance.

Tagging does not *add* complexity; it makes the type system state distinctions that already exist
physically. The correct SSOT model collapses special cases: the honest kind set is ~9 length-kinds +
~7 angle-kinds, and the conversions between same-dimension kinds turn out to be the **same LCA graph** the
frame system already runs — proven, because `verticalDatum.h`'s ortho↔ellipsoid walk is the frame-LCA
recursion hand-rolled a second time.

## The honest kind taxonomy

**Length kinds (9):**

1. `heights::Ellipsoidal` (HAE) — height above the reference ellipsoid *(exists)*
2. `heights::Orthometric` (MSL) — height above the geoid *(exists)*
3. `heights::Undulation` (N) — signed ellipsoid↔geoid separation; the ONLY legal bridge between 1 and 2
4. `CartesianComponent` — ECEF/ENU/NED/body axis + Helmert translation (one kind; axes interconvert by
   rotation only, no bias). `CartesianTuple` stays one homogeneous kind-set.
5. `ranges::Slant` — AER straight-line range from an observer origin
6. `ranges::Geodesic` — along-surface great-circle distance
7. `ranges::Euclidean` — 3-D straight-line ‖Δ‖ (stop returning this as `height_unit_type`)
8. `EllipsoidParameter` — datum-defining lengths + radii of curvature (a, b, N_φ)
9. `RayParameter` — along-ray parametric distance (LOS `t`, bracketing steps)

**Angle kinds (~7 core, +2 optional):**

1. `angles::Latitude` (geodetic) — `[-90, 90]`
2. `angles::Longitude` — with a wrap convention (unify the `(-180,180]` vs `[0,360)` split)
3. `angles::Azimuth` / bearing — `[0,360)` from North (unifies AER azimuth + geodesic fwd/back bearing +
   sun azimuth — genuinely one kind)
4. `angles::Elevation` — above local horizon `[-90,90]` (internal +90° colatitude bridge stays internal)
5. `angles::Yaw`, 6. `angles::Pitch`, 7. `angles::Roll` — orientation
8. *(optional)* `angles::Geocentric` latitude — **a design hole today: never computed.** A NEW conversion
   (Phase B), not a rename.
9. *(optional)* delta-angle / grid-resolution kind; Helmert rotation stays `milliarcseconds`.

## Architecture — one type-level LCA graph, two value-level dispatchers

The frame LCA (`depth<>`, `least_common_ancestor<>`, `frameOfReference.h:722-833`) and the vertical-datum
`base_datum` walk (`verticalDatum.h:129-164`) are provably the same recursion:

| frame machinery | vertical-datum machinery |
|---|---|
| parent: `base_frame_type` | parent: `base_datum` |
| per-hop physics: `convertToBaseFrame` | per-hop physics: `+ correctionValue(lat,lon)` |
| root test: `base_frame_type == self` | root test: `is_ellipsoid` |
| termination overload split by `requires` | termination overload split by `requires` |

- **Type level unifies.** `depth<>` / `least_common_ancestor<>` already take a predicate-`Trait` hook; add a
  `Parent`-accessor template parameter (replace `frame_traits<U>::base_frame_type` with `Parent<U>::type`,
  root test `same_as<Parent<U>::type, U>`). One algorithm serves both frames and kinds.
- **Value level stays two dispatchers — context asymmetry.** A frame hop consumes `FrameData` (where the
  frame *is*); a kind hop consumes the *sibling coordinates of the same point* (lat/lon, to look up N). Do
  NOT pollute `FrameData`; keep a `(lat,lon)`-carrying kind-context. The `verticalDatum.h` walk is the
  degenerate case (root hard-coded to ellipsoid); geodetic↔geocentric latitude and slant↔geodesic distance
  are the cases that graduate it to true kind-A↔kind-B routing through a nearest common kind.

**Uniform node pattern:** erase-to-`double` at the node's math core (the trig already does this —
`geodesicInverse`, `rotate`, `isLineOfSight` all `.to<double>()` on entry), re-tag each output slot to its
own kind. The kinds guard the node *interface*; the double math inside runs unchanged.

**Boundary-permissive kinds (the key lever):** each kind must (a) subtract/compare against its underlying
unit (`kind - meters<>`, `meters<> == kind` compile), (b) keep `.to<double>()` / `.to<meters<>>()`, (c) be
implicitly constructible from its underlying unit, and (d) satisfy `is_angle_unit_v`/`is_length_unit_v`.
`units::kind` already keeps `abbreviation()` = the wrapped unit's (no tag suffix), so ostream tests are
unaffected. This single property auto-passes ~146 accessor-level test assertions with no edits, and keeps
`LLA p(34_deg, -118_deg, 100_m)` construction valid.

---

## Phase A — kinds header + tagged accessors + honor the README (SHIP)

Zero of the 24 tuple-tagging break-sites are touched here (all 24 are internal conversion machinery). Tuples
stay plain `meters<>`/`degrees<>`; only the public *boundary* is tagged.

- [x] `src/heights.h` — the height kinds + `kind_for<Datum>` (boundary-permissive), registered in
      `src/CMakeLists.txt` `PUBLIC_HEADERS` and `#include`d from `coordinates.h` before the position headers.
      (One header per dimension: `angles.h` lands in commit 2, `ranges.h` in commit 3 -- no monolithic
      catch-all header.)
- [x] `src/angles.h` — the seven angle kinds (`Latitude`, `Longitude`, `Azimuth`, `Elevation`, `Yaw`,
      `Pitch`, `Roll`; AER azimuth + geodesic bearing + sun azimuth unified as one `Azimuth`). Tagged
      `PositionGeodetic::latitude()/longitude()`, `PositionAER::azimuth()/elevation()`, and every geodesic
      bearing accessor. `test/anglesTest.h` proves the lat!=azimuth distinction. (committed).
- [x] `src/ranges.h` — the three distance kinds (`Slant`, `Geodesic`, `Euclidean`). Tagged
      `PositionAER::range()` → `Slant`, `distanceTo()`/`GeodesicInverseResult::distance()` → `Geodesic`,
      and every `distance()`/`magnitude()` → `Euclidean`. Decoupled `PositionAER`'s origin-altitude unit
      from its slant-range unit (a fourth template parameter). `test/rangesTest.h` proves the three stay
      apart and the AER split. (committed).
- [ ] Tag public accessors + matching setters/ctor params:
      `LLA::latitude()/longitude()/altitude()`, `AER::azimuth()/elevation()/range()`,
      `distance()/magnitude()` family, geodesic `distanceTo()/initialBearingTo()/finalBearingTo()` and the
      `GeodesicInverseResult`/`GeodesicDirectResult` bearings/distance, `DTED::orthometricHeight`,
      `verticalDatum` converters (already tagged), geoid `undulation`.
- [ ] Deliver the 4 README-advertised-but-missing APIs, tagged: `toOrthometricHeight()`/`toEllipsoidHeight()`
      members (README:631), `PositionAER::fromObserver(observer,target)` (README:560,845),
      `PositionXYZ` + `x()/y()/z()` (README:251,441-449), `distanceSquared(a,b)` (README:813).
- [ ] Decision: whether `ECEF::x()/y()/z()` etc. (Cartesian axes) tag now or defer to C. (Lean: keep plain
      until C, since a lone axis component has no cross-kind hazard the tuple-tag doesn't already cover.)
- [ ] Full `ctest` green; confirm the ~146 accessor assertions pass unedited via boundary-permissiveness.
- [ ] CHANGELOG `[Unreleased]`: `### Added` strong-typed kinds + `### Changed` accessor return types.
      Source-breaking accessor change ⇒ 2.0.0 (already the integration branch).

## Phase B — unify type-level LCA + the NEW conversions

- [ ] Generalize `depth<>` / `least_common_ancestor<>` over a `Parent` accessor (keep the `Trait` hook);
      re-express the frame graph and the vertical-datum walk on the one algorithm.
- [ ] Add the kind-conversion dispatcher with a `(lat,lon)` context (parallel to the frame dispatcher).
- [ ] NEW conversions as kind edges: `angles::Latitude` ↔ `angles::Geocentric` (via the ellipsoid; the
      design hole), `ranges::Slant` ↔ `ranges::Geodesic`. Truth-data tests for each.
- [ ] Full `ctest` green.

## Phase C — tag the internal tuple slots (frame-graph type-checking)

The 24 hard break-sites. ~500+ `std::get<>` test assertions churn. Land frame-family by frame-family, each
gated on its truth test.

- [ ] Split the shared tuples: `GeodeticTuple<Lat,Lon,Height-kind>`, `AERTuple<Az,El,Range-kind>`, tagged
      `CartesianTuple` — kills the load-bearing `(lat,lon,h)`≡`(az,el,range)` pun.
- [ ] Rework the 24 sites on the uniform erase-to-double / re-tag pattern. Hardest three first, eyes-open:
  1. `AERFrame::convertFromBaseFrame` (`frameOfReference.h:511-527`) + `PositionAER` ENU/NED fast-paths +
     ECEF-intermediate (`positionAER.h:307-368`).
  2. `rotate<LengthUnit>` homogeneity (`rotation.h:163-172`, `quaternion.h:155-175`) → heterogeneous
     `CartesianTuple` in/out; ripples to `bodyFrame.h`/`pose.h`.
  3. `ENUFrame::convertToBaseFrame` (`frameOfReference.h:395-399`) four-kind sum.
- [ ] Make `dispatchers` same-frame pass-throughs tag-forwarding (`return p;`) (`frameOfReference.h:850,874`).
- [ ] Full `ctest` green with the updated `std::get<>` assertions.

---

## Known cleanups surfaced by the audit (fold in opportunistically)

- **Dead/broken code:** `lineOfSight.h` references `hit->surfaceRange` but `TerrainHit` has no such member
  (untested 3-arg `terrainIntersection` path). `losTest.h` / `lineOfSightTest.h` are not wired into any
  `test/suites/*.cpp` (never built). LOS is `COORDINATES_ENABLE_LOS`-gated.
- **`PositionGeodetic::distance()` returns `height_unit_type`** (`positionGeodetic.h:253`) — mistypes a
  straight-line distance as an altitude; fix under kind 7.
- **`PositionAER` forces origin-altitude and range to the same `RangeUnits`** (`positionAER.h:80`) — must
  split (height vs slant range) in Phase A/C.

## Hard-won lessons (apply across all phases)

- **`requires{}` cannot prove a mixing is ill-formed.** `units::kind`'s cross-tag and plain±kind operators
  exist as *overloads* whose bodies are `static_assert(dependent_false...)`. So `mix` is well-formed to
  overload resolution (`requires{ a + b }` is `true`) and only detonates on instantiation. A
  `static_assert(!requires{ e + o })` regression guard therefore FAILS to compile. Test the sound,
  compile-time-testable facts instead: distinct types (`!is_same_v`), differing tags (`A::tag() != B::tag()`),
  and plain-constructibility (`is_constructible_v<Kind, PlainUnit>`). The mixing-is-a-compile-error guarantee
  is real but is proven by the negative test being *absent from the build*, not by a `requires`.
- **`to_linearized()` is NOT a base-unit normalizer.** It linearizes the numeric *scale* (e.g. log→linear),
  not the unit: `kilometers.to_linearized()` is 30, not 30000. To compare a tagged accessor against a
  plain-unit literal across units, UNWRAP the kind to its plain unit (`.to<unit_type>()`) and let the typed
  operator reconcile — see `test/gtest_units.h` `as_plain`. Reducing to `.to<double>()` is unit-relative and
  wrong for cross-unit compares.
- **Test macro is boundary-permissive via `as_plain`.** `gtest_units.h` unwraps any kind to its plain unit
  before the strongly-typed compare/subtract, so plain-vs-plain behavior is unchanged and tagged-vs-plain
  reconciles units natively. This is why ~146 accessor assertions pass with no call-site edits.

## Verification (every phase)

Full `ctest` green on the hardened CI matrix. Phase A proven by the accessor assertions passing unedited +
the 4 new APIs' tests. Phase B by the new-conversion truth-data tests. Phase C by per-frame-family numeric
diff vs pre-migration output. No unjustified skips.
