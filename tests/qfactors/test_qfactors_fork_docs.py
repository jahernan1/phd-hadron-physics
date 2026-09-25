"""Fork documentation matches the fork history; makePlots loads."""
import shutil
import subprocess
from pathlib import Path

import pytest

ENGINE = Path(__file__).resolve().parents[2] / "packages" / "qfactors"
BASE = "d130c051c8e8d7e02d6394b10e66d3b6730e7a29"
pytestmark = pytest.mark.skipif(not (ENGINE / "main.C").is_file(), reason="packages/qfactors not checked out")


def _subjects():
    out = subprocess.run(["git", "-C", str(ENGINE), "log", "--format=%s", f"{BASE}..HEAD"],
                         capture_output=True, text=True, check=True).stdout
    return [line for line in out.splitlines() if line]


def test_readme_starts_with_fork_notice():
    first = (ENGINE / "README.md").read_text().lstrip().splitlines()[0]
    assert "Fork of [lan13005/QFactors]" in first


def test_changes_lists_every_fork_commit():
    changes = (ENGINE / "CHANGES_THESIS.md").read_text()
    subjects = _subjects()
    assert subjects, "no fork commits on top of d130c05"
    missing = [s for s in subjects if s not in changes and not s.startswith("Document the fork")]
    assert not missing, missing


def test_fork_code_holds_no_thesis_names():
    # README/CHANGES may name the thesis and the branch; engine code may not.
    out = subprocess.run(["git", "-C", str(ENGINE), "diff", BASE, "HEAD", "--", ".",
                          ":!README.md", ":!CHANGES_THESIS.md"], capture_output=True, text=True, check=True).stdout
    added = [l for l in out.splitlines() if l.startswith("+") and not l.startswith("+++")]
    for bad in ("hjesse", "GXANA_", "kpkpxim", "decayxim", "hybrid_combo", "/d/grid"):
        assert not [l for l in added if bad in l], bad


@pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")
def test_makeplots_loads_under_cling(tmp_path):
    work = tmp_path / "engine"
    shutil.copytree(ENGINE, work, ignore=shutil.ignore_patterns(".git"))
    proc = subprocess.run(["root", "-l", "-b", "-q", "-e", ".L makePlots.C"], cwd=work,
                          capture_output=True, text=True, timeout=300)
    out = proc.stdout + proc.stderr
    assert proc.returncode == 0 and "error:" not in out, out[-4000:]
