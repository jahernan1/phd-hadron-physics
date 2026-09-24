import numpy as np
import pandas as pd
import os
import glob
import fnmatch

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

def split_text_file(input_file, output_dir):
    """
    Splits a text file into multiple files based on y-value and ey-value pairs.

    Parameters:
        input_file (str): Path to the input text file.

    The input file should have a header row with columns: [x, ex, y1, ey1, y2, ey2, ...].
    Each output file will have columns: [x, ex, y, ey] and will be saved with a prefix based on the processed column.
    """
    # Read the input file into a pandas DataFrame
    try:
        data = pd.read_csv(input_file, sep="\s+")
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
    base_name = input_file.rsplit('.', 1)[0]
    base_name = base_name[base_name.find("kpkpxim"):]

    # Split the data into multiple files
    for i in range(num_y_columns):
        y_col = value_columns[i * 2]
        ey_col = value_columns[i * 2 + 1]
        output_data = data[[x_col, y_col, ex_col, ey_col]]

         # Print the output data
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

def main(directory_path, output_dir, pattern=None):
    """
    Reads data from filtered text files in a directory, calculates the statistically weighted average of Y,
    and outputs the new dataset with the same X and EX.
    """
    file_paths = get_files_from_directory(directory_path, pattern=pattern)

    for file_path in file_paths:
        split_text_file(file_path, output_dir)

if __name__ == "__main__":
    # Directory containing the text files
    directory_path = "./data"  # Change to your directory path
    output_dir = "./components"  # Directory to save the output file
    data_dir = ["/sp17", "/sp18", "/fa18"]
    acc_dir = ["/acc_weight","/best_combo","/hybrid_combo","/johnson","/mcPdf", "/mcPdf_cheby1"]
    # Define a wildcard pattern for filtering filenames
    enBins = ["6.40","7.40","7.86","8.19","8.45","8.68","9.26","10.18"]

    for data in data_dir:
        for acc in acc_dir:
            output_dir_final = output_dir+data+acc
            # Ensure the output directory exists
            os.makedirs(output_dir_final, exist_ok=True)
            if data=="/sp17":
                tot_pattern = "totout*2017-01*.txt"
            elif data=="/sp18":
                tot_pattern = "totout*2018-01*.txt"
            elif data=="/fa18":
                tot_pattern = "totout*2018-08*.txt"
                
            main(directory_path+acc, output_dir_final, pattern=tot_pattern)
            for ebin in enBins:
                if data=="/sp17":
                    diff_pattern = "diffout*2017-01*_emin_" + ebin + "*.txt"
                elif data=="/sp18":
                    diff_pattern = "diffout*2018-01*_emin_" + ebin + "*.txt"
                elif data=="/fa18":
                    diff_pattern = "diffout*2018-08*_emin_" + ebin + "*.txt"
                
                main(directory_path+acc, output_dir_final, pattern=diff_pattern)
                    
