# Preserved analysis data

GlueX practice for documenting dissertation work: the code is published on
GitHub and the data it analysed is preserved on disk under
`/work/halld/gluex_analysis_data/` at JLab. This repository follows the same
split — code in git, data under `$GXANA_ANALYSIS_DATA`, never committed.

| Site | `GXANA_ANALYSIS_DATA` |
|---|---|
| default (laptop) | `$GXANA_ROOT/gluex_analysis_data` — gitignored; may be a symlink |
| JLab | the thesis directory under `/work/halld/gluex_analysis_data/`, set in `env/site.sh` |

Each channel keeps its files in `$GXANA_ANALYSIS_DATA/<channel>/` and documents
them in `analyses/<channel>/analysis_data.yaml`: a description per directory
and the sha256 + size of every file.

## kpkpxim

Tree stems `<P>`: `kpkpxim__M23_2017-01_ana56`, `kpkpxim__B4_M23_2018-01_ana03`,
`kpkpxim__B4_M23_2018-08_ana02`.

    kpkpxim/
      flat_trees/     postQVal_flatTree_<P>_nominal_kphighrap_1111111.root
                      flatTree_<P>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root
                      flatTree_thrown_<P>_gen_amp_V2_ac_YstarRest.root
      binned_trees/   binned_flatTree_<P>_nominal_kphighrap.root
                      binned_flatTree_<P>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root
                      binned_thrown_flatTree_<P>_gen_amp_V2_ac_YstarRest.root
      flux/           flux_30274_31057_r4.root flux_40856_42559.root flux_50685_51768.root
      reference/xsection/
        <label>/ weighted/<label>/ components/{sp17,sp18,fa18}/<label>/
          hybrid_combo  JohnsonMCShape study (legacy MakeXSecFiles.C), weight hybrid_combo
          best_combo    JohnsonMCShape study, weight best_combo  (combo-selection study)
          acc_weight    JohnsonMCShape study, weight acc_weight  (combo-selection study)
          johnson       dissertation fit (legacy MakeXSecFitVariations.C), weight hybrid_combo
        tables/         dissertation tables (*_scale.tex, from weighted/johnson) and
                        JohnsonMCShape study tables (*_runsyst.tex, from weighted/hybrid_combo)
        qvalues/

## Commands

    gxana data path   --channel kpkpxim   # where the files are expected
    gxana data status --channel kpkpxim   # ok | open (not locked) | miss | diff | new
    gxana data lock   --channel kpkpxim   # record sha256 + bytes, then commit the manifest
    gxana data stage  --channel kpkpxim [--dry-run]   # place the inputs where the stages read them

`status` on a clone without the data directory prints `no preserved data at <path>`
and exits 0 (golden tests skip); `lock` then fails with exit 2. A `miss` or `diff`
file makes `status` exit 1.

`stage` follows the `stage:` block of the manifest (per run period: `from` under the
data directory, `to` a stage location, `mode`). `copy` is for files a later step
writes at the same path (binned trees, post-Q-factor trees, MC flat trees), so the
preserved file is never written through a link; `link` is for the thrown trees, which
`gxana run select --thrown` replaces (a linked destination is unlinked first, never written through, also on a cross-filesystem move). It never overwrites a different file:
a destination that exists and differs is reported as `conflict`, nothing is staged and
the exit is 1 (as for a `missing` source). Re-running after a successful stage reports
every file `ok`. Without the data directory it exits 2.

## Staging and the thesis route

`stage:` in `analyses/<channel>/analysis_data.yaml` holds `mc_sample` (the MC sample
name that fills `{mc_stem}`), `next` (the command printed after a successful stage) and
`files`, a list of `{from, to, mode}`. `from` is relative to the data directory, `to`
is a stage location (`${GXANA_OUTPUT}` and `${GXANA_DATA}` expand); `{stem}` expands
once per run period and `{mc_stem}` to `<period stem>_<mc_sample>`; a `to` ending in `/`
is a directory. `mode: copy` or `link` follows the copy-versus-link rule above; a
different file at the destination is a `conflict`, a source that is absent is
`missing`. kpkpxim:

| Preserved file (per period) | Staged to | Mode |
|---|---|---|
| `binned_trees/binned_flatTree_<P>_nominal_kphighrap.root` | `$GXANA_OUTPUT/kpkpxim/xsection/binned_trees/` | copy |
| `binned_trees/binned_flatTree_<P>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root` | `$GXANA_OUTPUT/kpkpxim/xsection/binned_trees/` | copy |
| `binned_trees/binned_thrown_flatTree_<P>_gen_amp_V2_ac_YstarRest.root` | `$GXANA_OUTPUT/kpkpxim/xsection/binned_trees/` | copy |
| `flat_trees/postQVal_flatTree_<P>_nominal_kphighrap_1111111.root` | `$GXANA_OUTPUT/kpkpxim/qfactors/<P>_nominal_kphighrap_1111111/` | copy |
| `flat_trees/flatTree_<P>_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root` | `$GXANA_DATA/flatTrees/` | copy |
| `flat_trees/flatTree_thrown_<P>_gen_amp_V2_ac_YstarRest.root` | `$GXANA_DATA/flatTrees/` | link |

The thesis route (stage, then `gxana run xsection` without `bin`) is route 2 of
[RERUN_THESIS.md](RERUN_THESIS.md#2-rerun-from-the-preserved-data).

## Golden tests

Golden tests (marker `golden`) rerun the stages on these files and compare with the
legacy reference outputs; they skip when the data, ROOT or the built apps are absent.
How to run them, the tolerance variables, the ROOT 6.24 container run and the list of
tests: [tests/golden/README.md](../tests/golden/README.md).

## Macros that cannot run on the preserved data

Thesis-era macros in `analyses/kpkpxim/` that do not run on the preserved data: an
input is not preserved, belongs to an older MC sample, or is written by no macro in the
repository. Paths as the macro opens them.

| Macro (`analyses/kpkpxim/`) | Missing input | Effect / status | README |
|---|---|---|---|
| `selection/cut_studies/{chisqndf_cut,mm2_cut}/get_data_hists.C` | `$GXANA_DATA/Trees/flatTree/rawTrees/flatTree_<stem>_Ystar2400_1600_genr8.root` (older genr8 MC) | superseded by `get_data_hists_RF.C` | [cut_studies](../analyses/kpkpxim/selection/cut_studies/README.md) |
| `selection/cut_studies/{xim_vertex_cuts,lambda_vertex_cut}/get_data_hists.C`; `xim_vertex_cuts/make_plot.C` | `$GXANA_DATA/flatTrees/flatTree_<stem>_Ystar2400_1600_genr8_nominal_Weighted.root` | superseded by the `_RF` / `make_plot_RF.C` macros | [cut_studies](../analyses/kpkpxim/selection/cut_studies/README.md) |
| `selection/cut_studies/kaon_selection/get_data_hists.C` | `$GXANA_DATA/flatTrees/flatTree[_thrown]_<stem>_gen_amp_nominal_allKaonSep_Weighted.root` | superseded by `get_data_hists_RF.C` | [cut_studies](../analyses/kpkpxim/selection/cut_studies/README.md) |
| `selection/cut_studies/kaon_selection/get_data_hists_ellipse.C` | `ver56`/`ver03`/`ver02` `_nominal_vertexCuts` flat trees and `$GXANA_OUTPUT/legacy_macros/QFactors/logs/` | legacy stems; not runnable | [cut_studies](../analyses/kpkpxim/selection/cut_studies/README.md) |
| `selection/cut_studies/chisqndf_cut/chisqndf_2017.C` | none (histogram dump, no entry function) | not a runnable macro | [cut_studies](../analyses/kpkpxim/selection/cut_studies/README.md) |
| `selection/cut_studies/accidentals/get_data_hists.C`, `make_plot.C` | Q-factor output of `_nominal_momCut` (its `flatTreePrep.C` call is commented out); `$GXANA_DATA/flatTrees/flatTree_<stem>_gen_amp_V2_2D_ac_nominal_momCut.root` (`gen_amp_V2_2D` is 2018-08 only in `config/samples.yaml`); `data_allKaonSep.root` (no macro writes it) | `rfbunches_phase1.pdf`, `rfbunches_mc_phase1.pdf` and the combo-method plots are not produced | [cut_studies](../analyses/kpkpxim/selection/cut_studies/README.md) |
| `selection/mc_studies/get_data_hists.C`, `make_plot.C` | `$GXANA_DATA/flatTrees/flatTree[_thrown]_<stem>_gen_amp_nominal_allKaonSep_Weighted.root` and the `_nominal_allKaonSep_1111111` Q-factor output | superseded by `get_data_hists_RF.C` / `make_plot_RF.C` | [mc_studies](../analyses/kpkpxim/selection/mc_studies/README.md) |
| `selection/mc_studies/make_plot_acceptcorr.C` | `data_RF.root` (no macro writes it; `get_data_hists_RF.C` writes `data_ac_hist2d_kphighrap_2d.root`) | not runnable as preserved | [mc_studies](../analyses/kpkpxim/selection/mc_studies/README.md) |
| `measurements/mass/MakeXim1320_IM.C` | `$GXANA_DATA/flatTrees/flatTree_<stem>_nominal_ximVertexCut.root` | stops at the first delim; `Xim_InvariantMassFit_Phase1_{ximVertexCut,kphighrap}.pdf` not written | [measurements](../analyses/kpkpxim/measurements/README.md#cannot-run-as-preserved) |
| `measurements/mass/MakeXim1820_IM.C` | `$GXANA_DATA/KpKpKmL012017012018082018Real_31July.root` (hand-made tree) | excited-Ξ fit not reproducible; PDF `Print` commented out | [measurements](../analyses/kpkpxim/measurements/README.md#cannot-run-as-preserved) |
| `systematics/mc_weight_variations/get_data_hists.C`, `WeightMC.C` | `$GXANA_OUTPUT/kpkpxim/systematics/root_trees/flatTree_<stem>[_Ystar2400_1600_genr8]_vary<delim>.root`, `$GXANA_DATA/flatTrees/flatTree_thrown_<stem>_Ystar2400_1600_genr8.root` (older genr8 MC); `WeightMC.C` defaults name a `ver56` stem | MC-model weight study not reproducible | [mc_weight_variations](../analyses/kpkpxim/systematics/mc_weight_variations/README.md#cannot-run-as-preserved) |
| `backgrounds/KstarFit.C` | `$GXANA_DATA/flatTrees/flatTree_kpkpxim__M23_2017-01_ver56_allCuts.root` and `$GXANA_OUTPUT/legacy_macros/QFactors/logs/kpkpxim_2017-01__M23_ana56_allCut_111111/` | K* fit not reproducible | [backgrounds](../analyses/kpkpxim/backgrounds/README.md#fit-macros) |

## Depositing at JLab

    rsync -av gluex_analysis_data/kpkpxim/ /work/halld/gluex_analysis_data/<thesis-dir>/kpkpxim/
    # env/site.sh:  export GXANA_ANALYSIS_DATA=/work/halld/gluex_analysis_data/<thesis-dir>
    gxana data status --channel kpkpxim   # expect every file ok
