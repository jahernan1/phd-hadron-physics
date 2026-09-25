# kpkpkmlamb DSelector

γp → K⁺K⁺K⁻Λ (Λ → pπ⁻) selector, run with `gxana run select --channel kpkpkmlamb`
(PROOF-Lite via `DPROOFLiteManager`, compiled by ACLiC). It needs
gluex_root_analysis (DSelector base classes), so it loads only inside the
GlueX environment (`source env/setup.sh --gluex` or the container).

| Selector | Used for | `dOutputFileName` | flat tree |
|---|---|---|---|
| `DSelector_kpkpkmlamb.C/.h` | data, all three periods | `kpkpkmlamb.root` | `flatTree_kpkpkmlamb.root` (tree `flatTree_kpkpkmlamb`) |

`output_basename` in `config/channel.yaml` must equal `dOutputFileName`;
`tests/kpkpkmlamb/test_kpkpkmlamb_config.py` checks it.

## Known issues (documented, not fixed)

- `Process` prints per event and per combo (`cout` spam).
- The template's example branches (`my_int`, `my_p4`, `flat_my_*`) are still defined.

The selector was generated from the gluex_root_analysis DSelector template
(Jefferson Lab) and keeps its header comments.
