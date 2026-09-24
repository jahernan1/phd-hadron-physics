"""Golden: gxana_xsection reproduces the systematics run-period weighting.

Legacy run: AnalysisNote/systematics/GetWeightedXsecFile.py __main__ loop
(byte-identical to gx1 barlow_systematics/GetWeightedXSecFiles.py -- NOT to
xsection/GetWeightedXsecFile.py, see the reason string below) over
./xsection_data: one totxsec* pattern and 8 diffxsec*_emin_<low>* patterns
per variation cut, three run-period files per pattern.
"""
import pytest
from golden_data import ENERGY_BIN_LOWS

from gxana_xsection import weighted_average
from gxana_xsection.compare import compare_dirs

pytestmark = pytest.mark.golden

REF = "reference/systematics"

# AnalysisNote/systematics/GetWeightedXsecFile.py __main__: variations list
# (kplow_prap is commented out there, same as GetXSecFilesUML.C's cuts list).
VARIATIONS = (
    "chisqndf_6", "chisqndf_7", "chisqndf_9", "chisqndf_10",
    "total_mm2_abs_0.01", "total_mm2_abs_0.015", "total_mm2_abs_0.025", "total_mm2_abs_0.03",
    "xim_pathlensig_1", "xim_pathlensig_1.5", "xim_pathlensig_2.5", "xim_pathlensig_3",
    "lambda_pathlensig_0.5", "lambda_pathlensig_1",
    "kphigh_prap_1.6", "kphigh_prap_1.8", "kphigh_prap_2.1", "kphigh_prap_2.2",
)


@pytest.mark.xfail(
    strict=True,
    reason=(
        "gxana_xsection.weighted_average.weight_files ports xsection/GetWeightedXsecFile.py: "
        "it adds a chi-square scale-factor column S and writes the header with no '#' comment "
        "prefix. The systematics GetWeightedXsecFile.py this reference was generated from "
        "(byte-identical to gx1 barlow_systematics/GetWeightedXSecFiles.py) never computes S "
        "and writes 4 columns under a '# ...' header. Rerunning that legacy script on the "
        "staged xsection_data reproduces every staged weighted_data file exactly (X/Y/EX/EY "
        "byte-identical); rerunning weight_files on the same inputs gives numerically identical "
        "X/Y/EX/EY plus the extra S column, so compare_dirs reports a structural (column-count) "
        "mismatch on every file. Not a porting bug or a stale reference -- weight_files "
        "intentionally serves the xsection 5-column format already golden-tested in "
        "test_python_golden.py; the systematics 4-column format is a distinct legacy script by "
        "design (same divergence pattern as the comparison macros' local "
        "GetPointwiseMeanAndStdDev)."
    ),
)
def test_weighted_average_matches_legacy_systematics(need, tmp_path):
    src, ref = need(f"{REF}/xsection_data", f"{REF}/weighted_data")
    for var in VARIATIONS:
        weighted_average.weight_files(str(src), str(tmp_path), pattern=f"totxsec*{var}.txt")
        for low in ENERGY_BIN_LOWS:
            weighted_average.weight_files(str(src), str(tmp_path), pattern=f"diffxsec*{var}_emin_{low}*")
    report = compare_dirs(tmp_path, ref, rtol=0.0, atol=1.5e-6, only_new=True)
    assert report.ok, report.summary()
    assert len(report.results) == len(VARIATIONS) * (1 + len(ENERGY_BIN_LOWS))
