"""Build LaTeX cross-section tables from per-bin data files.

Port of AnalysisNote/xsection/MakeXsecTexTable{,1,Scale}.py, merged into a
single ``process_files_to_latex``:

- ``additional_files=None`` reproduces MakeXsecTexTable1.py: a plain 4-column
  table (no point-by-point systematics), empty caption, and no \\multirow
  markup on the energy-bin column.
- ``additional_files`` given, ``systematic_source="run_fraction"`` (default)
  reproduces MakeXsecTexTable.py: the "Run Combination" systematic is 5.1% of
  column 2 (dσ/dt), the quadrature systematics column is computed once
  after all files are read, per-file output is ``processed_<filename>`` (the
  reformatted table columns, unformatted "Run Combination"/Accidentals/Yield
  Extraction values).
- ``additional_files`` given, ``systematic_source="scale_factor"`` reproduces
  MakeXsecTexTableScale.py: the "Run Combination" systematic is
  ``col4 * col5 - col4`` clipped to 0 where col5 < 1 (col5 is the run-scale
  factor), the quadrature systematics column is computed per file (chunked
  over the additional files' rows), the systematics table gets an extra
  multicolumn header line, Accidentals/Yield Extraction/Run Combination are
  written "{:.3f}"-formatted, and the per-file output is ``syst_<filename>``
  (the original columns reordered/augmented with the syst column instead of
  the reformatted table columns).
"""
from __future__ import annotations

import os
import re
import sys
from glob import glob
from typing import Any, Dict, cast

import numpy as np
import pandas as pd

from gxana_xsection import syst_tables


def _finish_latex_table(latex_table, extra_after_toprule=None, rows_per_block=7):
    """Insert \\hline after every 7th row of a \\multirow block (and, when
    given, one extra header line right after \\toprule)."""
    lines = latex_table.split("\n")
    formatted = []
    inside_multirow = False
    row_count = 0
    for line in lines:
        formatted.append(line)
        if "\\multirow" in line:
            row_count = 0
            inside_multirow = True
        if inside_multirow:
            row_count += 1
        if inside_multirow and row_count == rows_per_block:
            formatted.append("\\hline")
            inside_multirow = False
        if extra_after_toprule is not None and "\\toprule" in line:
            formatted.append(extra_after_toprule)
    return "\n".join(formatted)


def process_files_to_latex(directory, pattern, delimiter, output_file,
                            additional_files=None, systematic_source="run_fraction",
                            columns=None, run_fraction=0.051):
    """
    Processes input files, reformats their data, and creates a LaTeX-compatible table.

    Parameters:
        directory (str): Directory containing the input files.
        pattern (str): File pattern to match (e.g., '*.txt').
        delimiter (str): Delimiter used in the input files (e.g., ',' or '\\t').
        output_file (str): Path to save the concatenated LaTeX table.
        additional_files (list | None): List of additional files from which the last
            column supplies the values used to build the point-by-point systematics
            table/column. None reproduces MakeXsecTexTable1.py (see module docstring).
        systematic_source (str): "run_fraction" (default, MakeXsecTexTable.py) or
            "scale_factor" (MakeXsecTexTableScale.py); only used when additional_files
            is given. See module docstring for the exact behavioral difference.
        columns (dict | None): ordered mapping column name -> stats file; replaces
            additional_files/systematic_source: every systematic column, including Run
            Combination, is the last column of its file (gxana run systematics); the
            legacy branches stay byte-compatible with the legacy scripts.
        run_fraction (float): "run_fraction" source only: Run Combination is this
            fraction of column 2 (dsigma/dt).

    Returns:
        str: the primary LaTeX table text written to output_file.
    """
    try:
        file_paths = glob(os.path.join(directory, pattern))
        if not file_paths:
            raise FileNotFoundError("No files matching the pattern were found in the directory.")

        file_paths.sort(key=lambda x: float(cast(re.Match, re.search(r"\d+\.\d+", os.path.basename(x))).group()))

        if columns is not None:
            return _columns_table(file_paths, delimiter, output_file, columns)

        output_dfs = []
        syst_dfs = []
        run_syst = []

        additional_dfs: Any = None  # a list of DataFrames once additional_files is given; the branches below that index it require that
        if additional_files:
            additional_dfs = [pd.read_csv(file, delimiter=delimiter) for file in additional_files]

        for index, file_path in enumerate(file_paths):
            filename = os.path.basename(file_path)
            match = re.findall(r"\d+\.\d+", filename)
            if len(match) < 2:
                raise ValueError(f"Filename {filename} does not contain two numerical values in the format '6.40'.")
            x, y = map(float, match[:2])

            df = pd.read_csv(file_path, delimiter=delimiter)
            num_rows = len(df)

            col1 = df.iloc[:, 0]
            col3 = df.iloc[:, 2]
            new_column_2 = [(f"{c1 - c3:.2f}", f"{c1 + c3:.2f}") for c1, c3 in zip(col1, col3)]

            if additional_files is None:
                new_column_1 = f"({x:.2f}, {y:.2f})"
                formatted_df = pd.DataFrame({
                    "$E_\\gamma\\ (\\text{GeV})$": [new_column_1] + [""] * (len(df) - 1),
                    "$-t\\ (\\text{GeV}^2)$": [f"({x}, {y})" for x, y in new_column_2],
                    "$d\\sigma/dt\\ (\\text{nb/GeV}^2)$": df.iloc[:, 1].map("{:.3f}".format),
                    "$\\delta y (stat)$": df.iloc[:, 3].map("{:.3f}".format),
                })

                output_file_name = os.path.join(directory, f"processed_{filename}")
                formatted_df.to_csv(output_file_name, index=False)
                print(f"Processed file saved: {output_file_name}")

                output_dfs.append(formatted_df)
                continue

            multirow_value = f"\\multirow{{{num_rows}}}{{*}}{{({x:.2f}, {y:.2f})}}"
            first_column = [multirow_value] + [""] * (num_rows - 1)

            if systematic_source == "scale_factor":
                scale_condition = df.iloc[:, 4] < 1
                scale_sys = df.iloc[:, 3] * df.iloc[:, 4] - df.iloc[:, 3]
                scale_sys[scale_condition] = 0
                run_syst.extend(scale_sys)

                index_start = index * num_rows
                quadrature_column = cast(pd.Series, np.sqrt(
                    sum(adf.iloc[index_start:index_start + num_rows, -1] ** 2 for adf in additional_dfs)
                    + np.array(scale_sys) ** 2
                )).map("{:.3f}".format)
                quadrature_column = quadrature_column.reset_index(drop=True)
                print(quadrature_column)

                formatted_df = pd.DataFrame({
                    "$E_\\gamma\\ (\\text{GeV})$": first_column,
                    "$-t\\ (\\text{GeV}^2)$": [f"({x}, {y})" for x, y in new_column_2],
                    "$d\\sigma/dt\\ (\\text{nb/GeV}^2)$": df.iloc[:, 1].map("{:.3f}".format),
                    "$\\delta y$ ({\\it stat})": df.iloc[:, 3].map("{:.3f}".format),
                    "$\\delta y$ ({\\it syst})": quadrature_column,
                })

                syst_df = pd.DataFrame({
                    "$E_\\gamma\\ (\\text{GeV})$": first_column,
                    "$-t\\ (\\text{GeV}^2)$": [f"({x}, {y})" for x, y in new_column_2],
                    "Run Combination": scale_sys.map("{:.3f}".format),
                })

                df_syst = df
                df_syst["\\delta_y_syst"] = quadrature_column
                print(df_syst)
                df_syst = df.iloc[:, [0, 1, 2, 5, 3, 4]]

                output_file_name = os.path.join(directory, f"syst_{filename}")
                df_syst.to_csv(output_file_name, index=False, sep=" ")
                print(f"Processed file saved: {output_file_name}")
            else:
                additional_col = df.iloc[:, 1] * run_fraction
                run_syst.extend(additional_col)

                formatted_df = pd.DataFrame({
                    "$E_\\gamma\\ (\\text{GeV})$": first_column,
                    "$-t\\ (\\text{GeV}^2)$": [f"({x}, {y})" for x, y in new_column_2],
                    "$d\\sigma/dt\\ (\\text{nb/GeV}^2)$": df.iloc[:, 1].map("{:.3f}".format),
                    "$\\delta y$ ({\\it stat})": df.iloc[:, 3].map("{:.3f}".format),
                })

                syst_df = pd.DataFrame({
                    "$E_\\gamma\\ (\\text{GeV})$": first_column,
                    "$-t\\ (\\text{GeV}^2)$": [f"({x}, {y})" for x, y in new_column_2],
                })

                output_file_name = os.path.join(directory, f"processed_{filename}")
                formatted_df.to_csv(output_file_name, index=False)
                print(f"Processed file saved: {output_file_name}")

            output_dfs.append(formatted_df)
            syst_dfs.append(syst_df)

        combined_df = pd.concat(output_dfs, ignore_index=True)

        if additional_files is None:
            latex_table = combined_df.to_latex(
                index=False, escape=False, multicolumn=True, multirow=True,
                longtable=True, caption="", label="tab:diffxsec",
                column_format="c|c|cc", sparsify=False,
            )
            with open(output_file, "w") as f:
                f.write(latex_table)
            print(f"Concatenated LaTeX table saved to {output_file}")
            return latex_table

        combined_syst_df = pd.concat(syst_dfs, ignore_index=True)

        if systematic_source == "scale_factor":
            combined_syst_df["Accidentals"] = additional_dfs[0].iloc[:, -1].map("{:.3f}".format)
            combined_syst_df["Yield Extraction"] = additional_dfs[1].iloc[:, -1].map("{:.3f}".format)
        else:
            if additional_dfs:
                quadrature_column = cast(pd.Series, np.sqrt(
                    sum(adf.iloc[:, -1] ** 2 for adf in additional_dfs) + np.array(run_syst) ** 2
                )).map("{:.3f}".format)
                combined_df["$\\delta y$ ({\\it syst})"] = quadrature_column
            else:
                combined_df["$\\delta y$ ({\\it syst})"] = ""
            combined_syst_df["Accidentals"] = additional_dfs[0].iloc[:, -1]
            combined_syst_df["Yield Extraction"] = additional_dfs[1].iloc[:, -1]
            combined_syst_df["Run Combination"] = run_syst

        latex_table = combined_df.to_latex(
            index=False, escape=False, multicolumn=True, multirow=True, longtable=True,
            caption="Table of the differential cross section for the \\GlueXI data.",
            label="tab:diffxsec", column_format="c|c|ccc", sparsify=False,
        )
        latex_table_syst = combined_syst_df.to_latex(
            index=False, escape=False, multicolumn=True, multirow=True, longtable=True,
            caption="Table of the point-by-point systematics of the differential cross section for the \\GlueXI data.",
            label="tab:diffxsec", column_format="c|c|ccc", sparsify=False,
        )

        formatted_table = _finish_latex_table(latex_table)
        with open(output_file, "w") as f:
            f.write(formatted_table)
        print(f"Concatenated LaTeX table saved to {output_file}")

        extra_header = None
        if systematic_source == "scale_factor":
            extra_header = "& & \\multicolumn{3}{c}{Systematic Source (nb/GeV${}^2$)} \\\\ \\cline{3-5}"
        formatted_table_syst = _finish_latex_table(latex_table_syst, extra_after_toprule=extra_header)
        # gxana: legacy hardcodes open("syst_" + output_file), which breaks
        # (or lands in the wrong place) for any output_file containing a
        # directory (e.g. "out/t.tex" -> "syst_out/t.tex"); write next to
        # output_file's own directory instead. Byte-identical to legacy for
        # a bare filename, where dirname(output_file) is "".
        syst_output_file = os.path.join(os.path.dirname(output_file), "syst_" + os.path.basename(output_file))
        with open(syst_output_file, "w") as f:
            f.write(formatted_table_syst)
        print(f"Concatenated LaTeX table saved to {syst_output_file}")

        return formatted_table

    except Exception as e:
        print(f"Error: {e}")
        return None


def _columns_table(file_paths, delimiter, output_file, columns):
    """Dissertation table with every systematic column read from a stats file
    (last column), named by its key, or the scale-factor run systematic
    (syst_tables.SCALE_FACTOR): fixes the legacy Accidentals/Yield Extraction
    swap (docs/KNOWN_ISSUES.md) and takes Run Combination from sfactor_stats.txt."""
    names = list(columns)
    output_dfs, syst_dfs = [], []
    for file_path, (df, chunk) in zip(file_paths, syst_tables.column_chunks(file_paths, delimiter, columns)):
        filename = os.path.basename(file_path)
        x, y = map(float, re.findall(r"\d+\.\d+", filename)[:2])
        n = len(df)
        first_column = [f"\\multirow{{{n}}}{{*}}{{({x:.2f}, {y:.2f})}}"] + [""] * (n - 1)
        t_bins = [f"({c1 - c3:.2f}, {c1 + c3:.2f})" for c1, c3 in zip(df.iloc[:, 0], df.iloc[:, 2])]
        total = syst_tables.total_systematic(chunk)
        output_dfs.append(pd.DataFrame({
            "$E_\\gamma\\ (\\text{GeV})$": first_column,
            "$-t\\ (\\text{GeV}^2)$": t_bins,
            "$d\\sigma/dt\\ (\\text{nb/GeV}^2)$": df.iloc[:, 1].map("{:.3f}".format),
            "$\\delta y$ ({\\it stat})": df.iloc[:, 3].map("{:.3f}".format),
            "$\\delta y$ ({\\it syst})": pd.Series(total).map("{:.3f}".format),
        }))
        syst: Dict[str, Any] = {"$E_\\gamma\\ (\\text{GeV})$": first_column, "$-t\\ (\\text{GeV}^2)$": t_bins}
        for name in names:
            syst[name] = pd.Series(chunk[name]).map("{:.3f}".format)
        syst_dfs.append(pd.DataFrame(syst))
    combined, combined_syst = pd.concat(output_dfs, ignore_index=True), pd.concat(syst_dfs, ignore_index=True)
    latex = combined.to_latex(
        index=False, escape=False, multicolumn=True, multirow=True, longtable=True,
        caption="Table of the differential cross section for the \\GlueXI data.",
        label="tab:diffxsec", column_format="c|c|ccc", sparsify=False)
    ncol = len(names)
    latex_syst = combined_syst.to_latex(
        index=False, escape=False, multicolumn=True, multirow=True, longtable=True,
        caption="Table of the point-by-point systematics of the differential cross section for the \\GlueXI data.",
        label="tab:diffxsec", column_format="c|c|" + "c" * ncol, sparsify=False)
    formatted = _finish_latex_table(latex)
    with open(output_file, "w") as f:
        f.write(formatted)
    header = f"& & \\multicolumn{{{ncol}}}{{c}}{{Systematic Source (nb/GeV${{}}^2$)}} \\\\ \\cline{{3-{ncol + 2}}}"
    syst_output_file = os.path.join(os.path.dirname(output_file), "syst_" + os.path.basename(output_file))
    with open(syst_output_file, "w") as f:
        f.write(_finish_latex_table(latex_syst, extra_after_toprule=header))
    print(f"Concatenated LaTeX table saved to {output_file}")
    print(f"Concatenated LaTeX table saved to {syst_output_file}")
    return formatted


def _build_arg_parser():
    import argparse

    parser = argparse.ArgumentParser(description=(__doc__ or "").splitlines()[0])
    parser.add_argument("directory")
    parser.add_argument("pattern")
    parser.add_argument("output_file")
    parser.add_argument("--delimiter", default=r"\s+")
    parser.add_argument("--additional", nargs="*", dest="additional_files", default=None)
    parser.add_argument("--systematic-source", default="run_fraction",
                         choices=["run_fraction", "scale_factor"])
    parser.add_argument("--column", action="append", default=None, metavar="NAME=FILE",
                        help="systematic column from the last column of FILE (repeat; replaces --additional)")
    parser.add_argument("--run-fraction", type=float, default=0.051,
                        help="run_fraction source: Run Combination = this fraction of dsigma/dt "
                             "(no documentary source; *_runsyst study tables only)")
    return parser


def main(argv=None):
    args = _build_arg_parser().parse_args(argv)
    columns = None
    if args.column:
        columns = {}
        for item in args.column:
            name, _, path = item.partition("=")
            columns[name] = path
    result = process_files_to_latex(args.directory, args.pattern, args.delimiter, args.output_file,
                                     additional_files=args.additional_files,
                                     systematic_source=args.systematic_source,
                                     columns=columns, run_fraction=args.run_fraction)
    return 0 if result is not None else 1


if __name__ == "__main__":
    sys.exit(main())
