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

The thesis route is `stage`, then `gxana run xsection --steps
tables,weight,integrate,components`, `gxana run systematics`, `gxana run xsection
--steps tex`. It starts from the preserved binned trees and leaves `bin` out: the
preserved MC and thrown flat trees are a later production than the thesis binned trees,
so re-binning them moves the MC and thrown yields and the acceptance (measured
numbers: `docs/KNOWN_ISSUES.md`, section "MC sample provenance"). `test_binning_golden.py` marks the MC and thrown cases as
expected failures for this reason. `--steps bin` still runs on staged data; its numbers
are not the thesis.

## Golden tests

Golden tests rerun the stages on these files and compare with the legacy
reference outputs (`REFACTOR_SPEC.md` D20). They carry the `golden` marker and skip, each
with its reason, when `$GXANA_ANALYSIS_DATA/kpkpxim` or a file they need is
absent, when ROOT is not on `PATH`, or when the C++ apps are not built
(`uv run cmake --build build`). Run them all with `uv run pytest -m golden -v`,
or one with `uv run pytest <path>`; `-rs` lists the skips (a skipped test proves
nothing).

The ROOT 6.24 container golden run has not been run yet. On the ifarm, inside `gxana.sif`
(container section of [environment.md](environment.md); the image has no `uv`, it
pip-installs pytest, pyyaml, numpy and pandas, and `setup.sh` puts the gxana Python
packages on `PYTHONPATH`), from the checkout:

    source env/setup.sh --gluex && cmake -S . -B build && cmake --build build -j
    GXANA_GOLDEN_FIT_RTOL=1e-5 python3 -m pytest -m golden -rs

In `tests/golden/`:

- `test_manifest_golden.py` — every file on disk matches the sha256 in
  `analyses/kpkpxim/analysis_data.yaml`.
- `test_xsection_reproduction_golden.py` — the documented route end to end: `gxana data
  stage`, `gxana run xsection --steps tables,weight,integrate,components`, then
  `tex --systematics published`; compares the `johnson` tables, weighted
  averages, components and printed LaTeX values with the preserved reference tables
  (tolerances in the module docstring).
- `test_binning_golden.py` — `gxana_xsec_bin` reproduces the legacy binned trees
  from the preserved flat trees: tree names, entry counts and branch sums exact.
- `test_xsec_golden.py` — `gxana_xsec_tables` reproduces the legacy tables of the
  `hybrid_combo`, `best_combo`, `acc_weight` and `johnson` labels: deterministic
  columns at `GXANA_GOLDEN_RTOL` (default `1e-5`), fit-yield columns at
  `GXANA_GOLDEN_FIT_RTOL` (default per label, the maximum seen on ROOT 6.40). The
  authoritative run is in the analysis container (ROOT 6.24.04, as for the thesis)
  with `GXANA_GOLDEN_FIT_RTOL=1e-5`; on newer ROOT record the reported maximum deviation.
- `test_python_golden.py` — `gxana_xsection` reproduces the weighted average (to the
  printed 6 decimals), components and Q-value rescale (`1e-12`) and the LaTeX
  tables (byte-identical).
- `test_xsec_figures_golden.py` — `gxana run xsection --steps figures --systematics
  published` on the preserved `johnson` tables (`xsection.published_systematics`: the
  preserved `fit_variations_stats.txt`, `combo_variations_stats.txt`, the scale-factor run
  systematic and the preserved `hybrid_combo` tables): the `syst_weighted_diffxsec_*` tables are byte-identical to the legacy ones;
  the drawn points, statistical bars and systematic band of
  `diffxsec_phase1_systematics_johnson.pdf` equal `diffxsec_table_scale.tex` to its
  3 decimals, panels in ascending energy; `diffxsec_runs_johnson.pdf` draws the preserved
  period tables; `totxsec_clas_gluex_Phase1.pdf` draws the `hybrid_combo` direct totals and
  its fit prints χ²/ν = 0.74, as in the dissertation. Pixels are not compared.
- `test_qfactors_golden.py` — the QFactors fork with `config/qfactors.yaml`
  reproduces the thesis 2017-01 Q-factor values on four slices of 3 events;
  `GXANA_GOLDEN_QFACTORS_MODEL=<model>` compares another `configPDFs` model.
- `test_sampling_golden.py` — `AcceptanceCorrect` (GxanaCommon) reproduces the
  preserved 2-D sampling histogram (the gen_amp `Hist2D` input) from its stored raw
  per-period histograms, bin for bin.
- `test_barlow_plot_golden.py` — `gxana_barlow_plot`: σ_B in every `.txt` equals an
  independent recomputation of the legacy formula (`1e-6`), and every PDF matches the
  archived `PlotXSecBarlow*.C` PDF pixel for pixel within a small tolerance.
- `test_systematics_text_golden.py` — `gxana run barlow --steps weight` reproduces the
  legacy weighted variation tables.
- `test_systematics_numbers_golden.py` (exact) — the accidentals spread, the PDG scale
  factor S, the Run Combination column and the LaTeX tables in columns mode reproduce
  the preserved thesis files.
- `test_systematics_summary_golden.py` — the `summary` total reproduces the published
  δy(syst) column of `diffxsec_table_scale.tex` in all 56 bins.
- `test_systematics_plot_golden.py` — `gxana_syst_plot` draws what the archived
  comparison macros drew (accidentals, fit, Q-value yield, run comparison).
- `test_systematics_track_golden.py` — the track-efficiency study reproduces the
  dissertation table and the figures of the archived `get_hists.C` /
  `get_track_efficiency.C`.
- `test_systematics_chain_golden.py` (loose) — runs `gxana run xsection --steps
  tables,weight` and `gxana run systematics --study fit --steps fit,qvalue,weight,spread`
  (about 7 min on a laptop) and compares `fit_variations_stats.txt` at loose ROOT 6.40
  tolerances (a regression guard, see `docs/KNOWN_ISSUES.md` §3); set
  `GXANA_GOLDEN_SYST_OUTPUT` to a finished run's `GXANA_OUTPUT` to check it without
  rerunning.
- `test_runperiod_golden.py` — `gxana run systematics --steps runperiod`
  (`GetRunPeriodPctSig.C`) against the original macro (frozen in
  `tests/golden/legacy/runperiod/`), both single-threaded: the same printed lines, the
  same 27 PDF names and, with Ghostscript, identical rasters.
- `test_measurements_golden.py` — the split mass, lifetime and spin macros reproduce
  the fit results of the original `GetXimProperties.C` and `PlotGlueXSpin.C`
  (`tests/golden/data/measurements_reference.txt`), single-threaded.
- `test_measurements_stage_golden.py` — `gxana run measurements --channel kpkpxim`:
  the same fit-result lines, the three ROOT files and the thirteen PDFs.

In the packages:

- `packages/studies/tests/python/test_kinematics_equivalence.py` — the `kinematics`
  study of `gxana run studies` for 2018-08 against a frozen copy of
  `GetKinematicsDataMC_RF.C` on the preserved trees, single-threaded: the same 36 PDFs,
  the same printed lines and, with Ghostscript, identical rasters for four PDFs.

## Depositing at JLab

    rsync -av gluex_analysis_data/kpkpxim/ /work/halld/gluex_analysis_data/<thesis-dir>/kpkpxim/
    # env/site.sh:  export GXANA_ANALYSIS_DATA=/work/halld/gluex_analysis_data/<thesis-dir>
    gxana data status --channel kpkpxim   # expect every file ok
