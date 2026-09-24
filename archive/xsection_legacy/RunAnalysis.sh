#!/bin/sh
# Run the analysis workflow

############################################################
# Help                                                     #
############################################################
Help()
{
   # Display Help
   echo "This script runs the full analysis."
   echo
   usage
   echo
   echo "options:"
   echo "-h                    Print this Help."
   echo "-Q                    Will exclude running the QFactor scripts."
   echo "-C                    Select directory where DSelector is stored."
   echo "-s                    Save with default output file, i.e., 'kpkpxim.root', to analysis/kpkpxim/." 
   echo "-S  outputName        Save to analysis/kpkpxim/ with specific outputName." 
   echo
   echo "arguments:"
   echo "treePath           Path to directory with root trees."
   echo "DSelectorPath      Path to DSelector."
   echo "outputName         Name of output tree from DSelector."
}

# usage
usage()
{
    echo "Usage: $0 [-h] [-t treePath] [-d DSelectorPath] [-S saveName] [-s]" 1>&2
    echo "Maintain ordering of Usage to save properly."
}

############################################################
############################################################
# Main program Start                                       #
############################################################
############################################################
#initialize directories
prep_dir="/d/grid17/hjesse/AnalysisNote/code"
qfactor_dir="/d/grid17/hjesse/AnalysisNote/QFactors"
cut_analysis_dir="/d/grid17/hjesse/AnalysisNote/analysis/event_selection" 
# Process event selection and qfactors
root -q "$prep_dir/flatTreePrep.C"

# Run the cut analysis scripts
root -q "$cut_analysis_dir/CutAnalysis.C"
