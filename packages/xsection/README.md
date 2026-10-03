# packages/xsection

Cross-section machinery of the thesis, extracted from `AnalysisNote/xsection`
and `AnalysisNote/systematics` (behavior-preserving port, `docs/REFACTOR_SPEC.md` D20).

## C++ — `GxanaXsec` (namespace `gxana::xsec`)

| Header | Contents |
|---|---|
| `Binning.h` | `divideNominalIntoBins`, `divideThrownIntoBins`, `divideVariationTreesIntoBins`; bin names `emin_<E>_emax_<E>[_tmin_<t>_tmax_<t>]` |
| `YieldFit.h` | RooFit Ξ⁻ mass fits: `RooFitMC`, `RooFitData` (Johnson / Gaussian / Voigtian + Chebychev); `RooFitMCShapeSeed`, `RooFitDataMCShape` (`JohnsonMCShape`, the combo-selection study fit); `RooFitMCPdf` (`MCPdf`: RooHistPdf of the MC mass shape + Chebychev, a fit-model variation); `SetFitPlotDir`, `SetFitStyle` |
| `Flux.h` | `GetFluxHist(file, "tagged_flux")` |
| `XSec.h` | `GetDiffXSecFile`, `GetTotXSecFile`, `WriteXSecTables` |
| `Plotting.h` | `plotDiffXSec`, `plotWeightedXSec`, `plotOneWeightedXSec`, `plotFinalWeightedXSec`; `SetPlotDir`, `PlotDir` |

Executables (in `build/bin`):

    gxana_xsec_bin data|mc IN.root OUT.root --energy 6.4,7.4,...,11.4 --t 0.1,0.35,...,2.4 \
        --tree NAME --branch B [--branch B ...] [--data-branch B]
    gxana_xsec_bin thrown IN.root OUT.root --energy 6.4,7.4,...,11.4 --t 0.1,0.35,...,2.4 --tree NAME
    gxana_xsec_bin variation IN.root OUT.root --energy 6.4,7.4,...,11.4 --t 0.1,0.35,...,2.4
    gxana_xsec_tables --fit Johnson --param mu=1.3217,1.31,1.33 ... --weight W --label johnson --out DIR \
        NAME:DATA.root:MC.root:THROWN.root:FLUX.root [...] CHANNEL

No channel is built in: `gxana run` passes the channel's values from
`analyses/<channel>/config` as flags. `gxana_xsec_bin` takes the flat-tree name
(`--tree`, `physics.flat_tree` / `thrown_flat_tree`) and, for data and MC, the
binned columns (`--branch`, `xsection.branches`; data adds `--data-branch`,
`physics.qvalue_branch`). `gxana_xsec_tables` requires CHANNEL =
`--observable B --observable-title T` (`physics.observable`), `--gate EXPR`
(`xsection.gate`), `--qvalue-branch B|none` (`physics.qvalue_branch`),
`--br V,E` (`physics.branching_ratio`), `--target ZMIN,ZMAX,DENSITY,MOLAR_MASS,ATOMS`
(`xsection.target`) and one `--mass-window NAME=GEV` per `xsection.mass_windows` entry.
The `physics:` keys are described in the channel README
([`analyses/kpkpxim`](../../analyses/kpkpxim/README.md#channel-physics)).

`--label`, `--cheby` and `--weight` are order-sensitive: all JOBs run in order in one
process and share one set of fit parameters, and each JOB uses whichever
`--label`/`--cheby`/`--weight` last preceded it on the command line (a JOB before the
first `--label` or `--weight` is a usage error). Each JOB writes its tables into
`DIR/<label>/`, so labels never overwrite each other. This reproduces the legacy
johnson → johnson_cheby1 parameter carry-over, e.g.:

    gxana_xsec_tables --fit Johnson --param ... --out DIR --weight W \
        --label johnson JOBS... --cheby 1 --label johnson_cheby1 JOBS... CHANNEL

`--fit JohnsonMCShape` is the fit of the JohnsonMCShape study (legacy `MakeXSecFiles.C`; the dissertation tables use `--fit Johnson`, label `johnson`): per bin, a
Johnson fit to MC fixes the signal shape of the data fit (Johnson + 2nd-order
Chebychev). It takes exactly `mu`, `lambda`, `gamma`, `delta` and `--cheby 2`,
and every bin restarts from those parameters. The legacy combo-selection study
ran it once per event weight into `data/<weight>/`:

    gxana_xsec_tables --fit JohnsonMCShape --param mu=1.3217,1.32,1.33 \
        --param lambda=0.004,0.002,0.007 --param gamma=-0.01,-1,1 --param delta=1.2,0.2,5 \
        --out DIR --weight hybrid_combo --label hybrid_combo JOBS... \
        --weight best_combo --label best_combo JOBS... --weight acc_weight --label acc_weight JOBS... CHANNEL

See `gxana_xsec_tables --help` for the exact usage text.

## Python — `gxana_xsection` (numpy, pandas)

| Module | Contents |
|---|---|
| `weighted_average` | error-weighted average over the run periods (`weight_files`); `python -m gxana_xsection.weighted_average DIR OUT [--pattern P] [--n-periods N]` (default 3) |
| `components` | split yield/acceptance/flux tables per quantity (`split_files`); `python -m gxana_xsection.components DIR OUT --anchor A [--pattern P]` (A = the channel's `reaction`, where each output name starts) |
| `qvalue_rescale` | scale dσ/dt by qval_yield / data_yield (`process_files`); `python -m gxana_xsection.qvalue_rescale FILE1 COL1 COL2 FILE2 OUT` |
| `tex_table` | build the LaTeX cross-section/systematics tables (`process_files_to_latex`, merges `MakeXsecTexTable{,1,Scale}.py`); `python -m gxana_xsection.tex_table DIR PATTERN OUT [--delimiter D] [--additional F ...] [--systematic-source {run_fraction,scale_factor}]` |
| `compare` | numeric table comparison; `python -m gxana_xsection.compare NEW REF` |

## Tests

Unit tests: `uv run ctest --test-dir build`, `uv run pytest packages/xsection`.
Golden tests rerun the chain on the preserved thesis data
([docs/analysis_data.md](../../docs/analysis_data.md)): `uv run pytest -m golden -v`.
