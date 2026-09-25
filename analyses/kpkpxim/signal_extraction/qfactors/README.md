# Q-factor signal weights

## What

Event-by-event Q-factors (probabilistic signal/background weights) for
γp → K⁺K⁺Ξ⁻, computed with a fork of
[lan13005/QFactors](https://github.com/lan13005/QFactors) at
`packages/qfactors` (branch `kpkpxim-thesis`; changes listed in that
repository's `CHANGES_THESIS.md`). This directory holds the run
configuration the fork engine reads for this channel: the fit models below,
the legacy helper macros in `scripts/`, and `../../config/qfactors.yaml`.
`gxana run qfactors` drives the engine through `QFACTORS_SETTINGS`.

## Models

Each `configPDFs_<model>.h` fits the discriminating variable `decayxim_M`
(M(Λπ⁻)) with a signal + background RooFit model over nearest-neighbor
subsets in phase space. Details below are read from each file, not assumed.

| Model | Signal PDF | Background PDF | Bins | Fit range (GeV) | File date |
|---|---|---|---|---|---|
| `JohnsonCheb1` | `RooJohnson` | `RooChebychev`, 1st order | 40 | 1.28–1.45 | 2025-02-13 |
| `Johnson` | `RooJohnson` | `RooChebychev`, 2nd order | 50 | 1.28–1.45 | 2024-08-20 |
| `JohnsonGaus` | `RooJohnson` | `RooChebychev` (1st order) + `RooGaussian` reflection | 50 | 1.28–1.45 | 2024-08-20 |
| `Gaussian` | `RooGaussian` | `RooChebychev`, 2nd order | 100 | 1.27–1.45 | 2024-08-20 |

`JohnsonCheb1` is the author's working-directory `configPDFs.h`; `Johnson`,
`JohnsonGaus` and `Gaussian` are the earlier variants tried before it. All
four pin `RooFit::Minimizer("Minuit","migrad")` in their `fitTo()` call
(ROOT 6.24's default minimizer, spec D25) and implement the fork engine's
`drawFitPlots(..., float* chisqndf, ...)` / `draw1DPlots(..., NLL, chisqndf, ...)`
signatures. For ROOT 6.40 each file also drops the unused `RooMinuit.h`/`RooChi2Var.h`
includes and evaluates the PDFs in `calculate_q` with a named `RooArgSet`
normalisation set instead of a temporary (same values; marked `gxana:`).
Thesis model: see `../../config/qfactors.yaml` (`model`); which
model the thesis used is decided in Task 7.

## How to run

```sh
source env/setup.sh --gluex
gxana run qfactors --channel kpkpxim --period 2017-01 --dry-run
gxana run qfactors --channel kpkpxim --period 2017-01
gxana run qfactors --channel kpkpxim --period 2017-01 --steps prepare   # stage + compile only; then ./main <i> in the work dir
```

Output lands in `$GXANA_OUTPUT/kpkpxim/qfactors/<file_tag>_1111111/`.

## Warning

`run.py` waits for every `main` process to finish through
`repeatProgressChecks.py`; if one crashes, it waits forever. Stop `run.py`
by hand and read `logs/<tag>/err<i>.txt` for the crashed process.

## Scripts

| Macro | Description |
|---|---|
| `GetQvalueSum.C` | Sums Q-value-weighted events over a mass window and draws the Q-subtracted `decayxim_M` distribution alongside a RooFit Johnson+Chebychev fit. |
| `MakeParamTrees.C` | Lists the files under a `qfactors` output run directory (log/param-tree inspection helper). |
| `PlotTOF.C` | Plots Ξ⁻ (and Λ) lifetime/time-of-flight distributions, with and without an Q(mass) cut, from a flat tree. |
| `QValWeightExample.C` | Minimal example: reads a post-Q-value tree and draws the raw, Q-signal-weighted and Q-background-weighted `decayxim_M` distributions. |
