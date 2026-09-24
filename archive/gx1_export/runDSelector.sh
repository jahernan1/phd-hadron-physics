#!/bin/sh
# Run the DSelector 

############################################################
# Help                                                     #
############################################################
Help()
{
   # Display Help
   echo "This script runs the DSelector for one file or uses TChain. If no arguments are given will default to 2017 Trees."
   echo
   usage
   echo
   echo "options:"
   echo "--------"
   echo "-h                    Print this Help."
   echo "-t  TreePath          Select directory where trees are stored (will use TChain)."
   echo "-d  DSelectorPath     Select directory where DSelector is stored."
   echo "-n  Deliminator       Save with deliminator, tack onto end of saveName."
   echo "-o  OutputName        Output root file from running DSelector."
   echo "-c  numCores          Select the number of cores to use to process tree." 
   echo "-s                    Save with default output file, i.e., 'kpkpxim.root', to analysis/kpkpxim/ and Trees/flatTrees/rawTrees/." 
   echo
   echo "arguments:"
   echo "----------"
   echo "TreePath           Path to directory with root trees."
   echo "DSelectorPath      Path to DSelector."
   echo "Deliminator        Tag at end of saveName od -s option used."
   echo "OutputName         Name of output tree from DSelector."
   echo "numCores           Number of cores to use for parallelization."
}

# usage
usage()
{
    echo "Usage: $0 [-h] [-t treePath] [-d DSelectorPath] [-n Deliminator] [-o OutputName] [-s]" 1>&2
    echo "Maintain ordering of Usage to save properly."
}

############################################################
############################################################
# Main program Start                                       #
############################################################
############################################################
#
#######################
# Set default variables
#######################
treePath="Trees/tree_kpkpxim__M23_2017-01_ana56/trees/"
DSelectorPath="DSelector/DSelector_kpkpxim.C"
saveDir="analysis/kpkpxim/"
outputName="kpkpxim.root" #change name for default 
saveName=""
delimName=""
numCores=16
rootFile=("$treePath"*)
treeName=$(rootls "$rootFile")

############################################################
# Process the input options. Add options as needed.        #
############################################################
# Get the options
while getopts ":t:d:c:o:n:hs" option; do
   case $option in
       h) # display Help
           Help
           exit
	       ;;
       s) #default save 
           unset saveName
           treeDir=$(dirname $treePath )
           treeDirName=$(basename $treeDir)
           saveName=${treeDirName:5}
           ;;
       t) # choose treePath
	       unset treePath
	       unset rootFile
	       unset treeName
           treePath=${OPTARG}	   
	       rootFile=("$treePath"*)
	       treeName=$(rootls "$rootFile")
	       ;;
       d) # choose the DSelector
	       unset DSelectorPath
	       DSelectorPath=${OPTARG}
	       ;;
       c) # set the numCores
           unset numCores
           numCores=${OPTARG}
           ;;
       o) # different tree output name to save
           unset outputName
           unset saveName
	       outputName=${OPTARG}
           treeDir=$(dirname $treePath )
           treeDirName=$(basename $treeDir)
           saveName=${treeDirName:5} 
	       ;;
       n) # deliminator name to tack onto savedname 
           delimName="_${OPTARG}"
           echo "New tree name $delimName"
           ;;
       \?) # incorrect option
           echo "Invalid option."
	       Help
           exit
	   ;;
       : )
	   echo "Invalid option: $OPTARG requires an argument" 1>&2
	   Help
	   exit
	   ;;
   esac
done
shift $((OPTIND -1))

#echo "${saveName}${delimName}"

##############################
# Print root file information.
##############################
echo 
echo "Will process the following..."
echo "$rootFile"
echo "$treeName"
echo 
##########################################
# Clean working dir to save files properly
##########################################
if [ -n "$saveName" ]; then
    if [ -f "$outputName" ]; then
	rm "$outputName"
    echo "Removed ${outputName}"
    fi
    if [ -f "thrown_$outputName" ]; then
	rm "thrown_$outputName"
    echo "Removed thrown_${outputName}"
    fi
    if [ -f "flatTree_$outputName" ]; then
	rm "flatTree_$outputName"
    echo "Removed flatTree_${outputName}"
    fi
    if [ -f "flatTree_thrown_$outputName" ]; then
	rm "flatTree_thrown_$outputName"
    echo "Removed flatTree_thrown_${outputName}"
    fi
fi
#####################################
# Run DSelector over data using proof
#####################################
#
root -l -b <<EOF
gEnv->SetValue("ProofLite.Sandbox", "/d/grid17/hjesse/temp");
.x $ROOT_ANALYSIS_HOME/scripts/Load_DSelector.C
TChain *ch = new TChain("$treeName");
ch->Add("$treePath*");
DPROOFLiteManager::Process_Chain(ch,"$DSelectorPath++", $numCores); 
EOF
#ch->Process("$DSelectorPath++")
##################################
# Save the file if option is given
##################################
if [ -n "$saveName" ]; then
    if [ -f "$outputName" ]; then
        mv "$outputName" "$saveDir${saveName}${delimName}.root"
	    echo
	    echo "File saved: $saveDir${saveName}${delimName}.root"
        
    elif [ -f "thrown_$outputName" ]; then
	    mv "thrown_$outputName" "${saveDir}thrown_${saveName}${delimName}.root"
        echo
        echo "Thrown File saved: ${saveDir}thrown_${saveName}${delimName}.root"
    else
        echo 
        echo "Files not saved to analysis/kpkpxim/"
        echo  
    fi
    # Save any flatTree to proper place
    if [ -f "flatTree_$outputName" ]; then
	    mv "flatTree_$outputName" "Trees/flatTree/rawTrees/flatTree_${saveName}${delimName}.root"
        echo
        echo "FlatTree File saved: Trees/flatTree/rawTrees/flatTree_${saveName}${delimName}.root"
    elif [ -f "flatTree_thrown_$outputName" ]; then
	    echo
	    mv "flatTree_thrown_$outputName" "Trees/flatTree/rawTrees/flatTree_thrown_${saveName}${delimName}.root"
        echo
        echo "FlatTree File saved: Trees/flatTree/rawTrees/flatTree_thrown_${saveName}${delimName}.root"
    else
	    echo
	    echo "Flat Tree not saved."
    fi
fi    
####################
##### No PROOF #####
####################
# root -l Trees/tree_kpkpxim_ver38/tree_kpkpxim__B4_M23_030730.root << EOF
# .x $ROOT_ANALYSIS_HOME/scripts/Load_DSelector.C
# kpkpxim__B4_M23_Tree->Process("DSelector/DSelector_kpkpxim.C++","",1,4);
# EOF
