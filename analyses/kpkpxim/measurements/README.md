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
  under this name (see `docs/history/PORT_NOTES.md`, section 9);
- Q-weighted data: `$GXANA_OUTPUT/kpkpxim/qfactors/<stem>_nominal_kphighrap_1111111/postQVal_flatTree_<stem>_nominal_kphighrap_1111111.root`
  (`gxana run qfactors`; weight `hybrid_combo * qvalue_decayxim_M`);
- reconstructed MC: `$GXANA_DATA/flatTrees/flatTree_<stem>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root`
  (weight `hybrid_combo`);
- thrown MC: `$GXANA_DATA/flatTrees/flatTree_thrown_<stem>_gen_amp_V2_ac_YstarRest.root`
  (`gxana run select --thrown` writes it there).

The prep and fit macros open these through `common/XimInputs.h` (`XimPeriods`,
`XimDataFrame`, `XimQvalFrame`, `XimMcFrame`, `XimThrownFrame`, `XimOpen`, `setStyle`).

The prep and fit macros read the run periods (output directory names and tree stems) from
`$GXANA_OUTPUT/kpkpxim/config/channel.kv`; `MakeXim1320_IM*.C` has its stems in the source.
Generic macro conventions: [docs/MACROS_AND_OUTPUTS.md](../../../docs/MACROS_AND_OUTPUTS.md).

All three measurements run through the stage, which creates `$GXANA_OUTPUT/kpkpxim/measurements`
and `prod_plots` and runs each prep macro, then each fit macro, from the measurements directory
(`config/measurements.yaml`; `--item mass,spin` and `--steps prep|fit` select, `--dry-run` prints
the commands). It does not write `channel.kv`:

```sh
gxana config export --channel kpkpxim
gxana run measurements --channel kpkpxim
```

The recipes below run the same macros by hand, with the macro default thread count
(see [Reproducibility](#reproducibility)). Set up once (again `gxana config export` after any
edit of `config/*.yaml`; the macros refuse a stale `channel.kv`):

```sh
gxana config export --channel kpkpxim
mkdir -p $GXANA_OUTPUT/kpkpxim/prod_plots $GXANA_OUTPUT/kpkpxim/measurements
cd $GXANA_OUTPUT/kpkpxim/measurements
```

## Mass: `mass/PrepMass.C`, `mass/FitMass.C`

Prep fills M(Λπ⁻) per period for the data, reconstructed MC (both weighted by
`hybrid_combo`) and thrown MC. There is no acceptance correction. Fit runs the
original RooFit fits: a Johnson-SU signal on the reconstructed MC gives the
MC→PDG shift (PDG mass minus MC mean), and the data fit (Johnson signal,
Gaussian Ξ(1530), Chebychev background) reports the mass as data mean plus
that shift. The shift is applied to the data mean only. Inputs: data,
reconstructed MC, thrown MC. Run:

```sh
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
Q-weighted data, reconstructed MC, thrown MC. Run:

```sh
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
Q-weighted data, reconstructed MC, thrown MC. Run:

```sh
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
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/mass/MakeXim1320_IM_Res.C
```

## Reproducibility

Every prep and fit entry takes `n_threads` (default 4, the implicit
multithreading the original combined macro ran with; in `PlotGlueXSpin.C` it
applies to the per-period fits only, the merged fit runs without it as in the
original). `mass/MakeXim1320_IM.C` and `mass/MakeXim1320_IM_Res.C` carry a
forward declaration of `GetXim1320_IM` with `n_threads = 8` (the legacy
declaration, kept as is) and call it before its definition, so those two
entries run with 8 threads. Pass the thread count explicitly to avoid relying
on either default.

With multithreaded filling the summation order changes from run to run, so the
last digits of the fit errors vary between runs; for the mass fit some errors
vary up to the percent level. Passing `0` turns implicit multithreading off and
gives reproducible numbers:

```sh
root -l -b -q $GXANA_ROOT/rootlogon.C "$GXANA_ROOT/analyses/kpkpxim/measurements/mass/PrepMass.C(0)"
```

The shipped `config/measurements.yaml` runs every macro with `n_threads = 0`
(`args: [0]`), so `gxana run measurements` is single-threaded.
`tests/golden/test_measurements_golden.py` runs all six entries that way (and
`tests/golden/test_measurements_stage_golden.py` through the stage) and compares
the fit results with a single-threaded run of the original macros.

## Kept behaviour

The prep and fit macros split the combined `GetXimProperties.C` (archived in
`archive/root_macros/`) and `PlotGlueXSpin.C`. Fit models, ranges, binning,
weights, names and plot texts are those of the originals; the fit macros add
one `FITRESULT` line per fit. These points are original behaviour, reproduced,
not fixed (dissertation-number differences: `docs/KNOWN_ISSUES.md` section 7):

- Lifetime (`FitLifetime.C`): the shift `PDG − MC mean` is computed but never
  applied to τ or written out; τ is the acceptance-corrected data fit only.
- Mass (`FitMass.C`): the mass is the data mean plus `1.32171 − mean(MC fit)`;
  the shift is applied to the data mean only, σ is not shifted. `thrown_mass`
  (centre of the thrown histogram's peak bin, printed) and `pdg_mass` are
  computed but not used. The data Johnson fit starts from fixed numbers
  (`xiParams`), not from the MC fit result, with `gamma` and `delta` constant.
  The printed σ error uses the legacy formula with `delta/delta_err`
  (`gxana::fit::Moments`, `packages/fit/include/gxana/fit/Johnson.h`), so it is
  `inf` for the data fits; it is only printed. The canvases are written without
  `kOverwrite`: rerunning `FitMass.C` on an existing `xim_mass.root` without
  `PrepMass.C` adds cycles (`fitCan;2`, ...); readers get the highest cycle.
- Spin (`PlotGlueXSpin.C`): the result of record is the fit of the merged
  `pim_costheta_hf_phase1`. The per-period plots keep the original per-period
  copy `GetSpinAnalysisPeriod`: no free J = 3/2 fit, the fixed-β J = 3/2 fit
  starts from default parameters, χ²/ndf drawn as text instead of a legend.
  Unifying the two gives the same fit values but changes the per-period plots.
  The free J = 3/2 fit (`beta1` in the merged `FITRESULT` line) sits at its
  lower limit 0.
- Merged acceptance (`PrepSpinData.C`): `pim_costheta_hf_avg_accept_phase1` is
  the sum of the three period acceptances, not their mean (`TH1::kIsAverage` is
  set on the first histogram only, and `TH1::Add` averages only when both
  operands carry it). It is only drawn; no fit depends on it.

## Cannot run as preserved

Listed with the other channel macros in
[docs/analysis_data.md](../../../docs/analysis_data.md#macros-that-cannot-run-on-the-preserved-data).

- `mass/MakeXim1320_IM.C` (`MakeXim1320_IM()`): needs
  `flatTree_<stem>_nominal_ximVertexCut.root`; stops there, so neither
  `Xim_InvariantMassFit_Phase1_{ximVertexCut,kphighrap}.pdf` is written.
- `mass/MakeXim1820_IM.C` (excited Ξ, chapter 6): needs the hand-made tree
  `$GXANA_DATA/KpKpKmL012017012018082018Real_31July.root`; its PDF `Print` is
  commented out.
