# kpkpxim

## Physics

This channel measures the GlueX Phase-I differential and total cross section
for γp → K⁺K⁺Ξ⁻, reconstructed from the K⁺K⁺Ξ⁻ final state (Ξ⁻ → π⁻Λ,
Λ → pπ⁻) across the three GlueX Phase-I run periods analyzed here: Spring
2017 (S17), Spring 2018 (S18) and Fall 2018 (F18).

## Directory map

- `selectors/` — DSelector source compiled and run by `gxana run select`.
- `selection/` — flat-tree production (`flatTreePrep.C`), the chapter-4 cut
  studies (`cut_studies/`) and the chapter-5 data/MC comparisons
  (`mc_studies/`); see [`selection/README.md`](selection/README.md).
- `backgrounds/` — background-channel selectors and studies;
  [`backgrounds/README.md`](backgrounds/README.md).
- `signal_extraction/qfactors/` — Q-factor run README and helper scripts (fit
  models live in the QFactors fork, `packages/qfactors`); single-bin
  lineshape studies in
  [`signal_extraction/lineshape/`](signal_extraction/lineshape/README.md).
- `xsection/` — plotting macros for the differential and total cross section,
  flux input in [`xsection/flux/`](xsection/flux/README.md), and published
  external comparison data in `xsection/external_data/`.
- `systematics/` — variant comparisons
  ([`comparisons/`](systematics/comparisons/README.md)), track efficiency
  ([`track_efficiency/`](systematics/track_efficiency/README.md)) and MC-weight
  variations ([`mc_weight_variations/`](systematics/mc_weight_variations/README.md)).
- `measurements/` — Ξ⁻(1320) mass, lifetime and spin;
  [`measurements/README.md`](measurements/README.md).
- `simulation/` — thesis signal-MC inputs and sampling macros;
  [`simulation/README.md`](simulation/README.md).
- `config/` — channel configuration consumed by `gxana` (all files are merged;
  a top-level key may appear in one file only). After any edit run
  `gxana config export --channel kpkpxim`: the C++ macros read the exported
  `channel.kv` and refuse a stale one.
  - `channel.yaml` — reaction, selector directory and names, `output_basename`,
    and the `physics:` block (see [Channel physics](#channel-physics));
  - `periods.yaml` — run periods: launch tag, ReactionFilter prefix, `label`,
    flux run range and file, `dir` and `title` (described in the file header);
  - `samples.yaml` — tree-directory template and the data and MC samples;
  - `binning.yaml` — beam-energy and −t bin edges;
  - `mc.yaml` — signal-MC production for `gxana run mc`;
  - `qfactors.yaml` — Q-factor run for `gxana run qfactors`;
  - `xsection.yaml` — cross-section chain for `gxana run xsection`;
  - `barlow.yaml` — Barlow cut variations for `gxana run barlow`;
  - `systematics.yaml` — variant pool and studies for `gxana run systematics`;
  - `studies.yaml` — cut scans and data/MC kinematics for `gxana run studies`;
  - `measurements.yaml` — Ξ⁻(1320) prep and fit macros for `gxana run measurements`.

### Channel physics

The `physics:` block of `config/channel.yaml` holds the channel constants the
compiled apps take as flags (no channel is built into them). `gxana.config.physics`
checks it; unknown keys are an error.

- `flat_tree`, `thrown_flat_tree` — flat-tree names (`gxana_xsec_bin --tree`);
- `observable` — `{branch, title}`: the fitted mass branch and its axis title
  (`gxana_xsec_tables --observable/--observable-title`, `gxana_barlow_trees --check`);
- `qvalue_branch` — Q-factor weight branch for the data (`--data-branch`,
  `--qvalue-branch`); `null` for a channel without Q-factors;
- `branching_ratio` — `{value, error}`, product over the decay chain (`--br`);
- `reaction_title` — reaction label on the Barlow plots (`gxana_barlow_plot --reaction-title`).

Readers: `gxana run xsection` (steps `bin`, `tables`), `gxana run barlow`
(`check`, `tables`, `plot`) and `gxana run systematics` (step `fit`, through the
xsection `tables` command). See also [`packages/xsection`](../../packages/xsection/README.md).

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
   `packages/qfactors`, run config `config/qfactors.yaml`; settings as run in
   [`signal_extraction/qfactors/README.md`](signal_extraction/qfactors/README.md)):

   ```sh
   gxana run qfactors --channel kpkpxim --period P
   ```

   The output `$GXANA_OUTPUT/kpkpxim/qfactors/<stem>_nominal_kphighrap_1111111/postQVal_flatTree_*.root`
   is the data input of step 4.

4. Bin, fit, weight, integrate and split the cross-section tables (default
   steps: `bin,tables,weight,integrate,components`). The `bin` step reads the
   thrown MC flat trees from `$GXANA_DATA/flatTrees/` (`xsection.inputs.thrown`);
   `gxana run select --thrown` (step 1) writes them there directly
   (`flatTreePrep.C` does not touch them), so no copy is needed
   (`xsection.mc_sample` is `gen_amp_V2_ac_YstarRest`):

   ```sh
   gxana run xsection --channel kpkpxim
   ```

   From the preserved data, stage the inputs first with
   `gxana data stage --channel kpkpxim` (`--dry-run` prints the plan and changes
   nothing). It places the thesis binned trees and the post-Q-factor trees under
   `$GXANA_OUTPUT` and the MC flat trees under `$GXANA_DATA/flatTrees/` as the
   `stage:` block of `analysis_data.yaml` lists: files a later step rewrites are
   copied, the thrown trees are symlinked (`select --thrown` replaces a linked
   destination instead of writing through it), so the preserved files are never written through. It never overwrites
   a different file (`conflict`, exit 1, nothing staged). Then run
   `gxana run xsection --channel kpkpxim --steps tables,weight,integrate,components`;
   a run that includes `bin` re-bins the later-production MC flat trees and does not
   reproduce the thesis.

   Before it runs a step, `gxana run xsection` checks that step's inputs
   (`bin`: the post-Q-factor, MC and thrown flat trees; `tables`: the binned
   trees and the flux; `weight`, `integrate`, `components`: the `tables` output)
   and, if any is missing, lists each one on stderr with the command that makes it
   (`qfactors`, `flatTreePrep.C`, `select --thrown`, `--steps bin`, `--steps tables`
   or `gxana data stage`) and exits 1 without running that step. `--dry-run` skips
   the check; `tex` keeps its own check of the systematics statistics files.

   The `tables` step writes the nominal fit label, `johnson`, into
   `$GXANA_OUTPUT/kpkpxim/xsection/data/johnson/`, which `weight`, `integrate`
   and `components` read, laid out as the legacy `AnalysisNote/xsection/data/`.
   The systematic variants are fitted by `gxana run systematics` (step `fit`,
   configured in `systematics.yaml`) into
   `$GXANA_OUTPUT/kpkpxim/systematics/variants/data/<label>/`, with the same
   layout:

   | label | where | fit | event weight |
   |---|---|---|---|
   | `johnson` (**dissertation result**) | `xsection/data/` (also refit in `systematics/variants/data/`) | Johnson + 2nd-order Chebychev background (legacy `MakeXSecFitVariations.C`) | `hybrid_combo` |
   | `johnson_cheby1`, `voigt`, `voigt_cheby1` | `systematics/variants/data/` | Johnson (or Voigtian) signal + Chebychev background of the given order | `hybrid_combo` |
   | `mcPdf`, `mcPdf_cheby1` | `systematics/variants/data/` | MC mass-PDF signal shape (`MCPdf` fit, no free shape parameters) + Chebychev background of order 2 / 1 | `hybrid_combo` |
   | `hybrid_combo`, `best_combo`, `acc_weight` | `systematics/variants/data/` | JohnsonMCShape study (legacy `MakeXSecFiles.C`): per bin, a Johnson fit to MC fixes skewness and tail of a Johnson + 2nd-order Chebychev data fit | the label (combo-selection study) |

   The published differential and total cross-section tables are the
   `johnson` label after the run-period weighted average
   (`weighted_data/johnson/`), with the scale-factor run systematic; the
   golden tests reproduce them byte for byte from the preserved data.

   The total cross section is written two ways, per period and weighted:

   - `totxsec_*.txt` / `totxsec_weighted_output.txt` — directly from the
     energy-only bins, which carry every value of −t (the "direct" total);
   - `intxsec_*.txt` / `intxsec_weighted_output.txt` — the integral of dσ/dt
     over the seven −t bins (`integrate` step), which covers only
     0.10 < −t < 2.40 GeV² and so carries that effective −t cut. The
     dissertation calls this the technically correct total because it uses
     the multidimensional acceptance correction.

   The `tex` step builds the dissertation LaTeX tables from
   `weighted_data/johnson/` with the scale-factor systematic and the
   systematic columns of `gxana run systematics` (step 6); run it last:

   ```sh
   gxana run xsection --channel kpkpxim --steps tex
   ```

5. Plot the differential and total cross section. Both macros take their
   directories as arguments and write the PDFs and the drawn graphs (ROOT
   files, read back by the tests) into the plot directory, never into the
   working directory; they exit 1 naming the missing input.

   - `PlotDiffXSec.C(xsecDir, label, plotDir)` draws one fit label (default
     `johnson`) from `<xsecDir>/{data,weighted_data}/<label>/`
     (default `$GXANA_OUTPUT/kpkpxim/xsection`): the run periods, the weighted
     average and the weighted average with the systematic band of the
     `syst_weighted_diffxsec_*` tables (one per weighted table; they are
     written by `gxana_xsection.syst_tables`, otherwise the macro exits 1).
     Output goes to `<plotDir>` (default `<xsecDir>/figures`).
   - `PlotTotXsecWithClas.C(xsecDir, label, plotDir)` plots the direct total,
     `totxsec_weighted_output.txt` and the three run periods next to the CLAS
     points with an exponential fit (the integrated total sits beside it in
     the same directory). The defaults reproduce the dissertation figure: label
     `hybrid_combo` of the systematics variant pool
     `$GXANA_OUTPUT/kpkpxim/systematics/variants`, not the published `johnson`
     tables (see `docs/KNOWN_ISSUES.md`); output defaults to
     `$GXANA_OUTPUT/kpkpxim/xsection/figures`.

   ```sh
   cd $GXANA_OUTPUT/kpkpxim/xsection && root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/xsection/PlotDiffXSec.C
   cd $GXANA_OUTPUT/kpkpxim/xsection && root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/xsection/PlotTotXsecWithClas.C
   ```

6. Systematics. `gxana run barlow` (packages/barlow; UML = unbinned maximum
   likelihood, the chain that produced the thesis results) builds one variation
   tree per cut value from the raw flat trees, runs the xsection package over
   the variations, weights each variation over the three run periods and draws
   the Barlow-significance plots against the nominal `johnson` tables of step 4
   (see [Barlow variations](#barlow-variations) for the cut list and
   `config/barlow.yaml` for the fit):

   ```sh
   gxana run barlow --channel kpkpxim            # steps: trees,bin,tables,weight,plot (check opt-in)
   ```

   Outputs under `$GXANA_OUTPUT/kpkpxim/barlow/`: `variations.json`,
   `variation_trees/` (`trees`, `bin`), `xsection_data/johnson/` and
   `fits/johnson/` (`tables`),
   `weighted_data/johnson/weighted_{totxsec,diffxsec}_vary_<id>*.txt`
   (`weight`), `plots/barlow_*.{pdf,txt}` (`plot`, σ_B per point in the `.txt`),
   `output_yields.txt` and `fits/` PDFs (`--steps check`). `combine_pdf.sh [label]`
   merges the per-variation fit PDFs into `combined_pdf/`. The fit-model,
   accidental-subtraction, run-period and track-efficiency studies run with
   `gxana run systematics` (packages/systematics, see its
   [README](../../packages/systematics/README.md)), between the xsection steps:

   ```sh
   gxana run xsection --channel kpkpxim --steps bin,tables,weight
   gxana run systematics --channel kpkpxim       # fit,qvalue,weight,spread,track,summary
   gxana run xsection --channel kpkpxim --steps tex
   ```

   Outputs under `$GXANA_OUTPUT/kpkpxim/systematics/`: `run/` (PDG scale factor,
   `sfactor_stats.txt`), `accidentals/` and `fit/` (spread stats files and
   `plots/`), `track/` (`track_counts.txt`, `track_efficiency.txt`, figures) and
   `summary/` (quadrature total per bin and the normalization record);
   `variants/` holds the fitted and weighted variant tables. The opt-in checks
   (`--steps compare`, `--steps runperiod`) are described with the
   [comparison macros](systematics/comparisons/README.md). The other studies
   (RF-bunch, REST-version, MC-model weights) have their own READMEs under
   `systematics/`.

7. Measurements: Ξ⁻(1320) mass, lifetime and spin
   ([`measurements/README.md`](measurements/README.md)):

   ```sh
   gxana run measurements --channel kpkpxim      # items mass,lifetime,spin; steps prep,fit
   ```

8. Studies (`config/studies.yaml`, [`packages/studies`](../../packages/studies/README.md)):
   the chapter-4 cut scans `chisqndf_scan` and `mm2_scan` (raw trees of step 1)
   into `$GXANA_OUTPUT/kpkpxim/cut_analysis_plots/`, and the chapter-5 data/MC
   kinematics `kinematics` (Q-factor output of step 3, MC trees of step 2) into
   `$GXANA_OUTPUT/kpkpxim/data_mc_kinematics/`. The other cut and MC studies
   are macros described in [`selection/README.md`](selection/README.md):

   ```sh
   gxana run studies --channel kpkpxim           # --study chisqndf_scan,mm2_scan,kinematics; steps fill,fit,plot
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

There is no −t cut in the nominal selection: the energy-only bins (direct
total cross section) keep every value of −t. The differential bins cover
0.10 < −t < 2.40 GeV² (`config/binning.yaml`), which is the effective −t range
of the integrated total. (`flatTreePrep.C` also writes a `_nominal_tCut`
variant with `t_dist < 2.4`, used by studies only.)

## Barlow variations

The 18 systematic cut variations from `config/barlow.yaml` (legacy `GetVariationTreesUML.C`, archived),
each replacing one nominal cut in turn:

- χ²/ndf: 6, 7, 9, 10
- |MM²| (GeV²): 0.01, 0.015, 0.025, 0.03
- Ξ⁻ path-length significance: 1, 1.5, 2.5, 3
- Λ path-length significance: 0.5, 1
- y(K⁺_fast): 1.6, 1.8, 2.1, 2.2

A `kplow` (slow K⁺ momentum) variation exists in the legacy variation list
but is disabled there, consistent with there being no nominal `kplow` cut in
`flatTreePrep.C`.

The variations are fitted with `JohnsonMCShapeSyst`, the transcription of the
legacy `GetXSecFilesUML.C` fit (a Johnson fit to MC seeds the shape of a
Johnson + 2nd-order Chebychev data fit); the literal differences from the
thesis-table fits are tabulated in `packages/xsection/src/YieldFit.cxx`. The
nominal the Barlow plots compare against is the `johnson` label
(`barlow.label`); the variation fit is not the same fit as the nominal
label's, as in the legacy chain. Only the nominal-cut flat trees are
preserved, so the variation fit has no golden test; the run-period weighting
of the preserved variation tables is golden-tested. The legacy
`GetVariationTreesUML.C`, `GetXSecFilesUML.C`, `SplitVariationTrees.C`,
`GetWeightedXsecFile.py`, `run.sh` and the six `PlotXSecBarlow*.C` are kept
under `archive/systematics_legacy/`.

The six legacy `PlotXSecBarlow*.C` macros are archived; their drawing code is
`gxana::barlow::PlotBarlow` and their per-family differences (canvas, legends,
σ_B axis range, title offsets) are the `style` entries of `config/barlow.yaml`
(`BarlowPlotSpec`, `packages/barlow`). `kplow_prap` had no variation trees and
is not configured.
