import pandas as pd
import os
import re
import numpy as np
from glob import glob

def process_files_to_latex(directory, pattern, delimiter, additional_files, output_file="diffxsec_table_runsyst.tex"):
    """
    Processes input files, reformats their data, and creates a LaTeX-compatible table.
    
    Parameters:
        directory (str): Directory containing the input files.
        pattern (str): File pattern to match (e.g., '*.txt').
        delimiter (str): Delimiter used in the input files (e.g., ',' or '\t').
        additional_files (list): List of additional files from which the last column will be used to compute the 5th column.
        output_file (str): Path to save the concatenated LaTeX table.
    """
    try:
        # Get all primary files matching the pattern
        file_paths = glob(os.path.join(directory, pattern))
        if not file_paths:
            raise FileNotFoundError("No files matching the pattern were found in the directory.")
        
        # Sort files by numerical value in filenames
        file_paths.sort(
            key=lambda x: float(re.search(r'\d+\.\d+', os.path.basename(x)).group())
        )
  
        output_dfs = []
        syst_dfs = []
        run_syst = []
        
        for file_path in file_paths:
            # Extract x and y values from the filename
            filename = os.path.basename(file_path)
            match = re.findall(r'\d+\.\d+', filename)
            if len(match) < 2:
                raise ValueError(f"Filename {filename} does not contain two numerical values in the format '6.40'.")
            x, y = map(float, match[:2])
            
            # Read the input file into a DataFrame
            df = pd.read_csv(file_path, delimiter=delimiter)
            num_rows = len(df)
            
            # Use `\multirow` for the first column
            multirow_value = f"\\multirow{{{num_rows}}}{{*}}{{({x:.2f}, {y:.2f})}}"
            first_column = [multirow_value] + [''] * (num_rows - 1)  # First row has the value, others are empty
                        
            # Create the new columns
            new_column_1 = f"({x:.2f}, {y:.2f})"
            col1 = df.iloc[:, 0]
            col3 = df.iloc[:, 2]
            new_column_2 = [
                (f"{c1 - c3:.2f}", f"{c1 + c3:.2f}") for c1, c3 in zip(col1, col3)
            ]

            # Add an additional column that is 8% of column 3
            additional_col = (df.iloc[:, 1] * 0.051)
            run_syst.extend(additional_col)  # Append the values of this column to the list
            
            # Build the new DataFrame
            formatted_df = pd.DataFrame({
                "$E_\\gamma\\ (\\text{GeV})$": first_column,  # First row gets "(x, y)", others empty
                "$-t\\ (\\text{GeV}^2)$": [f"({x}, {y})" for x, y in new_column_2],
                "$d\\sigma/dt\\ (\\text{nb/GeV}^2)$": df.iloc[:, 1].map("{:.3f}".format),
                "$\\delta y$ ({\\it stat})": df.iloc[:, 3].map("{:.3f}".format),
            })

            # Build systematics data frame
            syst_df = pd.DataFrame({
                "$E_\\gamma\\ (\\text{GeV})$": first_column,  # First row gets "(x, y)", others empty
                "$-t\\ (\\text{GeV}^2)$": [f"({x}, {y})" for x, y in new_column_2],
            })
            
            # Save individual output files
            output_file_name = os.path.join(directory, f"processed_{filename}")
            formatted_df.to_csv(output_file_name, index=False)
            print(f"Processed file saved: {output_file_name}")

            
            # Collect for concatenation
            output_dfs.append(formatted_df)
            syst_dfs.append(syst_df)
            
        # Concatenate all output DataFrames
        combined_df = pd.concat(output_dfs, ignore_index=True)
        combined_syst_df = pd.concat(syst_dfs, ignore_index=True)
        
        #print(run_syst)
        # Compute quadrature column after processing all files
        if additional_files:
            additional_dfs = [pd.read_csv(file, delimiter=delimiter) for file in additional_files]
            quadrature_column = np.sqrt(sum(df.iloc[:, -1]**2 for df in additional_dfs) + np.array(run_syst)**2).map("{:.3f}".format)
            combined_df["$\\delta y$ ({\\it syst})"] = quadrature_column
        else:
            combined_df["$\\delta y$ ({\\it syst})"] = ""

        # Add systematics to dataframe to make table
        combined_syst_df["Accidentals"] = additional_dfs[0].iloc[:, -1]
        combined_syst_df["Yield Extraction"] = additional_dfs[1].iloc[:, -1]
        combined_syst_df["Run Combination"] = run_syst

        
        # Generate LaTeX table
        latex_table = combined_df.to_latex(index=False, escape=False, multicolumn=True, multirow=True, longtable=True, caption="Table of the differential cross section for the \\GlueXI data.", label="tab:diffxsec", column_format='c|c|ccc', sparsify=False)
        latex_table_syst = combined_syst_df.to_latex(index=False, escape=False, multicolumn=True, multirow=True, longtable=True, caption="Table of the point-by-point systematics of the differential cross section for the \\GlueXI data.", label="tab:diffxsec", column_format='c|c|ccc', sparsify=False)

         # Manually insert \hline after each multirow block
        latex_lines = latex_table.split("\n")
        formatted_lines = []
        inside_multirow = False
        for line in latex_lines:
            formatted_lines.append(line)
            if "\\multirow" in line:
                row_count = 0
                inside_multirow = True
            if inside_multirow:
                row_count += 1
            
            # Insert \hline after the 7th row in a multirow block
            if inside_multirow and row_count == 7:
                formatted_lines.append("\\hline")
                inside_multirow = False  # Reset after placing \hline
    
        # Save LaTeX table
        with open(output_file, 'w') as f:
            f.write("\n".join(formatted_lines))
        print(f"Concatenated LaTeX table saved to {output_file}")

        latex_lines_syst = latex_table_syst.split("\n")
        formatted_lines_syst = []
        inside_multirow = False
        for line in latex_lines_syst:
            formatted_lines_syst.append(line)
            if "\\multirow" in line:
                row_count = 0
                inside_multirow = True
            if inside_multirow:
                row_count += 1
            
            # Insert \hline after the 7th row in a multirow block
            if inside_multirow and row_count == 7:
                formatted_lines_syst.append("\\hline")
                inside_multirow = False  # Reset after placing \hline
        
        with open("syst_"+output_file, 'w') as f:
            f.write("\n".join(formatted_lines_syst))
        print(f"Concatenated LaTeX table saved to syst_{output_file}")
    
    except Exception as e:
        print(f"Error: {e}")

# Example usage:
process_files_to_latex("./weighted_data/hybrid_combo", "weighted*.txt", delimiter="\s+",additional_files=["fit_variations_stats.txt", "combo_variations_stats.txt"])

