# Port Notes

How the legacy code was migrated into this repository: behaviour kept from
the original macros, defects reproduced on purpose, how each port was
checked, and the decisions taken on the way. Findings that change or disagree
with a published thesis number, table or figure are in
`docs/KNOWN_ISSUES.md`. Nothing here should reference a private site path or
a tracking-issue identifier.

## 1. Documented, not fixed

The following legacy behaviors are preserved as faithful copies and are
not in scope for this migration:
- DSelector `cout` spam during processing.
- Commented-out PID ΔT and Ξ⁻ mass-window cuts.
- `PlotComponents.C` name clash; fixed on migration (f58181b renamed the
  function).

## 2. Fixed in the fork and the patches

- QFactors `qvalueSum` was never initialised and the `fitParams` branch used a variable-length array: both fixed in the fork (`packages/qfactors`, see its `CHANGES_THESIS.md`). The fork also resets `chiSqNdf_<var>` to NaN for events whose fit is not drawn; q does not depend on it (it is filled only when a fit is drawn, after the q-factor is chosen), so thesis q-factors are unaffected. The preserved thesis trees already have NaN `chiSqNdf_decayxim_M` in 19–33 % of events (section 13).
- MCwrapper `MakeMC.sh` used csh's `>>!` redirection, which bash reads as a write to a file named `!`: fixed in `packages/montecarlo/patches/gluex_MCwrapper/0002-*`. The `MakeMC.csh` copy is correct as is.

## 3. kpkpkmlamb (side channel)

- `DSelector_kpkpkmlamb.C` prints per event and per combo, and still defines
  the template's example branches; kept as run.
- Legacy `flatTreePrep.C` repeated default arguments on function
  definitions, which cling rejects; the migrated `flatTreePrep.C` keeps them on
  the declaration only.
- Legacy `FitXimStarCuts.C` did not compile (`histTitlek` undeclared). It
  runs as `FitXimStar(n, true)` with the selection `t_dist > 2`, not the
  saved `t_dist > 1`: the dissertation's three −t panels
  (`Xi1820massFit_TCut1/2/3.pdf`, labelled `t < 1`, `t = [1,2]`, `t > 2`)
  were made on 2025-06-21 and the macro was saved three days later with the
  `_TCut3` output commented out, so the saved filter made none of them. The
  `t > 2` label, the `_TCut3` name and the panel series agree. Only the third
  panel has a mode; the first two are not reproduced.
- The fit reads the merged GlueX-I tree; the legacy tree was made by hand, the
  README's `hadd` command reproduces it.

## 4. Published label and totals (reconciled 2026-09-29): port notes

The findings of this area are in `docs/KNOWN_ISSUES.md`; these points concern the port.

- `weighted_average` prints an `inf` error where a variation bin is empty
  (NaN row); the legacy script printed `0.000000`. The Barlow weight golden
  test maps one to the other.
- `gxana run barlow` replaces the UML chain (`gxana_barlow_trees`, the
  xsection package with `JohnsonMCShapeSyst`, `gxana_barlow_plot`). No variation trees are preserved,
  so that fit is transcribed, not golden-tested.
- The Barlow plots are pixel-compared with the archived macros' output and the σ_B tables with the legacy formula; the variation-tree fits and the `check` yields have no golden (no variation trees are preserved).
- The preserved `reference/xsection/qvalues` tables were made from an earlier
  `hybrid_combo` run than the preserved `reference/xsection/hybrid_combo`
  tables. Rescaling the preserved `hybrid_combo` with the legacy
  `MakeQValXSecFile.py` and with `gxana_xsection.qvalue_rescale` gives
  identical output except in two rows (below), where the legacy script
  divides by `data_yield` 0 and writes an empty `dsigmadt` and the port
  writes 0. The port's output differs from the preserved `qvalues` by 0.11 %
  (median) and at most 0.57 % in both `dsigmadt` and `Yerr` in 166 of 168
  rows. In the other two (2017-01, 6.40–7.40 and 7.40–7.86 GeV,
  −t 1.53–2.40 GeV²) the preserved `hybrid_combo` row holds a failed fit
  (`data_yield` 0, uninitialised MC columns), so the port's rescale gives 0 where
  `qvalues` has 1.408 and 0.456. `hybrid_combo` and `qvalues` are studies, not
  the published label; `tests/golden/test_python_golden.py` keeps the
  comparison as a strict expected failure.

## 5. Fit-model systematic (`gxana run systematics --study fit`, checked 2026-09-29): port notes

The findings of this area are in `docs/KNOWN_ISSUES.md`; these points concern the port.

- The per-bin data-fit plots use the style of the preserved legacy
  `FitFunctions.cpp` (30 mass bins, a pull panel); the dissertation figures
  (36 bins, no pull panel, a framed parameter box without
  `delta`/`gamma`/`width`) came from a plotting version that is not
  preserved. This is not a change made by the port.

## 6. Accidentals spread over three methods

The published Accidentals column is the spread of `acc_weight` and `hybrid_combo` only. The
suite's `accidentals` study spreads over all three methods (`acc_weight`, `best_combo`,
`hybrid_combo`), so that column, and the quadrature total, change relative to the published
tables. The legacy two-member set is reproduced with `spread: [acc_weight, hybrid_combo]`.

## 7. Paired t-test index in the legacy comparison macros

The archived `PlotComboComparison.C:104` and `PlotFitComparison.C:125` accumulate
`sum1 += yValues1[1]`, a fixed index where the loop index was meant. The statistic is printed
only, and the affected `t_stat1` is discarded, so the port's fix in `PlotSpread` has no
observable effect on any printed or returned number.

## 8. Acceptance correction: behaviour kept from the original macros

The acceptance (reco/thrown) and its application to the data now live in
`gxana::Acceptance` and `gxana::AcceptanceCorrect` (`packages/common`). The
original copies agreed on contents but not on error treatment; each site keeps
the treatment it had, so no published number changes.

- Binomial errors for ε: the 3-D sampling macros (`simulation/sampling/getHist3D.C`,
  `getHist3D_F18.C`) and the 1-D copies in `selection/mc_studies/get_data_hists.C`,
  `simulation/validation/get_data_hists_RF.C` and
  `systematics/mc_weight_variations/get_data_hists.C`. Plain division: the track
  study (`TrackHists`). The 2-D sampling macro (`PrepSampling.C`, from
  `getHist2D_gen_amp.C`) turns `Sumw2` off on both ε and the corrected histogram;
  the 2-D MC-study copy in `selection/mc_studies/get_data_hists_RF.C` turns it off
  on ε only. Ported results equal the originals bin for bin, contents and errors.
- The preserved 2-D sampling histograms have data bins with zero acceptance
  (Spring 2017 274, Spring 2018 281, Fall 2018 274 of 2500 bins), which `gen_amp`
  cannot sample; the corrected bin is 0. The original was silent about them;
  `PrepSampling.C` now prints a warning. The unused 1-D copies in the old sampling
  macro were dropped; the written objects are identical to the original's,
  including key names, object names, titles and bin contents and errors.
- Not migrated, original code kept: the 1-D copy in
  `selection/mc_studies/get_data_hists_RF.C` (its two-argument `Divide` differs
  from the library by up to 1.8e-15 in the errors when `Sumw2` is on),
  `GetAcceptanceCorrHist3dByBin` in the 3-D sampling macros (never called; its
  explicit `Sumw2` on the data has no library mode), and the inline 1-D
  t-projection acceptance in `getHist3D.C`.
- The 3-D sampling macros still do not run end to end; only their acceptance
  functions were ported and checked against the originals.
- `systematics::GetAcceptanceCorrHist2D` still ignores its `weighted` argument.
- Original defects reproduced, not fixed: in
  `selection/mc_studies/get_data_hists_RF.C` the sources of
  `costheta_ystar_all_thrown` and `costheta_ystar_all_mc` are swapped, and the
  `Write(...), TObject::kOverwrite;` statements never pass `kOverwrite`;
  `simulation/validation/compare_iters.C` and `in_out_test.C` read the TH3F
  objects of the 3-D macros through `(TH3D*)` casts.

## 9. Measurements: behaviour kept from the original macros: port notes

The findings of this area are in `docs/KNOWN_ISSUES.md`; these points concern the port.

- Reproducibility. The histograms are filled with implicit multithreading by
  default, as the original did, so weighted fills are not bit-reproducible and the
  fits inherit that. Lifetime and spin fit errors change in their last printed
  digits between runs. The mass RooFit fit amplifies the noise: the unchanged
  original macro spread by up to about 0.9 % run to run on one printed error (the
  Fall 2018 data yield error), and a plot label (the FWHM) can change in its last
  digit. The central values quoted in the dissertation are unchanged at their
  printed precision. Passing `0` threads to the prep and fit macros gives
  reproducible numbers. `tests/golden/test_measurements_golden.py` runs the new
  macros single-threaded and compares them with a single-threaded run of the
  original macros: all 13 `FITRESULT` lines were string-identical, and on the
  original's own stored histograms the fit macros reproduce its output exactly.
  Since the minimiser pin (section 15) the reference holds the port's pinned
  single-threaded lines; unpinned, the port still equals the original.
  The single-threaded numbers differ from the multithreaded ones by up to about
  1.5e-3 relative (Spring 2018 data mass yield error; measured with Minuit2,
  before the pin), of the same order as the
  original's own spread. Whether implicit multithreading is on during the fit,
  not only during the fill, also changes the last digit of the per-period spin
  `beta_err` and of the Spring 2018 lifetime `slope_err`, which is why the fit
  entries take the thread count too.
- Histogram identity. The golden test pins the `FITRESULT` lines only. Identity
  of the histograms with the original was checked once, by a bit-exact
  single-threaded comparison of the new prep and fit macros with the original
  macros: all 41 histograms written by the original (mass, lifetime and spin, per
  period and merged) are bit-identical in contents, errors and entries, with the
  same names, titles, axis titles, binning and stored drawing attributes; the six
  mass fit canvases hold identical points; all 13 plots are identical apart from
  their timestamps; and the printed output is identical except for the
  acceptance-check lines and ROOT's canvas-replacement warnings.
- Input tree. The plain data tree `flatTree_<stem>_nominal_kphighrap.root` read
  by the mass fit is not part of the preserved data. The post-Q-factor tree stands
  in for it in the golden test and in the recorded reference; the mass histogram
  uses `hybrid_combo` only, as the original does. The post-Q-factor tree is made
  by `packages/qfactors/mergeQresults.C` as an unfiltered clone of the Q-factor
  input, which `config/qfactors.yaml` sets to the plain tree, so it holds the same
  entries (9893 / 36986 / 32580); the data mass, yield and σ central values
  reproduce the dissertation figures to the printed precision (the Spring 2018
  data yield error prints 77 with the TMinuit pin, 79 with Minuit2, against
  the figure's 78).
- Archived. `GetXimProperties.C` is replaced by the prep and fit macros
  (`archive/root_macros/`); it (now `FitMass.C`) propagates the Johnson-mean
  parameter errors in quadrature. The invariant-mass figure macros
  `MakeXim1320_IM*.C` and `MakeXim1820_IM.C` keep their own copies of the Johnson
  fit and style and are unchanged by this rework. `MakeXim1320_IM()` reads the
  `_ximVertexCut` trees, which are not in the preserved data, so it cannot run
  as preserved; `MakeXim1320_IM_Res()` reads the plain `_kphighrap` tree, which
  is also not preserved (see the input tree above).

## 10. Common plot style, bin names and table readers: behaviour kept

The plot styles, the 3x3-grid gStyle tail, the bin-name format, the energy-bin table
reader and the command-line parsers of `xsection`, `systematics` and `barlow` now live in
`packages/common` (`ApplyStyle` with `ThesisStyle`, `FitStyle`, `ComparisonStyle`,
`TrackStyle`, `BarlowStyle`, `GridTrailingTweak`; `BinNames.h`; `ReadBinnedGraphs`;
`Cli.h`). Every style leaves the same `gStyle` as its original (compared as a full dump,
from the default style and from every other style's state), and every reader returns the
same graphs, so no figure or number changes. Kept as they were:

- `CreateTGraphErrorsFromTxt` (used by `xsection/PlotDiffXSec.C` and `PlotComponents.C`)
  ordered tables with `NumericCompare`, which compared only the integer part of emin: the
  configured edges 7.40/7.86 and 8.19/8.45/8.68 tied, and their order then depended on the
  directory listing and the C++ library. Fixed on 2026-10-02: the order is now the full
  emin value, with the name as tie-break. Before and after orders on the preserved `johnson`
  tables (macOS, ROOT 6.40; the reader called with the patterns the macros pass), listing the
  emin of each panel: with `weighted_diffxsec*` (8 tables), before 6.40, 7.40, 7.86, 8.19,
  8.68, 8.45, 9.26, 10.18; with `syst_weighted_diffxsec*`, before 6.40, 7.86, 7.40, 8.45,
  8.68, 8.19, 9.26, 10.18; with the PlotComponents pattern `*diffxsec*.txt` (the `syst_`
  and plain table of each bin in one list), the pairs came out as 6.40, 6.40, 7.40, 7.86,
  7.86, 7.40, 8.45, 8.68, 8.19, 8.68, 8.45, 8.19, 9.26, 9.26, 10.18, 10.18; with the
  per-period pattern `diffxsec*2017-01*`, before 6.40, 7.40, 7.86, 8.45, 8.19, 8.68, 9.26,
  10.18. After, every pattern gives ascending emin 6.40 ... 10.18 (8.19, 8.45, 8.68 in
  order), and in the `*diffxsec*.txt` list the `syst_` table precedes the plain one of
  the same bin (name tie-break). No table value changes, only the panel order of the figures and the graph order in the ROOT
  file the reader writes. Figures made with the legacy code may show the old order, so the
  dissertation figures should be compared panel by panel. With a single matched file the
  comparator is never called, so a name without `emin` is titled `(, )`.
- The 3x3 grid functions write the pad margins and the Y label offset to `gStyle` after
  drawing, so a later canvas in the same process inherits them.
- `SetTitleFont(132)` and `SetNdivisions(505)` without an axis act on X only; the thesis,
  fit and Barlow styles never set the pad-title font (the comparison style sets 132, the
  thesis style leaves ROOT's default, 42); `SetTitleOffset(0,"T")` does nothing; the original styles
  created a `TLatex` they never used (the ports do not; `gStyle` and the figures are
  unaffected).
- Bin-edge labels truncate (`0.375` -> `0.37`) and the truncated text is also the
  RDataFrame cut value; no configured edge has three decimals.
- `barlow::calculateStdDevGraph` (no production caller) divides by N; the systematics
  spreads use N-1.
- The comparison plots' per-point significance treats the two tables as independent,
  although several comparisons use the same data; it is an on-plot annotation only.
- `xsection.unit`, `barlow.unit` and `systematics.unit` write to fixed shared temporary
  directories, so two test runs at the same time on one machine can fail each other (seen
  once; reruns pass).

Changed only for malformed input: `ParseBinName` (cross-section tables) throws for a name
without `emin_`/`emax_`, or with `tmin_` but no `tmax_`, where the old code read from
position 4; it also cuts emax at the next `_`, where the old total-cross-section parse
read to the end of the string, which differs only for a name with a suffix after emax
(the formatters never produce one). `ReadBinnedGraphs` rejects two tables with
numerically equal emin (for example `6.4` and `6.40`), whose order was unspecified. Neither
occurs in the configured binning or the preserved tables.

Not verified: the ports were checked with ROOT 6.40 only; the GlueX container
(ROOT 6.24) build and tests have not been run.

## 11. Stage command layer: behaviour kept (`gxana run`)

The xsection, barlow and systematics stages share `gxana.stages.runner`, `gxana.paths.gxana_root`
and `gxana.bins`; `tests/stage_plans/` holds the recorded command plans, dry-run output and run
traces that the shared layer reproduces. Kept as they were:

- Bin-edge labels: python formats edges with two decimals and rounds (`gxana.bins.edge_label`),
  the C++ `BinEdgeLabel` cuts after two decimals. They agree on every configured edge; an edge
  with three decimals (0.375, 6.405, 7.855) would give python glob patterns, barlow preflight file
  lists and `--energy` labels that differ from the tree and table names the C++ writes
  (`packages/common/tests/python/test_bins_cxx.py` pins the difference; neither side was changed).
- When several unknown steps are passed, `gxana run xsection` names the alphabetically first one,
  `gxana run barlow` and `gxana run systematics` the first one given; `gxana run systematics`
  reports a step that moved to barlow (`bin`, `tables`, `barlow`) with that hint.
- `gxana_root(environ)` falls back to the shell's `GXANA_ROOT` when the given mapping has none, so
  an explicit mapping cannot hide a set `GXANA_ROOT`.
- Plans depend on whether `$GXANA_ROOT/build/bin/<exe>` exists (full path, else the bare name),
  on the python interpreter running `gxana`, and, for the systematics qvalue and compare steps, on
  the files already present. The recorded plans replace the first two by placeholders and record the
  systematics plans on an empty and on a populated output tree.
- The weighted-table name patterns differ: `gxana_systematics.tables` accepts `\d+\.\d+` edges,
  `gxana_xsection.integrated_total` `[0-9.]+`; both match every name the C++ writes, and they were
  not unified.
- What the recorded plans cover: command plans, dry-run output, the calls made to the runner and
  the files created. The order in which directories are created relative to the commands, and a
  failure at a middle command, are not part of the recorded plans; they were compared once, outside
  the test suite, between the code before and after the stages moved onto the shared runner (276
  runs: 19 scenarios on both channels with stdout and stderr in one stream, failures at the first,
  last, middle and step-boundary commands, directory snapshots at every call) and were identical.

## 12. Analysis-macro plot styles: behaviour kept and open decisions

The 39 local style functions of the analysis macros (`setStyle`, `style_format`,
`SetStyle`) keep their names and calls; their bodies now apply a `packages/common`
preset (`FitStyle`, `CutStudyStyle`, `ComparisonStyle`, `DistributionStyle`,
`ThesisStyle`, `BarlowStyle`) with per-macro overrides. `tests/macros/test_macro_styles.py`
compares the whole `gStyle` each function leaves with a verbatim copy of its original
body, from ROOT's default style and from a state in which every member that an original
body or a preset writes holds a value none of them writes, so members a body never set are
still left alone. Of the 39 original bodies, 25 differ in text and 23 in the `gStyle` state
they leave; the harness states which copies coincide. Ten macros are also checked by
rerunning them before and after on the preserved inputs, identical when rasterized
(Ghostscript, 100 dpi grayscale): `xsection/PlotTotXsecWithClas.C`, `systematics/GetRunPeriodPctSig.C`,
`xsection/PlotXSecComponents.C` (labels `hybrid_combo` and `johnson`), the fit canvases of
`signal_extraction/lineshape/SingleGaussianFit.C`, `DoubleGaussianFit.C` and `OneUMLFit.C`,
`selection/GetKinematicsDataMC_RF.C`, `archive/root_macros/GetKinematicsDataMC.C` (called with the
gen_amp_V2 `kphighrap` stems), `selection/cut_studies/kaon_selection/make_plot.C` (the
`_kphighrap` plots) and `selection/cut_studies/rapidity_cuts/PlotKPlusLowRapidity.C` (the
last two from the histogram files that their unchanged `get_data_hists` macros write from
the preserved trees). The remaining 29 were not rerun (none of them runs on the preserved
inputs alone): they are verified by the identical `gStyle` state alone (their drawing code
is unchanged). The two archived macros (`selection/CutAnalysis.C`,
`selection/GetKinematicsDataMC.C`) and the deleted `systematics/track_efficiency/WeightMC.C` copy
are no longer in the macro-style harness (2026-10-02). Kept as they were:

- The original bodies created a `TLatex` they never drew; it is no longer created.
  `SetTitleOffset(x,"T")` (in `accidentals/get_data_hists.C` and
  `systematics/GetRunPeriodPctSig.C`) does nothing in ROOT and is not reproduced.
- `setStyle()` is never called in `backgrounds/YstarBWFitsData.C` (its calls are
  commented out) or in `kpkpkmlamb/measurements/FitXimStar.C`; both were converted.
  `systematics/mc_weight_variations/WeightMC.C` and the former
  `systematics/track_efficiency/WeightMC.C` were the same file (the copy was removed on 2026-10-02).
- `SingleGaussianFit.C` and `DoubleGaussianFit.C` save no plot: their `SaveAs` lines are
  commented out.
- `PlotXSecComponents.C` loops over four fit labels (`hybrid_combo`, `johnson`, `mcPdf`,
  `mcPdf_cheby1`). The preserved data holds only the first two: the macro writes ten PDFs,
  then, for a label whose component directory is missing, it gets no graphs and
  `plotComponent` reads the second graph of an empty list and stops with a segmentation
  fault. This is the same before and after the style change.
- Eight converted macros had no gxana include before and now need the gxana libraries to
  load, like every other migrated macro: run them through `rootlogon.C` with `GXANA_ROOT`
  set (a plain `root macro.C` no longer loads them):
  `selection/cut_studies/kaon_selection/make_plot.C`,
  `selection/cut_studies/lambda_vertex_cut/make_plot.C`,
  `selection/cut_studies/xim_vertex_cuts/make_plot.C` and `make_plot_RF.C`,
  `selection/mc_studies/make_plot.C`, `make_plot_RF.C` and `make_plot_acceptcorr.C`,
  `simulation/validation/make_plot_RF.C` (all under `analyses/kpkpxim/`). The load test
  (`tests/macros/test_macros_load.py`) passes in that setting.
- `SetOptStat`/`SetOptFit` also update the statistics box of the current pad. The style
  functions call the same setters with the same values, and in every original body, as in
  `ApplyStyle`, they are the last setters, so a pad sees the same style. The style harness
  runs without a pad: this is covered by that reasoning and, for `SetOptStat` only, by the
  plots of `PlotXSecComponents.C`, `GetKinematicsDataMC.C` and `GetKinematicsDataMC_RF.C`
  (their style functions run again while earlier canvases are open). No rerun macro sets
  `SetOptFit` from its style function.
- Two findings left as they are: the legend coordinates that some plot macros take from
  `GetUxmax()`/`GetUymax()` of a freshly created canvas are always 1, so those legends sit
  at the literal fractions written in the code (harmless); `ProcessFilesToTFile` in
  `PlotXSecComponents.C` joins its `directory` argument and each file name with no
  separator, so the directory needs a trailing `/` (the macro's own calls pass one).

Closed in section 19 (2026-10-02): the graph-reader adoptions below (1 to 3) were skipped, and
the mixed-bin panel of `xsection/PlotXSecComponents.C` stays as recorded.

Not done, because each changes an output:

1. `xsection/PlotXSecComponents.C` takes each period's tables in directory-listing order
   and draws graph j of every period in pad j, so on the preserved tables one panel
   overlays different energy bins of the three periods under the first period's title.
2. `xsection/MakeWeightedDiffXSecTGraphs.C` keeps its own reader: moving it to the common
   `ReadBinnedGraphs` would rename the stored objects from `Graph` to the table stem
   (keys unchanged) and make an empty or `emin`-less table an error instead of an empty
   graph; the common reader also has no full-path `diffxsec` filter (the macro admits
   every `.txt` of a directory whose path contains `diffxsec`). Its `binEdge` parse
   would move with the reader.
3. `PlotXSecComponents.C` `ProcessFilesToTFile` would need a sorted reader with a
   four-column format and a per-prefix grouping that the common library does not have;
   its bin-edge parse reads `emax` without checking that it is present.
4. `xsection/PlotDiffXSec.C` and `PlotComponents.C` read through
   `CreateTGraphErrorsFromTxt`. Since the ordering fix (section 10) they get ascending
   panels from that reader and remain on it; there is no `ReadBinnedGraphs` switch.

Not verified: the macros were checked with ROOT 6.40 only; the GlueX container
(ROOT 6.24) has not been run.

## 13. Q-factor fit models: findings recorded, not fixed

The thesis q-factors use `packages/qfactors/configPDFs.h`, which none of the
items below touches. The other models are the earlier variants, kept as they
were run; none of them was changed.

- `configPDFs_JohnsonGaus.h:127` builds the total PDF as
  `RooAddPdf(..., RooArgList(*rooSig,*rooBkg,*rooSigmaBkg), RooArgSet(*nsig,*nbkg,*rooSigmaBkg))`:
  the third coefficient is the Gaussian PDF `rooSigmaBkg`, not the yield
  `nsigmabkg` (declared at `:97`, created at `:125`). It compiles because a PDF
  is a `RooAbsReal`. `nsigmabkg` is therefore never fitted but still enters
  the signal fraction at its start value `kDim/10` (`:148`).
- In the same file, `reinitialize` (`:130-142`) does not reset `nsigmabkg`;
  `calculate_q` (`:151`) adds the two unit-normalised background PDFs without
  their relative yields; `sigma_width` (`:88`) is declared and never used.
- `configPDFs_Johnson.h`, `configPDFs_JohnsonGaus.h` and `configPDFs_Gaussian.h`
  differ from the thesis model in more than the PDF shape: initial values,
  parameter ranges, number of bins and `SumW2Error(true)` (the thesis model
  uses `false`). On the preserved 2017-01 slices of the golden test they give
  the thesis neighbour sets but not its q-factors (max |dq| 0.046, 0.074 and
  0.27 respectively).
- `packages/qfactors/configSettings.h:10` keeps upstream's site path as the
  default of `cwd`. It is always overwritten: `run.py` rewrites it with the
  current directory, and `gxana run qfactors` renders it as the work
  directory.
- `mergeQresults.C` and `makePlots.C` open `logs/<tag>/...` relative to the
  current directory, so they only work when run from the work directory
  (`run.py` and `gxana run qfactors` do that); `main.C` writes absolute
  `cwd`-based paths.
- Failed fits keep their q-factor. `main.C` keeps the lowest-NLL fit of each
  event whatever its status and writes that status to `fitStatus_<var>`; a
  NaN q-factor is set to 0. Nothing downstream reads `fitStatus_<var>`. In the
  preserved thesis trees every non-zero status is −1 (did not converge):
  1666 of 9893 events (2017-01), 10297 of 36986 (2018-01), 9450 of 32580
  (2018-08). `chiSqNdf_<var>` is NaN in 1898, 11968 and 10812 events, 1627,
  9922 and 8905 of them with status −1. The cross-section effect is in
  `docs/KNOWN_ISSUES.md` (Q-factor fits that did not converge).

## 14. Channel-agnostic packages: behaviour kept and open decisions

The cross-section, barlow and systematics packages take every channel value as a
command-line argument written by `gxana run` from `analyses/<channel>/config`: the `physics`
block of `channel.yaml` (flat trees, fit observable, Q-value branch, branching ratio,
reaction title), `xsection.{binned_suffix, gate, target, branches, mass_windows}` and
`barlow.{check.mass_windows, plot}`. The kpkpxim values are the former C++ literals. The apps
now require these flags: a command line without them (`gxana_xsec_tables` without the CHANNEL
flags, `gxana_barlow_plot` without the reaction title or ranges, a JOB before the first
`--weight`) is refused with a usage error, exit code 2 (`gxana_xsec_tables` and
`gxana_xsec_bin` name the missing flag). `gxana.config.physics` checks the `physics` block:
an unknown or incomplete key or a mistyped value is a `ConfigError` naming the key. The
`xsection.*` blocks (target, mass windows, a non-empty gate) and the weight keys
`xsection.weight` / `barlow.weight` are checked when the stage plans its commands, with the
same `ConfigError`; an empty or whitespace-only `xsection.gate` is rejected there, since an
empty `--gate` would make the app count every entry. `physics.qvalue_branch` must be written
explicitly, as `null` for a channel without Q-factors; then no Q-value column is binned, the
tables run with `--qvalue-branch none`, and the qval columns of the tables are `nan`.

Kept as they were, for the author:

- The fit gate ignores the label's weight: `xsection.gate` is
  `(hybrid_combo)*(decayxim_M>1.3&&decayxim_M<1.35)` for every label (legacy
  `FitFunctions.cpp`, `MakeXSecFiles.C`), so the `best_combo` and `acc_weight` labels may fit
  a bin their own weight would gate out, or the reverse. The gate counts entries whose
  expression is non-zero; it is not a weighted yield. A per-label gate would change which bins
  are fitted.
- The branching-ratio uncertainty (0.005 on 0.641) is added in quadrature to every point's
  error: a fully correlated normalization inside point-by-point errors.
- The target density is the 2018 value (70.08e-3 g/cm^3) for all three run periods.
- `gxana_xsection.components` cuts each output name at the anchor (`--anchor`, the channel's
  reaction, now required). A name without the anchor keeps only its last character, so the
  files of such names overwrite each other; every table `gxana run` writes contains the
  reaction.
- Still kpkpxim-shaped in code: the MC-shape literal sets (Chebychev seeds, the scan start
  1.32/1.30, the 1.31-1.33 GeV `mu` range of the systematics fit) and the barlow check's
  Johnson/Argus seeds (`mu[1.3217,1.32,1.33]`, `m0`); the RooFit names `xisignal`/`nxi` (the
  `nxi` parameter box of the Johnson-type fit PDFs; the MCPdf fit uses `nsig`/`nbkg`). A
  second channel's MC-shape fit needs them moved into the fit library first.
- `packages/xsection/src/Plotting.cxx` still hard-wires a "Spring 2017 / Spring 2018 /
  Fall 2018" legend; it is reached only from the kpkpxim analysis macros
  (`PlotDiffXSec.C`, `PlotComponents.C`), not from any stage.
- `gxana run xsection|barlow|systematics|mc|qfactors` keep `--channel kpkpxim` as their
  default; with the migration-only legacy path prefixes in `paths.py` it is the one channel
  literal left in package code (`tests/test_no_channel_literals.py` pins this).
- The track-efficiency study fills its histograms with 16 implicit-multithreading threads: the
  last bits of the bin contents of `particle_kinematics.root` change from run to run (the
  archived `get_hists.C` does the same). With `ROOT_MAX_THREADS=1` the file is reproducible,
  and every output comparison of this work was made single-threaded.

Second channel: the fixture `tests/fixtures/channels` (kpkpkmlamb with a synthetic MC sample)
checks the planned commands and component names only. No second-channel run on data was made,
because kpkpkmlamb has no MC; its `Int_t` weight `best_combo` read as a RooFit weight is
untested until a real run.

Verification level. For every commit of this work the stage-plan fixtures (regenerated once,
after a mechanical check that only appended flag groups differ) and a seeded equivalence run
of the apps (185 output files) were identical to the output of the code before the work. The
golden groups for the cross-section (`johnson` tables), binning, barlow plot, systematics
chain and the python tests (components, weighted averages) were compared file by file with
the pre-change outputs after the later changes. A comparison of every golden output file
(7136 files) was identical after the yield-fit change (`4e5e6a4`) and after the bin-step
change (`6dfd525`) and was not repeated after the later commits. The analysis container's
ROOT 6.24 was not available: the C++ is written for C++14, but its build and outputs on 6.24
are unverified; everything above ran with ROOT 6.40.

## 15. Fit library (`packages/fit`): behaviour kept and open decisions

Twelve analysis macros build and run their RooFit lineshape fits through
`gxana::fit`. Every fitted value and every printed line is identical to the
original on the same ROOT build (apart from the exceptions listed below);
for the seven macros checked by the equivalence test, identical means the
macro's fit function run on seeded synthetic input against its frozen
original. The plots were compared once as rasters when each macro was
ported. The library changes no global state and adds no
`fitTo` argument. Nothing here is checked on ROOT 6.24; everything ran with
ROOT 6.40. The points below are original behaviour that was reproduced, not
fixed.

How each macro was checked:

- Against an instrumented copy of the original on preserved or toy inputs
  (fitted values at 17 digits, factory statements, printed output, plots):
  `FitMass.C` (its measurements golden is unchanged), `SingleGaussianFit.C`,
  `DoubleGaussianFit.C`, `OneUMLFit.C` and `analyses/kpkpkmlamb/measurements/FitXimStar.C`
  (the toy tree of its test, both modes).
- Against a frozen copy of the original fit function on seeded synthetic
  histograms, because the real inputs are not preserved: `MakeXim1320_IM.C`,
  `MakeXim1320_IM_Res.C`, `GetQvalueSum.C`, `CutAnalysis.C`, `CutAnalysisRF.C`,
  `KstarFit.C` and `YstarBWFitsData.C`
  (`packages/fit/tests/python/test_legacy_sites.py`, three seeds each).
- Factory statements are identical up to blanks between arguments, which
  RooFit removes before parsing.

Limits of that check:

- The committed equivalence test compares fit results, factory statements
  and printed output, not plots. Plots were compared as rasters (Ghostscript)
  once per macro, by scratch scripts that are not in the repository.
- Printed RooFit evaluation-error reports list their objects in an order that
  differs from run to run (seen in `SingleGaussianFit.C`'s uncalled data fit
  and in `OneUMLFit.C`); for those two the stdout comparison ignores that
  order only. `DoubleGaussianFit.C` and `OneUMLFit.C` also print a "Creation
  of NLL object took ... μs" line whose time differs; it was ignored too.
- `GetQvalueSum.C`: on the synthetic input the window scan always finds its
  edge in the first bin and the macro then clamps the lower edge to 1.28, so
  the scan's result is not exercised by the test (the scan arguments were
  compared with the removed loop by reading).
- `KstarFit.C`: on the synthetic input the background and peak shape
  parameters end on their limits for every seed, so the test's value
  comparison rests on the two yields; the factory statements and fit
  arguments are compared exactly in every case.
- The `FitXimStar.C` toy fit reports zero errors for every parameter, so its
  error values are not exercised (the fitted values are).
- `OneUMLFit.C` adopts the library for its models only; its fits still go
  through `gxana::xsec::AttemptFitMC`/`AttemptFit`.

Not adopted (their fits are unchanged and have no equivalence check):

- `flatTreeCutsMC.C`: its fit function `rooFitHist` is declared at line 7
  with a default argument for `canName` and defined again at line 199 with
  the same default. The macro's own calls (lines 156-168) come before the
  definition and are not affected; a call made after the macro is loaded
  with `.L`, as the equivalence test makes it, is reported by cling as
  ambiguous, so the test cannot run this fit function and there is no run to
  compare with. Not changed.
- `MakeXim1820_IM.C`: its `Polynomial` background is used by no other macro
  and the macro does not run on the preserved data.
- `flatTreeCuts.C`, `flatTreePlots.C`, `flatTreePrepQVal.C`: three copies of
  one fit; whether they share one selection fit function is an author
  decision.
- `weighted_unbinned_fit.C`: reads `test_tree.root` from the working
  directory.
- The fit functions of `CutAnalysis.C` and `CutAnalysisRF.C`, and the fit
  blocks of `MakeXim1320_IM.C` and `MakeXim1320_IM_Res.C`, remain duplicate
  copies of each other (each now built from the same library calls).
- The package yield fits (`packages/xsection`) and the barlow check fits do
  not use the library; moving them is an open decision.

Behaviour kept:

- `OneUMLFit.C` is not the production `johnson` fit: it refits the data with
  all four Johnson parameters free over their full ranges, whereas the
  production data fit fixes γ to the MC value and floats δ and λ with their
  lower limit at the MC value; it also has other Chebychev start values,
  five retries without narrowing the window and a different window scan. The lineshape README presents it
  as the fit behind the cross-section yields.
- Johnson errors. `FitMass.C` prints the mean error from the diagonal of the
  four parameter errors (no correlations) and a σ error that uses δ/δ_err
  instead of δ_err/δ and ignores λ (infinite in the data fits, see
  `docs/KNOWN_ISSUES.md` section 7). `MakeXim1320_IM*.C` compute a relative-error sum with the same
  inverted term and never print it; "Mu: x ± y" there prints the Johnson mean
  with the μ error. "FWHM" there is 2.3548 σ, not the Johnson FWHM.
- χ²/ndf parameter counts are literals: 10 in both `FitMass.C` fits and 8 in
  `FitXimStar.C`; the values `MakeXim1320_IM*.C` use are likewise literals,
  not counts from the fit.
- Fixed-zero yields (`nxim1620[0]` in `FitXimStar.C`, `nbw1[0]` in
  `YstarBWFitsData.C`) leave that peak's mean and width floating without any
  effect on the likelihood.
- `SingleGaussianFit.C`: the data fit is never called. The MC fit starts
  `sigma1` at 0.015, above its upper limit 0.01 (RooFit clamps it), and its
  threshold background ends at its limits (`b` at −5, `m0` at 1.255) on the
  preserved bin.
- `DoubleGaussianFit.C`: the MC `SUM` statement ends with a comma.
- `CutAnalysis*.C` compute `min_mass` and do not use it; `KstarFit.C`'s
  `if(!data)` check can never fire.
- Tree-based fits import with a weight range of [−10, 10]: entries with
  |w| > 10 are dropped (`ImportTree` keeps this).
- Minimiser and evaluation backend (decided 2026-10-03: pinned to the ROOT
  6.24 defaults). `rootlogon.C` and the apps that fit (`gxana_xsec_tables`,
  `gxana_barlow_trees`, `gxana_study_cutscan`) call
  `gxana::fit::UseThesisMinimizer()`: default minimiser `Minuit`/`Migrad`
  (TMinuit), used by `fitTo` without a `Minimizer` argument and by
  `TH1::Fit`/`TGraph::Fit`, and on ROOT ≥ 6.32 the legacy RooFit evaluation
  backend. No macro was edited and `RunFit` still adds no argument;
  `YieldFit.cxx` and `VariationTrees.cxx` keep their explicit per-call
  `Minimizer("Minuit","migrad")`. Not pinned: macros run without
  `rootlogon.C` or `GXANA_ROOT`, and the Q-factor fork's backend. On ROOT
  6.40 (single-threaded) the pin moves 54 of the 76 measurement `FITRESULT`
  values (section 9; `docs/KNOWN_ISSUES.md` section 7) and the first
  run-period Gaussian mean from 0.927561 to 0.927529; with TMinuit
  the backend moves none of them; on top of Minuit2 the backend alone moves
  19 by at most 1.2e-3 relative. The cross-section fits already passed these
  settings per call and do not change.
- Fit order matters under the pin. TMinuit keeps one static `gMinuit`
  whose state carries from fit to fit within a process (Minuit2 has no such
  state, so unpinned the order never mattered). The cut-scan fit step
  therefore runs every block in one `gxana_study_cutscan` process, in the
  thesis macro's order (period outer; `chisqndf` then `mm2` scan inner).
  Running one (study, period) block per process gives a different
  first-row fit for every block but the first, by at most 4e-4 relative
  (15 of 810 table values differ from the thesis-order run, for example the
  first figure of merit of the `mm2` scan of `M23_2017-01_ana56`: 19.948454
  per process against 19.948676 in thesis order). A fresh TMinuit per fit would instead move 384 of
  423 cut-scan rows (up to 2.1e-3) and the fit-variations spread, so the
  thesis fit order is kept.

## 16. Per-period histograms and comparison plots (`packages/common`): behaviour kept and open decisions

What exists now. `gxana config export --channel kpkpxim` writes
`$GXANA_OUTPUT/kpkpxim/config/channel.kv` (periods, their directory names
`dir`, titles, labels and the tree stems). Nothing writes it implicitly; the
macros that read it stop with the export command in the message when the file
is missing or when a `config/*.yaml` file changed after the export.
`gxana::Period`, `gxana::ChannelInfo` and `gxana::MakePeriods`
(`GxanaCommon`) hand the periods to the macros; the per-period fill, get and
merge functions (`gxana::FillPeriodHists`, `GetPeriodHists`, `MergeHists`) are
in their own library `GxanaPeriodHists`, so that the other libraries do not
load RDataFrame; `gxana::DrawOverlay` draws the two-histogram comparison plot
of the validation macros.

Adopted, and how each was checked (single-threaded, ROOT 6.40; nothing is
checked on 6.24; every output histogram compared by key, object name, title,
binning, contents, errors and entries; every plot by its rasterised pages;
stdout compared line by line):

- `TrackHists` (`packages/systematics`): the track golden test before and after;
  `particle_kinematics.root` identical (74 histograms), `track_counts.txt` and
  `track_efficiency.txt` byte-identical.
- `selection/mc_studies/get_data_hists_RF.C`: the preserved trees plus a
  stand-in 2-D sampling file. The preserved
  `data_ac_ximVertexCut_hist2d_YstarRest.root` lacks `ResMassVsCosTheta_mc_Phase1`
  and `ResMassVsCosTheta_thrown_Phase1`, which the macro reads after its fills, so
  on the preserved data alone it stops at that read. Original and port ran on the
  same stand-in (two scaled copies of `ResMassVsCosTheta_qval_Phase1`): 80
  histograms identical. A deliberately broken port (the Spring 2018 clone name
  removed) was caught by the comparison.
- `simulation/validation/get_data_hists_RF.C` (57 histograms),
  `systematics/mc_weight_variations/get_data_hists.C` (5 files of 22) and
  `simulation/sampling/PrepSampling.C` (68): seeded toy trees, original against
  port, output files identical. Their real inputs are not preserved. The sampling
  golden test checks the acceptance library on stored histograms; it does not run
  `PrepSampling.C`.
- The measurement macros (`measurements/common/XimInputs.h`, `XimPeriods`) read
  the period list from `channel.kv`: the printed list is identical to the
  hard-coded one and the measurements golden test passes with its reference
  unchanged.
- The four `compare_plot` copies in `simulation/validation` (`compare_iters.C`,
  `compare_iters_2D.C`, `in_out_test.C`, `make_plot_RF.C`): seeded toy inputs
  (`make_plot_RF.C` on the toy `data_RF.root`), original against port: same list
  of PDFs, identical rasters (4, 5, 5 and 5 files), identical stdout. In
  `make_plot_RF.C` only two of the five PDFs come from `compare_plot`; the others
  come from the unported `merge_plot` and `compare_plot_log`, so 16 rasters
  exercise `DrawOverlay`. `in_out_test.C` takes its 2018-08 tree stem from
  `channel.kv`.

The comparisons are exact, but they are comparisons of the port with the
original on these inputs, not of either with a physics result. The toy
histograms of `make_plot_RF.C` never reach the `raiseMaximum` branch of
`DrawOverlay` (the first maximum stays above the scaled second one, and the macro
passes `raiseMaximum = false` in any case), so the toy proof does not exercise that
switch; a toy variant with the two series exchanged takes the branch, the port
(`raiseMaximum = false`) is raster-identical to the original there, and setting it to
`true` changes the `ystarM_phase1_input_thrown.pdf` raster.

Reproduced, not fixed:

- `selection/mc_studies/get_data_hists_RF.C` writes the summed `_mc` 2-D histogram
  as `costheta_ystar_all_thrown` and the summed `_thrown` one as
  `costheta_ystar_all_mc` (the 1-D `tdist_all_*` are right); `make_plot_RF.C`
  reads them under those names.
- Same macro: the Spring 2018 copies of the 2-D histograms are cloned as
  `costheta_ystar_M<kind>` (the other periods `costheta_ystarM<kind>`), so
  `Spring_2018/costheta_ystarM_acceptcorr` holds an object named
  `costheta_ystar_M_qval`. The port keeps that name.
- Same macro: `Write(name), TObject::kOverwrite;` (misplaced parenthesis) for three
  copied 2-D histograms; no effect on a freshly created file.
- Same macro: the thrown t histogram title is `" ; -t (GeV)^{2} ); Events"` (stray
  parenthesis).
- `simulation/sampling/PrepSampling.C`: the data `ResCosThetaVst_qval` histogram is
  titled `beam_E` on x but filled with `t_dist` (the MC and thrown ones say `t_dist`).
- The Ξ⁻ mass window is `1.31` in the two `get_data_hists_RF.C` and `1.308` in
  `mc_weight_variations/get_data_hists.C`; `qvalue_acc` is `qvalue_decayxim_M*acc_weight`
  in `simulation/validation/get_data_hists_RF.C` and `qvalue_decayxim_M*best_combo` in
  `PrepSampling.C`. Kept per macro.
- `compare_iters.C` builds its legend but never draws it (`drawLegend = false`
  in its port); it reads files named `*_hist2d.root`, but the 3-D keys it reads
  (`ResMassVsCosThetaVsT_Phase1`, `.../Fall_2018/acceptance`) are written by
  `simulation/sampling/getHist3D.C` into `*_hist3d.root`; no macro writes the
  `*_hist2d.root` names it opens.
- `simulation/validation/make_plot_RF.C` reads `Spring_2018/xim_costheta_hf_qval` as the
  Fall 2018 data histogram (copy-paste) and saves `ystarM_phase1_input_thrown.pdf`
  twice, from two identical calls (lines 68-69); its reads were not ported.
- The fill functions were declared with 16 threads and defined with 20. The
  callers used the declared 16, and the port passes 16 everywhere. Single-threaded
  results do not depend on it.
- Period stems and period names are still literals in the macros that were not
  adopted (for example the selection, background, lineshape and q-factor macros).

Open decisions:

- `gen_amp_V2_3D_ac` (validation) is not in `config/samples.yaml`, and
  `Ystar2400_1600_genr8` (MC-weight variations) is configured for 2018-08 only; both
  macros build their stems as data stem plus sample suffix, as before. Adding the
  samples to the config would let `channel.kv` carry them.
- `kpkpkmlamb`'s `dir`/`title` (`Spring_2017`, ...) have no reader yet; the
  exported `title` and `label` of `kpkpxim` have no C++ reader either.
- Not adopted (behaviour unchanged): the flat-tree preps and a cut catalogue
  (three sites with three different filter sets); `selection/mc_studies/make_plot*.C`
  (their `compare_plot` differs from the four validation copies in many
  respects; `make_plot_acceptcorr.C` reads `<period>/t_dist_acceptcorr` from
  `data_RF.root`, a key the macro that writes that file does not write: it holds
  `tdist_qval_acceptcorr`),
  `compare_plot_log`, `merge_plot` and `make_plot` (its
  `hs->Draw("no stack")`, also in `simulation/validation/make_plot_RF.C`, whose
  `make_plot` is never called (its only call is commented out), draws
  the per-period acceptances stacked); the track study's stacked plot; the 3-D
  sampling macros (`getHist3D.C`, `getHist3D_F18.C`); the non-`_RF` drivers and
  `get_data_hists_ellipse.C`, whose MC inputs are not preserved; the selection
  study macros (cut studies, kinematics data/MC, rapidity plots; the kinematics: see section 17).

Limits:

- Weighted histograms filled with implicit multithreading are not bit-reproducible
  (the default thread count of the macros), so every comparison above ran with
  `ROOT_MAX_THREADS=1`.
- `channel.kv` stores the absolute path of the config directory: a moved
  checkout must export again. The staleness check covers the `config/*.yaml`
  files present at export time; a YAML file added later is not noticed.

## 17. Analysis studies (`packages/studies`, `gxana run studies`): behaviour kept and open decisions

What exists now. A study is a block in `analyses/<channel>/config/studies.yaml`, run by an app that
takes only arguments (`gxana_study_cutscan fill|fit|plot`, `gxana_study_datamc fill|plot`) through
the stage `gxana run studies --channel C [--study a,b] [--steps ...] [--dry-run]`. The stage builds
every command from the block and prints it, checks the inputs of each step (a missing one is listed;
for a step that reads an earlier step's output the listing names the command that makes it, for an
input no study makes (raw trees, the kinematics trees) it names only the study) and runs nothing of a step with a missing input, and it stops at
the first failing command. It creates the output directories, which the macros assumed to exist.
kpkpxim has three studies: the cut scans `chisqndf_scan` and `mm2_scan` (kind `cutscan`) and the
data/MC kinematics `kinematics` (kind `datamc`). The fills go through `gxana::FillHists` /
`gxana::FillPeriodHists`, whose `HistDef` gained two options for this: a one-column histogram with
no axes uses RDataFrame's automatic binning, and `like` takes the binning of a histogram already
written under that key. The cut-scan fits go through `gxana::fit`. The kinematics plots are drawn by
`gxana::studies::DrawStacked`, which holds the drawing statements of the macro's `MakeStackedHist`;
`gxana::DrawOverlay` is not used for them.

Adopted, and how each was checked (every comparison single-threaded with `ROOT_MAX_THREADS=1`,
ROOT 6.40; nothing is checked on 6.24):

- `selection/CutAnalysisRF.C` (now `archive/root_macros/`) is the study of `chisqndf_scan` and
  `mm2_scan`. Its raw trees are not preserved, so the check uses three seeded toy raw trees (20000
  events, one per period). The original macro run twice and the then in-repo macro gave the same
  36 files; the study run through `gxana run studies` equals them: the 18 FOM, S/B and yield tables
  byte for byte, the rasters of the 18 PDFs (Ghostscript, 100 dpi; per scan and period one fit grid,
  one FOM/S/B plot and its copy) and the 816 printed fit lines. `packages/studies/tests/python/
  test_cutscan_equivalence.py` repeats this against a frozen copy of the macro, comparing the printed
  fit lines as a multiset (both sides now run with the minimiser pin and the fit step is one
  process, section 15); it is skipped without ROOT or the built app, and its raster comparison is a
  test of its own that is skipped, with the reason shown by `pytest -rs`, when Ghostscript is not
  installed. A copy with the |MM²| cut changed to 0.03 fails it
  (tried once by hand; the mutation is not a test).
- `selection/GetKinematicsDataMC_RF.C` (now `archive/root_macros/`) is the study `kinematics`, checked
  on the preserved trees of the three periods. The original macro run twice and the then in-repo
  macro gave the same 108 PDFs, 12 sampled rasters and 219 printed lines. A copy of the macro that
  writes its histograms right after they are filled gave 72 histograms per period: 29 data, 29 MC,
  7 thrown and 7 MC histograms of the truth loop. The study's histograms equal all 216 in object name,
  title, binning, entries, contents and errors (the study writes no separate truth-loop MC
  histogram; its `<var>_mc` is compared with the loop's). The study writes the same list of 108
  PDFs, and a sample of 12 (four per period, covering data and thrown plots and both legend
  positions) is raster-identical; the 219 printed lines (sums and bin widths) are identical. The
  other 96 PDFs rest on the identical histograms and the shared drawing code, not on rasters.
  `packages/studies/tests/python/test_kinematics_equivalence.py` repeats the comparison for one period
  (2018-08) against a frozen copy of the macro, run through its per-period function `GetDataMCPlots`
  with `n_threads = 0` on links to the preserved trees; the study side is the stage's own plan for
  that period (the configuration with the other periods removed) with `--threads 0` added to the
  fill. It asserts the same 36 PDF files, the same printed lines in the same order (the weighted sum
  and a pair of bin widths per PDF) and, in a separate test, identical rasters of four PDFs (data
  against MC with the legend top right and top left, and the truth plot each way). It takes about 35
  s, is marked `golden`, and skips with its own reason without ROOT, without the built app and
  without the preserved trees; the raster test also skips without Ghostscript. A copy of the
  frozen macro that rebins the MC twice fails both tests (tried by hand). The test runs the fill
  sequentially because with implicit multithreading enabled (even with `ROOT_MAX_THREADS=1`) the
  automatic binning of the data histogram differed from the sequential one in the first plot;
  the three-period comparison above used the same setting on both sides.
  The two study styles are also compared with the `setStyle()` bodies of the macros in
  `packages/common/tests/cpp/test_style.cxx` (`ApplyCutScanStyle`, `ApplyDataMCStyle`, from the same
  start states as the other styles), no longer only through the frozen cut-scan macro.
  A skipped run proves nothing about these studies: run `uv run pytest packages/studies/tests/python
  -rs` and read the skip reasons (no ROOT, no built app, no preserved trees, no Ghostscript).

Reproduced, not fixed:

- The χ²/ndf fit grid is divided into 4 × 5 = 20 pads for 21 fits (the pad count comes from the
  bins after the first fit bin, the loop runs one more); the 21st fit asks for a pad the canvas does
  not have. Which pad it lands on is not checked.
- The RF tables carry no `_kphighrap` suffix (the PDFs do), so they are written to the same paths
  as the tables of the older `CutAnalysis.C`.
- The |MM²| scan applies `chisqndf < 15`, not the nominal cut 8.
- `MakeStackedHist` takes the scale direction from the maxima but the factor from the integrals;
  when the two disagree the MC is scaled away from the data's area.
- The kinematics PDF tags say `ver56`/`ver03`/`ver02`; the period stems say `ana56`/`ana03`/`ana02`.
- The kinematics PDF names are `<branch>_weighted_qvalue_acc_<tag>[_MC_Truth]_ac.pdf` (RDataFrame's
  default histogram name for the weighted data histogram), the truth plots named after that
  histogram too.
- The kinematics figures read the rapidity branches of `docs/KNOWN_ISSUES.md` section 1, with the same caveat.

Left out because they have no effect on any output: `min_mass` and `xifsBins` (computed or declared,
never used; no Ξ⁻ path-length scan runs), `sigYieldErr` (computed, only used in commented-out
lines), the `RDataFrame` filter names (only a cut-flow report would show them), and the dead
`entries`/`sprintf` of `MakeStackedHist` (a never-drawn "%.0f Events" text), which `DrawStacked`
does not carry over.

Changed on purpose: the cut-scan histogram is filled with `Histo2D` instead of a `Foreach` into one
shared TH2 with implicit multithreading, whose `Fill` is not thread-safe (single-threaded the two
give identical contents). New files: `<scan>Cut_hist_flatTree_<stem>.root` (the TH2D `cutscan`, so
`fit` and `plot` rerun without the raw trees) and `data_mc_kinematics/kinematics_kphighrap.root` (per
period directory `<var>`, `<var>_mc` and `<var>_thrown`).

Left as macros, and why:

- `CutAnalysis.C`, now `archive/root_macros/CutAnalysis.C`: the older selection (`best_combo==1`,
  kaon momentum cuts, no weight) with its own style placement (`setStyle()` inside `GetCutAnalysis`)
  and values (margins 0.05, `AutoPrecision(1)`); it has no configured use, and porting it would need a
  second style and panel-label rule for a test-only config. Archived on 2026-10-02 as migrated
  (nothing was ported; its `gxana` style and fit calls are kept).
- `GetKinematicsDataMC.C`, now `archive/root_macros/GetKinematicsDataMC.C` (archived on 2026-10-02 as
  migrated, superseded by the `datamc` study), and the non-`_RF` copies of the cut-study fill macros
  (`get_data_hists.C`): they read older MC samples (`Ystar2400_1600_genr8`,
  `gen_amp_..._Weighted`) that are not preserved (read from the macro text for
  `GetKinematicsDataMC.C`, `chisqndf_cut` and `kaon_selection`; the cut-studies README states it for
  the others). `get_data_hists_ellipse.C` reads legacy stems, and `chisqndf_2017.C` is a
  ROOT-generated histogram dump with no entry function.
- `selection/cut_studies/{chisqndf_cut,mm2_cut,xim_vertex_cuts,lambda_vertex_cut,kaon_selection}/`
  and `rapidity_cuts/`: the 13 plot macros hold 27 `TLine`/`TArrow` statements between them (cut
  positions are argument defaults or literals): 5 are commented out (in `PlotKPlusLowP.C` and three
  of the `*Comparison.C`), and of the 22 live ones 5 never reach a `Draw` (the `Draw` is commented out
  in `kaon_selection/make_plot.C` for two arrows, in `PlotKPlusLowRapidity.C` for a line and an
  arrow, in `PlotTDistComparison.C` for one line), so 17 are drawn; three of the eight `rapidity_cuts/Plot*.C`
  draw a `TBox` instead of a line (`PlotKPlusHighComparison.C`, `PlotKPlusLowComparison.C`,
  `PlotKPlusMomSepComparison.C`) and `PlotKPlusLowP.C` and `PlotKPlusLowRapidity.C` draw no decoration at all. The macros rescale the MC by their own rule (to the data
  maximum in `chisqndf_cut` and `mm2_cut`, by an integral ratio in the others). Beyond that, the
  fill macros set their own binning, `kaon_selection` fills 2-D histograms with 60 momentum bins for
  data and 100 for MC, the two vertex studies fill from a filtered sub-frame (`df1`), and `rapidity_cuts` sums the periods
  (`combineAllHistograms`) and compares `t_dist` with `t_dist_truth`. Expressing them would need a
  decoration and option engine with one user per option. Both vertex studies also read the Q-factor
  output of the un-suffixed `_nominal` tree, which the standard run does not make.
- `selection/cut_studies/accidentals/`: not run. `get_data_hists.C` writes `data_momCut.root` and
  then opens `data_allKaonSep.root`, whose producing call is commented out; its MC files carry
  the `_momCut` variant, which the cut-studies README does not list; `make_plot.C` draws Fall 2018
  only.

Closed decisions: `CutAnalysis.C` and `GetKinematicsDataMC.C` were archived on 2026-10-02 as
superseded; the `cut_studies` fill macros stay macros (not moved onto `gxana::FillPeriodHists`).

Open decisions for the author: a Breit-Wigner model for a kpkpkmlamb cut scan (the `cutscan` model is
the legacy Johnson + Chebychev, parameter names included).

Limits:

- The cut scans were checked on toy trees only; a comparison with previously produced FOM tables
  needs the raw trees, which are not preserved.
- Only single-threaded runs were compared. The shipped kpkpxim `studies.yaml` now runs single-threaded
  (`threads: 0` on `chisqndf_scan`, inherited by `mm2_scan`, and on `kinematics`, passed as
  `--threads 0` to the fill). The apps keep the macros' thread defaults (4 for the cut scan, 8 for
  the kinematics) when a study sets no `threads`, so a multi-threaded run, possible by config, can
  differ in the last bits of weighted sums and is not reproducible bit for bit (the kinematics data
  histogram binning also depends on the thread mode); the macro's multi-threaded cut-scan fill is not thread-safe, so macro and study
  are not comparable bit for bit in that mode.
- The ROOT 6.24 container was not verified.
- Adding `studies.yaml` changed the config checksums in `channel.kv`: a previously exported file is
  stale until `gxana config export --channel kpkpxim` is rerun.
- `gxana run` prints each command with a buffered `print`; with stdout redirected to a file a command
  line can end up inside an app's output (seen while checking). Set `PYTHONUNBUFFERED=1` when the
  log is parsed.

## 18. Measurements stage and the run-period check (`gxana run measurements`): behaviour kept and open decisions

What exists now. `gxana run measurements --channel C [--item a,b] [--steps prep,fit] [--dry-run]`
runs the measurement macros listed in `analyses/<channel>/config/measurements.yaml`: per item a prep
and/or a fit macro, each as `root -l -b -q rootlogon.C <macro>(<args>)` with the working directory
set to the `output_dir` of the block, prep before fit, stopping at the first command that fails and
returning its exit code. The stage only builds the commands. It creates `output_dir` and the
`make_dirs` directories first (a macro that saves a PDF into a missing directory prints an error and
still exits 0), and creates nothing with `--dry-run`. No macro was changed: they write their ROOT
files to the current directory and their PDFs where they did before. kpkpxim has the items `mass`,
`lifetime` and `spin` (`args: [0]`, so each macro runs with `n_threads = 0`); kpkpkmlamb has `ximstar`
and `ximstar_tcut` (`FitXimStar.C(4, false)` and `FitXimStar.C(4, true)`). The macro arguments must
be plain C++ literals; anything else is a configuration error before anything runs. A configured
macro file that does not exist is also a configuration error, naming the item, the step and the
path. A missing input is not checked: the macro's own output and exit code show it. The
`runperiod` step of `gxana run systematics` builds its command with the same helper; its command is
unchanged.

Adopted, and how each was checked (single-threaded, `ROOT_MAX_THREADS=1` and `n_threads` 0, ROOT
6.40; nothing is checked on 6.24):

- The six kpkpxim macros (`PrepMass.C`, `FitMass.C`, `PrepLifetime.C`, `FitLifetime.C`,
  `PrepSpinData.C`, `PlotGlueXSpin.C`), run twice by hand, gave the recorded `FITRESULT` reference
  (13 lines, section 9), the same histograms in `xim_mass.root`, `xim_lifetime.root` and
  `xim_spin.root`, the same 13 PDFs and identical rasters (Ghostscript, 100 dpi). Through the stage
  the three ROOT files equal the hand run key by key in title, binning, entries, contents and
  errors, the `FITRESULT` lines equal the reference, the same 13 PDFs are written, and six sampled
  rasters are identical. The input is the post-Q-factor tree standing in for the plain data tree
  (section 9).
  `tests/golden/test_measurements_stage_golden.py` pins the `FITRESULT` lines, the three file names
  in the output directory and the 13 PDF names in `prod_plots`, which it removes first so that the
  stage must create it. The histogram and raster comparisons with the hand run were made once and
  are not in a committed test.
- `FitXimStar.C` through the stage, on the seeded toy tree: both PDFs (`Xi1820massFit.pdf`,
  `Xi1820massFit_TCut3.pdf`) are raster-identical to the macro run by hand. With `tCut = false` the
  hand run is raster-identical to the original `FitXimStar.C`. With `tCut = true` (`t_dist > 2`)
  there is no runnable original (section 3), so it is compared only with the same macro run by hand
  (and with itself in two runs).
  `tests/kpkpkmlamb/test_kpkpkmlamb_measurements_stage.py` pins the stage against the hand run; the
  comparison with the original was made once.
- The run-period check (`GetRunPeriodPctSig.C` behind `--steps runperiod`) was not changed. On the
  preserved per-period `johnson` tables (24 per-period tables and 8 weighted ones) the in-repo macro,
  run by hand twice and through the stage, equals the original
  (`AnalysisNote/systematics/GetRunPeriodPctSig.C` with its two directories pointed at the tables and
  the run directory, run twice): the same 221 filtered output lines (168 point significances and
  three Gaussian fits with means 0.927561, 0.878222 and 0.979189 under Minuit2, before the minimiser
  pin of section 15), the same 27 PDFs and 108 raster
  comparisons without a difference. So the difference from the dissertation figure in `docs/KNOWN_ISSUES.md` section 6
  comes from the inputs, not from the macro.
  `tests/golden/test_runperiod_golden.py` runs the stage's `runperiod` step against a frozen copy of
  the original (`tests/golden/legacy/runperiod/`, its two directories templated, both sides run
  after `rootlogon.C` so they share the minimiser pin) and asserts the
  same printed lines (lines that carry run paths, and the return-value line, left out), the three
  means as TMinuit prints them (9.27529e-01 / 8.78222e-01 / 9.79189e-01), the same 27 PDF names and, in a test of its own, identical rasters. It
  skips without ROOT or the preserved tables, and its raster test skips with its reason shown by
  `pytest -rs` when Ghostscript is not installed. Changing the number of bins in a copy of the
  frozen macro from 25 to 24 fails the line and the raster tests (tried by hand; the mutation is not
  a test).

Left as it is, and why:

- `GetRunPeriodPctSig.C` keeps its channel literals (period display names, energy bins, file stems,
  pair titles, output names). The stage-plan fixtures fix the hook's command, `channel.kv` carries
  the period stems and titles but no energy bins or pair titles, and no other channel has a cross
  section. Making it channel-agnostic would need new `channel.kv` content or a changed hook command
  (which regenerates the fixtures); neither was done.
- No Q-factor validation step was built. `GetQvalueSum.C` prints only the Q-weighted sum and saves a
  figure that shows no fit (its fit yield is computed but neither printed nor saved: it is drawn
  on a canvas that is not saved). Of what such a step would report, only the Q-weighted sum (with
  the legacy weight and tree name) has an original output; the fit yield and the ratio have none
  to be checked against. `GetQvalueSum.C`, `PlotTOF.C` and
  `MakeParamTrees.C` stay in `signal_extraction/qfactors/scripts/`.
- No new study kinds for the lineshape, background-reflection, MC-iteration and MC-reweighting
  macros: each would be a generic engine with one user.

Behaviour kept:

- The shipped kpkpxim `measurements.yaml` now runs every macro with `n_threads = 0` (`args: [0]`),
  as the committed tests do; the kpkpkmlamb items keep 4. A multi-threaded run, possible by config,
  is not reproducible bit for bit: it differs in the last digits of the fit errors and the original
  mass fit spreads up to about 8.8e-3 (section 9).
- `GetRunPeriodPctSig.C`: the ratio and significance graphs have y errors 0; the ratio is filled
  into its histogram for every point, including an infinite value where the second period's cross
  section is 0 (not checked on the preserved tables); the Spring 2017 : Fall 2018 ratio histogram is
  named `ratio_s18` (internal only); the significance histograms are never filled (the `Fill` is
  commented out); the Gaussian fit values appear only on the canvases and in ROOT's printed fit
  output.

Closed in section 19 (2026-10-02): no Q-factor validation step (`GetQvalueSum.C` stays a
script); `MakeXim1820_IM.C` and `KstarFit.C` stay scripts and are not archived; no golden for
`compare_iters_2D.C`; the lineshape macros stay macros.

Open decisions for the author:

- Archiving `PlotTOF.C` and `MakeParamTrees.C`.
- A Ξ(1820) mass and width model spread for kpkpkmlamb, if wanted, is smallest as a model choice
  in `FitXimStar.C` driven by new `measurements.yaml` items plus a small spread over their
  outputs.
- Background-reflection fits: `YstarBWFitsData.C` only prints and already uses `gxana::fit`; a golden
  on the preserved `..._nominal_kphighrap_1111111` post-Q-factor file would pin its printed yields.
- Making the run-period check channel-agnostic, and validating its `runperiod` block in the
  systematics configuration (no defect seen).

MC reweighting: `systematics/track_efficiency/WeightMC.C` (byte-identical to
`mc_weight_variations/WeightMC.C`) was removed on 2026-10-02; `mc_weight_variations/WeightMC.C` and
`get_data_hists.C` stay, because they serve a superseded sample with no preserved input.

Limits:

- Only single-threaded runs were compared; the shipped kpkpxim configuration is single-threaded
  (section 17 for the studies).
- The ROOT 6.24 container was not verified.
- Adding `measurements.yaml` changed the config checksums in `channel.kv`: a previously exported
  file is stale (re-export it after the `threads` and `args` change too), and the measurement macros stop with the export hint until
  `gxana config export --channel kpkpxim` is rerun.

## 19. Packaging rule: what stays a script

Packaging rule:

- Code becomes package code only if the thesis calls it many times (per bin, per variation, per
  period) or it is a general method another channel would run as is. Everything else stays a
  standalone script with a README run command and, where its inputs are preserved, a golden test.

What is a package:

- `GxanaXsec`: the per-bin yield fits (`YieldFit`: Johnson, `JohnsonMCShape`, `MCPdf`, Voigtian),
  used by `gxana run xsection`, `barlow` and `systematics`.
- `GxanaStudies`: the cut-scan and data-versus-MC studies (section 17).
- `GxanaCommon`: periods, per-period histograms, acceptance correction, comparison plots
  (section 16).
- `GxanaFit`: used by 10 analysis macros and the cut-scan study, plus the archived `CutAnalysis.C`;
  it is not extended further (section 15).

Done in this closeout:

- Energy panels are ordered by the full `emin` value, with the name as tie-break (sections 10 and 12).
- The thesis studies and the kpkpxim measurement macros run single-threaded: `threads: 0` and
  `args: [0]` (sections 17 and 18).
- `selection/CutAnalysis.C` and `selection/GetKinematicsDataMC.C` were archived to
  `archive/root_macros/`, and the duplicate `systematics/track_efficiency/WeightMC.C` was deleted
  (section 17, closed decisions).

What stays a script:

- Lineshape and mass fits: `MakeXim1320_IM*.C`, `MakeXim1820_IM.C`, `KstarFit.C`,
  `YstarBWFitsData.C`, `compare_iters*.C`. They are thesis-specific and are not archived.
- `GetQvalueSum.C`: the Q-factor validation step is not wanted as a stage.
- `flatTreeCutsMC.C`: left as is.
- `GetRunPeriodPctSig.C`: kept behind the run-period hook.
- The two spin-fit copies: both are kept.

Dropped plan items, one reason each:

- Rebuilding `YieldFit` on `GxanaFit`: the per-bin fit is already the `GxanaXsec` package.
- Study kinds for the lineshape, background-reflection and iteration-comparison macros:
  one-off thesis figures, not called per bin or variation.
- A Q-factor validation kind: `GetQvalueSum.C` stays a script.
- DSelector helpers: not shared with another channel.

Kept as recorded, not fixed:

- `get_data_hists_RF.C` keeps its own copy of the 1-D acceptance.
- The graph-reader adoptions were skipped; the mixed-bin panel of `PlotXSecComponents.C` stays as
  recorded.
- The QFactors fork keeps the gxana file copy.
- The thesis findings (the `ATan` angle, `dIsMC`, the gate) are recorded only.
- The stacked-overlay and swapped-name items are recorded only.
- The F18 mass table versus figure is an author physics call.

Author-run items:

- The ROOT 6.24 container golden run on the ifarm (command in `docs/analysis_data.md`, golden
  tests); it has not been run yet.
- The ifarm kpkpkmlamb bunch-count check; the kpkpkmlamb fixes are applied only after it, and only
  if those results are claimed.
- Merging the stacked branches, after the container run.
