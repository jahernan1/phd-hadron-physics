# Golden tests

Golden tests rerun the stages on the preserved thesis data
([`docs/analysis_data.md`](../../docs/analysis_data.md)) and compare with the legacy
reference outputs (`docs/history/REFACTOR_SPEC.md` D20). They carry the `golden`
marker and skip, each with its reason, when `$GXANA_ANALYSIS_DATA/kpkpxim` or a file
they need is absent, when ROOT is not on `PATH`, or when the C++ apps are not built
(`uv run cmake --build build`); fixtures in `conftest.py`.

    uv run pytest -m golden -v       # all of them
    uv run pytest <path>             # one module
    uv run pytest -m golden -rs      # -rs lists the skips; a skipped test proves nothing

| Variable | Effect |
|---|---|
| `GXANA_GOLDEN_RTOL` | relative tolerance of the deterministic columns (default `1e-5`) |
| `GXANA_GOLDEN_FIT_RTOL` | relative tolerance of the fit-yield columns (default per test and label) |
| `GXANA_GOLDEN_SYST_OUTPUT` | `GXANA_OUTPUT` of a finished systematics run; `test_systematics_chain_golden.py` checks it instead of rerunning |
| `GXANA_GOLDEN_QFACTORS_MODEL` | `configPDFs` model compared by `test_qfactors_golden.py` instead of the configured one |

## ROOT 6.24 container run

The ROOT 6.24 container golden run has not been run yet. On the ifarm, inside
`gxana.sif` (container section of [`docs/environment.md`](../../docs/environment.md);
the image has no `uv`, it pip-installs pytest, pyyaml, numpy and pandas, and
`setup.sh` puts the gxana Python packages on `PYTHONPATH`), from the checkout:

    source env/setup.sh --gluex && cmake -S . -B build && cmake --build build -j
    GXANA_GOLDEN_FIT_RTOL=1e-5 python3 -m pytest -m golden -rs

## Tests

In `tests/golden/`:

- `test_manifest_golden.py` — every file on disk matches the sha256 in
  `analyses/kpkpxim/analysis_data.yaml`.
- `test_xsection_reproduction_golden.py` — the documented route end to end: `gxana data
  stage`, `gxana run xsection --steps tables,weight,integrate,components`, then
  `tex --systematics published`; compares the `johnson` tables, weighted
  averages, components and printed LaTeX values with the preserved reference tables
  (tolerances in the module docstring).
- `test_binning_golden.py` — `gxana_xsec_bin` reproduces the legacy binned trees
  from the preserved flat trees: tree names, entry counts and branch sums exact. The MC
  and thrown cases are strict expected failures: the preserved flat trees are a later
  production than the thesis binned trees (`docs/KNOWN_ISSUES.md`, "MC sample provenance").
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
