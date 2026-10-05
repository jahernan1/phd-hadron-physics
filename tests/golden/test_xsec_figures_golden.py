"""Golden: `gxana run xsection --steps figures` redraws the three cross-section figures of
dissertation chapter 6 from the preserved tables, and the numbers it draws are the published ones.

The GXANA_OUTPUT of the run holds the preserved johnson tables where the stages write them
(xsection/data; xsection/weighted_data with weighted_diffxsec_* and totxsec only), and the step
runs with `--systematics published` (xsection.published_systematics): the preserved
fit_variations_stats.txt and combo_variations_stats.txt, then the scale-factor run systematic
(the legacy order), and the preserved tables of the JohnsonMCShape study label hybrid_combo for
the total-cross-section figure. Pixels are not compared (fonts and ROOT versions move them); the graphs
the macros write beside the PDFs are (tests/macros/dump_graphs.C).

What each graph is compared to: the per-period and total graphs, to the staged preserved
tables they are drawn from (the macros must not change a number); the weighted figure, to the
published diffxsec_table_scale.tex, which prints 3 decimals (TABLE_ATOL, half a unit of the
last printed digit) and the -t edges to 2 decimals (EDGE_ATOL = 6e-3: the 5e-3 rounding of a
2-decimal edge plus 1e-3 of slack for the center/half-width arithmetic); and the total-cross-section
fit, to the chi2_nu = 0.74 the dissertation figure prints.

The graph dump (tests/macros/dump_graphs.C) writes %.9g, about 9 significant digits, so
numbers that must be identical to the table they were read from are compared with rtol=1e-8,
above that print precision and far below any physical change.

The total-cross-section fit is checked against chi2/ndf = 24.6385/33. Measured with the
TMinuit/Migrad pin of rootlogon.C: chi2 = 24.6384989, ndf = 33, a difference of 1.1e-6 from
the 24.6385 measured before the pin. The 1e-3 tolerance on chi2/ndf is slack for ROOT version
drift, not for the fit: the measured agreement is 1.1e-6.
"""
import os
import re
import shutil
import subprocess
from pathlib import Path

import numpy as np
import pytest

from gxana import config
from gxana.paths import repo_root
from gxana.stages import xsection as xs
from golden_data import ENERGY_BIN_LOWS, PERIOD_TREES

pytestmark = pytest.mark.golden

REF = "reference/xsection"
PERIODS = ("2017-01", "2018-01", "2018-08")
BINS = list(zip(ENERGY_BIN_LOWS, ENERGY_BIN_LOWS[1:] + ("11.40",)))
TABLE_ATOL = 5e-4 + 1e-9   # the published table prints 3 decimals
EDGE_ATOL = 6e-3           # and the -t edges 2
FIGURES = ("diffxsec_runs_johnson.pdf", "diffxsec_phase1_systematics_johnson.pdf", "totxsec_clas_gluex_Phase1.pdf")


def _published_rows(tex: Path) -> np.ndarray:
    """(t_lo, t_hi, dsigma/dt, stat, syst) of every row of diffxsec_table_scale.tex, in table order."""
    pat = re.compile(r"& \(([\d.]+), ([\d.]+)\) & ([\d.]+) & ([\d.]+) & ([\d.]+)")
    return np.array([[float(v) for v in m.groups()] for m in map(pat.search, tex.read_text().splitlines()) if m])


def _dump(path: Path, env) -> list:
    proc = subprocess.run(["root", "-l", "-b", "-q", f'{repo_root() / "tests/macros/dump_graphs.C"}("{path}")'],
                          env=env, capture_output=True, text=True, timeout=300)
    assert proc.returncode == 0, proc.stderr[-2000:]
    return [line.split("\t") for line in proc.stdout.splitlines() if line.startswith(("G\t", "F\t"))]


def _points(rows, key=None) -> np.ndarray:
    return np.array([[float(v) for v in r[4:8]] for r in rows if r[0] == "G" and (key is None or r[1] == key)])


def _table(path: Path) -> np.ndarray:
    return np.loadtxt(path, skiprows=1, usecols=(0, 1, 2, 3), ndmin=2)


@pytest.fixture(scope="module")
def run(golden, need, build_bin, root_exe, tmp_path_factory):
    johnson, weighted, hybrid, whybrid, tables = need(
        f"{REF}/johnson", f"{REF}/weighted/johnson", f"{REF}/hybrid_combo", f"{REF}/weighted/hybrid_combo",
        f"{REF}/tables")
    out = tmp_path_factory.mktemp("figures_output")
    xsec = out / "kpkpxim" / "xsection"
    (xsec / "data").mkdir(parents=True)
    (xsec / "data" / "johnson").symlink_to(johnson)
    wj = xsec / "weighted_data" / "johnson"
    wj.mkdir(parents=True)
    for path in sorted(weighted.glob("weighted_diffxsec_*.txt")) + [weighted / "totxsec_weighted_output.txt"]:
        shutil.copy(path, wj)
    cfg = xs.with_systematics(config.load_channel("kpkpxim"), "published")
    with pytest.MonkeyPatch.context() as mp:
        for var, value in (("GXANA_ROOT", repo_root()), ("GXANA_OUTPUT", out),
                           ("GXANA_ANALYSIS_DATA", golden.parent), ("ROOT_MAX_THREADS", "1")):
            mp.setenv(var, str(value))
        env = dict(os.environ)
        assert xs.run_xsection(cfg, ["figures"], environ=env) == 0
    assert not (out / "kpkpxim" / "systematics").exists()   # needs no gxana run systematics
    return {"fig": xsec / "figures", "wj": wj, "weighted": weighted, "johnson": johnson, "hybrid": hybrid,
            "whybrid": whybrid, "tables": tables, "env": env}


def test_figures_written(run):
    for name in FIGURES:
        assert (run["fig"] / name).stat().st_size > 0, name


def test_systematic_tables_equal_the_legacy_ones(run):
    written = sorted(p.name for p in run["wj"].glob("syst_weighted_diffxsec_*.txt"))
    assert len(written) == 8
    assert written == sorted(p.name for p in run["weighted"].glob("syst_weighted_diffxsec_*.txt"))
    for name in written:
        assert (run["wj"] / name).read_text() == (run["weighted"] / name).read_text(), name


def test_weighted_figure_draws_the_published_table(run):
    published = _published_rows(run["tables"] / "diffxsec_table_scale.tex")
    assert published.shape == (56, 5)
    syst_rows = _dump(run["fig"] / "SystWeightedDiffXSecTGraphs_johnson.root", run["env"])
    assert [r[2] for r in syst_rows if r[3] == "0"] == [f"#bf{{E_{{#gamma}} (GeV): ({lo}, {hi})}}" for lo, hi in BINS]
    s = _points(syst_rows)
    w = _points(_dump(run["fig"] / "WeightedDiffXSecTGraphs_johnson.root", run["env"]))
    np.testing.assert_allclose(s[:, 0] - s[:, 2], published[:, 0], atol=EDGE_ATOL)
    np.testing.assert_allclose(s[:, 0] + s[:, 2], published[:, 1], atol=EDGE_ATOL)
    np.testing.assert_allclose(s[:, 1], published[:, 2], atol=TABLE_ATOL)   # points
    np.testing.assert_allclose(s[:, 3], published[:, 4], atol=TABLE_ATOL)   # band = total systematic
    np.testing.assert_allclose(w[:, 1], published[:, 2], atol=TABLE_ATOL)
    np.testing.assert_allclose(w[:, 3], published[:, 3], atol=TABLE_ATOL)   # error bars = statistical


def test_run_period_figure_draws_the_preserved_tables(run):
    for period, stem in zip(PERIODS, PERIOD_TREES):
        rows = _dump(run["fig"] / f"DiffXSecTGraphs_{period}_johnson.root", run["env"])
        want = np.vstack([_table(run["johnson"] / f"diffxsec_flatTree_{stem}_emin_{lo}_emax_{hi}.txt")
                          for lo, hi in BINS])
        np.testing.assert_allclose(_points(rows), want, rtol=1e-8)


def test_total_figure_draws_the_study_tables_and_the_printed_fit(run):
    rows = _dump(run["fig"] / "totxsec_clas_gluex_Phase1.root", run["env"])
    for key, stem in zip(("sp17", "sp18", "fa18"), PERIOD_TREES):
        np.testing.assert_allclose(_points(rows, key), _table(run["hybrid"] / f"totxsec_flatTree_{stem}.txt"),
                                   rtol=1e-8)
    np.testing.assert_allclose(_points(rows, "weighted"), _table(run["whybrid"] / "totxsec_weighted_output.txt"),
                               rtol=1e-8)
    (fit,) = [r for r in rows if r[0] == "F"]
    chi2ndf = float(fit[2]) / int(fit[3])
    assert f"{chi2ndf:.6f}"[:4] == "0.74"       # the figure prints (chi2_nu = 0.74)
    assert abs(chi2ndf - 24.6385 / 33) < 1e-3   # ROOT 6.40, TMinuit/Migrad pin: chi2 = 24.6384989 (24.6385 before the pin)
