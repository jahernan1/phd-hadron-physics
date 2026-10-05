# ROOT macros and output layout

How to run the channel ROOT macros by hand, and where every stage and macro
reads and writes. Examples are for `kpkpxim`; another channel replaces the
channel directory name. Variable defaults are in
[environment.md](environment.md#variables).

## Running the ROOT macros

```sh
cd $GXANA_OUTPUT/kpkpxim/<dir>
root -l -b -q $GXANA_ROOT/rootlogon.C $GXANA_ROOT/analyses/kpkpxim/<path>/<Macro>.C
```

`$GXANA_ROOT/rootlogon.C` comes first. With `GXANA_ROOT` set it adds
`$GXANA_ROOT/build/lib` to the dynamic path, adds the include directories of
`packages/{common,xsection,barlow,systematics,fit}`, loads `libGxanaCommon`,
`libGxanaPeriodHists`, `libGxanaXsec`, `libGxanaBarlow`, `libGxanaSystematics`
and `libGxanaFit` (a warning names a missing one: run `cmake --build build`) and
calls `gxana::fit::UseThesisMinimizer()`. With `GXANA_ROOT` unset it does
nothing, and every macro that includes a `gxana/...` header or calls the style
presets fails to load. The stages run macros the same way
(`root -l -b -q $GXANA_ROOT/rootlogon.C <macro>(<args>)`, `gxana.stages.runner.root_macro`).

### Paths

Macros build every path with `gxana::EnvPath(var, rel)`
(`packages/common/include/gxana/common/Paths.h`): `$var` joined with `rel`; an
unset or empty variable throws `<var> is not set; run source env/setup.sh`.
The macros read three variables:

| Variable | Used for |
|---|---|
| `GXANA_DATA` | flat trees: `Trees/flatTree/rawTrees/`, `flatTrees/` |
| `GXANA_OUTPUT` | stage outputs read back (`<channel>/qfactors/`, `<channel>/xsection/`, ...), absolute output paths, `<channel>/config/channel.kv` |
| `GXANA_ROOT` | repository files (`xsection/PlotTotXsecWithClas.C` reads `analyses/kpkpxim/xsection/external_data/Clas_data.csv`) |

`GXANA_ANALYSIS_DATA` is read by the stage configs only (flux, preserved
reference tables), never by a macro.

### Working directory

Many macros write their ROOT files and PDFs into the current directory (the
cut-study and MC-study fills, the measurement prep macros). Run each from its
output directory under `$GXANA_OUTPUT/<channel>/` (create it first), never from
the repository; each folder README names the directory. Some save into a
subdirectory of the current directory that must exist (`plots/` in
`cut_studies/kaon_selection` and `cut_studies/accidentals`).

### Entry function

`root <file>.C` calls the function named like the file. When a file defines
another name, load it and call the function:

```sh
root -l -b -q -e ".x $GXANA_ROOT/rootlogon.C" -e ".L $GXANA_ROOT/analyses/kpkpxim/selection/cut_studies/rapidity_cuts/PlotKPlusLowP.C" -e "PlotKPlusComparison()"
```

Arguments go in parentheses after the file name; quote the whole argument for
the shell, for example
`"$GXANA_ROOT/analyses/kpkpxim/measurements/mass/PrepMass.C(0)"`.

### Threads

Entry functions that fill with RDataFrame take a thread count; `0` turns
implicit multithreading off. With threads on, the entry order of written trees
and the summation order of fills change from run to run.

| Macro | Argument | Default |
|---|---|---|
| `selection/flatTreePrep.C` | `flatTreePrep(name, n_threads)` | 8 |
| `measurements/*/Prep*.C`, `Fit*.C`, `PlotGlueXSpin.C` | `n_threads` | 4; the stage passes 0 ([measurements reproducibility](../analyses/kpkpxim/measurements/README.md#reproducibility)) |
| `measurements/mass/MakeXim1320_IM.C`, `MakeXim1320_IM_Res.C` | `GetXim1320_IM(delim, n_threads)` | 8 (forward declaration) |

The cut-study `get_data_hists*.C` fill helpers (`save_from_flattrees`) enable 16
or 20 threads internally; it is not an argument of the entry function. Stage
configs set threads with `threads:` (`studies.yaml`, `barlow.trees.threads`;
both 0 as shipped) or with the macro `args` (`measurements.yaml`).

### Macros that read `channel.kv`

These macros read the run periods, ROOT directory names and tree stems from
`$GXANA_OUTPUT/<channel>/config/channel.kv` (`gxana::ChannelInfo::Load`,
`packages/common/include/gxana/common/Periods.h`). Write it first, and again
after any edit of `analyses/<channel>/config/*.yaml`; the file records the MD5
of every config file and a stale one is refused (`stale channel.kv: run gxana
config export --channel C`):

```sh
gxana config export --channel kpkpxim
```

| Macro (`analyses/kpkpxim/`) | Through |
|---|---|
| `measurements/mass/PrepMass.C`, `FitMass.C` | `XimPeriods()` in `measurements/common/XimInputs.h` |
| `measurements/lifetime/PrepLifetime.C`, `FitLifetime.C` | `XimPeriods()` |
| `measurements/spin/PrepSpinData.C`, `PlotGlueXSpin.C` | `XimPeriods()` |
| `selection/mc_studies/get_data_hists_RF.C` | `gxana::MakePeriods` |
| `simulation/sampling/PrepSampling.C` | `gxana::MakePeriods` |
| `simulation/validation/get_data_hists_RF.C` | `gxana::MakePeriods` |
| `simulation/validation/in_out_test.C` | `ChannelInfo::Stem` |
| `systematics/mc_weight_variations/get_data_hists.C` | `gxana::MakePeriods` |

`gxana run measurements` runs four of these and does not export the file
itself. Every other macro has its period stems written in the source.

## Output layout

### `$GXANA_DATA`

| Path | Written by | Read by |
|---|---|---|
| `Trees/tree_<stem>/{trees,thrown}/` | skims; for MC the `ln -sfn` lines `gxana run mc` prints (`samples.yaml` `tree_dir_template`) | `gxana run select` |
| `Trees/flatTree/rawTrees/flatTree_<stem>.root` | `gxana run select` (data and reconstructed MC) | `selection/flatTreePrep.C`, `gxana run barlow` (`trees`), `gxana run studies` (cut scans), cut-study `get_data_hists*.C` |
| `flatTrees/flatTree_<stem>_nominal[_<variant>].root` | `selection/flatTreePrep.C`; the MC `_nominal_kphighrap` trees also by `gxana data stage` (copy) | `gxana run qfactors`, `gxana run xsection` (`bin`, MC), `gxana run systematics` (`track`), `gxana run studies` (`kinematics`), `gxana run barlow` (`check`), measurements, cut and MC studies |
| `flatTrees/flatTree_thrown_<mc_stem>.root` | `gxana run select --thrown`; `gxana data stage` (link) | `gxana run xsection` (`bin`), `gxana run systematics` (`track`), `gxana run studies`, measurements, MC studies |

### `$GXANA_OUTPUT/kpkpxim/`

| Directory | Written by | Read by |
|---|---|---|
| `config/channel.kv` | `gxana config export --channel kpkpxim` | the macros listed above |
| `selector_hists/` | `gxana run select` (DSelector histogram files) | none |
| `mc/tree_<stem>/` | `gxana run mc` (rendered configs, MCwrapper run directory) | the `ln -sfn` lines it prints |
| `qfactors/<stem>_nominal_kphighrap_1111111/postQVal_flatTree_*.root` | `gxana run qfactors` (`plots` step); `gxana data stage` (copy) | `gxana run xsection` (`bin`), `gxana run systematics` (`track`), `gxana run studies` (`kinematics`), measurements, cut and MC studies, `simulation/sampling/PrepSampling.C` |
| `qfactors_plots/` | `gxana run qfactors` (`histograms/`, `diagnosticPlots/`, linked from the work directory under `$GXANA_SCRATCH/qfactors/`) | none |
| `xsection/binned_trees/` | `gxana run xsection` (`bin`); `gxana data stage` (copy) | `xsection` (`tables`), `gxana run systematics` (`fit`), `gxana run barlow` (`tables`: the thrown binned trees) |
| `xsection/data/<label>/` | `gxana run xsection` (`tables`) | `xsection` (`weight`, `integrate`, `components`), `gxana run systematics` (`spread`: run scale factor; `runperiod`, `compare`), `PlotDiffXSec.C` |
| `xsection/fits/<label>/` | `gxana run xsection` (`tables`, per-bin fit PDFs) | none |
| `xsection/weighted_data/<label>/` | `gxana run xsection` (`weight`, `integrate`); `syst_weighted_diffxsec_*` by `figures` | `xsection` (`tex`, `figures`), `gxana run systematics` (nominal check, `summary`), `gxana run barlow` (`plot`), `PlotDiffXSec.C` |
| `xsection/components/<period label>/<label>/` | `gxana run xsection` (`components`) | `xsection/PlotXSecComponents.C` |
| `xsection/tables/` | `gxana run xsection` (`tex`) | none |
| `xsection/figures/` | `gxana run xsection` (`figures`), `PlotDiffXSec.C`, `PlotTotXsecWithClas.C` | tests |
| `xsection/plots/` | `xsection/PlotXSecComponents.C`, `xsection/PlotComponents.C` | none |
| `systematics/variants/{data,fits,weighted_data}/<label>/` | `gxana run systematics` (`fit`, `qvalue`, `weight`) | `systematics` (`spread`, `compare`), `PlotTotXsecWithClas.C` (`hybrid_combo`) |
| `systematics/<study>/` (`run`, `accidentals`, `fit`, `track`, `run_compare`, ...) | `gxana run systematics` (`spread`, `track`, `compare`): stats files, `plots/` | `xsection` (`tex`, `figures`: the stats files), `systematics` (`summary`) |
| `systematics/summary/` | `gxana run systematics` (`summary`) | none |
| `systematics/runperiod/` | `gxana run systematics --steps runperiod` | none |
| `barlow/variations.json`, `barlow/variation_trees/` | `gxana run barlow` (`trees`, `bin`) | `barlow` (`bin`, `tables`) |
| `barlow/xsection_data/<label>/`, `barlow/fits/<label>/` | `gxana run barlow` (`tables`) | `barlow` (`weight`); `systematics/combine_pdf.sh` (`fits/`) |
| `barlow/weighted_data/<label>/` | `gxana run barlow` (`weight`) | `barlow` (`plot`) |
| `barlow/plots/` | `gxana run barlow` (`plot`) | none |
| `barlow/output_yields.txt`, `barlow/fits/` | `gxana run barlow --steps check` | none |
| `barlow/combined_pdf/` | `systematics/combine_pdf.sh` (run from `barlow/`) | none |
| `measurements/` | `gxana run measurements` or the measurement macros (`xim_mass.root`, `xim_lifetime.root`, `xim_spin.root`) | the fit macros |
| `prod_plots/` | measurement fit macros, `measurements/mass/MakeXim1320_IM*.C` | none |
| `cut_analysis_plots/` | `gxana run studies` (`chisqndf_scan`, `mm2_scan`) | none |
| `data_mc_kinematics/` | `gxana run studies` (`kinematics`) | none |
| `analysis/event_selection/<study>/` | cut-study and MC-study macros, run there by hand; the cut scans also copy their plots into `chisqndf_cut/` | the matching `make_plot*.C` |
| `MC/` | placed by hand (`data_ac_ximVertexCut_hist2d_YstarRest.root` from `PrepSampling.C`) | `selection/mc_studies/get_data_hists_RF.C`, `simulation/validation/compare_iters*.C` |
| `simulation/sampling/` | `simulation/sampling/PrepSampling.C`, run there by hand | copied to `$GXANA_ANALYSIS_DATA/kpkpxim/simulation/sampling/` for `gen_amp_cfg/` |
| `backgrounds/`, `systematics/mc_weight_variations/` | the macros of those folders, run there by hand | none |

Every stage path above is a key of `analyses/kpkpxim/config/*.yaml`
(`output_dir`, `out_dir`, `inputs`, `make_dirs`) or is built from one in
`packages/common/python/gxana/stages/` and the package stages
(`gxana_systematics.stage`, `gxana_barlow.stage`, `gxana_studies.stage`).
Before running a step, `gxana run xsection` and `gxana run systematics` check
its inputs and print the command that makes each missing one.

The preserved inputs (`$GXANA_ANALYSIS_DATA/kpkpxim/`) and what
`gxana data stage` places are in [analysis_data.md](analysis_data.md).
