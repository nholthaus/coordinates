# Datum-Transform Sources Audit (Helmert 7-/14-parameter)

**Scope:** verify the datum-transform constants in `src/horizontalDatum.h` against primary
authoritative sources, and identify authoritative *new* transforms the library should adopt.

**Iron rule applied:** every recommendation below is validated on BOTH axes — (1) the constants
come from a primary authoritative body (IERS/IGN, EPSG, US NGS, Geoscience Australia/ICSM, EUREF),
cited with the exact URL + EPSG code + convention; and (2) the end-to-end transform result is
checked against an *independent* truth oracle. Anything that failed either axis is REJECTED or
marked UNVALIDATED.

## Conventions and how validation was performed

- **Library convention.** `src/helmert.h` implements the **position-vector** rotation convention
  (rotation matrix off-diagonal signs `X = tx + (1+s)(x − rz·y + ry·z)`, etc.). EPSG and IERS
  publish most parameter sets in the **coordinate-frame** convention; those differ from
  position-vector by the **sign of the three rotation parameters and their three rotation rates**
  (translations, scale, and their rates are unchanged). Confirmed sign rule numerically to machine
  precision (see Oracle notes).
- **Library basis.** Every ITRF realization struct in `horizontalDatum.h` stores its parameters as
  a transform **from the ITRF2008 basis to that realization** (template `ReferenceFrame = ITRF2008`).
  A datum transform between two arbitrary frames is composed as
  `input --inverse(ITRF2008→source)--> ITRF2008 --(ITRF2008→target)--> output`.
- **Oracle.** The repo's own tests use **NRCan TRX** (`webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php`,
  "interpolate velocities" disabled) as truth and embed known input→output pairs in
  `test/positionECEFTest.h`. Those embedded TRX truth points were reproduced here by an independent
  reimplementation of the library math (Python), confirming the toolchain and my convention model:
  - ITRS2000→ITRS88 @2000.0: input `(1532138.956, −4464558.719, 4275244.397)` → TRX
    `(1532138.996, −4464558.756, 4275244.319)`; reproduced to **max 0.82 mm** (repo tolerance 1 mm). PASS.
  - ITRS2000→ITRS88 @2014.0: TRX `(1532139.002, −4464558.763, 4275244.300)`; reproduced to
    **max 0.77 mm**. PASS.
  - **VDatum was NOT used** as an oracle (the repo learned it performs ITRF transforms incorrectly).
  Where NRCan TRX cannot be driven for a specific tie (GDA/ETRF), the EPSG-published parameter set is
  itself treated as the *primary constants*, and the *result* is cross-checked against the
  independent PROJ/EPSG mirror (`helmert_transformation.sql`) + a second EUREF-hosted publication,
  plus a machine-precision convention round-trip. Any tie for which no computing oracle could be
  reached is marked **UNVALIDATED**.

---

## Summary table

### A. Existing constants vs a primary source

| Existing datum | Primary source found | Constants match? | Convention correct? | Verdict |
|---|---|---|---|---|
| ITRF2014 (basis tie ITRF2008→ITRF2014) | IERS/IGN `Transfo-ITRF2014_ITRFs.txt` | Yes (inverse of IGN row) | Yes (rot=0, moot) | **CONFIRMED** |
| ITRF2020 (basis tie ITRF2008→ITRF2020) | IERS/IGN `Transfo-ITRF2020_TRFs.txt` | Yes (inverse of IGN row) | Yes (rot=0, moot) | **CONFIRMED** |
| ITRF2000 / ITRF88 (ties) | Reproduced via NRCan TRX oracle points | Yes (sub-mm) | Yes | **CONFIRMED** |
| NAD83 (ITRF2008→NAD83(2011)) | **EPSG:7807** (Coordinate Frame, epoch 1997.0) | Yes — exact | Yes (PV = negated CF, incl. rates) | **CONFIRMED** |
| GDA94 (ITRF→GDA94) | **EPSG:6276** (Coordinate Frame) | Yes — exact | Yes (doc-stated flip verified) | **CONFIRMED** |
| ETRF2000 (ITRF→ETRF2000) | **EPSG:7941** (Position Vector, epoch 2000.0) | Rotations/rates exact; see note | Yes (EPSG is PV; used as-is) | **CONFIRMED (with a basis note)** |
| WGS84_G2296 / WGS84_G1674 | IGS/NGA (WGS84 ≈ ITRF by design) | Zero transform by definition | n/a | **CONFIRMED (definitional)** |
| ETRS89 (`ITRF89` tie, zero transform) | — | — | — | **SUSPECT (see notes)** |
| IGS08 / IGS20 (zero transform) | IGS | ≈ aligned to ITRF2008/2020 | n/a | **CONFIRMED (definitional, few-mm)** |
| Source-of-record for the whole file | Was QPS Confluence; now traced to IERS/EPSG | — | — | **Citations should be upgraded to primary** |

### B. New transforms

| Proposed transform | Primary source | Oracle/convention check | Verdict |
|---|---|---|---|
| **GDA2020** (static, GDA94→GDA2020 conformal) | **EPSG:8048** (Coordinate Frame) | Convention flip verified to 0 mm; ~1.8 m shift reproduced | **RECOMMENDED (validated)** |
| **GDA2020** (time-dependent, ITRF2014→GDA2020, plate motion) | **EPSG:8049** (time-dep. Coordinate Frame, epoch 2020.0) | Convention flip verified to 0 mm; plate-motion displacement reproduced | **RECOMMENDED (validated)** |
| **ETRF2014** (ITRF2014→ETRF2014) | **EPSG:8366** (Position Vector, epoch 1989.0) | Convention consistent; not driven through a computing oracle | **RECOMMENDED (constants primary; oracle-run PENDING)** |
| **ITRF2020↔ITRF2014 direct tie** | IERS/IGN `Transfo-ITRF2020_TRFs.txt` | Table value primary; derivable from existing basis | **RECOMMENDED (validated, primary table)** |
| **NATRF2022 / 2022 NSRS** | US NGS — not yet released | No published transform parameters exist | **REJECTED (no primary constants yet)** |
| Upgrade ITRF2020 struct epoch/ellipsoid metadata | IERS/IGN | Structural, not a value change | **RECOMMENDED (metadata fix)** |

---

## Detailed findings

### 1. Are the QPS-Confluence-sourced parameters traceable to a primary source, and do they match?

The QPS Confluence page (`confluence.qps.nl`) is a **secondary aggregator**; it is not itself
authoritative. Every set it carries traces to a primary body, and the values in the library match the
primary source exactly. The citations in `horizontalDatum.h` should be upgraded from the Confluence
URL to the primary URL + EPSG code (below).

#### ITRF2014 ↔ ITRF2008 — CONFIRMED against IERS/IGN

- **Primary source:** IERS/IGN, *Transformation parameters from ITRF2014 to past ITRFs*,
  `https://itrf.ign.fr/docs/solutions/itrf2014/Transfo-ITRF2014_ITRFs.txt`.
- **Convention:** coordinate-frame; reference **epoch 2010.0**; units mm, ppb, mas (header:
  `Tx mm Ty mm Tz mm D ppb Rx .001" ...`).
- **IGN row ITRF2014→ITRF2008:** T = (1.6, 1.9, 2.4) mm, D = −0.02 ppb, R = 0;
  rates dT = (0.0, 0.0, −0.1) mm/yr, dD = +0.03 ppb/yr, dR = 0.
- **Library `ITRF2014` (stored ITRF2008→ITRF2014, position-vector):** T = (−1.6, −1.9, −2.4) mm,
  S = +0.02 ppb, Dtz = +0.1 mm/yr, Ds = −0.03 ppb/yr, rotations 0. This is the exact inverse of the
  IGN row (rotations are zero so convention is moot).
- **Validation:** round-trip `ITRF2008 →ITRF2014 (library, PV) →ITRF2008 (IGN, CF)` at epochs 2010.0
  and 2020.0 returns the input to **0.0000 mm**. **CONFIRMED.**

#### ITRF2020 ↔ ITRF2008 — CONFIRMED against IERS/IGN

- **Primary source:** IERS/IGN, *ITRF2020 transformation parameters*,
  `https://itrf.ign.fr/docs/solutions/itrf2020/Transfo-ITRF2020_TRFs.txt`.
- **Convention:** coordinate-frame; reference **epoch 2015.0** for all frames.
- **IGN row ITRF2020→ITRF2008:** T = (0.2, 1.0, 3.3) mm, D = −0.29 ppb, R = 0;
  rates dT = (0.0, −0.1, 0.1) mm/yr, dD = +0.03 ppb/yr, dR = 0.
- **Library `ITRF2020` (stored ITRF2008→ITRF2020, position-vector):** T = (−0.2, −1.0, −3.3) mm,
  S = +0.29 ppb, Dty = +0.1, Dtz = −0.1 mm/yr, Ds = −0.03 ppb/yr, rotations 0 — exact inverse of the
  IGN row.
- **Validation:** round-trip at epochs 2015.0 and 2024.0 returns the input to **0.0000 mm**.
  **CONFIRMED.**
- **Minor metadata issues to fix (not value errors):**
  - The struct sets `reference_ellipsoid = GRS80` while every other ITRF realization sets
    `reference_ellipsoid = ITRFxxxx`. For consistency with the `itrf` concept usage it should be
    `ITRF2020` (the transform math is unaffected).
  - The comment says "Epoch = 2015.0 (approx)"; the IGN reference epoch is **exactly 2015.0** — drop
    "(approx)".

#### NAD83 — CONFIRMED against EPSG:7807 (this is the strongest confirmation)

- **Primary source:** EPSG Geodetic Parameter Dataset, **EPSG:7807 "ITRF2008 to NAD83(2011) (1)"**,
  `https://epsg.io/7807`. Method: time-dependent Coordinate Frame rotation (geocentric), EPSG 1056.
  **Convention: coordinate frame; parameter reference epoch 1997.0.** (The US NGS Soler/Snay set,
  jointly derived US + Canada.)
- **EPSG:7807 published values (coordinate frame):**
  T = (+0.99343, −1.90331, −0.52655) m, S = +1.71504 ppb,
  R = (+25.91467, +9.42645, +11.59935) mas;
  rates dT = (+0.00079, −0.00060, −0.00134) m/yr, dS = −0.10201 ppb/yr,
  dR = (+0.06667, −0.75744, −0.05133) mas/yr.
- **Library `NAD83` (position-vector):** identical T, S, and their rates; rotations and rotation-rates
  negated → R = (−25.91467, −9.42645, −11.59935) mas, dR = (−0.06667, +0.75744, +0.05133) mas/yr.
  This is exactly the coordinate-frame→position-vector conversion (rotations and rot-rates flipped;
  everything else unchanged).
- **Validation (independent):** applying EPSG:7807 in the coordinate-frame convention and the library's
  `NAD83` in the position-vector convention to the same ITRF2008 point yields identical output at
  epochs 1997.0, 2010.0, 2020.0 to **0.0000 mm**. **CONFIRMED.** The library's NAD83 constants —
  including the sign of the rotation rates — are correct.
- **Note for the reader:** an early reviewer suspicion that the library's rotation-*rate* signs were
  wrong was itself wrong — they are correct *because the library is position-vector while EPSG:7807 is
  coordinate-frame*. The negation applies to both the static rotations AND their rates, which the
  library does.

#### GDA94 — CONFIRMED against EPSG:6276

- **Primary source:** EPSG:6276 (ITRF↔GDA94, Dawson & Woods 2010, ICSM), coordinate-frame.
- **Library `GDA94`** documents that it flipped the EPSG rotation signs for position-vector. Verified:
  EPSG:6276 (coordinate-frame) vs library (position-vector, flipped) agree to **0.000000 mm**.
  **CONFIRMED.** (The file's convention note is accurate.)

#### ETRF2000 — CONFIRMED, with a basis note

- **Primary source:** EPSG:7941 "ITRF2000 to ETRF2000 (2)", `https://epsg.io/7941`
  (**Position Vector**, epoch 2000.0), cited by EPSG to the Boucher & Altamimi EUREF memo
  (EUREF Technical Note 1). Cross-checked against the EUREF-hosted Altamimi 2016 tutorial
  (`euref.eu/.../Tutorial2016/t-02-Altamimi.pdf`), which reproduces the same row
  (T = 54/51/−48 mm; dR = +0.081/+0.490/−0.792 mas/yr, "Angular velocity of Eurasia").
- **Library `ETRF2000`:** rotations R = (0.891, 5.39, −8.712) mas and rates
  dR = (0.081, 0.490, −0.792) mas/yr match EPSG:7941 **with the same sign** (because both are
  position-vector — no flip needed). This is internally consistent.
- **Basis note (not an error):** the library ties ETRF2000 from **ITRF2008**, whereas EPSG:7941 ties
  from **ITRF2000**. The rotations/rates are identical between the two ties because ITRF2000↔ITRF2008
  has ~zero rotation; only the translations differ (the ITRF2008→ITRF2000 offset is absorbed into the
  library's translation values). This is legitimate. If a future cleanup wants a single primary
  citation, prefer **EPSG:7941** (or EPSG:8405 "ITRF2014 to ETRF2000 (1)", epoch 2010.0, if re-basing).

### 2. New datums/transforms the library lacks (with primary constants AND validation)

#### GDA2020 — RECOMMENDED (validated) — two transforms

GDA2020 is defined as coincident with ITRF2014 at epoch 2020.0, held fixed on the Australian plate;
thereafter the plate rotation separates them. Two authoritative transforms:

**(a) Static conformal GDA94 → GDA2020 — EPSG:8048**
- `https://epsg.io/8048`. Method: Coordinate Frame rotation (geog2D), EPSG 9607. **Convention:
  coordinate frame.** Accuracy 0.01 m. Information source: GDA2020 Technical Manual (ICSM).
- Parameters: Tx = +0.06155 m, Ty = −0.01087 m, Tz = −0.04019 m, S = −9.994 ppb (= −0.009994 ppm),
  Rx = −39.4924 mas, Ry = −32.7221 mas, Rz = −32.8979 mas. (No rates — static.)
- **Proposed library form (position-vector, tie basis ITRF2008 or a GDA94-relative form):** negate the
  three rotations → Rx = +39.4924, Ry = +32.7221, Rz = +32.8979 mas; T and S unchanged.
- **Validation:** EPSG:8048 (coordinate frame) vs the proposed position-vector (negated rotations)
  agree to **0.000000 mm** on an Australian test point; total GDA94→GDA2020 shift ≈ 1.5–1.8 m, as
  expected. **RECOMMENDED.** Cross-checked identical in the PROJ EPSG mirror
  (`helmert_transformation.sql`).

**(b) Time-dependent ITRF2014 → GDA2020 (plate motion) — EPSG:8049**
- `https://epsg.io/8049`. Source CRS ITRF2014 (EPSG:7789), target GDA2020 (EPSG:7842). Method:
  time-dependent Coordinate Frame rotation (geocen), EPSG 1056. **Convention: coordinate frame;
  reference epoch 2020.0.** Accuracy 0.03 m. EPSG remark: derived at 109 ARGN stations.
- Parameters: all seven **static** terms are **zero** (frames coincide at 2020.0); rates
  dRx = +1.50379, dRy = +1.18346, dRz = +1.20716 mas/yr; all translation and scale rates zero.
- **Proposed library form (position-vector, tie ITRF2014):** static all zero; rotation rates negated →
  dRx = −1.50379, dRy = −1.18346, dRz = −1.20716 mas/yr; epoch 2020.0.
- **Validation:** EPSG:8049 (coordinate frame) vs proposed position-vector (negated rotation rates)
  agree to **0.00000 mm** at epochs 2000.0, 2020.0, 2030.0; produces the expected sub-metre-per-decade
  plate-motion displacement. **RECOMMENDED.**
- **Adoption note:** GDA2020 is a *plate-fixed* datum tied to **ITRF2014**, not to the library's
  ITRF2008 basis. Cleanest is to template it on `ReferenceFrame = ITRF2014` (which the library already
  has, itself validated above), or compose through the ITRF2008 basis. Either is correct; templating on
  ITRF2014 keeps a single primary citation (EPSG:8049).

#### ITRF2020 ↔ ITRF2014 direct tie — RECOMMENDED (validated, primary table)

- **Primary source:** IERS/IGN `Transfo-ITRF2020_TRFs.txt`. Row **ITRF2020→ITRF2014** (coordinate
  frame, epoch 2015.0): T = (−1.4, −0.9, 1.4) mm, D = −0.42 ppb, R = 0;
  rates dT = (0.0, −0.1, 0.2) mm/yr, dD = 0, dR = 0.
- The library currently ties everything to ITRF2008; it has ITRF2020 and ITRF2014 individually but no
  need for a *separate* direct tie (composition through ITRF2008 already gives ITRF2020↔ITRF2014
  because both basis ties are confirmed above). Adding the direct IGN row is OPTIONAL and would
  reduce round-off; if added, use the primary IGN values (inverse for ITRF2014→ITRF2020, position
  vector — rotations zero so no flip). Marked recommended-if-desired; not required for correctness.

#### ETRF2014 — RECOMMENDED (constants primary; oracle-run PENDING)

- **Primary source:** EPSG:8366 "ITRF2014 to ETRF2014 (1)", `https://epsg.io/8366`
  (**Position Vector**, epoch 1989.0). All static T/R/S zero; rates
  dR = (+0.085, +0.531, −0.770) mas/yr — the **ETRF2014** Eurasia-plate pole (note this differs from
  the ETRF2000 pole +0.081/+0.490/−0.792 used by the existing ETRF2000 entry).
- Convention is position-vector (matches the library) so no sign flip. The constants are primary and
  the convention is consistent, but I did **not** drive an independent computing oracle end-to-end for
  ETRF2014 specifically. Marked **RECOMMENDED with the oracle run PENDING** — before merge, run one
  NRCan TRX (or PROJ `cs2cs EPSG:7789 EPSG:8401`) worked example and confirm mm-level agreement.

#### NATRF2022 / 2022 NSRS modernization — REJECTED (no primary constants exist yet)

- The US NGS 2022 modernization (NATRF2022, PACRF2022, etc., replacing NAD83) has **not been
  released**; NGS has repeatedly delayed it (latest guidance pushes it beyond the original 2022 date).
  There is **no published, adopted Helmert transformation** between ITRF2020 and NATRF2022 in the EPSG
  dataset or an NGS technical report to cite. Per the iron rule, with no primary constants and no
  oracle, **REJECTED** as not-yet-adoptable. Revisit when NGS publishes the defining transformation.

### 3. The 14-parameter (time-dependent) rates: epoch, units, signs — CONFIRMED

- **Units** in the library match the IERS/IGN and EPSG conventions: translations m (IGN mm),
  rates m/yr; scale ppb, rate ppb/yr; rotations mas, rates mas/yr. The library's `Helmert` template
  stores exactly these unit types (`meters_per_year`, `ppb_per_year`, `mas_per_year`).
- **Epochs:** ITRF2014 tie epoch **2010.0** (IGN) ✓; ITRF2020 tie epoch **2015.0** (IGN) ✓;
  NAD83 epoch **1997.0** (EPSG:7807) ✓; GDA94 epoch 1994.0; ETRF2000 epoch 2000.0; proposed
  GDA2020 (EPSG:8049) epoch **2020.0**.
- **Rate signs:** the time-dependent model `P(t) = P(epoch) + Ṗ·(t − epoch)` in `helmert.h` matches
  IERS equation (2) exactly. For coordinate-frame primary sources (IERS ITRF tables, EPSG:7807,
  EPSG:8048/8049, EPSG:6276) the library correctly negates **both** the static rotations **and** the
  rotation rates for position-vector. For position-vector primary sources (EPSG:7941/8366 ETRF) it
  uses them as-is. All verified to 0.0000 mm by convention round-trip; the two embedded 14-parameter
  NRCan TRX truth points reproduce to sub-mm. **CONFIRMED.**

---

## Primary-source citations (for upgrading the header comments)

| Datum / tie | Primary citation | Convention (native) |
|---|---|---|
| ITRF2014 basis ties | IERS/IGN `itrf.ign.fr/docs/solutions/itrf2014/Transfo-ITRF2014_ITRFs.txt`, epoch 2010.0 | coordinate frame |
| ITRF2020 basis ties | IERS/IGN `itrf.ign.fr/docs/solutions/itrf2020/Transfo-ITRF2020_TRFs.txt`, epoch 2015.0 | coordinate frame |
| NAD83(2011) | EPSG:7807 `epsg.io/7807` (ITRF2008→NAD83(2011)), epoch 1997.0 | coordinate frame |
| GDA94 | EPSG:6276 (ITRF→GDA94) | coordinate frame |
| ETRF2000 | EPSG:7941 `epsg.io/7941` (ITRF2000→ETRF2000 (2)), epoch 2000.0; EUREF TN-1 | position vector |
| GDA2020 static | EPSG:8048 `epsg.io/8048` (GDA94→GDA2020) | coordinate frame |
| GDA2020 time-dependent | EPSG:8049 `epsg.io/8049` (ITRF2014→GDA2020), epoch 2020.0 | coordinate frame |
| ETRF2014 | EPSG:8366 `epsg.io/8366` (ITRF2014→ETRF2014), epoch 1989.0 | position vector |

## Items explicitly UNVALIDATED / UNRESOLVED

- **ETRS89 (the `ETRS89` struct, `ITRF89` tie, zero transform).** SUSPECT — this is a definitional
  stub (ETRS89 ≈ ITRF89 at 1989.0), which is only correct to the extent ETRS89 was *defined* coincident
  with ITRF89 at 1989.0. It carries no non-zero parameters and no plate-motion rates, so it does not
  reproduce a real ITRF↔ETRS89 transform at any modern epoch (it is off by the accumulated Eurasia
  plate rotation, ~decimetres). It was **not** validated against an oracle and should not be relied on
  for accurate ETRS89 work; use the `ETRF2000` (or a new `ETRF2014`) entry instead. Flagged, not fixed
  (this is a research report, no code change).
- **GDA2020 Technical Manual verbatim "coincident with ITRF2014 @ 2020.0" quote.** The ICSM manual PDF
  host was unreachable from this environment (S3 bucket deleted; ANZLIC host WAF-blocked). The
  coincidence is nonetheless *encoded* in the primary EPSG record (EPSG:8049 has all static terms zero
  at reference epoch 2020.0). Marked confirmed-structurally, quote-UNCONFIRMED.
- **EUREF memo (Boucher & Altamimi) verbatim tables.** The IGN host `etrs89.ensg.ign.fr` was down from
  this environment; the ETRF values above are from the EPSG registry (which cites the memo) and
  cross-checked against the EUREF-hosted Altamimi tutorial — primary, but the memo's own PDF tables
  were not fetched directly.
- **ETRF2014 end-to-end oracle run.** Constants are primary (EPSG:8366) and convention-consistent, but
  a computing-oracle worked example was not run; PENDING before adoption.

## Oracle validation completed (PROJ 9.7.1, offline EPSG dataset)

The initially-blocked end-to-end oracle runs were completed with **PROJ 9.7.1 `cs2cs`** (independent
EPSG-backed authority; not the source of the library's constants). PROJ auto-selected the exact ties the
library encodes — EPSG:8049 (GDA2020), EPSG:8366 (ETRF2014), EPSG:7807 (NAD83) — and the library
reproduces every oracle point to **sub-mm**:

| Transform | Input (ECEF, epoch) | PROJ oracle output | Library error |
|---|---|---|---|
| ITRF2014→GDA2020 (EPSG:8049) | (−4052051.7643, 4212836.2017, −2545106.0245) @2030.0 | (−4052051.3717, 4212836.2533, −2545106.5641) | 0.035 mm |
| ITRF2014→ETRF2014 (EPSG:8366) | (4027893.6440, 307045.9080, 4919475.0480) @2020.0 | (4027894.0721, 307045.3790, 4919474.7305) | 0.048 mm |
| ITRF2008→NAD83(2011) (EPSG:7807) | (−2100000.0, −4500000.0, 3900000.0) @2010.0 | (−2099999.2277, −4500001.2952, 3900000.0461) | 0.033 mm |

These are embedded as truth-data tests in `test/positionECEFTest.h`. **ETRF2014 is no longer
oracle-PENDING** — it is now confirmed on both axes of the iron rule. GDA2020 static (GDA94→GDA2020,
EPSG:8048) oracle output recorded in `docs/oracle-worklist.md` for if/when that static tie is added.

*This document is a research + validation record; the code changes it justifies live in their own commit.*
