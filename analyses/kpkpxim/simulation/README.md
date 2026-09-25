# Signal-MC production

Thesis signal-MC (γp → K⁺K⁺Ξ⁻ via Y* → K⁺Ξ⁻) production chain, templates
and the `gxana run mc` stage that renders and submits them. Replaces the
author's ifarm `runAllMC.sh`.

## 1. Chain

1. **gen_amp_V2** generates events sampled from a data-derived Y* → K⁺Ξ⁻
   mass-vs-cos θ (`YstarRest` frame) 2-D histogram (`gen_amp_cfg/`).
2. **hdgeant4** simulates the detector response, with Λ → pπ⁻ forced.
3. **mcsmear** applies detector smearing.
4. **hd_root** runs `ReactionFilter` (`Reaction1 1_14__11_11_23`, with
   period-dependent flags — see the directory table below) to produce the
   flat trees and thrown trees the selectors read.

All four steps are driven end to end by MCwrapper (`gluex_MC.py`) on the
JLab batch farm.

## 2. One-time setup

Inside the GlueX container, once per sim version set:

```
packages/montecarlo/scripts/build_halld_sim.sh <version-set>
gxana externals fetch gluex_MCwrapper
```

## 3. Produce

```
source env/setup.sh --sim=recon-2018_08-ver02_31
gxana run mc --period 2018-08 --sample gen_amp_V2_ac_YstarRest --dry-run
gxana run mc --period 2018-08 --sample gen_amp_V2_ac_YstarRest
```

Always run `--dry-run` first to check the rendered plan (output directory,
generator config, sim version set, `gluex_MC.py` argv) before submitting.

Periods, sim version sets, run ranges and event counts (`config/mc.yaml`,
`config/periods.yaml`):

| Period | Sim version set | Runs | Events |
|---|---|---|---|
| 2017-01 | `recon-2019_11-ver01_13` | 30274-31057 | 3,735,000 |
| 2018-01 | `recon-2018_01-ver02_32` | 40856-42559 | 11,190,000 |
| 2018-08 | `recon-2018_08-ver02_31` | 50685-51768 | 7,055,000 |

Note: the 2017-01 sample was produced against the `recon-2019_11-ver01_13`
set, not a 2017-dated one, as the thesis production did.

## 4. After the jobs

`gxana run mc` prints the `ln -s` commands that put MCwrapper's
`<run_dir>/root/{trees,thrown}/` where `gxana run select` expects them.
Run them, then:

```
gxana run select --period P --sample S
gxana run select --period P --sample S --thrown
```

## 5. Sampling histograms

The 2-D sampling histograms consumed by `gen_amp_cfg/` are built by
`sampling/getHist2D_gen_amp.C` from the previous iteration's flat trees.
Run it from `$GXANA_OUTPUT/kpkpxim/simulation/sampling`; it writes
`data_ac_ximVertexCut_hist2d_YstarRest.root`.

The copies used for the thesis production are preserved analysis data:
see `gxana data status --channel kpkpxim`, under `simulation/sampling/`.

## 6. Directory table

| Directory | Contents |
|---|---|
| `gen_amp_cfg/` | gen_amp_V2 generator configs (templates; rendered by `gxana run mc`) |
| `mcwrapper/` | Per-period MCwrapper confs (templates; rendered by `gxana run mc`) |
| `hd_root/` | `hd_root_xim_2017.conf` (flags `M23`), `hd_root_xim_2018.conf` (flags `B4_M23`), `hd_root_xim_pi0_2018.conf` (π⁰ background variant), `hd_root_xim__B4_F1_M23.config` (F1 flags), `hd_root_xim13.config` (local hd_root re-run config) |
| `genr8/` | Legacy genr8 generator inputs, including the F1 `ystar2400_genr8` production config; reference only |
| `local_beam.conf` | Local beam configuration used by the chain |
| `sampling/` | Sampling-histogram macros (§5) |
| `validation/` | Acceptance and iteration-convergence checks, including both `compare_iters` variants |

## 7. Re-run hd_root on existing REST files

`packages/montecarlo/scripts/run_hdroot.py` re-runs hd_root with a
`ReactionFilter` over existing MCwrapper REST files, one worker per run
number, without regenerating events. See its `--help` for options.
