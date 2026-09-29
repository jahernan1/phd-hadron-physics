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

## Layout

| Path | Contents |
|---|---|
| `packages/common` | `gxana` Python CLI + `GxanaCommon` C++/ROOT library |
| `packages/xsection` | `GxanaXsec` cross-section library + `gxana_xsection` Python helpers |
| `packages/montecarlo` | pinned upstream MC generators + patches (`gxana externals`, `gxana run mc`) |
| `packages/qfactors` | git submodule: the QFactors fork as run for the thesis (`gxana run qfactors`) |
| `analyses/kpkpxim` | main thesis channel: selectors, selection, cross section, systematics, measurements, backgrounds; pipeline in [`analyses/kpkpxim/README.md`](analyses/kpkpxim/README.md) |
| `analyses/kpkpkmlamb` | side channel: excited Ξ* → K⁻Λ; [`analyses/kpkpkmlamb/README.md`](analyses/kpkpkmlamb/README.md) |
| `archive/` | superseded legacy code kept verbatim for provenance ([`archive/README.md`](archive/README.md)) |
| `scripts/` | migration helpers (`migrate_paths.py`, `archive_copy.sh`) |
| `env/` | environment setup and container definition |
| `docs/` | environment guide, preserved-data guide, known issues, refactor spec |

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
uv run pytest -m golden                   # reproduce the thesis tables from preserved data (~10 min)
```

On the JLab ifarm or the FSU grid see [`docs/environment.md`](docs/environment.md).

## License and credit

The author's code is MIT-licensed ([`LICENSE`](LICENSE)). Upstream software
used or patched here (Jefferson Lab GlueX tools, AmpTools, QFactors) keeps its
own authorship and license; see [`NOTICE.md`](NOTICE.md).

## Citing

See [`CITATION.cff`](CITATION.cff) (GitHub shows it under "Cite this repository").

## Documentation

- [`docs/environment.md`](docs/environment.md) — laptop, ifarm and FSU setup, containers, simulation environment
- [`docs/analysis_data.md`](docs/analysis_data.md) — preserved data and golden tests
- [`docs/KNOWN_ISSUES.md`](docs/KNOWN_ISSUES.md) — legacy behaviours kept or fixed on migration
- [`docs/REFACTOR_SPEC.md`](docs/REFACTOR_SPEC.md) — how the legacy working directory became this repository
