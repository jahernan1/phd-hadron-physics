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
    weights[np.isinf(weights)] = 0 #if we have zero values due to stat
    weighted_avg = np.sum(weights * y_values, axis=0) / np.sum(weights, axis=0)
    weighted_error = 1 / np.sqrt(np.sum(weights, axis=0))
    chisquare = np.sum(weights * pow(weighted_avg - y_values, 2), axis=0 )
    nValues = np.count_nonzero(weights, axis=0)#get nummber of data sets combined
    scale_factor = np.sqrt(chisquare / (nValues - 1) )
        
    return weighted_avg, weighted_error, scale_factor

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
    assert len(file_paths)==3,"[Error]: Taking more than 3 files."
    
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

    #print(weighted_error_y)
    # Scale the weighted average of y by scale factor if > 1
    # scale_condition = scale_factor > 1
    # weighted_error_y[scale_condition] *= scale_factor[scale_condition]
    
    # Create the output filename dynamically
    output_file = create_output_filename(file_paths, output_dir)

    # Save the new dataset
    new_data = np.vstack([x_values, weighted_avg_y, ex_values, weighted_error_y, scale_factor]).T
    np.savetxt(output_file, new_data, header="-t d\sigma/dt \delta_x \delta_y S", fmt="%.6f", comments='')
    
    print(f"Processed {len(file_paths)} files.")
    print(f"New weighted data saved to: {output_file}")

if __name__ == "__main__":
    # Directory containing the text files
    directory_path = "./data"  # Change to your directory path
    output_dir = "./weighted_data"  # Directory to save the output file
    acc_type = ["/acc_weight","/best_combo","/hybrid_combo", "/johnson", "/johnson_cheby1", "/voigt", "/voigt_cheby1", "/gaus", "/gaus_cheby1", "/mcPdf", "/mcPdf_cheby1","/oneRfBunch", "/s17_rest3", "/oneEBin"]

    # Define a wildcard pattern for filtering filenames
    enBins = ["6.40","7.40","7.86","8.19","8.45","8.68","9.26","10.18"]

    for acc in acc_type:
        tot_pattern = "totxsec*.txt"
        # Ensure the output directory exists
        os.makedirs(output_dir+acc, exist_ok=True)
        main(directory_path+acc, output_dir+acc, pattern=tot_pattern)
        
        for ebin in enBins:
            if(acc=="/s17_rest3" or acc=="/oneEBin"):
                break
            else:
                diff_pattern = "diffxsec*_emin_" + ebin + "*.txt"
                print(diff_pattern)
                main(directory_path+acc, output_dir+acc, pattern=diff_pattern)
                main(directory_path+"/qvalues", output_dir+"/qvalues",pattern=diff_pattern)
                
    main(directory_path+"/s17_rest3", output_dir+"/s17_rest3",pattern="diffxsec*.txt")
    main(directory_path+"/oneEBin", output_dir+"/oneEBin",pattern="diffxsec*.txt")
    
    
