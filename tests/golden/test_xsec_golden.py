"""Golden: gxana_xsec_tables reproduces the legacy Johnson cross-section tables.

Legacy run (MakeXSecFitVariations.C main): Johnson signal + 2nd-order
Chebychev, weight hybrid_combo, label "johnson", the three periods in
PERIOD_TREES order sharing one parameter map. Several minutes.
Tolerance: GXANA_GOLDEN_RTOL (default 1e-5, i.e. the printed 6 digits). The
authoritative comparison runs with the thesis ROOT (6.24.04, analysis
container); on newer ROOT record the reported max deviation.

The Johnson/johnson command exercised here comes straight from
gxana.stages.xsection.plan_xsection (the "tables" step): we cut its argv
before the second --cheby to keep only the johnson label, and substitute the
JOB paths for the golden binned_trees/flux layout under
$GXANA_ANALYSIS_DATA/kpkpxim (the stage itself points JOBs at
$GXANA_OUTPUT/.../xsection/binned_trees, which golden data doesn't populate).
"""
import os
import subprocess

import pytest
from golden_data import FLUX_FILES, PERIOD_TREES

from gxana import config
from gxana.paths import repo_root
from gxana.stages import xsection as xs
from gxana_xsection.compare import compare_dirs

pytestmark = pytest.mark.golden


def test_johnson_tables_match_legacy(need, build_bin, tmp_path):
    ref = need("reference/xsection/johnson")[0]

    # environ only needs to be well-formed enough for plan_xsection to
    # resolve; the JOB paths it produces are replaced below with the golden
    # binned_trees/flux layout, and --out is replaced with tmp_path.
    environ = {"GXANA_ROOT": str(repo_root()), "GXANA_DATA": "/unused", "GXANA_OUTPUT": "/unused"}
    argv = xs.plan_xsection(config.load_channel("kpkpxim"), ["tables"], environ=environ)[0].argv
    cheby_positions = [i for i, a in enumerate(argv) if a == "--cheby"]
    argv = argv[:cheby_positions[1]]  # keep only the johnson label (drop johnson_cheby1)

    fit = argv[argv.index("--fit") + 1]
    params = [argv[i + 1] for i, a in enumerate(argv) if a == "--param"]
    weight = argv[argv.index("--weight") + 1]
    cheby = argv[argv.index("--cheby") + 1]
    label = argv[argv.index("--label") + 1]
    assert (fit, weight, cheby, label) == ("Johnson", "hybrid_combo", "2", "johnson")

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
    param_args = [arg for p in params for arg in ("--param", p)]
    subprocess.run([str(build_bin / "gxana_xsec_tables"), "--fit", fit, *param_args, "--label", label,
                    "--weight", weight, "--cheby", cheby, "--out", str(out), *jobs], check=True)
    rtol = float(os.environ.get("GXANA_GOLDEN_RTOL", "1e-5"))
    report = compare_dirs(out, ref, rtol=rtol)
    print(report.summary())
    assert report.ok, report.summary()
    assert len(report.results) == 54
