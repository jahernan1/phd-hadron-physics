# Ξ⁻(1320) measurements

Mass, lifetime and spin (decay-angle) measurements of the Ξ⁻(1320) from the
γp → K⁺K⁺Ξ⁻ sample (lifetime and spin Q-factor weighted; dissertation
chapter 6, Ξ⁻ properties; the invariant-mass fit figure is in chapter 4).

Each measurement is a prep step (fills the per-period histograms from the
flat trees and writes them to a ROOT file in the current directory) followed by
a fit step (reads that file, fits each period and writes the plots). The three
run periods are the stems `kpkpxim__M23_2017-01_ana56`,
`kpkpxim__B4_M23_2018-01_ana03` and `kpkpxim__B4_M23_2018-08_ana02`, stored in
the output files under `Spring_2017`, `Spring_2018` and `Fall_2018`.

Inputs (each recipe lists the ones it reads):

- data: `$GXANA_DATA/flatTrees/flatTree_<stem>_nominal_kphighrap.root`
  (`selection/flatTreePrep.C`; weight `hybrid_combo`). Not in the preserved
  data: the post-Q-factor tree below holds the same entries and can be linked
  under this name (see `docs/KNOWN_ISSUES.md`, section 14);
- Q-weighted data: `$GXANA_OUTPUT/kpkpxim/qfactors/<stem>_nominal_kphighrap_1111111/postQVal_flatTree_<stem>_nominal_kphighrap_1111111.root`
  (`gxana run qfactors`; weight `hybrid_combo * qvalue_decayxim_M`);
- reconstructed MC: `$GXANA_DATA/flatTrees/flatTree_<stem>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root`
  (weight `hybrid_combo`);
- thrown MC: `$GXANA_DATA/flatTrees/flatTree_thrown_<stem>_gen_amp_V2_ac_YstarRest.root`
  (copied from `rawTrees/` as in the channel README).

The run periods (output directory names and tree stems) come from
`$GXANA_OUTPUT/kpkpxim/config/channel.kv`: run `gxana config export --channel kpkpxim`
once before the macros, and again after any edit of `config/*.yaml` (the macros refuse a
stale file).

All three measurements run through the stage, which creates `$GXANA_OUTPUT/kpkpxim/measurements`
and `prod_plots` and runs each prep macro, then each fit macro, from the measurements directory
(`config/measurements.yaml`; `--item mass,spin` and `--steps prep|fit` select, `--dry-run` prints
the commands):

```sh
gxana config export --channel kpkpxim
gxana run measurements --channel kpkpxim
```

The recipes below run the same macros by hand.

Every prep and fit entry takes `n_threads` (default 4, the implicit
multithreading the original combined macro ran with; in `PlotGlueXSpin.C` it
applies to the per-period fits only, the merged fit runs without it as in the
original). The recipes use the default. With multithreaded filling the
summation order changes from run to run, so the last digits of the fit errors vary between runs; for the mass fit
some errors vary up to the percent level. Passing `0` (for example
`root -l -b -q $GXANA_ROOT/rootlogon.C "$GXANA_ROOT/analyses/kpkpxim/measurements/mass/PrepMass.C(0)"`)
turns implicit multithreading off and gives reproducible numbers;
`tests/golden/test_measurements_golden.py` runs all six entries that way (and
`tests/golden/test_measurements_stage_golden.py` through the stage) and compares the fit
results with a single-threaded run of the original macros.

## Mass: `mass/PrepMass.C`, `mass/FitMass.C`

Prep fills M(Λπ⁻) per period for the data, reconstructed MC (both weighted by
`hybrid_combo`) and thrown MC. There is no acceptance correction. Fit runs the
original RooFit fits: a Johnson-SU signal on the reconstructed MC gives the
MC→PDG shift (PDG mass minus MC mean), and the data fit (Johnson signal,
Gaussian Ξ(1530), Chebychev background) reports the mass as data mean plus
that shift. The shift is applied to the data mean only. Inputs: data,
reconstructed MC, thrown MC. Run (prod_plots directory must exist):

```sh
gxana config export --channel kpkpxim
mkdir -p $GXANA_OUTPUT/kpkpxim/prod_plots $GXANA_OUTPUT/kpkpxim/measurements
cd $GXANA_OUTPUT/kpkpxim/measurements
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/mass/PrepMass.C
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/mass/FitMass.C
```

Outputs: `xim_mass.root` (histograms; FitMass writes the fit canvases back into
it), `prod_plots/XimIM_mc_<stem>.pdf`, `prod_plots/XimIM_dataCorr_<stem>.pdf`;
fit result lines `FITRESULT mass_mc ...` and `FITRESULT mass_data ...` on
stdout.

## Lifetime: `lifetime/PrepLifetime.C`, `lifetime/FitLifetime.C`

Prep fills the Q-weighted data, `hybrid_combo`-weighted reconstructed-MC and
thrown-MC Ξ⁻ rest-frame lifetime per period, builds ε = reco/thrown (binomial
errors) with `gxana::AcceptanceCorrect`, and corrects the data. Fit runs the
original `expo` fit (0.02–0.7 ns, "WLR", τ = −1/slope) per period. The
PDG−MC-mean lifetime value is computed in the code but not applied. Inputs:
Q-weighted data, reconstructed MC, thrown MC. Run (prod_plots directory must
exist):

```sh
gxana config export --channel kpkpxim
mkdir -p $GXANA_OUTPUT/kpkpxim/prod_plots $GXANA_OUTPUT/kpkpxim/measurements
cd $GXANA_OUTPUT/kpkpxim/measurements
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/lifetime/PrepLifetime.C
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/lifetime/FitLifetime.C
```

Outputs: `xim_lifetime.root`, `prod_plots/accepted_ximlifetime_<stem>.pdf`;
fit result lines `FITRESULT lifetime ...` on stdout.

## Spin: `spin/PrepSpinData.C`, `spin/PlotGlueXSpin.C`

Prep fills the Q-weighted data, `hybrid_combo`-weighted reconstructed-MC and
thrown-MC π⁻ cos θ (helicity frame) per period, builds ε = reco/thrown
(binomial errors) with `gxana::AcceptanceCorrect`, corrects the data and
merges the three corrected periods into `pim_costheta_hf_phase1`, with the
merged acceptance `pim_costheta_hf_avg_accept_phase1` (only drawn). Fit
(`PlotGlueXSpin.C`) fits the merged distribution with 1 + βx (J = 1/2; β
limited to [0, 1] by the fit-parameter limit) and with the J = 3/2 shape
1 + 3x² + β x (5 − 9x²) with β fixed to the J = 1/2 value, compares χ²/ndf,
then fits each period with the original per-period fit function. Inputs:
Q-weighted data, reconstructed MC, thrown MC. Run (prod_plots directory must
exist):

```sh
gxana config export --channel kpkpxim
mkdir -p $GXANA_OUTPUT/kpkpxim/prod_plots $GXANA_OUTPUT/kpkpxim/measurements
cd $GXANA_OUTPUT/kpkpxim/measurements
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/spin/PrepSpinData.C
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/spin/PlotGlueXSpin.C
```

Outputs: `xim_spin.root` (per-period histograms and the merged
`pim_costheta_hf_phase1`, `pim_costheta_hf_avg_accept_phase1`),
`prod_plots/accepted_pimcostheta_gluex_phase1.pdf` and
`prod_plots/accepted_pimcostheta_<stem>.pdf`; fit result lines
`FITRESULT spin ...` on stdout.

## Ξ⁻(1320) invariant-mass fit figure (chapter 4): `mass/MakeXim1320_IM*.C`

`MakeXim1320_IM_Res()` fits M(Λπ⁻) of the three periods'
`flatTree_<stem>_nominal_kphighrap.root` from `$GXANA_DATA/flatTrees/` with a
residual panel. Output:
`$GXANA_OUTPUT/kpkpxim/prod_plots/Xim_InvariantMassFit_Phase1_residual_kphighrap.pdf`
(the directory must exist). It reads the plain data tree, which is not in the
preserved data (see Inputs).
These macros are separate from the mass measurement (they fit the combined
Phase-I spectrum for the chapter 4 figure) and keep their own fit code.

```sh
gxana config export --channel kpkpxim
cd $GXANA_OUTPUT/kpkpxim/measurements
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/mass/MakeXim1320_IM_Res.C
```

## Cannot run as preserved

- `mass/MakeXim1320_IM.C` (`MakeXim1320_IM()`, delims `_ximVertexCut` then
  `_kphighrap`): reads `flatTree_<stem>_nominal_ximVertexCut.root`, which is not
  in the preserved data, and stops there, so neither
  `Xim_InvariantMassFit_Phase1_ximVertexCut.pdf` nor
  `Xim_InvariantMassFit_Phase1_kphighrap.pdf` is written.
- `mass/MakeXim1820_IM.C` (excited Ξ, chapter 6): reads
  `$GXANA_DATA/KpKpKmL012017012018082018Real_31July.root`, a hand-made tree
  that is not in the preserved data; its PDF `Print` is commented out.
