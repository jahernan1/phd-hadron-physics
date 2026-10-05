# Nominal-versus-variant cross-section comparisons (dissertation chapters 6 and 7)

The comparison macros that lived here now run as studies of
`gxana run systematics` (`config/systematics.yaml`, package `packages/systematics`).
The drawing is ported verbatim into `gxana_syst_plot` (`PlotSpread`, one layout per
legacy plotting function); the numbers come from `gxana_systematics`. The legacy
macros are kept, unchanged, in `archive/systematics_legacy/comparisons/`.

| Legacy macro | Study (`systematics.studies`) | Step | Layout / numbers | Output (`$GXANA_OUTPUT/kpkpxim/systematics/<study>/`) |
|---|---|---|---|---|
| `PlotComboComparison.C` | `accidentals` | `spread` (default) | `grid3`, `pair_band`; `gxana_systematics.spread` | `plots/weighted_diffxsec_{ComboSelection,Combos_StdDev}.pdf`, `combo_variations_stats.txt` |
| `PlotFitComparison.C` | `fit` | `spread` (default) | `grid3` (x3), `all_band`; `gxana_systematics.spread` | `plots/weighted_diffxsec_{SignalFitVoigt,SignalFitMC,SignalFitJohn,AllFits}.pdf`, `fit_variations_stats.txt` |
| `PlotRunComparison.C` | `run_compare` | `compare` (opt-in) | `run_grid`, `stddev_band`; `gxana_systematics.runcompare` (std dev across periods scaled by the PDG S where S > 1) | `plots/weighted_diffxsec_{RunComparison,RunCompStdDevScaled}.pdf`, `run_comp_stddev_scaled.txt` |
| `PlotQValueComparison.C` | `qval_yield` | `compare` (opt-in) | `grid2` | `plots/weighted_diffxsec_QValYield.pdf` |
| `PlotBunchComparison.C` | `bunch` | `compare` (opt-in) | `grid2` (the weighted overlay only; the per-period Spring/Fall 2018 panels are not ported) | `plots/weighted_diffxsec_oneRFBunch.pdf` |
| `PlotFitBkgdComparison.C` | `bkgd` | `compare` (opt-in) | `grid2` (x-axis ndiv 205) | `plots/weighted_diffxsec_bkgdfit.pdf` |
| `PlotRestVComparison.C` | none | | omitted pending re-evaluation (Spring-2017 REST-version check) | |

```sh
gxana run systematics --channel kpkpxim                    # default steps, incl. spread
gxana run systematics --channel kpkpxim --steps compare    # the opt-in checks
gxana run systematics --channel kpkpxim --steps compare --study run_compare
```

`run_compare` reads the per-period tables of the nominal,
`$GXANA_OUTPUT/kpkpxim/xsection/data/<nominal>/diffxsec_flatTree_<stem>_emin_*_emax_*.txt`
(`gxana run xsection --steps tables`). The `grid2` checks read weighted label tables of
the systematics variant pool; a check whose label is not configured in
`systematics.variants` (`oneRfBunch`, `bkgd`), or whose tables do not exist yet, is skipped
with a note.

Which stage reads each output: [`docs/MACROS_AND_OUTPUTS.md`](../../../../docs/MACROS_AND_OUTPUTS.md#gxana_outputkpkpxim).

The run-period ratio/significance macro, `../GetRunPeriodPctSig.C`, runs as the opt-in
`runperiod` step and writes to `$GXANA_OUTPUT/kpkpxim/systematics/runperiod/`.
