"""Weighted differential cross-section tables with their total systematic.

Port of the per-table output of the legacy scale-factor LaTeX tables
(MakeXsecTexTableScale.py): for every ``weighted_diffxsec_*.txt`` of a directory,
``syst_<name>`` beside it (or in --out-dir) holding the columns of the weighted table
with the total systematic inserted as the fourth column
(``-t d\\sigma/dt \\delta_x \\delta_y_syst \\delta_y S``). A plain TGraphErrors read of
such a file (x, y, ex, ey) is the systematic band of the dissertation figure
``diffxsec_phase1_systematics_<label>`` (analyses/kpkpxim/xsection/PlotDiffXSec.C).

The total is the quadrature sum, in the given order, of named columns: the last column
of a stats file (row i belongs to row i of the tables taken in ascending emin; its first
column must equal the table's -t), or SCALE_FACTOR, the run-period systematic
delta_y*S - delta_y (0 where S < 1) from the table's own S column.
"""
from __future__ import annotations

import argparse
import os
import re
import sys
from glob import glob
from typing import Dict, List, Mapping, Optional, Sequence, Tuple

import numpy as np
import pandas as pd

SCALE_FACTOR = "scale_factor"
PATTERN = "weighted_diffxsec*.txt"
SYST_HEADER = "\\delta_y_syst"


def table_files(directory: str, pattern: str = PATTERN) -> List[str]:
    """The tables of `directory` matching `pattern`, in ascending order of the first decimal
    number of their name (emin); FileNotFoundError naming the pattern if there is none."""
    paths = glob(os.path.join(directory, pattern))
    if not paths:
        raise FileNotFoundError(f"no {pattern} in {directory}")
    def emin(path: str) -> float:
        found = re.search(r"\d+\.\d+", os.path.basename(path))
        if found is None:
            raise ValueError(f"{path}: no emin (decimal number) in the table name")
        return float(found.group())

    return sorted(paths, key=emin)


def scale_factor_systematic(table: pd.DataFrame) -> np.ndarray:
    """delta_y*S - delta_y of a weighted table (its fourth and fifth columns), 0 where S < 1."""
    if table.shape[1] < 5:
        raise ValueError(f"{SCALE_FACTOR}: the table has no S column (fifth column)")
    stat, s = table.iloc[:, 3], table.iloc[:, 4]
    syst = stat * s - stat
    syst[s < 1] = 0
    return syst.to_numpy(dtype=float)


def column_chunks(file_paths: Sequence[str], delimiter: str, columns: Mapping[str, str]
                  ) -> List[Tuple[pd.DataFrame, Dict[str, np.ndarray]]]:
    """Per table (in the given order): the table and {name: its values of each column}.
    A stats file is consumed row by row across the tables; a short or long file, or one whose
    first column is not the table's -t, is a ValueError naming it."""
    stats = {name: pd.read_csv(path, delimiter=delimiter) for name, path in columns.items() if path != SCALE_FACTOR}
    values = {name: df.iloc[:, -1].to_numpy() for name, df in stats.items()}
    xvals = {name: df.iloc[:, 0].to_numpy(dtype=float) for name, df in stats.items()}
    out = []
    start = 0
    for file_path in file_paths:
        filename = os.path.basename(file_path)
        table = pd.read_csv(file_path, delimiter=delimiter)
        n = len(table)
        chunk: Dict[str, np.ndarray] = {}
        for name, path in columns.items():
            if path == SCALE_FACTOR:
                chunk[name] = scale_factor_systematic(table)
                continue
            rows = values[name][start:start + n]
            if len(rows) != n:
                raise ValueError(f"stats files have fewer rows than the tables at {filename}")
            if not np.allclose(xvals[name][start:start + n], table.iloc[:, 0].to_numpy(dtype=float)):
                raise ValueError(f"{path}: XVal does not match the -t column of {filename}")
            chunk[name] = rows
        out.append((table, chunk))
        start += n
    for name, v in values.items():
        if len(v) != start:
            raise ValueError(f"{columns[name]} has {len(v)} rows, the tables {start}")
    return out


def total_systematic(chunk: Mapping[str, np.ndarray]) -> np.ndarray:
    """Quadrature sum of the columns, in their order (the legacy summation order)."""
    return np.sqrt(sum(v ** 2 for v in chunk.values()))


def write_syst_tables(directory: str, columns: Mapping[str, str], pattern: str = PATTERN,
                      delimiter: str = r"\s+", out_dir: Optional[str] = None) -> List[str]:
    """Write syst_<table> for every table of `directory` matching `pattern` (into out_dir,
    default the directory) and return the written paths in ascending emin."""
    if not columns:
        raise ValueError("need at least one systematic column")
    paths = table_files(directory, pattern)
    out_dir = out_dir or directory
    os.makedirs(out_dir, exist_ok=True)
    written = []
    for path, (table, chunk) in zip(paths, column_chunks(paths, delimiter, columns)):
        ncol = table.shape[1]
        table = table.copy()
        table[SYST_HEADER] = pd.Series(total_systematic(chunk)).map("{:.3f}".format)
        order = [0, 1, 2, ncol] + list(range(3, ncol))
        out = os.path.join(out_dir, "syst_" + os.path.basename(path))
        table.iloc[:, order].to_csv(out, index=False, sep=" ")
        written.append(out)
    return written


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=(__doc__ or "").splitlines()[0])
    parser.add_argument("directory", help="directory of the weighted tables")
    parser.add_argument("--column", action="append", required=True, metavar="NAME=FILE",
                        help=f"a systematic: the last column of FILE, or {SCALE_FACTOR} "
                             "(repeat; summed in quadrature in this order)")
    parser.add_argument("--pattern", default=PATTERN)
    parser.add_argument("--delimiter", default=r"\s+")
    parser.add_argument("--out-dir", default=None, help="where syst_<table> goes (default: the directory)")
    args = parser.parse_args(argv)
    columns: Dict[str, str] = {}
    for item in args.column:
        name, sep, path = item.partition("=")
        if not sep or not name or not path:
            parser.error(f"--column {item!r}: need NAME=FILE or NAME={SCALE_FACTOR}")
        columns[name] = path
    try:
        written = write_syst_tables(args.directory, columns, args.pattern, args.delimiter, args.out_dir)
    except (OSError, ValueError) as err:
        print(f"gxana_xsection.syst_tables: error: {err}", file=sys.stderr)
        return 1
    for path in written:
        print(f"wrote {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
