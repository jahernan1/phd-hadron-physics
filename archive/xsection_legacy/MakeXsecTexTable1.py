import pandas as pd
import os
import re
from glob import glob

def process_files_to_latex(directory, pattern, delimiter, output_file="output1.tex"):
    """
    Processes input files, reformats their data, and creates a LaTeX-compatible table.
    
    Parameters:
        directory (str): Directory containing the input files.
        pattern (str): File pattern to match (e.g., '*.txt').
        delimiter (str): Delimiter used in the input files (e.g., ',' or '\t').
        output_file (str): Path to save the concatenated LaTeX table.
    """
    try:
        # Get all files matching the pattern
        file_paths = glob(os.path.join(directory, pattern))
        if not file_paths:
            raise FileNotFoundError("No files matching the pattern were found in the directory.")
        
        # Sort files by numerical value in filenames
        file_paths.sort(
            key=lambda x: float(re.search(r'\d+\.\d+', os.path.basename(x)).group())
        )
        
        output_dfs = []

        
        for file_path in file_paths:
            # Extract x and y values from the filename
            filename = os.path.basename(file_path)
            match = re.findall(r'\d+\.\d+', filename)
            if len(match) < 2:
                raise ValueError(f"Filename {filename} does not contain two numerical values in the format '6.40'.")
            x, y = map(float, match[:2])
            
            # Read the input file into a DataFrame
            df = pd.read_csv(file_path, delimiter=delimiter)
            
            # Create the new columns
            new_column_1 = f"({x:.2f}, {y:.2f})"
            col1 = df.iloc[:, 0]
            col3 = df.iloc[:, 2]
            new_column_2 = [
                (f"{c1 - c3:.2f}", f"{c1 + c3:.2f}") for c1, c3 in zip(col1, col3)
            ]
            
            # Build the new DataFrame
            formatted_df = pd.DataFrame({
                "$E_\\gamma\\ (\\text{GeV})$": [new_column_1] + [''] * (len(df) - 1),
                "$-t\\ (\\text{GeV}^2)$": [f"({x}, {y})" for x, y in new_column_2],
                "$d\\sigma/dt\\ (\\text{nb/GeV}^2)$": df.iloc[:, 1].map("{:.3f}".format),
                "$\delta y (stat)$": df.iloc[:, 3].map("{:.3f}".format),
            })
            
            # Save individual output files
            output_file_name = os.path.join(directory, f"processed_{filename}")
            formatted_df.to_csv(output_file_name, index=False)
            print(f"Processed file saved: {output_file_name}")
            
            # Collect for concatenation
            output_dfs.append(formatted_df)
        
        # Concatenate all output DataFrames
        combined_df = pd.concat(output_dfs, ignore_index=True)
        
        # Generate LaTeX table
        latex_table = combined_df.to_latex(index=False, escape=False, multicolumn=True, multirow=True, longtable=True, caption="", label="tab:diffxsec", column_format='c|c|cc', sparsify=False)
        
        # Save LaTeX table
        with open(output_file, 'w') as f:
            f.write(latex_table)
        print(f"Concatenated LaTeX table saved to {output_file}")
    
    except Exception as e:
        print(f"Error: {e}")

# Example usage:
process_files_to_latex("./weighted_data/hybrid_combo", "weighted*.txt", delimiter="\s+")
