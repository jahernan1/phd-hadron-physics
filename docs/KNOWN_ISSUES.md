# Known Issues

Findings from the legacy-to-clean-repo migration that affect interpretation
of existing results or explain deviations from a literal AnalysisNote copy.
Nothing here should reference a private site path or a tracking-issue
identifier; see the migration plan documents for those.

## 1. Rapidity / pseudorapidity branch swap (spec D18)

The AnalysisNote `flatTreePrep.C` defined `*_rapidity` as
`atanh(p4.Pz()/p4.P())` (that is pseudorapidity) and `*_prapidity` as
`TLorentzVector::Rapidity()` (true rapidity) — the names were swapped for
`kphigh`, `kplow`, and `ystar`. `analyses/kpkpxim/selection/flatTreePrep.C`
now defines these branches with the gx1 `PrepFlatTrees.C` naming instead:
`*_rapidity` is `TLorentzVector::Rapidity()` and `*_prapidity` is
`atanh(pz/p)`.

The nominal K⁺(high) rapidity cut and the Barlow `kphigh_prap` systematic
variations call `TLorentzVector::Rapidity()` directly on the four-vector
(never through the swapped branch names), so those selections are
unaffected by either the bug or the fix.

Figures and tables that read the swapped branch names directly are affected
and must be regenerated:
- The rapidity cut-study plots, `selection/cut_studies/rapidity_cuts/get_data_hists.C`.
- The kinematics comparisons, `gxana run studies --channel kpkpxim --study kinematics` (formerly `selection/GetKinematicsDataMC_RF.C`).

Legacy binned trees produced before this fix carry `kphigh_prapidity`
(and the `kplow_`/`ystar_` equivalents) holding true rapidity; after this
fix, the same branch name holds pseudorapidity. Any script consuming a
pre-fix binned tree must account for this when comparing to trees produced
by the migrated `flatTreePrep.C`.

`flatTreePrepQVal.C` does not define any `*_rapidity`/`*_prapidity`
branches, so it is unaffected by the swap and was not changed.

## 2. Documented, not fixed

The following legacy behaviors are preserved as faithful copies and are
not in scope for this migration:
- DSelector `cout` spam during processing.
- Commented-out PID ΔT and Ξ⁻ mass-window cuts.
- `Xim1320Properties.h` and `Xim1320Properties.cpp` disagreed; fixed on
  migration (b6e3cef aligned the header to the .cpp); both are now in
  `archive/root_macros/`.
- `PlotComponents.C` name clash; fixed on migration (f58181b renamed the
  function).
- `GetBarlowResults.C` reads from `xsection/data_files`, which nothing produces; archived under `archive/systematics_legacy/`.
- `MakeHistoQVal.C` defines `MakeHistos()`, not `MakeHistoQVal()`; archived rather than fixed.

## 3. Legacy drivers that never ran as checked in

`RunAnalysis.sh` and `RunXSec.py` are archived rather than migrated as
working drivers: neither reflects the order analysis stages were actually
run in. `analyses/kpkpxim/README.md` documents the real stage order.

## 4. Fixed in their own migration plans

- QFactors `qvalueSum` was never initialised and the `fitParams` branch used a variable-length array: both fixed in the fork (`packages/qfactors`, see its `CHANGES_THESIS.md`, Plan 5). The fork also resets `chiSqNdf_<var>` to NaN for events whose fit is not drawn; the thesis configuration drew every event, so thesis outputs are unaffected.
- MCwrapper `MakeMC.sh` used csh's `>>!` redirection, which bash reads as a write to a file named `!`: fixed in `packages/montecarlo/patches/gluex_MCwrapper/0002-*` (Plan 4). The `MakeMC.csh` copy is correct as is.

## 5. kpkpkmlamb (side channel)

- `DSelector_kpkpkmlamb.C` prints per event and per combo, and still defines
  the template's example branches; kept as run.
- Legacy `flatTreePrep.C` and `get_data_hists.C` repeated default arguments on
  function definitions, which cling rejects; the migrated `flatTreePrep.C`
  keeps them on the declaration only. `get_data_hists.C` (a kpkpxim macro) is
  archived.
- Legacy `FitXimStarCuts.C` did not compile (`histTitlek` undeclared); its
  selection and label now run as `FitXimStar(n, true)`.
- The fit reads the merged GlueX-I tree; the legacy tree was made by hand, the
  README's `hadd` command reproduces it.

## 6. Published label and totals (reconciled 2026-09-29)

- The dissertation and analysis-note tables are the legacy label `johnson`
  (Johnson + 2nd-order Chebychev fit, weight `hybrid_combo`, run-period
  weighted average, scale-factor run systematic). The `hybrid_combo` label
  (`JohnsonMCShape`, legacy `MakeXSecFiles.C`, `*_runsyst.tex`) is a study;
  earlier repo text called it the dissertation result. The golden tests now
  reproduce the `*_scale.tex` tables from `weighted_data/johnson`.
- The Barlow systematics were produced by the UML chain
  (`GetVariationTreesUML.C` and the `GetXSecFilesUML.C` fit); the non-UML
  drafts were never used and live in `archive/systematics_legacy/`.
  `gxana run barlow` replaces the whole chain (`gxana_barlow_trees`, the
  xsection package with `JohnsonMCShapeSyst`, `gxana_barlow_plot`). No variation trees are preserved,
  so that fit is transcribed, not golden-tested.
- The Barlow plots are pixel-compared with the archived macros' output and the σ_B tables with the legacy formula; the variation-tree fits and the `check` yields have no golden (no variation trees are preserved).
- The nominal selection has no −t cut. The energy-only bins (direct total
  cross section) keep every −t; the legacy `MakeBinnedTrees.cpp` later added
  `t_dist<2.4` to them, which the port does not apply. The integrated total
  (`intxsec_*`, Σ dσ/dt·Δt over 0.10 < −t < 2.40 GeV²) carries that range as
  an effective cut; the dissertation names it the correct method, while its
  total-cross-section figure was drawn from the direct files.
- Q-factors ran with `kDim: 200` nearest neighbours (the dissertation text
  says 150). The MC reconstruction version sets as run were the newest
  available (`ver01_13 / ver02_32 / ver02_31`); the analysis-note text lists
  older sets.
- `weighted_average` prints an `inf` error where a variation bin is empty
  (NaN row); the legacy script printed `0.000000`. The Barlow weight golden
  test maps one to the other.

## 7. Fit-model systematic (`gxana run systematics --study fit`, checked 2026-09-29)

- The rerun of the eight fit variations on the preserved inputs (ROOT 6.40)
  does not reproduce the preserved `fit_variations_stats.txt` or the ch7
  fit-comparison figures to the thesis precision: YMean agrees to 0.2 %
  (median) but up to 12 %, StdDev to 7 % (median) but up to 72 %. The
  example fits of the dissertation bin reproduce the figures exactly for
  `mcPdf`, `mcPdf_cheby1`, `voigt`, `voigt_cheby1` and `johnson_cheby1`
  (yields 524, 522, 513, 529, 527), but `johnson` gives 500 ± 29 where
  `johnsonFit.pdf` shows 511 ± 25. The rerun `johnson` tables match the
  published ones within the ROOT 6.40 fit tolerance of
  `tests/golden/test_xsec_golden.py` (4e-2 for `johnson`), while the figures' nominal (`johnson`) band sits several
  percent above them, like the stale `processed_weighted_diffxsec_*` files kept in
  `reference/xsection/weighted/johnson/`: the figures and the stats file were
  most likely made from an earlier `johnson` run than the published tables.
  The two 2017-01 −t bins gated in the tables (see the xsection golden) also
  change the `qvalues` average there. The golden test uses tolerances set
  from this run.
- Effect on the published tables (label `johnson`): regenerating the
  dissertation LaTeX tables on ROOT 6.40 (`gxana run systematics --study fit`, then `gxana run xsection --steps tex`) changes
  the fit-model spread column of `syst_diffxsec_table_scale.tex` in 55 of 56
  rows (median 6 %, max 72 %; first row 0.157 against 0.170), and with it the
  total systematic of `diffxsec_table_scale.tex` in 50 of 56 rows (median 3 %,
  max 71 %; first row 0.166 against 0.177). The scale-factor run-period column (from `weighted_average.py`) differs
  in one row by 0.001; dσ/dt moves by at most 0.6 % in the rerun. That check
  fed the preserved `combo_variations_stats.txt` as the accidental-subtraction
  column, which is therefore unchanged in it; the suite's `accidentals` study
  spreads over three methods and changes that column too (section 9). The
  regenerated tables head each column by its own stats file
  (`xsection.tex.columns`, section 8): the fit-model spread, the column that
  changes substantially in the rerun, is "Yield Extraction" there and
  "Accidentals" in the published tables.
- The per-bin data-fit plots use the style of the preserved legacy
  `FitFunctions.cpp` (30 mass bins, a pull panel); the dissertation figures
  (36 bins, no pull panel, a framed parameter box without
  `delta`/`gamma`/`width`) came from a plotting version that is not
  preserved. This is not a change made by the port.
- The `mcPdf` fit of the example bin fails to converge, as it did in the
  thesis (a0 = 0.9117597, a1 = −0.0999999). Its 2nd-order Chebychev is then
  negative at the low mass edge (−0.0118 at 1.275 GeV), so on ROOT 6.40 the
  model normalisation evaluates to NaN while plotting and the total and
  background curves are drawn with NaN points, i.e. not visibly. This was
  confirmed with the fitted parameters and a Gaussian stand-in for the MC
  shape. The edge value 1 − a0 + a1 is negative for a1 < −0.088, and with
  a1 = −0.05 the curves draw normally.

## 8. Accidentals and Yield Extraction columns swapped in the published tables (D1)

The dissertation and AnalysisNote `syst_diffxsec_table_scale.tex` list the fit-model
spread under "Accidentals" and the accidental-subtraction spread under "Yield Extraction".
Legacy `MakeXsecTexTableScale.py` labels the columns by the position of its input files,
and the inputs were given in the opposite order. The totals are unaffected.
`gxana run xsection --steps tex` now maps each column heading to its own stats file
(`xsection.tex.columns`), so regenerated tables carry the labels the other way round from
the published ones.

## 9. Accidentals spread over three methods (D2)

The published Accidentals column is the spread of `acc_weight` and `hybrid_combo` only. The
suite's `accidentals` study spreads over all three methods (`acc_weight`, `best_combo`,
`hybrid_combo`), so that column, and the quadrature total, change relative to the published
tables. The legacy two-member set is reproduced with `spread: [acc_weight, hybrid_combo]`.

## 10. Paired t-test index in the legacy comparison macros

The archived `PlotComboComparison.C:104` and `PlotFitComparison.C:125` accumulate
`sum1 += yValues1[1]`, a fixed index where the loop index was meant. The statistic is printed
only, and the affected `t_stat1` is discarded, so the port's fix in `PlotSpread` has no
observable effect on any printed or returned number.

## 11. Track-efficiency totals

The dissertation quotes track-efficiency totals of 18.58 % (20.29 % with the proton
override); the per-track sum computed by `gxana_systematics.track` on the preserved inputs
(verification run 2026-09-30) is 18.65 % (20.36 %). The suite reports the per-track sum.

## 12. Run-period ratio check (C1) does not reproduce

The opt-in `runperiod` step (`GetRunPeriodPctSig.C`) gives Gaussian means 0.928 / 0.878 /
0.979 for the period ratios with the `johnson` label (verification run 2026-09-30), against
the dissertation figure (for example Sp17:Fa18 mean 0.942). It is a check only; its numbers
are not in the published tables. The Spring-2017 REST-version check
(`PlotRestVComparison.C`) is omitted from the suite pending re-evaluation.

## 13. Acceptance correction: behaviour kept from the original macros

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

## 14. Measurements: behaviour kept from the original macros

The mass, lifetime and spin measurements (`analyses/kpkpxim/measurements/`) were
split from the combined `GetXimProperties.C` and `PlotGlueXSpin.C` into prep and
fit macros. Fit models, ranges, binning, weights, names and plot texts are those
of the originals; the fit macros add one `FITRESULT` line per fit, and the prep
macros print their acceptance check in a new form. The points below are original
behaviour that was reproduced, not fixed.

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
  The single-threaded numbers differ from the multithreaded ones by up to about
  1.5e-3 relative (Spring 2018 data mass yield error), of the same order as the
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
- Lifetime (`lifetime/FitLifetime.C`). The shift `PDG − MC mean` is computed but
  never applied to τ or written out; τ is the acceptance-corrected data fit only.
- Mass (`mass/FitMass.C`). The mass is the data mean plus `1.32171 − mean(MC fit)`;
  the shift is applied to the data mean only and σ is not shifted. `thrown_mass`
  (the centre of the thrown histogram's peak bin) and `pdg_mass` are computed
  (`thrown_mass` is printed) but not used. The data Johnson fit starts from
  fixed numbers, not from the MC fit result. The printed σ error is wrong in
  both fits: the formula uses `delta/deltaErr` where `deltaErr/delta` was
  meant, and for the data fits `gamma` and `delta` are constants, so it divides
  by zero and prints `inf`. It is only printed. Rerunning `FitMass.C` on an
  existing `xim_mass.root` without rerunning `PrepMass.C` adds canvas cycles (`fitCan;2`, ...) because the
  `Write` has no `kOverwrite`; readers get the highest cycle. Without
  `xim_mass.root` it leaves an empty output file behind and stops naming the
  missing histogram.
- Spin (`spin/PlotGlueXSpin.C`). The spin result of record is the fit of the
  merged histogram `pim_costheta_hf_phase1`. The per-period plots keep the
  original per-period function (`GetSpinAnalysisPeriod`), a second copy that had
  diverged from the merged one: it has no free J = 3/2 fit, starts the fixed-β
  J = 3/2 fit from default parameters rather than from the free fit, and draws
  the χ²/ndf values as text instead of a legend. Unifying the two gives the same
  fit values but changes the per-period plots, so both copies are kept. The free
  J = 3/2 fit (`beta1` in the merged `FITRESULT` line) sits at its lower limit 0.
- Merged efficiency (`spin/PrepSpinData.C`). `pim_costheta_hf_avg_accept_phase1`
  is the sum of the three run-period acceptances, not their mean: `kIsAverage` is
  set only on the first histogram and `TH1::Add` averages only when both operands
  carry it. The histogram is only drawn, so no fit depends on it.
- Printed χ²/ndf. The lifetime and spin macros print `GetChisquare()/ndf` after
  `"WLR"` likelihood fits. Under ROOT 6.40 that is twice the negative
  log-likelihood per degree of freedom (lifetime 212 / 586 / 410, spin 164 to
  4723), not a χ². Recomputing with `TH1::Chisquare(f, "R")/ndf` on the same
  histograms reproduces the dissertation's values (lifetime 1.03 / 1.45 /
  1.66; spin 0.77 to 16.33). The fitted parameters are unchanged. The printed
  values are reproduced as they are. The spin β errors printed under ROOT 6.40
  (0.056 / 0.042 / 0.039, merged 0.027) are about ten times the dissertation's
  0.005 / 0.004 / 0.004, merged 0.002; the cause was not established. The Monte
  Carlo mass fit errors (yield error 255 / 337 / 347 against 259 / 290 / 313 in
  the figures) and the Spring 2018 data mass χ²/ndf (1.099–1.100 against 1.147)
  also differ slightly, with identical central values.
- Dissertation tables. The Fall 2018 entries of the Ξ⁻ mass table (MC correction
  −0.77 MeV, corrected mass 1321.27 MeV) disagree with the dissertation's own
  figure (−0.76 MeV, 1321.28 MeV). The macros give −0.762 and
  1321.280–1321.281 MeV (1321.28), the figure values. The other masses
  (1321.32 / 1321.57 MeV), lifetimes (0.1806 / 0.1770 / 0.1960 ns) and β
  central values (0.0706 / 0.0760 / 0.0831, merged 0.0774) agree with the
  dissertation.
- Input tree. The plain data tree `flatTree_<stem>_nominal_kphighrap.root` read
  by the mass fit is not part of the preserved data. The post-Q-factor tree stands
  in for it in the golden test and in the recorded reference; the mass histogram
  uses `hybrid_combo` only, as the original does. The post-Q-factor tree is made
  by `packages/qfactors/mergeQresults.C` as an unfiltered clone of the Q-factor
  input, which `config/qfactors.yaml` sets to the plain tree, so it holds the same
  entries (9893 / 36986 / 32580); the data mass, yield and σ central values
  reproduce the dissertation figures to the printed precision (the Spring 2018
  data yield error prints 79 against the figure's 78).
- Archived. `GetXimProperties.C` is replaced by the prep and fit macros.
  `Xim1320Properties.cpp` and `.h` were an older near-duplicate whose header and
  source disagreed, and `PlotXim1320Properties.C` calls a function that is never
  defined; all are in `archive/root_macros/`. The Johnson-moment mean error of the
  archived `Xim1320Properties.cpp` is a relative-error product with the
  `delta/deltaErr` inversion above; `GetXimProperties.C` (now `FitMass.C`)
  propagates the parameter errors in quadrature. The invariant-mass figure macros
  `MakeXim1320_IM*.C` and `MakeXim1820_IM.C` keep their own copies of the Johnson
  fit and style and are unchanged by this rework. `MakeXim1320_IM()` reads the
  `_ximVertexCut` trees, which are not in the preserved data, so it cannot run
  as preserved; `MakeXim1320_IM_Res()` reads the plain `_kphighrap` tree, which
  is also not preserved (see the input tree above).

## 15. Common plot style, bin names and table readers: behaviour kept

The plot styles, the 3x3-grid gStyle tail, the bin-name format, the energy-bin table
reader and the command-line parsers of `xsection`, `systematics` and `barlow` now live in
`packages/common` (`ApplyStyle` with `ThesisStyle`, `FitStyle`, `ComparisonStyle`,
`TrackStyle`, `BarlowStyle`, `GridTrailingTweak`; `BinNames.h`; `ReadBinnedGraphs`;
`Cli.h`). Every style leaves the same `gStyle` as its original (compared as a full dump,
from the default style and from every other style's state), and every reader returns the
same graphs, so no figure or number changes. Kept as they were:

- `CreateTGraphErrorsFromTxt` (used by `xsection/PlotDiffXSec.C` and `PlotComponents.C`)
  orders tables with `NumericCompare`, which compares only the integer part of emin: with
  the configured edges 7.40/7.86 and 8.19/8.45/8.68 tie, and their order then depends on
  the directory listing and the C++ library. On the preserved weighted tables (macOS,
  labels `johnson`, `hybrid_combo`, `best_combo`, `acc_weight`) the panels come out as
  ... (8.19, 8.45), (8.68, 9.26), (8.45, 8.68) ...; each panel keeps its own title, and
  the ROOT file written alongside keeps that order. The dissertation figures made with
  this code should be checked panel by panel. With a single matched file the comparator
  is never called, so a name without `emin` is titled `(, )`. The analysis macros still
  use this reader unchanged.
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

## 16. Stage command layer: behaviour kept (`gxana run`)

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

## 17. Analysis-macro plot styles: behaviour kept and open decisions

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
`selection/GetKinematicsDataMC_RF.C`, `selection/GetKinematicsDataMC.C` (called with the
gen_amp_V2 `kphighrap` stems), `selection/cut_studies/kaon_selection/make_plot.C` (the
`_kphighrap` plots) and `selection/cut_studies/rapidity_cuts/PlotKPlusLowRapidity.C` (the
last two from the histogram files that their unchanged `get_data_hists` macros write from
the preserved trees). The remaining 29 were not rerun (none of them runs on the preserved
inputs alone): they are verified by the identical `gStyle` state alone (their drawing code
is unchanged). Kept as they were:

- The original bodies created a `TLatex` they never drew; it is no longer created.
  `SetTitleOffset(x,"T")` (in `accidentals/get_data_hists.C` and
  `systematics/GetRunPeriodPctSig.C`) does nothing in ROOT and is not reproduced.
- `setStyle()` is never called in `backgrounds/YstarBWFitsData.C` (its calls are
  commented out) or in `kpkpkmlamb/measurements/FitXimStar.C`; both were converted.
  `systematics/mc_weight_variations/WeightMC.C` and `systematics/track_efficiency/WeightMC.C`
  are the same file.
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

Open decisions (not done, because each changes an output):

1. `xsection/PlotXSecComponents.C` takes each period's tables in directory-listing order
   and draws graph j of every period in pad j, so on the preserved tables one panel
   overlays different energy bins of the three periods under the first period's title.
   Proposed: sort by emin, as its own change with before/after plots, after checking the
   dissertation's component figures panel by panel.
2. `xsection/MakeWeightedDiffXSecTGraphs.C` keeps its own reader: moving it to the common
   `ReadBinnedGraphs` would rename the stored objects from `Graph` to the table stem
   (keys unchanged) and make an empty or `emin`-less table an error instead of an empty
   graph; the common reader also has no full-path `diffxsec` filter (the macro admits
   every `.txt` of a directory whose path contains `diffxsec`). Its `binEdge` parse
   would move with the reader.
3. `PlotXSecComponents.C` `ProcessFilesToTFile` would need a sorted reader with a
   four-column format and a per-prefix grouping that the common library does not have;
   its bin-edge parse reads `emax` without checking that it is present. It goes with
   decision 1.
4. `xsection/PlotDiffXSec.C` and `PlotComponents.C` read through
   `CreateTGraphErrorsFromTxt`, whose integer-part ordering lets tied energy panels follow
   the directory listing (section 15); switching them to `ReadBinnedGraphs` changes the
   panel order of those figures.

Not verified: the macros were checked with ROOT 6.40 only; the GlueX container
(ROOT 6.24) has not been run.

## 18. Q-factor fit models: findings recorded, not fixed

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

## 19. Channel-agnostic packages: behaviour kept and open decisions

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

## 20. Fit library (`packages/fit`): behaviour kept and open decisions

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
  instead of δ_err/δ and ignores λ (infinite in the data fits, see section
  14). `MakeXim1320_IM*.C` compute a relative-error sum with the same
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
- The macros do not pin the minimiser or the evaluation backend, so on
  ROOT ≥ 6.30 they run Minuit2 with the new backend, unlike the thesis-era
  ROOT 6.24 runs. Pinning them is an open decision.

## 21. Per-period histograms and comparison plots (`packages/common`): behaviour kept and open decisions

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
  study macros (cut studies, kinematics data/MC, rapidity plots; the kinematics: see section 22).

Limits:

- Weighted histograms filled with implicit multithreading are not bit-reproducible
  (the default thread count of the macros), so every comparison above ran with
  `ROOT_MAX_THREADS=1`.
- `channel.kv` stores the absolute path of the config directory: a moved
  checkout must export again. The staleness check covers the `config/*.yaml`
  files present at export time; a YAML file added later is not noticed.

## 22. Analysis studies (`packages/studies`, `gxana run studies`): behaviour kept and open decisions

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
  fit lines as a multiset; it is skipped without ROOT or the built app, and its raster comparison is a
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
- The kinematics figures read the rapidity branches of section 1, with the same caveat.

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

- `selection/CutAnalysis.C`: the older selection (`best_combo==1`, kaon momentum cuts, no weight)
  with its own style placement (`setStyle()` inside `GetCutAnalysis`) and values (margins 0.05,
  `AutoPrecision(1)`); it has no configured use, and porting it would need a second style and
  panel-label rule for a test-only config. Not archived (nothing was ported).
- `selection/GetKinematicsDataMC.C` and the non-`_RF` copies of the cut-study fill macros
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

Open decisions for the author: archive `CutAnalysis.C` and `GetKinematicsDataMC.C` as superseded;
move the `cut_studies` fill macros onto `gxana::FillPeriodHists` in place (as other drivers were),
keeping their plot macros; a Breit-Wigner model for a kpkpkmlamb cut scan (the `cutscan` model is
the legacy Johnson + Chebychev, parameter names included).

Limits:

- The cut scans were checked on toy trees only; a comparison with previously produced FOM tables
  needs the raw trees, which are not preserved.
- Only single-threaded runs were compared. The apps keep the macros' thread defaults (4 for the cut
  scan, 8 for the kinematics, `--threads`), so a multi-threaded run can differ in the last bits of
  weighted sums; the macro's multi-threaded cut-scan fill is not thread-safe, so macro and study
  are not comparable bit for bit in that mode.
- The ROOT 6.24 container was not verified.
- Adding `studies.yaml` changed the config checksums in `channel.kv`: a previously exported file is
  stale until `gxana config export --channel kpkpxim` is rerun.
- `gxana run` prints each command with a buffered `print`; with stdout redirected to a file a command
  line can end up inside an app's output (seen while checking). Set `PYTHONUNBUFFERED=1` when the
  log is parsed.
