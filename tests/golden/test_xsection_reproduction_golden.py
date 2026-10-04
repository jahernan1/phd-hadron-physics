"""Golden: the documented preserved-data route reproduces the dissertation tables (label johnson).

The route of the top README ("Reproducing the thesis", route 1): `gxana data stage`
(thesis binned trees, Q-factor and MC flat trees), `gxana run xsection --steps
tables,weight,integrate,components`, `gxana run systematics --study run --steps spread`
(the scale-factor Run Combination column), then `gxana run xsection --steps tex` with
the preserved fit_variations_stats.txt and combo_variations_stats.txt as the Yield
Extraction and Accidentals columns, the inputs the dissertation tables were made from.
The rerun spreads are compared separately (test_systematics_chain_golden.py) because
they move the systematic columns (docs/KNOWN_ISSUES.md section 3).

Reference: reference/xsection/{johnson,weighted/johnson,components,tables}; the
*_scale.tex tables there equal the published dissertation tables number for number
(formatting differs). Tolerances: fit-yield columns GXANA_GOLDEN_FIT_RTOL (default 4e-2,
johnson, as test_xsec_golden.py), others GXANA_GOLDEN_RTOL (1e-5); the weighted S
column (sqrt(chi2/ndf) of three periods, amplifies fit shifts) 2e-1, measured 0.1645 on
ROOT 6.40 with the TMinuit/Migrad pin; printed LaTeX values 1e-2 relative or 1e-3
absolute, measured 0.0083 (dsigma/dt table) and 0.0080 (systematics table).
Measured on ROOT 6.40 after the Minuit pin: johnson period fit columns 3.84e-2 (inside
the 4e-2 golden tolerance), weighted dsigma/dt and delta_y 2.72e-2 and 3.32e-3 at most,
deterministic columns and totals 2.6e-3 at most.
The published tables swap the Accidentals and Yield Extraction headings
(docs/KNOWN_ISSUES.md), so those two columns are compared crosswise.
"""
import os
import re
import shutil

import pytest
from golden_data import PERIOD_LABELS

from gxana.cli import main as gxana
from gxana.paths import repo_root
from gxana_xsection.compare import compare_dirs

pytestmark = pytest.mark.golden

FIT_COLUMNS = ("data_yield", "yield_err", "sigma", "dsigmadt", "Yerr")
S_RTOL = 2e-1
TEX_RTOL, TEX_ATOL = 1e-2, 1e-3


def _rtol():
    return float(os.environ.get("GXANA_GOLDEN_RTOL", "1e-5")), float(os.environ.get("GXANA_GOLDEN_FIT_RTOL", "4e-2"))


@pytest.fixture(scope="module")
def produced(golden, build_bin, root_exe, tmp_path_factory):
    work = tmp_path_factory.mktemp("repro")
    env = {"GXANA_ROOT": str(repo_root()), "GXANA_DATA": str(work / "data"),
           "GXANA_OUTPUT": str(work / "output"), "GXANA_SCRATCH": str(work / "scratch"),
           "GXANA_ANALYSIS_DATA": str(golden.parent), "ROOT_MAX_THREADS": "1"}
    mp = pytest.MonkeyPatch()
    try:
        for key, value in env.items():
            mp.setenv(key, value)
        assert gxana(["data", "stage", "--channel", "kpkpxim"]) == 0
        assert gxana(["run", "xsection", "--channel", "kpkpxim", "--steps", "tables,weight,integrate,components"]) == 0
        assert gxana(["run", "systematics", "--channel", "kpkpxim", "--study", "run", "--steps", "spread"]) == 0
        syst = work / "output" / "kpkpxim" / "systematics"
        for sub, name in (("fit", "fit_variations_stats.txt"), ("accidentals", "combo_variations_stats.txt")):
            (syst / sub).mkdir(parents=True, exist_ok=True)
            shutil.copy2(golden / "reference/xsection/tables" / name, syst / sub / name)
        assert gxana(["run", "xsection", "--channel", "kpkpxim", "--steps", "tex"]) == 0
    finally:
        mp.undo()
    return work / "output" / "kpkpxim" / "xsection"


def _assert(report, count):
    print(report.summary())
    assert report.ok, report.summary()
    assert len(report.results) == count


def test_period_tables(produced, golden):
    rtol, fit = _rtol()
    ref = golden / "reference/xsection/johnson"
    for pattern, count in (("diff*.txt", 48), ("tot*.txt", 6)):
        _assert(compare_dirs(produced / "data/johnson", ref, pattern, rtol=rtol,
                             column_rtol={c: fit for c in FIT_COLUMNS}), count)


def test_weighted_tables(produced, golden):
    rtol, fit = _rtol()
    columns = {"d\\sigma/dt": fit, "\\delta_y": fit, "S": S_RTOL}
    ref = golden / "reference/xsection/weighted/johnson"
    for pattern, count in (("weighted_*.txt", 8), ("totxsec_weighted_output.txt", 1)):
        _assert(compare_dirs(produced / "weighted_data/johnson", ref, pattern, rtol=rtol,
                             column_rtol=columns, only_new=True), count)


@pytest.mark.parametrize("label", [p for p, _ in PERIOD_LABELS])
def test_components(produced, golden, label):
    rtol, fit = _rtol()
    report = compare_dirs(produced / "components" / label / "johnson",
                          golden / "reference/xsection/components" / label / "johnson",
                          rtol=rtol, column_rtol={c: fit for c in FIT_COLUMNS})
    print(report.summary())
    assert report.ok, report.summary()


def _tex_rows(path):
    """The last three numbers of each data row of a dissertation table (56 rows)."""
    rows = []
    for line in path.read_text().splitlines():
        line = re.sub(r"\\multirow\{\d+\}\{\*\}", "", line)
        if "&" in line and "\\\\" in line and not re.search(r"[A-Za-z]{3,}", line):
            rows.append([float(x) for x in re.findall(r"-?\d+\.\d+|-?\d+", line.replace("${}^2$", ""))][-3:])
    assert len(rows) == 56, path
    return rows


def _close(a, b):
    return a == pytest.approx(b, rel=TEX_RTOL, abs=TEX_ATOL)


def test_dissertation_table(produced, golden):
    new = _tex_rows(produced / "tables/diffxsec_table_scale.tex")
    ref = _tex_rows(golden / "reference/xsection/tables/diffxsec_table_scale.tex")
    bad = [(i, n, r) for i, (n, r) in enumerate(zip(new, ref)) if not all(map(_close, n, r))]
    assert not bad, bad


def test_dissertation_systematics_table(produced, golden):
    new = _tex_rows(produced / "tables/syst_diffxsec_table_scale.tex")
    ref = _tex_rows(golden / "reference/xsection/tables/syst_diffxsec_table_scale.tex")
    # columns: Run Combination, Accidentals, Yield Extraction; published headings swapped (KNOWN_ISSUES.md)
    bad = [(i, n, r) for i, (n, r) in enumerate(zip(new, ref))
           if not (_close(n[0], r[0]) and _close(n[1], r[2]) and _close(n[2], r[1]))]
    assert not bad, bad
