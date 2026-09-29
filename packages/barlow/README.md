# barlow

Barlow cut-variation consistency check, run as `gxana run barlow --channel <ch>`.
Configured by `analyses/<channel>/config/barlow.yaml`; another analysis reuses it by
writing that one file.

| Header | Contents |
|---|---|
| `Barlow.h` | `calc_barlow` (signed σ_B = Δ/√\|σ_n² − σ_v²\|, 0 when that is 0), `calculateStdDevGraph`, `BarlowPlotStyle`, `BarlowPlotSpec`, `PlotBarlow` (`gxana_barlow_plot`): the `PlotXSecBarlow*.C` Barlow plots, per-family differences as arguments |
| `VariationTrees.h` | `WriteVariationTrees` (`gxana_barlow_trees`): data/MC snapshots per variation; `CheckVariationYields` (`--check`): legacy nominal-vs-variation Johnson yield side check |

Apps: `gxana_barlow_trees` (variation trees, `--check`) and `gxana_barlow_plot` (Barlow plots and
σ_B tables per variation family).
