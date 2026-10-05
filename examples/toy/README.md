# Toy cross-section walkthrough

A toy reaction, γp → toy, with synthetic data, signal Monte Carlo and photon
flux, run through the real cross-section stage `gxana run xsection`. It needs
no GlueX data and no JLab account: ROOT, a C++17 compiler and `uv` are enough.
The full run takes under a minute on a laptop. The background for each step
is in [`docs/GETTING_STARTED.md`](../../docs/GETTING_STARTED.md).

| File | What |
|---|---|
| `make_toy.py` | writes the toy inputs into a directory you choose (calls `make_toy.C` once per run period); the injected physics is `TRUTH` at the top |
| `make_toy.C` | ROOT macro: flux histogram, data, reconstructed-MC and thrown-MC flat trees of one run period |
| `channel/config/*.yaml` | the toy channel configuration (`channel.yaml`, `periods.yaml`, `samples.yaml`, `binning.yaml`, `xsection.yaml`), the same keys as `analyses/kpkpxim/config` |
| `channel/PlotToyXSec.C` | the figure macro run by `--steps figures`: measured against injected cross sections |
| `check_toy.py` | prints measured against injected values bin by bin; exit 1 if one is off by more than 4σ or 15 % |

`tests/test_toy_example.py` runs the same commands in a temporary directory.

## What the toy contains

For each of the three run periods (2017-01, 2018-01, 2018-08):

- **Flux**: a `tagged_flux` histogram, 1/E shape on 6–12 GeV, 0.6, 1.6 and
  1.0 × 10¹² photons.
- **Signal**: dσ/dt = A₀ e^(−b·t) with A₀ = 30 nb/GeV², b = 1.5 GeV⁻²,
  the same at every beam energy, generated on 0.1 < −t < 2.4 GeV² only
  (so σ = 16.67 nb). The number of produced events is σ × target thickness
  (liquid hydrogen, 1.2 atoms/barn) × flux. Each event is reconstructed with
  probability ε(t) = 0.55 − 0.10·t and gets a mass from a Gaussian at
  1.000 GeV, width 0.010 GeV.
- **Background**: 0.8 events per produced signal event, mass linear on
  0.9–1.1 GeV, a flatter −t slope.
- **Signal MC**: 60 000 thrown events with the same E and t distributions; the
  reconstructed ones pass the same ε(t).

Every event has `weight = 1`. Reconstructed t and beam energy equal the true
ones (no resolution, so no bin migration), there are no accidentals and no
Q-factors: the yield fit alone separates signal from background.

## Run it

From the repository root, in bash or zsh, after the one-time build
([`docs/GETTING_STARTED.md`](../../docs/GETTING_STARTED.md#prerequisites)):

1. Write the inputs into a scratch directory outside the checkout:

   ```bash
   TOY="${TMPDIR:-/tmp}/gxana-toy"
   rm -rf "$TOY"
   uv run python examples/toy/make_toy.py "$TOY"
   export GXANA_ROOT="$TOY/gxana_root" GXANA_DATA="$TOY/data" GXANA_OUTPUT="$TOY/output"
   ```

   `$TOY/data/toy/` now holds nine flat trees, `flux/flux_<period>.root` and
   `truth.txt`. The three variables point `gxana` at the toy:
   `gxana` reads a channel's config from `$GXANA_ROOT/analyses/<channel>/config`,
   so `make_toy.py` builds `$TOY/gxana_root` with links to this checkout's
   `build/`, `packages/` and `rootlogon.C` and `analyses/toy` →
   `examples/toy/channel`.

2. Look at what will run:

   ```bash
   uv run gxana config show --channel toy
   uv run gxana run xsection --channel toy --dry-run
   ```

   The dry run prints every command of the default steps: 9 `gxana_xsec_bin`
   calls (data, MC, thrown for each period), one `gxana_xsec_tables`, then
   the `python -m gxana_xsection.*` calls of `weight`, `integrate` and
   `components`.

3. Run the cross-section chain:

   ```bash
   uv run gxana run xsection --channel toy
   uv run gxana run xsection --channel toy --steps tex,figures
   ```

4. Compare with the injected truth:

   ```bash
   uv run python examples/toy/check_toy.py
   ```

   It prints one row per bin, for example

   ```
   bin                                  measured      truth     stat    pull
   dsigma/dt E 6.40-8.40 t 0.10-0.50      19.150     19.417    0.371   -0.72
   ...
   sigma direct E 6.40-8.40               16.833     16.668    0.250   +0.66
   10/10 within 4 sigma and 15% of the truth (units: nb/GeV^2 for dsigma/dt, nb for sigma)
   ```

Open a new shell when you are done: the exported `GXANA_ROOT` makes
`uv run pytest` refuse to start, and `source env/setup.sh` keeps the toy
`GXANA_DATA` and `GXANA_OUTPUT`
([one checkout per shell](../../docs/environment.md#one-checkout-per-shell)).

## What each step wrote

Everything is under `$GXANA_OUTPUT/toy/xsection/`:

| Step | Output | Contents |
|---|---|---|
| `bin` | `binned_trees/binned_flatTree_<stem>.root`, `binned_thrown_flatTree_<mc stem>.root` | one tree per beam-energy bin (`emin_6.40_emax_8.40`) and per energy and −t bin (`..._tmin_0.10_tmax_0.50`) |
| `tables` | `fits/toy/recon_*.pdf`, `fits/toy/data_*.pdf` | the fit of every bin: MC (yield = weighted count) and data (Gaussian + 2nd-order Chebychev; yield = fitted signal) |
| | `data/toy/totout_<name>.txt`, `diffout_<name>_emin_*_emax_*.txt` | per bin: data yield, MC yield, thrown count, acceptance = MC/thrown, flux, with errors |
| | `data/toy/totxsec_<name>.txt`, `diffxsec_<name>_emin_*_emax_*.txt` | σ(E) in nb and dσ/dt in nb/GeV² per run period: yield / (target × flux × branching ratio × acceptance [× Δt]) |
| `weight` | `weighted_data/toy/weighted_diffxsec_emin_*_emax_*.txt`, `totxsec_weighted_output.txt` | error-weighted average over the three periods; column `S` = √(χ²/(N−1)) of that average |
| `integrate` | `data/toy/intxsec_<name>.txt`, `weighted_data/toy/intxsec_weighted_output.txt` | σ(E) as the sum of dσ/dt × bin width over the −t bins |
| `components` | `components/<sp17,sp18,fa18>/toy/<quantity>_<stem>*.txt` | the columns of `totout`/`diffout` split into one file per quantity (`accept_`, `flux_`, `data_yield_`, ...) |
| `tex` | `tables/diffxsec_table.tex`, `syst_diffxsec_table.tex` | LaTeX tables; the only systematic column is the run-period scale factor (`Run Combination: scale_factor` in `xsection.yaml`) |
| `figures` | `weighted_data/toy/syst_weighted_diffxsec_*.txt`, `figures/toy_dsigma_dt.{pdf,png}`, `figures/toy_sigma_total.{pdf,png}` | total systematic per point, then `PlotToyXSec.C`: measured dσ/dt with the injected curve and its bin means, σ(E) direct and integrated with the injected value |

`<name>` is `flatTree_<stem>`; the stem is `toy__<period>_<launch>` for data
and `toy__<period>_<launch>_signal_mc` for MC (`config.tree_stem`, from
`periods.yaml` and `samples.yaml`).

The red curve in `toy_dsigma_dt` lies below the measured point of a wide −t
bin: a bin measures the mean of dσ/dt over the bin, which for a falling
exponential is above its value at the bin centre. The open red squares are
those bin means; `check_toy.py` compares against them.

## Things to try

- Change `TRUTH`, `FLUX` or `N_THROWN` in `make_toy.py`, rerun steps 1–4 and
  watch the pulls and errors.
- Change `t_bins` or `energy_edges` in `channel/config/binning.yaml` and rerun
  from `bin`.
- Fit with another model: in `channel/config/xsection.yaml` set
  `model: Johnson` with
  `params: {mu: [1.0, 0.98, 1.02], lambda: [0.01, 0.005, 0.03], gamma: [0.0, -0.5, 0.5], delta: [1.0, 0.2, 1.5]}`
  and rerun `--steps tables,weight,integrate` and `check_toy.py`. On the
  toy the cross sections come out about 9 % high, several σ off: the Johnson
  tails absorb part of the background under the peak. Comparing fit models
  like this is what the fit-model systematic (`gxana run systematics`) does.
- Make the MC t slope differ from the data (edit `make_toy.C`): the acceptance
  of a wide bin then no longer matches the data and the result is biased.

## What the toy does not cover

The toy starts at the flat trees, so it skips the stages that need GlueX
data or software: event selection (`gxana run select`, DSelectors on
ReactionFilter trees), flat-tree preparation (`flatTreePrep.C`), signal
weighting (`gxana run qfactors`), simulation (`gxana run mc`) and the tagged
flux (produced at JLab with `hd_utilities`). It also skips the systematics
stages (`gxana run systematics`, `gxana run barlow`), which need several cut
and fit variants of the inputs.
