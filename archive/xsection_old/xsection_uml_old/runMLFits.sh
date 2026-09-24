#!/bin/sh
# Run the rootFiles to generate comparison plots

DELIM=$2
if [[ -z $DELIM ]]; then
    DELIM="rapidityCuts"
fi
COMBO_FLAG=$3
if [[ -z $COMBO_FLAG ]]; then
    COMBO_FLAG="hybrid_combo"
fi

FILE_DIR="/d/grid17/hjesse/AnalysisNote/xsection/prepped_ttrees/gen_amp_V2_2D_ac/"
DATA_DIR="/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/data/gen_amp_V2_2D_ac/"
COMPONENT_DIR="/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/components/gen_amp_V2_2D_ac/"
SUBDIR=$(basename $FILE_DIR)/
echo "IS THIS CORRECT: $SUBDIR"
#Make directories if they dont exist
mkdir -p data/$SUBDIR/$COMBO_FLAG/
mkdir -p components/$COMPONENT_DIR


DATAFILES2017=`ls -v ${FILE_DIR}kpkpxim__M23_2017*${DELIM}*`
DATAFILES201801=`ls -v ${FILE_DIR}kpkpxim__B4_M23_2018-01*${DELIM}*`
DATAFILES201808=`ls -v ${FILE_DIR}kpkpxim__B4_M23_2018-08*${DELIM}*`

arrRootFile=()

if [[ "$1" == "0" ]] || [[ "$1" == "2" ]];then
    # backup and clean text files
    rm ${DATA_DIR}${COMBO_FLAG}/*kpkpxim*${DELIM}* #${DATA_DIR}/bak
    #START FOR LOOP
    for entry in $DATAFILES2017
    do
        #echo \"$(basename "$entry")\"
        if [[ \"$(basename "$entry")\" == *"Tmin"* ]]; then
            echo "TBins**********************************************"
            root -q -b -l 'UMLFits.C('\"$(basename "$entry")\"',true,'\"${SUBDIR}\"','\"$COMBO_FLAG\"')'
        else
            echo "EBins**********************************************"
            root -q -b -l 'UMLFits.C('\"$(basename "$entry")\"',false,'\"${SUBDIR}\"','\"$COMBO_FLAG\"')'
        fi
    done

    for entry in $DATAFILES201801
    do
        #echo \"$(basename "$entry")\"
        if [[ \"$(basename "$entry")\" == *"Tmin"* ]]; then
            echo "TBins**********************************************"
            root -q -b -l 'UMLFits.C('\"$(basename "$entry")\"',true,'\"${SUBDIR}\"','\"$COMBO_FLAG\"')'
        else
            echo "EBins**********************************************"
            root -q -b -l 'UMLFits.C('\"$(basename "$entry")\"',false,'\"${SUBDIR}/\"','\"$COMBO_FLAG\"')'
        fi
    done

    for entry in $DATAFILES201808
    do
        #echo \"$(basename "$entry")\"
        if [[ \"$(basename "$entry")\" == *"Tmin"* ]]; then
            echo "TBins**********************************************"
            root -q -b -l 'UMLFits.C('\"$(basename "$entry")\"',true,'\"${SUBDIR}\"','\"$COMBO_FLAG\"')'
        else
            echo "EBins**********************************************"
            root -q -b -l 'UMLFits.C('\"$(basename "$entry")\"',false,'\"${SUBDIR}\"','\"$COMBO_FLAG\"')'
        fi
    done
elif [[ "$1" == "1" ]]; then
    echo "[INFO]: Running plotting scripts only."
else
    echo "[ERROR]: Incorrect command line arguments... "
    exit
fi

if [[ "$1" == "1" ]] || [[ "$1" == "2" ]]; then
    XSEC_FILES=`ls -v ${DATA_DIR}${COMBO_FLAG}/diffxsec*${DELIM}*Emin*`
    #######################
    #SET UP COMPONENT FILES
    #######
    # python GetXSecComponentFiles.py
    #########################
    #RETRIEVE COMPONENT FILES
    #########
    mkdir -p  ${COMPONENT_DIR}${COMBO_FLAG}
    #Need to run (python GetXSecCompentFile.py) outside the container...
    XSEC_DATA_YIELD_FILES=`ls -v ${COMPONENT_DIR}${COMBO_FLAG}/data_yield*${DELIM}*`
    XSEC_MC_YIELD_FILES=`ls -v ${COMPONENT_DIR}${COMBO_FLAG}/mc_yield*${DELIM}*`
    XSEC_THROWN_YIELD_FILES=`ls -v ${COMPONENT_DIR}${COMBO_FLAG}/thrown_yield*${DELIM}*`
    XSEC_ACCEPT_FILES=`ls -v ${COMPONENT_DIR}${COMBO_FLAG}/accept*${DELIM}*`
    XSEC_FLUX_FILES=`ls -v ${COMPONENT_DIR}${COMBO_FLAG}/flux*${DELIM}*`
    arrOutRootFile=()

    for entry in $XSEC_FILES
    do
        #echo \""$entry,"\"
        arrRootFile+=\""${entry},"\"
    done
    root -q -b -l 'MakeXSec.C('${arrRootFile[*]}')'

    # for entry in $XSEC_DATA_YIELD_FILES
    # do
    #     #echo \"$(basename "$entry")\"
    #     arrOutRootFile+=\""$entry,"\"
    # done
    # root -q -b -l 'MakeXSecComponents.C('${arrOutRootFile[*]}')'

    # arrOutRootFile=()
    # for entry in $XSEC_MC_YIELD_FILES
    # do
    #     #echo \"$(basename "$entry")\"
    #     arrOutRootFile+=\""$entry,"\"
    # done
    # root -q -b -l 'MakeXSecComponents.C('${arrOutRootFile[*]}')'

    # arrOutRootFile=()
    # for entry in $XSEC_THROWN_YIELD_FILES
    # do
    #     #echo \"$(basename "$entry")\"
    #     arrOutRootFile+=\""$entry,"\"
    # done
    # root -q -b -l 'MakeXSecComponents.C('${arrOutRootFile[*]}')'
    
    # arrOutRootFile=()
    # for entry in $XSEC_ACCEPT_FILES
    # do
    #     #echo \"$(basename "$entry")\"
    #     arrOutRootFile+=\""$entry,"\"
    # done
    # root -q -b -l 'MakeXSecComponents.C('${arrOutRootFile[*]}')'

elif [[ "$1" == "0" ]]; then
    echo "[INFO]: Only run the fits. "
else
    echo "[ERROR]: Incorrect command line arguments... "
    exit
fi
