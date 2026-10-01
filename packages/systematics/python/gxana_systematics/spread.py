"""Per-bin mean and sample standard deviation (N-1) of the weighted dsigma/dt of a
set of analysis variants: the spread systematic (dissertation ch7; legacy
GetPointwiseMeanAndStdDev in PlotComboComparison.C / PlotFitComparison.C)."""
from __future__ import annotations

import argparse
import sys
from typing import Dict, List, Optional, Sequence, Tuple

import numpy as np

from gxana_systematics.tables import Block, read_weighted

Row = Tuple[float, float, float, float]
HEADER = "XVal XErr YMean StdDev"


class SpreadError(ValueError):
    pass


def spread(members: Dict[str, List[Block]]) -> List[Row]:
    if len(members) < 2:
        raise SpreadError(f"a spread needs at least 2 members, got {list(members)}")
    labels = list(members)
    first = members[labels[0]]
    for label in labels[1:]:
        if [(b.emin, b.emax) for b in members[label]] != [(b.emin, b.emax) for b in first]:
            raise SpreadError(f"{label!r} and {labels[0]!r} have different energy bins")
    rows: List[Row] = []
    for i, ref in enumerate(first):
        for label in labels[1:]:
            other = members[label][i].rows
            if other.shape[0] != ref.rows.shape[0] or not np.allclose(other[:, [0, 2]], ref.rows[:, [0, 2]]):
                raise SpreadError(f"{label!r} and {labels[0]!r} have different -t points in "
                                  f"energy bin {ref.emin}-{ref.emax}")
        y = np.array([members[label][i].rows[:, 1] for label in labels])
        mean = y.mean(axis=0)
        std = y.std(axis=0, ddof=1)
        for j in range(ref.rows.shape[0]):
            rows.append((float(ref.rows[j, 0]), float(ref.rows[j, 2]), float(mean[j]), float(std[j])))
    return rows


def write_stats(path: str, rows: Sequence[Sequence[float]], header: str = HEADER) -> None:
    with open(path, "w") as f:
        f.write(header + "\n")
        for row in rows:
            f.write(" ".join(f"{v:.6g}" for v in row) + "\n")


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--out", required=True)
    parser.add_argument("--member", action="append", required=True, metavar="LABEL=DIR")
    args = parser.parse_args(argv)
    members = {}
    for item in args.member:
        label, _, directory = item.partition("=")
        members[label] = read_weighted(directory)
    try:
        write_stats(args.out, spread(members))
    except SpreadError as err:
        print(f"gxana: error: {err}", file=sys.stderr)
        return 1
    print(f"wrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
