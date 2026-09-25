"""FitXimStar.C runs on a toy merged tree in both modes and writes the legacy plot names."""
import os
import re
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
MACRO = ROOT / "analyses/kpkpkmlamb/measurements/FitXimStar.C"


def test_as_run_selections_kept():
    text = MACRO.read_text()
    assert '"kphigh_p4.Rapidity()>0&&t_dist>1"' in text      # FitXimStarCuts.C filter as saved
    assert '"kphigh_p4.Rapidity()>0"' in text                 # FitXimStar.C filter
    assert re.search(r'Histo1D\(\{"",\s*" ; M\(#LambdaK\^\{-\}\) \(GeV/c\^\{2\}\); Counts", 150,1\.6,2\.6\}', text)


@pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")
@pytest.mark.parametrize("tcut,pdf", [("false", "Xi1820massFit.pdf"), ("true", "Xi1820massFit_TCut3.pdf")])
def test_fit_runs_on_toy(tmp_path, tcut, pdf):
    data = tmp_path / "data"
    env = dict(os.environ, GXANA_ROOT=str(ROOT), GXANA_DATA=str(data),
               GXANA_OUTPUT=str(tmp_path / "out"), GXANA_SCRATCH=str(tmp_path / "scratch"))
    (data / "kpkpkmlamb").mkdir(parents=True)
    toy = data / "kpkpkmlamb" / "flatTree_kpkpkmlamb_GlueX-I.root"
    p = subprocess.run(["root", "-l", "-b", "-q", f'{ROOT}/analyses/kpkpkmlamb/measurements/tests/make_toy_tree.C("{toy}")'],
                       cwd=ROOT, env=env, capture_output=True, text=True, timeout=300)
    assert p.returncode == 0, p.stdout + p.stderr
    run = tmp_path / "run"
    run.mkdir()
    p = subprocess.run(["root", "-l", "-b", "-q", f"{ROOT}/rootlogon.C", f"{MACRO}(1, {tcut})"],
                       cwd=run, env=env, capture_output=True, text=True, timeout=900)
    out = p.stdout + p.stderr
    assert p.returncode == 0 and "error:" not in out, out[-4000:]
    assert (run / pdf).is_file(), sorted(x.name for x in run.iterdir())
