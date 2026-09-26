# Refactor Spec — phd-hadron-physics

Status: **applied** (Plans 1–6, 2026-09-25). Kept as the design record.
Supersedes `REFACTOR_PLAN.md` (punch list) and builds on `PROJECT_REVIEW.md`
(both move to `docs/history/`). Implementation is split into the plans listed
in §14; each plan lives in `docs/superpowers/plans/`.

## 1. Goal

Turn the current working directory (a copied JLab/FSU `$HOME` with ~5 GB of
data, nested upstream clones, and ~440 hardcoded `/d/grid17/hjesse` paths)
into **one cohesive, public GitHub repository** `jahernan1/phd-hadron-physics`:

- reusable **packages** (common utilities, cross-section library, Q-factor
  engine fork, Monte-Carlo generation machinery),
- channel **analyses** that consume those packages (main thesis channel
  `kpkpxim`; side channel `kpkpkmlamb`),
- a pinned, documented **environment** that runs the code the same way at
  FSU, on the JLab ifarm, and on OSG,
- **clear credit** for upstream code (JLab, lan13005, mashephe) — never
  vendored as if it were the author's.

Non-goals: changing physics results (except the one fix in §12.1), rewriting
algorithms, publishing GlueX data.

## 2. Decision log

| ID | Decision |
|---|---|
| D1 | Repo root stays `/Users/jessehernandez/phd-hadron-physics`; GitHub `jahernan1/phd-hadron-physics`, **public from first push**. |
| D2 | All current contents move into `_workdir/` (gitignored). Clean tree is built beside it by *copying* from `_workdir/`. `_workdir/` is never deleted by the refactor. |
| D3 | Layout = `packages/` (reusable) + `analyses/<channel>/` (channel-specific) + `env/ docs/ archive/ scripts/ tests/`. |
| D4 | `kpkpxim` = main thesis: selection, signal extraction, cross section, systematics, simulation, `measurements/mass`, `measurements/spin`, `backgrounds/` (π⁰K⁺K⁺Ξ⁻ and π⁺π⁻K⁺Λ channels live here). |
| D5 | `kpkpkmlamb` = excited-Ξ\* channel, own dir + README; purpose: show the framework consistently extracts rare signals. |
| D6 | `packages/xsection` = reusable cross-section library (main thesis result). |
| D7 | QFactors: fork `lan13005/QFactors` → `jahernan1/QFactors`; author's engine changes as real commits on top of upstream `d130c05`; submodule at `packages/qfactors`. Channel run configs stay in `analyses/kpkpxim/signal_extraction`. |
| D8 | JLab/upstream code (halld_sim, gluex_MCwrapper, AmpTools, HDGeant4) **never vendored**. `packages/montecarlo/external.lock` pins upstream URL + ref; author edits ship as `.patch` files with upstream-credit headers; a fetch script clones + applies. |
| D9 | The author's JLab ifarm MC area (retrieved as a local backup, upstream `.git` dirs stripped) is the *source* for extracting patches and configs. It is the author's own work and is **not named or linked** in the public repo; provenance is described only as "the author's ifarm MC area". |
| D10 | `AnalysisNote/mc_weights/` = deprecated reweighting, not in thesis → `archive/`. Real simulation = `AnalysisNote/MC` + ifarm `gen_amp/` area. |
| D11 | Active selectors: `DSelector_kpkpxim`, `_F1`, `_2017`, `_hybrid`, `DSelector_thrown_kpkpxim`, `_thrown_kpkpxim_F1`. Migrated **as separate files** with a README table; collapsing into config flags is deferred. `_legacy`, `pi0kpkpxim_1` → archive. |
| D12 | Stale/superseded code goes to `archive/` inside the live tree, with `archive/README.md` index (what, era, superseded by). |
| D13 | Environment = official JLab GlueX Apptainer image + per-run-period halld version-set XML + `env/setup.sh`. |
| D14 | Build = CMake with ROOT dictionaries for `packages/common` and `packages/xsection`. DSelectors stay ACLiC (required by `DPROOFLiteManager`). |
| D15 | Config = env vars (`GXANA_ROOT`, `GXANA_DATA`, `GXANA_OUTPUT`, `GXANA_SCRATCH`) + per-channel YAML. No absolute site paths in code. |
| D16 | Orchestration = Python CLI `gxana` with one subcommand per stage (`gxana run select --channel kpkpxim --period 2018-08 --sample data`). |
| D17 | Data policy: **code only**. No GlueX data, derived trees, yields, cross-section tables or result plots in git. Tests use inline fixtures; tests on real data read preserved data under `$GXANA_ANALYSIS_DATA` (D24) and skip when absent. |
| D18 | Rapidity/pseudorapidity branch swap in `AnalysisNote/utilities/flatTreePrep.C` is fixed **during migration** by porting gx1 `PrepFlatTrees.C` (§12.1). |
| D19 | gen_amp sampling histograms (data-derived ROOT files) stay external; `analyses/kpkpxim/simulation/inputs.lock` records name, sha256, size, JLab path. |
| D20 | Library extraction is **behavior-preserving**: unit tests in repo + golden comparison on preserved data (D24) (new vs legacy outputs) run at FSU/JLab before any dedupe of copy-paste clusters. |
| D21 | LICENSE: **MIT** for author code; upstream code keeps its own licenses (§11). |
| D22 | Tool name **`gxana`** everywhere: CLI, Python package, env vars (`GXANA_*`), C++ namespace `gxana::`, libraries `GxanaCommon`/`GxanaXsec`. Repo name stays `phd-hadron-physics`. |
| D23 | `docs/superpowers/` (execution plans) is gitignored. Legacy site paths (`/d/grid17/hjesse`, `/work/halld/home/jahernan`, …) are kept as provenance in `archive/`, `docs/history/`, this spec, the `gxana` legacy path map and git history; there is no automated public gate (author decisions 2026-09-22 and 2026-09-25). |
| D25 | Plan 3 simplifications and thesis fidelity: no `TreeHist.h`, `StackedHist.h`, `cuts.yaml`/`variations.yaml` or style-function dedupe (copies diverge; single consumers); systematics macros copied verbatim until ported onto `GxanaXsec`; the library reproduces the thesis on any ROOT by pinning ROOT-6.24 behavior (`LegacyFindBin` for flux windows, RooFit `Minimizer("Minuit","migrad")`); total-σ energy bins carry no `t_dist<2.4` cut, as in the thesis (author decision 2026-09-24). |
| D24 | Preserved analysis data (GlueX convention: code on GitHub, data under `/work/halld/gluex_analysis_data/`): `GXANA_ANALYSIS_DATA` (default `<repo>/gluex_analysis_data`, gitignored) holds golden inputs + legacy reference outputs; `analyses/<channel>/analysis_data.yaml` records path, sha256, size; `gxana data path\|status\|lock`. Replaces the `gxana toys` generator (2026-09-22). |
| D26 | Plan 4 simplifications and as-run fidelity: `gxana externals fetch\|status` (Python) replaces `fetch_externals.sh`; the lock records the sha256 of every patched file and fetch proves the tree reproduces the author's files; `compare_iters{,_2D}.C` are not merged (they diverge) and live in `analyses/kpkpxim/simulation/validation/`; the gen_amp sampling histograms are preserved data in `analysis_data.yaml` (no JLab path, replaces `inputs.lock`); `setup.sh --sim=<version-set>` renders into `$GXANA_EXTERNALS/version_sets/` (shared disk for batch jobs); one halld_sim checkout per version set; 2017-01 thesis MC uses `recon-2019_11-ver01_13`, as it ran; `runAllMC.sh` becomes `gxana run mc` + `config/mc.yaml` (author decision 2026-09-24). |
| D27 | Plan 5 as-built: fork commits made locally on `kpkpxim-thesis` from `d130c05` (pushed with Task 9) add a `QFACTORS_SETTINGS` env-var hook and a `makePlotsVars.txt` diagnostic-variable list; separate ROOT 6.40/clang portability commits drop dead includes (`RooMinuit.h`/`RooChi2Var.h`) in the engine and the then-default `configPDFs.h`, move PDF evaluation to named normalisation sets, and re-enable silent `RooRealVar` range clipping for ROOT >= 6.38 (reproducing 6.24's clipping of out-of-range `setVal`); the thesis fit models move into the fork as their own commits (author decision 2026-09-25, superseding D7's "channel run configs stay in `analyses/kpkpxim/signal_extraction`" for the fit models) in sequence — an as-run commit, then a build/run compat commit (drops the same dead includes, adds named normalisation sets, and the `chi2` argument the engine passes to `drawFitPlots`), then a commit pinning the ROOT 6.24 default `Minuit`/`migrad` minimizer in the channel models; the fork also resets `chiSqNdf_<var>` to NaN for events whose fit is not drawn and names the `fitParams_<var>` leaves without the per-process suffix; `analyses/kpkpxim` config is `config/qfactors.yaml` (not `run.yaml`, no `configSettings.h` copy); `gxana run qfactors --channel C --period P [--model M] [--steps prepare,fit,plots]` drives the fork; the golden test shows `configPDFs.h` reproduces the preserved 2017-01 thesis q-factors (neighbours exact, max \|dq\| < 1e-10) while the alternative models do not (plan and remaining choices: author decision 2026-09-24). |
| D28 | Plan 6 as-built: `analyses/kpkpkmlamb` = config (periods ana55/ana22/ana19, fit prefix `B4_M18_`, tree dirs `Trees/kpkpkmlamb/tree_{stem}/{kind}/`, data only), `selectors/`, `flat_trees/flatTreePrep.C` (writes `$GXANA_DATA/kpkpkmlamb/`), `measurements/FitXimStar.C(n_threads, tCut)` merging `FitXimStarCuts.C` (tCut = the saved `t_dist>1` selection); legacy `get_data_hists.C` was a kpkpxim macro → `archive/root_macros/`. Release: MIT `LICENSE`, `NOTICE.md`, `CITATION.cff` (author-filled fields), version 1.0.0; commit metadata keeps the author's email (author decisions 2026-09-25). |

## 3. Target layout

```
phd-hadron-physics/
  README.md                 physics goal, pipeline diagram, quickstart
  LICENSE                   MIT, author code only (D21)
  NOTICE.md                 upstream credits + licenses (§11)
  CITATION.cff
  CMakeLists.txt            top-level; add_subdirectory(packages/common, packages/xsection, analyses/kpkpxim/xsection)
  pyproject.toml            python package `gxana` (src in packages/common/python)
  .gitignore  .gitmodules
  env/
    README.md
    setup.sh                exports GXANA_*; sources gluex env for a run period
    site.example.sh         template; real env/site.sh is gitignored
    apptainer/gxana.def       FROM JLab gluex image; adds cmake, python deps
    version_sets/           recon-*.xml.in sim version-set templates, halld_sim/MCwrapper home= ${GXANA_EXTERNALS}
  packages/
    common/
      CMakeLists.txt  LinkDef.h
      include/gxana/common/{Style.h,Strings.h,Paths.h,GraphIO.h}
      src/*.cxx
      python/gxana/           config.py paths.py cli.py doctor.py analysis_data.py stages/select.py
      scripts/              add_hists.sh clean_proof.sh
      tests/                python (pytest) + C++ (ctest) tests
    xsection/
      CMakeLists.txt  LinkDef.h  README.md
      include/gxana/xsection/{Binning.h,YieldFit.h,XSec.h,Flux.h,Plotting.h,Barlow.h}
      src/*.cxx
      python/gxana_xsection/  weighted_average.py components.py tex_table.py qvalue_rescale.py
      tests/
    qfactors/               git submodule → jahernan1/QFactors (branch kpkpxim-thesis; fork incl. thesis fit models configPDFs*.h)
    montecarlo/
      README.md  NOTICE.md
      external.lock         YAML: name, url, ref, sha, fetch, patches[], files{path: sha256}
      patches/halld_sim/0001..0005-*.patch
      patches/gluex_MCwrapper/0001..0002-*.patch
      scripts/build_halld_sim.sh  run_hdroot.py
      tests/
  analyses/
    kpkpxim/
      README.md
      config/               channel.yaml periods.yaml samples.yaml binning.yaml xsection.yaml mc.yaml qfactors.yaml
      selectors/            DSelector_kpkpxim{,_F1,_2017,_hybrid}.{C,h}, DSelector_thrown_kpkpxim{,_F1}.{C,h}, README.md
      backgrounds/          selectors/ (pi0kpkpxim, pippimkplamb + thrown), KstarFit.C, YstarBWFitsData.C
      selection/            flatTreePrep.C (rapidity fixed), flatTreePrepQVal.C, CutAnalysis.C, CutAnalysisRF.C, cut_studies/<cut>/
      signal_extraction/    qfactors/ (README.md, scripts/), lineshape/
      xsection/             CMakeLists.txt, make_binned_trees.cxx, make_xsec_fit_variations.cxx, plotting macros, flux/getFlux.sh, external_data/Clas_data.csv
      systematics/          variation trees, barlow/, comparisons/, track_efficiency/, mc_weight_variations/
      simulation/           gen_amp_cfg/ mcwrapper/ hd_root/ genr8/ sampling/ validation/ local_beam.conf README.md
      measurements/mass/    Xim1320Properties.{h,cpp}, MakeXim1320_IM*.C, MakeXim1820_IM.C
      measurements/spin/    PlotGlueXSpin.C
    kpkpkmlamb/
      README.md  config/  selectors/  flat_trees/flatTreePrep.C  measurements/FitXimStar.C
  archive/
    README.md               index table
    mc_weights/ xsection_old/ xsection_legacy/ systematics_legacy/ selectors/ root_macros/ mc_legacy/ gx1_export/ env_fsu/
  docs/
    REFACTOR_SPEC.md  pipeline.md  environment.md  KNOWN_ISSUES.md
    history/PROJECT_REVIEW.md  history/REFACTOR_PLAN.md
    superpowers/plans/      gitignored (D23): local execution plans
  tests/                    golden tests on preserved data (tests/golden)
  _workdir/                 gitignored; everything that existed before the refactor
  gluex_analysis_data/      gitignored; preserved data (D24), default $GXANA_ANALYSIS_DATA
```

## 4. Packages

### 4.1 `packages/common`

C++ library `GxanaCommon` (namespace `gxana`) + Python package `gxana`.

| Unit | Content | Source |
|---|---|---|
| `Style.h` | `void gxana::SetStyle()` — body verbatim from `AnalysisNote/xsection/PlotFunctions.cpp:4-80` | PlotFunctions.cpp (≈60 copies of style funcs elsewhere converge here later) |
| `Strings.h` | `bool gxana::NumericCompare(const std::string&, const std::string&)` verbatim (`PlotFunctions.cpp:82-100`); `SplitString` from MakeXSec.C not ported (its only callers, MakeXSec.C and MakeXSecComponents.C, are archived) | PlotFunctions.cpp, MakeXSec.C |
| `Paths.h` | `std::string gxana::EnvPath(const std::string& var, const std::string& rel = "")` — throws `std::runtime_error` if `var` unset | new |
| ~~`TreeHist.h`, `StackedHist.h`~~ | dropped in Plan 3: the 48 `save_from_flattrees` copies have 18 signatures and the 11 `MakeStackedHist` bodies all differ, so each stays local to its macro | D25 |
| `GraphIO.h` | `GetAllTGraphErrors(const char*)` (9 copies), `CreateTGraphErrorsFromTxt` | Plan 2 |

Python `gxana`:
- `gxana.paths` — resolve `GXANA_*` env vars; `legacy_to_env(path)` maps legacy prefixes (§7.3).
- `gxana.config` — load + merge `analyses/<channel>/config/*.yaml`, expand `${GXANA_*}`.
- `gxana.cli` — entry point `gxana` (argparse): `doctor`, `config show`, `run select`, `run xsection` (Plan 3), `data path|status|lock`. Later plans add `run systematics|qfactors|mc`, `fetch-inputs`.
- `gxana.stages.select` — Python port of `runDSelector.sh` (§9).
- `gxana.publiccheck` — public-release gate (§13). (DROPPED by user 2026-09-22.)
- `gxana.analysis_data` — preserved-data manifest (`analyses/<channel>/analysis_data.yaml`), status and sha256 lock (D24).

Python deps: stdlib + `pyyaml`. `numpy`/`pandas` only in `gxana_xsection`. PyROOT is **not** imported by `gxana` (host PyROOT is bound to Python 3.9; container differs) — ROOT work is done by shelling out to `root`.

### 4.2 `packages/xsection` (Plan 2)

C++ library `GxanaXsec` built from the existing seeds; API kept signature-compatible first, cleaned later:

| Header | Functions (initial, verbatim signatures) | Source |
|---|---|---|
| `Binning.h` | `divideNominalIntoBins(...)`, `divideThrownIntoBins(...)`; `SplitVariationTrees` merged in | `xsection/MakeBinnedTrees.{h,cpp}`, `systematics/SplitVariationTrees.C` |
| `YieldFit.h` | `RooFitMC`, `RooFitData`, `AttemptFit`, `AttemptFitMC`, `constructFitString`, `constructFitStringData` (Gaussian/Johnson/Voigtian + Chebychev) | `xsection/FitFunctions.{h,cpp}` |
| `XSec.h` | `GetDiffXSecFile`, `GetTotXSecFile`; `MakeBinnedDiffXSec`, `calc_weightedavg`, `calc_totalxsec` not ported (callers archived; run-period averaging done in Python `gxana_xsection.weighted_average`) | FitFunctions.cpp, MakeXSec.C |
| `Flux.h` | `GetFluxHist(std::string)` | FitFunctions.cpp |
| `Plotting.h` | Plan 3: `plotDiffXSec`, `plotWeightedXSec`, `plotOneWeightedXSec`, `plotFinalWeightedXSec` with `SetPlotDir`; `GetPointwiseMeanAndStdDev` stays local (absent from 3 of 7 comparison macros) | PlotFunctions.cpp, Plot*Comparison.C |
| `Barlow.h` | `calc_barlow`, `calculateStdDevGraph`; `plotDiffXSecAndBarlow`, `plotTotXSecAndBarlow` moved to Plan 3 | systematics/PlotXSecBarlow*.C, GetBarlowResults.C |

Side effect to remove: `CreateTGraphErrorsFromTxt` writes ROOT files into cwd → take explicit output path.
Python `gxana_xsection`: `calculate_weighted_average` (run-period error-weighted mean), component split, Q-value rescale, one `tex_table` with options replacing the three `MakeXsecTexTable*.py`.

### 4.3 `packages/qfactors` (Plan 5)

Submodule of `jahernan1/QFactors`, branch `kpkpxim-thesis`, commits on top of `d130c05`. Planned (design intent):
1. `.gitignore` (logs/, histograms/, diagnosticPlots/, main, *.so, *.d); `#!/usr/bin/env python3` shebang on `run.py` and `repeatProgressChecks.py` (prevents the ImageMagick `import` screen-grab junk `os`/`sys`/`time`/`subprocess`).
2. `main.C`: per-event `chiSqNdf_<var>` + `fitParams_<var>` branches, `paramLog<i>.txt`, NaN q→0. Parametrize the Johnson-specific paramLog header; replace VLA `float vecParams[numParams]` with `std::vector<float>`.
3. `drawPlots.h`: `draw1DPlots` chi² out-param + stat box.
4. `mergeQresults.C`: carry `chiSqNdf_*`.
5. `makePlots.C`: NaN counter, total signal sum (**initialize `qvalueSum = 0`**), diagnostic var list from config instead of hardcoded.
6. `run.py`: optional `termcolor`, sleep tweak.
Fork README: "Fork of lan13005/QFactors; changes listed in CHANGES_THESIS.md; all original authorship retained." As built, the run config is `analyses/kpkpxim/config/qfactors.yaml` (`_SET_*` settings, input paths, model choice); the fit models `configPDFs*.h` live in the fork itself (see As built below); `configSettings.h` is rendered by `gxana` from the fork's template, not copied by hand; helper `scripts/*.C` live in `analyses/kpkpxim/signal_extraction/qfactors/scripts/`.

**As built (Plan 5, D27).** Fork commits on `kpkpxim-thesis` from `d130c05`, in order: "Ignore run outputs; run the drivers with python3"; "run.py: optional termcolor, QFACTORS_SETTINGS file, longer progress-check delay"; "Build with ROOT 6.40 and clang"; "Save per-event fit chi2/ndf and fit parameters"; "mergeQresults: carry chiSqNdf into the postQVal tree"; "makePlots: variable list file, NaN counter, signal sum, branch-type fix"; "Document the fork: README notice and CHANGES_THESIS.md"; "Evaluate PDFs with named normalisation sets for ROOT 6.40"; "Add the kpkpxim thesis fit models (as run)"; "Make the fit models build and run with the current engine and ROOT 6.40"; "Pin the ROOT 6.24 default minimizer in the fit models"; "Keep RooRealVar range clipping on ROOT >= 6.38"; "Name fit-parameter leaves without the process suffix" (13 commits total). `gxana run qfactors --channel C --period P [--model M] [--steps prepare,fit,plots]` drives the fork end to end. The golden test on the preserved 2017-01 data identifies `configPDFs.h` as the thesis model: it reproduces the preserved q-factors (neighbours exact, max |dq| < 1e-10), while the alternative models (`configPDFs_{Johnson,JohnsonGaus,Gaussian}.h`) do not.

### 4.4 `packages/montecarlo` (Plan 4)

`external.lock` (YAML):

| name | url | ref | sha | patches |
|---|---|---|---|---|
| halld_sim | https://github.com/JeffersonLab/halld_sim | `4.54.0` | `bcff7a5c4e8453b1e75e83facdc6f2ad95ba5719` | 0001-Hist2D-add-CosTheta-histTypes, 0002-AMPTOOLS_AMPS-add-Hist3D-amplitude, 0003-gen_amp_V2-register-Hist3D-lvRange-costheta-diagnostics, 0004-gen_amp-force-KPlus-for-suffixed-particle, 0005-mcsmear-FCALSmearer-revert-c56a7e52 |
| gluex_MCwrapper | https://github.com/JeffersonLab/gluex_MCwrapper | `c4e918a2` | `c4e918a2fe0e48bdf67f69326ed1820458f26b9e` | 0001-add-UPPER-LOWER_VERTEX_INDICES, 0002-geant4-force-Lambda-to-p-pim (also fixes `>>!` in MakeMC.sh) |
| AmpTools | https://github.com/mashephe/AmpTools | `v0.15.2` | `fed18194954f0857f4e5a22f8c51940285b26237` | none |
| HDGeant4 | https://github.com/JeffersonLab/HDGeant4 | `2.42.0` (from version sets) | `e09d41fad1801f28667e92086306173ee1f50075` | none |

Patch sources: the ifarm area's `my_halld_sim/halld_sim-2019_11-ver01_13` (all four period dirs have identical source) vs upstream 4.54.0; its `gluex_MCwrapper` vs `c4e918a2`. Drop orphan `Hist2D_MultiPart.cc`. Each patch header: `From:` author, `Subject:`, and `Upstream: <repo>@<sha>; upstream copyright retained`.
Local `AnalysisNote/MC/halld_sim` (4.47.0-17, unmodified) and `AnalysisNote/MC/gluex_MCwrapper` (2.5.0) are **stale and unused** → not migrated (stay in `_workdir/`); `set_gluexenv.sh` → `archive/env_fsu/`.
Plan 4 (D26): AmpTools and HDGeant4 are `fetch: false` (provided by the version sets). Actual patch file names are listed in `external.lock`. `gxana externals fetch [NAME] [--dest DIR]` clones at the sha, runs `git am` on the patches and verifies the locked sha256s; `gxana externals status` reports ok/diff/miss/pin. `scripts/build_halld_sim.sh <set>` fetches into `$GXANA_EXTERNALS/halld_sim-<set>` and scons-builds it in the sim env; `scripts/run_hdroot.py` is generalized from `AnalysisNote/MC/run_hdroot.py`.

## 5. Analyses

### 5.1 `analyses/kpkpxim` migration map (abridged; full per-file map in Plan 3)

| Destination | Sources |
|---|---|
| `selectors/` | `DSelector/DSelector_kpkpxim{,_F1,_2017,_hybrid}.*`, `DSelector_thrown_kpkpxim{,_F1}.*` |
| `backgrounds/selectors/` | `DSelector_pi0kpkpxim.*`, `DSelector_pippimkplamb.*`, `DSelector_thrown_pippimkplamb.*` |
| `backgrounds/` | `utilities/KstarFit.C`, `utilities/YstarBWFitsData.C` |
| `selection/` | gx1 `PrepFlatTrees.C` → `flatTreePrep.C` (D18); `utilities/flatTreePrepQVal.C`, `flatTreeCuts{,MC}.C`, `flatTreePlots.C`, `CompareFromTree.C`; `analysis/event_selection/CutAnalysis{,RF}.C`; `analysis/event_selection/<cut>/` → `cut_studies/<cut>/` (minus ROOT-generated `c_format_plots/`); `analysis/GetKinematicsDataMC{,_RF}.C`; `analysis/analysis/mc_studies/*`; `analysis/event_selection/qfactors/XimMassQVal.C` |
| `signal_extraction/` | QFactors run config (§4.3); `xsection/OneUMLFit.C`, `SingleGaussianFit.C`, `DoubleGaussianFit.C` → `lineshape/` |
| `xsection/` | `MakeBinnedTrees.C` main, `MakeXSecFitVariations.C` main (gx1 name `MakeXSection.C`), `GetWeightedXsecFile.py`/`GetXSecComponentFiles.py`/`MakeQValXSecFile.py`/`MakeXsecTexTable*.py` `__main__` drivers, `MakeWeightedDiffXSecTGraphs.C`, `PlotDiffXSec.C`, `PlotComponents.C` (rename clashing `PlotDiffXSec()`), `PlotXSecComponents.C`, `PlotTotXsecWithClas.C`; `fluxFiles/getFlux.sh` → `flux/`; `Clas_data.csv` → `external_data/` (published CLAS data; cite source in README) |
| `systematics/` | `GetVariationTreesUML.C`, `SplitVariationTrees.C` main, `GetXSecFilesUML.C`+`run.sh`, `GetXSecFitVariations.C`, `GetWeightedXsecFile.py`, `PlotXSecBarlow*.C`+`PlotAllVariations.sh` → `barlow/`, `xsection/Plot{Fit,Combo,Run,Bunch,QValue,RestV,FitBkgd}Comparison.C` → `comparisons/`, `GetRunPeriodPctSig.C`, `GetBarlowResults.C`, `combine_pdf.sh`, `track_efficiency/{get_hists,get_track_efficiency}.C`, `mc_weight_variations/*` |
| `simulation/` | ifarm `gen_amp/gen_amp_cfg/*.cfg` → `gen_amp_cfg/`; `gen_amp/xim_jlab_MC_gen_amp_*.conf` → `mcwrapper/`; `gen_amp/hd_root_files/*.conf` + local `hd_root_xim13.config` → `hd_root/`; `runAllMC.sh` → `run_all_mc.sh`; local `local_beam.conf`; local `getHist2D_gen_amp.C`, `getHist3D{,_F18}.C` → `sampling/`; local `tree_3d/*.C` → `validation/`; genr8 `Xi_1320*.input` → `genr8/` (legacy, documented) |
| `measurements/mass/` | `utilities/Xim1320Properties.{h,cpp}` (fix header/impl mismatch), `PlotXim1320Properties.C`, `MakeXim1320_IM{,_Res}.C`, `MakeXim1820_IM.C`; `analysis/analysis/cascade_properties/GetXimProperties.C` (reconcile vs Xim1320Properties.cpp) |
| `measurements/spin/` | `analysis/analysis/cascade_properties/PlotGlueXSpin.C` (+ `GetSpinAnalysis` split out of Xim1320Properties.cpp) |

gx1-only improvements ported: rapidity fix (D18), `XSecFunctions` rename with fixed include. AnalysisNote wins where it is newer (`mc_studies/get_data_hists_RF.C`, `make_plot_RF.C`, `chisqndf_cut/get_data_hists_RF.C`). gx1 `barlow_systematics/GetWeightedXSecFiles.py` diverged → diff reviewed in Plan 3, newer logic kept.

### 5.2 `analyses/kpkpkmlamb`

`DSelector/kpkpkmlamb/DSelector_kpkpkmlamb.{C,h}` → `selectors/`; `kpkpkmlamb/flatTreePrep.C` → `flat_trees/`; `get_data_hists.C`; `FitXimStar.C` + `FitXimStarCuts.C` merged (`bool tCut` argument) → `measurements/`. Config: periods `2017-01_ana55`, `2018-01_ana22`, `2018-08_ana19`, fit prefix `B4_M18_`. README: physics motivation (Ξ(1690)/Ξ(1820) → K⁻Λ), which kpkpxim pipeline decisions were reused, results pointer (thesis chapter). As built: see D28.

## 6. Archive (`archive/`)

| Dir | Content | Superseded by |
|---|---|---|
| `mc_weights/` | `AnalysisNote/mc_weights/**` incl. `archive/` (genr8 scheme) | not used in thesis |
| `xsection_old/` | `AnalysisNote/xsection_old/**` | `packages/xsection` |
| `xsection_legacy/` | `MakeXSec.C`, `MakeXSecCompare.C`, `MakeXSecComponents.C`, `MakeCutComparison.C`, `MakeComponentComparison.C`, `RunXSec.py`, `MakeXSecFiles.C`, `MakeXSecFit{Bkgd,SingleGaus,DoubleGaus,Voigtian,MC}.C`, `FitFunctions_bak.cpp`, `RooHistPdfFitTest{,_1}.C` | FitFunctions + MakeXSecFitVariations |
| `systematics_legacy/` | `GetVariationTrees.C`, `GetXSecFiles.C`, `GetDiffXSec.C`, `GetRunPeriodComparison.C`, `TestUMLRecursive{,_1}.C`, `get_track_efficieny.C`, `data_files/`-era scripts | UML pipeline |
| `selectors/` | `DSelector_kpkpxim_legacy.C`, `DSelector_pi0kpkpxim_1.C` | main selectors |
| `root_macros/` | `MakeHistos.C`, `MakeHistoQVal.C`, `PlotfromFlatTree{,MC}.C`, `analysis/CutAnalysis.C` (old draft), `flatTreePrepQVal_old.C`, `MakeXim1320_IM_Volker.C`, `AcceptanceCorrect.C`, `lambda_vertex_cut/old/` | AnalysisNote pipeline |
| `mc_legacy/` | early gen_amp cfgs, `genr8/` (non-Ξ inputs), `ystar_inputs/`, `MC.config`, `xim_jlab_MC.config`, `exampleHist2D.C`, `getHist2D{,_s17_v3,_test}.C`, `version.xml` (FSU 5.12.0) | `analyses/kpkpxim/simulation` |
| `gx1_export/` | files of the earlier gx1 export that are not byte-identical to any AnalysisNote/DSelector/migrated file (Plan 3) | this repo |
| `env_fsu/` | `set_gluexenv.sh`, FSU container alias notes | `env/` |

Never migrated (stay in `_workdir/` only): dotfiles, `temp/`, `Trees/`, empty dirs, all build artifacts, editor junk, stray `C` ls-dumps, `bins.txt`, `tmp.cfg` (ROOT binary), ROOT-generated `c_format_plots/*.C`, local halld_sim/gluex_MCwrapper clones, QFactors junk (`main`, `os`, `sys`, `time`, `subprocess`), tmux scripts, `switchgridname.sh`, all text outputs (`*.txt` data, `*.out`, `*.tex`, `output.csv`).

## 7. Environment

### 7.1 Container
`env/apptainer/gxana.def`: `Bootstrap: docker`, `From: jeffersonlab/gluex_almalinux_9:gxi2.40@sha256:99ed43152dab0932328fc46cf215362f529fdd2758233e2d2beb8c25da5c4670` (verified on Docker Hub 2026-09-22); `%post` installs `cmake>=3.20`, `python3-pyyaml`, `python3-pytest`. The GlueX software itself comes from `/group/halld` (CVMFS bind) via `gxenv`, exactly as on ifarm. At JLab/OSG the same image is available under `/cvmfs/singularity.opensciencegrid.org/jeffersonlab/`. CI (no CVMFS) uses `rootproject/root:6.24.06-ubuntu20.04` to build/test the packages only. Thesis-era FSU image (`gluex_centos-7.7.1908_sng3.8_gxi2.20.sif`) documented in `archive/env_fsu/`.

### 7.2 Version sets
Two environments, because the analysis code needs ROOT ≥ 6.20 (`RooJohnson`) while the MC production sets ship ROOT 6.08.06:

- **analysis** (selectors, selection, Q-factors, xsection, systematics): upstream halld version set **5.12.0** — ROOT 6.24.04, gluex_root_analysis 1.25.0, amptools 0.15.1 (matches `AnalysisNote/MC/version.xml`; FSU runs used `gxenv /d/grid13/sdobbs/GlueX/eeg/version.xml`, whose content is to be confirmed identical at FSU). Used unmodified from `$HALLD_VERSIONS/version_5.12.0.xml`; nothing copied into the repo.
- **sim** (per run period, Plan 4): `env/version_sets/*.xml.in` templates of the recon sets below, with `halld_sim` `home="${GXANA_EXTERNALS}/halld_sim-<set>"` and `gluex_MCwrapper` `home="${GXANA_EXTERNALS}/gluex_MCwrapper"`; `source env/setup.sh --sim=<set>` renders them into `$GXANA_EXTERNALS/version_sets/` before `gxenv` (gxenv does not expand env vars; batch jobs read the file, so it sits on shared disk). Period → set lives in `analyses/kpkpxim/config/mc.yaml`.

| Period | analysis env | recon/sim env | hdgeant4 | amptools | root |
|---|---|---|---|---|---|
| 2017-01 | analysis-2017_01-ver56 (alt ver45) | recon-2019_11-ver01_13 (thesis MC, as run; recon-2017_01-ver03_40 for the pre-thesis ana45 MC) | 2.42.0 | 0.15.2 | 6.08.06 |
| 2018-01 | analysis-2018_01-ver03 | recon-2018_01-ver02_32 | 2.42.0 | 0.15.2 | 6.08.06 |
| 2018-08 | analysis-2018_08-ver02 | recon-2018_08-ver02_31 | 2.42.0 | 0.15.2 | 6.08.06 |

`source env/setup.sh [--gluex] [--sim PERIOD]`: sources `env/site.sh` (if present), exports `GXANA_ROOT` (repo dir), defaults `GXANA_DATA`, `GXANA_OUTPUT`, `GXANA_SCRATCH`, `GXANA_EXTERNALS`; `--gluex` sources `/group/halld/Software/build_scripts/gluex_env_boot_jlab.sh` + `gxenv $HALLD_VERSIONS/version_5.12.0.xml`; `--sim=<set>` (Plan 4; exclusive with `--gluex`) renders + gxenv's that recon template and exports `GXANA_SIM_VERSION_SET`; always prepends `$GXANA_ROOT/build/lib` to `LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH` and `$GXANA_ROOT/packages/common/python` to `PYTHONPATH`.

### 7.3 Env vars and legacy path map

| Var | Meaning | FSU value (site.sh) |
|---|---|---|
| `GXANA_ROOT` | repo checkout | set by setup.sh |
| `GXANA_DATA` | input trees/flat trees root | `/d/grid17/hjesse` |
| `GXANA_OUTPUT` | stage outputs root | `/d/grid17/hjesse/gxana_output` |
| `GXANA_SCRATCH` | PROOF sandbox, temp | `/d/grid17/hjesse/temp` |
| `GXANA_EXTERNALS` | fetched+patched upstream builds | `/d/grid17/hjesse/externals` |

| Legacy prefix | Replacement |
|---|---|
| `/d/grid17/hjesse/kpkpkmlamb/` | `${GXANA_DATA}/kpkpkmlamb/` |
| `/d/grid17/hjesse/Trees/` | `${GXANA_DATA}/Trees/` |
| `/d/grid17/hjesse/AnalysisNote/flatTrees/` | `${GXANA_DATA}/flatTrees/` |
| `/d/grid17/hjesse/AnalysisNote/QFactors/logs/` | `${GXANA_OUTPUT}/kpkpxim/qfactors/` |
| `/d/grid17/hjesse/AnalysisNote/<stage>/` | `${GXANA_OUTPUT}/kpkpxim/<stage>/` |
| `/d/grid17/hjesse/AnalysisNote/fluxFiles/` | `${GXANA_DATA}/flux/` |
| `/d/grid17/hjesse/analysis/kpkpxim/` | `${GXANA_OUTPUT}/kpkpxim/selector_hists/` |
| `/d/grid17/hjesse/analysis/` | `${GXANA_OUTPUT}/kpkpxim/selector_hists/` |
| `/d/grid17/hjesse/macros/` | `${GXANA_OUTPUT}/legacy_macros/` |
| `/d/grid17/hjesse/temp` | `${GXANA_SCRATCH}` |
| `/d/grid17/hjesse/Clas_data.csv` | `${GXANA_ROOT}/analyses/kpkpxim/xsection/external_data/Clas_data.csv` |
| `/work/halld/home/jahernan/`, `/w/halld-scshelf2101/home/jahernan/` | `${GXANA_EXTERNALS}` / `${GXANA_OUTPUT}` (per file) |

## 8. Configuration schema (`analyses/<channel>/config/`)

```yaml
# channel.yaml
channel: kpkpxim
reaction: kpkpxim
selector_dir: analyses/kpkpxim/selectors
default_selector: DSelector_kpkpxim.C
thrown_selector: DSelector_thrown_kpkpxim.C
output_basename: kpkpxim.root          # must equal selector dOutputFileName
```
```yaml
# periods.yaml
periods:
  2017-01: {launch: ana56, fit_prefix: "M23_",    label: sp17, runs: [30274, 31057]}
  2018-01: {launch: ana03, fit_prefix: "B4_M23_", label: sp18, runs: [40856, 42559]}
  2018-08: {launch: ana02, fit_prefix: "B4_M23_", label: fa18, runs: [50685, 51768]}
```
```yaml
# samples.yaml — replaces runMultiDSelector.sh
samples:
  data:                     {mc: false}
  gen_amp_V2_ac_YstarRest:  {mc: true}
  gen_amp_V2_noac_YstarRest:{mc: true}
  F1:                       {mc: false, fit_prefix: "B4_F1_M23_", selector: DSelector_kpkpxim_F1.C, output_basename: kpkpxim_F1.root}
tree_dir_template: "Trees/tree_{stem}/{kind}/"
# mc_suffix = "" for data else "_" + sample; kind = trees | thrown
```
```yaml
# binning.yaml
energy_edges: [6.40, 7.40, 7.86, 8.19, 8.45, 8.68, 9.26, 10.18, 11.40]
t_bins: [[0.10,0.35],[0.35,0.53],[0.53,0.71],[0.71,0.92],[0.92,1.19],[1.19,1.53],[1.53,2.40]]
total_energy_range: [6.4, 11.4]
```
stem = `<reaction>__<fit_prefix><period>_<launch><tree_suffix>` (`gxana.config.tree_stem`; `tree_suffix` defaults to `_<sample>` for MC); periods also carry `flux: <file>` under `$GXANA_DATA/flux/`. `xsection.yaml` (Plan 3) lists the MC sample, weight, input templates (`${GXANA_*}` expanded), fits (model, params, ordered labels with Chebychev order), weighted/component labels and `qvalue_label`. Nominal cuts and the 18 Barlow variations stay in `selection/flatTreePrep.C` / `systematics/GetVariationTreesUML.C` and are listed in `analyses/kpkpxim/README.md` (D25).

## 9. CLI `gxana`

```
gxana doctor                                   # env vars, root/rootls/hadd, ROOT_ANALYSIS_HOME, gxenv, python deps
gxana config show --channel kpkpxim            # merged YAML, env-expanded
gxana run select --channel C --period P --sample S [--thrown] [--tag T] [--cores N] [--selector F] [--dry-run]
gxana check-public [PATH...]                   # (DROPPED by user 2026-09-22) release gate (§13); exit 1 on violation
gxana data status --channel kpkpxim           # preserved data vs manifest (D24)
gxana run xsection --channel C [--steps bin,tables,weight,components,qvalue] [--dry-run]
gxana run mc --channel C --period P --sample S [--dry-run]   # Plan 4: render MCwrapper inputs, submit gluex_MC.py
gxana externals fetch|status [NAME...] [--dest DIR]          # Plan 4: pinned upstreams + patches
gxana run qfactors --channel C --period P [--model M] [--steps prepare,fit,plots] [--dry-run]   # Plan 5
# later plans: run systematics
```

`run select` reproduces `runDSelector.sh` exactly, minus hardcoded paths:
- tree dir from `tree_dir_template`; tree name = the single key of first `*.root` file (error if ≠1 key);
- save name = tree-dir parent basename minus `tree_` prefix, plus `_<tag>`;
- deletes stale `{,thrown_,flatTree_,flatTree_thrown_}<output_basename>` in the run dir (`$GXANA_SCRATCH/run/<save>`), not repo root;
- ROOT heredoc: `gEnv->SetValue("ProofLite.Sandbox","$GXANA_SCRATCH/proof")`, `.x $ROOT_ANALYSIS_HOME/scripts/Load_DSelector.C`, `TChain`, `DPROOFLiteManager::Process_Chain(ch,"<selector>++",N)`;
- moves outputs: hist file → `$GXANA_OUTPUT/<channel>/selector_hists/[thrown_]<save>.root`; flat tree → `$GXANA_DATA/Trees/flatTree/rawTrees/flatTree_[thrown_]<save>.root`.
Bugs in `runMultiDSelector.sh` (`-S`, `-s <name>`, `-c 16-s`, `./run DSelector.sh`) disappear because samples come from YAML.

## 10. Build

Top-level CMake ≥ 3.20, C++17, `find_package(ROOT REQUIRED COMPONENTS RIO Tree Hist Gpad Graf RooFit RooFitCore ROOTDataFrame)`. Per package: `add_library(GxanaCommon SHARED ...)`, `ROOT_GENERATE_DICTIONARY(G__GxanaCommon <headers> MODULE GxanaCommon LINKDEF LinkDef.h)`. Outputs `build/lib/libGxanaCommon.so` + `.pcm` + `.rootmap`. `rootlogon.C` at repo root: `gSystem->Load("libGxanaCommon")`, `gInterpreter->AddIncludePath("$GXANA_ROOT/packages/common/include")`. Tests: `enable_testing()` + plain `add_test` executables (no gtest dependency). Existing `compile_lib.sh`/`compile_code.txt` retired.

## 11. Credit & licensing

`NOTICE.md` lists: JeffersonLab/halld_sim, JeffersonLab/gluex_MCwrapper, JeffersonLab/HDGeant4, mashephe/AmpTools, lan13005/QFactors, JeffersonLab gluex_root_analysis (DSelector template, `DPROOFLiteManager`), JLab halld version sets; each with URL, pinned ref, license as stated upstream, and "not redistributed here; patches in `packages/montecarlo/patches` are modifications by J. Hernandez to the listed upstream revision". DSelector files keep their template header comment. Provenance of the patches/configs is stated as "the author's JLab ifarm MC area" (D9).

## 12. Known issues handling

1. **Rapidity swap (D18)** — AnalysisNote `flatTreePrep.C` defines `kphigh/kplow/ystar_rapidity = atanh(pz/p)` (pseudorapidity) and `*_prapidity = .Rapidity()`. gx1 `PrepFlatTrees.C` is correct and becomes the migrated file. `docs/KNOWN_ISSUES.md` records: which outputs were produced with the swapped definition (any cut on `*_rapidity`, incl. `kphighrap` nominal selection and `KHighRapidity`/`KLowRapidity` Barlow variations) and that results must be regenerated to confirm impact.
2. Documented, not fixed during migration: per-combo `cout` spam in `DSelector_kpkpxim.C::Process`; PID ΔT and Ξ mass-window cuts commented out in selector (applied downstream); `Xim1320Properties.h` ≠ `.cpp`; `PlotComponents.C` defines `PlotDiffXSec()` (name clash — renamed on migration); `GetBarlowResults.C` reads nonexistent `xsection/data_files`; `MakeHistoQVal.C` defines `MakeHistos()`.
3. Fixed in their own package plans: QFactors `qvalueSum` uninitialized, VLA; MCwrapper `MakeMC.sh` `>>!` (QFactors items fixed in Plan 5; MCwrapper in Plan 4).

## 13. Public-release gate (dropped)

Dropped by the author (2026-09-22); the pre-push check is a manual review of `git ls-files` and a fresh-clone build (Plan 6). Original design kept below for reference.

`gxana check-public` (and `.git/hooks/pre-commit` calling it on staged files) fails on:
- content matching: `/d/grid1[37]/`, `/work/halld/home/`, `/w/halld-scshelf`, `/home/(ln16|lawrence|tbritton)`, `hjesse@`, `jahernan@`, `10\.0\.0\.\d+`, `scigrid\d`, `LAPTOP-`, `-----BEGIN .*PRIVATE KEY`, `(ghp|gho|github_pat)_[A-Za-z0-9_]{20,}`;
- files: extensions `.root .hddm .so .o .d .pcm .a .pdf .png .eps .ps .svg .evio`, ELF/Mach-O magic, size > 1 MiB, names `*~`, `#*#`, `.#*`, dotfiles from `$HOME` (`.bash_history`, `.Xauthority`, `.esd_auth`, `.viminfo`, `.root_hist`);
- private deny-list `.public-deny.local` (gitignored, one regex per line; rule `private-ref`) for names that must never appear publicly;
- allowlist `.public-allow` (glob per line) for deliberate exceptions (e.g. `docs/img/*.svg`, the check's own test fixtures).
First push only after `gxana check-public` passes on the whole tree **and** a manual review of `git ls-files`.

## 14. Implementation plans (sequenced)

| # | Plan | Delivers | Depends on |
|---|---|---|---|
| 1 | Foundation & common library | `_workdir` move, skeleton, .gitignore, `gxana` Python pkg (paths, config, doctor, check-public, `run select`), CMake + `GxanaCommon` (Style, Strings/NumericCompare, Paths), env/ (setup.sh, site.example.sh, gxana.def, rootlogon.C), pre-commit hook, CI | — |
| 2 | xsection package | `GxanaXsec` + `gxana_xsection` from seeds; `GXANA_ANALYSIS_DATA` + `gxana data`; `GraphIO`; golden tests (`tests/golden`, `gxana_xsection.compare`); plotting and `tex_table` moved to Plan 3 | 1 |
| 3 | kpkpxim analysis migration (done 2026-09-24) | §5.1 moves with `scripts/migrate_paths.py` + guard tests, rapidity fix, `gxana run xsection` + `xsection.yaml`, `Plotting.h`, `tex_table`, thesis-fidelity pins (D25), golden tests on the full preserved data, archive/ (mc_legacy deferred to Plan 4) | 1, 2 |
| 4 | montecarlo package (done 2026-09-24) | external.lock + 7 patches with per-file sha256, `gxana externals`, sim version-set templates + `setup.sh --sim`, `build_halld_sim.sh`, `run_hdroot.py`, `analyses/kpkpxim/simulation/` + `config/mc.yaml`, `gxana run mc`, sampling histograms in the preserved data, `archive/mc_legacy/` (D26) | 1 |
| 5 | QFactors fork (done 2026-09-25 except publishing the fork: Task 9) | fork commits on d130c05 (engine changes, thesis fit models, ROOT 6.40 portability), submodule, config/qfactors.yaml, gxana run qfactors, toy + golden tests (D27) | 1 |
| 6 | kpkpkmlamb + docs + release (done 2026-09-25 except publishing, which the author runs) | side channel, LICENSE/NOTICE/CITATION, README, KNOWN_ISSUES, fresh-clone check, publish | 1–5 |

Outward-facing actions (creating `jahernan1/QFactors` fork, creating/pushing `jahernan1/phd-hadron-physics`) require explicit author confirmation at execution time.
