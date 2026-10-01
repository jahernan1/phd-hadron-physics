# packages/common

- `python/gxana/` — the `gxana` command-line tool: `doctor`, `config show`,
  `config export` (writes `$GXANA_OUTPUT/<channel>/config/channel.kv`, the period list,
  directory names and tree stems the C++ macros read through `gxana::ChannelInfo`),
  `data path|status|lock`, `externals fetch|status`, and the stages
  `run select|xsection|mc|qfactors` (`uv run gxana --help`). Depends only on
  the Python standard library and PyYAML; ROOT work is done by shelling out
  to `root`.
- `include/gxana/common/`, `src/` — `GxanaCommon` C++/ROOT library:
  `SetStyle` and the plot-style presets (`ApplyStyle` with `ThesisStyle`,
  `FitStyle`, `ComparisonStyle`, `TrackStyle`, `BarlowStyle`,
  `GridTrailingTweak`, `CutStudyStyle`, `DistributionStyle`),
  `NumericCompare`, `EnvPath` (resolves `GXANA_*`
  variables), `BinNames` (bin-edge labels, bin names and titles,
  `ParseBinName`), `GraphIO` (`ReadBinnedGraphs`: one graph per energy-bin
  table), `AcceptanceCorrect` (acceptance ε = reco/thrown
  and data/ε for 1-D/2-D/3-D histograms, period merge). Loaded into ROOT by
  the repository's `rootlogon.C`. `Periods` (`ChannelInfo` reads `channel.kv` and refuses
  it when a config file changed since the export; `Period`, `MakePeriods`, `GetPeriodHists`,
  `MergeHists`) and `Overlay` (`DrawOverlay`, the two-histogram comparison plot of
  `simulation/validation`). `GxanaPeriodHists` (separate library, links RDataFrame):
  `FillHists` / `FillPeriodHists`, the per-period RDataFrame histogram fill of the macros. `gxana/common/Cli.h` (header-only): the
  command-line parsers shared by the gxana executables.
- `tests/` — pytest (`tests/python`) and ctest (`tests/cpp`) tests.
