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
