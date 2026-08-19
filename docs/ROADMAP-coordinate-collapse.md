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
- **C-1 — the `Coordinate` body.** Introduce `Coordinate<Frame,Tuple>` with tuple storage + the full shared
  member surface (point/setPoint/frameData, converting ctor/`operator=` with the `is_local` branch, arithmetic,
  the forwarders) + the free-function accessors driven by `frame_axes`. Do NOT delete the old classes yet.
- **C-2..C-6 — migrate frame-family by family.** For each of ECEF, Geodetic, ENU/NED, AER (and the vectors):
  re-alias the position name to `Coordinate<…>`, delete the old class body, run that family's truth test +
  MSVC build. One family per commit, each independently green. Order: ECEF first (simplest, the LCA root),
  then Geodetic, then ENU/NED (the axis-swap pair), then AER (the trickiest — origin + fast paths + tagged
  az/el/range), then the three vectors.
- **C-7 — delete the `Point` ABC.** Once all five positions are `Coordinate` aliases, remove the pure-virtual
  `Point` base (concepts are structural, nothing stores `Point*`); add the `is_coordinate` concept and make
  `is_point`/`is_vector` aliases of it for back-compat. Grep-verify no `virtual`/vtable remains on the
  coordinate types.

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
