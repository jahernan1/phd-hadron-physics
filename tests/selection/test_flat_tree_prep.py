"""flatTreePrep.C on a synthetic raw tree: branch definitions and nominal cut (spec D18)."""
import os
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
pytestmark = pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")
NAME = "flatTree_kpkpxim__TEST_2018-08_ana02"

CHECK = r'''
void check(const char* f) {
  ROOT::RDataFrame d("flatTree_kpkpxim", f);
  auto n = d.Count();
  auto bad_rap = d.Filter("fabs(kphigh_rapidity - kphigh_p4.Rapidity()) > 1e-9").Count();
  auto bad_prap = d.Filter("fabs(kphigh_prapidity - atanh(kphigh_p4.Pz()/kphigh_p4.P())) > 1e-9").Count();
  auto bad_low = d.Filter("fabs(kplow_rapidity - kplow_p4.Rapidity()) > 1e-9").Count();
  auto bad_y = d.Filter("fabs(ystar_rapidity - ystar_p4.Rapidity()) > 1e-9").Count();
  auto below = d.Filter("kphigh_p4.Rapidity() <= 2").Count();
  printf("N=%llu BADRAP=%llu BADPRAP=%llu BADLOW=%llu BADY=%llu BELOW=%llu\n", *n, *bad_rap, *bad_prap, *bad_low, *bad_y, *below);
}
'''


def _root(args, env, cwd=ROOT):
    return subprocess.run(["root", "-l", "-b", "-q", *args], cwd=cwd, env=env, capture_output=True, text=True, timeout=600)


def test_prep_defines_true_rapidity(tmp_path):
    env = dict(os.environ, GXANA_ROOT=str(ROOT), GXANA_DATA=str(tmp_path / "data"),
               GXANA_OUTPUT=str(tmp_path / "out"), GXANA_SCRATCH=str(tmp_path / "scratch"))
    raw = tmp_path / "data" / "Trees" / "flatTree" / "rawTrees"
    raw.mkdir(parents=True)
    p = _root([f'analyses/kpkpxim/selection/tests/make_raw_tree.C("{raw / (NAME + ".root")}")'], env)
    assert p.returncode == 0, p.stdout + p.stderr
    p = _root(["rootlogon.C", f'analyses/kpkpxim/selection/flatTreePrep.C("{NAME}", 1)'], env)
    assert p.returncode == 0, p.stdout[-3000:] + p.stderr[-3000:]
    out = tmp_path / "data" / "flatTrees" / f"{NAME}_nominal_kphighrap.root"
    assert out.is_file()
    (tmp_path / "check.C").write_text(CHECK)
    p = _root([f'{tmp_path / "check.C"}("{out}")'], env)
    line = next(l for l in p.stdout.splitlines() if l.startswith("N="))
    vals = dict(kv.split("=") for kv in line.split())
    assert int(vals["N"]) > 0
    assert vals["BELOW"] == "0"          # nominal cut is on true rapidity (unchanged by the fix)
    assert vals["BADRAP"] == "0" and vals["BADPRAP"] == "0" and vals["BADLOW"] == "0" and vals["BADY"] == "0"
