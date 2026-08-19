# Coordinates

## Description

Coordinates is a generic, mostly-compile-time, type-driven C++23 library for representing, transforming, and reasoning about physical position on and around the Earth. It is designed for technically demanding domains such as aerospace simulation, radar and RF analysis, navigation, autonomy, and scientific modeling—domains where *implicit assumptions* about coordinate frames, datums, or units routinely lead to subtle and costly errors.

The library provides **strongly typed point representations**, **explicit reference frames**, and **well-defined geodetic models**. Conversions between representations are expressed directly in the type system, allowing many classes of errors to be detected at compile time rather than at runtime. Where implicit conversions are permitted, they are intentionally constrained to cases that are physically unambiguous.

Reference frames form a graph, and a single generic `convert<From, To>` transforms between any two by routing through their **least common ancestor**—there is no N² matrix of hand-written pairwise conversions. The algorithm is chosen from the frame *types*, so adding a new frame (including the body frames described below) requires no changes to the conversion dispatcher.

Coordinates builds on the `units` library to enforce dimensional correctness and integrates geodesy concepts—ellipsoids, horizontal and vertical datums, geoids, and terrain—as first-class abstractions rather than hidden global assumptions.

Optional Digital Terrain Elevation Data (DTED) support enables terrain-aware elevation queries and line-of-sight (LOS) analysis, including Earth curvature and terrain masking, without imposing additional dependencies on users who do not require these capabilities.

---

## Design Goals and Non-Goals

### Design Goals

- **Strong typing and physical correctness**
- **Explicit reference frames and datums**
- **Compile-time validation where possible**
- **Deterministic and reproducible results**
- **Scalability**
- **Optional terrain integration**  

### Example: Why Strongly Typed Coordinates Matter

```cpp
PositionECEF ecef = ...;
PositionENU  enu  = ...;

// This will not compile: the frames are physically incompatible.
auto d = ecef - enu;
```

With raw doubles, this would compile and silently produce meaningless results.

---

## Requirements and Dependencies

- **Language standard:** C++23
- **CMake:** 3.28 or newer
- **Required dependency:** `units` 3.6.1
- **Optional runtime data:** DTED elevation tiles

If `units` is not found on the system, CMake will fetch and build `units` 3.6.1 automatically (this is on by default via `COORDINATES_FETCH_DEPS`).

### Example: Dependency Resolution

```cmake
find_package(coordinates CONFIG REQUIRED)

target_link_libraries(my_target
    PRIVATE coordinates::coordinates
)
```

The `units` dependency is propagated transitively.

---

## How to Build

Coordinates uses a standard out-of-source CMake workflow and supports common generators (Ninja, Makefiles, MSVC).

### Example: Building Coordinates (Default Configuration)

```bash
cmake -S . -B build
cmake --build build
```

By default, all optional components (DTED and geoid support) are enabled.

---

## Build Configuration Options

The library provides feature toggles to minimize compile time, binary size, and runtime dependencies when appropriate.

| Option | Description |
|------|-------------|
| `BUILD_TESTING` | Enable unit tests |
| `COORDINATES_ENABLE_DTED` | Enable DTED terrain support |
| `COORDINATES_ENABLE_GEOIDS` | Enable geoid models |
| `COORDINATES_ENABLE_EGM96` | Enable EGM96 geoid |
| `COORDINATES_ENABLE_GEOID12A` | Enable GEOID12A |
| `COORDINATES_ENABLE_USGG2012` | Enable USGG2012 |

Disabling options may speed up compile time in applications that don't use the features.

### Example: Disabling Optional Components

```bash
cmake -S . -B build   -DCOORDINATES_ENABLE_DTED=OFF   -DCOORDINATES_ENABLE_GEOIDS=OFF
```

This configuration is appropriate for purely geometric or space-based applications.

---

## How to Install

### Example: Installing Coordinates (Default Case)

```bash
cmake --build build
cmake --install build
```

Headers, libraries, and CMake package files are installed into the system default prefix.

---

## How to Use Coordinates in Your Project

### Example: Typical Integration

```cmake
find_package(coordinates CONFIG REQUIRED)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE coordinates::coordinates)
```

No additional include paths or compile flags are required.

---

## Background

Coordinates models Earth-referenced position using several distinct concepts:

- **Ellipsoid**  
  A mathematical approximation of the Earth’s shape (e.g., WGS84).

- **Horizontal datum**  
  Defines how latitude and longitude are interpreted relative to an ellipsoid.

- **Vertical datum**  
  Defines how height is measured (ellipsoidal vs orthometric).

- **Geoid**  
  A gravity-based surface used to convert ellipsoidal height to mean sea level.

- **Topography**  
  The physical terrain surface derived from elevation data.

These distinctions are critical when computing distances, angles, and visibility.

### Example: Height Depends on Datum

A point at a given latitude and longitude may have:
- One height relative to the ellipsoid
- A different height relative to mean sea level

Coordinates requires these choices to be explicit.

---

## Core Concepts

Coordinates is built around a small set of foundational concepts that define how physical location is represented, transformed, and reasoned about. Understanding these concepts is essential to using the library correctly and effectively.

---

### Points Represent Physical Locations

A *point* represents a **real physical location**, not merely a tuple of numbers. Each point encodes:

- a mathematical representation (e.g., spherical, Cartesian),
- an associated reference frame,
- and an implicit or explicit datum context.

Different point types may use different representations, but they can still refer to the same physical location.

#### Example: Same Location in Multiple Frames

```cpp
PositionGeodetic geo  = ...;
PositionECEF     ecef = geo;
PositionENU      enu  = geo;
```

---

## Using Coordinates

### Point Classes

The library provides several concrete point types:

- `LLA` — latitude, longitude, height
- `ECEF` — Earth-centered Earth-fixed Cartesian
- `ENU` — local East-North-Up tangent frame
- `NED` — local North-East-Down tangent frame
- `AER` — azimuth, elevation, range

These frames are based on the WGS84 ellipsoid. *Many* additional datums are available, and can be specified using the non-aliased classes:

- `PositionGeodetic<Datum>` — latitude, longitude, height
- `PositionECEF<Datum>` — Earth-centered Earth-fixed Cartesian
- `PositionENU<Datum>` — local East-North-Up tangent frame
- `PositionNED<Datum>` — local North-East-Down tangent frame
- `PositionAER<Datum>` — azimuth, elevation, range
- 
Each type encodes both representation and reference frame.

### Example: Creating Geodetic Points

```cpp
PositionGeodetic p(34.0_deg, -118.0_deg, 100.0_m);
```

---

# Point Class Operations

This document describes the operations exposed by point types in the **Coordinates** library.
It is intended to be read as a *user-facing contract*: what users can rely on being available,
what each operation means physically, and when particular operations are appropriate.

The descriptions below are derived from the public headers in the project and avoid speculative
or implied APIs.

---

## Overview

Point types in Coordinates are **value types** representing physical locations expressed in a
specific coordinate representation, reference frame, and datum context.

All point types provide:

1. Representation-specific component accessors
2. Equality and validity queries
3. Distance and magnitude-related operations (where physically meaningful)
4. Frame and datum context required for safe conversion

Operations are intentionally constrained. Only operations that are physically meaningful for a
given representation are exposed.

---

## Common Operations Across Point Types

The following operations are implemented consistently across the primary point types
(`PositionGeodetic`, `PositionECEF`, `PositionENU`, `PositionNED`, `PositionAER`).

### Null State

- `bool isNull() const`  
  Returns `true` if the point represents the zero-value of its representation.
  This is a representational check, not a physical validity check.

---

### Equality and Comparison

- `bool isSame(const Point& other, T tolerance) const`  
  Compares two points using a scalar tolerance.

- `bool isSame(const Point& other, const Point& tolerance) const`  
  Compares two points using per-component tolerances.

Equality is defined in terms of **representation values**, not semantic equivalence across frames.

---

### Straight-Line Distance

- `auto distance(const Point& other) const`

Returns the Euclidean (straight-line) distance between two points in the same representation.
This operation does **not** compute geodesic or surface distance.

---

### Frame Data Access

- `const FrameData& frameData() const`  
  Returns the frame metadata associated with the point.

- `void setFrameData(const FrameData&)`  
  Sets frame metadata (used internally and during conversions).

These functions exist primarily to support the conversion infrastructure and are rarely needed
in application-level code.

---

## PositionGeodetic

Represents a geographic position using latitude, longitude, and altitude, together with datum
and epoch information.

### Component Accessors

- `latitude()` / `setLatitude(...)`
- `longitude()` / `setLongitude(...)`
- `altitude()` / `setAltitude(...)`

Accessors return **strongly-typed geodesy kinds**, not bare `units` quantities: `latitude()` is an
`angles::Latitude`, `longitude()` an `angles::Longitude`, `altitude()` the height kind the datum measures
(`heights::Ellipsoidal` or `heights::Orthometric`). Likewise `PositionAER::azimuth()`/`elevation()` are
`angles::Azimuth`/`angles::Elevation` and geodesic bearings are `angles::Azimuth`; `range()` is a
`ranges::Slant`, `distanceTo()` a `ranges::Geodesic`, and `distance()`/`magnitude()` a `ranges::Euclidean`.
Each is a `units::kind` tag over a plain unit, so quantities that share a dimension but not a meaning — a
latitude and an azimuth (both `degrees<>`), or a slant range and a surface distance (both `meters<>`) — can
never be silently interchanged; mixing two different kinds is a compile error. A plain unit still constructs
into a kind implicitly (so `LLA p(34_deg, -118_deg, 100_m)` is unchanged), and `.to<PlainUnit>()` unwraps
when you need the raw value.

---

### Datum and Epoch

- `date()`  
  Returns the decimal-year date associated with the position.

The date is used when converting between datums that include time-dependent components.

---

### Equality and Distance

- `isNull()`
- `isSame(...)`
- `distance(...)`  
  Straight-line Euclidean distance through space.

---

### Geodesic (Ellipsoid Surface) Operations

These operations use the reference ellipsoid associated with the position’s datum.

- `inverseTo(const PositionGeodetic&)`  
  Returns a geodesic inverse result containing surface distance and bearings.

- `distanceTo(const PositionGeodetic&)`  
  Returns surface distance along the ellipsoid.

- `initialBearingTo(const PositionGeodetic&)`
- `finalBearingTo(const PositionGeodetic&)`

- `destination(azimuth, distance)`  
  Computes a destination point along the ellipsoid.

---

## PositionECEF

Represents a position in Earth-Centered, Earth-Fixed Cartesian coordinates.

### Component Accessors

- `x()` / `setX(...)`
- `y()` / `setY(...)`
- `z()` / `setZ(...)`

---

### Epoch

- `date()`  
  Date-of-observation used for datum transformations.

---

### Vector Operations

- `dotProduct(const PositionECEF&)`
- `magnitude()`

These operations treat the position as a vector from the Earth’s center.

---

### Equality and Distance

- `isNull()`
- `isSame(...)`
- `distance(...)`

---

## PositionENU

Represents a local East-North-Up tangent frame centered on a specific origin.

### Component Accessors

- `east()` / `setEast(...)`
- `north()` / `setNorth(...)`
- `up()` / `setUp(...)`

---

### Frame Origin

- `origin()`  
  Returns the geodetic origin defining the local frame.

---

### Magnitude and Distance

- `magnitude()`  
  Euclidean magnitude relative to the local origin.

- `distance(...)`

---

## PositionNED

Represents a local North-East-Down tangent frame.

### Component Accessors

- `north()` / `setNorth(...)`
- `east()` / `setEast(...)`
- `down()` / `setDown(...)`

Other operations mirror `PositionENU` with axis orientation differences.

---

## PositionAER

Represents an observer-relative position using azimuth, elevation, and range.

### Component Accessors

- `azimuth()`
- `elevation()`
- `range()`

### Construction

`PositionAER` cannot be constructed implicitly from absolute positions.
It must be explicitly created with observer context.

---

## Body-relative positions

A position expressed relative to a moving, rotating body — a sensor at a fixed offset on an aircraft, in
the vehicle's own axes — is not a distinct coordinate type. It is modelled by the frame graph and the
rigid-transform types:

- **`BodyFrame<Parent, Transform>`** — a Cartesian frame rigidly attached to a parent at a **compile-time**
  offset and orientation (a static mounting). A body-local point converts through the frame graph to NED,
  ECEF, and so on. Body frames nest to arbitrary depth (camera on a wingtip on an aircraft in local NED).
- **`Pose`** — a **runtime** 6-DOF rigid transform (a translation plus a `Quaternion`) for a body whose
  position and attitude vary each frame (a moving vehicle, a slewing sensor). `Pose::at(position, attitude)`
  places it; `transformPoint` maps a body-local offset into the parent frame (returning the same position
  type — e.g. an `ECEF` sensor location); `rotateDirection` carries a body-axis boresight into the parent
  frame as the body slews; `Pose::from<Mount>()` lifts a fixed `BodyTransform` mount so it composes onto a
  live vehicle pose (`sensorPose = vehiclePose * Pose::from<Mount>()`).

```cpp
// A wing-mounted sensor on a moving aircraft.
ECEF cg = ...;                                   // aircraft centre of gravity, updated live
Pose plane = Pose::at(cg, EulerAngles(yaw, pitch, roll));
using Mount = Offset<0.5_m, 3.2_m, -0.1_m>;      // fixed sensor offset in body axes
ECEF sensor = (plane * Pose::from<Mount>()).transformPoint(ECEF(0_m, 0_m, 0_m));
```

---

# Coordinate Conversions

This document describes how coordinate conversions are performed in the **Coordinates** library.
It focuses on how *users* express conversions in code, the rules governing implicit and explicit
conversions, and the rationale behind those rules.

The goal of the conversion model is to preserve **physical meaning**, avoid ambiguity, and make
incorrect transformations difficult or impossible to express.

---

## Overview

A coordinate conversion transforms a point from one representation or reference frame into another
while preserving the underlying physical location.

Examples of conversions include:

- Geodetic ↔ Earth-centered Cartesian
- Global ↔ local tangent frames
- Absolute ↔ observer-relative representations

Conversions may be **implicit** or **explicit**, depending on whether additional context is required.

---

## Implicit Conversions

Implicit conversions are permitted only when the transformation is *physically unambiguous* and
does not require additional user-supplied context.

In these cases, the compiler is allowed to perform the conversion automatically.

### Characteristics of Implicit Conversions

Implicit conversions:

- Preserve physical location exactly
- Require no observer, origin, or auxiliary context
- Do not discard information
- Are guaranteed to be reversible

---

### Example: Geodetic to Earth-Centered Cartesian

```cpp
PositionGeodetic geo = ...;
PositionECEF     ecef = geo;
```

This conversion is unambiguous: both representations describe the same absolute physical location
in different coordinate systems.

A conversion (one point re-expressed) is distinct from a **measurement** (a relationship between two
points). Measurements read as directional members returning their distinctly-typed kind, so a surface
distance, a slant range, and a straight-line distance cannot be confused:

```cpp
LLA a = ..., b = ...;
ranges::Geodesic  surface = a.geodesicDistanceTo(b);   // along the ellipsoid
ranges::Slant     slant   = a.slantRangeTo(b);         // straight line, observer->target
ranges::Euclidean euclid  = a.euclideanDistanceTo(b);  // straight line, 3-D
angles::Azimuth   bearing = a.bearingTo(b);            // forward azimuth
```

---

### Example: Chained Implicit Conversions

```cpp
PositionGeodetic geo = ...;

PositionECEF ecef = geo;
PositionNED  ned  = ecef;
```

Each step preserves physical meaning. The intermediate representation is explicit in the type
system, even if it is elided in user code.

---

### Example: Mixing Representations in Algorithms

```cpp
PositionGeodetic a = ...;
PositionECEF     b = ...;

auto d = distance(a, b);
```

The algorithm internally performs the necessary conversions to operate on a common representation.
The user does not need to manually align frames.

---

## Explicit Conversions

Some conversions require additional information to be physically meaningful. These conversions
cannot be implicit and must be expressed explicitly by the user.

Explicit conversions make required context visible in the call site.

---

### Observer-Relative Conversions

Azimuth, elevation, and range are defined *relative to an observer*. There is no meaningful way to
compute them without specifying that observer.

### Example: Absolute to Observer-Relative

```cpp
PositionGeodetic observer = ...;
PositionGeodetic target  = ...;

// Explicit construction required
PositionAER aer = PositionAER::fromObserver(observer, target);
```

This conversion cannot be implicit because the observer is not inherent to the target.

---

### Local Tangent Frame Conversions

Local ENU and NED frames are defined relative to an explicit origin.

### Example: Global to Local Frame

```cpp
PositionGeodetic origin = ...;
PositionGeodetic target = ...;

PositionENU local(target, origin);
```

The origin defines the orientation and position of the local frame.

---

## Conversion as Assignment

Implicit conversions are expressed using assignment or copy-initialization syntax.

### Example: Assignment Conversion

```cpp
PositionECEF ecef;
ecef = geo;
```

The assignment expresses a conversion, not a mutation of the original object.

---

## Value Semantics

All conversions produce new value objects. The source object is never modified.

```cpp
PositionECEF ecef = geo;   // geo remains unchanged
```

This allows conversions to be composed safely and used freely in multithreaded contexts.

---

## Conversion Design Principles

The conversion model follows these rules:

- Preserve physical meaning at all times
- Require explicit user intent when context is missing
- Prefer compile-time enforcement over runtime checks
- Make common, safe conversions easy
- Make dangerous or ambiguous conversions impossible

---

### Converting Between Datums

Coordinates allows explicit conversion between datums when sufficient information is available.

### Example: Converting Height Using a Geoid

Height comes in two flavours that must never be silently mixed: **ellipsoidal** (HAE, what GPS reports)
and **orthometric** (MSL, what a map reports). They differ by the geoid undulation, which depends on
position, so the conversion needs a point, not a bare number. `PositionGeodetic` carries the point, and
its `altitude()` is tagged with the height kind the datum measures — an ellipsoid-referenced datum yields
an `heights::Ellipsoidal`, a geoid-referenced datum an `heights::Orthometric` — so the reference surface
is part of the type. `toEllipsoidHeight()` and `toOrthometricHeight()` convert between them.

```cpp
// A point on a geoid-referenced datum stores an orthometric (MSL) height.
PositionGeodetic<datums::NAD83_NAVD88> point(41.87917_deg, -87.62917_deg, 0.0_m);

auto msl = point.altitude();            // heights::Orthometric (deduced from the datum)
auto hae = point.toEllipsoidHeight();   // heights::Ellipsoidal (undulation added)

// The two heights are distinct types: `msl + hae` is a compile error. Unwrap to operate in plain units:
meters<> altitudeMSL = point.toOrthometricHeight().to<meters<>>();
```

---

# Rotations

Orientation is a first-class concept in Coordinates, provided by a small, reusable rotation-math library
(`lib/quaternion.h`, `lib/rotation.h`) that depends only on `units`. It offers **four fully
interconvertible** representations of the same 3D rotation, each suited to a different task:

- **`Quaternion`** — the canonical internal representation (Hamilton convention, active rotation).
  Cheap to compose, free of gimbal lock, and ready for interpolation.
- **`EulerAngles`** — human-readable yaw/pitch/roll, using the intrinsic **Z-Y-X (Tait-Bryan)** convention:
  yaw about Z, then pitch about the new Y, then roll about the new X (the aerospace body-axis convention).
- **`RotationMatrix`** — a 3×3 direction-cosine matrix, applied directly to Cartesian vectors.
- **`AxisAngle`** — a unit axis and an angle about it.

Every representation converts to and from every other through the quaternion via the free functions
`toQuaternion`, `toEulerAngles`, `toRotationMatrix`, and `toAxisAngle`. All operations are
`constexpr`-capable: a rotation known at compile time is computed at compile time, and the same code runs at
run time otherwise.

### Example

```cpp
#include <quaternion.h>
#include <rotation.h>

using namespace coordinates;
using namespace units::literals;

// Build a rotation from yaw/pitch/roll (intrinsic Z-Y-X).
const EulerAngles euler(90.0_deg, 0.0_deg, 0.0_deg);   // yaw 90 deg
const Quaternion  yaw90 = toQuaternion(euler);

// Rotate a vector: body +X (forward) maps to parent +Y under a +90 deg yaw.
const std::tuple<meters<>, meters<>, meters<>> forward(1.0_m, 0.0_m, 0.0_m);
const auto rotated = yaw90.rotate(forward);            // ~ (0, 1, 0) meters

// Compose two rotations: `a * b` applies `b` first, then `a`.
const Quaternion roll45     = toQuaternion(EulerAngles(0.0_deg, 0.0_deg, 45.0_deg));
const Quaternion yawThenRoll = roll45 * yaw90;

// Inverse / identity are always available.
const Quaternion undo     = yaw90.conjugate();         // inverse for a unit quaternion
const Quaternion identity = Quaternion::identity();
```

Additional operations include `normalized`, `inverse`, `dot`, `fromTwoVectors` (shortest-arc rotation between
two directions), and `slerp` (spherical linear interpolation).

---

---

# Body Frames and Pose

Because reference frames form a graph joined by a generic `convert<From, To>`, a *rigid body* is naturally
modeled as a frame attached to a parent frame. Coordinates provides two complementary models, both backed by
the rotation library:

- **`BodyFrame<Parent, Transform>`** — a **compile-time** rigid mounting. The offset and orientation are
  carried in the frame *type* through a `BodyTransform` policy, so the mounting is fixed and free of runtime
  cost. Convenience aliases `Offset<X, Y, Z>` (pure translation) and `Attitude<Yaw, Pitch, Roll>` (pure
  rotation) cover the common cases. Body frames **nest to arbitrary depth** and slot into the frame graph
  with no changes to the conversion dispatcher.
- **`Pose`** — a **runtime** 6-DOF rigid transform (a translation plus a `Quaternion`) for a body whose
  position and attitude vary over time, such as a moving vehicle or a slewing sensor.

### Nested body frames — a camera on a wingtip

```cpp
#include <bodyFrame.h>

using namespace coordinates;
using namespace coordinates::coordinateFrames;
using namespace units::literals;

using WgsDatum  = datums::WGS84_G1674;
using LocalNED  = NEDFrame<WgsDatum>;

// Aircraft body: yawed 90 deg relative to local NED, no offset.
using PlaneBody = BodyFrame<LocalNED,  Attitude<90.0_deg, 0.0_deg, 0.0_deg>>;
// Wingtip: offset from the body origin, no rotation.
using Wingtip   = BodyFrame<PlaneBody, Offset<0.5_m, 3.2_m, -0.1_m>>;
// Camera: mounted on the wingtip, bore pointing aft (yaw 180 deg).
using CameraAft = BodyFrame<Wingtip,   Attitude<180.0_deg, 0.0_deg, 0.0_deg>>;

// A point 10 m in front of the camera, expressed all the way down to local NED.
const CartesianTuple pCam(10.0_m, 0.0_m, 0.0_m);
const CartesianTuple pNed = convert<CameraAft, LocalNED>(pCam, FrameData{}, FrameData{});
```

The `convert` call composes each leg's rotate-and-translate (camera → wingtip → plane body → NED) purely from
the frame types.

### Runtime pose composition

```cpp
#include <pose.h>

using namespace coordinates;
using namespace units::literals;

// A pose maps a point from its local frame into its parent frame (rotate, then translate).
const Pose parentFromMid(CartesianTuple(10.0_m, 0.0_m, 0.0_m),
                         EulerAngles(90.0_deg, 0.0_deg, 0.0_deg));
const Pose midFromLocal (CartesianTuple(0.0_m, 5.0_m, 0.0_m),
                         EulerAngles(0.0_deg, 45.0_deg, 0.0_deg));

// Compose: `a * b` applies the inner (right-hand) transform first.
const Pose parentFromLocal = parentFromMid * midFromLocal;

const CartesianTuple local(1.0_m, 2.0_m, 3.0_m);
const CartesianTuple inParent = parentFromLocal.transformPoint(local);

// The inverse maps parent-to-local; `Pose::identity()` is the neutral element.
const Pose localFromParent = parentFromLocal.inverse();
```

---

---

# Algorithms and Helper Functions

This document describes the free functions, helper algorithms, and extension points provided by
the **Coordinates** library. These facilities operate on point types while preserving physical
meaning, reference frames, and datum context.

The emphasis is on *what problems the algorithms solve* and *how they are used*, rather than on
internal implementation details.

---

## Overview

Algorithms in Coordinates are designed to:

- Operate on points expressed in arbitrary coordinate representations
- Preserve physical meaning across conversions
- Require explicit context where ambiguity would otherwise arise
- Remain usable in performance-critical code

Where possible, algorithms are **frame-agnostic**: inputs may be expressed in different coordinate
types, and the algorithm will internally perform the necessary conversions.

---

## Distance and Separation Algorithms

### Straight-Line Distance

The most common geometric query is the straight-line (Euclidean) distance between two points.

#### Example: Distance Between Two Points

```cpp
PositionGeodetic a = ...;
PositionECEF     b = ...;

auto d = a.distance(b);
```

The points may be expressed in different representations. The algorithm converts them as needed
to compute the physical separation.

> **Note**  
> This operation computes straight-line distance through space. It does **not** compute distance
> along the Earth’s surface.

---

### Squared Distance

For performance-sensitive applications where only relative distance comparisons are required,
a squared-distance variant is typically provided.

#### Example: Squared Distance Comparison

```cpp
if (distanceSquared(a, b) < 100_m2)
{
    // within 100 meters
}
```

---

### Vertical and Horizontal Separation

Where meaningful, algorithms may expose vertical or horizontal separation between points.

These operations are useful for altitude gating, terrain clearance checks, and filtering.

---

## Observer-Relative Algorithms

Some spatial quantities are inherently observer-dependent and therefore require explicit
observer context.

### Look Angles (Azimuth, Elevation, Range)

The library provides helper facilities for computing azimuth, elevation, and range between
an observer and a target.

#### Example: Look Angles Between Points

```cpp
PositionGeodetic observer = ...;
PositionGeodetic target  = ...;

PositionAER aer = PositionAER::fromObserver(observer, target);

auto az = aer.azimuth();
auto el = aer.elevation();
auto r  = aer.range();
```

This construction is intentionally explicit. Look angles have no meaning without an observer.

---

### Local Frame Projection

Points may be projected into a local tangent frame for relative computations.

#### Example: Projecting Into a Local ENU Frame

```cpp
PositionGeodetic origin = ...;
PositionGeodetic target = ...;

PositionENU local(target, origin);

auto e = local.east();
auto n = local.north();
auto u = local.up();
```

Local frames are always defined relative to an explicit origin.

---

## Frame-Agnostic Algorithm Design

Many helper functions accept points in different frames and representations.

#### Example: Mixed-Frame Usage

```cpp
PositionGeodetic a = ...;
PositionECEF     b = ...;

auto d = distance(a, b);
```

The algorithm performs internal conversions as required. The user does not need to manually align
representations before calling the function.

---

## Line-of-Sight Helpers

> **Status: not yet working.** Line-of-sight (LOS) is a work in progress. It is gated behind an off-by-default
> build option and is not part of the supported API yet. The description below is the intended design.

Higher-level algorithms, such as line-of-sight (LOS), build on the same conversion and distance
infrastructure.

These algorithms typically:

- Construct observer-relative rays
- Account for Earth curvature
- Optionally incorporate terrain data

---

## Defining Custom Ellipsoids, Geoids, and Datums

Coordinates is designed to be extensible. Advanced users may define custom Earth models and integrate
them into the existing conversion framework.

Custom models are defined as *types*, not runtime configuration objects.

---

## Custom Ellipsoids

An ellipsoid defines the mathematical surface used for geodetic computations.

### Example: Custom Ellipsoid Definition

```cpp
struct MyEllipsoid {
    static constexpr auto a = meters(6'378'137.0);   // semi-major axis
    static constexpr auto f = 1.0 / 298.257223563;  // flattening
};
```

The ellipsoid parameters are compile-time constants.

---

## Custom Geoids

A geoid defines the relationship between ellipsoidal height and orthometric (mean sea level) height.

Custom geoids may be defined by providing an interface that computes height offsets as a function
of position.

The exact interface is documented in the geoid headers.

---

## Custom Datums

A datum combines:

- a reference ellipsoid,
- a horizontal reference frame,
- and a vertical reference model.

Custom datums are defined by composing these elements into a single type.

### Example: Custom Datum Composition

```cpp
struct MyDatum {
    using reference_ellipsoid = MyEllipsoid;
    using horizontal_datum    = /* custom horizontal datum */;
    using vertical_datum      = /* custom vertical datum */;
};
```

Once defined, the custom datum participates in the same conversion and algorithm infrastructure
as built-in datums.

---

## Performance Model

- Most conversions are constexpr-capable
- No hidden heap allocations
- Terrain data is cached and reused

### Example: Efficient Reuse

```cpp
ECEF ecef = p;
for (auto& t : targets) {
   ENU local(ecef, origin);
}
```

---

## Thread Safety and Concurrency

- Point types are immutable
- Terrain access is thread-safe
- Suitable for parallel LOS analysis

---
# DTED (Digital Terrain Elevation Data)

This section documents terrain support in the **Coordinates** library as implemented through the
`Topography` abstraction and the DTED-backed implementation. All descriptions below are derived
directly from the public headers and avoid undocumented behavior.

---

## Overview

Terrain support in Coordinates is provided through the **Topography** concept. A topography model
maps geographic location (latitude and longitude) to an **orthometric height**: the height of the
terrain surface above mean sea level.

DTED (Digital Terrain Elevation Data) is provided as a concrete implementation of this concept,
allowing terrain-aware height queries and higher-level algorithms (such as terrain-aware
line-of-sight) to be built on top.

Topography is orthogonal to coordinate representation: it operates on geodetic location, not on
any specific point type.

---

## The Topography Concept

A class qualifies as a topography model if it satisfies the requirements defined in
`topography.h`. In particular, a topography:

- Is default constructible
- Declares an associated reference geoid
- Provides a callable interface for retrieving orthometric height

The library also provides a `NULL_TOPOGRAPHY` implementation that always returns zero elevation.
This is useful for disabling terrain effects while preserving API compatibility.

---

## DTED Topography

The `DTED` class is a topography implementation that reads terrain elevation from DTED files
(`.dt0`, `.dt1`, `.dt2`). It interprets DTED samples as orthometric height relative to the geoid
associated with the selected vertical datum.

DTED data is organized into tiles and accessed through an internal tile manager.

---

## DTED Data Location

DTED data is discovered via the environment variable:

```bash
DTED_ROOT_DIR
```

This variable must point to the root directory containing DTED tiles. The directory is searched
recursively.

The library does not download or manage DTED data automatically.

---

## DTED Tiles and Tile Management

DTED data is accessed through two primary internal components:

- **`DTEDTile`**  
  Represents a single DTED tile and is responsible for loading and sampling height data.

- **`DTEDTileManager`**  
  Locates, loads, and caches DTED tiles as needed. Tile management is transparent to users.

Tile resolution and coverage depend entirely on the DTED dataset provided.

---

## Querying Terrain Height

Terrain queries are performed through the topography interface, not through raw DTED objects.
Algorithms and higher-level code interact with terrain via the `Topography` abstraction.

Height values returned by topography are **orthometric heights** (height above mean sea level).

### Example: Querying Orthometric Terrain Height

```cpp
DTED topo;

degrees<> lat = ...;
degrees<> lon = ...;

meters<> terrainHeight = topo(lat, lon);
```

The returned value represents ground elevation relative to the geoid.

---

## Interaction with Vertical Datums

Vertical datums define the relationship between ellipsoidal height and orthometric height.
The DTED topography integrates with the vertical datum system through `verticalDatum.h`.

Conversion helpers are provided to convert between:

- Ellipsoidal height (relative to the reference ellipsoid)
- Orthometric height (relative to mean sea level)

These conversions are explicit and depend on the selected datum and geoid model.

---

## Disabling Terrain Effects

When terrain is not required, users may:

- Disable DTED support at build time, or
- Use `NULL_TOPOGRAPHY`, which returns zero elevation everywhere

This allows the same algorithms to be reused without conditional logic.

---

## Usage in Higher-Level Algorithms

Terrain support is intended to be consumed by higher-level algorithms such as:

- Terrain-aware line-of-sight
- Clearance and masking computations
- Altitude-relative filtering

These algorithms accept a topography model explicitly and do not assume global terrain state.

---

## Design Guarantees

- DTED access is encapsulated behind the `Topography` abstraction
- Terrain queries are read-only and do not mutate point objects
- Tile loading and caching are managed internally
- No implicit terrain usage occurs unless explicitly enabled

---

## Summary

DTED support in Coordinates provides a structured, explicit mechanism for incorporating terrain
elevation into spatial computations. By separating terrain from coordinate representation and
making all terrain access explicit, the library ensures correctness, flexibility, and
predictability across both terrain-aware and terrain-free applications.


## License

Coordinates is released under the MIT License.
