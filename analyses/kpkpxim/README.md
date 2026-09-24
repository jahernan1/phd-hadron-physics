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
- `signal_extraction/` — Q-factor and yield-extraction machinery, including
  single-fit lineshape studies in `signal_extraction/lineshape/`.
- `xsection/` — binning, weighting and plotting macros for the differential
  and total cross section, flux input in `xsection/flux/`, and published
  external comparison data in `xsection/external_data/`.
- `systematics/` — Barlow-variation cross-section pipeline: UML variation
  trees, per-variation cross sections and weighting, Barlow plots
  (`systematics/barlow/`), nominal-vs-variation comparison plots
  (`systematics/comparisons/`), track efficiency
  (`systematics/track_efficiency/`) and MC-weight variations
  (`systematics/mc_weight_variations/`).
- `config/` — channel configuration consumed by `gxana`.

## Pipeline

1. Select events from the skim into per-period, per-sample flat trees:

   ```sh
   gxana run select --channel kpkpxim --period P --sample S
   gxana run select --channel kpkpxim --period P --sample S --thrown
   ```

2. Apply the nominal analysis cuts and write the flat trees used downstream:

   ```sh
   root -l -b -q rootlogon.C 'analyses/kpkpxim/selection/flatTreePrep.C("flatTree_<stem>")'
   ```

3. Compute Q-factors and extract signal yields (Plan 5; see
   `signal_extraction/`).

4. Bin, fit, weight and rescale the cross-section tables:

   ```sh
   gxana run xsection --channel kpkpxim
   ```

5. Plot the differential and total cross section:

   ```sh
   cd $GXANA_OUTPUT/kpkpxim/xsection && root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/xsection/PlotDiffXSec.C
   ```

6. Systematics: build the per-variation trees, cross-section tables, the
   run-period weighted average, and the Barlow-significance plots (see
   [Barlow variations](#barlow-variations) below for the cut list):

   ```sh
   cd $GXANA_OUTPUT/kpkpxim/systematics
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/GetVariationTreesUML.C
   root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/systematics/GetXSecFilesUML.C
   python3 $GXANA_ROOT/analyses/kpkpxim/systematics/GetWeightedXsecFile.py
   for f in $GXANA_ROOT/analyses/kpkpxim/systematics/barlow/PlotXSecBarlow*.C; do
       root -l -b -q $GXANA_ROOT/rootlogon.C "$f"
   done
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
