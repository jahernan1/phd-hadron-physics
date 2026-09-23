# phd-hadron-physics

Analysis code for a PhD measurement of Ξ⁻(1320) photoproduction at the GlueX
experiment (Hall D, Jefferson Lab):

```
γ p → K⁺ K⁺ Ξ⁻,   Ξ⁻ → π⁻ Λ,   Λ → p π⁻
```

Pipeline: DSelector event selection → flat trees → cut optimisation →
Q-factor signal weighting → yield fits → acceptance & flux correction →
differential/total cross sections → Barlow systematics; plus Ξ(1320) mass and
spin measurements, Monte-Carlo generation (gen_amp + halld_sim), and a side
channel (K⁺K⁺K⁻Λ, excited Ξ*) used to validate the framework on rare signals.

> **Status:** repository is being restructured — see
> [`docs/REFACTOR_SPEC.md`](docs/REFACTOR_SPEC.md). No GlueX data or results
> are stored here; code only.

## Layout

| Path | Contents |
|---|---|
| `packages/common` | `gxana` Python CLI + `GxanaCommon` C++/ROOT library |
| `analyses/kpkpxim` | main thesis channel (config) |
| `env/` | environment setup and container definition |
| `docs/` | environment guide and refactor spec |

## Quickstart (laptop, ROOT ≥ 6.20 installed)

```bash
uv sync                                   # python toolkit + dev tools
source env/setup.sh                       # GXANA_* variables
uv run cmake -S . -B build -DCMAKE_PREFIX_PATH="$(root-config --prefix)"
uv run cmake --build build -j && uv run ctest --test-dir build
uv run pytest
uv run gxana doctor
```

On the JLab ifarm or the FSU grid see [`docs/environment.md`](docs/environment.md).
