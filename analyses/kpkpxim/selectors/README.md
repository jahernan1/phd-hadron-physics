# kpkpxim DSelectors

γp → K⁺K⁺Ξ⁻ (Ξ⁻ → π⁻Λ, Λ → pπ⁻) selectors, run with `gxana run select`
(PROOF-Lite via `DPROOFLiteManager`, compiled by ACLiC). They need
gluex_root_analysis (DSelector base classes), so they load only inside the
GlueX environment (`source env/setup.sh --gluex` or the container), not on a
plain ROOT install.

| Selector | Used for | `dOutputFileName` | flat tree | Notes |
|---|---|---|---|---|
| `DSelector_kpkpxim.C/.h` | nominal data and signal MC | `kpkpxim.root` | `flatTree_kpkpxim.root` | default selector |
| `DSelector_kpkpxim_F1.C/.h` | F1-flag trees (`F1`, `F1_ystar2400_genr8`) | `kpkpxim_F1.root` | none | |
| `DSelector_kpkpxim_2017.C/.h` | 2017-01 study variant | `kpkpxim2017.root` | none | not referenced by `samples.yaml` |
| `DSelector_kpkpxim_hybrid.C` | alternate `DSelector_kpkpxim` implementation | `kpkpxim.root` | `flatTree_kpkpxim.root` | includes `DSelector_kpkpxim.h` (no own header); same file names as the nominal selector, so never run both in one directory (`run select` uses a per-save run directory) |
| `DSelector_thrown_kpkpxim.C/.h` | thrown trees of every MC sample | `thrown_kpkpxim.root` | `flatTree_thrown_kpkpxim.root` | default thrown selector |
| `DSelector_thrown_kpkpxim_F1.C/.h` | thrown F1 study | `thrown_kpkpxim_F1.root` | none | not referenced by `samples.yaml` |

`output_basename` in `config/` must equal `dOutputFileName`;
`packages/common/tests/python/test_selectors_config.py` checks every sample.

## Known issues (documented, not fixed)

- `DSelector_kpkpxim.C::Process` prints per combo (`cout` spam).
- PID ΔT and Ξ mass-window cuts are commented out in the selector; they are
  applied downstream in `selection/`.

The selectors were generated from the gluex_root_analysis DSelector template
(Jefferson Lab) and keep its header comments.
