# Copy to env/site.sh (gitignored) and uncomment/edit for your site.
# env/setup.sh sources it before applying defaults.
#
# JLab ifarm example:
# export GXANA_DATA=/volatile/halld/home/$USER/gxana/data
# export GXANA_OUTPUT=/volatile/halld/home/$USER/gxana/output
# export GXANA_SCRATCH=/scratch/$USER/gxana
# export GXANA_EXTERNALS=$HOME/gxana/externals
#
# FSU grid example: point GXANA_DATA at the directory that contains Trees/ and
# flatTrees/, and GXANA_SCRATCH at a disk with room for PROOF-Lite sandboxes.
#
# Preserved analysis data (golden inputs + reference outputs; docs/analysis_data.md).
# At JLab the thesis deposit lives under /work/halld/gluex_analysis_data/:
# export GXANA_ANALYSIS_DATA=/work/halld/gluex_analysis_data/<thesis-dir>
