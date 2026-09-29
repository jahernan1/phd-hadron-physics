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
- The kinematics comparisons, `selection/GetKinematicsDataMC_RF.C`.

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
  migration (b6e3cef aligned the header to the .cpp).
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
