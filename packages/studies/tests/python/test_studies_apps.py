"""The study apps reject bad arguments with exit 2 and a message naming the problem."""
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[4]
BIN = ROOT / "build" / "bin"


def _run(app, *args):
    exe = BIN / app
    if not exe.is_file():
        pytest.skip(f"{app} not built (uv run cmake --build build)")
    return subprocess.run([str(exe), *args], capture_output=True, text=True, timeout=120)


@pytest.mark.parametrize("args, message", [
    ((), "usage: gxana_study_cutscan"),
    (("scan",), "unknown step scan"),
    (("fill", "--input"), "--input needs a value"),
    (("fill", "--input", "a.root", "--tree", "t", "--out", "o.root", "--mass", "m:100,1,2"),
     "fill needs --input, --tree, --out, --mass and --scan"),
    (("fill", "--mass", "m:1.5,1,2"), "--mass needs VAR:N,LO,HI with integer N >= 1"),
    (("fit", "--hist", "h.root", "--tables", "t_{what}.txt", "--grid-pdf", "g.pdf", "--first-bin", "4",
      "--panel-label", "x", "--mass-title", "m", "--range", "1,2"), "fit needs --param a0=..."),
    (("plot", "--tables", "t", "--title", "x", "--name", "n", "--pdf", "p.pdf"), "plot needs --tables, --title, --cut"),
])
def test_cutscan_usage_errors(args, message):
    result = _run("gxana_study_cutscan", *args)
    assert result.returncode == 2
    assert message in result.stdout + result.stderr


@pytest.mark.parametrize("args, message", [
    ((), "usage: gxana_study_datamc"),
    (("fit",), "unknown step fit (fill, plot)"),
    (("fill", "--out", "o.root", "--tree", "t", "--thrown-tree", "tt", "--period", "a:b:c:d"),
     "--period needs NAME:DIR:DATA:MC:THROWN"),
    (("fill", "--define", "reco:x=1"), "--define needs SAMPLE:VALUE with SAMPLE data, mc or thrown"),
    (("fill", "--out", "o.root", "--tree", "t", "--thrown-tree", "tt", "--period", "p:D:a:b:c", "--var", "x",
      "--truth-var", "y"), "--truth-var y is not a --var"),
    (("plot", "--in", "i.root", "--out-dir", "d", "--period", "D:tag", "--var", "x:top:title"),
     "POS must be tl or tr"),
])
def test_datamc_usage_errors(args, message):
    result = _run("gxana_study_datamc", *args)
    assert result.returncode == 2
    assert message in result.stdout + result.stderr


FIT_BLOCK = ("--hist", "h.root", "--tables", "t_{what}.txt", "--grid-pdf", "g.pdf", "--first-bin", "4",
             "--panel-label", "x", "--mass-title", "m", "--range", "1,2",
             *(arg for name in ("a0", "a1", "mu", "lambda", "gamma", "delta", "nbkgd", "nxi")
               for arg in ("--param", f"{name}=1,0,2")))


@pytest.mark.parametrize("args, message", [
    (FIT_BLOCK + ("--next",), "fit needs --hist"),
    (("--next",) + FIT_BLOCK, "fit needs --hist"),
    (FIT_BLOCK + ("--next", "--next") + FIT_BLOCK, "fit needs --hist"),
    (FIT_BLOCK + ("--next", "--hist", "h.root"), "fit needs --hist"),
    (FIT_BLOCK + ("--next", "--bogus", "1"), "unknown option --bogus"),
])
def test_cutscan_fit_checks_every_block_before_fitting(args, message, tmp_path):
    """h.root does not exist: were the first (valid) block fitted, the error would name it instead."""
    result = _run("gxana_study_cutscan", "fit", *args)
    assert result.returncode == 2
    assert message in result.stdout + result.stderr
    assert "no TH2 cutscan" not in result.stdout + result.stderr
