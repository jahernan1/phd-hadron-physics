"""Golden: gxana_xsection reproduces the legacy weighted, component, Q-value and LaTeX tables."""
import shutil

import pytest
from golden_data import ENERGY_BIN_LOWS, PERIOD_LABELS

from gxana_xsection import components, qvalue_rescale, tex_table, weighted_average
from gxana_xsection.compare import compare_dirs

pytestmark = pytest.mark.golden

REF = "reference/xsection"

# JohnsonMCShape study (legacy MakeXSecFiles.C) per combo-selection weight;
# johnson: the dissertation fit (Johnson + Chebychev 2, weight hybrid_combo).
FIT_LABELS = ("hybrid_combo", "best_combo", "acc_weight", "johnson")


@pytest.mark.parametrize("fit_label", FIT_LABELS)
def test_weighted_average_matches_legacy(need, tmp_path, fit_label):
    src, ref = need(f"{REF}/{fit_label}", f"{REF}/weighted/{fit_label}")
    weighted_average.weight_files(str(src), str(tmp_path), pattern="totxsec*.txt")
    for low in ENERGY_BIN_LOWS:
        weighted_average.weight_files(str(src), str(tmp_path), pattern=f"diffxsec*_emin_{low}*.txt")
    # Legacy output is printed with %.6f: allow one unit in the last digit.
    report = compare_dirs(tmp_path, ref, rtol=0.0, atol=1.5e-6, only_new=True)
    assert report.ok, report.summary()
    assert len(report.results) == 9


@pytest.mark.parametrize("fit_label", FIT_LABELS)
@pytest.mark.parametrize("label,period", PERIOD_LABELS)
def test_components_match_legacy(need, tmp_path, label, period, fit_label):
    src, ref = need(f"{REF}/{fit_label}", f"{REF}/components/{label}/{fit_label}")
    components.split_files(str(src), str(tmp_path), pattern=f"totout*{period}*.txt", anchor="kpkpxim")
    components.split_files(str(src), str(tmp_path), pattern=f"diffout*{period}*.txt", anchor="kpkpxim")
    report = compare_dirs(tmp_path, ref, rtol=1e-12, only_new=True)
    assert report.ok, report.summary()
    assert len(report.results) == 54


def test_johnson_mcshape_study_latex_tables_match_legacy(need, tmp_path):
    """Legacy MakeXsecTexTable.py on weighted/hybrid_combo -> the JohnsonMCShape
    study (runsyst) tables, not the dissertation tables."""
    weighted, tables = need(f"{REF}/weighted/hybrid_combo", f"{REF}/tables")
    # tex_table writes processed_* beside its input: work on a copy.
    src = tmp_path / "in"
    src.mkdir()
    for path in weighted.glob("weighted_*.txt"):
        shutil.copy(path, src)
    out = tmp_path / "diffxsec_table_runsyst.tex"
    tex_table.process_files_to_latex(
        str(src), "weighted*.txt", r"\s+", str(out),
        additional_files=[str(tables / "fit_variations_stats.txt"), str(tables / "combo_variations_stats.txt")])
    assert out.read_text() == (tables / "diffxsec_table_runsyst.tex").read_text()
    syst = tmp_path / "syst_diffxsec_table_runsyst.tex"
    assert syst.read_text() == (tables / "syst_diffxsec_table_runsyst.tex").read_text()
    for path in src.glob("processed_*.txt"):
        assert path.read_text() == (weighted / path.name).read_text(), path.name


def test_dissertation_latex_tables_match_legacy(need, tmp_path):
    """Legacy MakeXsecTexTableScale.py on weighted/johnson -> the dissertation
    tables (scale-factor run systematic, fit- and combo-variation spreads)."""
    weighted, tables = need(f"{REF}/weighted/johnson", f"{REF}/tables")
    # tex_table writes syst_weighted_* beside its input: work on a copy.
    src = tmp_path / "in"
    src.mkdir()
    for path in weighted.glob("weighted_*.txt"):
        shutil.copy(path, src)
    assert len(list(src.glob("weighted_*.txt"))) == 8
    out = tmp_path / "diffxsec_table_scale.tex"
    tex_table.process_files_to_latex(
        str(src), "weighted*.txt", r"\s+", str(out),
        additional_files=[str(tables / "fit_variations_stats.txt"), str(tables / "combo_variations_stats.txt")],
        systematic_source="scale_factor")
    assert out.read_text() == (tables / "diffxsec_table_scale.tex").read_text()
    syst = tmp_path / "syst_diffxsec_table_scale.tex"
    assert syst.read_text() == (tables / "syst_diffxsec_table_scale.tex").read_text()
    written = sorted(p.name for p in src.glob("syst_weighted_*.txt"))
    assert written == sorted(p.name for p in weighted.glob("syst_weighted_*.txt"))
    assert len(written) == 8
    for name in written:
        assert (src / name).read_text() == (weighted / name).read_text(), name


@pytest.mark.xfail(
    strict=True,
    reason=(
        "The preserved reference/xsection/qvalues was made from an earlier hybrid_combo "
        "run than the preserved reference/xsection/hybrid_combo: the legacy "
        "MakeQValXSecFile.py rerun on the preserved hybrid_combo is identical to "
        "this port except in two 2017-01 rows whose preserved hybrid_combo fit failed "
        "(data_yield 0), and it differs from qvalues by up to 0.6 % in dsigmadt and Yerr "
        "and in those two rows. Not a porting bug (docs/PORT_NOTES.md section 4)."
    ),
)
def test_qvalue_rescale_matches_legacy(need, tmp_path):
    src, ref = need(f"{REF}/hybrid_combo", f"{REF}/qvalues")
    qvalue_rescale.process_files_in_directory(str(src), "diffout*.txt", "diffxsec*.txt", str(tmp_path),
                                              "data_yield", "qval_yield")
    report = compare_dirs(tmp_path, ref, rtol=1e-12, only_new=True)
    assert report.ok, report.summary()
    assert len(report.results) == 24
