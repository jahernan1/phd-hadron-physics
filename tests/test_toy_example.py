"""The toy walkthrough of examples/toy/README.md runs end to end on synthetic data.

make_toy.py writes the inputs into tmp_path, then `gxana run xsection --channel toy`
runs bin, tables, weight, integrate, components and then tex, figures, and
check_toy.py compares the weighted cross sections with the injected truth. No
preserved data is used (GXANA_ANALYSIS_DATA points at a directory that does not exist). Skips
without ROOT or the built apps. Under a minute."""
import os
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
TOY = ROOT / "examples" / "toy"

pytestmark = [
    pytest.mark.skipif(shutil.which("root") is None, reason="needs ROOT (`root` on PATH)"),
    pytest.mark.skipif(not (ROOT / "build" / "bin" / "gxana_xsec_tables").is_file(),
                       reason="needs the built apps (cmake --build build)"),
]


def _run(argv, env):
    proc = subprocess.run(argv, env=env, capture_output=True, text=True, cwd=ROOT)
    assert proc.returncode == 0, f"{argv[:4]} failed:\n{proc.stdout[-3000:]}\n{proc.stderr[-3000:]}"
    return proc.stdout


def test_toy_walkthrough(tmp_path):
    env = {**os.environ, "GXANA_ROOT": str(tmp_path / "gxana_root"), "GXANA_DATA": str(tmp_path / "data"),
           "GXANA_OUTPUT": str(tmp_path / "output"), "GXANA_ANALYSIS_DATA": str(tmp_path / "no_analysis_data")}
    _run([sys.executable, str(TOY / "make_toy.py"), str(tmp_path)], env)
    gxana = [sys.executable, "-m", "gxana.cli"]
    _run(gxana + ["run", "xsection", "--channel", "toy"], env)
    _run(gxana + ["run", "xsection", "--channel", "toy", "--steps", "tex,figures"], env)

    xsec = tmp_path / "output" / "toy" / "xsection"
    weighted = xsec / "weighted_data" / "toy"
    assert len(list(weighted.glob("weighted_diffxsec_emin_*.txt"))) == 2
    for name in ("totxsec_weighted_output.txt", "intxsec_weighted_output.txt"):
        assert (weighted / name).is_file(), name
    assert len(list((xsec / "components" / "sp17" / "toy").glob("accept_*.txt"))) == 3
    assert (xsec / "tables" / "diffxsec_table.tex").is_file()
    for name in ("toy_dsigma_dt.pdf", "toy_sigma_total.pdf"):
        assert (xsec / "figures" / name).stat().st_size > 0, name

    report = _run([sys.executable, str(TOY / "check_toy.py")], env)
    assert "10/10 within" in report, report
