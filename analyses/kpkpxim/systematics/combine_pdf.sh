#!/bin/bash
# Merge the per-variation fit PDFs written by `gxana run barlow` (tables step)
# into one PDF per cut. Run from $GXANA_OUTPUT/kpkpxim/barlow; usage: combine_pdf.sh [label]
# (label = barlow.label of config/barlow.yaml, default johnson).

label=${1:-johnson}
variations=("chisqndf" "total_mm2_abs" "kphigh_prap" "xim_pathlensig" "lambda_pathlensig")

mkdir -p combined_pdf
for var in "${variations[@]}"; do
    pdfunite fits/${label}/data_flatTree_kpkpxim__*${var}* combined_pdf/syst_data__${var}_fits.pdf
done
