import numpy as np
import os
import glob
import fnmatch

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
    weighted_avg = np.sum(weights * y_values, axis=0) / np.sum(weights, axis=0)
    weighted_error = 1 / np.sqrt(np.sum(weights, axis=0))
    return weighted_avg, weighted_error

def get_files_from_directory(directory_path, file_extension="*.txt", pattern=None):
    """
    Retrieves files with the specified extension from the directory that match the wildcard pattern.

    Parameters:
    - directory_path: Path to the directory containing files.
    - file_extension: File extension to filter (default is "*.txt").
    - pattern: A wildcard pattern to match filenames (e.g., "experiment_*.txt").

    Returns:
    - List of file paths that match the pattern.
    """
    all_files = glob.glob(os.path.join(directory_path, file_extension))
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

    Parameters:
    - input_files: List of input file paths.
    - output_dir: Directory to save the output file.

    Returns:
    - The dynamically generated output file name.
    """
    basenames = [os.path.basename(f) for f in input_files]
    # Find common prefix and suffix
    common_prefix = os.path.commonprefix(basenames).rsplit("_")[0]#only keep first part
    common_suffix = find_common_suffix(basenames).lstrip("_").replace(".txt","")#make sure extension not printed twice
    print(f"Common_suffix {common_suffix}")
    # Construct the filename
    if not common_prefix:
        common_prefix = "output"
    output_name = f"weighted_{common_prefix}_{common_suffix}.txt" if common_suffix else f"{common_prefix}_weighted_output.txt"
    return os.path.join(output_dir, output_name)

def main(directory_path, output_dir, pattern=None):
    """
    Reads data from filtered text files in a directory, calculates the statistically weighted average of Y,
    and outputs the new dataset with the same X and EX.
    """
    file_paths = get_files_from_directory(directory_path, pattern=pattern)
    
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
    weighted_avg_y, weighted_error_y = calculate_weighted_average(y_values, y_errors)

    # Create the output filename dynamically
    output_file = create_output_filename(file_paths, output_dir)

    # Save the new dataset
    new_data = np.vstack([x_values, weighted_avg_y, ex_values, weighted_error_y]).T
    np.savetxt(output_file, new_data, header="X Y_weighted EX EY_weighted", fmt="%.6f")
    
    print(f"Processed {len(file_paths)} files.")
    print(f"Processed: {file_paths[0]}")
    print(f"Processed: {file_paths[1]}")
    print(f"New weighted data saved to: {output_file}")

if __name__ == "__main__":
    # Directory containing the text files
    directory_path = "./xsection_data"  # Change to your directory path
    nominal_directory_path = "../xsection/ml_fits/data/gen_amp_V2_2D_ac/hybrid_combo" 
    output_dir = "./weighted_data"  # Directory to save the output file

    # Ensure the output directory exists
    os.makedirs(output_dir, exist_ok=True)

    # Define a wildcard pattern for filtering filenames
    # Example: Files matching "experiment_*.txt"
    variations = ["chisqndf_6","chisqndf_7",
                  "chisqndf_9", "chisqndf_10",
                  "total_mm2_abs_0.01","total_mm2_abs_0.015",
                  "total_mm2_abs_0.025","total_mm2_abs_0.03",
                  "xim_pathlensig_1","xim_pathlensig_1.5",
                  "xim_pathlensig_2.5","xim_pathlensig_3",
                  "lambda_pathlensig_0.5", "lambda_pathlensig_1",
                  "kphigh_prap_1.6","kphigh_prap_1.8",
                  "kphigh_prap_2.1","kphigh_prap_2.2",
                ];
    enBins = ["6.40","7.40","7.86","8.19","8.45","8.68","9.26","10.18"]
    for var in variations:
        tot_pattern = "totxsec*"+ var + ".txt";
        main(directory_path, output_dir, pattern=tot_pattern)
        for ebin in enBins:
            diff_pattern = "diffxsec*" + var + "_emin_" + ebin + "*"
            main(directory_path, output_dir, pattern=diff_pattern)

    # Get nominal weighted xsec too
    nom_tot_pattern = "totxsec*kphighrap*"
    main(nominal_directory_path, output_dir, pattern=nom_tot_pattern)
    for ebin in enBins:
        nom_diff_pattern = "diffxsec*kphighrap*" + "Emin-" + ebin + "*"
        main(nominal_directory_path, output_dir, pattern=nom_diff_pattern)
            
