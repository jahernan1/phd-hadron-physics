# Adding a reaction channel

This guide is for an analyst who wants to run the framework on a reaction
other than γp → K⁺K⁺Ξ⁻. It walks through creating `analyses/<channel>/`,
writing its configuration, checking every stage with `--dry-run`, and then
running it. `analyses/kpkpkmlamb` (γp → K⁺K⁺K⁻Λ, selection and a mass fit)
is the worked example; `tests/fixtures/channels/analyses/kpkpkmlamb/config/`
is the same channel with a synthetic MC sample and every cross-section,
Barlow and systematics block filled in, and serves as the example for those
stages. The last two sections are a reference of every configuration key the
code reads and a list of gotchas.

## What belongs to a channel

| Where | What | Channel-specific? |
|---|---|---|
| `analyses/<channel>/config/*.yaml` | every name, cut, path, binning and fit setting the stages use | yes |
| `analyses/<channel>/selectors/` | the DSelector (`.C`/`.h`) run by `gxana run select` | yes |
| `analyses/<channel>/<anything>/*.C` | flat-tree preparation and measurement or plot macros, called by path from the config (`measurements.items`, `xsection.figures.plots`, `systematics.runperiod`) | yes |
| `analyses/<channel>/README.md` | what the channel measures, its pipeline and directory map | yes (required by the tests) |
| `analyses/<channel>/analysis_data.yaml` | manifest of preserved inputs and reference outputs, only if the channel has any ([analysis_data.md](analysis_data.md)) | yes, optional |
| `packages/common` | `gxana` CLI, config loader, stage drivers, `channel.kv` reader for C++ | no |
| `packages/xsection`, `packages/barlow`, `packages/systematics`, `packages/studies`, `packages/fit` | C++ apps and Python helpers; every channel fact reaches them as command-line flags | no |
| `packages/qfactors` | QFactors engine (git submodule); the thesis fit model `configPDFs.h` lives here | no, but see [Gotchas](#gotchas) |
| `packages/montecarlo` | MCwrapper/halld_sim pins and helper scripts | no |

Package code names no channel; `tests/test_no_channel_literals.py` enforces
that. If a stage needs a channel fact that is not yet a config key, add the
key rather than a literal.

## Step by step

### 0. Environment

```bash
source env/setup.sh          # GXANA_DATA, GXANA_OUTPUT, GXANA_SCRATCH, ... (docs/environment.md)
uv run gxana doctor
```

`--dry-run` plans need only these variables. Real `select` runs need
gluex_root_analysis: `source env/setup.sh --gluex` or the container. Real `mc`
runs need `source env/setup.sh --sim=<set>`. Every later stage (`flatTreePrep.C`,
`qfactors`, `xsection`, `systematics`, `barlow`, `measurements`, `studies`) needs
only ROOT, on any machine, once the raw flat trees `select` writes are copied
into `$GXANA_DATA`.

### 1. Create the channel directory

```bash
mkdir -p analyses/<channel>/config analyses/<channel>/selectors
```

Then write `analyses/<channel>/README.md`. `tests/test_readme_commands.py`
requires that it exists, that it contains at least one `cd` command, that
every `cd` target starts with `$GXANA_OUTPUT/`, and that every
`analyses/...*.C` path it names exists. `tests/test_docs_layout.py`
requires the root `README.md` to name the directory as
`` `analyses/<channel>` ``.

Only `channel.yaml`, `periods.yaml` and `samples.yaml` are needed to run the
selector. Add one file per further stage. File names are free: the loader
merges every `config/*.yaml` into one mapping, and a top-level key defined in
two files is an error.

### 2. `channel.yaml`

kpkpkmlamb (`analyses/kpkpkmlamb/config/channel.yaml`):

```yaml
channel: kpkpkmlamb
reaction: kpkpkmlamb
selector_dir: analyses/kpkpkmlamb/selectors
default_selector: DSelector_kpkpkmlamb.C
output_basename: kpkpkmlamb.root   # must equal the selector's dOutputFileName
```

The cross-section, Barlow and systematics stages also need a `physics` block.
The fixture adds it for kpkpkmlamb:

```yaml
physics:
  flat_tree: flatTree_kpkpkmlamb
  thrown_flat_tree: flatTree_thrown_kpkpkmlamb
  observable: {branch: ximstar_M, title: "M(#LambdaK^{-}) (GeV/c^{2})"}
  qvalue_branch: null               # no Q-factors for this channel
  branching_ratio: {value: 0.641, error: 0.005}
  reaction_title: "#gamma p#rightarrow K^{+}K^{+}K^{-}#Lambda"
```

### 3. `periods.yaml` and `samples.yaml`

```yaml
# analyses/kpkpkmlamb/config/periods.yaml
periods:
  "2017-01": {launch: ana55, fit_prefix: "B4_M18_", label: sp17, dir: Spring_2017, title: "Spring 2017"}
  "2018-01": {launch: ana22, fit_prefix: "B4_M18_", label: sp18, dir: Spring_2018, title: "Spring 2018"}
  "2018-08": {launch: ana19, fit_prefix: "B4_M18_", label: fa18, dir: Fall_2018, title: "Fall 2018"}
```

```yaml
# analyses/kpkpkmlamb/config/samples.yaml
tree_dir_template: "Trees/kpkpkmlamb/tree_{stem}/{kind}/"
samples:
  data: {mc: false}
```

Each (period, sample) pair has a tree stem
`<reaction>__<fit_prefix><period>_<launch><tree_suffix>`; for kpkpkmlamb data
in 2017-01 that is `kpkpkmlamb__B4_M18_2017-01_ana55`, and the selector reads
`$GXANA_DATA/Trees/kpkpkmlamb/tree_kpkpkmlamb__B4_M18_2017-01_ana55/trees/`.
For a cross section add the MC sample (`{mc: true}`), a `flux` file per period
and, for `gxana run mc`, the `runs` range; the fixture's `periods.yaml` and
`samples.yaml` show the first two.

### 4. The selector

Generate the DSelector from the gluex_root_analysis template and place it in
`selector_dir`. Its `dOutputFileName` must equal `output_basename`; the flat
tree it writes is then `flatTree_<output_basename>`, and its tree name is what
`physics.flat_tree` names. `analyses/kpkpkmlamb/selectors/README.md` is an
example of what to record.

### 5. Check the merged config and export it for C++

```bash
uv run gxana config show --channel kpkpkmlamb      # the merged YAML, as the stages see it
uv run gxana config export --channel kpkpkmlamb    # writes $GXANA_OUTPUT/kpkpkmlamb/config/channel.kv
```

`config show` fails on a duplicate top-level key. `config export` fails if a
period lacks `label`, `dir` or `title`. Every C++ macro that reads the run
periods (`gxana::ChannelInfo`) refuses a `channel.kv` whose recorded MD5 of a
`config/*.yaml` no longer matches, so rerun the export after every config
edit (see [`channel.kv`](#channelkv-read-by-c)).

### 6. Dry-run every stage, then run it

Every `gxana run` stage takes `--dry-run`, which prints the planned commands
and runs nothing. Flags follow the stage name. For kpkpkmlamb:

```bash
uv run gxana run select --channel kpkpkmlamb --period 2017-01 --sample data --dry-run
uv run gxana run measurements --channel kpkpkmlamb --dry-run
uv run gxana run measurements --channel kpkpkmlamb --item ximstar --steps fit --dry-run
```

The `select` plan shows the tree directory, the selector, the run directory
and where the histogram and flat-tree files will be saved. Check those paths
before the real run:

```bash
source env/setup.sh --gluex
for p in 2017-01 2018-01 2018-08; do
  uv run gxana run select --channel kpkpkmlamb --period $p --sample data
done
```

kpkpkmlamb then applies its nominal cuts with
`analyses/kpkpkmlamb/flat_trees/flatTreePrep.C` (run by hand, see its README)
and fits with `uv run gxana run measurements --channel kpkpkmlamb`.

For the stages kpkpkmlamb does not configure, the fixture shows the plans of
a complete config. Pointing `GXANA_ROOT` at it is for dry runs only (macro
paths then resolve under the fixture):

```bash
GXANA_ROOT=$PWD/tests/fixtures/channels uv run gxana run xsection --channel kpkpkmlamb --dry-run
GXANA_ROOT=$PWD/tests/fixtures/channels uv run gxana run barlow --channel kpkpkmlamb --dry-run
GXANA_ROOT=$PWD/tests/fixtures/channels uv run gxana run systematics --channel kpkpkmlamb --dry-run
```

A stage whose block is missing stops with
`gxana: error: channel '<channel>' config missing required key '<block>'`.

The order of a full analysis, with the file each stage reads:

| Stage | Command | Reads | Writes |
|---|---|---|---|
| MC production | `gxana run mc --channel C --period P --sample S` | `mc.yaml`, `periods.runs` | MCwrapper jobs; prints the `ln -s` that put the trees where `select` reads |
| Selection | `gxana run select --channel C --period P --sample S [--thrown]` | `channel.yaml`, `periods.yaml`, `samples.yaml` | `$GXANA_DATA/Trees/flatTree/rawTrees/flatTree_<stem>.root` (thrown: `$GXANA_DATA/flatTrees/`) |
| Flat-tree cuts | channel macro, by hand | the raw flat trees | the nominal flat trees the later stages name in their `input`/`inputs` |
| Q-factors | `gxana run qfactors --channel C --period P` | `qfactors.yaml` | `postQVal_flatTree_*.root` |
| Cross section | `gxana run xsection --channel C` | `xsection.yaml`, `binning.yaml`, `physics` | binned trees, per-period and weighted tables |
| Systematics | `gxana run systematics --channel C` | `systematics.yaml` (+ `xsection`) | variant tables, stats files, summary |
| Barlow check | `gxana run barlow --channel C` | `barlow.yaml` (+ `xsection`) | variation tables and plots |
| Studies | `gxana run studies --channel C` | `studies.yaml` | cut scans, data/MC plots |
| Measurements | `gxana run measurements --channel C` | `measurements.yaml` | whatever the channel macros write |

## Configuration key reference

Derived from the code that reads each key. Paths in the "Read by" column are
abbreviated:

| Abbreviation | File |
|---|---|
| `config.py` | `packages/common/python/gxana/config.py` |
| `stages/<x>.py` | `packages/common/python/gxana/stages/<x>.py` |
| `analysis_data.py` | `packages/common/python/gxana/analysis_data.py` |
| `gxana_barlow/<x>.py` | `packages/barlow/python/gxana_barlow/<x>.py` |
| `gxana_systematics/<x>.py` | `packages/systematics/python/gxana_systematics/<x>.py` |
| `gxana_studies/<x>.py` | `packages/studies/python/gxana_studies/<x>.py` |

"Required" means required by the stage that reads the block; a block a
channel never runs can be left out. String values in path keys may use
`${GXANA_*}` variables (`config.py:expand_env`; an unset variable is an
error, except `GXANA_ANALYSIS_DATA`, which falls back to
`<repo>/gluex_analysis_data`). Where a key says so, the placeholders
`{stem}` (the period's data tree stem), `{mc_stem}` (the stem of the MC
sample), `{period}`, `{mc_sample}`, `{variant}`, `{family}`, `{kind}` and
`{what}` are filled in by the stage.

### `channel.yaml`

| Key | Req. | Type | Default | Meaning | Read by |
|---|---|---|---|---|---|
| `channel` | yes | string | — | channel name; must equal the directory name (macros are found at `analyses/<channel>/`, outputs go to `$GXANA_OUTPUT/<channel>/`) | `config.py:channel_kv`, `stages/select.py:plan_select`, `stages/mc.py:plan_mc`, `stages/measurements.py:plan`, every stage's error hints |
| `reaction` | yes | string | — | first part of every tree stem; anchor of the component-table names | `config.py:tree_stem`, `stages/xsection.py:_plan_components` |
| `selector_dir` | select | string | — | selector directory, relative to the repository root | `stages/select.py:plan_select` |
| `default_selector` | select | string | — | selector file for samples without their own `selector` | `config.py:selector_name` |
| `thrown_selector` | `select --thrown` | string | — | thrown-tree selector for samples without their own `thrown_selector` | `config.py:selector_name` |
| `output_basename` | select | string | — | the selector's `dOutputFileName`; the files moved after a run are `<basename>`, `thrown_<basename>`, `flatTree_<basename>`, `flatTree_thrown_<basename>` | `config.py:output_basename`, `stages/select.py:planned_moves` |
| `physics` | xsection, barlow, systematics | mapping | — | channel physics passed to the C++ apps (below) | `config.py:physics`, `config.py:physics_block` |

`physics` (all keys required once the block is used; unknown keys are an
error):

| Key | Type | Meaning | Read by |
|---|---|---|---|
| `flat_tree` | string | tree name in the data and MC flat trees (`gxana_xsec_bin --tree`) | `stages/xsection.py:bin_physics_args` |
| `thrown_flat_tree` | string | tree name in the thrown flat trees | `stages/xsection.py:bin_physics_args` |
| `observable` | mapping | fitted mass: `branch` (branch name) and `title` (ROOT axis title) | `stages/xsection.py:tables_physics_args`, `gxana_barlow/stage.py:check_physics_args` |
| `branch` | string | (in `observable`) the mass branch | as above |
| `title` | string | (in `observable`) its axis title | as above |
| `qvalue_branch` | string or `null` | Q-factor weight branch kept in the binned data trees and summed into the `qval` columns; `null` for a channel without Q-factors (columns are `nan`) | `stages/xsection.py:bin_physics_args`, `tables_physics_args` |
| `branching_ratio` | mapping | `value` and `error` (numbers): product of the branching ratios of the reconstructed decay chain | `stages/xsection.py:tables_physics_args` |
| `reaction_title` | string | ROOT-latex reaction label on the Barlow plots | `gxana_barlow/stage.py:plot_physics_args` |

### `periods.yaml`

`periods` is a mapping from period name (quote it: `"2017-01"`) to its
settings. Its order is the order of the periods everywhere (tables, plots,
`channel.kv`).

| Key | Req. | Type | Default | Meaning | Read by |
|---|---|---|---|---|---|
| `launch` | yes, unless every sample sets it | string | — | analysis-launch tag in the tree directory name (`ana55`) | `config.py:tree_stem` |
| `fit_prefix` | yes, unless every sample sets it | string | — | ReactionFilter flags prefix of the tree name (`B4_M18_`) | `config.py:tree_stem` |
| `label` | yes | string | — | short tag (`sp17`): component-table directories; `channel.kv` | `config.py:channel_kv`, `stages/xsection.py:_plan_components` |
| `dir` | yes | string | — | directory name of the period inside the macros' ROOT files; `channel.kv` | `config.py:channel_kv`, `gxana_studies/stage.py:_period_dir`, C++ `gxana::MakePeriods` |
| `title` | yes | string | — | readable name for plot labels; `channel.kv` | `config.py:channel_kv` |
| `runs` | mc | `[first, last]` | — | run range passed to `gluex_MC.py` | `stages/mc.py:plan_mc` |
| `flux` | xsection, barlow, systematics fit | string | — | tagged-flux file name inside `xsection.inputs.flux_dir` | `stages/xsection.py:tables_commands`, `gxana_barlow/stage.py:_tables_inputs` |

### `samples.yaml`

| Key | Req. | Type | Default | Meaning | Read by |
|---|---|---|---|---|---|
| `tree_dir_template` | yes | string | — | tree directory relative to `$GXANA_DATA`, with `{stem}` and `{kind}` (`trees` or `thrown`); the directory holding `{kind}` must be named `tree_<something>`, which becomes the saved-file name | `config.py:tree_dir`, `stages/select.py:plan_select` |
| `samples` | yes | mapping | — | sample name → settings; xsection, barlow, systematics, studies and `gxana data stage` take the data stem from the sample named `data` | `config.py:sample_settings` |

Per sample:

| Key | Req. | Type | Default | Meaning | Read by |
|---|---|---|---|---|---|
| `mc` | no | bool | false | MC sample: has thrown trees; default `tree_suffix` | `config.py:tree_dir`, `config.py:tree_stem` |
| `launch` | no | string or `{period: launch}` | the period's `launch` | launch tag for this sample; a mapping limits the sample to its periods | `config.py:tree_stem` |
| `fit_prefix` | no | string | the period's `fit_prefix` | flags prefix for this sample | `config.py:tree_stem` |
| `tree_suffix` | no | string | `_<sample>` if `mc`, else empty | end of the stem | `config.py:tree_stem` |
| `selector` | no | string | `default_selector` | selector file in `selector_dir` | `config.py:selector_name` |
| `thrown_selector` | no | string | `thrown_selector` | thrown selector file | `config.py:selector_name` |
| `output_basename` | no | string | channel `output_basename` | the sample selector's `dOutputFileName`; used for a thrown job only if the sample also sets `thrown_selector` | `config.py:output_basename` |

An empty string for any of the optional keys is an error, not "unset".

### `binning.yaml`

| Key | Req. | Type | Meaning | Read by |
|---|---|---|---|---|
| `energy_edges` | xsection, barlow, systematics | list of numbers | beam-energy bin edges (GeV), ascending; table names use two decimals (`gxana/bins.py:edge_label`) | `stages/xsection.py:plan_xsection`, `gxana_barlow/stage.py:_plan_bin`, `gxana_systematics/stage.py:_plan_weight` |
| `t_bins` | xsection, barlow | list of `[lo, hi]` | −t bins (GeV²); each must start where the previous ended | `gxana/bins.py:flatten_t_bins` via `stages/xsection.py:plan_xsection`, `gxana_barlow/stage.py:_plan_bin` |

### `xsection.yaml`

Block `xsection`, read by `gxana run xsection` (`stages/xsection.py`) and,
for its inputs, output directory and flux, by `barlow` and `systematics`.

| Key | Req. | Type | Default | Meaning | Read by |
|---|---|---|---|---|---|
| `mc_sample` | yes | string | — | the signal-MC sample (in `samples`); also `mc_sample` in `channel.kv` | `_plan_bin`, `tables_paths`, `config.py:channel_kv`, `gxana_systematics/stage.py:_track_inputs` |
| `weight` | yes | string | — | event-weight branch of the fits, unless a fit label sets its own | `tables_commands` |
| `inputs` | yes | mapping | — | `data`, `mc`, `thrown`, `flux_dir` (below) | `_plan_bin`, `tables_commands`, `preflight` |
| `data` | yes | string | — | (in `inputs`) data flat tree per period; `{stem}` | `_plan_bin` |
| `mc` | yes | string | — | (in `inputs`) reconstructed MC flat tree; `{mc_stem}` | `_plan_bin` |
| `thrown` | yes | string | — | (in `inputs`) thrown MC flat tree; `{mc_stem}` | `_plan_bin` |
| `flux_dir` | yes | string | — | (in `inputs`) directory of the `periods.<p>.flux` files | `tables_commands`, `gxana_barlow/stage.py:_tables_inputs` |
| `output_dir` | yes | string | — | root of all xsection outputs (`binned_trees/`, `data/<label>/`, `weighted_data/<label>/`, `components/`) | `_resolve_xcfg`; barlow and systematics `_xs_output` |
| `fit_plots` | no | string | no fit PDFs | directory for per-bin fit PDFs (`--plots`) | `_plan_tables` |
| `fits` | tables | list | — | one `gxana_xsec_tables` process per entry: `model`, `params`, `labels` (below) | `tables_commands` |
| `model` | yes | string | — | (in a fit) `Johnson`, `Gaussian`, `Voigtian`, `JohnsonMCShape`, `JohnsonMCShapeSyst` or `MCPdf` | `tables_commands` |
| `params` | yes | mapping | — | (in a fit) parameter → `[start, min, max]`; `{}` for `MCPdf` | `tables_commands` |
| `labels` | yes | list | — | (in a fit) `{label, cheby, weight}` entries fitted in order, parameters carried over | `tables_commands` |
| `label` | yes | string | — | (in `labels`) output directory name `data/<label>/` | `tables_commands` |
| `cheby` | yes | int | — | (in `labels`) Chebychev background order | `tables_commands` |
| `weight` | no | string | `xsection.weight` | (in `labels`) weight branch for this label | `tables_commands` |
| `weighted_labels` | yes | list | — | labels averaged over periods (`weight`, `integrate`) | `_plan_weight`, `_plan_integrate`, `_output_dirs` |
| `component_labels` | yes | list | — | labels split into component tables | `_plan_components`, `_output_dirs` |
| `binned_suffix` | yes | string | — | binned-tree names `binned_flatTree_<stem><binned_suffix>.root` | `_bin_output` |
| `gate` | tables | string | — | selection a bin's data tree must pass to be fitted | `tables_physics_args` |
| `target` | tables | mapping | — | `z` (`[zmin, zmax]`, cm, zmin < zmax), `density` (g/cm³), `molar_mass` (g/mol), `atoms` per molecule | `tables_physics_args` |
| `branches` | bin | list | — | columns kept in the binned data and MC trees | `bin_physics_args` |
| `mass_windows` | tables | mapping | — | fit windows (GeV), all seven required ([Mass windows](#mass-windows)) | `tables_physics_args` |
| `tex` | `tex` step | mapping | — | LaTeX table: `label`, `output`, `columns`, `run_fraction` (below) | `_tex_settings`; `gxana_systematics/config.py:nominal` |
| `figures` | `figures` step | mapping | — | `output_dir`, `label`, `columns`, `plots` (below) | `_figures_settings` |
| `published_systematics` | `--systematics published` | mapping | — | `tex` and `figures` sub-blocks whose keys replace those of `xsection.tex` / `xsection.figures` | `with_systematics` |

#### Mass windows

`xsection.mass_windows` and `barlow.check.mass_windows` hold fit windows in
GeV. Each key is passed as `--mass-window <key>=<GeV>`: by `gxana run xsection`
to `gxana_xsec_tables` (all seven keys of its column required;
`gxana/xsection/Physics.h` `MassWindows`) and by `gxana run barlow --steps check`
to `gxana_barlow_trees --check` (all seven of its column required;
`gxana/barlow/VariationTrees.h` `CheckWindows`). Any other name is a usage error.

| Key | `xsection.mass_windows` (`gxana_xsec_tables`) | `barlow.check.mass_windows` (`gxana_barlow_trees --check`) |
|---|---|---|
| `lo` | lower edge of every fit variable, fit range and plot | lower edge of the MC fit variable, signal range and plot, and of the data fit range |
| `mc_hi` | upper edge of the MC fit variable | same |
| `mc_signal_hi` | upper edge of the MC signal fit range | same |
| `mc_plot_hi` | upper edge of the MC fit plot | same |
| `data_lo` | — | lower edge of the data fit variable and plot |
| `data_hi` | upper edge of the data fit variable and plot | upper edge of the data fit variable and plot |
| `data_edge` | the lower-edge scan of the data fit stops here | — |
| `scan_start` | — | the lower-edge search of the data fit starts here |
| `mcpdf_data_lo` | lower edge of the `MCPdf` data fit variable | — |

`tex`:

| Key | Req. | Type | Default | Meaning |
|---|---|---|---|---|
| `label` | yes | string | — | weighted tables `weighted_data/<label>/` to tabulate; also the systematics nominal when `systematics.nominal` is unset |
| `output` | yes | string | — | `.tex` file to write |
| `columns` | yes | mapping | — | column heading → stats file of `gxana run systematics`, or `scale_factor` (the run systematic from the table's S column) |
| `run_fraction` | no | number | 0.051 | fraction used by the run-fraction source only |

`figures`:

| Key | Req. | Type | Default | Meaning |
|---|---|---|---|---|
| `output_dir` | yes | string | — | where the macros run and write |
| `label` | yes | string | — | weighted tables the systematic bands are added to |
| `columns` | no | mapping | `tex.columns` | systematic columns summed in quadrature, in order |
| `plots` | yes | list | — | `{macro, args, requires}` entries |
| `macro` | yes | string | — | (in `plots`) macro path relative to `analyses/<channel>/` |
| `args` | no | list | macro defaults | (in `plots`) numbers, booleans or strings (no `"` or `\`); `${GXANA_*}` expanded |
| `requires` | no | list | — | (in `plots`) paths checked to exist before running |

### `qfactors.yaml`

Block `qfactors`, read by `stages/qfactors.py:plan_qfactors`.

| Key | Req. | Type | Default | Meaning |
|---|---|---|---|---|
| `engine_dir` | yes | string | — | QFactors checkout, relative to the repository root (`packages/qfactors`) |
| `model` | yes | string | — | fit model: a `configPDFs*.h` file name in `engine_dir`, or a repository-relative path (containing `/`) to the channel's own model file; `--model` overrides |
| `sample` | yes | string | — | sample whose stem names the input (`data`) |
| `variant` | yes | string | — | suffix of the input file tag (`_nominal_kphighrap`) |
| `input` | yes | string | — | input flat tree; `{stem}`, `{variant}` |
| `tree` | yes | string | — | tree name in `input` |
| `work_dir` | yes | string | — | parent of the per-period work directory `<work_dir>/<stem><variant>/` |
| `output_dir` | yes | string | — | result `<output_dir>/<tag>_<combo>/postQVal_flatTree_<tag>_<combo>.root` |
| `plots_dir` | yes | string | — | histograms and diagnostic plots |
| `settings` | yes | mapping | — | every run.py `_SET_*` value (below); all 27 required |
| `extra_settings` | no | mapping | — | further `configSettings.h` values run.py never sets (`key: value`, written unquoted) |
| `diagnostic_vars` | yes | list | — | variables written to `makePlotsVars.txt` |

`settings` (`stages/qfactors.py:SETTINGS_KEYS`; meanings in
`packages/qfactors/README.md`): `fitWeights`, `sigWeights`, `altWeights`,
`varStringBase` (`;`-separated phase-space variables; the combo tag has one
`1` per variable), `discrimVars`, `extraVars`, `neighborReqs`, `nProcess`,
`kDim`, `nentries`, `numberEventsToSavePerProcess`, `standardizationType`,
`redistributeBkgSigFits`, `nRndRepSubset`, `doKRandomNeighbors`, `nBS`,
`runTag` (must be `""`), `seedShift`, `saveBShistsAlso`,
`alwaysSaveTheseEvents`, `saveBranchOfNeighbors`, `saveMemUsage`,
`saveEventLevelProcessSpeed`, `emailWhenFinished`, `runBatch` (must be 0),
`runAllPhaseCombos` (must be 0), `extraLibs` (a list).

### `mc.yaml`

Block `mc`, read by `stages/mc.py:plan_mc`.

| Key | Req. | Type | Default | Meaning |
|---|---|---|---|---|
| `simulation_dir` | yes | string | — | directory with `gen_amp_cfg/` and `mcwrapper/` templates, relative to the repository root |
| `batch` | yes | int | — | `gluex_MC.py batch=` value |
| `periods` | yes | mapping | — | period → `{conf, events, sim_version_set}` |
| `conf` | yes | string | — | (per period) MCwrapper conf in `<simulation_dir>/mcwrapper/`; must contain exactly one `DATA_OUTPUT_BASE_DIR=`, `GENERATOR_CONFIG=`, `ENVIRONMENT_FILE=`, `WORKFLOW_NAME=` line |
| `events` | yes | int | — | (per period) events to generate |
| `sim_version_set` | yes | string | — | (per period) version set the job needs (`source env/setup.sh --sim=<set>`) |
| `samples` | yes | mapping | — | sample → `{generator_config, periods, overrides}` |
| `generator_config` | yes | string | — | (per sample) generator config in `<simulation_dir>/gen_amp_cfg/` |
| `periods` | no | list | all | (per sample) periods the sample is produced for |
| `overrides` | no | mapping | — | (per sample) further conf keys to set (`{BKG: None}`) |

### `measurements.yaml`

Block `measurements`, read by `stages/measurements.py:block`.

| Key | Req. | Type | Default | Meaning |
|---|---|---|---|---|
| `output_dir` | yes | string | — | working directory of every macro; created before the run |
| `make_dirs` | no | list | — | further directories to create (ROOT does not create a PDF's directory) |
| `items` | yes | mapping | — | item name → `{prep, fit}`; each item needs at least one |
| `prep` | no | mapping | — | (in an item) `{macro, args}` run in the `prep` step |
| `fit` | no | mapping | — | (in an item) `{macro, args}` run in the `fit` step |
| `macro` | yes | string | — | macro path relative to `analyses/<channel>/`; must exist |
| `args` | no | list | macro defaults | numbers, booleans or strings (no `"` or `\`) |

kpkpkmlamb:

```yaml
measurements:
  output_dir: "${GXANA_OUTPUT}/kpkpkmlamb/measurements"
  items:
    ximstar:      {fit: {macro: measurements/FitXimStar.C, args: [4, false]}}
    ximstar_tcut: {fit: {macro: measurements/FitXimStar.C, args: [4, true]}}
```

### `barlow.yaml`

Block `barlow`, read by `gxana_barlow/config.py:validate` and
`gxana_barlow/stage.py`. It also reads `xsection.output_dir`,
`xsection.inputs.flux_dir`, `periods.<p>.flux`, `energy_edges`, `t_bins` and
`physics`. `{stem}` is the period's data stem.

| Key | Req. | Type | Default | Meaning | Read by |
|---|---|---|---|---|---|
| `label` | yes | string | — | label of the variation tables; the nominal compared against is `xsection/weighted_data/<label>` | `plan`, `_nominal_dir` |
| `output_dir` | yes | string | — | root of all Barlow outputs | `_output_dir` |
| `mc_sample` | yes | string | — | MC sample of the variation trees and thrown binned trees | `_plan_trees`, `_tables_inputs` |
| `weight` | yes | string | — | weight branch of the fits | `_plan_tables`, `_plan_check` |
| `threshold` | yes | number | — | half-height of the shaded σ_B band | `plot_commands` |
| `fit` | yes | mapping | — | `model`, `params`, `cheby` (default 2), as in `xsection.fits` | `_plan_tables` |
| `trees` | yes | mapping | — | variation-tree inputs (below) | `_plan_trees` |
| `check` | `check` step | mapping | — | `nominal`, `nominal_mc` (nominal trees; `{stem}`, `{mc_sample}`), `mass_windows` | `_plan_check`, `check_physics_args` |
| `plot` | `plot` step | mapping | — | `t_limits`, `energy_limits`, `graph_limits`: `[lo, hi]` x ranges | `plot_physics_args` |
| `nominal` | yes | mapping | — | family → nominal cut term; every family needs one | `gxana_barlow/variations.py:expand` |
| `fixed` | no | list | — | cut terms always applied, never varied | `expand` |
| `families` | yes | mapping | — | family → `{op, values, label, style}` | `validate`, `expand`, `plot_commands` |

`trees`:

| Key | Req. | Type | Default | Meaning |
|---|---|---|---|---|
| `tree` | yes | string | — | tree name of the inputs |
| `input` | yes | string | — | data flat tree; `{stem}` |
| `input_mc` | yes | string | — | MC flat tree; `{stem}`, `{mc_sample}` |
| `output` | yes | string | — | variation file relative to `output_dir`; must contain `{stem}` and `{family}` |
| `branches` | yes | list | — | columns kept |
| `threads` | no | int | 0 | implicit multithreading (0 = off) |
| `defines` | no | mapping | — | new column → expression, data and MC |
| `filters` | no | mapping | — | `data` and `mc`: lists of filter expressions |

`check.mass_windows`: the seven keys of [Mass windows](#mass-windows), all required.

A family:

| Key | Req. | Type | Meaning |
|---|---|---|---|
| `op` | yes | `<`, `>`, `<=`, `>=` | comparison of the varied cut |
| `values` | yes | list of quoted strings | cut values; the variation id is `<family>_<value>` |
| `label` | yes | string | legend text, followed by the value |
| `style` | yes | mapping | plot layout, all ten keys required (below) |

`style`: `canvas` (`default` or `[w, h]`), `legend_diff` and `legend_tot`
(four numbers in [0, 1]), `y_floor`, `y_pad_diff`, `canvas_def_w`,
`title_offset_y`, `title_offsets_diff` and `title_offsets_tot` (`[x, y]`),
`tot_y_ndiv` (bool). See the comment in `analyses/kpkpxim/config/barlow.yaml`
for what each sets.

The `trees` step records a hash of `trees`, `nominal`, `fixed`, `mc_sample`
and each family's `op` and `values` in `<output_dir>/variations.json`; later
steps stop if those changed (`gxana_barlow/config.py:config_hash`).

### `systematics.yaml`

Block `systematics`, read by `gxana_systematics/config.py:validate` and
`gxana_systematics/stage.py`. Schema details and the plot layouts:
`packages/systematics/README.md`.

| Key | Req. | Type | Default | Meaning | Read by |
|---|---|---|---|---|---|
| `output_dir` | yes | string | — | root of all systematics outputs | `output_dir` |
| `nominal` | no | string | `xsection.tex.label` | label of the nominal tables in `xsection/weighted_data/` | `config.py:nominal` |
| `variants` | yes | list | — | fit groups `{model, params, labels}` (as `xsection.fits`) and `{qvalue: {label, source}}` entries | `fit_groups`, `qvalue_variants`, `_plan_fit` |
| `qvalue` | no | mapping | — | (in a variant) `label`: new label; `source`: fit label whose dσ/dt is scaled by `qval_yield/data_yield` | `_plan_qvalue` |
| `source` | yes | string | — | (in `qvalue`) a fit label of the pool | `validate` |
| `studies` | yes | mapping | — | study name → study (below) | `config.py:studies` |
| `runperiod` | no | mapping | step skipped | `macro`: run-period macro (relative to `analyses/<channel>/`) for `--steps runperiod` | `_plan_runperiod` |
| `summary` | no | mapping | no summary | `point_by_point` and `normalization`: lists of study names | `_plan_summary` |

Study keys by `kind` (unknown keys are an error):

| Key | Kinds | Req. | Meaning |
|---|---|---|---|
| `kind` | all | yes | `spread`, `sfactor`, `track`, `constant` or `compare` |
| `stats` | spread, sfactor, compare | spread, sfactor; compare with `per_period` | stats file name in `<output_dir>/<study>/` |
| `spread` | spread | yes | at least two labels whose spread is the systematic |
| `plots` | spread, compare | no | `gxana_syst_plot` figures (keys below) |
| `examples` | spread | no | `bin: {period, tree}` and `fits: {figure name: label}`: copies one bin's fit PDF per label |
| `bin` | spread | with `examples` | (in `examples`) `period` and binned `tree` name |
| `period` | spread | with `examples` | (in `examples.bin`) period of the example bin |
| `fits` | spread | with `examples` | (in `examples`) figure name → label |
| `per_period` | compare | no | label whose per-period tables are compared (needed by `run_grid` and `stddev_band` plots) |
| `tree`, `thrown_tree` | track | yes | flat-tree names of the inputs (`xsection.inputs`) |
| `data_weight`, `mc_weight` | track | no (default empty) | weight expressions |
| `theta_cut_deg` | track | yes | polar angle (degrees) between the `low` and `high` per-track uncertainties |
| `low`, `high` | track | yes | per-track uncertainty below / above `theta_cut_deg` |
| `override` | track | no | particle name → fixed uncertainty |
| `report` | track | yes | `data` or `mc` |
| `legend_header` | track | no | legend header text |
| `particles` | track | yes | list of `{name, p4, thrown_p4, theta, p, title}`: reconstructed and thrown p4 branches, `[n, lo, hi]` θ and p binnings, axis titles |
| `name`, `p4`, `thrown_p4`, `theta`, `p`, `title` | track | yes | (in `particles`) as above |
| `value` | constant | yes | the relative uncertainty |

Plot keys (`gxana_systematics/config.py:PLOT_KEYS`): `layout` (required:
`grid3`, `pair_band`, `all_band`, `grid2`, `run_grid`, `stddev_band`),
`name` (required; output name), `labels`, `legend` (`"text|option"`
entries), `legend_header`, `first_style`, `annotate`, `axis_format`,
`x_axis_format`, `xmax`, `ymax`.

Fixture example:

```yaml
systematics:
  output_dir: "${GXANA_OUTPUT}/kpkpkmlamb/systematics"
  nominal: nominal
  variants:
    - model: Johnson
      params: {mu: [1.823, 1.81, 1.835], lambda: [0.01, 0.005, 0.03], gamma: [0.0, -0.5, 0.5], delta: [1.0, 0.2, 1.5]}
      labels: [{label: nominal, cheby: 2}, {label: nominal_cheby1, cheby: 1}]
    - model: MCPdf
      params: {}
      labels: [{label: mcPdf, cheby: 2}]
  studies:
    run: {kind: sfactor, stats: sfactor_stats.txt}
    fit: {kind: spread, stats: fit_variations_stats.txt, spread: [nominal, nominal_cheby1, mcPdf]}
    luminosity: {kind: constant, value: 0.05}
  summary:
    point_by_point: [run, fit]
    normalization: [luminosity]
```

### `studies.yaml`

Block `studies` (study name → study), read by
`gxana_studies/config.py:validate` and `gxana_studies/stage.py`. Paths take
`${GXANA_*}`, `{period}`, `{stem}`, `{mc_stem}`; outputs are relative to
`out_dir` unless absolute. Details: `packages/studies/README.md`.

| Key | Kinds | Req. | Type | Meaning |
|---|---|---|---|---|
| `kind` | both | yes | string | `cutscan` or `datamc` |
| `out_dir` | both | yes | string | output directory |
| `threads` | both | no (0) | int | implicit multithreading of the fill step |
| `tree` | both | yes | string | tree name of the input(s) |
| `input` | cutscan | yes | string | flat tree per period |
| `steps` | cutscan; datamc per sample | yes | list | `{filter: EXPR}` or `{define: NAME, expr: EXPR}`, in order |
| `filter`, `define`, `expr` | both | — | string | (in `steps`) as above |
| `weight` | cutscan; datamc per sample | no | string | weight branch |
| `mass` | cutscan | yes | mapping | `var` and `bins` `[n, lo, hi]` of the mass histogram |
| `scan` | cutscan | yes | mapping | `var`, `bins` `[n, lo, hi]`, `first_bin` (≥ 1) of the scanned cut |
| `var`, `bins`, `first_bin` | cutscan | yes | — | (in `mass` / `scan`) as above |
| `fit` | cutscan | yes | mapping | `mass_title`, `range` `[lo, hi]`, `params` |
| `mass_title`, `range` | cutscan | yes | — | (in `fit`) axis title and fit range |
| `params` | cutscan | yes | mapping | `a0`, `a1`, `mu`, `lambda`, `gamma`, `delta`, `nbkgd`, `nxi`, each an `"init,min,max"` (or fixed) string |
| `panel_label`, `plot_title` | cutscan | yes | string | plot text |
| `cut` | cutscan | yes | number | the chosen cut, marked on the plot |
| `outputs` | cutscan | yes | mapping | `hist`, `tables` (must contain `{what}`: FOM, SB, Yield), `grid`, `plots` (list) |
| `hist`, `tables`, `grid` | cutscan | yes | string | (in `outputs`) as above |
| `hist_file` | datamc | yes | string | histogram file |
| `mc_sample` | datamc | when a path uses `{mc_stem}` | string | MC sample |
| `inputs` | datamc | yes | mapping | `data`, `mc`, `thrown` paths |
| `thrown_tree` | datamc | yes | string | tree name of the thrown input |
| `tags` | datamc | yes | mapping | period → tag; every period required |
| `samples` | datamc | yes | mapping | `data`, `mc`, `thrown` → `{steps, weight}` |
| `vars` | datamc | yes | list | `{var, title, legend}`; `legend` is `tl` or `tr` (default `tr`) |
| `truth_vars` | datamc | no | list | as `vars`, each `var` also in `vars` |
| `title`, `legend` | datamc | — | string | (in `vars`) as above |

### `analysis_data.yaml` (not in `config/`)

`analyses/<channel>/analysis_data.yaml`, read by `analysis_data.py` for
`gxana data path|status|lock|stage`. A channel without preserved data has no
such file, and the `data` commands then report an error. See
[analysis_data.md](analysis_data.md).

| Key | Req. | Type | Meaning | Read by |
|---|---|---|---|---|
| `subdir` | yes | string | directory under `$GXANA_ANALYSIS_DATA` | `load_manifest`, `data_dir` |
| `directories` | no | mapping | description of each subdirectory (documentation only, not read) | — |
| `files` | yes | mapping | relative path → `{sha256, bytes}`, written by `gxana data lock` | `status`, `lock` |
| `sha256` | — | string | (in `files`) checksum | `status`, `stage_plan` |
| `stage` | for `data stage` | mapping | `mc_sample`, `next`, `files` | `stage_plan` |
| `mc_sample` | no | string | (in `stage`) sample whose stem fills `{mc_stem}` | `stage_plan` |
| `next` | no | string | (in `stage`) command printed after staging | `gxana/cli.py:_data` |
| `files` | yes | list | (in `stage`) `{from, to, mode}`: `from` relative to `subdir`, with `{stem}`/`{mc_stem}`; `to` a path, or a directory if it ends in `/`; `mode` `copy` or `link` | `stage_plan` |
| `from`, `to`, `mode` | yes | — | (in `stage.files`) as above | `stage_plan` |

### `channel.kv` (read by C++)

`gxana config export --channel C` writes `$GXANA_OUTPUT/C/config/channel.kv`
(or `--out FILE`) from the YAML (`config.py:channel_kv`). C++ reads it
through `gxana::ChannelInfo` (`packages/common/include/gxana/common/Periods.h`):

| Line | From | C++ accessor |
|---|---|---|
| `channel=` | the `--channel` value | `Get("channel")` |
| `source_dir=` | `analyses/C/config` | checked on load |
| `source.<file>.md5=` | MD5 of every `config/*.yaml` | checked on load: a changed or missing file is a "stale channel.kv" error |
| `periods=` | `periods` keys, in order | `PeriodNames()` |
| `period.<period>.dir=`, `.title=`, `.label=` | `periods.<period>.dir`, `title`, `label` | `PeriodValue(period, key)`; `MakePeriods` uses `dir` |
| `stem.<period>.<sample>=` | `tree_stem` of every (period, sample) pair that has one | `Stem(period, sample)` |
| `mc_sample=` | `xsection.mc_sample`, if set | `Get("mc_sample")`; `MakePeriods` for `{mc_stem}` patterns |

## Gotchas

- **Always pass `--channel`.** `gxana run xsection`, `barlow`,
  `systematics`, `mc` and `qfactors` default to `--channel kpkpxim`
  (`packages/common/python/gxana/cli.py`). Without it a dry run plans the
  thesis channel and a real run writes into `$GXANA_OUTPUT/kpkpxim/`.
  (`select`, `studies`, `measurements` and the `config`/`data` commands
  require it.)
- **`qfactors.model`.** The thesis model `configPDFs.h` is a file in the
  `packages/qfactors` submodule, fitted to the Ξ⁻ lineshape. Write your own
  model file in the channel (the error message suggests
  `analyses/<channel>/config/qfactors_models/configPDFs_<name>.h`) and set
  `model:` to its repository-relative path; a bare name is looked up only in
  `packages/qfactors`.
- **`run_hdroot.py --tree-prefix`.**
  `packages/montecarlo/scripts/run_hdroot.py` defaults `--tree-prefix` to
  `kpkpxim`; pass your `reaction` when you use it.
- **Rerun `gxana config export` after every YAML edit**, not only after
  editing `periods.yaml`. `channel.kv` records the MD5 of every
  `config/*.yaml`, and `gxana::ChannelInfo` refuses a stale file.
- **Selectors need gluex_root_analysis.** `gxana run select` loads
  `$ROOT_ANALYSIS_HOME/scripts/Load_DSelector.C` and runs PROOF-Lite; it
  works only after `source env/setup.sh --gluex` (or in the container).
  `--dry-run` works without it.
- **The tree directory must be named `tree_<...>`.** The saved-file name is
  the name of the directory above `{kind}` without `tree_`; a
  `tree_dir_template` without it is an error.
- **The `data` sample.** xsection, barlow, systematics, studies and
  `gxana data stage` take the data stem from the sample named `data`.
- **Quote Barlow values** (`values: ["0.5", "1"]`): they become file names
  and legend text with their exact spelling, and an unquoted number is an
  error.
- **No duplicate top-level keys across files.** Each block (`xsection`,
  `barlow`, ...) must live in exactly one `config/*.yaml`.
- **Stage outputs are named by channel**, but input paths are whatever the
  config says. When copying a kpkpxim block, replace every `kpkpxim`,
  `decayxim`, `hybrid_combo` and `kphighrap` in it;
  `tests/test_second_channel.py` checks that none of these reaches a second
  channel's commands.
