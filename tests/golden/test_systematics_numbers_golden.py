"""Golden: the systematics numbers on the preserved thesis tables.

1. spread.py with the legacy accidentals members (acc_weight, hybrid_combo) reproduces
   reference combo_variations_stats.txt (inputs are 6-decimal tables: 1e-5 relative).
2. sfactor.scale_factor on the preserved per-period johnson tables reproduces column 5 (S)
   of the preserved weighted tables, and run_systematic the published Run Combination
   column of syst_diffxsec_table_scale.tex to 3 decimals in every bin.
3. The tex columns mode with the legacy stats files reproduces the published
   diffxsec_table_scale.tex and, with Accidentals and Yield Extraction swapped back,
   syst_diffxsec_table_scale.tex.
"""
import re

import numpy as np
import pytest

from gxana_systematics import sfactor, spread
from gxana_systematics.tables import read_periods, read_weighted
from gxana_xsection import tex_table

pytestmark = pytest.mark.golden
REF = "reference/xsection"


def test_accidentals_spread_reproduces_legacy(need, tmp_path):
    acc, hyb, ref = need(f"{REF}/weighted/acc_weight", f"{REF}/weighted/hybrid_combo",
                         f"{REF}/tables/combo_variations_stats.txt")
    out = tmp_path / "combo.txt"
    spread.write_stats(str(out), spread.spread({"acc_weight": read_weighted(str(acc)),
                                                "hybrid_combo": read_weighted(str(hyb))}))
    got, want = np.loadtxt(out, skiprows=1), np.loadtxt(ref, skiprows=1)
    assert got.shape == want.shape
    np.testing.assert_allclose(got, want, rtol=1e-5, atol=1e-9)


def _published_run_column(tex_path):
    text = tex_path.read_text()
    return np.array([float(m.group(1)) for m in re.finditer(r"\(\d+\.\d+, \d+\.\d+\) & (\d+\.\d+) &", text)])


def test_scale_factor_and_run_column(need):
    per, weighted, syst_tex = need(f"{REF}/johnson", f"{REF}/weighted/johnson", f"{REF}/tables/syst_diffxsec_table_scale.tex")
    run = []
    for block in read_weighted(str(weighted)):
        periods = read_periods(str(per), block.emin, block.emax, 3)
        r = sfactor.scale_factor(np.array([p[:, 1] for p in periods]), np.array([p[:, 3] for p in periods]))
        np.testing.assert_allclose(r.s, block.rows[:, 4], atol=1e-6)
        run += list(sfactor.run_systematic(r.stat_err, r.s))
    published = _published_run_column(syst_tex)
    assert len(published) == len(run) == 56
    np.testing.assert_array_equal(np.round(run, 3), published)


def test_columns_mode_reproduces_published_tables(need, tmp_path):
    per, weighted, tables = need(f"{REF}/johnson", f"{REF}/weighted/johnson", f"{REF}/tables")
    rows = []
    for block in read_weighted(str(weighted)):
        periods = read_periods(str(per), block.emin, block.emax, 3)
        r = sfactor.scale_factor(np.array([p[:, 1] for p in periods]), np.array([p[:, 3] for p in periods]))
        syst = sfactor.run_systematic(r.stat_err, r.s)
        rows += [(p0[0], p0[2], m, e, c, n, s, y) for p0, m, e, c, n, s, y in
                 zip(periods[0], r.mean, r.stat_err, r.chi2, r.n, r.s, syst)]
    run = tmp_path / "sfactor_stats.txt"
    spread.write_stats(str(run), rows, header=sfactor.HEADER)
    wdir = tmp_path / "w"
    wdir.mkdir()
    for f in weighted.glob("weighted_diffxsec_emin_*.txt"):
        (wdir / f.name).write_bytes(f.read_bytes())
    out = tmp_path / "diffxsec_table_scale.tex"
    columns = {"Run Combination": str(run),
               "Accidentals": str(tables / "combo_variations_stats.txt"),
               "Yield Extraction": str(tables / "fit_variations_stats.txt")}
    tex_table.process_files_to_latex(str(wdir), "weighted*.txt", r"\s+", str(out), columns=columns)
    assert out.read_text() == (tables / "diffxsec_table_scale.tex").read_text()
    # published table: Accidentals column holds the fit file and vice versa (legacy swap)
    swapped = tmp_path / "swapped.tex"
    columns["Accidentals"], columns["Yield Extraction"] = columns["Yield Extraction"], columns["Accidentals"]
    tex_table.process_files_to_latex(str(wdir), "weighted*.txt", r"\s+", str(swapped), columns=columns)
    assert (tmp_path / "syst_swapped.tex").read_text() == (tables / "syst_diffxsec_table_scale.tex").read_text()
