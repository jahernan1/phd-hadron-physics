"""flatTreePrep.C runs all three periods on toy raw trees and applies the as-run nominal cuts."""
import os
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
pytestmark = pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")
STEMS = ["kpkpkmlamb__B4_M18_2017-01_ana55", "kpkpkmlamb__B4_M18_2018-01_ana22", "kpkpkmlamb__B4_M18_2018-08_ana19"]
COUNT = 'void count(const char* f) { printf("N=%llu\\n", *ROOT::RDataFrame("flatTree_kpkpkmlamb", f).Count()); }\n'


def _root(args, env, cwd=ROOT):
    return subprocess.run(["root", "-l", "-b", "-q", *args], cwd=cwd, env=env, capture_output=True, text=True, timeout=600)


def test_prep_all_periods(tmp_path):
    data = tmp_path / "data"
    env = dict(os.environ, GXANA_ROOT=str(ROOT), GXANA_DATA=str(data),
               GXANA_OUTPUT=str(tmp_path / "out"), GXANA_SCRATCH=str(tmp_path / "scratch"))
    raw = data / "Trees" / "flatTree" / "rawTrees"
    raw.mkdir(parents=True)
    for stem in STEMS:
        p = _root([f'analyses/kpkpkmlamb/flat_trees/tests/make_raw_tree.C("{raw / ("flatTree_" + stem + ".root")}")'], env)
        assert p.returncode == 0, p.stdout + p.stderr
    # $GXANA_DATA/kpkpkmlamb does not exist yet: the prep must create it.
    p = _root(["rootlogon.C", "analyses/kpkpkmlamb/flat_trees/flatTreePrep.C"], env)
    assert p.returncode == 0 and "error" not in (p.stdout + p.stderr).lower(), p.stdout[-3000:] + p.stderr[-3000:]
    (tmp_path / "count.C").write_text(COUNT)
    for stem in STEMS:
        out = data / "kpkpkmlamb" / f"flatTree_{stem}_nominal_allCuts.root"
        assert out.is_file(), out
        p = _root([f'{tmp_path / "count.C"}("{out}")'], env)
        assert "N=4" in p.stdout, p.stdout + p.stderr
