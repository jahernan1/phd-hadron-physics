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

Executables (in `build/bin`):

    gxana_xsec_bin data|mc|thrown|variation IN.root OUT.root --energy 6.4,7.4,...,11.4 --t 0.1,0.35,...,2.4
    gxana_xsec_tables --fit Johnson --param mu=1.3217,1.31,1.33 ... --label johnson --out DIR \
        NAME:DATA.root:MC.root:THROWN.root:FLUX.root [...]

`--label` and `--cheby` are order-sensitive: all JOBs run in order in one
process and share one set of fit parameters, and each JOB uses whichever
`--label`/`--cheby` last preceded it on the command line (a JOB before the
first `--label` is a usage error). This reproduces the legacy
johnson → johnson_cheby1 parameter carry-over, e.g.:

    gxana_xsec_tables --fit Johnson --param ... --out DIR \
        --label johnson JOBS... --cheby 1 --label johnson_cheby1 JOBS...

See `gxana_xsec_tables --help` for the exact usage text.

## Python — `gxana_xsection` (numpy, pandas)

| Module | Contents |
|---|---|
| `weighted_average` | error-weighted average over the three run periods (`weight_files`) |
| `components` | split yield/acceptance/flux tables per quantity |
| `qvalue_rescale` | scale dσ/dt by qval_yield / data_yield |
| `compare` | numeric table comparison; `python -m gxana_xsection.compare NEW REF` |

## Tests

Unit tests: `uv run ctest --test-dir build`, `uv run pytest packages/xsection`.
Golden tests rerun the chain on the preserved thesis data
([docs/analysis_data.md](../../docs/analysis_data.md)): `uv run pytest -m golden -v`.
