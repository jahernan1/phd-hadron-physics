"""Split yield/acceptance/flux tables into one file per quantity.

Port of AnalysisNote/xsection/GetXSecComponentFiles.py; the name anchor
("kpkpxim") that starts each output file name is now a parameter.

Deviation: the anchor is searched in the file's basename, not the full path,
so a directory named like the anchor (e.g. gluex_analysis_data/kpkpxim/)
cannot match; identical output for every legacy invocation.
"""
from __future__ import annotations

import os

import pandas as pd

from gxana_xsection.weighted_average import get_files_from_directory


def split_text_file(input_file, output_dir, anchor="kpkpxim"):
    """
    Splits a text file into multiple files based on y-value and ey-value pairs.

    The input file should have a header row with columns: [x, ex, y1, ey1, y2, ey2, ...].
    Each output file has columns [x, y, ex, ey] and is named <y column>_<input name from anchor>.txt.
    """
    # Read the input file into a pandas DataFrame
    try:
        data = pd.read_csv(input_file, sep=r"\s+")
    except Exception as e:
        raise ValueError(f"Error reading file: {e}")

    # Ensure the column structure matches the expected pattern
    column_names = data.columns.tolist()
    print(column_names)
    if len(column_names) < 3 or (len(column_names) - 2) % 2 != 0:
        raise ValueError("Invalid file structure. There must be an equal number of y and ey columns after the first two columns.")

    # Identify y and ey column pairs
    x_col, ex_col = column_names[:2]
    value_columns = column_names[2:]
    num_y_columns = len(value_columns) // 2

    # Extract the base name of the input file (without extension)
    base_name = os.path.basename(input_file).rsplit('.', 1)[0]
    base_name = base_name[base_name.find(anchor):]

    # Split the data into multiple files
    for i in range(num_y_columns):
        y_col = value_columns[i * 2]
        ey_col = value_columns[i * 2 + 1]
        output_data = data[[x_col, y_col, ex_col, ey_col]]

        print(f"Output for {y_col} and {ey_col}:")
        print(output_data)
        print("---")

        # Save the output file with a prefix based on the y column name
        output_file = os.path.join(output_dir, f"{y_col}_{base_name}.txt")
        # Write the comment and DataFrame to CSV
        with open(output_file, 'w') as f:
            f.write('#')
            output_data.to_csv(f, sep=" ", index=False, header=True)
        print(f"Created file: {output_file}")


def split_files(directory_path, output_dir, pattern=None, anchor="kpkpxim"):
    """
    Splits every text file in directory_path that matches pattern (legacy main()).
    """
    file_paths = get_files_from_directory(directory_path, pattern=pattern)

    for file_path in file_paths:
        split_text_file(file_path, output_dir, anchor)


def _build_arg_parser():
    import argparse

    parser = argparse.ArgumentParser(description="Split yield/acceptance/flux tables into one file per quantity.")
    parser.add_argument("directory")
    parser.add_argument("output_dir")
    parser.add_argument("--pattern", default=None)
    parser.add_argument("--anchor", default="kpkpxim")
    return parser


def main(argv=None):
    args = _build_arg_parser().parse_args(argv)
    split_files(args.directory, args.output_dir, pattern=args.pattern, anchor=args.anchor)


if __name__ == "__main__":
    main()
