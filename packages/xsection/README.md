# packages/xsection

Cross-section machinery of the thesis, extracted from `AnalysisNote/xsection`
and `AnalysisNote/systematics` (behavior-preserving port, spec D20).

## C++ — `GxanaXsec` (namespace `gxana::xsec`)

| Header | Contents |
|---|---|
| `Binning.h` | `divideNominalIntoBins`, `divideThrownIntoBins`, `divideVariationTreesIntoBins`; bin names `emin_<E>_emax_<E>[_tmin_<t>_tmax_<t>]` |
| `YieldFit.h` | RooFit Ξ⁻ mass fits: `RooFitMC`, `RooFitData` (Johnson / Gaussian / Voigtian + Chebychev), `SetFitPlotDir`, `SetFitStyle` |
| `Flux.h` | `GetFluxHist(file, "tagged_flux")` |
| `XSec.h` | `GetDiffXSecFile`, `GetTotXSecFile`, `WriteXSecTables` |
| `Barlow.h` | `calc_barlow`, `calculateStdDevGraph` |
| `Plotting.h` | `plotDiffXSec`, `plotWeightedXSec`, `plotOneWeightedXSec`, `plotFinalWeightedXSec`; `SetPlotDir`, `PlotDir` |

Executables (in `build/bin`):

    gxana_xsec_bin data|mc|thrown|variation IN.root OUT.root --energy 6.4,7.4,...,11.4 --t 0.1,0.35,...,2.4
    gxana_xsec_tables --fit Johnson --param mu=1.3217,1.31,1.33 ... --label johnson --out DIR \
        NAME:DATA.root:MC.root:THROWN.root:FLUX.root [...]

`--label` and `--cheby` are order-sensitive: all JOBs run in order in one
process and share one set of fit parameters, and each JOB uses whichever
`--label`/`--cheby` last preceded it on the command line (a JOB before the
first `--label` is a usage error). Each JOB writes its tables into
`DIR/<label>/`, so labels never overwrite each other. This reproduces the legacy
johnson → johnson_cheby1 parameter carry-over, e.g.:

    gxana_xsec_tables --fit Johnson --param ... --out DIR \
        --label johnson JOBS... --cheby 1 --label johnson_cheby1 JOBS...

See `gxana_xsec_tables --help` for the exact usage text.

## Python — `gxana_xsection` (numpy, pandas)

| Module | Contents |
|---|---|
| `weighted_average` | error-weighted average over the three run periods (`weight_files`); `python -m gxana_xsection.weighted_average DIR OUT [--pattern P]` |
| `components` | split yield/acceptance/flux tables per quantity (`split_files`); `python -m gxana_xsection.components DIR OUT [--pattern P] [--anchor A]` |
| `qvalue_rescale` | scale dσ/dt by qval_yield / data_yield (`process_files`); `python -m gxana_xsection.qvalue_rescale FILE1 COL1 COL2 FILE2 OUT` |
| `tex_table` | build the LaTeX cross-section/systematics tables (`process_files_to_latex`, merges `MakeXsecTexTable{,1,Scale}.py`); `python -m gxana_xsection.tex_table DIR PATTERN OUT [--delimiter D] [--additional F ...] [--systematic-source {run_fraction,scale_factor}]` |
| `compare` | numeric table comparison; `python -m gxana_xsection.compare NEW REF` |

## Tests

Unit tests: `uv run ctest --test-dir build`, `uv run pytest packages/xsection`.
Golden tests rerun the chain on the preserved thesis data
([docs/analysis_data.md](../../docs/analysis_data.md)): `uv run pytest -m golden -v`.
