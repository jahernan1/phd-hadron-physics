# Environment

Two environments (docs/REFACTOR_SPEC.md §7):

- **analysis** — halld version set 5.12.0 (ROOT 6.24.04, gluex_root_analysis 1.25.0):
  selectors, selection, Q-factors, cross sections, systematics.
- **sim** — per-run-period recon version sets with patched halld_sim (added in Plan 4).

## Variables

| Variable | Meaning | Default from `env/setup.sh` |
|---|---|---|
| `GXANA_ROOT` | repository checkout | directory containing `env/` |
| `GXANA_DATA` | input trees (`Trees/`, `flatTrees/`, `flux/`) | `$GXANA_ROOT/_data` (gitignored) |
| `GXANA_OUTPUT` | stage outputs | `$GXANA_ROOT/_output` |
| `GXANA_SCRATCH` | PROOF-Lite sandboxes, run dirs | `${TMPDIR:-/tmp}/gxana-$USER` |
| `GXANA_EXTERNALS` | fetched + patched upstream builds | `$GXANA_ROOT/_externals` |
| `GXANA_ANALYSIS_DATA` | preserved analysis data: golden inputs + reference outputs ([analysis_data.md](analysis_data.md)) | `$GXANA_ROOT/gluex_analysis_data` |

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
