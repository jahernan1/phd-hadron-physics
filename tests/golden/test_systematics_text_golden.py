"""Golden: the migrated systematics weighting script reproduces its legacy output.

Runs analyses/kpkpxim/systematics/GetWeightedXsecFile.py itself (a plain
subprocess -- it is a verbatim copy, not a library entry point) against the
staged xsection_data and compares its weighted_data output to the staged
legacy reference with gxana_xsection.compare.compare_dirs.

The migrated __main__ also runs one nominal pass out of
"../xsection/ml_fits/data/gen_amp_V2_2D_ac/hybrid_combo" (relative to the
script's cwd), which is not staged for this golden (it is a different
directory layout from anything reference/systematics or reference/xsection
stages, and the nominal weighted cross section is already covered by
test_python_golden.py's test_weighted_average_matches_legacy). The script
therefore exits non-zero at that last step, after every variation output
below has already been written; the test only requires those files.
"""
import os
import subprocess
import sys
from pathlib import Path

import pytest

from gxana_xsection.compare import compare_dirs

pytestmark = pytest.mark.golden

REF = "reference/systematics"
SCRIPT = Path(__file__).resolve().parents[2] / "analyses/kpkpxim/systematics/GetWeightedXsecFile.py"

# GetWeightedXsecFile.py __main__: 18 variation cuts x (1 totxsec + 8 diffxsec
# energy-bin patterns) = 162 weighted_data files.
N_VARIATIONS = 18
N_ENERGY_BINS = 8


def test_weighted_average_matches_legacy_systematics(need, tmp_path):
    src, ref = need(f"{REF}/xsection_data", f"{REF}/weighted_data")

    # Lay out the run directory the way the script's __main__ expects:
    # ./xsection_data (input) next to the script, ./weighted_data (output)
    # created by the script itself via os.makedirs.
    run_dir = tmp_path / "systematics"
    xsection_data = run_dir / "xsection_data"
    xsection_data.mkdir(parents=True)
    for f in src.glob("*.txt"):
        (xsection_data / f.name).write_bytes(f.read_bytes())

    env = dict(os.environ)
    for var in ("GXANA_DATA", "GXANA_OUTPUT", "GXANA_SCRATCH", "GXANA_EXTERNALS", "GXANA_ANALYSIS_DATA"):
        env[var] = str(tmp_path / var.lower())

    proc = subprocess.run([sys.executable, str(SCRIPT)], cwd=run_dir, env=env,
                          capture_output=True, text=True)

    out_dir = run_dir / "weighted_data"
    produced = sorted(p.name for p in out_dir.glob("*.txt")) if out_dir.is_dir() else []
    # The only expected failure is the un-staged nominal pass at the very
    # end of __main__, after every variation file has been written.
    assert produced, f"script produced no weighted_data files:\n{proc.stdout}\n{proc.stderr}"

    rtol = float(os.environ.get("GXANA_GOLDEN_RTOL", "1e-5"))
    report = compare_dirs(out_dir, ref, rtol=rtol, only_new=True)
    assert report.ok, report.summary()
    assert len(report.results) == N_VARIATIONS * (1 + N_ENERGY_BINS)
