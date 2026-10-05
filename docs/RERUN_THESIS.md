# Rerunning the thesis

This guide is for a GlueX collaborator or reviewer who wants to regenerate or
check the results of the dissertation (γp → K⁺K⁺Ξ⁻, channel `kpkpxim`; the
excited-Ξ section uses `kpkpkmlamb`). It gives three routes, from cheapest to
most complete, a map from every dissertation figure and table to the command
that makes it, and the differences a careful rerun will show. Command flags and
every input and output path are in the channel README,
[`analyses/kpkpxim/README.md`](../analyses/kpkpxim/README.md#pipeline); this
guide links there rather than repeating them.

| Route | Machine | Needs | Reproduces |
|---|---|---|---|
| [1. Check](#1-check-the-golden-tests) | laptop | preserved data | every stage with preserved inputs, against the preserved thesis outputs (`pytest -m golden`, about 15 min) |
| [2. Rerun from preserved data](#2-rerun-from-the-preserved-data) | laptop | preserved data | the chapter-6 cross-section tables and figures, the chapter-8 summary tables, the Ξ mass and spin, most chapter-7 systematics, the chapter-5 data/MC kinematics |
| [3. Full chain](#3-full-chain-from-the-skims) | JLab ifarm, then any machine | GlueX skims, the MC farm | everything, from event selection on; chapter-4 cut studies and the Barlow variations need it |

To add a channel or reuse the framework instead, see
[NEW_CHANNEL.md](NEW_CHANNEL.md).

## Before you start

- Build the repository as in [GETTING_STARTED.md](GETTING_STARTED.md#prerequisites)
  (`uv sync`, `source env/setup.sh`, the CMake build), then run
  `uv run gxana doctor`.
- **Preserved data.** Routes 1 and 2 read the author's preserved inputs and
  reference outputs from `$GXANA_ANALYSIS_DATA/kpkpxim`. They are GlueX data and
  not public; collaboration members find them under `/work/halld/gluex_analysis_data`
  ([analysis_data.md](analysis_data.md#depositing-at-jlab)). Copy or mount that
  directory, export `GXANA_ANALYSIS_DATA` to it, and check it:

  ```bash
  uv run gxana data status --channel kpkpxim   # every file ok; a miss or diff exits 1
  ```

- **ROOT version.** The thesis ran with ROOT 6.24.04 (GlueX version set 5.12.0);
  every rerun quoted in this repository used ROOT 6.40.04. Fit yields can move in
  their last digits between versions, which is why the fit-yield golden
  comparisons carry a tolerance. The authoritative check, the golden suite in the
  ROOT 6.24 analysis container, has not been run yet
  ([tests/golden/README.md](../tests/golden/README.md#root-624-container-run) gives the command).
- **Threads and fit order.** The shipped studies and measurements configs run
  single-threaded, as the golden tests do: implicit multithreading changes the
  mass fit and the kinematics binning. Thesis-era fits share one static TMinuit
  object, so numbers depend on the fit order; run the stages as documented, not
  bins or labels in isolation ([KNOWN_ISSUES.md](KNOWN_ISSUES.md), preamble).
- **Outputs** land under `$GXANA_OUTPUT/kpkpxim/` (default `_output/` in the
  checkout). Paths below are relative to it unless they start with `$`.

## 1. Check: the golden tests

```bash
uv run pytest -m golden -rs                          # about 15 min; -rs lists skips
uv run pytest tests/golden/test_xsec_golden.py       # one stage at a time
```

Each golden test reruns one stage on preserved inputs and compares with the
preserved thesis output: tables byte for byte, fits within tolerances set by
`GXANA_GOLDEN_RTOL` and `GXANA_GOLDEN_FIT_RTOL`. A skipped test proves nothing:
`-rs` prints why it skipped (no data, no ROOT, apps not built). The list of
tests and what each compares is in
[tests/golden/README.md](../tests/golden/README.md#tests).
`tests/golden/test_systematics_chain_golden.py` takes 5–7 min of the total; set
`GXANA_GOLDEN_SYST_OUTPUT` to a finished run's `GXANA_OUTPUT` to reuse its output.

## 2. Rerun from the preserved data

### Cross section (chapters 6 and 8)

```bash
uv run gxana data stage  --channel kpkpxim       # --dry-run first to see the plan
uv run gxana run xsection --channel kpkpxim --steps tables,weight,integrate,components
uv run gxana run xsection --channel kpkpxim --steps tex,figures --systematics published
```

`data stage` copies the thesis binned trees, the post-Q-factor trees and the MC
flat trees to where the stages read them, and links the thrown trees; it never
overwrites a different file. The second command refits every bin of the thesis
binned trees (the published label `johnson`), forms the run-period weighted
average, integrates dσ/dt and splits the components. The third builds the
dissertation LaTeX tables in `xsection/tables/` and the chapter-6 figures in
`xsection/figures/`, taking the systematic columns from the preserved
dissertation inputs.

Leave `bin` out of the steps. The preserved MC and thrown flat trees are a later
production than the thesis binned trees; re-binning them moves the acceptance by
up to 3 % in single (E, −t) bins ([KNOWN_ISSUES.md](KNOWN_ISSUES.md#10-mc-sample-provenance)).

Compare with the dissertation: `xsection/tables/diffxsec_table_scale.tex` and
`syst_diffxsec_table_scale.tex` are the chapter-8 tables;
`tests/golden/test_xsection_reproduction_golden.py` runs this exact route and
compares them with the preserved reference.

### Systematics (chapter 7)

To regenerate the systematic columns instead of reading the published ones:

```bash
uv run gxana run systematics --channel kpkpxim                   # fit,qvalue,weight,spread,track,summary
uv run gxana run systematics --channel kpkpxim --steps runperiod # run-period ratio figures (opt-in)
uv run gxana run xsection --channel kpkpxim --steps tex,figures  # tables and band from the regenerated columns
```

The regenerated fit-model spread does not equal the published one
([KNOWN_ISSUES.md](KNOWN_ISSUES.md#3-fit-model-systematic-gxana-run-systematics---study-fit-checked-2026-09-29));
the other columns reproduce the preserved files. Outputs under `systematics/`
are listed in the channel README, step 5.

### Measurements (chapter 6) and data/MC kinematics (chapters 4, 5)

The mass fit reads the plain data flat tree, which is not preserved; the
post-Q-factor tree holds the same entries and stands in for it
([`measurements/README.md`](../analyses/kpkpxim/measurements/README.md)).
After `gxana data stage`:

```bash
for stem in kpkpxim__M23_2017-01_ana56 kpkpxim__B4_M23_2018-01_ana03 kpkpxim__B4_M23_2018-08_ana02; do
  ln -s "$GXANA_OUTPUT/kpkpxim/qfactors/${stem}_nominal_kphighrap_1111111/postQVal_flatTree_${stem}_nominal_kphighrap_1111111.root" \
        "$GXANA_DATA/flatTrees/flatTree_${stem}_nominal_kphighrap.root"
done
uv run gxana config export --channel kpkpxim
uv run gxana run measurements --channel kpkpxim                # mass, lifetime, spin
uv run gxana run studies --channel kpkpxim --study kinematics  # data/MC kinematics
```

Fit results go to `measurements/`, figures to `prod_plots/` and
`data_mc_kinematics/`.

## 3. Full chain from the skims

Selection and simulation run on the JLab ifarm (`source env/setup.sh --gluex`,
[environment.md](environment.md)); every later stage runs anywhere. The order,
with the outputs that feed the next stage, is in the channel README
([pipeline](../analyses/kpkpxim/README.md#pipeline)):

| Step | Command | Dissertation |
|---|---|---|
| 0 | `gxana run mc --channel kpkpxim --period P --sample S` | ch. 5 (simulation) |
| 1 | `gxana run select --channel kpkpxim --period P --sample S [--thrown]` | ch. 4 (selection) |
| 2 | `flatTreePrep.C` (nominal cuts) | ch. 4, table of cuts |
| 3 | `gxana run qfactors --channel kpkpxim --period P` | ch. 4 (Q-factors) |
| 4 | `gxana run xsection --channel kpkpxim` (with `bin`) | ch. 6 |
| 5 | `gxana run systematics`, `gxana run barlow` | ch. 7 |
| 6 | `gxana run xsection --steps tex,figures` | ch. 6, 8 |
| 7 | `gxana run measurements --channel kpkpxim` | ch. 6 (mass, spin) |
| 8 | `gxana run studies --channel kpkpxim` | ch. 4 cut scans, ch. 5 kinematics |

Checkpoints: route 1 still applies while you run route 3. The Q-factor golden
test shows the engine reproduces the thesis Q-factors on preserved slices, and
the preserved binned trees and tables under `$GXANA_ANALYSIS_DATA/kpkpxim` are the
reference to compare your step-4 output with. The thesis used Q-factors with
`kDim: 200` and the newest MC reconstruction version sets of the time
([KNOWN_ISSUES.md](KNOWN_ISSUES.md#2-published-label-and-totals-reconciled-2026-09-29));
other MC changes the MC and thrown yields. No runtime for steps 0–3 is recorded
in the repository.

## Dissertation map

Status: **golden**, checked against preserved thesis output by the named test;
**runs**, shipped and documented but not compared; **macro**, a standalone ROOT
macro run as its folder README says; **not shipped**, no current code makes it.
Route is the cheapest route above that can make it. Commands below leave out
`--channel kpkpxim` for width; paths are under `$GXANA_OUTPUT/kpkpxim/`.

### Chapter 4: event selection and Q-factors

| Item | Made by | Output | Status | Route |
|---|---|---|---|---|
| Table of analysis cuts | `selection/flatTreePrep.C` ([cuts](../analyses/kpkpxim/README.md#nominal-selection)) | flat trees | runs | 3 |
| Q-factor example fit | `gxana run qfactors --period P` (per-event canvases in the work directory) | `histograms/` | not shipped as the named figure | 3 |
| Ξ mass and χ²/ndf with Q-factors | `selection/XimMassQVal.C` | `xim_qacc_*`, `chisqndf_qvalue_*` | macro (needs a `_tCut` Q-factor run) | 3 |
| Phase-I Ξ mass fit with residuals | `measurements/mass/MakeXim1320_IM_Res.C` | `prod_plots/Xim_InvariantMassFit_Phase1_residual_kphighrap.pdf` | macro | 2, with the link above |
| RF bunches | `selection/cut_studies/accidentals/get_data_hists.C` | `rfbunches_phase1.pdf` | macro, cannot run as preserved ([README](../analyses/kpkpxim/selection/cut_studies/README.md)) | 3 |
| χ²/ndf and MM² cut scans and fit grids | `gxana run studies --study chisqndf_scan,mm2_scan` | `cut_analysis_plots/results/` | runs | 3 |
| χ²/ndf and MM² data/MC panels | `selection/cut_studies/{chisqndf_cut,mm2_cut}/` macros | `*_data_mc_*_kphighrap.pdf` | macro | 3 |
| Kaon selection, rapidity, momentum and −t comparisons | `selection/cut_studies/{kaon_selection,rapidity_cuts}/` macros | see the [cut-studies README](../analyses/kpkpxim/selection/cut_studies/README.md) | macro | 3 |
| Vertex positions | `gxana run studies --study kinematics` | `data_mc_kinematics/*vertexZ_weighted_qvalue_acc_2018-08_*` | golden (`packages/studies/tests/python/test_kinematics_equivalence.py`, against the original macro) | 2 |
| Ξ and Λ path length and significance | `selection/cut_studies/{xim_vertex_cuts,lambda_vertex_cut}/` macros | `*_pathlen*_data_mc*` | macro | 3 |

### Chapter 5: simulation

| Item | Made by | Output | Status | Route |
|---|---|---|---|---|
| Final-state momenta, data vs MC and reconstructed vs generated | `gxana run studies --study kinematics` | `data_mc_kinematics/*_P3_weighted_qvalue_acc_2018-01_*` | runs (the golden test checks 2018-08) | 2 |
| Acceptance-corrected −t slope | `selection/mc_studies/make_plot_RF.C` | `tdist_phase1_data_thrown_ac_2d_kphighrap_log.pdf` (renamed from the dissertation file) | macro | 3 |
| Hyperon mass and decay angle | `selection/mc_studies/` macros | `{ystarM,costheta_gen_amp}_phase1_input_thrown_ac_2d_kphighrap.pdf`; the data/MC panels are not shipped | macro | 3 |

### Chapter 6: results

| Item | Made by | Output | Status | Route |
|---|---|---|---|---|
| Example MC and data yield fits, per-bin fits | `gxana run xsection --steps tables` (`xsection.fit_plots`) | `xsection/fits/johnson/` | runs | 2 |
| Data, MC, thrown yields and acceptance per period | `xsection/PlotXSecComponents.C`, after `--steps components` | `xsection/plots/*_runs_johnson.pdf` | macro (also draws labels the preserved route does not make) | 2 |
| dσ/dt per run period | `gxana run xsection --steps figures --systematics published` | `xsection/figures/diffxsec_runs_johnson.pdf` | golden (`test_xsec_figures_golden.py`) | 2 |
| dσ/dt weighted, with systematic band | same | `xsection/figures/diffxsec_phase1_systematics_johnson.pdf` | golden (same) | 2 |
| Total σ with CLAS | same | `xsection/figures/totxsec_clas_gluex_Phase1.pdf` | golden (same; χ²/ν = 0.74) | 2 |
| Spin β table and figures | `gxana run measurements --item spin` | `prod_plots/accepted_pimcostheta_*` | golden (`test_measurements_golden.py`, `test_measurements_stage_golden.py`) | 2 |
| Ξ mass figures and table | `gxana run measurements --item mass` | `prod_plots/XimIM_*` | golden (same) | 2, with the link above |
| Excited Ξ: cuts, Λ K⁻ mass fit, −t bins | `gxana run measurements --channel kpkpkmlamb` ([README](../analyses/kpkpkmlamb/README.md)) | `$GXANA_OUTPUT/kpkpkmlamb/measurements/Xi1820massFit*.pdf` | runs (toy-tested; no kpkpkmlamb data preserved); the first two −t bins are not reproduced | 3 |

### Chapter 7: systematics

| Item | Made by | Output | Status | Route |
|---|---|---|---|---|
| Track-efficiency table and figures | `gxana run systematics --study track --steps track` | `systematics/track/` | golden (`test_systematics_track_golden.py`) | 2 |
| Run-period ratios, significances, table | `gxana run systematics --steps runperiod` | `systematics/runperiod/` | golden against the original macro; does not reproduce the dissertation numbers ([§6](KNOWN_ISSUES.md#6-run-period-ratio-check-does-not-reproduce)) | 2 |
| Combo-selection (accidentals) spread | `gxana run systematics --study accidentals` | `systematics/accidentals/plots/` | golden (`test_systematics_plot_golden.py`, `test_systematics_numbers_golden.py`) | 2 |
| Fit-model examples and spreads | `gxana run systematics --study fit` | `systematics/fit/plots/` | drawing golden; numbers loose ([§3](KNOWN_ISSUES.md#3-fit-model-systematic-gxana-run-systematics---study-fit-checked-2026-09-29)) | 2 |
| One-RF-bunch comparison | `gxana run systematics --steps compare --study bunch` | none | not shipped (label and inputs absent) | — |
| REST-version comparison | none | none | not shipped ([§6](KNOWN_ISSUES.md#6-run-period-ratio-check-does-not-reproduce)) | — |
| Barlow variations table and figures | `gxana run barlow --channel kpkpxim` | `barlow/plots/barlow_*` | `weight` and `plot` golden (`test_systematics_text_golden.py`, `test_barlow_plot_golden.py`); `trees`, `bin`, `tables` need the raw flat trees | 3 |

### Chapter 8: summary tables

| Item | Made by | Output | Status | Route |
|---|---|---|---|---|
| dσ/dt and systematics tables | `gxana run xsection --steps tex --systematics published` | `xsection/tables/{,syst_}diffxsec_table_scale.tex` | golden (`test_xsection_reproduction_golden.py`, `test_python_golden.py`, `test_systematics_summary_golden.py`) | 2 |

## What will differ, and why

Every known disagreement with a published number is in
[KNOWN_ISSUES.md](KNOWN_ISSUES.md). The ones a rerun hits first:

- **Systematic column headings** in the chapter-8 table: the fit-model and
  accidental-subtraction spreads sit under each other's headings; numbers and
  totals are the published ones ([§4](KNOWN_ISSUES.md#4-accidentals-and-yield-extraction-columns-swapped-in-the-published-tables)).
- **Total-σ figure label**: the published tables are the label `johnson`, but
  the dissertation's total-σ figure was drawn from `hybrid_combo`, and the
  figure step keeps that ([§2](KNOWN_ISSUES.md#2-published-label-and-totals-reconciled-2026-09-29)).
- **Regenerated fit-model systematic** moves the fit column (§3 above).
- **Re-binned MC** moves the acceptance (§10 above); start from the binned trees.
- **Flux windows** are offset by one flux bin, kept as published
  ([§9](KNOWN_ISSUES.md#9-flux-windows-offset-by-one-flux-bin)).
- **Track-efficiency totals** are 18.65 % (20.36 % with the proton override)
  against the quoted 18.58 % (20.29 %)
  ([§5](KNOWN_ISSUES.md#5-track-efficiency-totals)).
- **Rapidity branch names**: the analysis-note `flatTreePrep.C` swapped
  `*_rapidity` and `*_prapidity`; the port's flat trees name them correctly, so
  flat trees from route 3 differ in those branches from older ones
  ([§1](KNOWN_ISSUES.md#1-rapidity--pseudorapidity-branch-swap-docshistoryrefactor_specmd-d18)).
- **Q-factor fits** that did not converge are used as they are
  ([§11](KNOWN_ISSUES.md#11-q-factor-fits-that-did-not-converge)).

## When something fails

- A stage exits 1 listing missing inputs: each line names the command that makes
  the file (`gxana data stage`, an earlier step, `select --thrown`). Use
  `--dry-run` to print the plan without running it.
- `gxana data stage` reports `conflict`: a different file already sits at the
  destination. Move it aside; stage never overwrites.
- A macro refuses a stale `channel.kv`: rerun `gxana config export --channel kpkpxim`.
- A golden test fails on fit columns only: compare the reported maximum
  deviation with the tolerance in the test docstring; ROOT versions differ
  (see [Before you start](#before-you-start)).
