import os
import argparse

def main(args):
    # Directory paths
    FILE_DIR = "/d/grid17/hjesse/AnalysisNote/xsection/prepped_ttrees/gen_amp_V2/"
    DATA_DIR = "/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/data/"

    # File lists
    DATAFILES2017 = sorted([file for file in os.listdir(FILE_DIR) if file.startswith("kpkpxim__M23_2017")])
    DATAFILES201801 = sorted([file for file in os.listdir(FILE_DIR) if file.startswith("kpkpxim__B4_M23_2018-01")])
    DATAFILES201808 = sorted([file for file in os.listdir(FILE_DIR) if file.startswith("kpkpxim__B4_M23_2018-08")])

    # Output lists
    arrRootFile = []
    arrOutRootFile = []

    if args.mode in ["11", "10"]:
        # Backup and clean text files
        # os.system(f"rm {DATA_DIR}*kpkpxim*") # Uncomment if needed

        # Process files for 2017
        print("Processing 2017 data...")
        for entry in DATAFILES2017:
            if "Tmin" in entry:
                print("TBins**********************************************")
                os.system(f"root -q -b -l 'UMLFits.C(\"{entry}\",true)'")  # Uncomment if needed
            else:
                print("EBins**********************************************")
                os.system(f"root -q -b -l 'UMLFits.C(\"{entry}\",false)'")  # Uncomment if needed

        # Process files for 2018-01
        print("Processing 2018-01 data...")
        for entry in DATAFILES201801:
            if "Tmin" in entry:
                print("TBins**********************************************")
                os.system(f"root -q -b -l 'UMLFits.C(\"{entry}\",true)'")  # Uncomment if needed
            else:
                print("EBins**********************************************")
                os.system(f"root -q -b -l 'UMLFits.C(\"{entry}\",false)'")  # Uncomment if needed

        # Process files for 2018-08
        print("Processing 2018-08 data...")
        for entry in DATAFILES201808:
            if "Tmin" in entry:
                print("TBins**********************************************")
                os.system(f"root -q -b -l 'UMLFits.C(\"{entry}\",true)'")  # Uncomment if needed
            else:
                print("EBins**********************************************")
                os.system(f"root -q -b -l 'UMLFits.C(\"{entry}\",false)'")  # Uncomment if needed

    if args.mode in ["01", "11"]:
        # Process XSEC files
        XSEC_FILES = sorted([file for file in os.listdir(DATA_DIR) if "diffxsec" in file])
        XSEC_OUT_FILES = sorted([file for file in os.listdir(DATA_DIR) if "diffout" in file])

        print("Processing XSEC files...")
        for entry in XSEC_FILES:
            print(f"\"{entry}\",")
            arrRootFile.append(entry)

        # Uncomment below if needed
        os.system(f"root -q -b -l 'MakeXSec.C({','.join(arrRootFile)})'")
        
        # Uncomment below if needed
        # for entry in XSEC_OUT_FILES:
        #     arrOutRootFile.append(entry)
        # os.system(f"root -l 'MakeXSecComponents.C({','.join(arrOutRootFile)})'")

    else:
        print("[ERROR]: Incorrect mode argument...")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Script to process data files.")
    parser.add_argument("mode", choices=["11", "10", "01"], help="Run ML Fits, Plots Everything.")
    args = parser.parse_args()
    main(args)
