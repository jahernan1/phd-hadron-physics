"""The gxana_barlow_* executables start, print usage, and reject bad arguments."""
import subprocess

import pytest

from gxana.paths import repo_root

BIN = repo_root() / "build" / "bin"


def run(name, *args):
    exe = BIN / name
    if not exe.exists():
        pytest.skip(f"{exe} not built")
    return subprocess.run([str(exe), *args], capture_output=True, text=True)


def test_trees_help_and_usage():
    out = run("gxana_barlow_trees", "--help")
    assert out.returncode == 0 and "usage: gxana_barlow_trees" in out.stdout
    assert run("gxana_barlow_trees").returncode == 2
    bad = run("gxana_barlow_trees", "--tree", "t", "--input", "d", "--input-mc", "m", "--branch", "b",
              "--out", "o", "--variation", "no_equals_sign")
    assert bad.returncode == 2 and "NAME=VALUE" in bad.stderr
    assert run("gxana_barlow_trees", "--tree", "t", "--input", "d", "--input-mc", "m", "--out", "o",
               "--variation", "v=x").returncode == 2  # no --branch
    assert run("gxana_barlow_trees", "--threads", "-1").returncode == 2
