"""The run-period check (`gxana run systematics --steps runperiod`, analyses/kpkpxim/systematics/
GetRunPeriodPctSig.C) against the original macro (AnalysisNote/systematics/GetRunPeriodPctSig.C, frozen
in legacy/runperiod/ with its two directories templated) on the preserved per-period `johnson` tables,
both single-threaded: the same printed lines (168 point significances and three Gaussian fits, means
0.927561 / 0.878222 / 0.979189 as printed, 0.928 / 0.878 / 0.979 in docs/KNOWN_ISSUES.md section 12),
the same 27 PDF names and, with Ghostscript, identical rasters. Lines that carry the run's own
paths, and the return-value line, are left out of the comparison."""
import os
import shutil
import subprocess
from pathlib import Path

import pytest

from gxana import config
from gxana.paths import repo_root
from gxana_systematics import stage

LEGACY = Path(__file__).parent / "legacy" / "runperiod" / "GetRunPeriodPctSig.C"
NOISE = ("Processing ", "Info in <TCanvas::Print>", "(int) ")

pytestmark = [pytest.mark.golden, pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")]


@pytest.fixture(scope="module")
def runs(golden, tmp_path_factory):
    johnson = golden / "reference" / "xsection" / "johnson"
    weighted = golden / "reference" / "xsection" / "weighted" / "johnson"
    if not any(johnson.glob("diffxsec_*_emin_*.txt")) or not any(weighted.glob("weighted_diffxsec_emin_*.txt")):
        pytest.skip(f"no preserved johnson tables under {johnson.parent}")
    tmp = tmp_path_factory.mktemp("runperiod")
    env = dict(os.environ, GXANA_ROOT=str(repo_root()), GXANA_OUTPUT=str(tmp / "new"),
               GXANA_DATA=str(tmp / "data"), ROOT_MAX_THREADS="1")
    old, src = tmp / "old", tmp / "legacy"
    old.mkdir()
    src.mkdir()
    (src / LEGACY.name).write_text(LEGACY.read_text().replace("@DATA_DIR@", f"{johnson}/").replace("@SAVE_DIR@", f"{old}/"))
    proc = subprocess.run(["root", "-l", "-b", "-q", str(src / LEGACY.name)], cwd=old, env=env,
                          capture_output=True, text=True, timeout=900)
    assert proc.returncode == 0, (proc.stdout + proc.stderr)[-3000:]
    xs = tmp / "new" / "kpkpxim" / "xsection"
    (xs / "data").mkdir(parents=True)
    (xs / "weighted_data").mkdir()
    (xs / "data" / "johnson").symlink_to(johnson)
    (xs / "weighted_data" / "johnson").symlink_to(weighted)
    texts = []

    def runner(argv, **kwargs):
        p = subprocess.run(argv, env=env, capture_output=True, text=True, timeout=900, **kwargs)
        texts.append(p.stdout + p.stderr)
        return p

    rc = stage.run_systematics(config.load_channel("kpkpxim"), ["runperiod"], runner=runner, environ=env)
    assert rc == 0, "".join(texts)[-3000:]
    return {"old": (old, proc.stdout + proc.stderr),
            "new": (tmp / "new" / "kpkpxim" / "systematics" / "runperiod", "".join(texts))}


def _lines(text):
    return [line for line in text.splitlines() if not line.startswith(NOISE)]


def _pdfs(d):
    return sorted(p.name for p in d.glob("*.pdf"))


def test_printed_lines_equal_the_original(runs):
    old, new = _lines(runs["old"][1]), _lines(runs["new"][1])
    assert new == old
    means = [line.split("=")[1].split("+/-")[0].strip() for line in new if line.startswith("Mean ")]
    assert means == ["0.927561", "0.878222", "0.979189"]


def test_pdf_names_equal_the_original(runs):
    assert _pdfs(runs["new"][0]) == _pdfs(runs["old"][0])
    assert len(_pdfs(runs["new"][0])) == 27


def _raster(pdf):
    return subprocess.run(["gs", "-q", "-dNOPAUSE", "-dBATCH", "-dSAFER", "-sDEVICE=pgmraw", "-r100",
                           "-sOutputFile=-", str(pdf)], capture_output=True, check=True).stdout


@pytest.mark.skipif(shutil.which("gs") is None, reason="Ghostscript (gs) not installed")
def test_pdf_rasters_equal_the_original(runs):
    (new_dir, _), (old_dir, _) = runs["new"], runs["old"]
    different = [n for n in _pdfs(old_dir) if _raster(new_dir / n) != _raster(old_dir / n)]
    assert not different, different
