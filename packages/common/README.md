# packages/common

- `python/gxana/` — the `gxana` command-line tool: `doctor`, `config show`,
  `data path|status|lock`, `externals fetch|status`, and the stages
  `run select|xsection|mc|qfactors` (`uv run gxana --help`). Depends only on
  the Python standard library and PyYAML; ROOT work is done by shelling out
  to `root`.
- `include/gxana/common/`, `src/` — `GxanaCommon` C++/ROOT library:
  `SetStyle`, `NumericCompare`, `EnvPath` (resolves `GXANA_*` variables),
  `GraphIO`. Loaded into ROOT by the repository's `rootlogon.C`.
- `tests/` — pytest (`tests/python`) and ctest (`tests/cpp`) tests.
