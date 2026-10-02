# phd-hadron-physics

Analysis code for a PhD cross-section measurement and analysis of Ξ⁻(1320)
photoproduction at the GlueX experiment (Hall D, Jefferson Lab):

```
γ p → K⁺ K⁺ Ξ⁻,   Ξ⁻ → π⁻ Λ,   Λ → p π⁻
```

Pipeline: DSelector event selection → flat trees → cut optimisation →
Q-factor signal weighting → yield fits → acceptance & flux correction →
differential/total cross sections → systematic studies; plus Ξ(1320) mass and
spin measurements, Monte-Carlo generation (gen_amp + halld_sim), and a side
channel (K⁺K⁺K⁻Λ, excited Ξ*) used to validate the framework on rare signals.

> **Code only.** The GlueX data it analyses are not public; the author's
> preserved copy lives outside git under `$GXANA_ANALYSIS_DATA`, following
> the GlueX `/work/halld/gluex_analysis_data` convention
> ([`docs/analysis_data.md`](docs/analysis_data.md)). Tests that need it skip
> without it.

The repository serves two uses:

- **Reproduce the thesis.** Rerun the kpkpxim chain with the shipped
  configuration, or check it against the preserved data with the golden
  tests ([Reproducing the thesis](#reproducing-the-thesis)).
- **Reuse the framework.** Point the same `gxana` stages and packages at a new
  channel through its own `analyses/<channel>/config/` files
  ([Reusing the framework](#reusing-the-framework)).

## Layout

| Path | Contents |
|---|---|
| `packages/common` | `gxana` Python CLI + `GxanaCommon` C++/ROOT library (style, acceptance, bin names, graph I/O, `channel.kv` reader, run periods) + `GxanaPeriodHists` (RDataFrame period fills); [README](packages/common/README.md) |
| `packages/xsection` | `GxanaXsec` cross-section library + `gxana_xsection` Python helpers (`gxana run xsection`); [README](packages/xsection/README.md) |
| `packages/barlow` | `GxanaBarlow` Barlow cut-variation check (`gxana run barlow`) + `gxana_barlow` Python helpers; [README](packages/barlow/README.md) |
| `packages/systematics` | `GxanaSystematics` systematic studies (`gxana run systematics`): fit, accidentals, run-period, track efficiency; `gxana_systematics` Python helpers; [README](packages/systematics/README.md) |
| `packages/fit` | `GxanaFit` RooFit lineshape helpers for the analysis macros: factory-statement builders, fit call, weighted tree import, Johnson moments; [README](packages/fit/README.md) |
| `packages/studies` | `GxanaStudies` analysis studies (`gxana run studies`): cut scan with figure of merit; data/MC kinematic comparison; `gxana_studies` Python helpers; [README](packages/studies/README.md) |
| `packages/montecarlo` | pinned upstream MC generators + patches (`gxana externals`, `gxana run mc`); [README](packages/montecarlo/README.md) |
| `packages/qfactors` | git submodule: the QFactors fork as run for the thesis (`gxana run qfactors`); how gxana runs it: [`signal_extraction/qfactors/README.md`](analyses/kpkpxim/signal_extraction/qfactors/README.md) |
| `analyses/kpkpxim` | main thesis channel: selectors, selection, cross section, systematics, measurements, backgrounds; pipeline in [`analyses/kpkpxim/README.md`](analyses/kpkpxim/README.md) |
| `analyses/kpkpkmlamb` | side channel: excited Ξ* → K⁻Λ; [`analyses/kpkpkmlamb/README.md`](analyses/kpkpkmlamb/README.md) |
| `archive/` | superseded legacy code kept verbatim for provenance ([`archive/README.md`](archive/README.md)) |
| `tests/` | repository-level tests: golden reproductions, second-channel and no-channel-literal checks, README command checks ([`tests/README.md`](tests/README.md)) |
| `scripts/` | migration helpers (`migrate_paths.py`, `archive_copy.sh`); [`scripts/README.md`](scripts/README.md) |
| `env/` | environment setup (`setup.sh`), version sets and container definition; [`env/README.md`](env/README.md) |
| `docs/` | environment guide, preserved-data and golden-test guide, known issues, refactor spec |

## Quickstart (laptop, ROOT ≥ 6.20 installed)

```bash
git clone --recurse-submodules https://github.com/jahernan1/phd-hadron-physics.git && cd phd-hadron-physics
uv sync                                   # python toolkit + dev tools
source env/setup.sh                       # GXANA_* variables
uv run cmake -S . -B build -DCMAKE_PREFIX_PATH="$(root-config --prefix)"
uv run cmake --build build -j && uv run ctest --test-dir build
uv run pytest
uv run gxana data status --channel kpkpxim   # preserved data present? (golden tests skip otherwise)
uv run gxana doctor
uv run pytest -m golden                   # reproduce the thesis tables from preserved data (~15 min including the ~5-7 min systematics chain golden, `tests/golden/test_systematics_chain_golden.py`; reuse a finished run's output via GXANA_GOLDEN_SYST_OUTPUT)
```

On the JLab ifarm or the FSU grid see [`docs/environment.md`](docs/environment.md).

## Environment

`source env/setup.sh [--gluex | --sim=<set>]` sets the variables below
(defaults shown); export your own value first to override one. `--gluex`
boots the GlueX analysis environment (needed by `gxana run select`);
`--sim=<set>` boots the MC environment of `env/version_sets/<set>.xml.in`
(needed by `gxana run mc`). Full list, including test and debug variables:
[`docs/environment.md`](docs/environment.md).

| Variable | Default | Holds |
|---|---|---|
| `GXANA_ROOT` | the checkout | code |
| `GXANA_DATA` | `$GXANA_ROOT/_data` | skims, raw and prepared flat trees |
| `GXANA_OUTPUT` | `$GXANA_ROOT/_output` | every stage's output, under `<channel>/` |
| `GXANA_SCRATCH` | `${TMPDIR:-/tmp}/gxana-$USER` | stage work directories |
| `GXANA_EXTERNALS` | `$GXANA_ROOT/_externals` | `gxana externals fetch` checkouts |
| `GXANA_ANALYSIS_DATA` | `$GXANA_ROOT/gluex_analysis_data` | preserved inputs and reference outputs for golden tests |

## `gxana` command reference

Every stage takes `--channel <name>` (the folder name under `analyses/`);
`xsection`, `barlow`, `systematics`, `mc` and `qfactors` default to
`--channel kpkpxim`, so always pass it for another channel. Every stage
reads that channel's `analyses/<channel>/config/*.yaml`, and accepts
`--dry-run` to print its plan without running anything. `--steps a,b` runs a
subset of a stage's steps in pipeline order; steps marked opt-in run only when
named. Each stage writes under `$GXANA_OUTPUT/<channel>/`. `uv run gxana
<command> --help` prints the same options.

### Pipeline stages: `gxana run <stage>`

| Stage | Arguments | Config | Needs first | Details |
|---|---|---|---|---|
| `select` | `--channel C --period P` `[--sample S] [--thrown] [--tag T] [--cores N] [--selector path.C]` | `channel.yaml`, `periods.yaml`, `samples.yaml` | skims in `$GXANA_DATA`; `setup.sh --gluex` | [selectors](analyses/kpkpxim/selectors/README.md) |
| `mc` | `[--channel C]` `--period P --sample S` | `mc.yaml` | `setup.sh --sim=<set>`, patched halld_sim | [simulation](analyses/kpkpxim/simulation/README.md) |
| `qfactors` | `[--channel C]` `--period P` `[--model F] [--steps prepare,fit,plots]` (default `fit,plots`) | `qfactors.yaml` | `flatTreePrep.C` output | [qfactors](analyses/kpkpxim/signal_extraction/qfactors/README.md) |
| `xsection` | `[--channel C]` `[--steps bin,tables,weight,integrate,components,tex]` (`tex` opt-in) | `xsection.yaml`, `binning.yaml`, `channel.yaml` `physics:` | `qfactors`, MC and thrown flat trees | [xsection](packages/xsection/README.md) |
| `barlow` | `[--channel C]` `[--steps trees,check,bin,tables,weight,plot]` (`check` opt-in) | `barlow.yaml` | raw flat trees; `xsection` for `plot` | [barlow](packages/barlow/README.md) |
| `systematics` | `[--channel C]` `[--steps fit,qvalue,weight,spread,track,runperiod,compare,summary]` (`runperiod`, `compare` opt-in) `[--study a,b]` | `systematics.yaml` | `xsection --steps bin,tables,weight` | [systematics](packages/systematics/README.md) |
| `studies` | `--channel C` `[--steps fill,fit,plot] [--study a,b]` | `studies.yaml` | raw flat trees (cut scans); `qfactors` + MC flat trees (data/MC) | [studies](packages/studies/README.md) |
| `measurements` | `--channel C` `[--steps prep,fit] [--item a,b]` | `measurements.yaml` | `gxana config export`; selected / Q-factor flat trees | [measurements](analyses/kpkpxim/measurements/README.md) |

### Support commands

| Command | Does |
|---|---|
| `gxana doctor` | checks the `GXANA_*` variables, ROOT tools (`root`, `rootls`, `hadd`, `cmake`), preserved data and gluex_root_analysis |
| `gxana config show --channel C` | prints the merged channel YAML |
| `gxana config export --channel C [--out F]` | writes `$GXANA_OUTPUT/<channel>/config/channel.kv`, the flat key/value file the C++ macros read (run periods, stems, titles); rerun after editing the YAML — macros refuse a stale copy |
| `gxana data path --channel C` | prints the channel's preserved-data directory |
| `gxana data status --channel C` | compares files on disk with `analyses/<channel>/analysis_data.yaml` |
| `gxana data lock --channel C` | records sha256 and size of every manifest file |
| `gxana externals fetch [names] [--dest D]` | clones the pinned upstream MC sources at the locked sha and applies the patches |
| `gxana externals status [names] [--dest D]` | compares checkouts with `packages/montecarlo/external.lock` |

## Reproducing the thesis

Two routes, from cheapest to most complete.

**1. Check against the preserved data (laptop).** With `$GXANA_ANALYSIS_DATA`
present, the golden tests rerun each stage on preserved inputs and compare
with the preserved thesis outputs: tables byte for byte, fits within the
stated tolerances. The list of golden tests, what each reproduces and the
tolerance variables is in [`docs/analysis_data.md`](docs/analysis_data.md).

```bash
uv run gxana data status --channel kpkpxim
uv run pytest -m golden
uv run pytest tests/golden/test_binning_golden.py   # one stage at a time
```

**2. Rerun the chain (JLab ifarm for selection and MC, any machine after).**
The shipped kpkpxim configuration is the one the dissertation used; the
published cross-section tables are the label `johnson`. Step-by-step commands,
inputs and output paths: [`analyses/kpkpxim/README.md`](analyses/kpkpxim/README.md#pipeline).
The shipped studies and measurements configs run single-threaded (`threads: 0`,
`args: [0]`), as the golden tests do; implicit multithreading changes the mass
fit and the kinematics binning.
In outline:

```bash
gxana run select   --channel kpkpxim --period P --sample S [--thrown]
root -l -b -q rootlogon.C 'analyses/kpkpxim/selection/flatTreePrep.C("flatTree_<stem>")'
gxana run qfactors --channel kpkpxim --period P
gxana run xsection --channel kpkpxim                          # bin,tables,weight,integrate,components
gxana run systematics --channel kpkpxim                       # fit,qvalue,weight,spread,track,summary
gxana run barlow   --channel kpkpxim
gxana run xsection --channel kpkpxim --steps tex              # dissertation LaTeX tables, last
gxana config export --channel kpkpxim
gxana run measurements --channel kpkpxim                      # mass, lifetime, spin
gxana run studies  --channel kpkpxim                          # chapter-4 cut scans, chapter-5 data/MC
```

Legacy behaviours kept on purpose, and differences the port found but did not
fix, are listed in [`docs/KNOWN_ISSUES.md`](docs/KNOWN_ISSUES.md).

## Reusing the framework

The packages carry no channel names: everything channel-specific comes from
`analyses/<channel>/config/`. `analyses/kpkpkmlamb` is the working second
channel (selection + measurements only).
`tests/test_second_channel.py` drives `gxana run xsection|barlow|systematics`
from a test copy of its config with synthetic MC
(`tests/fixtures/channels/`), and `tests/test_no_channel_literals.py` keeps
channel names out of package code. To add a channel:

1. Create `analyses/<channel>/` with `config/` and a `README.md` (the tests
   require both). Start from kpkpxim's config files and keep only the stages
   you need:
   - `channel.yaml` — selector names, output basename and the `physics:`
     block (flat-tree names, fit observable, Q-factor branch or `null`,
     branching ratio, reaction title) read by the xsection, barlow and
     systematics apps;
   - `periods.yaml`, `samples.yaml` — run periods (with `dir` and `title`)
     and data/MC samples;
   - one file per stage you run: `qfactors.yaml`, `xsection.yaml` +
     `binning.yaml`, `barlow.yaml`, `systematics.yaml`, `studies.yaml`,
     `measurements.yaml`, `mc.yaml`.
2. Put the selector in `analyses/<channel>/selectors/` and run
   `gxana run select --channel <channel> ...`.
3. Run `gxana config show --channel <channel>` to check the merged config and
   `gxana config export --channel <channel>` before any C++ macro.
4. Run each stage with `--dry-run` first, then for real.

Building blocks for new macros and studies, each documented in its package
README:

- `packages/common`: `ApplyStyle` presets, acceptance correction, bin names,
  graph I/O, run-period lists (`Periods.h`), RDataFrame period histograms
  (`PeriodHists.h`), overlays (`Overlay.h`).
- `packages/fit`: RooFit lineshape builders, fit call, weighted import, Johnson
  moments.
- `packages/studies`: study kinds `cutscan` and `datamc`, configured in
  `studies.yaml`.
- `packages/qfactors` with `qfactors.model`: a channel can ship its own
  `configPDFs*.h` model file.

## Documentation

Every package and channel folder has a README describing what it holds, how
it is run and what it writes. Folders without their own README are described
by their parent's.

- Packages: [common](packages/common/README.md) · [xsection](packages/xsection/README.md) ·
  [barlow](packages/barlow/README.md) · [systematics](packages/systematics/README.md) ·
  [fit](packages/fit/README.md) · [studies](packages/studies/README.md) ·
  [montecarlo](packages/montecarlo/README.md)
- kpkpxim: [overview and pipeline](analyses/kpkpxim/README.md) ·
  [selectors](analyses/kpkpxim/selectors/README.md) ·
  [selection](analyses/kpkpxim/selection/README.md)
  ([cut studies](analyses/kpkpxim/selection/cut_studies/README.md),
  [MC studies](analyses/kpkpxim/selection/mc_studies/README.md)) ·
  [Q-factors](analyses/kpkpxim/signal_extraction/qfactors/README.md) ·
  [lineshape](analyses/kpkpxim/signal_extraction/lineshape/README.md) ·
  [flux](analyses/kpkpxim/xsection/flux/README.md) ·
  [external data](analyses/kpkpxim/xsection/external_data/README.md) ·
  systematics ([comparisons](analyses/kpkpxim/systematics/comparisons/README.md),
  [track efficiency](analyses/kpkpxim/systematics/track_efficiency/README.md),
  [MC-weight variations](analyses/kpkpxim/systematics/mc_weight_variations/README.md)) ·
  [measurements](analyses/kpkpxim/measurements/README.md) ·
  [backgrounds](analyses/kpkpxim/backgrounds/README.md) ·
  [simulation](analyses/kpkpxim/simulation/README.md)
- kpkpkmlamb: [overview and pipeline](analyses/kpkpkmlamb/README.md) ·
  [selectors](analyses/kpkpkmlamb/selectors/README.md)
- [`docs/environment.md`](docs/environment.md) — laptop, ifarm and FSU setup, containers, simulation environment, every `GXANA_*` variable
- [`docs/analysis_data.md`](docs/analysis_data.md) — preserved data and golden tests
- [`docs/KNOWN_ISSUES.md`](docs/KNOWN_ISSUES.md) — legacy behaviours kept or fixed on migration
- [`docs/REFACTOR_SPEC.md`](docs/REFACTOR_SPEC.md) — how the legacy working directory became this repository: the as-built layout, packages, config and commands, and the legacy-to-port map of every archived script

**Documentation rule.** A change that adds a feature — a package, header or
app, a `gxana` command or flag, a config file or key, an environment
variable, a golden test, an archived macro — documents it in the README of
the folder that owns it, in the same branch. A new or changed `gxana`
command also updates the command reference above.
`tests/test_docs_layout.py` checks that every package, channel and `gxana`
command is listed here.

**Packaging rule.** Code becomes package code only if the thesis calls it many times (per bin, variation or period) or another channel would run it as is; a thesis-specific fit or figure stays a standalone script with a README run command and, where its inputs are preserved, a golden test (`docs/KNOWN_ISSUES.md` §24).

## License and credit

The author's code is MIT-licensed ([`LICENSE`](LICENSE)). Upstream software
used or patched here (Jefferson Lab GlueX tools, AmpTools, QFactors) keeps its
own authorship and license; see [`NOTICE.md`](NOTICE.md).

## Citing

See [`CITATION.cff`](CITATION.cff) (GitHub shows it under "Cite this repository").
