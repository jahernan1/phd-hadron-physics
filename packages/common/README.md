# packages/common

- `python/gxana/` — the `gxana` command-line tool: `doctor`, `config show`,
  `data path|status|lock`, `externals fetch|status`, and the stages
  `run select|xsection|mc|qfactors` (`uv run gxana --help`). Depends only on
  the Python standard library and PyYAML; ROOT work is done by shelling out
  to `root`.
- `include/gxana/common/`, `src/` — `GxanaCommon` C++/ROOT library:
  `SetStyle` and the plot-style presets (`ApplyStyle` with `ThesisStyle`,
  `FitStyle`, `ComparisonStyle`, `TrackStyle`, `BarlowStyle`,
  `GridTrailingTweak`), `NumericCompare`, `EnvPath` (resolves `GXANA_*`
  variables), `GraphIO`, `AcceptanceCorrect` (acceptance ε = reco/thrown and
  data/ε for 1-D/2-D/3-D histograms, period merge). Loaded into ROOT by the
  repository's `rootlogon.C`.
- `tests/` — pytest (`tests/python`) and ctest (`tests/cpp`) tests.
