# Ξ⁻(1320) measurements

Mass, lifetime and spin (decay-angle) measurements of the Ξ⁻(1320) from the
Q-factor-weighted γp → K⁺K⁺Ξ⁻ sample (dissertation chapter 6, Ξ⁻ properties;
the invariant-mass fit figure is in chapter 4).

## Mass, lifetime, decay angle: `mass/GetXimProperties.C`

Entry `GetXimProperties()`. For each period it fits M(Λπ⁻) in data and MC,
derives the mass correction, compares the Ξ⁻ rest-frame lifetime of data, MC
and thrown MC, and fills the acceptance-corrected π⁻ decay-angle histogram
(cos θ in the helicity frame), then merges the three periods.

Inputs (period stems `kpkpxim__M23_2017-01_ana56`,
`kpkpxim__B4_M23_2018-01_ana03`, `kpkpxim__B4_M23_2018-08_ana02`):

- `$GXANA_DATA/flatTrees/flatTree_<stem>_nominal_kphighrap.root`
  (`selection/flatTreePrep.C`);
- `$GXANA_OUTPUT/kpkpxim/qfactors/<stem>_nominal_kphighrap_1111111/postQVal_flatTree_<stem>_nominal_kphighrap_1111111.root`
  (`gxana run qfactors`);
- `$GXANA_DATA/flatTrees/flatTree_<stem>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root`
  and `$GXANA_DATA/flatTrees/flatTree_thrown_<stem>_gen_amp_V2_ac_YstarRest.root`
  (MC; thrown tree copied from `rawTrees/` as in the channel README).

Run (the `prod_plots` directory must exist):

```sh
mkdir -p $GXANA_OUTPUT/kpkpxim/prod_plots $GXANA_OUTPUT/kpkpxim/measurements
cd $GXANA_OUTPUT/kpkpxim/measurements
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/mass/GetXimProperties.C
```

Outputs: `xim1320_properties.root` (current directory; per-period directories
`Spring_2017`, `Spring_2018`, `Fall_2018` and the merged
`pim_costheta_hf_phase1`, `pim_costheta_hf_avg_accept_phase1`) and, in
`$GXANA_OUTPUT/kpkpxim/prod_plots/`, `XimIM_dataCorr_*.pdf`,
`XimIM_mc_*.pdf`, `accepted_ximlifetime_*.pdf`, `accepted_pimcostheta_*.pdf`.

## Spin: `spin/PlotGlueXSpin.C`

Entry `PlotGlueXSpin()`. Reads `xim1320_properties.root` from the current
directory (run it in the same directory after `GetXimProperties.C`) and fits
the acceptance-corrected π⁻ cos θ distribution with 1 + βx (J = 1/2; β limited
to [0, 1] by the fit-parameter limit) and with the J = 3/2 shape
1 + 3x² + β x (5 − 9x²) with β fixed to the J = 1/2 value, and compares
χ²/ndf. Output: `$GXANA_OUTPUT/kpkpxim/prod_plots/accepted_pimcostheta_gluex_phase1.pdf`.

```sh
cd $GXANA_OUTPUT/kpkpxim/measurements
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/spin/PlotGlueXSpin.C
```

The same J = 1/2 vs 3/2 fit code is also inside `GetSpinAnalysis` of
`mass/GetXimProperties.C` and `mass/Xim1320Properties.cpp`.

## Ξ⁻(1320) invariant-mass fit figure (chapter 4): `mass/MakeXim1320_IM*.C`

`MakeXim1320_IM()` (entry, delims `_ximVertexCut` and `_kphighrap`) fits
M(Λπ⁻) of the three periods' `flatTree_<stem>_nominal<delim>.root` from
`$GXANA_DATA/flatTrees/`; `MakeXim1320_IM_Res()` does the same for
`_kphighrap` with a residual panel. Outputs in
`$GXANA_OUTPUT/kpkpxim/prod_plots/` (must exist):
`Xim_InvariantMassFit_Phase1<delim>.pdf` and
`Xim_InvariantMassFit_Phase1_residual<delim>.pdf`.

```sh
cd $GXANA_OUTPUT/kpkpxim/measurements
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/mass/MakeXim1320_IM.C
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/measurements/mass/MakeXim1320_IM_Res.C
```

## Cannot run as preserved

- `mass/MakeXim1820_IM.C` (excited Ξ, chapter 6): reads
  `$GXANA_DATA/KpKpKmL012017012018082018Real_31July.root`, a hand-made tree
  that is not in the preserved data; its PDF `Print` is commented out.
- `mass/PlotXim1320Properties.C`: calls `GetXim1320_IM(rootName)` declared in
  `Xim1320Properties.h`, which is never defined.
- `mass/Xim1320Properties.cpp` / `.h`: an older near-duplicate of
  `GetXimProperties.C`, superseded by it.
