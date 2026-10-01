"""C2: run-period comparison of the nominal (legacy PlotRunComparison.C
GetPointwiseMeanAndStdDev): sample std dev across periods scaled by the PDG S where S > 1."""
from __future__ import annotations

import argparse
import sys
from typing import List, Optional, Sequence

import numpy as np

from gxana_systematics.sfactor import scale_factor
from gxana_systematics.spread import write_stats
from gxana_systematics.tables import read_periods

HEADER = "# XVal XErr YWMean StdDevScaled YMean StdDev S"


def rows(periods: List[np.ndarray]) -> List[tuple]:
    if len(periods) < 2:
        raise ValueError(f"a run comparison needs at least 2 run periods, got {len(periods)}")
    y = np.array([p[:, 1] for p in periods])
    err = np.array([p[:, 3] for p in periods])
    r = scale_factor(y, err)
    mean, std = y.mean(axis=0), y.std(axis=0, ddof=1)
    scaled = np.where(r.s > 1, std * r.s, std)
    return [(periods[0][j, 0], periods[0][j, 2], r.mean[j], scaled[j], mean[j], std[j], r.s[j])
            for j in range(y.shape[1])]


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--out", required=True)
    parser.add_argument("--periods-dir", required=True)
    parser.add_argument("--n-periods", type=int, required=True)
    parser.add_argument("--energy", action="append", required=True, metavar="LO:HI")
    args = parser.parse_args(argv)
    out = []
    for item in args.energy:
        lo, _, hi = item.partition(":")
        out += rows(read_periods(args.periods_dir, lo, hi, args.n_periods))
    write_stats(args.out, out, header=HEADER)
    return 0


if __name__ == "__main__":
    sys.exit(main())
