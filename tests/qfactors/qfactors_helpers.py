"""Helpers for the QFactors engine tests (shell out to ROOT; no PyROOT)."""
from __future__ import annotations

import math
import os
import subprocess
from pathlib import Path
from typing import Dict, List, NamedTuple, Tuple

HERE = Path(__file__).resolve().parent
GX_VARS = ("GXANA_DATA", "GXANA_OUTPUT", "GXANA_SCRATCH", "GXANA_EXTERNALS", "GXANA_ANALYSIS_DATA")


class Row(NamedTuple):
    entry: int
    qvalue: float
    status: int
    chisq: float
    neighbors: Tuple[int, ...]


def _root(args: List[str]) -> str:
    proc = subprocess.run(["root", "-l", "-b", "-q", *args], capture_output=True, text=True, timeout=900)
    if proc.returncode != 0:
        raise RuntimeError((proc.stdout + proc.stderr)[-4000:])
    return proc.stdout


def dump(path: Path, tree: str, var: str, first: int = 0, count: int = -1) -> List[Row]:
    out = _root([f'{HERE / "dump_qfactors.C"}("{path}","{tree}","{var}",{first},{count})'])
    rows = []
    for line in out.splitlines():
        if line.startswith("NOTREE"):
            raise RuntimeError(f"{path}: {line}")
        if not line.startswith("ROW "):
            continue
        f = line.split()
        rows.append(Row(int(f[1]), float(f[2]), int(f[3]), float(f[4]), tuple(int(x) for x in f[6:6 + int(f[5])])))
    return rows


def entries(path: Path, tree: str) -> int:
    # root -q exits with the value of the last -e expression; end on 0.
    out = _root(["-e", f'TFile f("{path}"); printf("ENTRIES %lld\\n", ((TTree*)f.Get("{tree}"))->GetEntries()); 0;'])
    return int(next(line.split()[1] for line in out.splitlines() if line.startswith("ENTRIES")))


def make_toy_tree(path: Path, n: int = 600) -> None:
    _root([f'{HERE / "make_toy_tree.C"}("{path}",{n})'])


def gx_env(tmp_path: Path) -> Dict[str, str]:
    env = dict(os.environ)
    env.update({v: str(tmp_path / v.lower()) for v in GX_VARS})
    return env


def is_nan(x: float) -> bool:
    return math.isnan(x)
