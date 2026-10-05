# tests

Repository-level pytest tests; each package keeps its own under
`packages/<name>/tests/` (all listed in `testpaths` of `pyproject.toml`).

- `test_*.py` here — repository checks: README commands, docs layout, release files,
  no legacy paths, no channel literals in package code, public hygiene
  (`test_public_hygiene.py`: no laptop home paths, no other users' home
  directories, no citations of planning notes outside `docs/history/REFACTOR_SPEC.md`),
  the second channel (`fixtures/channels/`), `scripts/migrate_paths.py`.
- `golden/` — golden tests on the preserved thesis data, marker `golden`; listed in
  [`docs/analysis_data.md`](../docs/analysis_data.md#golden-tests).
- `macros/` — every macro loads under cling (marker `macros`) and the macro style
  harness `test_macro_styles.py` (`GXANA_STYLE_DUMP_DIR` keeps the dumps).
- `env/` (`env/setup.sh`), `qfactors/` (the QFactors fork), `selection/`,
  `kpkpkmlamb/`, `stage_plans/` (stage plans against a recorded baseline,
  `stage_plans/record.py`).

Run:

    uv run pytest                 # everything: ~4 min without preserved data, ~22 min with it (golden tests included)
    uv run pytest -m "not golden" # all but the golden tests
    uv run pytest -m golden -v    # golden tests only

Type check: `uv run pyright` (from the repository root; the canonical form) reads `[tool.pyright]` in `pyproject.toml`
(the `.venv` interpreter, every `packages/*/python` directory on the import path, the
`packages/qfactors` submodule excluded) and reports 0 errors on `packages`, `tests`, `scripts`
and `env`. Install pyright separately (`npm i -g pyright` or `pip install pyright`). Run it through `uv run`: a bare
`pyright` can pick the system Python (3.9) instead of `.venv`'s, whose older numpy stubs hide errors
the project interpreter reports; `pythonVersion` is not pinned because `requires-python` is `>=3.9`.

The root `conftest.py` stops the session (exit code 4) when `GXANA_ROOT` or the imported
`gxana` belong to another checkout, i.e. the shell sourced another checkout's
`env/setup.sh`; fix it by sourcing this checkout's `env/setup.sh` or using a clean shell.

Golden tests skip without the preserved data under `$GXANA_ANALYSIS_DATA`
(default `$GXANA_ROOT/gluex_analysis_data`); tests that need ROOT or the built apps
skip without them (`-rs` lists the skips). Network tests run only with
`GXANA_NETWORK_TESTS=1` ([`docs/environment.md`](../docs/environment.md)).

Test directories have no `__init__.py`, so a test module's basename must be unique
across `tests/`, `packages/*/tests/python` and `packages/montecarlo/tests`.
