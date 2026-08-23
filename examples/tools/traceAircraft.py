#!/usr/bin/env python3
# ---------------------------------------------------------------------------------------------------------------------
# Trace an aircraft planform from a top-view silhouette into coordinates body-axis vertex tables.
#
# Pipeline: threshold a top-view image into a filled silhouette -> Moore-trace each closed loop's boundary ->
# arc-length resample to a dense point count -> Visvalingam-Whyatt simplify to a shape-optimal vertex count ->
# transform pixels to body axes (nose +x forward, right +y, z = 0), scaled to a real length -> emit
# `inline constexpr CartesianVector <part>()` tables ready to paste into a planform header (see f35Planform.h).
#
# The exterior outline is the boundary of the largest filled component; interior detail loops (canopy, intakes,
# vents, panels, nozzles) are the regions enclosed by interior strokes. `--interior` extracts and labels them so
# each can be named + tuned. Run `--label` first to see each loop's id, then map ids to part names in a config.
#
# Requires: Pillow, numpy (no scipy; marching/tracing is hand-rolled so it runs anywhere).
# ---------------------------------------------------------------------------------------------------------------------
import argparse
import json
import numpy as np
from PIL import Image
from collections import deque


def largest_component(mask, conn8=False):
    """Return (mask_of_largest_component, area)."""
    H, W = mask.shape
    lab = np.zeros((H, W), np.int32); cur = 0; best = None
    nbrs = ((1, 0), (-1, 0), (0, 1), (0, -1)) + (((1, 1), (1, -1), (-1, 1), (-1, -1)) if conn8 else ())
    for y in range(H):
        for x in range(W):
            if mask[y, x] and lab[y, x] == 0:
                cur += 1; area = 0; pts = deque([(y, x)]); lab[y, x] = cur
                while pts:
                    cy, cx = pts.popleft(); area += 1
                    for dy, dx in nbrs:
                        ny, nx = cy + dy, cx + dx
                        if 0 <= ny < H and 0 <= nx < W and mask[ny, nx] and lab[ny, nx] == 0:
                            lab[ny, nx] = cur; pts.append((ny, nx))
                if best is None or area > best[0]:
                    best = (area, cur)
    return lab == best[1], best[0]


def flood_exterior(light):
    """4-connected flood of `light` from every border pixel -> the exterior background mask."""
    H, W = light.shape
    out = np.zeros((H, W), bool); q = deque()
    for x in range(W):
        for y in (0, H - 1):
            if light[y, x] and not out[y, x]: out[y, x] = True; q.append((y, x))
    for y in range(H):
        for x in (0, W - 1):
            if light[y, x] and not out[y, x]: out[y, x] = True; q.append((y, x))
    while q:
        y, x = q.popleft()
        for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            ny, nx = y + dy, x + dx
            if 0 <= ny < H and 0 <= nx < W and light[ny, nx] and not out[ny, nx]:
                out[ny, nx] = True; q.append((ny, nx))
    return out


def trace_boundary(mask):
    """Moore-neighbor boundary trace of the outer contour of a filled mask -> ordered [(x, y), ...]."""
    H, W = mask.shape
    ys, xs = np.where(mask)
    sy = ys.min(); sx = xs[ys == sy].min()
    start = (sy, sx)
    moore = [(0, -1), (-1, -1), (-1, 0), (-1, 1), (0, 1), (1, 1), (1, 0), (1, -1)]

    def inside(y, x):
        return 0 <= y < H and 0 <= x < W and mask[y, x]

    def step(cur, b):
        d0 = (b[0] - cur[0], b[1] - cur[1]); i0 = moore.index(d0)
        for k in range(1, 9):
            d = moore[(i0 + k) % 8]; ny, nx = cur[0] + d[0], cur[1] + d[1]
            if inside(ny, nx):
                prev = (cur[0] + moore[(i0 + k - 1) % 8][0], cur[1] + moore[(i0 + k - 1) % 8][1])
                return (ny, nx), prev
        return cur, b

    contour = [start]; b = (start[0], start[1] - 1); cur = start
    nxt, b = step(cur, b); guard = 0
    while nxt != start and guard < 10 * int(mask.sum()):
        contour.append(nxt); cur = nxt; nxt, b = step(cur, b); guard += 1
    return [(x, y) for (y, x) in contour]


def resample(loop, n):
    """Resample a closed polyline to n points evenly by arc length."""
    p = np.array(loop, float)
    d = np.sqrt((np.diff(np.vstack([p, p[:1]]), axis=0) ** 2).sum(1))
    s = np.concatenate([[0], np.cumsum(d)]); total = s[-1]
    out = []; j = 0
    for t in np.linspace(0, total, n, endpoint=False):
        while j < len(s) - 1 and s[j + 1] < t: j += 1
        seg = s[j + 1] - s[j]; f = 0 if seg == 0 else (t - s[j]) / seg
        a = p[j % len(p)]; bb = p[(j + 1) % len(p)]
        out.append(a + f * (bb - a))
    return np.array(out)


def simplify(loop, keep):
    """Visvalingam-Whyatt: drop the least-area vertex until `keep` remain."""
    def area(a, b, c):
        return abs((b[0] - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (b[1] - a[1])) / 2.0
    pts = [tuple(p) for p in loop]
    while len(pts) > keep:
        n = len(pts); least = 0; la = area(pts[-1], pts[0], pts[1])
        for i in range(1, n):
            ar = area(pts[i - 1], pts[i], pts[(i + 1) % n])
            if ar < la: la = ar; least = i
        del pts[least]
    return np.array(pts)


def interior_holes(body, gray, stroke_thresh):
    """Label the regions enclosed by interior strokes inside the filled body. Returns a label image."""
    H, W = body.shape
    ys, xs = np.where(body); y0, y1, x0, x1 = ys.min(), ys.max(), xs.min(), xs.max()
    light_in = body & (gray >= stroke_thresh)
    edge = np.zeros((H, W), bool); q = deque()
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if light_in[y, x] and (not body[y - 1, x] or not body[y + 1, x] or not body[y, x - 1] or not body[y, x + 1]):
                edge[y, x] = True; q.append((y, x))
    while q:
        y, x = q.popleft()
        for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            ny, nx = y + dy, x + dx
            if light_in[ny, nx] and not edge[ny, nx]:
                edge[ny, nx] = True; q.append((ny, nx))
    holes = light_in & ~edge
    lab = np.zeros((H, W), np.int32); cur = 0
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if holes[y, x] and lab[y, x] == 0:
                cur += 1; pts = deque([(y, x)]); lab[y, x] = cur
                while pts:
                    cy, cx = pts.popleft()
                    for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                        ny, nx = cy + dy, cx + dx
                        if holes[ny, nx] and lab[ny, nx] == 0:
                            lab[ny, nx] = cur; pts.append((ny, nx))
    return lab


def to_body(loop, mid_y, cx, mps):
    """pixel (px, py) -> body (x forward = up-image, y right), meters, z = 0."""
    return [((mid_y - py) * mps, (px - cx) * mps) for px, py in loop]


def fmt_table(name, body_pts, comment):
    cells = [f"{{{x:+.3f}_m, {y:+.3f}_m, 0.0_m}}" for x, y in body_pts]
    rows = ["\t\t        " + ", ".join(cells[i:i + 3]) + ("," if i + 3 < len(cells) else "};")
            for i in range(0, len(cells), 3)]
    return "\n".join([f"\t/// {comment}", f"\tinline constexpr CartesianVector {name}()", "\t{", "\t\treturn {", *rows, "\t}"]) + "\n"


def main():
    ap = argparse.ArgumentParser(description="Trace an aircraft planform into coordinates vertex tables.")
    ap.add_argument("image", help="top-view silhouette (a solid or line-art plan view; nose up)")
    ap.add_argument("--crop", type=int, nargs=4, metavar=("X0", "Y0", "X1", "Y1"), help="crop box before tracing")
    ap.add_argument("--stroke", type=int, default=128, help="gray < this is a dark stroke/silhouette (0-255)")
    ap.add_argument("--length", type=float, default=15.70, help="real aircraft length in meters (scale reference)")
    ap.add_argument("--outline-keep", type=int, default=80, help="vertices to keep for the outer outline")
    ap.add_argument("--dense", type=int, default=1200, help="dense resample count before simplification")
    ap.add_argument("--label", action="store_true", help="write a labeled-holes PNG and print interior loop ids")
    ap.add_argument("--config", help="JSON mapping {id: [partName, keepCount]} for interior loops to emit")
    ap.add_argument("--out", default="-", help="output .h fragment path ('-' = stdout)")
    args = ap.parse_args()

    im = Image.open(args.image).convert("RGB")
    if args.crop:
        im = im.crop(tuple(args.crop))
    gray = np.asarray(im).astype(float).mean(2)
    H, W = gray.shape
    light = gray >= args.stroke
    ext = flood_exterior(light)
    body, _ = largest_component((~ext), conn8=False)

    ys, xs = np.where(body)
    mid_y = (ys.min() + ys.max()) / 2.0
    cx = (xs.min() + xs.max()) / 2.0
    mps = args.length / (ys.max() - ys.min())

    if args.label:
        lab = interior_holes(body, gray, args.stroke)
        import colorsys
        vis = Image.new("RGB", (W, H), (255, 255, 255))
        for k, i in enumerate([v for v in np.unique(lab) if v > 0]):
            yy, xx = np.where(lab == i)
            if len(xx) < 30: continue
            r, g, b = (int(c * 255) for c in colorsys.hsv_to_rgb((k * 0.13) % 1, 0.9, 1))
            for x, y in zip(xx, yy): vis.putpixel((int(x), int(y)), (r, g, b))
            cxm, cym = int(xx.mean()), int(yy.mean())
            print(f"id={i:3d} area={len(xx):6d} center=({cxm},{cym})")
        vis.save("traceAircraft_labels.png")
        print("wrote traceAircraft_labels.png")
        return

    blocks = [fmt_table("outline", to_body(simplify(resample(trace_boundary(body), args.dense), args.outline_keep), mid_y, cx, mps),
                        "The fuselage + wing + tail silhouette, one closed loop in draw order.")]
    if args.config:
        cfg = json.load(open(args.config))
        lab = interior_holes(body, gray, args.stroke)
        for sid, (name, keep) in cfg.items():
            m = (lab == int(sid))
            if m.sum() < 20:
                print(f"// {name}: id {sid} empty, skipped"); continue
            pts = to_body(simplify(resample(trace_boundary(m), max(200, keep * 20)), keep), mid_y, cx, mps)
            blocks.append(fmt_table(name, pts, name))

    text = "\n".join(blocks)
    if args.out == "-":
        print(text)
    else:
        open(args.out, "w").write(text)
        print(f"wrote {args.out}")


if __name__ == "__main__":
    main()
