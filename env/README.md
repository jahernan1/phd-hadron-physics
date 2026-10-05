# env

The shell environment: `GXANA_*` variables, library and Python paths, and the
GlueX analysis and simulation environments. Every variable, its default and
the site setups: [`docs/environment.md`](../docs/environment.md).

- `setup.sh` — `source env/setup.sh [--gluex | --sim=<set>]` from bash or zsh.
  Without options it sets the `GXANA_*` defaults, `LD_LIBRARY_PATH` /
  `DYLD_LIBRARY_PATH` (`build/lib`) and `PYTHONPATH` (the package `python/` dirs).
  `--gluex` boots the GlueX analysis environment (halld version set 5.12.0);
  `--sim=<set>` boots the MC environment of `version_sets/<set>.xml.in` and exports
  `GXANA_SIM_VERSION_SET`. The two are exclusive and need the JLab boot script
  under `/group/halld` (or `GXANA_GLUEX_BOOT`). Use one checkout per shell
  ([`docs/environment.md`](../docs/environment.md#one-checkout-per-shell)).
- `site.example.sh` — template for `site.sh` (gitignored), the site values
  `setup.sh` sources before applying defaults.
- `version_sets/` — MC version-set templates; `${GXANA_EXTERNALS}` is filled in
  when `--sim` renders them to `$GXANA_EXTERNALS/version_sets/<set>.xml`
  ([`packages/montecarlo`](../packages/montecarlo/README.md#version-sets)).
- `apptainer/gxana.def` — the GlueX analysis container
  (`apptainer build gxana.sif env/apptainer/gxana.def`); it sets `GXANA_CONTAINER=1`.

Tested by `tests/env/test_setup_sh.py`.
