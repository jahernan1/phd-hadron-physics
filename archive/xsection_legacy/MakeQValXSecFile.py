import pandas as pd
import os
import glob

def process_files(file1, col1_file1, col2_file1, file2, output_file):
    """
    Process two files to calculate a ratio from the first file and update the first column of the second file.

    Parameters:
        file1 (str): Path to the first input file.
        col1_file1 (str): Column name in file1 for the numerator of the ratio.
        col2_file1 (str): Column name in file1 for the denominator of the ratio.
        file2 (str): Path to the second input file.
        output_file (str): Path to save the output file.
    """
    # Read the first file
    df1 = pd.read_csv(file1, sep="\s+")
        
    # Check for zero values in the denominator column
    if col2_file1 not in df1.columns or df1[col2_file1].eq(0).any():
        raise ValueError("Denominator column contains zero or is not found in the first file.")

    # Calculate the ratio for each element
    df1['ratio'] = df1[col2_file1] / df1[col1_file1]

    # Read the second file
    df2 = pd.read_csv(file2, sep="\s+")

    # Multiply the first column by the corresponding ratio
    first_column_name = df2.columns[1]
    if len(df1['ratio']) != len(df2):
        raise ValueError("Mismatch in the number of rows between the ratio column and the second file.")

    df2[first_column_name] = df2[first_column_name] * df1['ratio'].values

    # Save the updated file
    df2.to_csv(output_file, sep=" ", index=False)
    print(f"Output saved to {output_file}")

def process_files_in_directory(directory, file1_pattern, file2_pattern, output_directory, col1_file1, col2_file1):
    """
    Process all files in a directory matching specific patterns.

    Parameters:
        directory (str): Path to the directory containing the files.
        file1_pattern (str): Pattern to match files for the first input (e.g., "*file1.csv").
        file2_pattern (str): Pattern to match files for the second input (e.g., "*file2.csv").
        output_directory (str): Path to save the output files.
        col1_file1 (str): Column name in file1 for the numerator of the ratio.
        col2_file1 (str): Column name in file1 for the denominator of the ratio.
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

def main():
    """
    Main function to execute the script.
    """
    # Define parameters
    directory = "/d/grid17/hjesse/AnalysisNote/xsection/data/hybrid_combo"
    file1_pattern = "diffout*.txt"
    file2_pattern = "diffxsec*.txt"
    output_directory = "/d/grid17/hjesse/AnalysisNote/xsection/data/qvalues"
    col1_file1 = "data_yield"
    col2_file1 = "qval_yield"

    # Run the processing
    process_files_in_directory(directory, file1_pattern, file2_pattern, output_directory, col1_file1, col2_file1)

if __name__ == "__main__":
    main()
