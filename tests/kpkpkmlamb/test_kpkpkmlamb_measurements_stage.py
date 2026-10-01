"""`gxana run measurements --channel kpkpkmlamb` on the seeded toy merged tree of test_kpkpkmlamb_fit.py:
FitXimStar.C runs in both selections from the output directory and writes the legacy PDF names there;
with Ghostscript, their rasters equal FitXimStar.C run by hand (n_threads 0 on both sides). The
hand-run false mode equals the original FitXimStar.C (checked once when the stage was added; the
original t_dist selection, FitXimStarCuts.C, does not compile)."""
import copy
import os
import shutil
import subprocess
from pathlib import Path

import pytest

from gxana import config
from gxana.stages import measurements

ROOT = Path(__file__).resolve().parents[2]
MACRO = ROOT / "analyses/kpkpkmlamb/measurements/FitXimStar.C"
PDFS = {"false": "Xi1820massFit.pdf", "true": "Xi1820massFit_TCut3.pdf"}

pytestmark = pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")


@pytest.fixture(scope="module")
def runs(tmp_path_factory):
    tmp = tmp_path_factory.mktemp("kml_stage")
    env = dict(os.environ, GXANA_ROOT=str(ROOT), GXANA_DATA=str(tmp / "data"), GXANA_OUTPUT=str(tmp / "out"),
               GXANA_SCRATCH=str(tmp / "scratch"), ROOT_MAX_THREADS="1")
    toy = tmp / "data" / "kpkpkmlamb" / "flatTree_kpkpkmlamb_GlueX-I.root"
    toy.parent.mkdir(parents=True)
    p = subprocess.run(["root", "-l", "-b", "-q", f'{ROOT}/analyses/kpkpkmlamb/measurements/tests/make_toy_tree.C("{toy}")'],
                       cwd=ROOT, env=env, capture_output=True, text=True, timeout=300)
    assert p.returncode == 0, p.stdout + p.stderr
    cfg = copy.deepcopy(config.load_channel("kpkpkmlamb"))
    for item in cfg["measurements"]["items"].values():
        item["fit"]["args"] = [0, item["fit"]["args"][1]]

    def runner(argv, **kwargs):
        return subprocess.run(argv, env=env, capture_output=True, text=True, timeout=900, **kwargs)

    assert measurements.run_measurements(cfg, ["fit"], runner=runner, environ=env) == 0
    hand = tmp / "hand"
    hand.mkdir()
    for tcut in PDFS:
        p = subprocess.run(["root", "-l", "-b", "-q", f"{ROOT}/rootlogon.C", f"{MACRO}(0, {tcut})"],
                           cwd=hand, env=env, capture_output=True, text=True, timeout=900)
        assert p.returncode == 0, (p.stdout + p.stderr)[-3000:]
    return tmp / "out" / "kpkpkmlamb" / "measurements", hand


def test_stage_writes_the_legacy_pdfs(runs):
    stage_dir, _ = runs
    assert sorted(p.name for p in stage_dir.iterdir()) == sorted(PDFS.values())


def _raster(pdf):
    return subprocess.run(["gs", "-q", "-dNOPAUSE", "-dBATCH", "-dSAFER", "-sDEVICE=pgmraw", "-r100",
                           "-sOutputFile=-", str(pdf)], capture_output=True, check=True).stdout


@pytest.mark.skipif(shutil.which("gs") is None, reason="Ghostscript (gs) not installed")
def test_stage_pdfs_equal_the_hand_run(runs):
    stage_dir, hand = runs
    for name in PDFS.values():
        assert _raster(stage_dir / name) == _raster(hand / name), name
