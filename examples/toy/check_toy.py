"""Compare the toy cross sections of `gxana run xsection --channel toy` with the injected truth.

    uv run python examples/toy/check_toy.py [--output DIR] [--data DIR]

Reads the weighted (run-period averaged) tables under DIR/toy/xsection/weighted_data/toy
(default DIR: $GXANA_OUTPUT) and the truth.txt that make_toy.py wrote (default
$GXANA_DATA/toy/truth.txt). Prints, per bin, the measured value, the truth and the pull
(measured - truth) / statistical error. Exits 1 if a pull exceeds --max-pull (default 4)
or a value is off by more than --max-rel (default 0.15) of the truth.
"""
from __future__ import annotations

import argparse
import math
import os
import re
import sys
from pathlib import Path

ROW = "{:<34} {:>10} {:>10} {:>8} {:>7}"


def read_truth(path: Path) -> dict:
    truth = {}
    for line in path.read_text().splitlines():
        parts = line.split()
        if len(parts) == 2 and not line.startswith("#"):
            truth[parts[0]] = float(parts[1])
    return truth


def read_rows(path: Path):
    """Rows x, y, ex, ey, S of a weighted table (header line skipped)."""
    return [[float(v) for v in line.split()] for line in path.read_text().splitlines()[1:] if line.strip()]


def mean_dsigma_dt(truth: dict, lo: float, hi: float) -> float:
    a0, b = truth["A0"], truth["b"]
    return a0 / b * (math.exp(-b * lo) - math.exp(-b * hi)) / (hi - lo)


def compare(xsec_dir: Path, truth: dict):
    """(label, measured, error, truth) for every weighted dsigma/dt point and both total cross sections."""
    wdir = xsec_dir / "weighted_data" / "toy"
    points = []
    tables = sorted(wdir.glob("weighted_diffxsec_emin_*.txt"))
    if not tables:
        raise FileNotFoundError(f"no weighted_diffxsec_emin_*.txt in {wdir}; run gxana run xsection --channel toy")
    for table in tables:
        emin, emax = re.findall(r"\d+\.\d+", table.name)[:2]
        for x, y, ex, ey, _ in read_rows(table):
            label = f"dsigma/dt E {emin}-{emax} t {x - ex:.2f}-{x + ex:.2f}"
            points.append((label, y, ey, mean_dsigma_dt(truth, x - ex, x + ex)))
    sigma = truth["A0"] / truth["b"] * (math.exp(-truth["b"] * truth["t_min"]) - math.exp(-truth["b"] * truth["t_max"]))
    for name, what in (("totxsec_weighted_output.txt", "sigma direct"), ("intxsec_weighted_output.txt", "sigma integrated")):
        for x, y, ex, ey, _ in read_rows(wdir / name):
            points.append((f"{what} E {x - ex:.2f}-{x + ex:.2f}", y, ey, sigma))
    return points


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=(__doc__ or "").partition("\n")[0])
    parser.add_argument("--output", default=os.environ.get("GXANA_OUTPUT"), help="GXANA_OUTPUT of the run")
    parser.add_argument("--data", default=os.environ.get("GXANA_DATA"), help="GXANA_DATA of the run")
    parser.add_argument("--max-pull", type=float, default=4.0)
    parser.add_argument("--max-rel", type=float, default=0.15)
    args = parser.parse_args(argv)
    if not args.output or not args.data:
        parser.error("set GXANA_OUTPUT and GXANA_DATA, or pass --output and --data")
    truth = read_truth(Path(args.data) / "toy" / "truth.txt")
    points = compare(Path(args.output) / "toy" / "xsection", truth)
    print(ROW.format("bin", "measured", "truth", "stat", "pull"))
    bad = 0
    for label, y, ey, true in points:
        pull = (y - true) / ey if ey > 0 else math.inf
        ok = abs(pull) <= args.max_pull and abs(y - true) <= args.max_rel * true
        bad += not ok
        print(ROW.format(label, f"{y:.3f}", f"{true:.3f}", f"{ey:.3f}", f"{pull:+.2f}") + ("" if ok else "  <- off"))
    print(f"{len(points) - bad}/{len(points)} within {args.max_pull:g} sigma and {args.max_rel:.0%} of the truth "
          "(units: nb/GeV^2 for dsigma/dt, nb for sigma)")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
