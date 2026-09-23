"""Rescale dsigma/dt by the Q-value yield ratio.

Port of AnalysisNote/xsection/MakeQValXSecFile.py: dsigma/dt from the fitted
yield is multiplied by qval_yield / data_yield taken from the matching
diffout table.
"""
from __future__ import annotations

import glob
import os

import pandas as pd


def process_files(file1, col1_file1, col2_file1, file2, output_file):
    """
    Calculate the ratio col2/col1 from file1 and multiply the second column of file2 by it.
    """
    # Read the first file
    df1 = pd.read_csv(file1, sep=r"\s+")

    # Check for zero values in the denominator column
    if col2_file1 not in df1.columns or df1[col2_file1].eq(0).any():
        raise ValueError("Denominator column contains zero or is not found in the first file.")

    # Calculate the ratio for each element
    df1['ratio'] = df1[col2_file1] / df1[col1_file1]

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


def process_files_in_directory(directory, file1_pattern, file2_pattern, output_directory,
                               col1_file1="data_yield", col2_file1="qval_yield"):
    """
    Process all file pairs in a directory; pairs are the sorted matches of the two patterns.
    """
    # Ensure the output directory exists
    os.makedirs(output_directory, exist_ok=True)

    # Get matching files
    file1_list = sorted(glob.glob(os.path.join(directory, file1_pattern)))
    file2_list = sorted(glob.glob(os.path.join(directory, file2_pattern)))

    if len(file1_list) != len(file2_list):
        raise ValueError("Mismatch in the number of files matching the patterns.")

    # Process each pair of files
    for file1, file2 in zip(file1_list, file2_list):
        print(file1)
        print(file2)
        output_file = os.path.join(output_directory, f"{os.path.basename(file2)}")
        process_files(file1, col1_file1, col2_file1, file2, output_file)
