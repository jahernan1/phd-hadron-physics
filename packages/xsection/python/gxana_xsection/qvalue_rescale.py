"""Rescale dsigma/dt by the Q-value yield ratio.

Port of AnalysisNote/xsection/MakeQValXSecFile.py: dsigma/dt from the fitted
yield is multiplied by qval_yield / data_yield taken from the matching
diffout table.
"""
from __future__ import annotations

import pandas as pd


def process_files(file1, col1_file1, col2_file1, file2, output_file):
    """
    Calculate the ratio col2/col1 from file1 and multiply the second column of file2 by it.
    """
    # Read the first file
    df1 = pd.read_csv(file1, sep=r"\s+")

    if col1_file1 not in df1.columns or col2_file1 not in df1.columns:
        raise ValueError(f"columns {col1_file1!r} and {col2_file1!r} must both be in {file1}")

    # Ratio qval_yield / data_yield per row. A bin the fit gated out (too few
    # entries) has data_yield 0 and dsigma/dt 0; the legacy script divided by
    # it (inf/NaN in the output). Its ratio is set to 0 so the rescaled value
    # stays 0. Rows with a non-zero data_yield are unchanged.
    denominator = df1[col1_file1].astype(float)
    ratio = df1[col2_file1].astype(float) / denominator.where(denominator != 0, float("nan"))
    df1['ratio'] = ratio.fillna(0.0)

    # Read the second file
    df2 = pd.read_csv(file2, sep=r"\s+")

    # Multiply the first column by the corresponding ratio
    first_column_name = df2.columns[1]
    if len(df1['ratio']) != len(df2):
        raise ValueError("Mismatch in the number of rows between the ratio column and the second file.")

    df2[first_column_name] = df2[first_column_name] * df1['ratio'].values

    # Save the updated file
    df2.to_csv(output_file, sep=" ", index=False)
    print(f"Output saved to {output_file}")


def _build_arg_parser():
    import argparse

    parser = argparse.ArgumentParser(description="Rescale dsigma/dt by the Q-value yield ratio.")
    parser.add_argument("file1")
    parser.add_argument("col1_file1")
    parser.add_argument("col2_file1")
    parser.add_argument("file2")
    parser.add_argument("output_file")
    return parser


def main(argv=None):
    args = _build_arg_parser().parse_args(argv)
    process_files(args.file1, args.col1_file1, args.col2_file1, args.file2, args.output_file)


if __name__ == "__main__":
    main()
