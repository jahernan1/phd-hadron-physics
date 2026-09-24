import pandas as pd
import os
import re
import numpy as np
from glob import glob

def process_files_to_latex(directory, pattern, delimiter, additional_files, output_file="diffxsec_table_scale.tex"):
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

        save_df = []
        output_dfs = []
        syst_dfs = []
        run_scale = []
        quadrature_column = []
        
        # Compute quadrature column after processing all files
        if additional_files:
            additional_dfs = [pd.read_csv(file, delimiter=delimiter) for file in additional_files]
            
        for index,file_path in enumerate(file_paths):
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
            scale_condition = df.iloc[:, 4] < 1
            scale_sys = df.iloc[:, 3] * df.iloc[:, 4] - df.iloc[:, 3];
            scale_sys[scale_condition] = 0
            run_scale.extend(scale_sys)
            
            new_column_2 = [
                (f"{c1 - c3:.2f}", f"{c1 + c3:.2f}") for c1, c3 in zip(col1, col3)
            ]

            index_start = index*num_rows
            quadrature_column = np.sqrt(sum(df.iloc[index_start:index_start+num_rows, -1]**2 for df in additional_dfs) + np.array(scale_sys)**2).map("{:.3f}".format)
            quadrature_column = quadrature_column.reset_index(drop=True)
            print(quadrature_column)
            # Build the new DataFrame
            formatted_df = pd.DataFrame({
                "$E_\\gamma\\ (\\text{GeV})$": first_column,  # First row gets "(x, y)", others empty
                "$-t\\ (\\text{GeV}^2)$": [f"({x}, {y})" for x, y in new_column_2],
                "$d\\sigma/dt\\ (\\text{nb/GeV}^2)$": df.iloc[:, 1].map("{:.3f}".format),
                "$\\delta y$ ({\\it stat})": df.iloc[:, 3].map("{:.3f}".format),
                "$\\delta y$ ({\\it syst})": quadrature_column
            })

            # Build systematics data frame
            syst_df = pd.DataFrame({
                "$E_\\gamma\\ (\\text{GeV})$": first_column,  # First row gets "(x, y)", others empty
                "$-t\\ (\\text{GeV}^2)$": [f"({x}, {y})" for x, y in new_column_2],
                "Run Combination": scale_sys.map("{:.3f}".format),
            })

            # Build new systematics data frame
            df_syst = df
            df_syst['\delta_y_syst'] = quadrature_column
            print(df_syst)
            df_syst = df.iloc[:, [0,1,2,5,3,4]]
            
            # Save individual output files
            output_file_name = os.path.join(directory, f"syst_{filename}")
            df_syst.to_csv(output_file_name, index=False, sep=" ")
            print(f"Processed file saved: {output_file_name}")

            # Collect for concatenation
            output_dfs.append(formatted_df)
            syst_dfs.append(syst_df)
            
        # Concatenate all output DataFrames
        combined_df = pd.concat(output_dfs, ignore_index=True)
        combined_syst_df = pd.concat(syst_dfs, ignore_index=True)
        
        # Add systematics to dataframe to make table
        combined_syst_df["Accidentals"] = additional_dfs[0].iloc[:, -1].map("{:.3f}".format)
        combined_syst_df["Yield Extraction"] = additional_dfs[1].iloc[:, -1].map("{:.3f}".format)
        
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

            #Add multiline for systematics and cline
            if "\\toprule" in line:
                formatted_lines_syst.append("& & \\multicolumn{3}{c}{Systematic Source (nb/GeV${}^2$)} \\\ \\cline{3-5}")
                
        with open("syst_"+output_file, 'w') as f:
            f.write("\n".join(formatted_lines_syst))
        print(f"Concatenated LaTeX table saved to syst_{output_file}")
    
    except Exception as e:
        print(f"Error: {e}")

# usage:
# process_files_to_latex("./weighted_data/hybrid_combo", "weighted*.txt", delimiter="\s+",additional_files=["fit_variations_stats.txt", "combo_variations_stats.txt"])
# process_files_to_latex("./weighted_data/mcPdf", "weighted*.txt", delimiter="\s+",additional_files=["fit_variations_stats.txt", "combo_variations_stats.txt"]
# )
# process_files_to_latex("./weighted_data/mcPdf_cheby1", "weighted*.txt", delimiter="\s+",additional_files=["fit_variations_stats.txt", "combo_variations_stats.txt"]
# )
process_files_to_latex("./weighted_data/johnson", "weighted*.txt", delimiter="\s+",additional_files=["fit_variations_stats.txt", "combo_variations_stats.txt"])
