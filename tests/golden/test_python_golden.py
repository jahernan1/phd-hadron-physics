"""Golden: gxana_xsection reproduces the legacy weighted, component and Q-value tables."""
import pytest
from golden_data import ENERGY_BIN_LOWS, PERIOD_LABELS

from gxana_xsection import components, qvalue_rescale, weighted_average
from gxana_xsection.compare import compare_dirs

pytestmark = pytest.mark.golden

REF = "reference/xsection"


def test_weighted_average_matches_legacy(need, tmp_path):
    src, ref = need(f"{REF}/johnson", f"{REF}/weighted/johnson")
    weighted_average.weight_files(str(src), str(tmp_path), pattern="totxsec*.txt")
    for low in ENERGY_BIN_LOWS:
        weighted_average.weight_files(str(src), str(tmp_path), pattern=f"diffxsec*_emin_{low}*.txt")
    # Legacy output is printed with %.6f: allow one unit in the last digit.
    report = compare_dirs(tmp_path, ref, rtol=0.0, atol=1.5e-6, only_new=True)
    assert report.ok, report.summary()
    assert len(report.results) == 9


@pytest.mark.parametrize("label,period", PERIOD_LABELS)
def test_components_match_legacy(need, tmp_path, label, period):
    src, ref = need(f"{REF}/johnson", f"{REF}/components/{label}/johnson")
    components.split_files(str(src), str(tmp_path), pattern=f"totout*{period}*.txt")
    components.split_files(str(src), str(tmp_path), pattern=f"diffout*{period}*.txt")
    report = compare_dirs(tmp_path, ref, rtol=1e-12, only_new=True)
    assert report.ok, report.summary()
    assert len(report.results) == 54


@pytest.mark.xfail(
    strict=True,
    reason=(
        "The staged reference/xsection/qvalues predates the staged hybrid_combo "
        "inputs (qvalues dated Jan/Feb 2025, hybrid_combo dated Mar 1 2025); the "
        "legacy MakeQValXSecFile.py rerun on the staged hybrid_combo is byte-identical "
        "to this port, and the reference Yerr column (copied unchanged from its input) "
        "differs. Not a porting bug. Author to decide whether to regenerate."
    ),
)
def test_qvalue_rescale_matches_legacy(need, tmp_path):
    src, ref = need(f"{REF}/hybrid_combo", f"{REF}/qvalues")
    qvalue_rescale.process_files_in_directory(str(src), "diffout*.txt", "diffxsec*.txt", str(tmp_path),
                                              "data_yield", "qval_yield")
    report = compare_dirs(tmp_path, ref, rtol=1e-12, only_new=True)
    assert report.ok, report.summary()
    assert len(report.results) == 24
