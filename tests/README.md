# tests

Repository-level pytest tests; each package keeps its own under
`packages/<name>/tests/` (all listed in `testpaths` of `pyproject.toml`).

- `test_*.py` here — repository checks: README commands, docs layout, release files,
  no legacy paths, no channel literals in package code, the second channel
  (`fixtures/channels/`), `scripts/migrate_paths.py`.
- `golden/` — golden tests on the preserved thesis data, marker `golden`; listed in
  [`docs/analysis_data.md`](../docs/analysis_data.md#golden-tests).
- `macros/` — every macro loads under cling (marker `macros`) and the macro style
  harness `test_macro_styles.py` (`GXANA_STYLE_DUMP_DIR` keeps the dumps).
- `env/` (`env/setup.sh`), `qfactors/` (the QFactors fork), `selection/`,
  `kpkpkmlamb/`, `stage_plans/` (stage plans against a recorded baseline,
  `stage_plans/record.py`).

Run:

    uv run pytest                 # everything: ~4 min without preserved data, ~15 min more for the golden tests with it
    uv run pytest -m "not golden" # all but the golden tests
    uv run pytest -m golden -v    # golden tests only

Golden tests skip without the preserved data under `$GXANA_ANALYSIS_DATA`
(default `$GXANA_ROOT/gluex_analysis_data`); tests that need ROOT or the built apps
skip without them (`-rs` lists the skips). Network tests run only with
`GXANA_NETWORK_TESTS=1` ([`docs/environment.md`](../docs/environment.md)).

Test directories have no `__init__.py`, so a test module's basename must be unique
across `tests/`, `packages/*/tests/python` and `packages/montecarlo/tests`.
