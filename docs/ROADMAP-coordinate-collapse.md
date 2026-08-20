# Roadmap — the `Coordinate<Frame, Tuple>` collapse

> The DRY end-state (roadmap-modernization.md Phase 3): replace the five copy-pasted position classes and
> three vector classes with ONE `Coordinate` template body; re-alias `ECEF`/`LLA`/`ENU`/`NED`/`AER`/`Vec*`
> back to it, unchanged for every caller and all 479 tests. Per-frame named accessors move to a
> `frame_axes<Frame>` trait. **Pure alias + trait form** (ratified). Grounded in the shared-vs-per-frame audit.
> Lands frame-family by family, each gated on its truth test and an MSVC parity check.

---

## Why

The audit measured the duplication: ~1,650 lines of byte-identical (or names-only) position member scaffolding
across 5 classes collapse to a single ~350-line `Coordinate` body, and ~600 lines of vector scaffolding across
3 classes collapse to ~200. The frame-conversion *logic* is already DRY (one free `coordinates::convert` → LCA);
what duplicates is the per-class member surface. This collapse writes it once, and — the forward-looking payoff —
every future member (a new measurement, a new conversion) is then written once instead of eight times.

## The three complications the audit surfaced (and how the design solves each)

### 1. Heterogeneous storage
ECEF/ENU/NED and all vectors are homogeneous (three of one length unit). But **Geodetic is (angle, angle,
length)** and **AER is (angle, angle, range)** — two unit families per point. So the single storage scheme is a
`std::tuple<C0, C1, C2>` whose element types come from the frame's `tuple_type` (already `SphericalTuple` =
`tuple<degrees<>, degrees<>, meters<>>` etc.), NOT a uniform `unit_type[3]`. `Coordinate` stores its
`tuple_type` directly (plus `FrameData`); `point()` returns it, `setPoint` assigns it — trivially uniform once
storage IS the tuple. (Named-scalar storage was the incidental reason the bodies looked per-frame; tuple storage
erases that.)

### 2. Per-frame named accessors → `frame_axes<Frame>`
`frame_axes<Frame>` maps each axis to `{ name, return-type }`. Crucially the return type is NOT always the raw
slot unit: Geodetic `altitude()` → `heights::kind_for<Datum>`, `latitude()` → `angles::Latitude`; AER
`azimuth()` → `angles::Azimuth`, `range()` → `ranges::Euclidean`; while ECEF/ENU/NED return the raw
`*_unit_type`. The trait therefore encodes a per-axis *return wrapper*, and the accessors are exposed as
**frame-constrained free functions** (`latitude(const Coordinate&)`, `x(const Coordinate&)`, …) plus a thin
member shim so `p.latitude()` reads unchanged. Geodetic/AER also carry their extra tagged verbs
(`geocentricLatitude`, `toEllipsoidHeight`/`toOrthometricHeight`) in their `frame_axes` specialization.

### 3. The forward-declaration blocker (the load-bearing constraint)
`coordinates_fwd.h:39-43` forward-declares `PositionGeodetic` / `PositionECEF` as **`class` templates**, and
`vectorECEF.h:57-61` forward-declares `VectorENU` / `VectorNED`. You cannot forward-declare a name as `class`
then define it as a `using` alias template. **Resolution:** convert those forward references to alias-template
form. `coordinates_fwd.h` becomes:
```cpp
template<class Frame, class Tuple> class Coordinate;                    // the one primary template
template<class Datum, template<class> class = meters, typename = double>
using PositionECEF = Coordinate<coordinateFrames::ECEFFrame<...>, CartesianTuple>;   // alias
// ... PositionGeodetic, and the four vector aliases likewise
```
and every consumer that only *uses* them by value (`algorithm.h`, `lineOfSight.h`, the vector headers, all
tests) is unaffected — the census found NO base-class use, NO external friend/specialization, only value/alias
uses (179 sites, all safe). The only edits are the fwd-decl declarations themselves + the self-friend inside
`Coordinate` (one `template<class,class> friend class Coordinate;` replacing the five cross-specialization
self-friends).

## The converting-ctor branch (the one behavioral fork)
`Coordinate`'s converting ctor / `operator=` branches `if constexpr (frame_traits<Frame>::is_local)`:
- **non-local (ECEF/Geodetic):** `m_frameData = point.frameData(); coordinates::convert(point, *this);` (one hop).
- **local (ENU/NED/AER):** adopt origin if `NULL_ORIGIN`; then the pairwise **fast paths** (NED↔ENU axis-swap,
  ENU/NED→AER trig) as `if constexpr` on the source frame; else the general two-hop through a
  `PositionECEF<Datum,…>` intermediate.

`is_local` is a new frame trait (`frame_traits<Frame>::is_local`, true for ENU/NED/AER — the tangent frames whose
`convertToBaseFrame` reads `f.origin`). The pairwise fast paths live in a `convert_fast_path<From,To>`
customization (default: no fast path → fall through to the two-hop), keyed on the source/dest frame pair — the
one genuinely non-uniform piece, isolated to that hook rather than smeared across the body.

## Phasing (each phase `ctest`-green + MSVC-checked; aliases keep every caller compiling)

- **C-0 — scaffolding, no behavior change.** Add `frame_traits<Frame>::is_local`, the `frame_axes<Frame>`
  trait (accessor name/return-type table), and the `convert_fast_path<From,To>` hook, alongside the existing
  classes. Nothing collapses yet; prove the traits compile + a unit test of `frame_axes` for each frame.
- **C-1 (DONE — landed C-1a/b/c/d).** `Coordinate<Frame,Tuple,FrameData>` with tuple storage + the full shared
  member surface (point/setPoint/frameData, converting ctor/`operator=` with the `is_local` branch + the
  `convert_fast_path` shortcuts, arithmetic, the forwarders, `operator<<`) + the `AxisAccessors<Frame,Derived,
  Tuple>` CRTP-lite mixin giving each frame its named member accessors with tagged returns. Fully generic: axis
  types come from the `Tuple` (`std::tuple_element_t`), so an `inches`-backed ECEF instantiates and converts.
  The old classes are untouched; `Coordinate` is proven by direct instantiation. gcc + MSVC green.
- **C-2..C-6 — migrate frame-family by family (the fresh focused pass).** Two coupled sub-steps, then the
  per-family aliasing:
  - **C-2a — break the `algorithm.h` forward-declaration cycle.** `algorithm.h` names `PositionGeodetic<Datum>`
    / `PositionECEF<Datum>` as params/returns in the geodesic + intersection functions (`geodesicInverse`,
    `geodesicDirect`, `geodesicDistance`, `initialBearing`/`finalBearing`, `intersectEllipsoid`,
    `isLineOfSight`) and forward-includes `coordinates_fwd.h`. Because `Coordinate` includes `algorithm.h`, a
    pure `using` alias of these names cannot be forward-declared (a cycle). Resolve by making those functions
    **generic over the point type** (template on the geodetic/ECEF point, constrained by concept) rather than
    naming the concrete class — so no forward declaration is needed. The one snag: `geodesicDirect` *constructs*
    a `PositionGeodetic<Datum>` result (and `GeodesicDirectResult<PositionGeodetic<Datum>>`); give it the
    destination point type as a template parameter (defaulted) or a small factory so it builds the alias
    without naming it pre-definition.
  - **C-2b — convert `coordinates_fwd.h` (+ `vectorECEF.h`'s `VectorENU`/`VectorNED` fwd-decls) to alias
    templates.** Once C-2a removes the fwd-*need*, replace the `class` forward-declarations with the `using`
    alias templates (or drop `coordinates_fwd.h` entirely if nothing else needs it).
  - **C-3..C-6 — alias each family.** For ECEF, Geodetic, ENU/NED, AER (and the three vectors): replace the old
    class with `template<class Datum, template<class> class Units = meters, typename T = double> using
    PositionECEF = Coordinate<ECEFFrame<...>, std::tuple<Units<T>,Units<T>,Units<T>>>;` (each frame's own tuple
    element types), delete the old class body, run that family's truth test + MSVC. One family per commit.
    Order: ECEF first (simplest, LCA root), then Geodetic, then ENU/NED (axis-swap pair), then AER (trickiest —
    origin ctors + `fromObserver` + `origin()`/`setOrigin` + tagged az/el/range), then the vectors (a separate
    hierarchy: `vector()`/`setVector`, `vector_tag`, no `frame_data` on the free ECEF vector). The
    origin-carrying ctors (`PositionENU(e,n,u,origin)`, `fromObserver`, `origin()`, `setOrigin`) are the
    local-frame surface the shared body does not yet host — add them to `Coordinate` (guarded by
    `is_local_frame`) or a local-frame accessor mixin during C-5/C-6.
- **C-7 — delete the `Point` ABC.** Once all five positions are `Coordinate` aliases, remove the pure-virtual
  `Point` base (concepts are structural, nothing stores `Point*`); add the `is_coordinate` concept and make
  `is_point`/`is_vector` aliases of it for back-compat. Grep-verify no `virtual`/vtable remains on the
  coordinate types.

## Migration findings (from a spike; must be handled in the fresh pass)

A first migration attempt (reverted to keep the foundation green) surfaced three concrete facts the pass
must design for — none are blockers, but each needs a real decision, which is why the migration is its own
focused effort, not a mechanical alias swap:

1. **An alias template to a forward-declared `Coordinate` is legal C++** (verified). So `coordinates_fwd.h`
   can forward-declare `template<class,class,class> class Coordinate;` and define the position aliases against
   it; no `algorithm.h` include cycle results, because consumers only name the aliases by value/reference.
   The `class` forward-declaration was the only thing that couldn't become an alias — the alias itself is fine.
2. **Aliases break `Datum` deduction in function signatures.** `intersectEllipsoid`, `isLineOfSight`, and the
   geodesic functions deduce `Datum` from a `PositionECEF<Datum>`/`PositionGeodetic<Datum>` parameter — which
   STOPS working once those are aliases (the alias transformation is not invertible by template argument
   deduction). Each such function must be re-templated on the point type `P` with `Datum` derived internally
   (`point_traits<P>::reference_frame::datum_type`).
3. **An ECEF point cannot recover a full 3-D `Datum`** — `ECEFFrame<HorizontalDatum>`'s `datum_type` is the
   *horizontal* datum, but `intersectEllipsoid`/`Intersection<Datum>` were parameterized on the full datum.
   This is a genuine semantic question: an ECEF-based intersection needs an *ellipsoid*, not a 3-D datum.
   Resolve by parameterizing `Intersection` (and these functions) on the ellipsoid or horizontal datum
   recoverable from the point, not the full datum — a small `Intersection` API adjustment.

The foundation additions that ARE sound and can be reused verbatim in the pass: a variadic
`setPoint(Components...)` on `Coordinate` (converts each component into the tuple's stored unit, needed by the
arithmetic recompute paths), and the point-generic re-templating pattern for the `algorithm.h` functions.

## Ratified decisions (owner, during the fresh pass)

- **Finding 3 — `Intersection` datum.** `Intersection<Datum>` and `intersectEllipsoid`/`isLineOfSight` are
  parameterized on the **horizontal** datum recoverable from the ECEF point's frame
  (`frame_traits<point_traits<P>::reference_frame>::datum_type`), not the full 3-D datum. An ellipsoid
  intersection is a purely horizontal-datum/ellipsoid quantity — no vertical component. `Intersection`'s
  static_assert relaxes from `is_datum` to `is_horizontal_datum`. Done + `-Werror` syntax-clean.
- **Frame-specific verbs → `AxisAccessors` mixin.** The ECEF verbs (`intersectRay`, `hasLineOfSightTo`) and
  the geodetic verbs (`inverseTo`, `distanceTo`, `initialBearingTo`, `finalBearingTo`, `bearingTo`,
  `geodesicDistanceTo`, `euclideanDistanceTo`, `slantRangeTo`, `destination`) live in each frame's
  `AxisAccessors<Frame, Derived, Tuple>` specialization — frame-scoped, zero cost on frames that lack them,
  member-call surface preserved. Same mechanism as the named axes.
- **`PositionGeodetic` signature simplified:** `template<class Datum, template<class> class AngleUnits =
  degrees, template<class> class HeightUnits = meters> using PositionGeodetic = Coordinate<
  Geodetic3DFrame<Datum>, std::tuple<AngleUnits<double>, AngleUnits<double>, HeightUnits<double>>>`. The
  trailing `T` storage-type parameter is DROPPED (fixed to `double`); `<Datum>`, `<Datum, radians>`, and
  `<Datum, degrees, feet>` all keep working. The two explicit-`T` sites (`vectorTest.h`,
  `positionAER.h`'s `origin_type` / `vectorENU.h`'s `fromAER`) migrate to the 3-param form.

## Verification (every phase)
Full `ctest` green (the MATLAB/GeographicLib/EPSG/VDatum truth suite is immovable) AND a local MSVC `cl.exe`
build (c++23, warnings-as-errors) — the parity check that the tagging campaign learned the hard way. Per-family
numeric diff vs pre-collapse output where a body is rewritten. No unjustified skips.

## Risk register (hardest first)
1. **AER's fast paths + origin + tagged az/el/range** — the least-uniform class; the `convert_fast_path` hook
   and `frame_axes` must carry its ENU/NED→AER trig and its `ranges::Euclidean` range return. Land it LAST,
   after the pattern is proven on the simpler frames.
2. **The fwd-decl → alias conversion** — a compile-wide change; do it in C-1 with the `Coordinate` primary
   template in place, verify `algorithm.h`/vectors/tests still resolve.
3. **MSVC parity** — heterogeneous-tuple storage + `if constexpr` branches + `frame_axes` return-type
   metaprogramming are exactly the constructs MSVC treats more strictly (just bit us: C2065 self-referential
   constexpr, C3615 false-constexpr). MSVC-check EVERY phase, not just at the end.
4. **Behavior drift in the collapse** — the per-class fast paths and AER's `isSame(origin)→zero` special case
   must survive verbatim; the per-family truth test is the gate.
