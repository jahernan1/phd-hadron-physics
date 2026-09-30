# Nominal-versus-variant cross-section comparisons (dissertation chapters 6 and 7)

Each macro overlays the weighted differential cross section of the nominal
analysis with variants and prints or writes the spreads used as systematic
uncertainties. All are ROOT macros with an entry function named like the file.

## How the inputs are made

1. Each variant is one `gxana run xsection` label. The `tables` and `weight`
   steps write the weighted tables to
   `$GXANA_OUTPUT/kpkpxim/xsection/weighted_data/<label>/*diffxsec*.txt`.
2. `xsection/MakeWeightedDiffXSecTGraphs.C` (`MakeWeightedDiffXSecTGraphs()`)
   converts the text tables of `acc_weight`, `best_combo`, `hybrid_combo`,
   `qvalues` and `oneRfBunch` into TGraphErrors, one file
   `weighted_data/<label>/weighted_diffxsec.root` per label. For any other
   label, call its `CreateRootFileFromTextFiles(dir, "weighted_diffxsec.root")`
   after `.L` (the directory string must end with `/`).
3. The comparison macros open `WeightedDiffXSecTGraphs_<label>.root` from the
   current directory (the `fileDir` variable in the macros is not used). Copy
   each file under that name and run from the comparisons output directory:

   ```sh
   mkdir -p $GXANA_OUTPUT/kpkpxim/systematics/comparisons
   cd $GXANA_OUTPUT/kpkpxim/xsection
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/xsection/MakeWeightedDiffXSecTGraphs.C
   for l in acc_weight best_combo hybrid_combo; do
     cp weighted_data/$l/weighted_diffxsec.root $GXANA_OUTPUT/kpkpxim/systematics/comparisons/WeightedDiffXSecTGraphs_$l.root
   done
   cd $GXANA_OUTPUT/kpkpxim/systematics/comparisons
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/comparisons/PlotComboComparison.C
   ```

Plots go to `$GXANA_OUTPUT/kpkpxim/xsection/plots/<name>.pdf` (the directory
must exist); statistics text files go to the current directory.

## Labels each macro needs

Labels in `config/xsection.yaml` (`fits`, `weighted_labels`): `hybrid_combo`,
`best_combo`, `acc_weight`, `johnson`, `johnson_cheby1`, `voigt`,
`voigt_cheby1`, `mcPdf`, `mcPdf_cheby1`. `qvalues` comes from the opt-in
`qvalue` step (`data/qvalues/`); the `fitfigs` step weights it into
`weighted_data/qvalues/`.

| Macro | Entry | Input files (`WeightedDiffXSecTGraphs_*.root` unless noted) | Outputs | Feeds | Available from `config/xsection.yaml` |
|---|---|---|---|---|---|
| `PlotComboComparison.C` | `PlotComboComparison()` | `acc_weight`, `best_combo`, `hybrid_combo` | `weighted_diffxsec_ComboSelection.pdf`, `weighted_diffxsec_Combos_StdDev.pdf`, `combo_variations_stats.txt` (spread of `acc_weight` and `hybrid_combo`) | ch7 accidental-subtraction (combo) systematic | yes |
| `PlotFitComparison.C` | `PlotFitComparison()` | `hybrid_combo`, `johnson`, `qvalues`, `johnson_cheby1`, `voigt`, `voigt_cheby1`, `mcPdf`, `mcPdf_cheby1` | `weighted_diffxsec_AllFits.pdf`, `weighted_diffxsec_SignalFitVoigt.pdf`, `weighted_diffxsec_SignalFitMC.pdf`, `weighted_diffxsec_SignalFitJohn.pdf`, `fit_variations_stats.txt` | ch7 yield-extraction (fit model) systematic | yes: `gxana run xsection --steps ...,qvalue,fitfigs` (below) |
| `PlotFitBkgdComparison.C` | `PlotFitBkgdComparison()` | `hybrid_combo`, `bkgd` | `weighted_diffxsec_bkgdfit.pdf` | ch7 background model | no: `bkgd` label not configured |
| `PlotQValueComparison.C` | `PlotQValueComparison()` | `qvalues`, `hybrid_combo` | `weighted_diffxsec_QValYield.pdf` (Q-value yield versus UML-fit yield) | ch6/ch7 | `qvalues` needs the `qvalue` step |
| `PlotBunchComparison.C` | `PlotBunchComparison()` | `oneRfBunch`, `hybrid_combo`; per period `DiffXSecTGraphs_<S18 and F18 stem>_{oneRfBunch,hybrid_combo}.root` | `weighted_diffxsec_oneRFBunch.pdf`, S18/F18 `diffxsec_*_oneRFBunch` plots | ch7 RF-beam-bunch study | no: needs the `oneRfBunch` trees (`flatTreePrep.C` lists them) and their own label |
| `PlotRestVComparison.C` | `PlotRestVComparison()` | `s17_rest3`, `oneEBin`; per period `DiffXSecTGraphs_kpkpxim__M23_2017-01_ana45_s17_rest3.root`, `..._ana56_oneEBin.root` | `weighted_diffxsec_s17_rest3.pdf`, `diffxsec_s17_s17_rest3.pdf` | ch7 Spring-2017 REST-version check | no: needs the ana45 REST-3 trees and own labels |
| `PlotRunComparison.C` | `PlotRunComparison()` | per period `DiffXSecTGraphs_<stem>_hybrid_combo.root` for the three stems | `weighted_diffxsec_RunComparison.pdf`, `weighted_diffxsec_RunCompStdDevScaled.pdf`, `run_comp_stddev_scaled.txt` | ch6 result plot (per-period cross sections), ch7 run-period systematic | no: no macro in the repository writes the per-period `DiffXSecTGraphs_*` files |

`PlotFitComparison.C` throws unless all eight inputs exist and have the same
number of points. The opt-in `fitfigs` step of `gxana run xsection` runs it
end to end (`fit_figures` in `config/xsection.yaml`): it weights `qvalues`,
converts the eight labels to
`$GXANA_OUTPUT/kpkpxim/systematics/comparisons/WeightedDiffXSecTGraphs_<label>.root`,
runs the macro there and copies the six example fits of the dissertation
figure (`fit_figures.example_bin`) to
`$GXANA_OUTPUT/kpkpxim/xsection/plots/fit_examples/`:

```sh
gxana run xsection --channel kpkpxim --steps tables,weight,qvalue,fitfigs
```

Run `fitfigs` before `tex`: `tex` writes `syst_weighted_diffxsec_*.txt` into
`weighted_data/johnson/`, and `fitfigs` refuses a label directory holding
them.

## Dissertation tables

`fit_variations_stats.txt` (from `PlotFitComparison.C`) and
`combo_variations_stats.txt` (from `PlotComboComparison.C`) are the
`additional` inputs of the cross-section LaTeX table (`xsection.tex` in
`config/xsection.yaml`), which reads them from
`$GXANA_OUTPUT/kpkpxim/systematics/comparisons/`; `fitfigs` writes the first,
run `PlotComboComparison.C` from that directory for the second.
