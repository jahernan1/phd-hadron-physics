# MC-model weight variations (dissertation chapter 7)

Study of the sensitivity of the acceptance to the angular model in the signal
MC: for five cut variations, the acceptance-corrected Ξ⁻ cos θ (helicity
frame) distribution of the data (`xim_costheta_hf_all_acceptcorr`) is fitted,
and the MC and thrown events are reweighted to the fit.

This study is not part of `gxana run systematics`; its MC is not preserved.

## Macros

| Macro | Entry | Role |
|---|---|---|
| `get_data_hists.C` | `get_data_hists()` | For each variation delim (`_chisqndf+1`, `_kp_momsep_+05`, `_lambda_pathlensig+05`, `_total_mm2+005`, `_xim_pathlensig-05`) builds `data<delim>.root` (current directory) with per-period data, MC and thrown histograms. |
| `WeightMC.C` | `WeightMC(Bool_t save=true)` | Reads `data<delim>.root`, fits that distribution, and writes reweighted MC and thrown trees `*_Weighted.root` / `*_vary<delim>_Weighted.root` into `$GXANA_OUTPUT/kpkpxim/systematics/root_trees/`. |

## Run

```sh
mkdir -p $GXANA_OUTPUT/kpkpxim/systematics/mc_weight_variations
cd $GXANA_OUTPUT/kpkpxim/systematics/mc_weight_variations
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/mc_weight_variations/get_data_hists.C
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/mc_weight_variations/WeightMC.C
```

## Cannot run as preserved

- Inputs are `$GXANA_OUTPUT/kpkpxim/systematics/root_trees/flatTree_<stem>_vary<delim>.root`
  and `flatTree_<stem>_Ystar2400_1600_genr8_vary<delim>.root` plus
  `$GXANA_DATA/Trees/flatTree/rawTrees/flatTree_thrown_<stem>_Ystar2400_1600_genr8.root`.
  The `Ystar2400_1600_genr8` MC sample is older than the thesis
  `gen_amp_V2_ac_YstarRest` sample and is not preserved; the `_vary<delim>`
  tree naming is that of the legacy variation-tree step and was not checked
  against `config/barlow.yaml` (legacy `archive/systematics_legacy/GetVariationTreesUML.C`).
- `WeightMC.C` defaults name a `ver56` stem that no longer exists.
