"""Constants and the measurement farm shared by the golden tests (legacy kpkpxim naming)."""
import math

import pytest

from gxana.config import export_channel_kv

# Tree stems of the three thesis run periods, in the order the legacy
# MakeXSecFitVariations.C processed them (fit parameters carry over).
PERIOD_TREES = (
    "kpkpxim__M23_2017-01_ana56",
    "kpkpxim__B4_M23_2018-01_ana03",
    "kpkpxim__B4_M23_2018-08_ana02",
)

# Legacy FitFunctions.cpp GetFluxHist: period -> flux file.
FLUX_FILES = {
    "kpkpxim__M23_2017-01_ana56": "flux/flux_30274_31057_r4.root",
    "kpkpxim__B4_M23_2018-01_ana03": "flux/flux_40856_42559.root",
    "kpkpxim__B4_M23_2018-08_ana02": "flux/flux_50685_51768.root",
}

# (label, period) of the legacy components/ layout.
PERIOD_LABELS = (("sp17", "2017-01"), ("sp18", "2018-01"), ("fa18", "2018-08"))

# Lower edges of the eight beam-energy bins, as they appear in file names.
ENERGY_BIN_LOWS = ("6.40", "7.40", "7.86", "8.19", "8.45", "8.68", "9.26", "10.18")


def measurements_farm(tmp_path, need):
    src = need(*[f"flat_trees/postQVal_flatTree_{s}_nominal_kphighrap_1111111.root" for s in PERIOD_TREES],
               *[f"flat_trees/flatTree_{s}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root" for s in PERIOD_TREES],
               *[f"flat_trees/flatTree_thrown_{s}_gen_amp_V2_ac_YstarRest.root" for s in PERIOD_TREES])
    data, out = tmp_path / "data" / "flatTrees", tmp_path / "out"
    data.mkdir(parents=True)
    (out / "kpkpxim" / "prod_plots").mkdir(parents=True)
    export_channel_kv("kpkpxim", out / "kpkpxim" / "config" / "channel.kv")  # the period list (XimInputs.h)
    by_name = {p.name: p for p in src}
    for s in PERIOD_TREES:
        q = by_name[f"postQVal_flatTree_{s}_nominal_kphighrap_1111111.root"]
        d = out / "kpkpxim" / "qfactors" / f"{s}_nominal_kphighrap_1111111"
        d.mkdir(parents=True)
        (d / q.name).symlink_to(q)
        (data / f"flatTree_{s}_nominal_kphighrap.root").symlink_to(q)  # stand-in for the plain data tree
        for n in (f"flatTree_{s}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root",
                  f"flatTree_thrown_{s}_gen_amp_V2_ac_YstarRest.root"):
            (data / n).symlink_to(by_name[n])
    return tmp_path / "data", out


def parse_fitresults(text):
    """FITRESULT lines -> {(kind, name, occurrence): [(key, value string), ...]}. The three mass
    periods share a name, so the n-th occurrence of (kind, name) is part of the key (period order)."""
    out, seen = {}, {}
    for line in text.splitlines():
        if not line.startswith("FITRESULT"):
            continue
        _, kind, name, *kv = line.split()
        n = seen[(kind, name)] = seen.get((kind, name), -1) + 1
        out[(kind, name, n)] = [tuple(x.split("=", 1)) for x in kv]
    return out


def same_value(got, want):
    g, w = float(got), float(want)
    if not (math.isfinite(g) and math.isfinite(w)):
        return got == want
    return g == pytest.approx(w, rel=1e-9, abs=1e-12)
