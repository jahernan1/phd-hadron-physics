"""Error-weighted average of cross-section tables over the three run periods.

Port of AnalysisNote/xsection/GetWeightedXsecFile.py (functions unchanged
except: file lists are sorted, and the three-file check raises ValueError).
"""
from __future__ import annotations

import fnmatch
import glob
import os
from typing import List, Optional

import numpy as np

HEADER = r"-t d\sigma/dt \delta_x \delta_y S"


def read_data(file_path):
    """
    Reads data from a text file. Assumes the file has four columns: X, Y, EX, EY.
    """
    data = np.loadtxt(file_path, unpack=True, skiprows=1)
    if data.shape[0] != 4:
        raise ValueError(f"File {file_path} must have exactly 4 columns: X, Y, EX, EY")
    return data


def calculate_weighted_average(y_values, y_errors):
    """
    Calculates the statistically weighted average and its error.
    """
    weights = 1 / (y_errors ** 2)
    weights[np.isinf(weights)] = 0  # if we have zero values due to stat
    weighted_avg = np.sum(weights * y_values, axis=0) / np.sum(weights, axis=0)
    weighted_error = 1 / np.sqrt(np.sum(weights, axis=0))
    chisquare = np.sum(weights * pow(weighted_avg - y_values, 2), axis=0)
    nValues = np.count_nonzero(weights, axis=0)  # get nummber of data sets combined
    scale_factor = np.sqrt(chisquare / (nValues - 1))

    return weighted_avg, weighted_error, scale_factor


def get_files_from_directory(directory_path, file_extension="*.txt", pattern=None) -> List[str]:
    """
    Retrieves files with the specified extension from the directory that match the wildcard pattern.
    """
    all_files = sorted(glob.glob(os.path.join(directory_path, file_extension)))
    if pattern:
        filtered_files = [f for f in all_files if fnmatch.fnmatch(os.path.basename(f), pattern)]
    else:
        filtered_files = all_files

    if not filtered_files:
        raise FileNotFoundError(f"No files matching pattern '{pattern}' found in directory: {directory_path}")
    return filtered_files


def find_common_suffix(filenames):
    """
    Finds the common suffix in a list of filenames.
    """
    reversed_names = [name[::-1] for name in filenames]
    common_reversed_suffix = os.path.commonprefix(reversed_names)
    return common_reversed_suffix[::-1]  # Reverse it back to the correct order


def create_output_filename(input_files, output_dir):
    """
    Creates a dynamic output filename based on the input files' common prefix and suffix.
    """
    basenames = [os.path.basename(f) for f in input_files]
    # Find common prefix and suffix
    common_prefix = os.path.commonprefix(basenames).rsplit("_")[0]  # only keep first part
    common_suffix = find_common_suffix(basenames).lstrip("_").replace(".txt", "")  # make sure extension not printed twice
    print(f"Common_suffix {common_suffix}")
    # Construct the filename
    if not common_prefix:
        common_prefix = "output"
    output_name = f"weighted_{common_prefix}_{common_suffix}.txt" if common_suffix else f"{common_prefix}_weighted_output.txt"
    return os.path.join(output_dir, output_name)


def weight_files(directory_path: str, output_dir: str, pattern: Optional[str] = None) -> str:
    """
    Reads the three run-period tables matching pattern, calculates the statistically
    weighted average of Y, and writes X, Y, EX, EY, S to output_dir. Returns the output path.
    """
    file_paths = get_files_from_directory(directory_path, pattern=pattern)
    if len(file_paths) != 3:
        raise ValueError(f"expected 3 run-period files matching {pattern!r} in {directory_path}, "
                         f"found {len(file_paths)}")

    x_values = None
    ex_values = None
    y_values = []
    y_errors = []

    for file_path in file_paths:
        x, y, ex, ey = read_data(file_path)
        if x_values is None:
            x_values = x
            ex_values = ex
        else:
            # Check if all X and EX values match
            if not np.allclose(x_values, x) or not np.allclose(ex_values, ex):
                raise ValueError(f"X or EX values do not match in file: {file_path}")

        y_values.append(y)
        y_errors.append(ey)

    # Combine all data
    y_values = np.array(y_values)
    y_errors = np.array(y_errors)

    # Calculate weighted average for each X
    weighted_avg_y, weighted_error_y, scale_factor = calculate_weighted_average(y_values, y_errors)

    # Create the output filename dynamically
    output_file = create_output_filename(file_paths, output_dir)

    # Save the new dataset
    new_data = np.vstack([x_values, weighted_avg_y, ex_values, weighted_error_y, scale_factor]).T
    np.savetxt(output_file, new_data, header=HEADER, fmt="%.6f", comments='')

    print(f"Processed {len(file_paths)} files.")
    print(f"New weighted data saved to: {output_file}")
    return output_file


def _build_arg_parser():
    import argparse

    parser = argparse.ArgumentParser(description="Error-weighted average of cross-section tables.")
    parser.add_argument("directory")
    parser.add_argument("output_dir")
    parser.add_argument("--pattern", default=None)
    return parser


def main(argv=None):
    args = _build_arg_parser().parse_args(argv)
    weight_files(args.directory, args.output_dir, pattern=args.pattern)


if __name__ == "__main__":
    main()
