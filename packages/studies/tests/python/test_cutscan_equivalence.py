"""The cut-scan studies against the macro they replace: a frozen copy of selection/CutAnalysisRF.C
(tests/legacy/) and `gxana run studies --study chisqndf_scan,mm2_scan` on the same seeded toy raw trees
(the real raw trees are not preserved), both single-threaded: the same files, byte-identical tables,
the same printed fit lines and, when Ghostscript is installed, identical PDF rasters."""
import os
import shutil
import subprocess
import sys
from collections import Counter
from pathlib import Path

import pytest

from gxana import config

ROOT = Path(__file__).resolve().parents[4]
LEGACY = Path(__file__).resolve().parents[1] / "legacy"
APP = ROOT / "build" / "bin" / "gxana_study_cutscan"
LINES = ("Fit Results", "Mean and Sigma", "Signal ", "Background ", "Pad Values")

pytestmark = [pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH"),
              pytest.mark.skipif(not APP.is_file(), reason="gxana_study_cutscan not built")]


def _env(tmp_path, out):
    return dict(os.environ, GXANA_ROOT=str(ROOT), GXANA_DATA=str(tmp_path / "data"), GXANA_OUTPUT=str(out),
                ROOT_MAX_THREADS="1", PYTHONUNBUFFERED="1")


def _files(base):
    return {p.relative_to(base).as_posix(): p for p in base.rglob("*") if p.is_file()}


def _raster(pdf):
    return subprocess.run(["gs", "-q", "-dNOPAUSE", "-dBATCH", "-dSAFER", "-sDEVICE=pgmraw", "-r100",
                           "-sOutputFile=-", str(pdf)], capture_output=True, check=True).stdout


def _printed(text):
    return Counter(line for line in text.splitlines() if line.startswith(LINES))


def test_cutscan_studies_reproduce_the_macro(tmp_path):
    cfg = config.load_channel("kpkpxim")
    raw = tmp_path / "data" / "Trees" / "flatTree" / "rawTrees"
    raw.mkdir(parents=True)
    for seed, period in enumerate(cfg["periods"], start=1):
        out = raw / f"flatTree_{config.tree_stem(cfg, period, 'data')}.root"
        subprocess.run(["root", "-l", "-b", "-q", f'{LEGACY / "make_cutscan_toy.C"}("{out}", {seed}, 20000)'],
                       check=True, capture_output=True, timeout=300)
    old, new = tmp_path / "old", tmp_path / "new"
    for sub in ("kpkpxim/cut_analysis_plots/data", "kpkpxim/cut_analysis_plots/results",
                "kpkpxim/analysis/event_selection/chisqndf_cut"):
        (old / sub).mkdir(parents=True)  # the macro does not create its output directories
    legacy = subprocess.run(["root", "-l", "-b", "-q", str(ROOT / "rootlogon.C"), str(LEGACY / "CutAnalysisRF.C")],
                            cwd=tmp_path, env=_env(tmp_path, old), capture_output=True, text=True, timeout=1200)
    assert legacy.returncode == 0, legacy.stderr[-2000:]
    study = subprocess.run([sys.executable, "-m", "gxana.cli", "run", "studies", "--channel", "kpkpxim",
                            "--study", "chisqndf_scan,mm2_scan"],
                           cwd=ROOT, env=_env(tmp_path, new), capture_output=True, text=True, timeout=1200)
    assert study.returncode == 0, study.stderr[-2000:]
    a, b = _files(old), _files(new)
    assert sorted(a) == sorted(k for k in b if "_hist_flatTree_" not in k)
    assert sum(k.endswith(".txt") for k in a) == 18 and sum(k.endswith(".pdf") for k in a) == 18
    for name in a:
        if name.endswith(".txt"):
            assert b[name].read_bytes() == a[name].read_bytes(), name
    assert sum(_printed(legacy.stdout).values()) == 816
    assert _printed(study.stdout) == _printed(legacy.stdout)
    if shutil.which("gs"):
        for name in a:
            if name.endswith(".pdf"):
                assert _raster(b[name]) == _raster(a[name]), name
