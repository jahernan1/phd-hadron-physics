"""packages/qfactors points at the published fork, and the recorded commit is on its kpkpxim-thesis branch."""
import os
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
URL = "https://github.com/jahernan1/QFactors.git"


def _git(*args):
    return subprocess.run(["git", *args], cwd=ROOT, capture_output=True, text=True, check=True).stdout.strip()


def test_gitmodules_points_at_fork_branch():
    assert _git("config", "-f", ".gitmodules", "submodule.packages/qfactors.url") == URL
    assert _git("config", "-f", ".gitmodules", "submodule.packages/qfactors.branch") == "kpkpxim-thesis"


@pytest.mark.network
@pytest.mark.skipif(os.environ.get("GXANA_NETWORK_TESTS") != "1", reason="opt-in: GXANA_NETWORK_TESTS=1")
def test_recorded_commit_is_published():
    recorded = _git("ls-tree", "HEAD", "packages/qfactors").split()[2]
    remote = _git("ls-remote", URL, "refs/heads/kpkpxim-thesis").split()[0]
    assert remote == recorded
