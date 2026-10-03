"""Golden: the fit-model systematic end to end (replaces test_fit_variations_golden.py).

Runs `gxana run xsection --steps tables,weight` (nominal johnson) and `gxana run
systematics --study fit --steps fit,qvalue,weight,spread` on the preserved binned trees
and flux ($GXANA_DATA = the preserved kpkpxim directory), then compares
systematics/fit/fit_variations_stats.txt with the preserved file at the loose
tolerances of docs/KNOWN_ISSUES.md section 6 (the preserved file most likely comes from an
earlier johnson run; ROOT 6.24 -> 6.32+ minimizer change; a spread of nearly equal values
amplifies any shift). Set GXANA_GOLDEN_SYST_OUTPUT to the GXANA_OUTPUT of a finished run
to check it without rerunning (the fits take about 7 min on a laptop).
"""
import os
from pathlib import Path

import pytest

from gxana import config
from gxana.paths import repo_root
from gxana.stages import xsection as xs
from gxana_systematics import stage as st
from gxana_xsection.compare import compare_tables

pytestmark = pytest.mark.golden

FIT_RTOL = {"YMean": "2e-1", "StdDev": "8e-1"}  # max on ROOT 6.40 (2026-09-30): 0.121, 0.716
FIGURES = ["fit_examples/" + n + ".pdf" for n in ("johnsonFit", "voigtFit", "mcFit", "johnsonChebFit",
                                                    "voigtChebFit", "mcChebFit")]


def _run_chain(golden, output, monkeypatch):
    cfg = config.load_channel("kpkpxim")
    binned = output / "kpkpxim" / "xsection" / "binned_trees"
    binned.mkdir(parents=True)
    for period in cfg["periods"]:
        for path in xs.tables_paths(cfg, cfg["xsection"], period, str(output))[1:]:
            src = golden / "binned_trees" / Path(path).name
            if not src.is_file():
                pytest.skip(f"missing golden file: {src}")
            (binned / src.name).symlink_to(src)
    for var, value in (("GXANA_ROOT", repo_root()), ("GXANA_DATA", golden), ("GXANA_OUTPUT", output)):
        monkeypatch.setenv(var, str(value))
    env = dict(os.environ)
    assert xs.run_xsection(cfg, ["tables", "weight"], environ=env) == 0
    assert st.run_systematics(cfg, ["fit", "qvalue", "weight", "spread"], environ=env, study_names=["fit"]) == 0


def test_fit_spread_end_to_end(need, build_bin, root_exe, tmp_path, monkeypatch):
    ref = need("reference/xsection/tables/fit_variations_stats.txt")[0]
    reuse = os.environ.get("GXANA_GOLDEN_SYST_OUTPUT")
    output = Path(reuse) if reuse else tmp_path / "output"
    if not reuse:
        _run_chain(need("binned_trees")[0].parent, output, monkeypatch)
    stats = output / "kpkpxim" / "systematics" / "fit" / "fit_variations_stats.txt"
    assert stats.is_file(), stats
    column_rtol = {c: float(os.environ.get("GXANA_GOLDEN_FIT_RTOL", t)) for c, t in FIT_RTOL.items()}
    result = compare_tables(stats, ref, rtol=float(os.environ.get("GXANA_GOLDEN_RTOL", "1e-5")),
                            column_rtol=column_rtol)
    print(f"fit_variations_stats.txt: max rel deviation {result.max_rel:.3g}")
    assert not result.problems, "\n".join(result.problems)
    plots = output / "kpkpxim" / "systematics" / "fit" / "plots"
    missing = [f for f in FIGURES if not (plots / f).is_file()]
    assert not missing, f"missing figures under {plots}: {missing}"
