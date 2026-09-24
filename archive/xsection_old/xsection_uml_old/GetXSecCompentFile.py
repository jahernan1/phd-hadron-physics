import os
import pandas as pd
import argparse

def subset_columns(input_file, output_file, columns):
    # Read the input file into a pandas DataFrame
    df = pd.read_csv(input_file, sep='\s+')

    #print("Input DataFrame:")
    #print(df.head())  # Print the first few rows of the DataFrame for debugging

    # Select the subset of columns
    subset_df = df[columns]

    #print("Subset DataFrame:")
    #print(subset_df.head())  # Print the first few rows of the subset DataFrame for debugging

    # Write the subset DataFrame to a new file
    subset_df.to_csv(output_file, index=False, sep=" ", float_format="%.10f")

def process_files_with_name_pattern(pattern, column_sets, combo_dir):
    # Get the current working directory
    cwd = os.getcwd()
    indir = os.path.join(cwd,"data/","gen_amp_V2_2D_ac/", combo_dir)
    outdir = os.path.join(cwd,"components/","gen_amp_V2_2D_ac/", combo_dir)

    # Create the output directory if it doesn't exist
    os.makedirs(indir, exist_ok=True)
    os.makedirs(outdir, exist_ok=True) 

    # Find files matching the pattern in the current directory
    matching_files = [file for file in os.listdir(indir) if pattern in file]
    
    print("Files grabbed:")
    for file in matching_files:
        # Remove the first 8 characters from the filename
        processed_file = file[8:]
        
        # Print the processed filename
        print(processed_file)
        
        # Full path of the input file
        input_file = os.path.join(indir, file)
        # Process the file with each column set
        for i, columns_to_select in enumerate(column_sets, 1):
            output_file = os.path.join(outdir, columns_to_select[1]+f"_subset_{processed_file}")
            subset_columns(input_file, output_file, columns_to_select)
            
def main():
    # Set up argument parser
    parser = argparse.ArgumentParser(description="Process files and subset columns.")
    parser.add_argument("output_dir", help="Directory to save the output files")
    args = parser.parse_args()

    name_pattern = "diffout_kpkpxim__"  # Change this to match the pattern of the files you want to process
    column_sets = [
        ["#tcenter", "data_yield", "terr", "yield_err"],        
        ["#tcenter", "mc_yield", "terr", "mc_err"],
        ["#tcenter", "thrown_yield", "terr", "thrown_err"],
        ["#tcenter", "accept", "terr", "accep_err"],
        ["#tcenter", "flux", "terr", "flux_err"]
        # Add more column sets as needed
    ]
    
    process_files_with_name_pattern(name_pattern, column_sets, args.output_dir)

if __name__ == "__main__":
    main()
