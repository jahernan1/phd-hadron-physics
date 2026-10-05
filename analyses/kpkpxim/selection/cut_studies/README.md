# Cut studies (dissertation chapter 4)

Data-versus-MC studies behind the event-selection cuts, in the order of
chapter 4: kinematic-fit χ²/ndf, missing mass², Ξ⁻ and Λ path-length
significance, kaon selection, y(K⁺) rapidity cuts, accidental (RF-bunch)
subtraction.

## Run pattern

Every study is two steps, run from that study's output directory (macro names differ
per study; see each section; general conventions in
[`docs/MACROS_AND_OUTPUTS.md`](../../../../docs/MACROS_AND_OUTPUTS.md)):

```sh
mkdir -p $GXANA_OUTPUT/kpkpxim/analysis/event_selection/<study>
cd $GXANA_OUTPUT/kpkpxim/analysis/event_selection/<study>
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/selection/cut_studies/<study>/get_data_hists_RF.C
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/selection/cut_studies/<study>/make_plot.C
```

1. `get_data_hists_RF.C` (entry `get_data_hists_RF()`) fills histograms per run
   period (directories `Spring_2017`, `Spring_2018`, `Fall_2018` in the output
   file) for data and gen_amp_V2 signal MC and writes `data*.root` in the
   current directory.
2. `make_plot*.C` reads that file and writes the PDFs.

Variant files:

- `_RF` = the `gen_amp_V2` acceptance-weighted MC (`gen_amp_V2_ac_YstarRest`),
  the thesis MC. Use these.
- The files without `_RF` need an older MC sample (`Ystar2400_1600_genr8` or
  `gen_amp_..._Weighted`) that is neither preserved nor produced by `gxana`.

Every macro that cannot run on the preserved data is listed in
[`docs/analysis_data.md`](../../../../docs/analysis_data.md#macros-that-cannot-run-on-the-preserved-data).

Period stems: `kpkpxim__M23_2017-01_ana56`, `kpkpxim__B4_M23_2018-01_ana03`,
`kpkpxim__B4_M23_2018-08_ana02`.

Inputs and their producers (shared by all studies):

| Input | Producer |
|---|---|
| raw trees `$GXANA_DATA/Trees/flatTree/rawTrees/flatTree_<stem>.root` (data), `..._gen_amp_V2*.root` (MC) | `gxana run select` |
| `$GXANA_DATA/flatTrees/flatTree_<stem>_nominal[_<variant>].root` and `..._gen_amp_V2_ac_YstarRest_nominal[_<variant>].root` | `selection/flatTreePrep.C` (variants: none, `_ximVertexCut`, `_kphighrap`, `_rapidityCuts`) |
| `$GXANA_DATA/flatTrees/flatTree_thrown_<stem>_gen_amp_V2_ac_YstarRest.root` | `gxana run select --thrown` (written there directly) |
| `$GXANA_OUTPUT/kpkpxim/qfactors/<stem>_nominal<variant>_1111111/postQVal_flatTree_<stem>_nominal<variant>_1111111.root` | `gxana run qfactors` (standard run: variant `_kphighrap`) |

## 1. Kinematic-fit χ²/ndf (`chisqndf_cut/`)

- Data histograms: `get_data_hists_RF.C` reads the raw trees and applies
  `chisqndf<100 && xim_pathlensig>2 && lambda_pathlensig>0 && kphigh_rap>2`
  on the fly; writes `data_kphighrap_RF.root`. The MC file it reads is
  `flatTree_<stem>_gen_amp_V2.root` in `rawTrees/`; the produced MC sample is
  named `..._gen_amp_V2_ac_YstarRest`, so link it under the expected name.
- Plot: `make_plot_chisqndf.C` (`make_plot_chisqndf()`) reads
  `$GXANA_OUTPUT/kpkpxim/analysis/event_selection/chisqndf_cut/data_kphighrap_RF.root`
  and writes `chisqndf_data_mc_{2017,201801,201808}_kphighrap.pdf` (cut at 8).
- Figures: χ²/ndf data/MC distributions per run period.
- Not runnable: `get_data_hists.C` (legacy MC); `chisqndf_2017.C` (histogram dump, no entry function).

## 2. Missing mass² (`mm2_cut/`)

- `get_data_hists_RF.C` applies `beam_E > 6.4 && beam_E < 11.4`, `chisqndf<14`,
  `beam_vertexZ > 50.4 && beam_vertexZ < 79.1`,
  `xim_pathlensig>2 && lambda_pathlensig>0 && kphigh_rap>2` and
  `decayxim_M<1.334 && decayxim_M>1.308`, same raw-tree
  inputs and MC-name caveat as §1; writes `data_kphighrap_RF.root`.
- `make_plot.C` reads it from `.../event_selection/mm2_cut/` and writes
  `mm2_data_mc_{2017,201801,201808}_kphighrap.pdf`.
- Figures: |MM²| data/MC distributions. `get_data_hists.C`: legacy MC.

## 3. Ξ⁻ path-length significance (`xim_vertex_cuts/`)

- `get_data_hists_RF.C` cut `lambda_pathlensig>0 && kphigh_rap>2`; writes
  `data_kphighrap.root`.
- `make_plot_RF.C` (`make_plot_RF()`) writes
  `xim_pathlensig_data_mc_kphighrap_{2017,201801,201808}.pdf` and the
  `xim_pathlendiff[_nocut]_data_mc_kphighrap_*.pdf` variants.
- Needs the Q-factor output `<stem>_nominal_1111111` (the standard run is
  `_nominal_kphighrap`): run Q-factors once more with `qfactors.variant: _nominal`
  in `config/qfactors.yaml`.
- Not runnable: `get_data_hists.C` / `make_plot.C` (legacy genr8 MC).

## 4. Λ path-length significance (`lambda_vertex_cut/`)

Same layout as §3: `get_data_hists_RF.C` (cut `xim_pathlensig>2 &&
kphigh_rap>2`, writes `data_kphighrap.root`), then `make_plot.C`
(`make_plot()`), which writes `lambda_pathlensig_data_mckphighrap_<yr>.pdf`,
`lambda_pathlendiff_data_mckphighrap_<yr>.pdf` and
`lambda_pathlendiff_nocut_data_mckphighrap_<yr>.pdf` (`<yr>` = `2017`, `201801`, `201808`;
no separator before `kphighrap`).
Same `_nominal_1111111` Q-factor input note as §3. `get_data_hists.C`:
legacy MC.

## 5. Kaon selection (`kaon_selection/`)

- `get_data_hists_RF.C` (variants `_ximVertexCut` and `_kphighrap`) writes
  `data_ximVertexCut_RF.root` and `data_kphighrap_RF.root`.
- `make_plot.C` reads both from the current directory; it saves into
  `plots/`, which must exist (`mkdir plots`), as
  `plots/<hist>_{2017,201801,201808}<variant>.pdf`, and writes
  `kp1_kp2_p3<variant>_phase1.pdf` and `kp1_kp2_p3_mc<variant>_phase1.pdf`
  (K⁺ momentum, slow/fast separation) to the current directory.
- Not runnable: `get_data_hists.C` (`gen_amp_..._allKaonSep_Weighted` MC);
  `get_data_hists_ellipse.C` (legacy stems and Q-factor path).

## 6. y(K⁺) rapidity cuts (`rapidity_cuts/`)

- `get_data_hists.C` (`get_data_hists()`, gen_amp_V2 MC) writes
  `data_ximVertexCut.root` and `data_kphighrap.root`.
- Plot macros read those files from the current directory and save
  into `$GXANA_OUTPUT/kpkpxim/analysis/event_selection/rapidity_cuts/`
  `<hist>_data_mc.pdf` (`PlotKPlusLowRapidity.C`, `PlotKPlusHighRapidity.C`: slow/fast K⁺
  rapidity) or `<hist>_CutComparison.pdf` (`PlotKPlusLowComparison.C`,
  `PlotKPlusHighComparison.C`, `PlotKPlusMomSepComparison.C`, `PlotTDistComparison.C`:
  before/after the cut). Run each as in the pattern above.
- Name mismatches (entry function differs from the file name): load with
  `.L`, then call the function:

  ```sh
  cd $GXANA_OUTPUT/kpkpxim/analysis/event_selection/rapidity_cuts
  root -l -b -q -e ".x $GXANA_ROOT/rootlogon.C" -e ".L $GXANA_ROOT/analyses/kpkpxim/selection/cut_studies/rapidity_cuts/PlotKPlusLowP.C" -e "PlotKPlusComparison()"
  ```

  `PlotKPlusLowP.C` defines `PlotKPlusComparison()`;
  `PlotKPlusLowPComparison.C` defines `PlotKPlusHighRapidity()` (a copy of
  the function in `PlotKPlusHighRapidity.C`). Both read
  `data_rapidityCuts.root`, which `get_data_hists.C` does not write (add
  `make_root_tree("_rapidityCuts")`).

## 7. Accidentals and RF bunches (`accidentals/`)

- `get_data_hists.C` (`get_data_hists()`) builds `data_momCut.root`, then
  `make_histos()` reads `data_allKaonSep.root` (which no macro here writes) to save
  `rfbunches_phase1.pdf` and `rfbunches_mc_phase1.pdf` (beam-photon RF-time
  distribution, the RF-bunch figure); as preserved these PDFs are not produced.
- `make_plot.C` reads `.../event_selection/accidentals/data_momCut.root` and
  writes `plots/chisqndf_mc_combo_methods_momCut.pdf` and
  `plots/decayxim_M_mc_combo_methods_momCut.pdf` (`plots/` must exist)
  comparing the combo-selection methods.
- Not runnable: needs the `_nominal_momCut` variant (its `flatTreePrep.C` call is
  commented out) and `gen_amp_V2_2D` MC for all three periods (2018-08 only in
  `config/samples.yaml`).
