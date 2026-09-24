#!/bin/bash

variations=("chisqndf" "total_mm2_abs" "kphigh_prap" "xim_pathlensig" "lambda_pathlensig")

for var in "${variations[@]}"; do
    pdfunite fits/data_flatTree_kpkpxim__*${var}* combined_pdf/syst_data__${var}_fits.pdf
done
