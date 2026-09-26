# kpkpxim

## Physics

This channel measures the GlueX Phase-I differential and total cross section
for γp → K⁺K⁺Ξ⁻, reconstructed from the K⁺K⁺Ξ⁻ final state (Ξ⁻ → π⁻Λ,
Λ → pπ⁻) across the three GlueX Phase-I run periods analyzed here: Spring
2017 (S17), Spring 2018 (S18) and Fall 2018 (F18).

## Directory map

- `selectors/` — DSelector source compiled and run by `gxana run select`.
- `selection/` — flat-tree production (`flatTreePrep.C`) and cut studies
  (`cut_studies/`, `mc_studies/`, `tests/`).
- `backgrounds/` — background-channel selectors and studies
  (`backgrounds/selectors/`).
- `signal_extraction/qfactors/` — Q-factor run README and helper scripts (fit
  models live in the QFactors fork, `packages/qfactors`); single-fit
  lineshape studies in `signal_extraction/lineshape/`.
- `xsection/` — binning, weighting and plotting macros for the differential
  and total cross section, flux input in `xsection/flux/`, and published
  external comparison data in `xsection/external_data/`.
- `systematics/` — Barlow-variation cross-section pipeline: UML variation
  trees, per-variation cross sections and weighting, Barlow plots
  (`systematics/barlow/`), nominal-vs-variation comparison plots
  (`systematics/comparisons/`), track efficiency
  (`systematics/track_efficiency/`) and MC-weight variations
  (`systematics/mc_weight_variations/`).
- `simulation/` — thesis signal-MC inputs (gen_amp_V2 cfgs, MCwrapper
  conf templates, hd_root configs, genr8 inputs) and sampling/validation
  macros; see `simulation/README.md`.
- `config/` — channel configuration consumed by `gxana` (`mc.yaml` drives
  `gxana run mc`).

## Pipeline

0. Simulation (signal MC, JLab farm): build the patched halld_sim, then
   `source env/setup.sh --sim=<set>` and
   `gxana run mc --channel kpkpxim --period P --sample S`
   (see [`simulation/README.md`](simulation/README.md)).

1. Select events from the skim into per-period, per-sample flat trees:

   ```sh
   gxana run select --channel kpkpxim --period P --sample S
   gxana run select --channel kpkpxim --period P --sample S --thrown
   ```

2. Apply the nominal analysis cuts and write the flat trees used downstream:

   ```sh
   root -l -b -q rootlogon.C 'analyses/kpkpxim/selection/flatTreePrep.C("flatTree_<stem>")'
   ```

3. Compute Q-factor signal weights per period (QFactors fork at
   `packages/qfactors`, run config `config/qfactors.yaml`; details in
   `signal_extraction/qfactors/README.md`):

   ```sh
   gxana run qfactors --channel kpkpxim --period P
   ```

   The output `$GXANA_OUTPUT/kpkpxim/qfactors/<stem>_nominal_kphighrap_1111111/postQVal_flatTree_*.root`
   is the data input of step 4.

4. Bin, fit, weight and split the cross-section tables (default steps:
   `bin,tables,weight,components`). The `bin` step reads the thrown MC flat
   trees from `$GXANA_DATA/flatTrees/` (`xsection.inputs.thrown`), but
   `gxana run select --thrown` writes them to
   `$GXANA_DATA/Trees/flatTree/rawTrees/` and `flatTreePrep.C` does not touch
   them, so copy them over first (`xsection.mc_sample` is
   `gen_amp_V2_ac_YstarRest`):

   ```sh
   mkdir -p $GXANA_DATA/flatTrees
   cp -p $GXANA_DATA/Trees/flatTree/rawTrees/flatTree_thrown_*_gen_amp_V2_ac_YstarRest.root $GXANA_DATA/flatTrees/
   gxana run xsection --channel kpkpxim
   ```

   The `tables` step writes each fit label into
   `$GXANA_OUTPUT/kpkpxim/xsection/data/<label>/`, which `weight` and
   `components` read.

   `qvalue` (Q-value rescaling) is opt-in, not run by default: it reads
   `data/<xsection.qvalue_label>/diffout*.txt` and `diffxsec*.txt`, a
   directory the `tables` step only populates for one of the `fits` labels
   in `analyses/kpkpxim/config/xsection.yaml` (e.g. `johnson`), not the
   default `qvalue_label: hybrid_combo`. Set `qvalue_label` to a populated
   `fits` label, then add the step explicitly:

   ```sh
   gxana run xsection --channel kpkpxim --steps bin,tables,weight,components,qvalue
   ```

5. Plot the differential and total cross section:

   ```sh
   cd $GXANA_OUTPUT/kpkpxim/xsection && root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/xsection/PlotDiffXSec.C
   ```

6. Systematics: build the per-variation trees, split them into the
   cross-section bins (`SplitVariationTrees.C` writes the
   `variation_trees/binned_*_variations.root` files `GetXSecFilesUML.C`
   reads), the per-variation cross-section tables, the run-period weighted
   average, and the Barlow-significance plots (see
   [Barlow variations](#barlow-variations) below for the cut list). The
   macros do not create their output directories, and `GetXSecFilesUML.C`
   reads the binned thrown trees from `$GXANA_DATA/flatTrees/`, where step 4's
   `bin` step does not write them, so create the directories and copy the
   thrown trees first:

   ```sh
   mkdir -p $GXANA_OUTPUT/kpkpxim/systematics/variation_trees $GXANA_OUTPUT/kpkpxim/systematics/fits $GXANA_OUTPUT/kpkpxim/systematics/xsection_data
   cp -p $GXANA_OUTPUT/kpkpxim/xsection/binned_trees/binned_thrown_flatTree_*_gen_amp_V2_ac_YstarRest.root $GXANA_DATA/flatTrees/
   cd $GXANA_OUTPUT/kpkpxim/systematics
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/GetVariationTreesUML.C
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/SplitVariationTrees.C
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/GetXSecFilesUML.C
   python3 $GXANA_ROOT/analyses/kpkpxim/systematics/GetWeightedXsecFile.py
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/barlow/PlotXSecBarlowChiSqNdf.C
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/barlow/PlotXSecBarlowMissingMass.C
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/barlow/PlotXSecBarlowKHighRapidity.C
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/barlow/PlotXSecBarlowKLowRapidity.C
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/barlow/PlotXSecBarlowXimFlightSig.C
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/barlow/PlotXSecBarlowLambdaFlightSig.C
   ```

## Nominal selection

`analyses/kpkpxim/selection/flatTreePrep.C` applies, in order:

- `beam_E > 6.4 && beam_E < 11.4` (photon beam energy, GeV)
- `chisqndf < 8` (`vec_cuts[0]`, combo χ²/ndf)
- `abs(total_mm2) < 0.02` (missing-mass squared, GeV²)
- `beam_vertexZ > 50.4 && beam_vertexZ < 79.1` (target z-vertex, cm)
- `xim_pathlensig > 0` and `lambda_pathlensig > 0` (loose positivity cuts)
- `xim_pathlensig > 2` (`vec_cuts[1]`, Ξ⁻ path-length significance)
- `kphigh_p4.Rapidity() > 2` (y(K⁺_fast))
- `t_dist < 2.4` (downstream Mandelstam |t|, GeV²)

## Barlow variations

The 18 systematic cut variations from `_workdir/AnalysisNote/systematics/GetVariationTreesUML.C:25-35`,
each replacing one nominal cut in turn:

- χ²/ndf: 6, 7, 9, 10
- |MM²| (GeV²): 0.01, 0.015, 0.025, 0.03
- Ξ⁻ path-length significance: 1, 1.5, 2.5, 3
- Λ path-length significance: 0.5, 1
- y(K⁺_fast): 1.6, 1.8, 2.1, 2.2

A `kplow` (slow K⁺ momentum) variation exists in the legacy variation list
but is disabled there, consistent with there being no nominal `kplow` cut in
`flatTreePrep.C`.

`systematics/GetXSecFilesUML.C` (run by `systematics/run.sh`) is the
canonical per-variation cross-section macro; the legacy
`GetXSecFitVariations.C` in the same directory is an older draft (single
nominal-file pass, no per-cut variation loop, looser Johnson/Chebychev fit
bounds) and was not migrated.

`systematics/barlow/` keeps the six legacy `PlotXSecBarlow*.C` macros
(ChiSqNdf, KHighRapidity, KLowRapidity, LambdaFlightSig, MissingMass,
XimFlightSig) unmerged: besides the variation list, labels and file names,
their diffs also touch canvas/legend geometry, symmetric y-range thresholds
and cut-value string parsing per family, so they fail the "names/lists/
labels/filenames only" merge test.

## Legacy provenance

This pipeline replaces `RunAnalysis.sh` / `RunXSec.py`, which did not run as
checked in (see `docs/KNOWN_ISSUES.md`).
