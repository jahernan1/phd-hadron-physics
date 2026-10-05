# Q-factor signal weights

## What

Event-by-event Q-factors (probabilistic signal/background weights) for
γp → K⁺K⁺Ξ⁻, computed with a fork of
[lan13005/QFactors](https://github.com/lan13005/QFactors) at
`packages/qfactors` (branch `kpkpxim-thesis`; changes listed in that
repository's `CHANGES_THESIS.md`). The fit models live in the fork, as
fork commits (see `CHANGES_THESIS.md` in `packages/qfactors`). This
directory holds the legacy helper macros in `scripts/`; the run settings,
paths and model choice are in `../../config/qfactors.yaml`.
`gxana run qfactors` drives the engine through `QFACTORS_SETTINGS`.

## Models

Each model is a `configPDFs*.h` file in `packages/qfactors` that fits the
discriminating variable `decayxim_M` (M(Λπ⁻)) with a signal + background
RooFit model over nearest-neighbor subsets in phase space. `gxana run
qfactors` copies the chosen file to `configPDFs.h` in the work directory.
Details below are read from each file, not assumed.

| Model (`packages/qfactors/…`) | Signal PDF | Background PDF | Bins | Fit range (GeV) | `SumW2Error` | File date |
|---|---|---|---|---|---|---|
| `configPDFs.h` (thesis model, default) | `RooJohnson` | `RooChebychev`, 1st order | 40 | 1.28–1.45 | `false` | 2025-02-13 |
| `configPDFs_Johnson.h` | `RooJohnson` | `RooChebychev`, 2nd order | 50 | 1.28–1.45 | `true` | 2024-08-20 |
| `configPDFs_JohnsonGaus.h` | `RooJohnson` | `RooChebychev` (1st order) + `RooGaussian` reflection | 50 | 1.28–1.45 | `true` | 2024-08-20 |
| `configPDFs_Gaussian.h` | `RooGaussian` | `RooChebychev`, 2nd order | 100 | 1.27–1.45 | `true` | 2024-08-20 |

Thesis model: `configPDFs.h` (reproduces the preserved 2017-01
q-factors; `tests/golden/test_qfactors_golden.py`; its SHA-256 is pinned
in `tests/qfactors/test_qfactors_run_config.py`). The three variants
do not: besides the PDF shape they differ from it in initial values,
parameter ranges, bins and `SumW2Error`, and `configPDFs_JohnsonGaus.h`
has a yield defect (`docs/history/PORT_NOTES.md`, section 13).

`configPDFs.h` is the author's working-directory `configPDFs.h`;
`configPDFs_Johnson.h`, `configPDFs_JohnsonGaus.h` and
`configPDFs_Gaussian.h` are the earlier variants tried before it. All
four pin `RooFit::Minimizer("Minuit","migrad")` in their `fitTo()` call
(ROOT 6.24's default minimizer, `docs/history/REFACTOR_SPEC.md` D25), implement the fork engine's
`drawFitPlots(..., float* chisqndf, ...)` / `draw1DPlots(..., NLL, chisqndf, ...)`
signatures, and build and run on ROOT 6.40 (no `RooMinuit.h`/`RooChi2Var.h`
includes; `calculate_q` evaluates the PDFs with a named `RooArgSet`
normalisation set; same values). The default model is `qfactors.model` in
`../../config/qfactors.yaml`; pick a variant with `--model`:

```sh
gxana run qfactors --channel kpkpxim --period 2017-01 --model configPDFs_Johnson.h
```

A channel can bring its own model without a commit to the fork: set
`model:` (or `--model`) to the path of the file relative to the repository
root, for example
`analyses/<channel>/config/qfactors_models/configPDFs_X.h`. A value that
is not a `configPDFs*.h` file of `packages/qfactors` must contain a `/`
and name an existing file. The stage copies that file to `configPDFs.h`
in the work directory, next to the rendered `configSettings.h` and the
engine's `auxilliary/`, so its `#include "configSettings.h"` and
`#include "./auxilliary/..."` lines resolve as they do for the fork's
models; it must implement the same `fitManager` interface.

## How to run

```sh
source env/setup.sh --gluex
gxana run qfactors --channel kpkpxim --period 2017-01 --dry-run
gxana run qfactors --channel kpkpxim --period 2017-01
gxana run qfactors --channel kpkpxim --period 2017-01 --steps prepare   # stage + compile only; then ./main <i> in the work dir
```

Output lands in `$GXANA_OUTPUT/kpkpxim/qfactors/<file_tag>_1111111/`.

`--steps` takes a comma-separated subset of `prepare,fit,plots` (default
`fit,plots`): `prepare` stages the work dir and compiles `main` only; `fit`
runs the `main` processes; `plots` runs `hadd` + `mergeQresults` into
`postQVal_flatTree_<file_tag>_1111111.root` (the xsection input) and then
`makePlots`. `run.py` exits 0 even when a step fails, so with `plots`
gxana checks that the `postQVal` file was written and otherwise exits 1,
pointing at the `err*.txt` logs.

## As run for the thesis

Settings from `../../config/qfactors.yaml` (`qfactors.settings`), which
produced the preserved `postQVal_*` outputs:

- Nearest neighbours: `kDim: 200`. The dissertation text quotes 150; the code
  and the preserved outputs were produced with 200.
- Seven phase-space variables (`varStringBase`, the seven `1`s of the
  `_1111111` output tag): `beam_E`, `kp_highp_CosTheta`, `kp_highp_Phi`,
  `kplow_costheta_hf`, `kplow_phi_hf`, `pim1_costheta_hf`, `decaylamb_M_meas`.
- Range standardisation of the variables (`standardizationType: range`).
- `seedShift: 1341`.
- Discriminating variable `decayxim_M`, fit range 1.28-1.45 GeV
  (`fitRangeX` in `packages/qfactors/configPDFs.h`), weights `hybrid_combo`.
- Input `flatTree_<stem>_nominal_kphighrap.root`, tree `flatTree_kpkpxim`.

## Warning

`run.py` waits for every `main` process to finish through
`repeatProgressChecks.py`; if one crashes, it waits forever. Stop `run.py`
by hand and read `logs/<tag>/err<i>.txt` for the crashed process.

`mergeQresults.C` and `makePlots.C` open `logs/<tag>/...` relative to the
current directory: run them from the work directory, as `run.py` and
`gxana run qfactors` do.

## Scripts

| Macro | Description |
|---|---|
| `GetQvalueSum.C` | Sums Q-value-weighted events over a mass window and draws the Q-subtracted `decayxim_M` distribution alongside a RooFit Johnson+Chebychev fit. |
| `MakeParamTrees.C` | Lists the files under a `qfactors` output run directory (log/param-tree inspection helper). |
| `PlotTOF.C` | Plots Ξ⁻ (and Λ) lifetime/time-of-flight distributions, with and without an Q(mass) cut, from a flat tree. |
| `QValWeightExample.C` | Minimal example: reads a post-Q-value tree and draws the raw, Q-signal-weighted and Q-background-weighted `decayxim_M` distributions. |
