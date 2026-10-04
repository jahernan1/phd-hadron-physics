"""Point-by-point systematic total (quadrature of the summary.point_by_point studies'
last columns) and the separate normalization record."""
from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Dict, List, Optional, Sequence

import numpy as np

from gxana_systematics.tables import Block, read_weighted


def combine(blocks: List[Block], columns: Dict[str, np.ndarray],
            sources: Optional[Dict[str, str]] = None) -> List[tuple]:
    """columns: name -> stats table (XVal first, the systematic last), rows in nominal
    table order; each XVal must match the nominal -t (sources name the files in errors)."""
    n = sum(b.rows.shape[0] for b in blocks)
    xval = np.concatenate([b.rows[:, 0] for b in blocks]) if blocks else np.zeros(0)
    values = {}
    for name, table in columns.items():
        where = (sources or {}).get(name, f"column {name!r}")
        table = np.asarray(table, dtype=float)
        if len(table) != n:
            raise ValueError(f"{where} has {len(table)} rows, the nominal tables {n}")
        if not np.allclose(table[:, 0], xval):
            bad = int(np.argmax(~np.isclose(table[:, 0], xval)))
            raise ValueError(f"{where}: XVal {table[bad, 0]:g} in row {bad + 1} does not match the nominal "
                             f"-t {xval[bad]:g}")
        values[name] = table[:, -1]
    total = np.sqrt(sum(v ** 2 for v in values.values()))
    rows, i = [], 0
    for b in blocks:
        for r in b.rows:
            rows.append((b.emin, b.emax, float(r[0]), float(r[2]), *(float(v[i]) for v in values.values()),
                         float(total[i])))
            i += 1
    return rows


def _stats(path: str) -> np.ndarray:
    return np.loadtxt(path, skiprows=1, ndmin=2)


def _normalization_value(spec: str) -> str:
    if Path(spec).is_file():
        for line in Path(spec).read_text().splitlines():
            parts = line.split()
            if parts and parts[0] == "report":
                return parts[2]
        raise ValueError(f"{spec}: no 'report' line")
    return f"{float(spec):.6g}"


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=(__doc__ or "").splitlines()[0])
    parser.add_argument("--nominal-dir", required=True)
    parser.add_argument("--out-dir", required=True)
    parser.add_argument("--column", action="append", default=[], metavar="NAME=FILE")
    parser.add_argument("--normalization", action="append", default=[], metavar="NAME=FILE|VALUE")
    args = parser.parse_args(argv)
    out = Path(args.out_dir)
    out.mkdir(parents=True, exist_ok=True)
    sources = {n: p for n, _, p in (c.partition("=") for c in args.column)}
    try:
        rows = combine(read_weighted(args.nominal_dir), {n: _stats(p) for n, p in sources.items()}, sources)
    except ValueError as err:  # TableError included
        print(f"gxana: error: {err}", file=sys.stderr)
        return 1
    with open(out / "systematics_summary.txt", "w") as f:
        f.write("Emin Emax XVal XErr " + " ".join(sources) + " Total\n")
        for r in rows:
            f.write(f"{r[0]} {r[1]} " + " ".join(f"{v:.6g}" for v in r[2:]) + "\n")
    with open(out / "normalization.txt", "w") as f:
        for name, _, spec in (n.partition("=") for n in args.normalization):
            f.write(f"{name} {_normalization_value(spec)}\n")
    print(f"wrote {out / 'systematics_summary.txt'} and {out / 'normalization.txt'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
