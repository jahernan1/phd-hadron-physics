# packages/studies

Analysis studies: a block in `analyses/<channel>/config/studies.yaml`, a compiled app that
takes only command-line arguments, and the `gxana run studies` stage that builds each
command from the config (`--dry-run` prints them).

- `include/gxana/studies/`, `src/` — `GxanaStudies` C++/ROOT library (namespace
  `gxana::studies`).
  - `CutScan.h`: the fit and plot of a cut scan with a figure of merit (port of
    `archive/root_macros/CutAnalysisRF.C`): cumulative mass projections over the cut axis,
    Johnson + Chebychev extended fit (`gxana::fit`), S and B in mean ± 2σ, the
    FOM = S/√(S+B), S/B and yield tables (`%f \t %f`), the fit grid and the FOM/S/B
    plot.
  - `DataMC.h`: `DrawStacked`, the data/MC (or thrown/MC) comparison plot of
    `archive/root_macros/GetKinematicsDataMC_RF.C` (`MakeStackedHist`): both rebinned by 2, the second
    scaled by the integral ratio in the direction its maximum picks, THStack, legend top left or
    top right; and that macro's style.
- `apps/gxana_study_cutscan` — `fill` (the mass-vs-cut TH2D through `gxana::FillHists`,
  key `cutscan`), `fit`, `plot`; `--help` prints the arguments. `fit` takes several blocks
  separated by `--next` and runs them in order in one process: the fits use TMinuit
  (`gxana::fit::UseThesisMinimizer`), which keeps its state from one fit to the next, so
  `gxana run studies` plans the fits of all cut-scan studies and periods as one command (period
  outer, study inner, the order of the thesis macro). Only that full run (all cut-scan studies, all
  periods, one process) reproduces the macro's TMinuit fit history; `--study` on the `fit` step fits
  a subset with another history.
- `apps/gxana_study_datamc` — `fill` (per period: data, MC and thrown histograms through
  `gxana::FillPeriodHists`; data with RDataFrame's automatic binning, MC and thrown binned like
  the data) and `plot` (one `DrawStacked` PDF per variable and period).
- `python/gxana_studies/` — `config.py` (checks the `studies` block) and `stage.py`
  (plans and runs `gxana run studies --channel C [--study a,b] [--steps fill,fit,plot]
  [--dry-run]`; kinds `cutscan`: fill, fit, plot; `datamc`: fill, plot). A study may set the optional
  key `threads` (integer >= 0, either kind): it is passed as `--threads` to the `fill` command only;
  `0` turns implicit multithreading off. Absent, the app default applies (4 for `cutscan`, 8 for
  `datamc`) and the command is unchanged. The shipped kpkpxim `studies.yaml` sets `threads: 0` for
  `chisqndf_scan` (inherited by `mm2_scan`) and `kinematics`, as the equivalence tests run.
- `tests/` — pytest: planning and validation (`test_studies_stage.py`), app usage errors
  (`test_studies_apps.py`), and the cut scan against a frozen copy of `CutAnalysisRF.C` on seeded
  toy raw trees (`test_cutscan_equivalence.py`, `tests/legacy/`) and the kinematics study for one
  period against a frozen copy of `GetKinematicsDataMC_RF.C` on the preserved trees
  (`test_kinematics_equivalence.py`, marker `golden`: skipped without `$GXANA_ANALYSIS_DATA`).
  Run with `uv run pytest packages/studies`. The styles `ApplyCutScanStyle` and
  `ApplyDataMCStyle` are checked against the legacy `setStyle()` bodies by the ctest
  `common.style` (`packages/common`); `GXANA_STYLE_DUMP_DIR` keeps the `gStyle` dumps.
