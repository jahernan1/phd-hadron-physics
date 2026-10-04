"""The split mass / lifetime / spin measurements reproduce the original GetXimProperties.C and
PlotGlueXSpin.C fit results on the preserved thesis trees with the minimiser pinned to TMinuit (reference:
tests/golden/data/measurements_reference.txt, whose header says how it was recorded).

Both sides run single-threaded (implicit MT off): the multithreaded histogram fill is not
bit-reproducible and the mass fit amplifies that noise, so only single-threaded runs can be pinned
tightly. The plain data tree is not preserved; the post-Q-factor tree stands in for it
(docs/PORT_NOTES.md, section 9)."""
import os
import shutil
import subprocess
from pathlib import Path

import pytest
from golden_data import assert_fitresults_equal, measurements_farm, parse_fitresults

from gxana.paths import repo_root

pytestmark = [pytest.mark.golden, pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")]

REFERENCE = Path(__file__).parent / "data" / "measurements_reference.txt"
MACROS = "analyses/kpkpxim/measurements"
# Every entry takes n_threads; 0 leaves implicit MT off, as in the reference run.
SEQUENCE = ("mass/PrepMass.C", "mass/FitMass.C", "lifetime/PrepLifetime.C", "lifetime/FitLifetime.C",
            "spin/PrepSpinData.C", "spin/PlotGlueXSpin.C")


def _root(macro, cwd, env):
    root = repo_root()
    proc = subprocess.run(["root", "-l", "-b", "-q", str(root / "rootlogon.C"), f"{root / MACROS / macro}(0)"],
                          cwd=cwd, env=env, capture_output=True, text=True, timeout=1800)
    assert proc.returncode == 0, f"{macro}: exit {proc.returncode}\n{(proc.stdout + proc.stderr)[-3000:]}"
    return proc.stdout + proc.stderr


def test_measurements_reproduce_original(tmp_path, need):
    data, out = measurements_farm(tmp_path, need)
    work = out / "kpkpxim" / "measurements"
    work.mkdir()
    env = dict(os.environ, GXANA_ROOT=str(repo_root()), GXANA_DATA=str(data), GXANA_OUTPUT=str(out))
    text = "".join(_root(macro, work, env) for macro in SEQUENCE)
    got, ref = parse_fitresults(text), parse_fitresults(REFERENCE.read_text())
    assert len(ref) == 13, "reference must hold 13 FITRESULT lines"
    # PlotGlueXSpin prints the merged spin line before the per-period ones (the original printed it
    # last), so lines are matched by (kind, name, occurrence), not by position.
    assert set(got) == set(ref), (
        f"missing: {sorted(set(ref) - set(got))}\nextra: {sorted(set(got) - set(ref))}\n{text[-3000:]}")
    assert_fitresults_equal(got, ref)
