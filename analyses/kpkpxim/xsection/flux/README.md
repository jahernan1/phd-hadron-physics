# Photon flux

`getFlux.sh` produced the tagged photon flux used in the cross-section
normalisation. It runs `plot_flux_ccdb.py` from the GlueX `hd_utilities`
(`psflux`) package, which queries the JLab CCDB and RCDB, with 500 bins
between 6.4 and 11.4 GeV, for:

| Period | Runs | `-r` | RCDB query |
|---|---|---|---|
| Spring 2017 | 30274-31057 | 4 | `@is_production and @status_approved` |
| Spring 2018 | 40856-42559 | 2 | `@is_2018production and @status_approved` |
| Fall 2018 | 50685-51768 | 2 | `... and beam_on_current>49` |
| Fall 2018, low current | 51384-51457 | 2 | `... and beam_on_current<49` (3.0-6.0 GeV) |
| Run range 71943-73266 (not used in this analysis) | 71943-73266 | 1 | `@is_dir_production and @status_approved` |

Each call writes `flux_<first run>_<last run>.root` with the histogram
`tagged_flux`. The first three files are the flux inputs of the analysis.

## Reproducing

The script needs python 2.7, `hd_utilities` and CCDB/RCDB access, so it runs
only at JLab (or in an environment with those databases); it cannot run from
this repository alone. The flux files are preserved analysis data: the three
files are under `$GXANA_DATA/flux/` (`gluex_analysis_data/kpkpxim/flux`), which
`config/xsection.yaml` (`flux_dir`) reads. Feeds the cross-section
normalisation (dissertation chapter 6, photon flux).
