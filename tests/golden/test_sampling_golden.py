"""The common AcceptanceCorrect library reproduces the preserved 2-D sampling histogram
(the gen_amp Hist2D input) from its stored raw per-period histograms, bin for bin."""
import os
import re
import shutil
import subprocess

import pytest

from gxana.paths import repo_root

pytestmark = [pytest.mark.golden, pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")]

SAMPLING = "simulation/sampling/data_ac_ximVertexCut_hist2d_YstarRest.root"


def _run(path):
    root = repo_root()
    env = dict(os.environ, GXANA_ROOT=str(root))
    proc = subprocess.run(
        ["root", "-l", "-b", "-q", "rootlogon.C", f'tests/golden/sampling_recompute.C("{path}")'],
        cwd=root, env=env, capture_output=True, text=True, timeout=600)
    return proc.stdout + proc.stderr


def test_sampling_histogram_reproduced(need):
    (path,) = need(SAMPLING)
    out = _run(path)
    diffs = {m.group(1): float(m.group(2)) for m in re.finditer(r"MAXDIFF (\S+) (\S+)", out)}
    assert "phase1_ac" in diffs, out[-3000:]
    for name, value in diffs.items():
        assert value < 1e-9, f"{name}: max bin difference {value}\n{out[-3000:]}"
