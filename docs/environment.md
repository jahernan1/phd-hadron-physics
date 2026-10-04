# Environment

Two environments (docs/REFACTOR_SPEC.md §7):

- **analysis** — halld version set 5.12.0 (ROOT 6.24.04, gluex_root_analysis 1.25.0):
  selectors, selection, Q-factors, cross sections, systematics.
- **sim** — per-run-period recon version sets with patched halld_sim.

Local checks in this repository ran with ROOT 6.40.04 (Homebrew, macOS), not
in the analysis container. Fits there use the ROOT 6.24 minimiser and RooFit
evaluation backend through `gxana::fit::UseThesisMinimizer`
(`packages/fit/README.md`); see `docs/KNOWN_ISSUES.md` for what that leaves.

## Variables

| Variable | Meaning | Default from `env/setup.sh` |
|---|---|---|
| `GXANA_ROOT` | repository checkout | directory containing `env/` (always set; Python falls back to this checkout when unset) |
| `GXANA_DATA` | input trees (`Trees/`, `flatTrees/`; the photon flux is read from `$GXANA_ANALYSIS_DATA/kpkpxim/flux`) | `$GXANA_ROOT/_data` (gitignored) |
| `GXANA_OUTPUT` | stage outputs | `$GXANA_ROOT/_output` |
| `GXANA_SCRATCH` | PROOF-Lite sandboxes, run dirs, Q-factor work dirs | `${TMPDIR:-/tmp}/gxana-$USER` |
| `GXANA_EXTERNALS` | fetched + patched upstream builds, rendered version sets | `$GXANA_ROOT/_externals` |
| `GXANA_ANALYSIS_DATA` | preserved analysis data: golden inputs + reference outputs ([analysis_data.md](analysis_data.md)) | `$GXANA_ROOT/gluex_analysis_data`; `${GXANA_ANALYSIS_DATA}` in channel config falls back to it when unset, every other unset `GXANA_*` stays an error |
| `GXANA_SIM_VERSION_SET` | active MC version set; `gxana run mc` refuses a period whose `sim_version_set` differs | set by `--sim=<set>`, unset by `--gluex`, else untouched |
| `GXANA_GLUEX_BOOT` | GlueX boot script sourced by `--gluex` / `--sim` (the tests point it at a stub) | `/group/halld/Software/build_scripts/gluex_env_boot_jlab.sh` |
| `GXANA_CONTAINER` | marker, `1` inside `gxana.sif` (`env/apptainer/gxana.def`) | not set by `setup.sh` |

`gxana doctor` requires `GXANA_ROOT`, `GXANA_DATA`, `GXANA_OUTPUT` and `GXANA_SCRATCH`.

Test and debug switches (unset by default; none is set by `env/setup.sh`):

| Variable | Effect |
|---|---|
| `GXANA_FIT_TRACE` | non-empty: `gxana::fit` prints `FACTORY` and `FITRESULT trace` lines ([`packages/fit`](../packages/fit/README.md)) |
| `GXANA_GOLDEN_RTOL` | relative tolerance of the deterministic golden columns (default `1e-5`) |
| `GXANA_GOLDEN_FIT_RTOL` | relative tolerance of the fit-dependent golden columns (default per label / column in `test_xsec_golden.py`, `test_systematics_chain_golden.py`; `1e-5` in the ROOT 6.24 container) |
| `GXANA_GOLDEN_SYST_OUTPUT` | `GXANA_OUTPUT` of a finished systematics run: `test_systematics_chain_golden.py` checks it instead of rerunning the fits |
| `GXANA_GOLDEN_QFACTORS_MODEL` | `configPDFs` model for `test_qfactors_golden.py` instead of `qfactors.model` |
| `GXANA_NETWORK_TESTS` | `1`: run the opt-in `network` tests that clone public upstream repositories |
| `GXANA_STYLE_DUMP_DIR` | directory where the style tests (`tests/macros/test_macro_styles.py`, ctest `common.style`) keep every `gStyle` JSON dump |

The golden tests are listed in [analysis_data.md](analysis_data.md#golden-tests).

Put site values in `env/site.sh` (copy `env/site.example.sh`; gitignored).

The default `GXANA_DATA=$GXANA_ROOT/_data` is an empty, gitignored scratch
area (never the read-only legacy `_workdir/`). For real runs point
`GXANA_DATA`/`GXANA_OUTPUT` at the site's tree area via `env/site.sh`.

## Laptop (macOS/Linux with ROOT)

```bash
uv sync && source env/setup.sh
uv run cmake -S . -B build -DCMAKE_PREFIX_PATH="$(root-config --prefix)" && uv run cmake --build build -j
uv run gxana doctor        # ROOT_ANALYSIS_HOME and gxenv warnings are expected without GlueX software
```

ROOT macros that use GxanaCommon must `#include "gxana/common/<Header>.h"`
explicitly (e.g. `#include "gxana/common/Style.h"`); ROOT's rootmap-based
autoparsing does not pick up free functions, only classes.

### One checkout per shell

`env/setup.sh` exports checkout-specific values: `GXANA_ROOT`, the default
data/output/externals/analysis-data directories and the package directories
on `PYTHONPATH` (needed in the container, which installs no `gxana`). A
shell that sourced checkout A therefore runs A's `gxana` in checkout B, even
under `uv run`. Source B's `env/setup.sh` (it replaces A's entries and the
defaults derived from A; values you set yourself are kept) or use a clean
shell; pytest refuses to start when `GXANA_ROOT` or the imported `gxana`
belong to another checkout. A script that sources `env/setup.sh` should run
`set --` first: a sourced file sees the script's own arguments.

## JLab ifarm / FSU grid (container)

```bash
apptainer build gxana.sif env/apptainer/gxana.def            # once
export GXANA_DATA=/path/to/data                            # or `source env/site.sh`; setup.sh
                                                             # hasn't run yet, so $GXANA_DATA
                                                             # must already be set here
apptainer shell --bind /group,$GXANA_DATA gxana.sif           # at FSU also bind the data disk
source env/setup.sh --gluex                                # gluex_env_boot + gxenv version_5.12.0.xml
cmake -S . -B build && cmake --build build -j
gxana doctor
gxana run select --channel kpkpxim --period 2018-08 --sample data --dry-run
```

`/group/halld` must be visible inside the container (native at JLab, CVMFS
`/cvmfs/oasis.opensciencegrid.org/gluex/group` bound to `/group` elsewhere).

## Simulation environment (MC production)

The thesis MC chain (gen_amp_V2 → hdgeant4 → mcsmear → hd_root, driven by
MCwrapper) runs in per-run-period recon version sets, not in the analysis
set. These sets ship ROOT 6.08.06 (the analysis environment is halld 5.12.0,
ROOT 6.24.04). Templates live in `env/version_sets/*.xml.in`; their
`halld_sim` and `gluex_MCwrapper` entries point at the patched checkouts
under `$GXANA_EXTERNALS` (`packages/montecarlo`).

| Version set | Used for |
|---|---|
| `recon-2019_11-ver01_13` | 2017-01 thesis MC (as run) |
| `recon-2018_01-ver02_32` | 2018-01 |
| `recon-2018_08-ver02_31` | 2018-08 |
| `recon-2017_01-ver03_40` | pre-thesis 2017-01 (ana45) production |

The period → set mapping lives in `analyses/kpkpxim/config/mc.yaml`.

Inside the GlueX container, once per version set:

```bash
packages/montecarlo/scripts/build_halld_sim.sh recon-2018_08-ver02_31   # fetch + scons build
gxana externals fetch gluex_MCwrapper                                  # once for all sets
```

Then, for each production shell:

```bash
source env/setup.sh --sim=recon-2018_08-ver02_31
```

This renders the template to `$GXANA_EXTERNALS/version_sets/<set>.xml`. The
file sits on shared disk because batch jobs read it. It then runs `gxenv` on
that file and exports `GXANA_SIM_VERSION_SET`. `--sim` and `--gluex` are
exclusive.
