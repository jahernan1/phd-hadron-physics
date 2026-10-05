# kpkpxim cross-section figures

The figures of the dissertation's cross-section results (chapter 6) and their inputs.

| Dissertation figure | Macro | Drawn from |
|---|---|---|
| `diffxsec_runs_johnson.pdf` (dσ/dt per run period) | `PlotDiffXSec.C` | `$GXANA_OUTPUT/kpkpxim/xsection/data/johnson/diffxsec_*` |
| `diffxsec_phase1_systematics_johnson.pdf` (weighted dσ/dt, statistical bars, systematic band) | `PlotDiffXSec.C` | `weighted_data/johnson/weighted_diffxsec_*` and `syst_weighted_diffxsec_*` |
| `totxsec_clas_gluex_Phase1.pdf` (total σ with CLAS, exponential fit) | `PlotTotXsecWithClas.C` | `$GXANA_OUTPUT/kpkpxim/systematics/variants/{data,weighted_data}/hybrid_combo/totxsec_*` |

Run them, after `gxana run xsection` and `gxana run systematics`:

    gxana run xsection --channel kpkpxim --steps figures

The step (config `xsection.figures` in `../config/xsection.yaml`) first writes
`syst_weighted_diffxsec_*.txt` beside the weighted tables of `figures.label` with
`python -m gxana_xsection.syst_tables`: the weighted table with the total systematic as
column 4, the quadrature sum of `figures.columns` (default `xsection.tex.columns`, the
stats files of `gxana run systematics`; `scale_factor` = δy·S − δy from the table's S
column). It then runs each macro of `figures.plots` from `figures.output_dir`
(`$GXANA_OUTPUT/kpkpxim/xsection/figures/`). Missing inputs stop the step with the list.
Besides the PDFs the macros write the drawn graphs:
`{Weighted,SystWeighted}DiffXSecTGraphs_johnson.root`, `DiffXSecTGraphs_<period>_johnson.root`
and `totxsec_clas_gluex_Phase1.root` (graphs `clas`, `weighted`, `sp17`, `sp18`, `fa18`
and the `fit`).

By hand (the arguments default to the paths above; run conventions and the output tree in
[`docs/MACROS_AND_OUTPUTS.md`](../../../docs/MACROS_AND_OUTPUTS.md)):

    python -m gxana_xsection.syst_tables $GXANA_OUTPUT/kpkpxim/xsection/weighted_data/johnson --column NAME=FILE ...
    root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/xsection/PlotDiffXSec.C
    root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/xsection/PlotTotXsecWithClas.C

(`PlotDiffXSec.C(xsecDir, label, plotDir)` defaults to `$GXANA_OUTPUT/kpkpxim/xsection`,
`johnson`, `<xsecDir>/figures`; `PlotTotXsecWithClas.C(xsecDir, label, plotDir)` to
`$GXANA_OUTPUT/kpkpxim/systematics/variants`, `hybrid_combo`,
`$GXANA_OUTPUT/kpkpxim/xsection/figures`; optional `dataDir` and `weightedDir` replace
`<xsecDir>/data/<label>` and `<xsecDir>/weighted_data/<label>`.)

## Reproduce the published figures exactly

The published systematic band is the quadrature sum of the preserved
`fit_variations_stats.txt`, `combo_variations_stats.txt` and the scale-factor run
systematic. With the default columns (the regenerated systematics) the band changes like
the regenerated tables (`docs/KNOWN_ISSUES.md` sections 3 and 8). To draw the published
band:

    gxana run xsection --channel kpkpxim --steps figures --systematics published

`--systematics published` puts `xsection.published_systematics.figures` over
`xsection.figures`: those three columns in that order, and the total-cross-section figure
from the preserved `hybrid_combo` tables (`PlotTotXsecWithClas.C` with explicit data and
weighted directories). It needs no `gxana run systematics`.

`tests/golden/test_xsec_figures_golden.py` runs the step that way on the preserved tables
and checks the drawn numbers against the published table.

The total-cross-section figure is drawn from the JohnsonMCShape study label `hybrid_combo`
(direct totals), as the dissertation figure was, not from the published label `johnson`
(`docs/KNOWN_ISSUES.md` section 2).

## Other macros

- `PlotXSecComponents.C` — data, MC and thrown yields and acceptance per run period from
  `$GXANA_OUTPUT/kpkpxim/xsection/components/` (after `gxana run xsection --steps components`)
  into `$GXANA_OUTPUT/kpkpxim/xsection/plots/`; its `*_runs_johnson.pdf` are the dissertation's
  chapter-6 yield and acceptance figures. It also loops over the labels `hybrid_combo`,
  `mcPdf` and `mcPdf_cheby1`, which the preserved-data route does not produce.
- `PlotComponents.C` — dσ/dt per run period and weighted for the study labels `hybrid_combo`,
  `acc_weight`, `best_combo`, `qvalues` and `oneRfBunch` (not in the dissertation;
  `docs/history/PORT_NOTES.md` sections 10 and 12).
- `flux/` — [flux input](flux/README.md); `external_data/` — [CLAS points](external_data/README.md).
