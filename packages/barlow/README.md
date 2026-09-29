# barlow

Barlow cut-variation consistency check, run as `gxana run barlow --channel <ch>`.
Configured by `analyses/<channel>/config/barlow.yaml`; another analysis reuses it by
writing that one file.

| Header | Contents |
|---|---|
| `Barlow.h` | `calc_barlow` (signed σ_B = Δ/√\|σ_n² − σ_v²\|, 0 when that is 0), `calculateStdDevGraph` |
