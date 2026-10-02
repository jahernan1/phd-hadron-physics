# packages/common

- `python/gxana/` — the `gxana` command-line tool (`uv run gxana --help`). Depends
  only on the Python standard library and PyYAML; ROOT work is done by shelling
  out to `root`.
  - `doctor`, `config show`, `data path|status|lock`, `externals fetch|status`.
  - `config export --channel C [--out F]` writes `$GXANA_OUTPUT/<C>/config/channel.kv`,
    a flat `key=value` file for the C++ macros (`gxana::ChannelInfo`, `Periods.h`): the
    period list in `periods.yaml` order with each period's `dir`, `title` and `label`,
    the tree stem of every (period, sample) pair, `mc_sample`, and the MD5 of every
    `config/*.yaml`. Rerun it after any edit of the channel config: C++ refuses a
    `channel.kv` whose recorded MD5s no longer match.
  - Stages (`gxana run <stage> --channel C ... [--dry-run]`):
    - `select` — DSelector with PROOF-Lite (`stages/select.py`);
    - `xsection` — bin, fit, weight, integrate, split and tabulate the cross section
      (`stages/xsection.py`; [`packages/xsection`](../xsection/README.md));
    - `mc` — render MCwrapper inputs, submit `gluex_MC.py` (`stages/mc.py`;
      [`packages/montecarlo`](../montecarlo/README.md));
    - `qfactors` — Q-factor weights with the QFactors fork (`stages/qfactors.py`);
    - `barlow` — Barlow cut-variation check; stage in
      [`packages/barlow`](../barlow/README.md) (`gxana_barlow.stage`);
    - `systematics` — systematic studies from `systematics.yaml`; stage in
      [`packages/systematics`](../systematics/README.md) (`gxana_systematics.stage`);
    - `studies` — cut scans and data/MC studies from `studies.yaml`; stage in
      [`packages/studies`](../studies/README.md) (`gxana_studies.stage`);
    - `measurements` — the prep and fit macros of `measurements.yaml`, each run as
      `root -l -b -q rootlogon.C <macro>` from `measurements.output_dir`
      (`stages/measurements.py`; channel READMEs, e.g.
      [`analyses/kpkpxim/measurements`](../../analyses/kpkpxim/measurements/README.md)).
  - Internal helpers for anyone adding a stage: `gxana.stages.runner` (the shared stage
    runner: `Command`, `check_steps`, `run_steps`, the print / dry-run / run loop,
    `executable`, `python_module`, `root_macro`), `gxana.paths.gxana_root` (the repository
    root from `GXANA_ROOT`, else this checkout) and `gxana.bins` (bin-edge labels and
    `--energy` arguments, the Python side of `BinNames.h`). `gxana.config` loads and
    checks the channel YAML (`physics`, `periods`, `samples`, tree stems).
- `include/gxana/common/`, `src/` — `GxanaCommon` C++/ROOT library:
  `Style.h`: `SetStyle` and the plot-style presets (`ApplyStyle` with `ThesisStyle`,
  `FitStyle`, `ComparisonStyle`, `TrackStyle`, `BarlowStyle`,
  `GridTrailingTweak`, `CutStudyStyle`, `DistributionStyle`),
  `NumericCompare` (orders bin tables by the full emin value, then by name), `Paths.h`: `EnvPath` (resolves `GXANA_*`
  variables), `BinNames.h` (bin-edge labels, bin names and titles,
  `ParseBinName`), `GraphIO.h` (`ReadBinnedGraphs`: one graph per energy-bin
  table; panels ascend by emin; two tables with the same emin are an error), `AcceptanceCorrect.h` (acceptance ε = reco/thrown
  and data/ε for 1-D/2-D/3-D histograms, period merge). Loaded into ROOT by
  the repository's `rootlogon.C`.
  - `Periods.h`: `ChannelInfo` reads `channel.kv` and refuses it when a config file
    changed since the export; `PeriodValue(p, key)` returns a period's `dir`, `title`
    or `label` from `periods.yaml`, all three required by `config export` (`dir` = the
    period's directory name inside the macros' ROOT files, e.g. `Spring_2017`; `title` =
    its readable name, e.g. `Spring 2017`, exported for plot labels; no macro reads it yet).
    `Period`, `MakePeriods` (one `Period` per channel period, `dir` from `periods.yaml`),
    `GetPeriodHists` (reads `<dir>/<name>` for every period), `MergeHists`.
  - `Overlay.h`: `DrawOverlay`, the two-histogram comparison plot of
    `simulation/validation`.
  - `PeriodHists.h`: library `GxanaPeriodHists` (separate, links RDataFrame; dictionary
    from `LinkDefPeriodHists.h`, `GxanaCommon`'s from `LinkDef.h`): `FillHists` /
    `FillPeriodHists`, the per-period RDataFrame histogram fill of the macros. A 1-D
    `HistDef` with no axes uses RDataFrame's default model (128 bins, automatic range);
    one with `like` takes the binning, name and title of the histogram already written
    under that key in the same directory (`TH1DModel(*h)`).
  - `Cli.h` (header-only): the command-line parsers shared by the gxana executables.
- `tests/` — pytest (`tests/python`) and ctest (`tests/cpp`) tests. `common.style`
  (`tests/cpp/test_style.cxx`) checks that every style preset, including the barlow,
  systematics and studies styles, leaves `gStyle` exactly as its legacy body did; with
  `GXANA_STYLE_DUMP_DIR` set it writes each compared pair there as JSON.
