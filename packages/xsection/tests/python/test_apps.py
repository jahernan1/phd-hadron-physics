"""The gxana_xsec_* executables start, print usage, and reject bad arguments."""
import subprocess

import pytest

from gxana.paths import repo_root

BIN = repo_root() / "build" / "bin"


def run(name, *args):
    exe = BIN / name
    if not exe.exists():
        pytest.skip(f"{exe} not built")
    return subprocess.run([str(exe), *args], capture_output=True, text=True)


def test_bin_help():
    out = run("gxana_xsec_bin", "--help")
    assert out.returncode == 0
    assert "usage: gxana_xsec_bin" in out.stdout


def test_bin_usage_errors():
    assert run("gxana_xsec_bin").returncode == 2
    out = run("gxana_xsec_bin", "data", "in.root", "out.root", "--energy", "6.4", "--t", "0.1,0.35")
    assert out.returncode == 2
    assert "at least two edges" in out.stderr


def test_bin_missing_input(tmp_path):
    out = run("gxana_xsec_bin", "thrown", str(tmp_path / "absent.root"), str(tmp_path / "o.root"),
              "--energy", "6.4,11.4", "--t", "0.1,0.35")
    assert out.returncode == 1
