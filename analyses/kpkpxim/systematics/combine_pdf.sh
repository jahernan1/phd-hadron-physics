#!/bin/bash
# Merge the per-variation fit PDFs written by `gxana run systematics` (tables step)
# into one PDF per cut. Run from $GXANA_OUTPUT/kpkpxim/systematics; usage: combine_pdf.sh [label]
# (label = the systematics.label of config/systematics.yaml, default johnson).

label=${1:-johnson}
variations=("chisqndf" "total_mm2_abs" "kphigh_prap" "xim_pathlensig" "lambda_pathlensig")

mkdir -p combined_pdf
for var in "${variations[@]}"; do
    pdfunite fits/${label}/data_flatTree_kpkpxim__*${var}* combined_pdf/syst_data__${var}_fits.pdf
done
