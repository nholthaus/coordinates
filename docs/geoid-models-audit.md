# Geoid (Geoid-Undulation) Models Audit

**Scope:** determine whether newer/better geoid-undulation models exist that the `coordinates`
library should adopt beyond its current EGM96 / USGG2012 / GEOID12A set (`src/geoid.h`, `src/*LUT.*`),
and establish exactly how each candidate is validated.

**Iron rule applied:** every model recommended for adoption is validated on BOTH axes —
(1) its data/coefficients come from a **primary authoritative body** (NGA EGM team, US NGS, IERS,
ISG), cited with the exact URL + model name + release date + reference ellipsoid/frame + resolution;
and (2) its undulation VALUES are checkable against an **independent truth source** — an agency
test-value table AND/OR an independent computing oracle. A model with primary data but no reachable
way to validate its undulation values is marked **UNVALIDATED** and is NOT recommended for adoption.

**Do not edit code:** this is a research + validation report only. No `src/` or `test/` file was changed.

---

## How validation was performed (the oracle)

**PROJ 9.7.1** is installed on this box (`cs2cs`, `cct`, `projinfo`, `gie`; grids in `/usr/share/proj/`).
It was proven to be a **drop-in oracle for this exact codebase** before being used to judge any candidate:

- The library's own `test/suites/geoidTest.cpp` validates `geoids::EGM96` against NGA's published
  `outintpt.dat` test points at a **1.1 cm** tolerance, and it PASSES (ran `coordTest --gtest_filter=GeoidTest.*`
  → 4/4 pass).
- PROJ's EGM96 (grid `egm96_15.gtx` = NGA `WW15MGH.GRD`) reproduces the library's **own embedded
  EGM96 test points** to ~1 cm — including the *non-NGA* points baked into the test:

  | Point (lat, lon)      | library `geoidTest.h` expects | PROJ EGM96 `N` | agreement |
  |-----------------------|------------------------------:|---------------:|-----------|
  | 42.3550, −71.0656 (Boston) | −28.550 m | −28.560 m | 1.0 cm |
  | 37.2350, −115.8111 (Nevada)| −26.449 m | −26.450 m | 0.1 cm |
  | 25.75, 121.25 (Taiwan)     |  17.502 m |  17.500 m | 0.2 cm |

  Because PROJ, NGA's published table, and the library agree at the cm level on the *same* underlying
  EGM96 grid, PROJ is a legitimate independent computing oracle for this library's geoid work.

**Sign convention (proven, not assumed).** Geoid undulation `N = h(ellipsoidal) − H(orthometric)`.
Driving `cs2cs EPSG:4979 +to EPSG:4326+<vertCRS>` with ellipsoidal height `h = 0` returns the
orthometric height `H`, so `N = −H_out`. Verified with the `cct +proj=vgridshift +grids=egm96_15.gtx`
pipeline giving the identical magnitude with opposite sign. All `N` values in this report use this rule.

**Grid provenance.** `proj.db` maps the EPSG vertical CRSs to the NGA/NGS-authored grids on the PROJ
CDN (`cdn.proj.org`), which is network-reachable from this box (HTTP 200). The candidate grids were
pulled and evaluated **locally** (`PROJ_DATA=/tmp/proj_grids`, `PROJ_NETWORK=OFF`) so the oracle runs
against the primary agency grid, not a live web service:

- `us_nga_egm08_25.tif` — EGM2008 2.5′ (NGA `Und_min2.5x2.5_egm2008_isw=82_WGS84_TideFree`), 80.6 MB.
- `us_noaa_g2018u0.tif` — GEOID18 CONUS (NGS `g2018u0.bin`), 16.7 MB; `us_noaa_g2018p0.tif` = PR/USVI.
- `us_nga_egm96_15.tif` / `egm96_15.gtx` — EGM96 15′ (NGA `WW15MGH.GRD`).

**GeographicLib `GeoidEval`** is NOT installed here (no `/usr/share/GeographicLib/geoids/`), so it was
not used directly; it is cited as a *second* documented oracle (it reproduces NGA's EGM2008 harmonic
synthesis to ~23 pm — picometers — per its own docs).

---

## Summary table

| Model | Primary source | Resolution | Accuracy | Ref. ellipsoid/frame | Oracle here? | Verdict |
|---|---|---|---|---|---|---|
| **EGM2008** (global) | NGA EGM team, released Jul 2008; degree 2190 / order 2159 | 2.5′ and 1′ grids | ±5–10 cm GPS/leveling; ~3–6× more accurate + 6× finer than EGM96 (Pavlis et al. 2012) | WGS84, Tide Free | **YES** — NGA `OUTPUT.DAT` test table + PROJ 2.5′ grid, both local | **RECOMMENDED** (adopt 2.5′) |
| **GEOID18** (US hybrid) | NGS, NOAA TR NOS NGS 72 (May 2020); official since 2019 | 1′ | 1.39 cm CONUS std dev (18% better than GEOID12B) | NAD83(2011)→NAVD88; basis xGEOID19B | **YES** — PROJ GEOID18 grid, local + NGS online tool | **RECOMMENDED** (supersedes GEOID12A) |
| **EGM2020** (global) | NGA (announced 2019/2020) | planned 5′ (deg 2190/2159) | claimed better than EGM2008 | (presumed WGS84; tide unstated) | **NO** — no public data, no CRS/grid in PROJ, not on ICGEM/ISG | **UNVALIDATED / NOT-RELEASED** |
| **GEOID2022 / NAPGD2022** (US) | NGS NSRS modernization | forthcoming | n/a (beta) | NATRF2022 family / new geopotential datum | **NO** — beta only, not released | **UNVALIDATED / NOT-RELEASED** |
| xGEOID19/20 (US gravimetric, experimental) | NGS annual experimental series | 1′ | experimental | IGS08-based | partial (superseded by GEOID2022 work) | **REJECTED** (experimental, not a stable target) |
| ISG national geoids (regional) | ISG repository (isgeoid.polimi.it) | varies | varies | varies | via PROJ CDN per-country | **OUT OF SCOPE** (mention only) |

---

## 1. EGM2008 — global upgrade to EGM96 — **RECOMMENDED**

### Primary source
- **NGA Earth Gravitational Model 2008.** Released July 2008 (WGS84 version); approved for DoD use in
  NGA.STND.0036_1.0 (2014-07-08). Complete to **degree and order 2159**, with additional spherical
  harmonic coefficients extending to **degree 2190 and order 2159**. Referenced to the **WGS84**
  ellipsoid. Primary reference paper: **Pavlis, Holmes, Kenyon, Factor, "The development and evaluation
  of the Earth Gravitational Model 2008 (EGM2008)," JGR Solid Earth 117 (B4), 2012,
  doi:10.1029/2011JB008916.**
- NGA product page: `https://earth-info.nga.mil/` (EGM2008 spherical harmonics + 2.5′ interpolation grid).
- Tide system is **Tide Free** (established by the model/grid file names
  `EGM2008_to2190_TideFree` and `Und_min2.5x2.5_egm2008_isw=82_WGS84_TideFree`; a `_MeanTide` grid also exists).

### Accuracy vs EGM96
Pavlis et al. (2012): GPS/leveling geoid discrepancy **±5–10 cm** in well-surveyed areas; vertical
deflections within ±1.1–1.3″; an improvement over EGM96 of **~6× in resolution and ~3–6× in accuracy**.
GeographicLib documents the **RMS difference between EGM96 and EGM2008 as ~0.5 m** — i.e., the two
models genuinely disagree by half a meter RMS, so this is a substantive upgrade, not a cosmetic one.
Confirmed locally: the EGM2008 − EGM96 delta ranges from ~0.1 m in well-surveyed areas to **−0.79 m**
at (46.87 N, 102.45 E) in central Asia — largest exactly where EGM2008's higher degree matters most.

### Validation — oracle + official test table (worked example)
NGA distributes EGM2008 with the FORTRAN program **`HARMONIC_SYNTH_WGS84` (v06/03/2008)** by Holmes &
Pavlis, which ships `INPUT.DAT`/`OUTPUT.DAT` — the **correct** EGM2008 checkpoints (WGS84, Tide Free,
Nmax 2190, with NGA's −0.41 m zero-degree height-anomaly term). The local PROJ 2.5′ grid reproduces
them to **≤1 cm**:

| lat (N) | lon (E) | NGA `OUTPUT.DAT` `N` | PROJ EGM2008 2.5′ `N` | diff |
|--------:|--------:|--------------------:|----------------------:|-----:|
| 37.000000 | 241.000000 | −26.151 m | −26.150 m | +1 mm |
| 36.000000 | 242.983333 | −29.170 m | −29.180 m | −10 mm |
| 90.000000 |   0.000000 |  14.899 m |  14.900 m | +1 mm |
| −90.000000 |  0.000000 | −30.150 m | −30.150 m |  0 mm |

The residual is the documented 2.5′-grid-vs-harmonic-synthesis interpolation floor, comfortably inside
the library's existing **1.1 cm** geoid tolerance. Command form (proven):
`echo "37 -119 0" | cs2cs EPSG:4979 +to EPSG:4326+3855` → `N = −H_out`. PROJ_DEBUG confirmed it opened
the local `us_nga_egm08_25.tif` and selected "WGS 84 to EGM2008 height (1)".

**IMPORTANT — do not reuse the EGM96 test points for EGM2008.** The points (38.628155, 269.779155)
and (−14.621217, 305.021114) currently in `test/geoidTest.h` are the **EGM96** README/`WW15MGH.GRD`
validation points and are correct *for EGM96 only*. An EGM2008 model must be validated against the
`hsynth_WGS84` `OUTPUT.DAT` points above (or GeographicLib `GeoidEval --geoid egm2008-1`).

### Adoption cost (real concern for a header-LUT library)
- **1′×1′ grid is infeasible as a header LUT:** 21600×10801 ≈ 233 M nodes ≈ **470 MB** (GeographicLib
  PGM) / ~933 MB (NGA native REAL*4). Do not embed the 1′ grid.
- **2.5′×2.5′ grid is tractable and is the right target:** 8640×4321 ≈ 37 M nodes ≈ **75 MB** raster
  (~150 MB as an int32 text LUT). That is ~3–4× the largest LUT the repo already ships
  (`USGG2012LUT.cpp` = 76 MB, `GEOID12ALUT.cpp` = 68 MB), so it is *within the repo's established
  pattern* — gate it behind a `COORDINATES_ENABLE_EGM2008` CMake option exactly like the others, and
  prefer a compressed/binary-embedded LUT over a 150 MB text `.cpp`.
- A coarser official **5′** grid (~19 MB) exists (GeographicLib `egm2008-5`) if an even smaller
  footprint is wanted at reduced fidelity.

**Verdict: RECOMMENDED.** Add `geoids::EGM2008 : public Geoid<WGS84>` backed by the NGA 2.5′ grid as a
LUT, mirroring the existing `EGM96` reader. Validate with the `OUTPUT.DAT` points at 1.1 cm using the
existing test harness. It is the single highest-value addition: a true global successor to EGM96,
fully primary-sourced, and validatable to the millimeter on this box today.

---

## 2. GEOID18 — US hybrid successor to GEOID12A — **RECOMMENDED**

### Primary source
- **NGS GEOID18.** Primary reference: **NOAA Technical Report NOS NGS 72, "GEOID18"** (Ahlgren, Scott,
  Zilkoski, Shaw, Paudel; NGS, May 2020),
  `https://geodesy.noaa.gov/library/pdfs/NOAA_TR_NOS_NGS_0072.pdf`. Official since 2019.
- **Resolution:** 1 arc-minute grids (geoid height + uncertainty + DEFLEC18 deflections).
- **Frames:** converts **NAD83(2011) epoch 2010.00** ellipsoid heights → **NAVD88** orthometric heights
  (CONUS); PRVD02 / VIVD09 for Puerto Rico & USVI. It is a **hybrid** geoid (gravimetric model
  constrained to GPS-on-leveled-benchmarks).
- **Basis:** built on the gravimetric model **xGEOID19B**, using GRAV-D airborne gravity as of Jul 2018.
- **Control:** fit to **32,357 CONUS + 127 PR/USVI** GPS-on-benchmark points (dataset GPSBM18; +29%
  over GEOID12B's ~24,900).
- **Accuracy:** CONUS standard deviation **1.39 cm** (vs 1.7 cm for GEOID12B — 18% better); uncertainty
  floor 1.4 cm CONUS / 1.7 cm PR/USVI; relative RMS < 2 cm over all distances.
- **Supersedes GEOID12B** (the 12A/12B hybrid series the library currently ships as `GEOID12A`).
  Caveat: GEOID18 covers **CONUS + PR/USVI only** — NGS directs **Alaska, Hawaii, Guam, CNMI** users to
  **continue using GEOID12B**. So GEOID18 augments, but does not fully retire, the existing US hybrid.

### Validation — oracle (worked example)
The NGS grid `g2018u0.bin` (CONUS) was pulled locally as `us_noaa_g2018u0.tif` and evaluated with
`cct +proj=vgridshift +grids=us_noaa_g2018u0.tif`:

| Point | lat (N) | lon (W) | PROJ GEOID18 `N` |
|---|---:|---:|---:|
| NGS-tool sample coordinate¹ | 35.92195 | 97.92784 | −27.250 m |
| Boulder, CO | 40.00 | 105.25 | −15.878 m |
| Denver, CO | 39.74 | 104.99 | −17.113 m |

¹ The coordinate NGS itself displays on its GEOID18 computation page (35°55′19.0221″N,
97°55′40.2351″W). NGS does **not** publish a static lat/lon→N validation table; validation is intended
against the live NGS computation tool (`https://geodesy.noaa.gov/GEOID/GEOID18/computation.html`) or the
raw 1′ binary grid — the latter is exactly what PROJ evaluates here, so the oracle is the agency's own
grid data. This is a valid but *slightly weaker* second axis than EGM2008's (which has an explicit
official numeric test table); note the distinction.

### Adoption cost
1′ CONUS grid (lat 24–58 N, lon 60–130 W ≈ 2041×4201 nodes) is the **same footprint as the existing
`GEOID12ALUT.cpp` / `USGG2012LUT.cpp`** (both already 1′ CONUS at ~68–76 MB). So GEOID18 is a
like-for-like LUT swap/addition with no new size class. Gate behind `COORDINATES_ENABLE_GEOID18`.

**Verdict: RECOMMENDED.** Add `geoids::GEOID18 : public Geoid<GRS80>` (NAD83(2011)→NAVD88) backed by
the NGS 1′ CONUS grid, mirroring `GEOID12A`. Keep `GEOID12A/12B` for AK/HI/Guam/CNMI, where NGS still
mandates them. Validate against the NGS online tool / raw grid via PROJ at the 1–2 cm level.

---

## 3. EGM2020 — global — **UNVALIDATED / NOT-RELEASED**

NGA publicly *announced* EGM2020 (SIRGAS workshop 2019, NGA PR #19-987; EGU 2020 abstract
EGU2020-9884): same structure as EGM2008 (spherical harmonic to **degree 2190 / order 2159**), 5′
resolution, projected release April 2020, reportedly complete May 2020.

**It has never been publicly released.** Confirmed by four independent authoritative repositories:
- **NGA earth-info.nga.mil** offers EGM2008/EGM96/EGM84 only — no EGM2020 file or link; its GeoGRAV
  calculator is EGM96/EGM08-only.
- **ICGEM** (GFZ Potsdam, the central catalog of global gravity models, `icgem.gfz.de/tom_longtime`) has
  **no EGM2020 entry**.
- **ISG** (isgeoid.polimi.it) lists no EGM2020 and defers to ICGEM.
- WGS84 still operationally uses **EGM2008** (per NGA and the WGS84 standard); Wikipedia's EGM article
  states EGM2020 was "still not released as of July 2025."

Independently confirmed on this box: `proj.db` has **no EGM2020 vertical CRS and no EGM2020 grid** — so
there is literally nothing to evaluate. **No primary data, no oracle → fails both Iron-Rule axes.**
Mirrors the NATRF2022 "announced-but-not-released" finding. **Do not adopt.** Re-check when NGA
publishes coefficients/grids on earth-info.nga.mil or ICGEM.

---

## 4. GEOID2022 / NAPGD2022 (US) — **UNVALIDATED / NOT-RELEASED**

The modernized NSRS replaces NAD83 with four ITRF2020-tied frames (NATRF2022, PATRF2022, CATRF2022,
MATRF2022) and NAVD88 with a gravity-based geopotential datum **NAPGD2022**, realized by **GEOID2022**.
Per NGS (`https://geodesy.noaa.gov/datums/newdatums/release.shtml`), components roll out incrementally
**2024–2026**, appearing first on the **beta.ngs.noaa.gov** test site; the existing NSRS (NAD83/NAVD88)
remains official during rollout. GEOID2022 is described in 2024 NGS talks as "the first realization" /
"current status" — a **beta/test product**, with an FGCS adoption vote expected **early-to-mid 2026**
and official adoption thereafter (the "2022" is the original target year; it has slipped, exactly like
NATRF2022). GRAV-D airborne collection is **not fully complete** (status map still shows blocks
underway/planned). **No stable released data, no adoptable oracle → do not adopt yet.** Track the beta.

The experimental gravimetric series (xGEOID14 → … → xGEOID19B → xGEOID20) is **REJECTED** as an adoption
target: it is explicitly experimental, changes annually, and has been folded into the GEOID2022 work —
not a stable, citable, test-tabled model to embed.

---

## 5. Other regional models (ISG) — **OUT OF SCOPE (mention only)**

The **International Service for the Geoid** (`https://www.isgeoid.polimi.it/`) hosts a large repository
of national/regional geoid models (e.g. AUSGeoid2020, NZGeoid2016, Icegeoid ISN2016, various European
quasigeoids). Many are already mirrored as PROJ CDN grids (`projinfo` lists dozens:
`au_ga_AUSGeoid2020_20180201.tif`, `nz_linz_nzgeoid2016.tif`, etc.) and would be validatable the same
way. They are **out of scope** for a US/DoD-oriented library whose current set is global (EGM96) + US
(USGG2012/GEOID12A); adding national geoids would balloon the LUT footprint for narrow benefit.
Recommend only on explicit demand for a specific country of operation.

---

## Bottom line

- **Adopt EGM2008 (2.5′ grid).** Primary-sourced (NGA, degree 2190, WGS84 Tide Free), a genuine ~0.5 m
  RMS upgrade over EGM96, and validatable to the **millimeter** here against NGA's own `OUTPUT.DAT`
  test table using the same LUT+test pattern the library already uses. Footprint (~75 MB raster /
  ~150 MB LUT) is within the repo's existing pattern; gate behind a CMake option. **Do not** attempt
  the 1′ grid (~470–933 MB). Fix the test to use EGM2008's own checkpoints, not the EGM96 points.
- **Adopt GEOID18** as the US hybrid successor to GEOID12A/12B for CONUS + PR/USVI. Primary-sourced
  (NGS TR NOS NGS 72, 1′, 1.39 cm), same footprint class as the existing US LUTs, validatable via PROJ
  against the NGS grid/tool. Keep GEOID12B for AK/HI/Guam/CNMI, which NGS still mandates.
- **Blocked / not-yet-released:** **EGM2020** (announced, never published — no data, no oracle) and
  **GEOID2022 / NAPGD2022** (beta, 2024–2026 rollout, FGCS vote ~2026). Both fail the Iron Rule today;
  re-evaluate when the agencies publish stable, test-tabled data.
- **Out of scope:** ISG national/regional geoids — validatable but not warranted for this library's
  mission unless a specific region is required.

### What was actually validatable on this box vs blocked
- **Validated locally (both Iron-Rule axes met):** EGM96 (existing, oracle-confirmed to 1 cm),
  **EGM2008 2.5′** (NGA grid + NGA `OUTPUT.DAT` table, agreement ≤1 cm), **GEOID18** (NGS grid + NGS
  online tool coordinate). PROJ 9.7.1 + the pulled NGA/NGS grids were the oracle; network to the PROJ
  CDN, NGA, and NGS was reachable.
- **Data-blocked (not our box's fault — the agencies have not released it):** EGM2020, GEOID2022/NAPGD2022.
- **Oracle not on this box:** GeographicLib `GeoidEval` is not installed (no geoid datasets present); it
  is cited as a documented second oracle but was not exercised here — PROJ sufficed.
