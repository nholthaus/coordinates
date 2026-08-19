# API-form policy — conversions vs. two-point measurements

> The open question "constructor vs free function vs member" resolved into a coherent, ergonomic,
> discoverable policy. The key insight: there are **two independent axes**, and they are apples and
> oranges — a conversion re-expresses one value; a measurement computes a relationship between two.
> Each gets its own policy. Goal: maximally ergonomic + discoverable (leans members) without DRY
> violations. Grounded in the `units` idiom the library is built on and the `Coordinate<Frame,Tuple>`
> end-state in `ROADMAP-modernization.md`.

---

## Plan A — Conversions (one value, re-expressed)

A conversion takes a value and yields *the same thing in a different representation*: a frame change
(`ECEF` ↔ `LLA`), a same-dimension kind change (geodetic ↔ geocentric latitude, orthometric ↔ ellipsoidal
height), or a unit change (`meters` ↔ `feet`). The **`units` idiom** is the model, and the library already
consumes it internally:

- **Converting constructor / `operator=`** — the implicit init surface: `ECEF e = lla;`. This is *the one
  place frame conversion is wired* (it delegates to the free `convert` engine → LCA dispatch). Kept.
- **Member `.to<Target>()`** — the explicit, prvalue-returning verb, target-type-driven, exactly as
  `units` (`x.to<meters>()`) and `units::kind` (`k.to<PlainTarget>()` / `to<WrapperTarget>()`) do it. The
  daily conversion verb.
- **Free `convert<From,To>()`** — the buried engine; not the daily surface.

**Policy:**
1. Frame conversion stays the converting-ctor + a member `.to<TargetFrame>()` (the units idiom).
2. Same-dimension unit/scale changes stay on `units`' own `.to<Unit>()` — not re-wrapped at the coordinate
   level. One verb, one meaning per layer.
3. Same-dimension **kind** conversions that need the datum/ellipsoid (geodetic↔geocentric latitude,
   ortho↔ellipsoid height) are exposed as **members on a position** (the position carries its datum, so the
   call is zero-argument and reads cleanly): `pos.geocentricLatitude()`, `pos.toEllipsoidHeight()` — the
   latter already exists and is the template. A bare `angles::Latitude` value has no datum, so the
   value-level converter (`convertLatitude<To,Ellipsoid>`) stays a free function; the ergonomic front door
   is the position member.
4. DRY: after the `Coordinate<Frame,Tuple>` collapse these members live once in one template body. Until
   then they follow the house style (thin per-class forwarders, as `toEllipsoidHeight()` already does).

**Status:** frame converting-ctors exist; height members exist; latitude is currently a free function
(`convertLatitude<>`) with no position member yet. `to<TargetFrame>()` as a member does not exist yet.

---

## Plan B — Two-point measurements (a relationship between two points)

A measurement is **not** a conversion. It computes a scalar/angle *relationship between two distinct
points* — a distance, a range, a bearing. It is inherently binary; `to<>` has nothing to do with it (a
point class would never use `to` for a range). The natural, discoverable form is the directional
**`a.<measure>To(b)`** member, with the free two-argument function as the SSOT engine underneath.

**Policy:**
1. The `...To(other)` member is the ergonomic surface: `a.geodesicDistanceTo(b)`, `a.slantRangeTo(b)`,
   `a.euclideanDistanceTo(b)`, `a.bearingTo(b)`. Directional (`this` → argument), returns the tagged kind.
2. The free two-argument function (`coordinates::distance(a,b)`, `geodesicInverse<Datum>(a,b)`, …) is the
   single implementation; the member is a thin forwarder. This is already true for `distance()`.
3. **Symmetric** relationships with no privileged receiver (a plain point-to-point `distance` where neither
   is "from"), and constructions where *neither* operand is a natural `this` (`AER::fromObserver(obs,tgt)`),
   stay free functions / static factories — a member would arbitrarily privilege one operand.
4. Consistency of naming: the measurement family is uniform — `euclideanDistanceTo`, `slantRangeTo`,
   `geodesicDistanceTo`, `bearingTo` — each returning the kind its quantity genuinely is. A slant range and
   a Euclidean distance are the SAME quantity (a straight-line distance through 3-space carries no reference
   surface), so both return `ranges::Euclidean`; `slantRangeTo` is a use-name, not a distinct kind. Only a
   geodesic distance (a surface arc) is a distinct `ranges::Geodesic`, so the caller cannot confuse a surface
   distance with a straight-line one. This is the "don't over-kind things that are the same" discipline: tag
   a distinct kind only where the reference genuinely differs.
5. DRY: like Plan A, these members are thin forwarders. They collapse to one body under
   `Coordinate<Frame,Tuple>`; until then they match the existing per-class forwarder style.

**Status:** `distance()` (Euclidean) exists on every position; `distanceTo()` (Geodesic) +
`initialBearingTo`/`finalBearingTo` on `PositionGeodetic`; **`slantRangeTo` does NOT exist** — it is the gap.
`range()` on AER is a *stored* value (group-a accessor), not a two-point measurement.

---

## The DRY question (applies to BOTH plans)

Members are ergonomic; the hazard is that the six `position*.h` files are copy-pasted today (no CRTP, no
mixin — just thin forwarders over a shared free-function layer in `point.h`). Adding member ergonomics the
naive way multiplies that copy-paste. Three ways to get members without new duplication, in increasing cost:

1. **Accept targeted duplication now** — add the members as thin one-line forwarders, matching the existing
   `distance()` copy-paste style. Fastest; adds to the debt the collapse later erases.
2. **Shared CRTP/mixin** — factor the forwarders into a base the six positions inherit; each member written
   once. Mid cost; the roadmap says "no CRTP," so it is a stepping-stone, not the end-state.
3. **`Coordinate<Frame,Tuple>` collapse (roadmap Phase 3)** — one template body, six aliases; every member
   (and both plans' surfaces) written once; named accessors → `frame_axes<Frame>` trait. The honest,
   intended end-state; a real rearchitect, several commits, beyond the kind-tagging campaign.

**This choice is orthogonal to the two policies** — the policies say *what shape the API is*; this says
*where the shared implementation lives*. Ratify the policies first; pick the DRY mechanism per the appetite
for rearchitecting now vs. later.
