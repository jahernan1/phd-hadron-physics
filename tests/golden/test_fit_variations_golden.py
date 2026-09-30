"""Golden: a regression guard for the fit-model systematic (dissertation ch7),
not a reproduction of the thesis numbers. It compares fit_variations_stats.txt
with the preserved file at loose tolerances and checks that the ten
dissertation figures exist (their content is not compared). Energy-block order
is only weakly checked here (the listing-order output of 2026-09-29 would
have failed on YMean); tests/macros/test_weighted_graphs.py covers it.

Runs `gxana run xsection --steps tables,weight,qvalue,fitfigs` on the
preserved binned trees (symlinked into $GXANA_OUTPUT/kpkpxim/xsection/
binned_trees, the layout the tables step reads) and flux files ($GXANA_DATA
= the preserved kpkpxim directory). The tables step fits all nine labels
(about 5 min on a laptop); set GXANA_GOLDEN_FITFIGS_OUTPUT to the GXANA_OUTPUT
of a finished run to check it without rerunning.

Tolerance, as in test_xsec_golden.py: XVal/XErr (bin centers and half-widths)
at GXANA_GOLDEN_RTOL (default 1e-5). YMean/StdDev, the per-bin mean and
spread of the eight fit variations' weighted dsigma/dt, depend on every fit
and are compared at GXANA_GOLDEN_FIT_RTOL (default FIT_RTOL below, the
maximum seen on ROOT 6.40 rounded up; median 2e-3 and 7e-2). Besides the
ROOT 6.24 -> 6.32+ minimizer change, the preserved file most likely comes
from an earlier `johnson` run than the published tables, and StdDev, a
spread of nearly equal cross sections, amplifies any shift
(docs/KNOWN_ISSUES.md section 7). The StdDev tolerance (8e-1) therefore guards
only against gross changes. Set GXANA_GOLDEN_FIT_RTOL to tighten.
"""
import os
from pathlib import Path

import pytest

from gxana import config
from gxana.paths import repo_root
from gxana.stages import xsection as xs
from gxana_xsection.compare import compare_tables

pytestmark = pytest.mark.golden

FIT_RTOL = {"YMean": "2e-1", "StdDev": "8e-1"}  # max on ROOT 6.40: 0.121, 0.716

FIGURES = [f"weighted_diffxsec_{n}.pdf" for n in ("AllFits", "SignalFitJohn", "SignalFitVoigt", "SignalFitMC")] + [
    f"fit_examples/{n}.pdf" for n in ("johnsonFit", "voigtFit", "mcFit", "johnsonChebFit", "voigtChebFit", "mcChebFit")]


def _run_chain(golden, output, monkeypatch):
    cfg = config.load_channel("kpkpxim")
    binned = output / "kpkpxim" / "xsection" / "binned_trees"
    binned.mkdir(parents=True)
    for period in cfg["periods"]:
        # the data, MC and thrown trees the tables step reads for this period
        for path in xs._tables_paths(cfg, cfg["xsection"], period, str(output))[1:]:
            src = golden / "binned_trees" / Path(path).name
            if not src.is_file():
                pytest.skip(f"missing golden file: {src}")
            (binned / src.name).symlink_to(src)
    # The macros and apps read GXANA_* from the process environment.
    for var, value in (("GXANA_ROOT", repo_root()), ("GXANA_DATA", golden), ("GXANA_OUTPUT", output)):
        monkeypatch.setenv(var, str(value))
    steps = ["tables", "weight", "qvalue", "fitfigs"]
    assert xs.run_xsection(cfg, steps, environ=dict(os.environ)) == 0


def test_fit_variations_match_legacy(need, build_bin, root_exe, tmp_path, monkeypatch):
    ref = need("reference/xsection/tables/fit_variations_stats.txt")[0]
    reuse = os.environ.get("GXANA_GOLDEN_FITFIGS_OUTPUT")
    output = Path(reuse) if reuse else tmp_path / "output"
    if not reuse:
        _run_chain(need("binned_trees")[0].parent, output, monkeypatch)

    stats = output / "kpkpxim" / "systematics" / "comparisons" / "fit_variations_stats.txt"
    assert stats.is_file(), stats
    rtol = float(os.environ.get("GXANA_GOLDEN_RTOL", "1e-5"))
    column_rtol = {c: float(os.environ.get("GXANA_GOLDEN_FIT_RTOL", t)) for c, t in FIT_RTOL.items()}
    result = compare_tables(stats, ref, rtol=rtol, column_rtol=column_rtol)
    print(f"fit_variations_stats.txt: max rel deviation {result.max_rel:.3g}")
    assert not result.problems, "\n".join(result.problems)

    plots = output / "kpkpxim" / "xsection" / "plots"
    missing = [f for f in FIGURES if not (plots / f).is_file()]
    assert not missing, f"missing figures under {plots}: {missing}"
