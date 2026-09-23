"""Golden: gxana_xsec_tables reproduces the legacy Johnson cross-section tables.

Legacy run (MakeXSecFitVariations.C main): Johnson signal + 2nd-order
Chebychev, weight hybrid_combo, label "johnson", the three periods in
PERIOD_TREES order sharing one parameter map. Several minutes.
Tolerance: GXANA_GOLDEN_RTOL (default 1e-5, i.e. the printed 6 digits). The
authoritative comparison runs with the thesis ROOT (6.24.04, analysis
container); on newer ROOT record the reported max deviation.
"""
import os
import subprocess

import pytest
from golden_data import FLUX_FILES, PERIOD_TREES

from gxana_xsection.compare import compare_dirs

pytestmark = pytest.mark.golden

JOHNSON = ("delta=1,0.2,1.5", "gamma=0,-0.5,0.5", "lambda=0.004,0.003,0.01", "mu=1.3217,1.31,1.33")


def test_johnson_tables_match_legacy(need, build_bin, tmp_path):
    ref = need("reference/xsection/johnson")[0]
    jobs = []
    for tree in PERIOD_TREES:
        data, mc, thrown, flux = need(
            f"binned_trees/binned_flatTree_{tree}_nominal_kphighrap.root",
            f"binned_trees/binned_flatTree_{tree}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root",
            f"binned_trees/binned_thrown_flatTree_{tree}_gen_amp_V2_ac_YstarRest.root",
            FLUX_FILES[tree],
        )
        jobs.append(f"flatTree_{tree}:{data}:{mc}:{thrown}:{flux}")
    out = tmp_path / "johnson"
    params = [arg for p in JOHNSON for arg in ("--param", p)]
    subprocess.run([str(build_bin / "gxana_xsec_tables"), "--fit", "Johnson", *params, "--label", "johnson",
                    "--weight", "hybrid_combo", "--cheby", "2", "--out", str(out), *jobs], check=True)
    rtol = float(os.environ.get("GXANA_GOLDEN_RTOL", "1e-5"))
    report = compare_dirs(out, ref, rtol=rtol)
    print(report.summary())
    assert report.ok, report.summary()
    assert len(report.results) == 54
