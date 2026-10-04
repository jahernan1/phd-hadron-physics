"""Track-efficiency systematic (dissertation external_syst.tex:19-58): per track
(low*N(theta<cut) + high*N(theta>cut)) / N, summed over the tracks; a per-particle
override replaces the computed value (GlueX guidance: proton 5 %). Counts come from
gxana_syst_track (track_counts.txt)."""
from __future__ import annotations

import argparse
import sys
from typing import Dict, Optional, Sequence, Tuple

Counts = Tuple[float, float, float, float]  # nlow_data, nhigh_data, nlow_mc, nhigh_mc


def _raw(nlow: float, nhigh: float, low: float, high: float, name: str) -> float:
    if nlow + nhigh <= 0:
        raise ValueError(f"track {name!r}: no entries")
    return (low * nlow + high * nhigh) / (nlow + nhigh)


def efficiencies(counts: Dict[str, Counts], low: float, high: float,
                 override: Dict[str, float]) -> Dict[str, Dict[str, float]]:
    out = {}
    for name, (nl_d, nh_d, nl_m, nh_m) in counts.items():
        data_raw, mc_raw = _raw(nl_d, nh_d, low, high, name), _raw(nl_m, nh_m, low, high, name)
        data = override.get(name, data_raw)
        mc = override.get(name, mc_raw)
        out[name] = {"data": data, "mc": mc, "data_raw": data_raw, "mc_raw": mc_raw}
    return out


def read_counts(path: str) -> Dict[str, Counts]:
    counts = {}
    with open(path) as f:
        next(f)
        for line in f:
            parts = line.split()
            if parts:
                counts[parts[0]] = tuple(float(v) for v in parts[1:5])
    return counts


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=(__doc__ or "").splitlines()[0])
    parser.add_argument("--counts", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--low", type=float, required=True)
    parser.add_argument("--high", type=float, required=True)
    parser.add_argument("--override", action="append", default=[], metavar="NAME=VALUE")
    parser.add_argument("--report", choices=["data", "mc"], required=True)
    args = parser.parse_args(argv)
    override = {k: float(v) for k, _, v in (o.partition("=") for o in args.override)}
    eff = efficiencies(read_counts(args.counts), args.low, args.high, override)
    cols = ("data", "mc", "data_raw", "mc_raw")
    with open(args.out, "w") as f:
        f.write("particle " + " ".join(cols) + "\n")
        for name, e in eff.items():
            f.write(name + " " + " ".join(f"{e[c]:.6g}" for c in cols) + "\n")
        totals = {c: sum(e[c] for e in eff.values()) for c in cols}
        f.write("total " + " ".join(f"{totals[c]:.6g}" for c in cols) + "\n")
        f.write(f"report {args.report} {totals[args.report]:.6g}\n")
    print(f"wrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
