# Data versus acceptance-corrected MC (dissertation chapter 5)

Compares the Q-factor-weighted data with the gen_amp_V2 signal MC and with
the thrown (generated) distribution after acceptance correction: cos θ in the
helicity frame, M(Ξ⁻K⁺ slow) and t.

## Run

Runs on the JLab farm inputs only: the preserved copy of
`data_ac_ximVertexCut_hist2d_YstarRest.root` lacks `ResMassVsCosTheta_mc_Phase1` and
`ResMassVsCosTheta_thrown_Phase1`, so the macro stops with a segmentation fault on the
preserved data.

```sh
mkdir -p $GXANA_OUTPUT/kpkpxim/analysis/event_selection/mc_studies
cd $GXANA_OUTPUT/kpkpxim/analysis/event_selection/mc_studies
gxana config export --channel kpkpxim   # run periods and stems for the macro (channel.kv)
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/selection/mc_studies/get_data_hists_RF.C
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/selection/mc_studies/make_plot_RF.C
```

`get_data_hists_RF.C` reads `channel.kv`; every plot macro here calls the common style
presets, so run all of them through `rootlogon.C`
([`docs/MACROS_AND_OUTPUTS.md`](../../../../docs/MACROS_AND_OUTPUTS.md)).

## Inputs

For each period stem (`kpkpxim__M23_2017-01_ana56`,
`kpkpxim__B4_M23_2018-01_ana03`, `kpkpxim__B4_M23_2018-08_ana02`):

- Q-factor output
  `$GXANA_OUTPUT/kpkpxim/qfactors/<stem>_nominal_kphighrap_1111111/postQVal_flatTree_<stem>_nominal_kphighrap_1111111.root`
  (`gxana run qfactors`);
- MC `$GXANA_DATA/flatTrees/flatTree_<stem>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root`
  (`gxana run select` then `selection/flatTreePrep.C`);
- thrown MC `$GXANA_DATA/flatTrees/flatTree_thrown_<stem>_gen_amp_V2_ac_YstarRest.root`
  (`gxana run select --thrown` writes it there);
- the 2-D sampling file `$GXANA_OUTPUT/kpkpxim/MC/data_ac_ximVertexCut_hist2d_YstarRest.root`,
  read after the fills for `ResMassVsCosTheta_Phase1_ac`, `ResMassVsCosTheta_qval_Phase1`,
  `ResMassVsCosTheta_mc_Phase1` and `ResMassVsCosTheta_thrown_Phase1`. `sampling/PrepSampling.C`
  writes these four names (into its own run directory, `simulation/sampling`; place the file
  under `kpkpxim/MC/`), but the thesis-production copy under `simulation/sampling/` of the
  preserved data lacks the last two.

## Outputs

- `get_data_hists_RF.C` (`get_data_hists_RF()`): `data_ac_hist2d_kphighrap_2d.root`
  in the current directory (per-period and merged data, MC and thrown 2-D
  histograms and their acceptance-corrected versions).
- `make_plot_RF.C` (`make_plot_RF()`): PDFs in the current directory,
  including `costheta_gen_amp_phase1_data_mc_2d_kphighrap.pdf`,
  `ystarM_phase1_data_mc_2d_kphighrap.pdf`,
  `costheta_gen_amp_phase1_data_thrown_ac_2d_kphighrap.pdf`,
  `ystarM_phase1_data_thrown_ac_2d_kphighrap.pdf`,
  `tdist_phase1_data_thrown_ac_2d_kphighrap.pdf`, the input-versus-thrown
  checks `*_input_thrown_ac_2d_kphighrap.pdf`, and the 2-D maps
  (`CosThetaVsMass_*`, `costheta_ystar*`, `t_dist_accept_phase1`).
- Feeds chapter 5 (data/MC agreement and acceptance-corrected cos θ,
  M(Ξ⁻K⁺) and t distributions).

## Not runnable as preserved

Also listed in [`docs/analysis_data.md`](../../../../docs/analysis_data.md#macros-that-cannot-run-on-the-preserved-data).

- `get_data_hists.C`, `make_plot.C`: need the `gen_amp_..._allKaonSep_Weighted` MC trees and `data.root`.
- `make_plot_acceptcorr.C`: needs `data_RF.root`, which no macro here writes
  (`get_data_hists_RF.C` writes `data_ac_hist2d_kphighrap_2d.root`).
