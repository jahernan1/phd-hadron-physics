# packages/xsection

Cross-section machinery of the thesis, extracted from `AnalysisNote/xsection`
and `AnalysisNote/systematics` (behavior-preserving port, `docs/history/REFACTOR_SPEC.md` D20).

## C++ — `GxanaXsec` (namespace `gxana::xsec`)

| Header | Contents |
|---|---|
| `Binning.h` | `divideNominalIntoBins`, `divideThrownIntoBins`, `divideVariationTreesIntoBins`; bin names `emin_<E>_emax_<E>[_tmin_<t>_tmax_<t>]` |
| `YieldFit.h` | RooFit Ξ⁻ mass fits: `RooFitMC`, `RooFitData` (Johnson / Gaussian / Voigtian + Chebychev); `RooFitMCShapeSeed`, `RooFitDataMCShape` (`JohnsonMCShape`, the combo-selection study fit); `RooFitMCPdf` (`MCPdf`: RooHistPdf of the MC mass shape + Chebychev, a fit-model variation); `SetFitPlotDir`, `SetFitStyle` |
| `Flux.h` | `GetFluxHist(file, "tagged_flux")` |
| `XSec.h` | `GetDiffXSecFile`, `GetTotXSecFile`, `WriteXSecTables` |
| `Physics.h` | channel facts passed as `gxana_xsec_tables` flags: `Observable`, `MassWindows`, `Target`, `XSecPhysics`; `TargetDensity` |
| `Plotting.h` | `plotDiffXSec`, `plotWeightedXSec`, `plotOneWeightedXSec`, `plotFinalWeightedXSec`; `SetPlotDir`, `PlotDir` |

Executables (in `build/bin`):

    gxana_xsec_bin data|mc IN.root OUT.root --energy 6.4,7.4,...,11.4 --t 0.1,0.35,...,2.4 \
        --tree NAME --branch B [--branch B ...] [--data-branch B]
    gxana_xsec_bin thrown IN.root OUT.root --energy 6.4,7.4,...,11.4 --t 0.1,0.35,...,2.4 --tree NAME
    gxana_xsec_bin variation IN.root OUT.root --energy 6.4,7.4,...,11.4 --t 0.1,0.35,...,2.4
    gxana_xsec_tables --fit Johnson --param mu=1.3217,1.31,1.33 ... --weight W --label johnson --out DIR \
        NAME:DATA.root:MC.root:THROWN.root:FLUX.root [...] [--plots PLOTDIR] CHANNEL

No channel is built in: `gxana run` passes the channel's values from
`analyses/<channel>/config` as flags. `gxana_xsec_bin` takes the flat-tree name
(`--tree`, `physics.flat_tree` / `thrown_flat_tree`) and, for data and MC, the
binned columns (`--branch`, `xsection.branches`; data adds `--data-branch`,
`physics.qvalue_branch`). `gxana_xsec_tables` requires CHANNEL =
`--observable B --observable-title T` (`physics.observable`), `--gate EXPR`
(`xsection.gate`), `--qvalue-branch B|none` (`physics.qvalue_branch`),
`--br V,E` (`physics.branching_ratio`), `--target ZMIN,ZMAX,DENSITY,MOLAR_MASS,ATOMS`
(`xsection.target`) and one `--mass-window NAME=GEV` per `xsection.mass_windows` entry
(names: [`docs/NEW_CHANNEL.md`](../../docs/NEW_CHANNEL.md#mass-windows)).
`--fit` takes `Johnson`, `Gaussian` or `Voigtian` (signal, Chebychev background of order
`--cheby`, default 2), `JohnsonMCShape`, `JohnsonMCShapeSyst` (the same with the literals of
the legacy `GetXSecFilesUML.C` variation fit, Barlow systematics) or `MCPdf` (RooHistPdf of
the MC mass shape, no `--param`). `--plots PLOTDIR` saves the fit PDFs under `PLOTDIR/<label>/`.
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

`gxana run xsection` passes each period's flux file from `xsection.inputs.flux_dir`,
which the kpkpxim config points at `${GXANA_ANALYSIS_DATA}/kpkpxim/flux` (preserved data).

`gxana run xsection` checks each step's inputs before running it (`gxana_xsec_bin`
would otherwise fail with a raw ROOT error): missing files are listed on stderr with the
command that makes them, and the step does not run. `--dry-run` skips the check.

## Python — `gxana_xsection` (numpy, pandas)

| Module | Contents |
|---|---|
| `weighted_average` | error-weighted average over the run periods (`weight_files`); `python -m gxana_xsection.weighted_average DIR OUT [--pattern P] [--n-periods N]` (default 3) |
| `components` | split yield/acceptance/flux tables per quantity (`split_files`); `python -m gxana_xsection.components DIR OUT --anchor A [--pattern P]` (A = the channel's `reaction`, where each output name starts) |
| `qvalue_rescale` | scale dσ/dt by qval_yield / data_yield (`process_files`); `python -m gxana_xsection.qvalue_rescale FILE1 COL1 COL2 FILE2 OUT` |
| `tex_table` | build the LaTeX cross-section/systematics tables (`process_files_to_latex`, merges `MakeXsecTexTable{,1,Scale}.py`); `python -m gxana_xsection.tex_table DIR PATTERN OUT [--delimiter D] [--additional F ...] [--systematic-source {run_fraction,scale_factor}]`; `--column NAME=FILE` (repeatable) also accepts `NAME=scale_factor`; `--run-fraction F` (default 0.051): with the `run_fraction` source, Run Combination = F × dσ/dt |
| `syst_tables` | the weighted tables with their total systematic, for the dissertation figure: `syst_<table>` beside every `weighted_diffxsec_*.txt` with the quadrature sum of the named columns inserted as column 4 (the per-table output of the legacy `MakeXsecTexTableScale.py`); a column is the last column of a stats file or `scale_factor` (δy·S − δy, 0 for S < 1, from the table's own S column). `python -m gxana_xsection.syst_tables DIR --column NAME=FILE\|scale_factor [--column ...] [--pattern P] [--delimiter D] [--out-dir D]`; `gxana run xsection --steps figures` runs it (`xsection.figures` in `xsection.yaml`: `label`, `columns` (default `xsection.tex.columns`), the `plots` macros and their `requires`); `gxana run xsection --systematics published` puts the keys of `xsection.published_systematics.{tex,figures}` over `xsection.tex` and `xsection.figures` (the preserved dissertation inputs) |
| `integrated_total` | total cross section integrated over the differential -t bins (the `integrate` step): sums dσ/dt times the bin width per energy bin, with the quadrature error; reads `diffxsec_<name>_emin_<E>_emax_<E>.txt` and writes `intxsec_<name>.txt` (columns `enBinCenter sigma enBinWidth Yerr`) beside them, which the step then averages over the run periods into `weighted_data/<label>/intxsec_weighted_output.txt`; `python -m gxana_xsection.integrated_total DIR OUT_DIR [--name PERIOD]` (default: every name found) |
| `compare` | numeric table comparison; `python -m gxana_xsection.compare NEW REF [--pattern P] [--rtol R] [--atol A] [--only-new]` (defaults `*.txt`, 1e-9, 0; `--only-new` ignores reference files not produced) |

## Tests

Unit tests: `uv run ctest --test-dir build`, `uv run pytest packages/xsection`.
Golden tests rerun the chain on the preserved thesis data
([`tests/golden/README.md`](../../tests/golden/README.md#tests)): `uv run pytest -m golden -v`.
