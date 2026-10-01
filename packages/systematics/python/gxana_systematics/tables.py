"""Read the cross-section text tables the xsection engine writes.

weighted_data/<label>/weighted_diffxsec_emin_<lo>_emax_<hi>.txt: x, dsigma/dt, ex, ey, S
(gxana_xsection.weighted_average); data/<label>/diffxsec_flatTree_<stem>_emin_<lo>_emax_<hi>.txt:
x, dsigma/dt, ex, ey per run period (gxana_xsec_tables)."""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import List, NamedTuple, Optional, Sequence

import numpy as np

WEIGHTED_RE = re.compile(r"^weighted_diffxsec_emin_(\d+\.\d+)_emax_(\d+\.\d+)\.txt$")


class TableError(ValueError):
    pass


class Block(NamedTuple):
    emin: str
    emax: str
    rows: np.ndarray


def _load(path: Path) -> np.ndarray:
    return np.loadtxt(path, skiprows=1, ndmin=2)


def read_weighted(directory: str) -> List[Block]:
    found = []
    for path in Path(directory).glob("weighted_diffxsec_emin_*.txt"):
        m = WEIGHTED_RE.match(path.name)
        if m:
            found.append(Block(m.group(1), m.group(2), _load(path)))
    if not found:
        raise TableError(f"no weighted_diffxsec_emin_*_emax_*.txt in {directory}")
    return sorted(found, key=lambda b: float(b.emin))


def read_periods(directory: str, emin: str, emax: str, n_periods: int) -> List[np.ndarray]:
    paths = sorted(Path(directory).glob(f"diffxsec*_emin_{emin}_emax_{emax}*.txt"))
    if len(paths) != n_periods:
        raise TableError(f"expected {n_periods} run-period tables diffxsec*_emin_{emin}_emax_{emax}*.txt "
                         f"in {directory}, found {len(paths)}")
    return [_load(p) for p in paths]


def _weighted_names(directory: str) -> set:
    return {p.name for p in Path(directory).glob("weighted_diffxsec_emin_*.txt") if WEIGHTED_RE.match(p.name)}


def same_tables(dir_a: str, dir_b: str) -> List[str]:
    for d in (dir_a, dir_b):
        if not _weighted_names(d):
            raise TableError(f"no weighted_diffxsec_emin_*_emax_*.txt in {d}")
    names = sorted(_weighted_names(dir_a) | _weighted_names(dir_b))
    differ = []
    for name in names:
        a, b = Path(dir_a) / name, Path(dir_b) / name
        if not (a.is_file() and b.is_file()):
            differ.append(name)
            continue
        x, y = _load(a), _load(b)
        if x.shape != y.shape or not np.array_equal(x, y, equal_nan=True):
            differ.append(name)
    return differ


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description="Compare two weighted_data label directories.")
    parser.add_argument("--same", nargs=2, metavar=("DIR_A", "DIR_B"), required=True)
    args = parser.parse_args(argv)
    try:
        differ = same_tables(*args.same)
    except TableError as err:
        print(f"gxana: error: {err}", file=sys.stderr)
        return 1
    if differ:
        print(f"gxana: error: {args.same[0]} and {args.same[1]} differ in: " + ", ".join(differ), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
