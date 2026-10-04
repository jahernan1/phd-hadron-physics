# Signal-MC production

Thesis signal-MC (γp → K⁺K⁺Ξ⁻ via Y* → K⁺Ξ⁻) production chain, templates
and the `gxana run mc` stage that renders and submits them. Replaces the
legacy AnalysisNote `runAllMC.sh`.

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

### Version sets

The reconstruction version sets are the newest available at production time:
`recon-2019_11-ver01_13` (Spring 2017 data are REST version 4, whose
reconstruction set maps to 2019_11), `recon-2018_01-ver02_32` and
`recon-2018_08-ver02_31` (`config/mc.yaml`). The analysis-note text lists
older `_11`/`_30`/`_29` sets; the confs are authoritative.

## 4. After the jobs

`gxana run mc` prints the `ln -sfn` commands that put MCwrapper's
`<run_dir>/root/{trees,thrown}/` where `gxana run select` expects them.
Run them, then:

```
gxana run select --period P --sample S
gxana run select --period P --sample S --thrown
```

## 5. Sampling histograms

The 2-D sampling histograms consumed by `gen_amp_cfg/` are built by
`sampling/PrepSampling.C` (entry `PrepSampling()`) from the previous iteration's flat trees.
Run `gxana config export --channel kpkpxim` first (it writes the run periods and tree stems
the macro reads, `$GXANA_OUTPUT/kpkpxim/config/channel.kv`), then
run it from `$GXANA_OUTPUT/kpkpxim/simulation/sampling`; it writes
`data_ac_ximVertexCut_hist2d_YstarRest.root`. The acceptance and the period merge
use the common acceptance library; it prints a warning when populated data bins
have zero acceptance, since `gen_amp` cannot sample them.

The copies used for the thesis production are preserved analysis data:
see `gxana data status --channel kpkpxim`, under `simulation/sampling/`.

`getHist3D.C` and `getHist3D_F18.C` (the 3-D (mass, cos θ, t) sampling histograms for the `Hist3D` amplitude) were
archived (in `archive/root_macros/`) on 2026-10-04: they are not part of the thesis production, read the `nominalBC` flat-tree
variant that `selection/flatTreePrep.C` does not write, and have never run in this repository.

### Sampling bootstrap

The sampling histograms are iterated:

1. Start from the `noac` (no acceptance correction) histogram configuration
   `gen_amp_cfg/kpkpxim_2dhist_noac_YstarRest.cfg`; its header records the
   generator options (`-t 1.45 1 -mask 1 1 0`).
   Line 1 of a generator cfg carries the gen_amp options (MCwrapper reads it with
   `head -n 1`) and must stay line 1. The thesis text quotes b = 1.43, but the
   as-run cfg header is 1.45 and is kept as run.
2. Produce and select the MC, then iterate the t slope (the `-t 1.45` value in
   the cfg header) until the MC t distribution matches the data.
3. Build the acceptance-corrected histogram with `PrepSampling.C`, use it
   in `gen_amp_cfg/kpkpxim_2dhist_ac_YstarRest.cfg`, and copy the result into
   `$GXANA_ANALYSIS_DATA/kpkpxim/simulation/sampling/`, where the cfgs read
   their histograms.

## 6. Directory table

| Directory | Contents |
|---|---|
| `gen_amp_cfg/` | gen_amp_V2 generator configs (templates; rendered by `gxana run mc`) |
| `mcwrapper/` | Per-period MCwrapper confs (templates; rendered by `gxana run mc`) |
| `hd_root/` | `hd_root_xim_2017.conf` (flags `M23`), `hd_root_xim_2018.conf` (flags `B4_M23`), `hd_root_xim_pi0_2018.conf` (π⁰ background variant), `hd_root_xim__B4_F1_M23.config` (F1 flags), `hd_root_xim13.config` (local hd_root re-run config) |
| `genr8/` | Legacy genr8 generator inputs, including the F1 `ystar2400_genr8` production config; reference only |
| `sampling/` | Sampling-histogram macros (§5) |
| `validation/` | Acceptance and iteration-convergence checks, including both `compare_iters` variants |

`validation/make_plot_RF.C` calls the common style presets, so it must be run through
`rootlogon.C` with `GXANA_ROOT` set, like the other plot macros. `validation/get_data_hists_RF.C`
and `validation/in_out_test.C` read the run periods and tree stems from
`$GXANA_OUTPUT/kpkpxim/config/channel.kv`: run `gxana config export --channel kpkpxim` first.

## 7. Re-run hd_root on existing REST files

`packages/montecarlo/scripts/run_hdroot.py` re-runs hd_root with a
`ReactionFilter` over existing MCwrapper REST files, one worker per run
number, without regenerating events. See its `--help` for options.

`--flags` defaults to `B4_U1_M23`, but the production trees used `M23`
(2017) and `B4_M23` (2018; see the `hd_root/` table above). Pass `--flags`
explicitly (`--flags M23` for 2017, `--flags B4_M23` for 2018) to reproduce
the production trees.
