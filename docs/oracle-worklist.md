# Oracle Worklist — datum-transform truth validation

Run each item in the named tool, paste the output back. Goal: turn the "oracle-pending" datum
transforms into real truth-data tests (an independent tool's input→output pair), per the iron rule
(constants primary AND end-to-end result validated).

**Easiest paste format, per item:** `input X Y Z (epoch) → output X Y Z`, plus which tool, and — for
NRCan TRX — confirm **"interpolate velocities" was DISABLED**. If a tool wants/returns geodetic
lat/lon/height instead of ECEF X/Y/Z, paste whatever it gives and say which; I'll adapt.

**Iron-rule caution:** if a tool's exact tie differs from the EPSG code cited (e.g. a "(1)" vs "(2)"
variant), note the tie name it actually used — a mismatch is a real signal, not noise.

---

## ① GDA2020 — time-dependent (ITRF2014 → GDA2020, plate motion)

- **Tool A (authority):** Geoscience Australia transformer —
  https://www.ga.gov.au/scientific-topics/positioning-navigation/geodesy/datums-projections/transformation-grids
- **Tool B (PROJ):**
  ```
  echo "-4052051.7643 4212836.2017 -2545106.0245 2030.0" | cs2cs EPSG:7789 EPSG:7842
  ```
  (EPSG:7789 = ITRF2014 geocentric; EPSG:7842 = GDA2020 geocentric)
- **Input (ECEF, near Canberra):** X = -4052051.7643, Y = 4212836.2017, Z = -2545106.0245
- **Epoch:** 2030.0  (10 yr past the 2020.0 tie, so plate motion is visible)
- **PASTE BACK:** GDA2020 X Y Z (metres): ______________________________________________

## ② ETRF2014 (ITRF2014 → ETRF2014)  [the explicitly oracle-pending one]

- **Tool A (PROJ):**
  ```
  echo "4027893.6440 307045.9080 4919475.0480 2020.0" | cs2cs EPSG:7789 EPSG:8401
  ```
  (EPSG:8401 = ETRF2014 geocentric; transform is EPSG:8366). Any transformer that exposes EPSG:8366 works.
- **Tool B:** EUREF / IGN transformer (if reachable).
- **Input (ECEF, near Frankfurt):** X = 4027893.6440, Y = 307045.9080, Z = 4919475.0480
- **Epoch:** 2020.0
- **PASTE BACK:** ETRF2014 X Y Z (metres): ______________________________________________

## ③ NAD83 — fresh point (ITRF2008 → NAD83(2011))  [re-confirm existing tie]

- **Tool (canonical, the repo's oracle):** NRCan TRX —
  https://webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php
  **→ DISABLE "interpolate velocities".**
- **Input frame:** ITRF2008 (ITRS2008), ECEF; **Epoch:** 2010.0
- **Input (ECEF, CONUS):** X = -2100000.0, Y = -4500000.0, Z = 3900000.0
- **PASTE BACK:** output NAD83(2011) X Y Z (metres) + confirm input frame/epoch:
  ______________________________________________

## ④ GDA2020 — static conformal (GDA94 → GDA2020, EPSG:8048)

- **Tool A (PROJ):**
  ```
  echo "<GDA94 X Y Z>" | cs2cs EPSG:4938 EPSG:7842
  ```
  (EPSG:4938 = GDA94 geocentric; EPSG:7842 = GDA2020 geocentric). Or the GA 7-parameter conformal tool.
- **Input (GDA94 ECEF, near Canberra):** X = -4052051.7643, Y = 4212836.2017, Z = -2545106.0245
  (epoch irrelevant — static conformal)
- **PASTE BACK:** GDA2020 X Y Z (metres): ______________________________________________
- **NOTE:** validating this implies you also want the **static** GDA94→GDA2020 transform added
  (earlier you chose time-dependent only). Confirm you want the static one too and I'll add it.

---

## Oracle tool reference (links)

| Oracle | Link | Best for | Notes |
|---|---|---|---|
| NRCan TRX | https://webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php | ITRF↔ITRF, ITRF↔NAD83 | DISABLE "interpolate velocities". Repo's canonical truth. Does NOT do GDA/ETRF. |
| PROJ cs2cs / projinfo | (CLI) EPSG:7789/7842/8401/4938 | GDA2020, ETRF2014, GDA2020-static | Independent EPSG-backed oracle. |
| Geoscience Australia | https://www.ga.gov.au/scientific-topics/positioning-navigation/geodesy/datums-projections/transformation-grids | GDA94↔GDA2020, ITRF↔GDA2020 | Official AU authority. |
| EUREF / IGN memo | http://etrs89.ensg.ign.fr/pub/EUREF-TN-1-Mar-04-2024.pdf | ETRF verbatim tables | Was network-blocked from the build environment. |

## What I do with your results

For each pasted input→output pair I add a truth-data test in `test/positionECEFTest.h` (same form as the
existing NRCan-TRX ITRF cases: construct the input datum point, implicitly convert to the target datum,
`EXPECT_UNITS_NEAR` the oracle output to ~1 mm), citing the tool + tie in the test comment. Until then the
GDA2020/ETRF2014 tests assert composition + round-trip + the 0 mm sign-convention property only, and are
labeled "oracle pending".
