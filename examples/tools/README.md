# Aircraft-tracing tools

`traceAircraft.py` turns a **top-view silhouette** of an aircraft into `coordinates` body-axis vertex tables
(the `inline constexpr CartesianVector <part>()` functions used by `examples/f35Planform.h`). It is how the
example airframes are authored: trace once from a reference image, paste the emitted tables into a planform
header, and the `consteval` builder assembles the 3D model at compile time.

## Pipeline

1. **Threshold** the image to a filled silhouette (dark strokes / solid fill vs. light background).
2. **Flood-fill** the exterior, take the largest filled component -> the aircraft body.
3. **Moore-trace** each closed loop's pixel boundary (the outer outline; interior detail loops are the regions
   enclosed by interior strokes).
4. **Arc-length resample** each loop to a dense point count (default 1200 for the outline).
5. **Visvalingam-Whyatt simplify** to a shape-optimal vertex count -- vertices land where curvature demands
   them (dense on the nose/tail, sparse on straight wing edges). This is the same algorithm the library exposes
   as `coordinates::simplify` and that the sensor demo runs at compile time on the outline.
6. **Transform** pixels -> body axes (nose +x forward, right +y, z = 0), scaled to a real length.

## Usage

```sh
# 1. See the interior loops and their ids (writes traceAircraft_labels.png):
python3 traceAircraft.py f35_topview.png --label

# 2. Map ids -> part names + keep-counts in a JSON config, e.g. f35.json:
#    { "2": ["canopy", 18], "14": ["intakeLeft", 10], "43": ["nozzleLeft", 12], ... }

# 3. Emit the tables:
python3 traceAircraft.py f35_topview.png --config f35.json --length 15.70 --out f35Tables.h
```

Requires `Pillow` and `numpy` (no scipy; the tracing/marching is hand-rolled so it runs anywhere).

## Notes

- **Vertical stabilizers need Z.** A flat top-view trace is all `z = 0`; a fin that must stand up for the
  banked-attitude animation is authored by lifting its tip edge in `-z` (root on the boom at `z = 0`, tip raked
  aft, canted outboard, lifted ~2 m). `traceAircraft.py` emits the flat footprint; the fin's height is added
  when the footprint is turned into a standing quad (see how `finLeft`/`finRight` are built in `f35Planform.h`).
- **Low-fi vs. detailed.** The airshow example draws every part; the low-fi sensor map glyph draws only the
  outline (further reduced by `coordinates::simplify<16>` at compile time), so it reads without interior clutter.
- **Source.** Use a true orthographic top view (a plan-view line drawing or silhouette), never an oblique photo
  -- perspective foreshortening would distort the planform.
