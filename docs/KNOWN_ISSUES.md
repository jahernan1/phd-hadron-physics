# Known Issues

Findings that change, or disagree with, a published thesis or analysis-note
number, table or figure. How the legacy code was ported, what it kept and how
each port was checked is in `docs/PORT_NOTES.md`. Nothing here should
reference a private site path or a tracking-issue identifier.

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

## 2. Published label and totals (reconciled 2026-09-29)

- The dissertation and analysis-note tables are the legacy label `johnson`
  (Johnson + 2nd-order Chebychev fit, weight `hybrid_combo`, run-period
  weighted average, scale-factor run systematic). The `hybrid_combo` label
  (`JohnsonMCShape`, legacy `MakeXSecFiles.C`, `*_runsyst.tex`) is a study;
  earlier repo text called it the dissertation result. The golden tests now
  reproduce the `*_scale.tex` tables from `weighted_data/johnson`.
- The Barlow systematics were produced by the UML chain
  (`GetVariationTreesUML.C` and the `GetXSecFilesUML.C` fit).
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

## 3. Fit-model systematic (`gxana run systematics --study fit`, checked 2026-09-29)

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
  spreads over three methods and changes that column too (`docs/PORT_NOTES.md` section 6). The
  regenerated tables head each column by its own stats file
  (`xsection.tex.columns`, section 4): the fit-model spread, the column that
  changes substantially in the rerun, is "Yield Extraction" there and
  "Accidentals" in the published tables.
- The `mcPdf` fit of the example bin fails to converge, as it did in the
  thesis (a0 = 0.9117597, a1 = −0.0999999). Its 2nd-order Chebychev is then
  negative at the low mass edge (−0.0118 at 1.275 GeV), so on ROOT 6.40 the
  model normalisation evaluates to NaN while plotting and the total and
  background curves are drawn with NaN points, i.e. not visibly. This was
  confirmed with the fitted parameters and a Gaussian stand-in for the MC
  shape. The edge value 1 − a0 + a1 is negative for a1 < −0.088, and with
  a1 = −0.05 the curves draw normally.

## 4. Accidentals and Yield Extraction columns swapped in the published tables (D1)

The dissertation and AnalysisNote `syst_diffxsec_table_scale.tex` list the fit-model
spread under "Accidentals" and the accidental-subtraction spread under "Yield Extraction".
Legacy `MakeXsecTexTableScale.py` labels the columns by the position of its input files,
and the inputs were given in the opposite order. The totals are unaffected.
`gxana run xsection --steps tex` now maps each column heading to its own stats file
(`xsection.tex.columns`), so regenerated tables carry the labels the other way round from
the published ones.

## 5. Track-efficiency totals

The dissertation quotes track-efficiency totals of 18.58 % (20.29 % with the proton
override); the per-track sum computed by `gxana_systematics.track` on the preserved inputs
(verification run 2026-09-30) is 18.65 % (20.36 %). The suite reports the per-track sum.

## 6. Run-period ratio check (C1) does not reproduce

The opt-in `runperiod` step (`GetRunPeriodPctSig.C`) gives Gaussian means 0.928 / 0.878 /
0.979 for the period ratios with the `johnson` label (verification run 2026-09-30), against
the dissertation figure (for example Sp17:Fa18 mean 0.942). It is a check only; its numbers
are not in the published tables. The Spring-2017 REST-version check
(`PlotRestVComparison.C`) is omitted from the suite pending re-evaluation.

## 7. Measurements: behaviour kept from the original macros

The mass, lifetime and spin measurements (`analyses/kpkpxim/measurements/`) were
split from the combined `GetXimProperties.C` and `PlotGlueXSpin.C` into prep and
fit macros. Fit models, ranges, binning, weights, names and plot texts are those
of the originals; the fit macros add one `FITRESULT` line per fit, and the prep
macros print their acceptance check in a new form. The points below are original
behaviour that was reproduced, not fixed.

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

## 8. Port findings that may affect thesis figures and tables

Recorded in full in `docs/PORT_NOTES.md`; listed here because they bear on
published figures or numbers. None was changed.

- Energy-panel order (PORT_NOTES section 10): the legacy
  `CreateTGraphErrorsFromTxt` ordered tables by the integer part of emin, so
  the bins 7.40/7.86 and 8.19/8.45/8.68 came out in directory-listing order.
  Figures made with the legacy `xsection/PlotDiffXSec.C` and `PlotComponents.C`
  may show those panels out of order; compare the dissertation figures panel
  by panel.
- Mixed-bin panel (PORT_NOTES section 12): `xsection/PlotXSecComponents.C`
  draws graph j of every period in pad j, in directory-listing order, so on the
  preserved tables one panel overlays different energy bins of the three
  periods under the first period's title.
- Cross-section inputs (PORT_NOTES section 14):
  - The fit gate `(hybrid_combo)*(decayxim_M>1.3&&decayxim_M<1.35)` is used for
    every label, so the `best_combo` and `acc_weight` labels may fit a bin
    their own weight would gate out, or the reverse.
  - The branching-ratio uncertainty (0.005 on 0.641) is added in quadrature to
    every point's error: a fully correlated normalization inside
    point-by-point errors.
  - The target density is the 2018 value (70.08e-3 g/cm^3) for all three run
    periods.
- Selection studies (PORT_NOTES section 17):
  - The |MM²| cut scan applies `chisqndf < 15`, not the nominal cut 8.
  - The data/MC kinematics plots (`MakeStackedHist`) take the scale direction
    from the maxima but the factor from the integrals; when the two disagree
    the MC is scaled away from the data's area.
