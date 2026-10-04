"""PDG scale factor of the run-period combination and the run-period systematic
built from it (dissertation ch6 eq. scale_factor, ch7 internal_syst.tex:82-97).

Per point, with the per-period values x_i and statistical errors sigma_i:
w_i = 1/sigma_i^2, mean = sum w_i x_i / sum w_i, stat_err = (sum w_i)^(-1/2),
chi2 = sum w_i (mean - x_i)^2, N = periods with nonzero weight, S = sqrt(chi2/(N-1)).
The systematic is a separate rule: stat_err*(S-1) if S > 1, else 0.
S itself comes from gxana_xsection.weighted_average (one implementation)."""
from __future__ import annotations

import argparse
import sys
from typing import NamedTuple, Optional, Sequence

import numpy as np

from gxana_systematics.spread import write_stats
from gxana_systematics.tables import read_periods
from gxana_xsection.weighted_average import calculate_weighted_average

HEADER = "XVal XErr YMean StatErr Chi2 N S Syst"


class SFactor(NamedTuple):
    mean: np.ndarray
    stat_err: np.ndarray
    chi2: np.ndarray
    n: np.ndarray
    s: np.ndarray


def scale_factor(y: np.ndarray, err: np.ndarray) -> SFactor:
    y = np.asarray(y, dtype=float)
    err = np.asarray(err, dtype=float)
    with np.errstate(divide="ignore", invalid="ignore"):
        weights = 1 / err ** 2
        weights[np.isinf(weights)] = 0
        n = np.count_nonzero(weights, axis=0)
        mean, stat_err, s = calculate_weighted_average(y, err.copy())
    chi2 = s ** 2 * (n - 1)
    return SFactor(mean, stat_err, chi2, n, s)


def run_systematic(stat_err: np.ndarray, s: np.ndarray) -> np.ndarray:
    stat_err = np.asarray(stat_err, dtype=float)
    s = np.asarray(s, dtype=float)
    return np.where(s > 1, stat_err * (s - 1), 0.0)


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=(__doc__ or "").splitlines()[0])
    parser.add_argument("--out", required=True)
    parser.add_argument("--periods-dir", required=True)
    parser.add_argument("--n-periods", type=int, required=True)
    parser.add_argument("--energy", action="append", required=True, metavar="LO:HI")
    args = parser.parse_args(argv)
    rows = []
    for item in args.energy:
        lo, _, hi = item.partition(":")
        periods = read_periods(args.periods_dir, lo, hi, args.n_periods)
        y = np.array([p[:, 1] for p in periods])
        err = np.array([p[:, 3] for p in periods])
        r = scale_factor(y, err)
        syst = run_systematic(r.stat_err, r.s)
        for j in range(y.shape[1]):
            rows.append((periods[0][j, 0], periods[0][j, 2], r.mean[j], r.stat_err[j], r.chi2[j],
                         int(r.n[j]), r.s[j], syst[j]))
    write_stats(args.out, rows, header=HEADER)
    print(f"wrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
