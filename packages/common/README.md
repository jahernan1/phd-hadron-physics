# packages/common

- `python/gxana/` — the `gxana` command-line tool (`uv run gxana --help`). Depends
  only on the Python standard library and PyYAML; ROOT work is done by shelling
  out to `root`.
  - Commands and stages: every `gxana` command, stage and flag is in the
    [`gxana` command reference](../../README.md#gxana-command-reference). Stage code is
    `gxana/stages/<stage>.py` (`select`, `xsection`, `mc`, `qfactors`, `measurements`) or
    the owning package's `stage` module (`gxana_barlow`, `gxana_systematics`,
    `gxana_studies`).
  - `config export --channel C [--out F]` writes `$GXANA_OUTPUT/<C>/config/channel.kv`,
    a flat `key=value` file for the C++ macros (`gxana::ChannelInfo`, `Periods.h`): the
    period list in `periods.yaml` order with each period's `dir`, `title` and `label`,
    the tree stem of every (period, sample) pair, `mc_sample`, and the MD5 of every
    `config/*.yaml`. Rerun it after any edit of the channel config: C++ refuses a
    `channel.kv` whose recorded MD5s no longer match.
  - Internal helpers for anyone adding a stage: `gxana.stages.runner` (the shared stage
    runner: `Command`, `check_steps`, `run_steps`, the print / dry-run / run loop,
    `executable`, `python_module`, `root_macro`), `gxana.paths.gxana_root` (the repository
    root from `GXANA_ROOT`, else this checkout), `gxana.paths.foreign_checkout` (reasons
    the environment points at another checkout; used by the root `conftest.py`) and `gxana.bins` (bin-edge labels and
    `--energy` arguments, the Python side of `BinNames.h`). `gxana.config` loads and
    checks the channel YAML (`physics`, `periods`, `samples`, tree stems).
- `include/gxana/common/`, `src/` — `GxanaCommon` C++/ROOT library:
  `Style.h` (plot style, below),
  `Strings.h`: `NumericCompare` (orders bin tables by the full emin value, then by name), `Paths.h`: `EnvPath` (resolves `GXANA_*`
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
  - `Cli.h` (header-only): argument-parsing helpers in `gxana::cli` (`Split`,
    `ParseDouble`, `ParseDoubleList`, `ParseParam`, `SplitAssign`) shared by the gxana
    executables.
- `tests/` — pytest (`tests/python`) and ctest (`tests/cpp`) tests.

## Plot style

`Style.h`: `SetStyle` (the thesis style) and the presets applied with `ApplyStyle`:
`ThesisStyle`, `FitStyle`, `ComparisonStyle`, `TrackStyle`, `BarlowStyle`,
`GridTrailingTweak`, `CutStudyStyle`, `DistributionStyle`. A preset is a `StyleParams`:
one field per `TStyle` setter, and fields left unset keep `gStyle`'s current value.
The package styles that also go through `ApplyStyle` are `gxana::barlow::SetBarlowStyle`,
`gxana::systematics::StyleFormat`, `gxana::studies::ApplyCutScanStyle` and
`gxana::studies::ApplyDataMCStyle`.

Checks: the ctest `common.style` (`tests/cpp/test_style.cxx`) runs every preset and
package style from ROOT's default style and from the state each legacy style leaves, and
requires the same `gStyle` as the verbatim legacy body (full JSON dump and
`gROOT->GetForceStyle()` compared as strings); `tests/macros/test_macro_styles.py` does the
same for each analysis macro's local style function. With `GXANA_STYLE_DUMP_DIR` set, both
keep every dump there as JSON.
