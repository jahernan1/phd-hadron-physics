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

## Running

```sh
gxana run barlow --channel kpkpxim [--steps trees,check,bin,tables,weight,plot] [--dry-run]
```

| Step | Runs | Legacy script |
|---|---|---|
| `trees` | `gxana_barlow_trees` | `GetVariationTreesUML.C` (snapshots) |
| `check` (opt-in) | `gxana_barlow_trees --check` | `GetVariationTreesUML.C` (yield fits) |
| `bin` | `gxana_xsec_bin variation` | `SplitVariationTrees.C` |
| `tables` | `gxana_xsec_tables --fit JohnsonMCShapeSyst` | `GetXSecFilesUML.C` |
| `weight` | `gxana_xsection.weighted_average` | `GetWeightedXsecFile.py` |
| `plot` | `gxana_barlow_plot` | `PlotXSecBarlow*.C` |

Output layout under `$GXANA_OUTPUT/<channel>/barlow/`: `variations.json`, `variation_trees/`,
`xsection_data/<label>/`, `fits/<label>/`, `weighted_data/<label>/`, `plots/barlow_*.{pdf,txt}`,
and `output_yields.txt` (`check`).

Another analysis reuses the package by writing `analyses/<channel>/config/barlow.yaml`; see
`analyses/kpkpxim/config/barlow.yaml` for every key.
