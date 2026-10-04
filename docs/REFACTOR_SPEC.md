# Refactor Spec — phd-hadron-physics

**Status (2026-10-02): as built.** This document now records what the port
built: the layout, packages, configuration and command line as they are in the
tree, the work that landed and the work that was dropped (§14), and where every
archived legacy script went (§15, Legacy → port map). The decision log (§2) and
the dropped public-release gate (§13) are kept as written at the time.
Supersedes the earlier refactor punch list and project review (not kept in
the repository). Behaviour kept from the legacy code, and differences
the port found but did not fix, are in `docs/PORT_NOTES.md`; findings that
change or disagree with published results are in `docs/KNOWN_ISSUES.md`.

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
| D1 | Repo root stays `~/phd-hadron-physics`; GitHub `jahernan1/phd-hadron-physics`, **public from first push**. |
| D2 | All current contents move into `_workdir/` (gitignored). Clean tree is built beside it by *copying* from `_workdir/`. `_workdir/` is never deleted by the refactor. `_workdir/` is the author's private legacy working directory and is not part of the repository. |
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
| D19 | gen_amp sampling histograms (data-derived ROOT files) stay external; `analyses/kpkpxim/simulation/inputs.lock` records name, sha256, size, JLab path. *Superseded by D26:* the sampling histograms are preserved data listed in `analysis_data.yaml`; there is no `inputs.lock`. |
| D20 | Library extraction is **behavior-preserving**: unit tests in repo + golden comparison on preserved data (D24) (new vs legacy outputs) run at FSU/JLab before any dedupe of copy-paste clusters. |
| D21 | LICENSE: **MIT** for author code; upstream code keeps its own licenses (§11). |
| D22 | Tool name **`gxana`** everywhere: CLI, Python package, env vars (`GXANA_*`), C++ namespace `gxana::`, libraries `GxanaCommon`/`GxanaXsec`. Repo name stays `phd-hadron-physics`. |
| D23 | Execution plans are local and gitignored. Legacy site paths (`/d/grid17/hjesse`, `/work/halld/home/jahernan`, …) are kept as provenance in `archive/`, this spec, the `gxana` legacy path map and git history; there is no automated public gate (author decisions 2026-09-22 and 2026-09-25). |
| D25 | Plan 3 simplifications and thesis fidelity: no `TreeHist.h`, `StackedHist.h`, `cuts.yaml`/`variations.yaml` or style-function dedupe (copies diverge; single consumers); systematics macros copied verbatim until ported onto `GxanaXsec`; the library reproduces the thesis on any ROOT by pinning ROOT-6.24 behavior (`LegacyFindBin` for flux windows, RooFit `Minimizer("Minuit","migrad")`); total-σ energy bins carry no `t_dist<2.4` cut, as in the thesis (author decision 2026-09-24). |
| D24 | Preserved analysis data (GlueX convention: code on GitHub, data under `/work/halld/gluex_analysis_data/`): `GXANA_ANALYSIS_DATA` (default `<repo>/gluex_analysis_data`, gitignored) holds golden inputs + legacy reference outputs; `analyses/<channel>/analysis_data.yaml` records path, sha256, size; `gxana data path\|status\|lock`. Replaces the `gxana toys` generator (2026-09-22). |
| D26 | Plan 4 simplifications and as-run fidelity: `gxana externals fetch\|status` (Python) replaces `fetch_externals.sh`; the lock records the sha256 of every patched file and fetch proves the tree reproduces the author's files; `compare_iters{,_2D}.C` are not merged (they diverge) and live in `analyses/kpkpxim/simulation/validation/`; the gen_amp sampling histograms are preserved data in `analysis_data.yaml` (no JLab path, replaces `inputs.lock`); `setup.sh --sim=<version-set>` renders into `$GXANA_EXTERNALS/version_sets/` (shared disk for batch jobs); one halld_sim checkout per version set; 2017-01 thesis MC uses `recon-2019_11-ver01_13`, as it ran; `runAllMC.sh` becomes `gxana run mc` + `config/mc.yaml` (author decision 2026-09-24). |
| D27 | Plan 5 as-built: fork commits made locally on `kpkpxim-thesis` from `d130c05` add a `QFACTORS_SETTINGS` env-var hook and a `makePlotsVars.txt` diagnostic-variable list; separate ROOT 6.40/clang portability commits drop dead includes (`RooMinuit.h`/`RooChi2Var.h`) in the engine and the then-default `configPDFs.h`, move PDF evaluation to named normalisation sets, and re-enable silent `RooRealVar` range clipping for ROOT >= 6.38 (reproducing 6.24's clipping of out-of-range `setVal`); the thesis fit models move into the fork as their own commits (author decision 2026-09-25, superseding D7's "channel run configs stay in `analyses/kpkpxim/signal_extraction`" for the fit models) in sequence — an as-run commit, then a build/run compat commit (drops the same dead includes, adds named normalisation sets, and the `chi2` argument the engine passes to `drawFitPlots`), then a commit pinning the ROOT 6.24 default `Minuit`/`migrad` minimizer in the channel models; the fork also resets `chiSqNdf_<var>` to NaN for events whose fit is not drawn and names the `fitParams_<var>` leaves without the per-process suffix; `analyses/kpkpxim` config is `config/qfactors.yaml` (not `run.yaml`, no `configSettings.h` copy); `gxana run qfactors --channel C --period P [--model M] [--steps prepare,fit,plots]` drives the fork; the golden test shows `configPDFs.h` reproduces the preserved 2017-01 thesis q-factors (neighbours exact, max \|dq\| < 1e-10) while the alternative models do not (plan and remaining choices: author decision 2026-09-24). |
| D28 | Plan 6 as-built: `analyses/kpkpkmlamb` = config (periods ana55/ana22/ana19, fit prefix `B4_M18_`, tree dirs `Trees/kpkpkmlamb/tree_{stem}/{kind}/`, data only), `selectors/`, `flat_trees/flatTreePrep.C` (writes `$GXANA_DATA/kpkpkmlamb/`), `measurements/FitXimStar.C(n_threads, tCut)` merging `FitXimStarCuts.C` (tCut = the published `t_dist>2` panel; the saved `t_dist>1` was edited after the panels); legacy `get_data_hists.C` was a kpkpxim macro → `archive/root_macros/`. Release: MIT `LICENSE`, `NOTICE.md`, `CITATION.cff` (author-filled fields), version 1.0.0; commit metadata keeps the author's email (author decisions 2026-09-25). The tables step writes each fit label into `data/<label>/` (as legacy `MakeXSecFitVariations.C` did), which the weight and components steps read. |

"Plan N" in the decision rows refers to row N of the first table in §14.

## 3. Layout (as built)

```
phd-hadron-physics/
  README.md                 physics goal, layout, quickstart, gxana command reference, documentation and packaging rules
  LICENSE  NOTICE.md  CITATION.cff
  CMakeLists.txt            top level; add_subdirectory(packages/{common,xsection,barlow,systematics,fit,studies})
  pyproject.toml  uv.lock   python package `gxana` + gxana_xsection, gxana_barlow, gxana_systematics, gxana_studies
  rootlogon.C               loads the Gxana* libraries and their include paths into ROOT
  .gitignore  .gitmodules
  env/
    README.md  setup.sh  site.example.sh
    apptainer/gxana.def     FROM the JLab GlueX image; adds cmake, python deps
    version_sets/           recon-*.xml.in sim version-set templates (4 sets)
  packages/
    common/                 GxanaCommon + GxanaPeriodHists (C++), gxana (Python CLI and stages); README
    xsection/               GxanaXsec, apps gxana_xsec_bin / gxana_xsec_tables, gxana_xsection (Python); README
    barlow/                 GxanaBarlow, apps gxana_barlow_trees / gxana_barlow_plot, gxana_barlow; README
    systematics/            GxanaSystematics, apps gxana_syst_plot / gxana_syst_track, gxana_systematics; README
    fit/                    GxanaFit: RooFit lineshape helpers for the analysis macros; README
    studies/                GxanaStudies, apps gxana_study_cutscan / gxana_study_datamc, gxana_studies; README
    qfactors/               git submodule → jahernan1/QFactors, branch kpkpxim-thesis (fork incl. thesis fit models configPDFs*.h)
    montecarlo/             external.lock, patches/{halld_sim,gluex_MCwrapper}, scripts/{build_halld_sim.sh,run_hdroot.py}; README, NOTICE.md
    each package: include/gxana/<pkg>/, src/, apps/, python/, tests/{cpp,python} as applicable
  analyses/
    kpkpxim/
      README.md  analysis_data.yaml
      config/               channel periods samples binning mc qfactors xsection barlow systematics studies measurements (.yaml)
      selectors/            DSelector_kpkpxim{,_F1,_2017,_hybrid}, DSelector_thrown_kpkpxim{,_F1}; README
      backgrounds/          selectors/ (pi0kpkpxim, pippimkplamb + thrown), KstarFit.C, YstarBWFitsData.C; README
      selection/            flatTreePrep.C (rapidity fixed), flatTreePrepQVal.C, cut_studies/<cut>/, mc_studies/, helper macros; README
      signal_extraction/    qfactors/ (README, scripts/), lineshape/ (OneUMLFit.C, SingleGaussianFit.C, DoubleGaussianFit.C)
      xsection/             PlotDiffXSec.C, PlotComponents.C, PlotXSecComponents.C, PlotTotXsecWithClas.C, MakeWeightedDiffXSecTGraphs.C, flux/getFlux.sh, external_data/Clas_data.csv
      systematics/          GetRunPeriodPctSig.C (runperiod hook), combine_pdf.sh, comparisons/ (README), track_efficiency/ (README), mc_weight_variations/
      simulation/           gen_amp_cfg/ mcwrapper/ hd_root/ genr8/ sampling/ validation/ local_beam.conf; README
      measurements/         common/XimInputs.h; mass/ PrepMass.C FitMass.C MakeXim1320_IM{,_Res}.C MakeXim1820_IM.C; lifetime/ PrepLifetime.C FitLifetime.C; spin/ PrepSpinData.C PlotGlueXSpin.C; README
    kpkpkmlamb/
      README.md  config/ (channel periods samples measurements)  selectors/  flat_trees/flatTreePrep.C  measurements/FitXimStar.C
  archive/
    README.md               index table
    mc_weights/ xsection_old/ xsection_legacy/ systematics_legacy/ selectors/ root_macros/ mc_legacy/ gx1_export/ env_fsu/   (194 code files, §15)
  scripts/                  migrate_paths.py, archive_copy.sh; README
  docs/
    REFACTOR_SPEC.md  environment.md  analysis_data.md  KNOWN_ISSUES.md  PORT_NOTES.md
  tests/                    repository tests; README
    golden/                 golden tests on the preserved data (marker golden); legacy/runperiod/ frozen original
    macros/                 every macro loads under cling; macro style harness
    env/ qfactors/ selection/ kpkpkmlamb/ stage_plans/ fixtures/channels/   plus repo checks (docs layout, README commands, no legacy paths, no channel literals, second channel, release files)
  _workdir/                 gitignored; everything that existed before the refactor
  gluex_analysis_data/      gitignored; preserved data (D24), default $GXANA_ANALYSIS_DATA
```

## 4. Packages

Each package README is the reference; the subsections below say what each
package holds and which stage drives it. Packaging rule (§5.2): code becomes
package code only if the thesis calls it many times (per bin, variation or
period) or another channel would run it as is. No package names a channel
(`tests/test_no_channel_literals.py`); channel values reach the C++ apps as
flags built by the stages from `analyses/<channel>/config/`.

### 4.1 `packages/common`

| Part | Content |
|---|---|
| `GxanaCommon` (namespace `gxana`) | `Style.h` (`SetStyle`, `ApplyStyle` with the presets `ThesisStyle`, `FitStyle`, `ComparisonStyle`, `TrackStyle`, `BarlowStyle`, `GridTrailingTweak`, `CutStudyStyle`, `DistributionStyle`), `Strings.h` (`NumericCompare`), `Paths.h` (`EnvPath`), `BinNames.h` (bin-edge labels, names, `ParseBinName`), `GraphIO.h` (`GetAllTGraphErrors`, `CreateTGraphErrorsFromTxt`, `ReadBinnedGraphs`), `AcceptanceCorrect.h` (`Acceptance`, `AcceptanceCorrect`, `LostBins`, `MergeCorrected`), `Periods.h` (`ChannelInfo`, `PeriodValue`, `Period`, `MakePeriods`, `GetPeriodHists`, `MergeHists`), `Overlay.h` (`DrawOverlay`), `Cli.h` (header-only argument parsers) |
| `GxanaPeriodHists` | `PeriodHists.h`: `FillHists`, `FillPeriodHists` (per-period RDataFrame fills; separate library so the others do not load RDataFrame) |
| Python `gxana` | `cli.py` (entry point), `config.py` (load, merge and check the channel YAML, `physics`, tree stems, `channel.kv` export), `paths.py` (`GXANA_*`, `legacy_to_env`), `bins.py`, `doctor.py`, `analysis_data.py` (D24), `externals.py` (§4.8), `stages/` (`runner.py` shared stage runner; `select`, `mc`, `qfactors`, `xsection`, `measurements`, and the entry points of `barlow`, `systematics`, `studies`) |
| Stage | `gxana run select`, `gxana run mc`, `gxana run qfactors`, `gxana run measurements`, `gxana config`, `gxana data`, `gxana externals` |
| Tests | ctest `common.*` (incl. `common.style`: every style preset leaves `gStyle` as its legacy body did); pytest `packages/common/tests/python` |

### 4.2 `packages/xsection`

| Part | Content |
|---|---|
| `GxanaXsec` (namespace `gxana::xsec`) | `Binning.h` (`divideNominalIntoBins`, `divideThrownIntoBins`, `divideVariationTreesIntoBins`), `YieldFit.h` (`RooFitMC`, `RooFitData`, `AttemptFit`, `AttemptFitMC`, `constructFitString` for Johnson / Gaussian / Voigtian + Chebychev; `RooFitMCShapeSeed`, `RooFitDataMCShape` for `JohnsonMCShape` and `JohnsonMCShapeSyst`; `RooFitMCPdf` for `MCPdf`; `SetFitPlotDir`, `SetFitStyle`), `Flux.h` (`GetFluxHist`), `XSec.h` (`GetDiffXSecFile`, `GetTotXSecFile`, `WriteXSecTables`), `Plotting.h` (`plotDiffXSec`, `plotWeightedXSec`, `plotOneWeightedXSec`, `plotFinalWeightedXSec`, `SetPlotDir`), `Physics.h` (`Observable`, the fit mass windows) |
| Apps | `gxana_xsec_bin data\|mc\|thrown\|variation`, `gxana_xsec_tables --fit Johnson\|Gaussian\|Voigtian\|JohnsonMCShape\|JohnsonMCShapeSyst\|MCPdf` |
| Python `gxana_xsection` | `weighted_average`, `components`, `qvalue_rescale`, `integrated_total`, `tex_table` (merges the three `MakeXsecTexTable*.py`), `compare` |
| Stage | `gxana run xsection` (`bin,tables,weight,integrate,components,tex`); also called by `barlow` and `systematics` |
| Tests | ctest `xsection.unit`; pytest `packages/xsection/tests/python`; goldens `test_binning_golden.py`, `test_xsec_golden.py`, `test_python_golden.py` |

### 4.3 `packages/barlow`

| Part | Content |
|---|---|
| `GxanaBarlow` (namespace `gxana::barlow`) | `Barlow.h` (`calc_barlow`, `calculateStdDevGraph`, `BarlowPlotStyle`, `BarlowPlotSpec`, `SetBarlowStyle`, `PlotBarlow`), `VariationTrees.h` (`WriteVariationTrees`, `CheckVariationYields`) |
| Apps | `gxana_barlow_trees` (variation trees; `--check` yield side check), `gxana_barlow_plot` |
| Python `gxana_barlow` | `config.py`, `variations.py`, `manifest.py` (`variations.json`), `stage.py` |
| Stage | `gxana run barlow` (`trees,check,bin,tables,weight,plot`; `check` opt-in), config `barlow.yaml` |
| Tests | ctest `barlow.unit`; pytest `packages/barlow/tests/python`; goldens `test_barlow_plot_golden.py`, `test_systematics_text_golden.py` |

### 4.4 `packages/systematics`

| Part | Content |
|---|---|
| `GxanaSystematics` (namespace `gxana::systematics`) | `PlotSpread.h` (`Plot`, the layouts `grid3`, `pair_band`, `all_band`, `grid2`, `run_grid`, `stddev_band`; graph readers; `StyleFormat`), `TrackHists.h` (`MakeKinematics`, `CountAndDraw`, `GetAcceptanceHist2D`, `GetAcceptanceCorrHist2D`) |
| Apps | `gxana_syst_plot` (draws only), `gxana_syst_track` (track kinematics and counts) |
| Python `gxana_systematics` | `config.py`, `stage.py`, `tables.py`, `spread.py`, `sfactor.py` (PDG scale factor), `track.py`, `runcompare.py`, `summary.py`: every number written to a stats file |
| Stage | `gxana run systematics` (`fit,qvalue,weight,spread,track,runperiod,compare,summary`; `runperiod`, `compare` opt-in), config `systematics.yaml` (variant pool, studies of kinds `spread`, `sfactor`, `track`, `constant`, `compare`, `runperiod` hook, `summary`) |
| Tests | ctest `systematics.*`; pytest `packages/systematics/tests/python`; goldens `test_systematics_{numbers,plot,track,summary,chain}_golden.py`, `test_runperiod_golden.py` |

### 4.5 `packages/fit`

| Part | Content |
|---|---|
| `GxanaFit` (namespace `gxana::fit`) | `Model.h` (factory-statement builders `Johnson`, `Gaussian`, `Voigtian`, `BreitWigner`, `Chebychev`, `Threshold`, `Sum`, `Fx`; `BuildModel`), `Fit.h` (`RunFit`, `ImportTree`, `FirstPopulatedEdge`), `Johnson.h` (`Moments`, `JohnsonMoments`); `GXANA_FIT_TRACE=1` prints factory statements and fit results |
| Users | twelve analysis macros (§15.2) and the `cutscan` study; no stage of its own; not extended further (`docs/PORT_NOTES.md` §15, §19) |
| Tests | ctest `fit.unit`, `fit.cling`; pytest `packages/fit/tests/python/test_legacy_sites.py` against frozen originals in `packages/fit/tests/legacy_sites/` |

### 4.6 `packages/studies`

| Part | Content |
|---|---|
| `GxanaStudies` (namespace `gxana::studies`) | `CutScan.h` (`FitCutScan`, `PlotCutScan`, `ApplyCutScanStyle`): cut scan with figure of merit S/√(S+B); `DataMC.h` (`DrawStacked`, `ApplyDataMCStyle`): data/MC and thrown/MC comparison plots |
| Apps | `gxana_study_cutscan` (`fill`, `fit`, `plot`), `gxana_study_datamc` (`fill`, `plot`) |
| Python `gxana_studies` | `config.py`, `stage.py` |
| Stage | `gxana run studies` (kinds `cutscan`, `datamc`), config `studies.yaml` |
| Tests | pytest `packages/studies/tests/python` incl. `test_cutscan_equivalence.py` and `test_kinematics_equivalence.py` (golden marker) against frozen originals in `packages/studies/tests/legacy/` |

### 4.7 `packages/qfactors`

Submodule of `jahernan1/QFactors`, branch `kpkpxim-thesis`, commits on top of
`d130c05` (D7, D27). Fork commits, in order: "Ignore run outputs; run the drivers with python3"; "run.py: optional termcolor, QFACTORS_SETTINGS file, longer progress-check delay"; "Build with ROOT 6.40 and clang"; "Save per-event fit chi2/ndf and fit parameters"; "mergeQresults: carry chiSqNdf into the postQVal tree"; "makePlots: variable list file, NaN counter, signal sum, branch-type fix"; "Document the fork: README notice and CHANGES_THESIS.md"; "Evaluate PDFs with named normalisation sets for ROOT 6.40"; "Add the kpkpxim thesis fit models (as run)"; "Make the fit models build and run with the current engine and ROOT 6.40"; "Pin the ROOT 6.24 default minimizer in the fit models"; "Keep RooRealVar range clipping on ROOT >= 6.38"; "Name fit-parameter leaves without the process suffix" (13 commits).

Driven by `gxana run qfactors` (`gxana/stages/qfactors.py`): the stage renders
`configSettings.h` from the fork's template with the `settings` of
`config/qfactors.yaml`; `qfactors.model` names a `configPDFs*.h` in the fork or
a channel's own model file. Helper macros live in
`analyses/kpkpxim/signal_extraction/qfactors/scripts/`. Tests:
`tests/qfactors/` and `tests/golden/test_qfactors_golden.py` (`configPDFs.h`
reproduces the preserved 2017-01 q-factors; neighbours exact, max |dq| < 1e-10).

### 4.8 `packages/montecarlo`

`external.lock` (YAML):

| name | url | ref | sha | patches |
|---|---|---|---|---|
| halld_sim | https://github.com/JeffersonLab/halld_sim | `4.54.0` | `bcff7a5c4e8453b1e75e83facdc6f2ad95ba5719` | 0001-Hist2D-add-CosTheta-histTypes, 0002-AMPTOOLS_AMPS-add-Hist3D-amplitude, 0003-gen_amp_V2-register-Hist3D-lvRange-recoil-histograms, 0004-gen_amp-force-K-for-the-suffixed-second-particle, 0005-mcsmear-revert-FCALSmearer-channel-loop-for-MC-recon |
| gluex_MCwrapper | https://github.com/JeffersonLab/gluex_MCwrapper | `c4e918a2` | `c4e918a2fe0e48bdf67f69326ed1820458f26b9e` | 0001-Add-UPPER-LOWER_VERTEX_INDICES-config-keys-for-gen_a, 0002-geant4-force-Lambda-p-pi-in-run.mac (also fixes `>>!` in MakeMC.sh) |
| AmpTools | https://github.com/mashephe/AmpTools | `v0.15.2` | `fed18194954f0857f4e5a22f8c51940285b26237` | none (`fetch: false`, from the version sets) |
| HDGeant4 | https://github.com/JeffersonLab/HDGeant4 | `2.42.0` | `e09d41fad1801f28667e92086306173ee1f50075` | none (`fetch: false`) |

Patch sources: the ifarm area's `my_halld_sim/halld_sim-2019_11-ver01_13` (all four period dirs have identical source) vs upstream 4.54.0; its `gluex_MCwrapper` vs `c4e918a2`. The orphan `Hist2D_MultiPart.cc` is not included. Each patch header: `From:` author, `Subject:`, and `Upstream: <repo>@<sha>; upstream copyright retained`. The lock records the sha256 of every patched file. `gxana externals fetch [names] [--dest D]` clones at the sha, runs `git am` and verifies the sha256s; `gxana externals status` reports ok/diff/miss/pin. `scripts/build_halld_sim.sh <set>` fetches into `$GXANA_EXTERNALS/halld_sim-<set>` and builds it in the sim env; `scripts/run_hdroot.py` is generalized from `AnalysisNote/MC/run_hdroot.py`. Local `AnalysisNote/MC/halld_sim` (4.47.0-17, unmodified) and `AnalysisNote/MC/gluex_MCwrapper` (2.5.0) were stale and are not migrated. Thesis MC production: `gxana run mc` + `analyses/kpkpxim/config/mc.yaml`. Tests: `packages/montecarlo/tests/`, `packages/common/tests/python/test_externals.py`, `test_mc_stage.py`; `tests/golden/test_sampling_golden.py` for the sampling histograms.

## 5. Analyses

### 5.1 `analyses/kpkpxim` migration map

Sources are relative to `_workdir/AnalysisNote/` unless marked; per-file detail for the archived ones is in §15.

| Destination | Sources |
|---|---|
| `selectors/` | `_workdir/DSelector/DSelector_kpkpxim{,_F1,_2017,_hybrid}.*`, `DSelector_thrown_kpkpxim{,_F1}.*` |
| `backgrounds/selectors/` | `DSelector_pi0kpkpxim.*`, `DSelector_pippimkplamb.*`, `DSelector_thrown_pippimkplamb.*` |
| `backgrounds/` | `utilities/KstarFit.C`, `utilities/YstarBWFitsData.C` |
| `selection/` | gx1 `PrepFlatTrees.C` → `flatTreePrep.C` (D18); `utilities/flatTreePrepQVal.C`, `flatTreeCuts{,MC}.C`, `flatTreePlots.C`, `CompareFromTree.C`; `analysis/event_selection/<cut>/` → `cut_studies/<cut>/` (minus ROOT-generated `c_format_plots/`); `analysis/analysis/mc_studies/*` → `mc_studies/`; `analysis/event_selection/qfactors/XimMassQVal.C`. `CutAnalysis{,RF}.C` and `GetKinematicsDataMC{,_RF}.C` were migrated here, then replaced by `gxana run studies` and archived. |
| `signal_extraction/` | QFactors run config (§4.7) and `QFactors/scripts/*.C` → `qfactors/scripts/`; `xsection/OneUMLFit.C`, `SingleGaussianFit.C`, `DoubleGaussianFit.C` → `lineshape/` |
| `xsection/` | `MakeWeightedDiffXSecTGraphs.C`, `PlotDiffXSec.C`, `PlotComponents.C` (clashing `PlotDiffXSec()` renamed), `PlotXSecComponents.C`, `PlotTotXsecWithClas.C`; `fluxFiles/getFlux.sh` → `flux/`; `_workdir/Clas_data.csv` → `external_data/`. The binning, fit, weighting, component and table code became `packages/xsection` (`gxana run xsection`). |
| `systematics/` | `systematics/GetRunPeriodPctSig.C` (the `runperiod` hook), `combine_pdf.sh`, `systematics/mc_weight_variations/*`. The variation-tree, fit-variation, Barlow, comparison and track-efficiency macros became `packages/barlow` and `packages/systematics` and are archived. |
| `simulation/` | ifarm `gen_amp/gen_amp_cfg/kpkpxim_2dhist_{ac,noac}_YstarRest.cfg` → `gen_amp_cfg/`; ifarm `xim_jlab_MC_gen_amp_*.conf` → `mcwrapper/`; ifarm `hd_root_files/*.conf` + `MC/hd_root_xim13.config` → `hd_root/`; `MC/local_beam.conf`; `MC/getHist2D_gen_amp.C` → `sampling/PrepSampling.C`, `MC/getHist3D{,_F18}.C` → `sampling/`; `MC/compare_iters{,_2D}.C`, `MC/tree_3d/*.C` → `validation/`; genr8 `Xi_1320*.input` → `genr8/` (reference only). `runAllMC.sh` became `gxana run mc` + `config/mc.yaml`. |
| `measurements/mass/` | `utilities/MakeXim1320_IM{,_Res}.C`, `MakeXim1820_IM.C`; `PrepMass.C` and `FitMass.C` split from `analysis/analysis/cascade_properties/GetXimProperties.C` (archived) |
| `measurements/lifetime/` | `PrepLifetime.C`, `FitLifetime.C` split from `GetXimProperties.C` |
| `measurements/spin/` | `analysis/analysis/cascade_properties/PlotGlueXSpin.C`; `PrepSpinData.C` split from `GetXimProperties.C` |

gx1-only improvements ported: rapidity fix (D18), `XSecFunctions` rename with fixed include. AnalysisNote wins where it is newer (`mc_studies/get_data_hists_RF.C`, `make_plot_RF.C`, `chisqndf_cut/get_data_hists_RF.C`; the gx1 copies are in `archive/gx1_export/`).

### 5.2 What stays a standalone script

Packaging rule (`docs/PORT_NOTES.md` §19, root `README.md`): code becomes
package code only if the thesis calls it many times (per bin, variation or
period) or another channel would run it as is. A thesis-specific fit or figure
stays a standalone script with a README run command and, where its inputs are
preserved, a golden test. Such scripts use the package building blocks
(`gxana::fit`, `ApplyStyle`, `AcceptanceCorrect`, `PeriodHists`, `ChannelInfo`;
§15.2) but keep their own logic.

| Stays a script | Where | Why |
|---|---|---|
| Lineshape and mass fits `MakeXim1320_IM*.C`, `MakeXim1820_IM.C`, `KstarFit.C`, `YstarBWFitsData.C`, `compare_iters*.C`, lineshape macros | `measurements/mass/`, `backgrounds/`, `simulation/validation/`, `signal_extraction/lineshape/` | thesis-specific one-off figures |
| Measurement prep and fit macros | `measurements/{mass,lifetime,spin}/` | run as listed macros by `gxana run measurements` (`measurements.yaml`); golden `test_measurements_golden.py`, `test_measurements_stage_golden.py` |
| `GetQvalueSum.C` | `signal_extraction/qfactors/scripts/` | the Q-factor validation is not wanted as a stage |
| `GetRunPeriodPctSig.C` | `systematics/` | run behind the `runperiod` hook of `systematics.yaml`; golden `test_runperiod_golden.py` |
| `flatTreePrep.C`, `flatTreeCutsMC.C`, cut and MC study macros | `selection/` | single consumers; `flatTreePrep.C` tested by `tests/selection/test_flat_tree_prep.py` |
| both spin-fit copies (merged and per-period) | `measurements/spin/PlotGlueXSpin.C` | unifying them changes the per-period plots |
| xsection plotting macros, MC-weight variations, sampling macros | `xsection/`, `systematics/mc_weight_variations/`, `simulation/sampling/` | single consumers |

### 5.3 `analyses/kpkpkmlamb`

`_workdir/DSelector/kpkpkmlamb/DSelector_kpkpkmlamb.{C,h}` → `selectors/`; `_workdir/kpkpkmlamb/flatTreePrep.C` → `flat_trees/`; `FitXimStar.C` + `FitXimStarCuts.C` merged (`bool tCut` argument) → `measurements/FitXimStar.C`, fitting through `gxana::fit` and run by `gxana run measurements --channel kpkpkmlamb` (`config/measurements.yaml`). Config: `channel.yaml`, `periods.yaml` (`2017-01_ana55`, `2018-01_ana22`, `2018-08_ana19`, fit prefix `B4_M18_`), `samples.yaml`, `measurements.yaml`. Tests: `tests/kpkpkmlamb/`. As built: see D28.

## 6. Archive (`archive/`)

194 code files plus `archive/README.md`. Archived code is verbatim, not built,
loaded or tested, and excluded from `tests/test_no_legacy_paths.py` and
`tests/macros/test_macros_load.py`. Every file is mapped to its replacement in §15.

| Dir | Files | Content | Superseded by |
|---|---|---|---|
| `mc_weights/` | 8 | `AnalysisNote/mc_weights/**` incl. `archive/` (genr8 reweighting) | not used in thesis |
| `xsection_old/` | 28 | `AnalysisNote/xsection_old/**` (early UML / histogram pipeline) | `packages/xsection` |
| `xsection_legacy/` | 28 | `AnalysisNote/xsection` macros and Python drivers | `packages/xsection` (`gxana run xsection`), `packages/common` |
| `systematics_legacy/` | 26 | `AnalysisNote/systematics` variation/fit/Barlow chain, track efficiency, the `AnalysisNote/xsection/Plot*Comparison.C` macros (`comparisons/`) | `packages/barlow` (`gxana run barlow`), `packages/systematics` (`gxana run systematics`) |
| `selectors/` | 2 | `DSelector_kpkpxim_legacy.C`, `DSelector_pi0kpkpxim_1.C` | the main `DSelector_kpkpxim*` family |
| `root_macros/` | 16 | home-dir and AnalysisNote macros; `CutAnalysis{,RF}.C`, `GetKinematicsDataMC{,_RF}.C` and `GetXimProperties.C` archived after their ports (2026-10-01/02) | `packages/studies`, `packages/common`, `analyses/kpkpxim/measurements` |
| `mc_legacy/` | 68 | pre-thesis MC: `jlab/` = the author's ifarm MC area, `local/` = `AnalysisNote/MC` | `analyses/kpkpxim/simulation`, `packages/montecarlo`, `gxana run mc` |
| `gx1_export/` | 5 | files of the earlier gx1 export not byte-identical to any migrated file | this repo |
| `env_fsu/` | 3 | FSU-cluster environment and selector scripts | `env/`, `gxana run select` |

Never migrated (stay in `_workdir/` only): dotfiles, `temp/`, `Trees/`, empty dirs, all build artifacts, editor junk, stray `C` ls-dumps, `bins.txt`, `tmp.cfg` (ROOT binary), ROOT-generated `c_format_plots/*.C`, local halld_sim/gluex_MCwrapper clones, QFactors junk (`main`, `os`, `sys`, `time`, `subprocess`), tmux scripts, `switchgridname.sh`, all text outputs (`*.txt` data, `*.out`, `*.tex`, `output.csv`).

## 7. Environment

### 7.1 Container
`env/apptainer/gxana.def`: `Bootstrap: docker`, `From: jeffersonlab/gluex_almalinux_9:gxi2.40@sha256:99ed43152dab0932328fc46cf215362f529fdd2758233e2d2beb8c25da5c4670` (verified on Docker Hub 2026-09-22); `%post` installs `cmake>=3.20`, `python3-pyyaml`, `python3-pytest`. The GlueX software itself comes from `/group/halld` (CVMFS bind) via `gxenv`, exactly as on ifarm. At JLab/OSG the same image is available under `/cvmfs/singularity.opensciencegrid.org/jeffersonlab/`. Thesis-era FSU image (`gluex_centos-7.7.1908_sng3.8_gxi2.20.sif`) documented in `archive/env_fsu/`. Full guide: `docs/environment.md`.

### 7.2 Version sets
Two environments, because the analysis code needs ROOT ≥ 6.20 (`RooJohnson`) while the MC production sets ship ROOT 6.08.06:

- **analysis** (selectors, selection, Q-factors, xsection, systematics): upstream halld version set **5.12.0** — ROOT 6.24.04, gluex_root_analysis 1.25.0, amptools 0.15.1 (matches `AnalysisNote/MC/version.xml`; FSU runs used `gxenv /d/grid13/sdobbs/GlueX/eeg/version.xml`, whose content is to be confirmed identical at FSU). Used unmodified from `$HALLD_VERSIONS/version_5.12.0.xml`; nothing copied into the repo.
- **sim** (per run period): `env/version_sets/*.xml.in` templates of the recon sets below, with `halld_sim` `home="${GXANA_EXTERNALS}/halld_sim-<set>"` and `gluex_MCwrapper` `home="${GXANA_EXTERNALS}/gluex_MCwrapper"`; `source env/setup.sh --sim=<set>` renders them into `$GXANA_EXTERNALS/version_sets/` before `gxenv` (gxenv does not expand env vars; batch jobs read the file, so it sits on shared disk). Period → set lives in `analyses/kpkpxim/config/mc.yaml`.

| Period | analysis env | recon/sim env | hdgeant4 | amptools | root |
|---|---|---|---|---|---|
| 2017-01 | analysis-2017_01-ver56 (alt ver45) | recon-2019_11-ver01_13 (thesis MC, as run; recon-2017_01-ver03_40 for the pre-thesis ana45 MC) | 2.42.0 | 0.15.2 | 6.08.06 |
| 2018-01 | analysis-2018_01-ver03 | recon-2018_01-ver02_32 | 2.42.0 | 0.15.2 | 6.08.06 |
| 2018-08 | analysis-2018_08-ver02 | recon-2018_08-ver02_31 | 2.42.0 | 0.15.2 | 6.08.06 |

`source env/setup.sh [--gluex] [--sim=<set>]`: sources `env/site.sh` (if present), exports `GXANA_ROOT` (repo dir), defaults `GXANA_DATA`, `GXANA_OUTPUT`, `GXANA_SCRATCH`, `GXANA_EXTERNALS`, `GXANA_ANALYSIS_DATA`; `--gluex` sources `/group/halld/Software/build_scripts/gluex_env_boot_jlab.sh` + `gxenv $HALLD_VERSIONS/version_5.12.0.xml`; `--sim=<set>` (exclusive with `--gluex`) renders + gxenv's that recon template and exports `GXANA_SIM_VERSION_SET`; always prepends `$GXANA_ROOT/build/lib` to `LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH` and the package Python directories to `PYTHONPATH`.

### 7.3 Env vars and legacy path map

| Var | Meaning | FSU value (site.sh) |
|---|---|---|
| `GXANA_ROOT` | repo checkout | set by setup.sh |
| `GXANA_DATA` | input trees/flat trees root | `/d/grid17/hjesse` |
| `GXANA_OUTPUT` | stage outputs root | `/d/grid17/hjesse/gxana_output` |
| `GXANA_SCRATCH` | PROOF sandbox, temp | `/d/grid17/hjesse/temp` |
| `GXANA_EXTERNALS` | fetched+patched upstream builds | `/d/grid17/hjesse/externals` |
| `GXANA_ANALYSIS_DATA` | preserved data (D24) | default `<repo>/gluex_analysis_data` |

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

All files of a channel are merged; a top-level key may appear in one file only.
`gxana config show --channel C` prints the merged result with `${GXANA_*}`
expanded. Unknown keys are rejected by the stage that owns the block. After any
edit, `gxana config export --channel C` rewrites
`$GXANA_OUTPUT/<C>/config/channel.kv`, the flat file the C++ macros read
(`gxana::ChannelInfo`); it records the MD5 of every `config/*.yaml` and the
macros refuse a stale copy.

| File | Top-level key | Read by | Content |
|---|---|---|---|
| `channel.yaml` | `channel`, `reaction`, `selector_dir`, `default_selector`, `thrown_selector`, `output_basename`, `physics` | all stages | selectors; `physics:` = `flat_tree`, `thrown_flat_tree`, `observable {branch, title}`, `qvalue_branch` (or `null`), `branching_ratio {value, error}`, `reaction_title` (passed as flags to the xsection and barlow apps) |
| `periods.yaml` | `periods` | all stages, `channel.kv` | per period: `launch`, `fit_prefix`, `label`, `runs`, `flux`, `dir` (directory name in the macros' ROOT files, e.g. `Spring_2017`), `title` (readable name, e.g. `Spring 2017`) |
| `samples.yaml` | `tree_dir_template`, `samples` | `select`, tree stems | per sample: `mc`, optional `launch` (per period), `fit_prefix`, `tree_suffix`, `selector`, `output_basename` |
| `binning.yaml` | `energy_edges`, `t_bins`, `total_energy_range` | `xsection`, `barlow`, `systematics` | bin edges |
| `mc.yaml` | `mc` | `mc` | `simulation_dir`, `batch`, per-period `conf`/`events`/`sim_version_set`, per-sample `generator_config`, `periods`, `overrides` (MCwrapper keys, e.g. `BKG`) |
| `qfactors.yaml` | `qfactors` | `qfactors` | `engine_dir`, `model`, `sample`, `variant`, `input`, `tree`, `work_dir`, `output_dir`, `plots_dir`, `settings` (the fork's `_SET_*` values, `kDim: 200`), `extra_settings`, `diagnostic_vars` |
| `xsection.yaml` | `xsection` | `xsection` (and the fit/flux/tree inputs of `barlow`, `systematics`) | `mc_sample`, `weight`, `inputs`, `output_dir`, `fit_plots`, `fits` (model, params, labels with `cheby` and optional `weight`), `weighted_labels`, `component_labels`, `tex` (`label`, `output`, `run_fraction`, `columns`), `binned_suffix`, `gate`, `target`, `branches`, `mass_windows` |
| `barlow.yaml` | `barlow` | `barlow` | `label`, `output_dir`, `mc_sample`, `weight`, `threshold`, `fit`, `trees` (`input`, `output`, `threads`, `defines`, `filters`, `branches`), `check`, `plot`, `nominal`, `fixed`, `families` (values, style) |
| `systematics.yaml` | `systematics` | `systematics` | `output_dir`, `nominal`, `variants` (fit groups and `qvalue` rescale), `studies` (`spread`, `sfactor`, `track`, `constant`, `compare`), `runperiod {macro}`, `summary {point_by_point, normalization}` |
| `studies.yaml` | `studies` | `studies` | per study: `kind` (`cutscan` or `datamc`), optional `threads` (passed to `fill`; `0` = single-threaded), inputs, `steps` (`define`/`filter`), weights, binning, fit, outputs |
| `measurements.yaml` | `measurements` | `measurements` | `output_dir`, `make_dirs`, `items` (each with `prep` and `fit`: `macro` path relative to the channel, `args`) |

The shipped kpkpxim `studies.yaml` (`threads: 0`) and `measurements.yaml`
(`args: [0]`) run single-threaded, as the golden and equivalence tests do.
Tree stem = `<reaction>__<fit_prefix><period>_<launch><tree_suffix>`
(`gxana.config`; `tree_suffix` defaults to `_<sample>` for MC). Nominal cuts
stay in `selection/flatTreePrep.C`; the 18 Barlow variations are in
`barlow.yaml` (D25). `analyses/kpkpkmlamb` ships `channel.yaml`,
`periods.yaml`, `samples.yaml` and `measurements.yaml` only.

## 9. CLI `gxana`

Usage lines as printed by `uv run gxana <command> --help`; the root `README.md`
command reference documents every flag (`tests/test_docs_layout.py`,
`tests/test_readme_commands.py`).

```
usage: gxana [-h] {doctor,config,run,data,externals} ...
usage: gxana doctor [-h]
usage: gxana config show [-h] --channel CHANNEL
usage: gxana config export [-h] --channel CHANNEL [--out OUT]
usage: gxana data path [-h] --channel CHANNEL
usage: gxana data status [-h] --channel CHANNEL
usage: gxana data lock [-h] --channel CHANNEL
usage: gxana externals fetch [-h] [--dest DEST] [names ...]
usage: gxana externals status [-h] [--dest DEST] [names ...]
usage: gxana run select [-h] --channel CHANNEL --period PERIOD [--sample SAMPLE] [--thrown] [--tag TAG] [--cores CORES] [--selector SELECTOR] [--dry-run]
usage: gxana run mc [-h] [--channel CHANNEL] --period PERIOD --sample SAMPLE [--dry-run]
usage: gxana run qfactors [-h] [--channel CHANNEL] --period PERIOD [--model MODEL] [--steps STEPS] [--dry-run]
usage: gxana run xsection [-h] [--channel CHANNEL] [--steps STEPS] [--dry-run]
usage: gxana run barlow [-h] [--channel CHANNEL] [--steps STEPS] [--dry-run]
usage: gxana run systematics [-h] [--channel CHANNEL] [--steps STEPS] [--study STUDY] [--dry-run]
usage: gxana run studies [-h] --channel CHANNEL [--steps STEPS] [--study STUDY] [--dry-run]
usage: gxana run measurements [-h] --channel CHANNEL [--steps STEPS] [--item ITEM] [--dry-run]
```

| Stage | Steps (default; opt-in) | Package |
|---|---|---|
| `select` | none | `gxana.stages.select` |
| `mc` | none | `gxana.stages.mc`, `packages/montecarlo` |
| `qfactors` | `prepare,fit,plots` (default `fit,plots`) | `gxana.stages.qfactors`, `packages/qfactors` |
| `xsection` | `bin,tables,weight,integrate,components`; `tex` | `packages/xsection` |
| `barlow` | `trees,bin,tables,weight,plot`; `check` | `packages/barlow` |
| `systematics` | `fit,qvalue,weight,spread,track,summary`; `runperiod`, `compare` | `packages/systematics` |
| `studies` | `fill,fit,plot` (per kind) | `packages/studies` |
| `measurements` | `prep,fit` | `gxana.stages.measurements` |

`xsection`, `barlow`, `systematics`, `mc` and `qfactors` default to
`--channel kpkpxim`; `select`, `studies` and `measurements` require it.
`gxana check-public` was dropped (§13) and does not exist.

`run select` reproduces `runDSelector.sh` exactly, minus hardcoded paths:
- tree dir from `tree_dir_template`; tree name = the single key of first `*.root` file (error if ≠1 key);
- save name = tree-dir parent basename minus `tree_` prefix, plus `_<tag>`;
- deletes stale `{,thrown_,flatTree_,flatTree_thrown_}<output_basename>` in the run dir (`$GXANA_SCRATCH/run/<save>`), not repo root;
- ROOT heredoc: `gEnv->SetValue("ProofLite.Sandbox","$GXANA_SCRATCH/proof")`, `.x $ROOT_ANALYSIS_HOME/scripts/Load_DSelector.C`, `TChain`, `DPROOFLiteManager::Process_Chain(ch,"<selector>++",N)`;
- moves outputs: hist file → `$GXANA_OUTPUT/<channel>/selector_hists/[thrown_]<save>.root`; flat tree → `$GXANA_DATA/Trees/flatTree/rawTrees/flatTree_[thrown_]<save>.root`.
Bugs in `runMultiDSelector.sh` (`-S`, `-s <name>`, `-c 16-s`, `./run DSelector.sh`) disappear because samples come from YAML.

## 10. Build

Top-level CMake ≥ 3.20, C++17, ROOT with RooFit and RDataFrame. Libraries
`GxanaCommon`, `GxanaPeriodHists`, `GxanaXsec`, `GxanaBarlow`,
`GxanaSystematics`, `GxanaFit`, `GxanaStudies` (ROOT dictionaries from each
package's `LinkDef.h`; `GxanaPeriodHists` from `LinkDefPeriodHists.h`) into
`build/lib`; apps `gxana_xsec_bin`, `gxana_xsec_tables`, `gxana_barlow_trees`,
`gxana_barlow_plot`, `gxana_syst_plot`, `gxana_syst_track`,
`gxana_study_cutscan`, `gxana_study_datamc` into `build/bin`. `rootlogon.C` at
the repo root loads the libraries and adds the package include paths. Tests:
`enable_testing()` + plain `add_test` executables (no gtest dependency), run
with `uv run ctest --test-dir build`. The legacy `compile_lib.sh` /
`compile_code.txt` are retired. DSelectors stay ACLiC (D14).

## 11. Credit & licensing

`NOTICE.md` lists: JeffersonLab/halld_sim, JeffersonLab/gluex_MCwrapper, JeffersonLab/HDGeant4, mashephe/AmpTools, lan13005/QFactors, JeffersonLab gluex_root_analysis (DSelector template, `DPROOFLiteManager`), JLab halld version sets; each with URL, pinned ref, license as stated upstream, and "not redistributed here; patches in `packages/montecarlo/patches` are modifications by J. Hernandez to the listed upstream revision". DSelector files keep their template header comment. Provenance of the patches/configs is stated as "the author's JLab ifarm MC area" (D9).

## 12. Known issues handling

1. **Rapidity swap (D18)** — AnalysisNote `flatTreePrep.C` defines `kphigh/kplow/ystar_rapidity = atanh(pz/p)` (pseudorapidity) and `*_prapidity = .Rapidity()`. gx1 `PrepFlatTrees.C` is correct and became the migrated `selection/flatTreePrep.C`. `docs/KNOWN_ISSUES.md` §1 records which outputs are affected.
2. Documented, not fixed during migration: per-combo `cout` spam in `DSelector_kpkpxim.C::Process`; PID ΔT and Ξ mass-window cuts commented out in selector (applied downstream); `PlotComponents.C` `PlotDiffXSec()` name clash (renamed on migration).
3. Fixed in their own package work: QFactors `qvalueSum` uninitialized, VLA (fork, §4.7); MCwrapper `MakeMC.sh` `>>!` (patch 0002, §4.8).
4. Everything the port kept from the legacy code, or found and did not fix, is in `docs/PORT_NOTES.md`, one section per area; what changes or disagrees with a published result is in `docs/KNOWN_ISSUES.md`.

## 13. Public-release gate (dropped)

Dropped by the author (2026-09-22); the pre-push check is a manual review of `git ls-files` and a fresh-clone build. Original design kept below for reference.

`gxana check-public` (and `.git/hooks/pre-commit` calling it on staged files) fails on:
- content matching: `/d/grid1[37]/`, `/work/halld/home/`, `/w/halld-scshelf`, other users' home directories (`/home/<user>`), `hjesse@`, `jahernan@`, `10\.0\.0\.\d+`, `scigrid\d`, `LAPTOP-`, `-----BEGIN .*PRIVATE KEY`, `(ghp|gho|github_pat)_[A-Za-z0-9_]{20,}`;
- files: extensions `.root .hddm .so .o .d .pcm .a .pdf .png .eps .ps .svg .evio`, ELF/Mach-O magic, size > 1 MiB, names `*~`, `#*#`, `.#*`, dotfiles from `$HOME` (`.bash_history`, `.Xauthority`, `.esd_auth`, `.viminfo`, `.root_hist`);
- private deny-list `.public-deny.local` (gitignored, one regex per line; rule `private-ref`) for names that must never appear publicly;
- allowlist `.public-allow` (glob per line) for deliberate exceptions (e.g. `docs/img/*.svg`, the check's own test fixtures).
First push only after `gxana check-public` passes on the whole tree **and** a manual review of `git ls-files`.

## 14. Implementation (as built)

Original sequence (the "Plan N" of §2 is row N):

| # | Work | Delivered | Done |
|---|---|---|---|
| 1 | Foundation and common library | `_workdir` move, skeleton, `.gitignore`, `gxana` Python package (paths, config, doctor, `run select`), CMake + `GxanaCommon` (Style, NumericCompare, Paths), `env/` (setup.sh, site.example.sh, gxana.def, rootlogon.C), CI | 2026-09-22 |
| 2 | Cross-section package | `GxanaXsec` + `gxana_xsection`; `GXANA_ANALYSIS_DATA` + `gxana data`; `GraphIO`; golden tests (`tests/golden`, `gxana_xsection.compare`) | 2026-09-23 |
| 3 | kpkpxim analysis migration | §5.1 moves with `scripts/migrate_paths.py` + guard tests, rapidity fix, `gxana run xsection` + `xsection.yaml`, `Plotting.h`, `tex_table`, thesis-fidelity pins (D25), golden tests on the full preserved data, `archive/` | 2026-09-24 |
| 4 | Monte-Carlo package | `external.lock` + 7 patches with per-file sha256, `gxana externals`, sim version-set templates + `setup.sh --sim`, `build_halld_sim.sh`, `run_hdroot.py`, `analyses/kpkpxim/simulation/` + `mc.yaml`, `gxana run mc`, sampling histograms in the preserved data, `archive/mc_legacy/` (D26) | 2026-09-24 |
| 5 | QFactors fork | fork commits on d130c05, submodule, `qfactors.yaml`, `gxana run qfactors`, toy + golden tests (D27) | 2026-09-25 |
| 6 | Side channel, docs, release | `analyses/kpkpkmlamb`, LICENSE/NOTICE/CITATION, README, KNOWN_ISSUES, fresh-clone check (D28) | 2026-09-25 |

Later work (2026-09-28 to 2026-10-02), by content:

| Work | Delivered |
|---|---|
| Port review reconcile | published label = `johnson`; two total cross sections (direct and integrated, `gxana_xsection.integrated_total`); Barlow from the UML chain; `kDim: 200`; newest MC version sets (`docs/KNOWN_ISSUES.md` §2) |
| Barlow package | `packages/barlow`, `gxana run barlow`, `barlow.yaml`; the UML variation chain archived |
| Systematics suite | `packages/systematics`, `gxana run systematics`, `systematics.yaml`: variant pool, accidentals and fit spreads, PDG scale factor, track efficiency, quadrature summary, opt-in `compare` and `runperiod` checks; comparison and track macros archived |
| Acceptance library | `AcceptanceCorrect.h` in `GxanaCommon`, adopted by the sampling, MC-study, measurement and MC-weight macros (§8 of PORT_NOTES) |
| Measurements rework | `GetXimProperties.C` split into prep and fit macros for mass, lifetime, spin; golden `test_measurements_golden.py`; `GetXimProperties.C` archived |
| Common consolidation | `BinNames.h`, `ReadBinnedGraphs`, `NumericCompare` by full emin |
| Stage infrastructure | `gxana.stages.runner` (shared plan / dry-run / run loop), `tests/stage_plans/` baselines |
| Macro styles | `ApplyStyle` presets replacing each macro's local style function; `common.style` and `tests/macros/test_macro_styles.py` |
| Q-factor config | `qfactors.model` accepts a channel's own `configPDFs*.h` |
| Channel-agnostic packages | `physics:` block in `channel.yaml`; every channel constant passed as a flag; `tests/test_second_channel.py`, `tests/test_no_channel_literals.py` |
| Fit library | `packages/fit` (`GxanaFit`), adopted by twelve analysis macros; `test_legacy_sites.py` |
| Period histograms | `gxana config export` / `channel.kv`, `Periods.h`, `GxanaPeriodHists`, `Overlay.h`, adopted by the MC-study, validation, sampling, MC-weight and track code |
| Study framework | `packages/studies`, `gxana run studies`, `studies.yaml` (`cutscan`, `datamc`); `CutAnalysisRF.C`, `GetKinematicsDataMC_RF.C` archived |
| Measurements stage and run-period check | `gxana run measurements`, `measurements.yaml` (both channels); golden `test_measurements_stage_golden.py`, `test_runperiod_golden.py` |
| Closeout | packaging rule (§5.2); studies and measurements single-threaded (`threads: 0`, `args: [0]`); `CutAnalysis.C`, `GetKinematicsDataMC.C` archived; duplicate `track_efficiency/WeightMC.C` deleted |

Dropped, one reason each (`docs/PORT_NOTES.md` §19):

| Dropped | Reason |
|---|---|
| Rebuilding `YieldFit` on `GxanaFit` | the per-bin fit is already the `GxanaXsec` package |
| Study kinds for the lineshape, background-reflection and MC-iteration macros | one-off thesis figures, not called per bin or variation |
| A Q-factor validation study kind | `GetQvalueSum.C` stays a script |
| DSelector helpers | not shared with another channel |
| Public-release gate `gxana check-public` | dropped by the author 2026-09-22 (§13) |

Author-run items still open: the ROOT 6.24 container golden run on the ifarm
(`docs/analysis_data.md`) and the ifarm kpkpkmlamb bunch-count check.
Outward-facing actions (publishing the QFactors fork and this repository)
require explicit author confirmation.

## 15. Legacy → port map

### 15.1 Archived scripts

Every code file under `archive/` (194), with the `_workdir/` path it was copied
from. "Replaced by" names the package code, app, stage step, config key or live
macro that does the job now; "Verified by" names the golden or equivalence test,
or says why there is none. Test paths are relative to the repo root;
`tests/golden/*` tests need the preserved data. Files of the author's ifarm MC
area (`archive/mc_legacy/jlab/`) were never under `_workdir/` (D9).

**`env_fsu/`, `gx1_export/`, `selectors/`**

| Legacy original | Archive path | Replaced by | Verified by |
|---|---|---|---|
| `_workdir/runDSelector.sh` | `env_fsu/runDSelector.sh` | `gxana run select` (`gxana/stages/select.py`), §9 | `packages/common/tests/python/test_select.py` |
| `_workdir/runMultiDSelector.sh` | `env_fsu/runMultiDSelector.sh` | `samples.yaml` + `gxana run select --sample S` | `packages/common/tests/python/test_config.py` |
| `_workdir/AnalysisNote/MC/gluex_MCwrapper/set_gluexenv.sh` | `env_fsu/set_gluexenv.sh` | `source env/setup.sh --gluex` | `tests/env/test_setup_sh.py` |
| `_workdir/kpkpxim_hjesse_gx1_analysis/PrepFlatTrees.C` | `gx1_export/PrepFlatTrees.C` | `analyses/kpkpxim/selection/flatTreePrep.C` (D18 rapidity fix) | `tests/selection/test_flat_tree_prep.py` |
| `_workdir/kpkpxim_hjesse_gx1_analysis/runDSelector.sh` | `gx1_export/runDSelector.sh` | `gxana run select` | `packages/common/tests/python/test_select.py` |
| `_workdir/kpkpxim_hjesse_gx1_analysis/data_analysis/{analysis/mc_studies/get_data_hists_RF.C, analysis/mc_studies/make_plot_RF.C, event_selection/chisqndf_cut/get_data_hists_RF.C}` (3) | `gx1_export/data_analysis/…` | the newer AnalysisNote copies: `analyses/kpkpxim/selection/mc_studies/{get_data_hists_RF,make_plot_RF}.C`, `selection/cut_studies/chisqndf_cut/get_data_hists_RF.C` | not ported — gx1 copies older than the migrated AnalysisNote ones |
| `_workdir/DSelector/{DSelector_kpkpxim_legacy.C, DSelector_pi0kpkpxim_1.C}` (2) | `selectors/` | `analyses/kpkpxim/selectors/DSelector_kpkpxim*.C`, `backgrounds/selectors/DSelector_pi0kpkpxim.C` | not ported — superseded drafts (D11) |

**`mc_weights/`, `xsection_old/`**

| Legacy original | Archive path | Replaced by | Verified by |
|---|---|---|---|
| `_workdir/AnalysisNote/mc_weights/**` (8: `get_data_hists.C`, `make_plot_{accept,acceptcorr,acceptcorr_pl,hangle}.C`, `WeightMC.C`, `archive/{get_data_hist_genr8,WeightMC_original}.C`) | `mc_weights/` | nothing (the thesis MC-weight study is `analyses/kpkpxim/systematics/mc_weight_variations/`, from `AnalysisNote/systematics/mc_weight_variations/`) | not ported — deprecated genr8 reweighting, not used in the thesis (D10) |
| `_workdir/AnalysisNote/xsection_old/*` (12: `GetChiSqPerBin.C`, `GetDiffXSec{,_UML}.C`, `PrepHistos.C`, `rf403_weightedevts.C`, `runXSec.sh`, `SplitFlatTrees.C`, `TestUMLFits{,RF}.C`, `Xim1320_{xsec,totxsec}_clas_gluex*.C`) | `xsection_old/` | `packages/xsection` (`gxana run xsection`); the CLAS comparison is `analyses/kpkpxim/xsection/PlotTotXsecWithClas.C` | not ported — early histogram/UML pipeline, superseded in `_workdir` by the `AnalysisNote/xsection` macros below |
| `_workdir/AnalysisNote/xsection_old/xsection_uml_old/*` (16: `MakeBinnedTrees.C`, `MakeXSec{,Files,Compare,Components}.C`, `Make{Combo,Component,Cut}Comparison*.C`, `UMLFits.C`, `canvas_weighted.C`, `test.C`, `RunXSec.py`, `GetXSecCompentFile.py`, `runMLFits.sh`, `clean.sh`) | `xsection_old/xsection_uml_old/` | `packages/xsection` (`gxana run xsection`) | not ported — older copies of the `AnalysisNote/xsection` macros below |

**`xsection_legacy/`** (from `_workdir/AnalysisNote/xsection/` unless noted)

| Legacy original | Archive path | Replaced by | Verified by |
|---|---|---|---|
| `FitFunctions.cpp`, `FitFunctions.h` | `xsection_legacy/FitFunctions.{cpp,h}` | `gxana/xsection/YieldFit.h` (`RooFitMC`, `RooFitData`, `AttemptFit`, `AttemptFitMC`, `constructFitString`), `XSec.h` (`GetDiffXSecFile`, `GetTotXSecFile`), `Flux.h` (`GetFluxHist`); app `gxana_xsec_tables` | `tests/golden/test_xsec_golden.py`; ctest `xsection.unit` |
| `FitFunctions_bak.cpp` | `xsection_legacy/FitFunctions_bak.cpp` | `FitFunctions.cpp` (row above) | not ported — backup copy |
| `PlotFunctions.cpp`, `PlotFunctions.h` | `xsection_legacy/PlotFunctions.{cpp,h}` | `gxana/common/Style.h` (`SetStyle`), `Strings.h` (`NumericCompare`), `GraphIO.h` (`CreateTGraphErrorsFromTxt`), `gxana/xsection/Plotting.h` (`plotDiffXSec`, `plotWeightedXSec`, `plotOneWeightedXSec`, `plotFinalWeightedXSec`) | ctest `common.style` (`packages/common/tests/cpp/test_style.cxx`), `test_common.cxx` |
| `MakeBinnedTrees.C`, `MakeBinnedTrees.cpp`, `MakeBinnedTrees.h` | `xsection_legacy/MakeBinnedTrees.{C,cpp,h}` | `gxana/xsection/Binning.h` (`divideNominalIntoBins`, `divideThrownIntoBins`); `gxana_xsec_bin data\|mc\|thrown`; `gxana run xsection --steps bin`; `binning.yaml`, `xsection.branches` | `tests/golden/test_binning_golden.py` |
| `MakeXSecFitVariations.C` (gx1 name `MakeXSection.C`) | `xsection_legacy/MakeXSecFitVariations.C` | `gxana_xsec_tables --fit Johnson\|Voigtian\|MCPdf`; `gxana run xsection --steps tables` (`xsection.fits`, label `johnson`) and `gxana run systematics --steps fit` (`systematics.variants`) | `tests/golden/test_xsec_golden.py` (`johnson`); `tests/golden/test_systematics_chain_golden.py` (loose) |
| `MakeXSecFiles.C` | `xsection_legacy/MakeXSecFiles.C` | `gxana_xsec_tables --fit JohnsonMCShape` (`RooFitMCShapeSeed`, `RooFitDataMCShape`); `systematics.variants` labels `hybrid_combo`, `best_combo`, `acc_weight` | `tests/golden/test_xsec_golden.py` (those three labels) |
| `MakeXSecFitMC.C` | `xsection_legacy/MakeXSecFitMC.C` | `gxana_xsec_tables --fit MCPdf` (`RooFitMCPdf`); labels `mcPdf`, `mcPdf_cheby1` | `tests/golden/test_systematics_chain_golden.py` (loose) |
| `MakeXSecFitVoigtian.C` | `xsection_legacy/MakeXSecFitVoigtian.C` | `gxana_xsec_tables --fit Voigtian`; labels `voigt`, `voigt_cheby1` | `tests/golden/test_systematics_chain_golden.py` (loose) |
| `MakeXSecFitBkgd.C` | `xsection_legacy/MakeXSecFitBkgd.C` | `gxana_xsec_tables --fit Johnson --cheby 1`; label `johnson_cheby1` | `tests/golden/test_systematics_chain_golden.py` (loose) |
| `MakeXSecFitSingleGaus.C`, `MakeXSecFitDoubleGaus.C` | `xsection_legacy/MakeXSecFit{Single,Double}Gaus.C` | none in a stage (`--fit Gaussian` exists, unconfigured); single-bin Gaussian fits are `analyses/kpkpxim/signal_extraction/lineshape/{Single,Double}GaussianFit.C` | not ported — fit models not in the published variants |
| `RooHistPdfFitTest.C`, `RooHistPdfFitTest_1.C` | `xsection_legacy/RooHistPdfFitTest{,_1}.C` | `RooFitMCPdf` (`--fit MCPdf`) | not ported — RooHistPdf test drafts |
| `MakeXSec.C` | `xsection_legacy/MakeXSec.C` | `gxana_xsection.weighted_average` (run-period average); `Plotting.h` plots via `analyses/kpkpxim/xsection/PlotDiffXSec.C` | not ported as such — its `MakeBinnedDiffXSec`, `calc_weightedavg`, `calc_totalxsec` and `SplitString` were dropped with it (averaging is done in Python) |
| `MakeXSecComponents.C` | `xsection_legacy/MakeXSecComponents.C` | `gxana_xsection.components` (`gxana run xsection --steps components`); plots `analyses/kpkpxim/xsection/PlotXSecComponents.C` | not ported as such — older component driver |
| `MakeXSecCompare.C` | `xsection_legacy/MakeXSecCompare.C` | run-period comparison: `gxana run systematics --steps compare` (`run_compare`) | not ported — early draft of the run comparison |
| `MakeCutComparison.C`, `MakeComponentComparison.C` | `xsection_legacy/Make{Cut,Component}Comparison.C` | none | not ported — comparison drafts not in the thesis |
| `GetWeightedXsecFile.py` | `xsection_legacy/GetWeightedXsecFile.py` | `gxana_xsection.weighted_average`; `gxana run xsection --steps weight` | `tests/golden/test_python_golden.py` |
| `GetXSecComponentFiles.py` | `xsection_legacy/GetXSecComponentFiles.py` | `gxana_xsection.components`; `--steps components` | `tests/golden/test_python_golden.py` |
| `MakeQValXSecFile.py` | `xsection_legacy/MakeQValXSecFile.py` | `gxana_xsection.qvalue_rescale`; `systematics.variants` `qvalue`, `gxana run systematics --steps qvalue` | `tests/golden/test_python_golden.py` |
| `MakeXsecTexTable.py`, `MakeXsecTexTable1.py`, `MakeXsecTexTableScale.py` | `xsection_legacy/MakeXsecTexTable{,1,Scale}.py` | `gxana_xsection.tex_table`; `gxana run xsection --steps tex` (`xsection.tex`) | `tests/golden/test_python_golden.py`, `packages/xsection/tests/python/test_tex_table.py` |

**`systematics_legacy/`** (from `_workdir/AnalysisNote/systematics/` unless noted)

| Legacy original | Archive path | Replaced by | Verified by |
|---|---|---|---|
| `GetVariationTreesUML.C` | `systematics_legacy/GetVariationTreesUML.C` | `gxana_barlow_trees` (`WriteVariationTrees`; `--check`: `CheckVariationYields`); `gxana run barlow --steps trees,check`; `barlow.yaml` `trees`, `nominal`, `families` | no golden — no variation trees preserved; ctest `barlow.unit`, `packages/barlow/tests/python/test_barlow_apps.py` |
| `SplitVariationTrees.C` | `systematics_legacy/SplitVariationTrees.C` | `divideVariationTreesIntoBins` (`Binning.h`); `gxana_xsec_bin variation`; `--steps bin` | `tests/golden/test_binning_golden.py` |
| `GetXSecFilesUML.C` | `systematics_legacy/GetXSecFilesUML.C` | `gxana_xsec_tables --fit JohnsonMCShapeSyst`; `--steps tables`; `barlow.fit` | no golden — no variation trees preserved (fit transcribed, KNOWN_ISSUES §2) |
| `run.sh` | `systematics_legacy/run.sh` | `gxana run barlow --steps tables` | not ported — one-line driver of `GetXSecFilesUML.C` |
| `GetWeightedXsecFile.py` | `systematics_legacy/GetWeightedXsecFile.py` | `gxana_xsection.weighted_average`; `gxana run barlow --steps weight` | `tests/golden/test_systematics_text_golden.py` |
| `PlotXSecBarlowChiSqNdf.C` | `systematics_legacy/PlotXSecBarlowChiSqNdf.C` | `gxana_barlow_plot` (`PlotBarlow`, `calc_barlow`); `barlow.families.chisqndf` | `tests/golden/test_barlow_plot_golden.py` |
| `PlotXSecBarlowMissingMass.C` | `systematics_legacy/PlotXSecBarlowMissingMass.C` | `gxana_barlow_plot`; `barlow.families.total_mm2_abs` | `tests/golden/test_barlow_plot_golden.py` |
| `PlotXSecBarlowXimFlightSig.C` | `systematics_legacy/PlotXSecBarlowXimFlightSig.C` | `gxana_barlow_plot`; `barlow.families.xim_pathlensig` | `tests/golden/test_barlow_plot_golden.py` |
| `PlotXSecBarlowLambdaFlightSig.C` | `systematics_legacy/PlotXSecBarlowLambdaFlightSig.C` | `gxana_barlow_plot`; `barlow.families.lambda_pathlensig` | `tests/golden/test_barlow_plot_golden.py` |
| `PlotXSecBarlowKHighRapidity.C` | `systematics_legacy/PlotXSecBarlowKHighRapidity.C` | `gxana_barlow_plot`; `barlow.families.kphigh_prap` | `tests/golden/test_barlow_plot_golden.py` |
| `PlotXSecBarlowKLowRapidity.C` | `systematics_legacy/PlotXSecBarlowKLowRapidity.C` | none (`kplow_prap` family not configured) | not ported — the variation had no trees |
| `GetXSecFitVariations.C` | `systematics_legacy/GetXSecFitVariations.C` | `MakeXSecFitVariations.C` port (`gxana run systematics --steps fit`) | not ported — retired draft |
| `GetDiffXSec.C` | `systematics_legacy/GetDiffXSec.C` | `packages/xsection` | not ported — histogram-based draft predating `FitFunctions.cpp` |
| `GetRunPeriodComparison.C` | `systematics_legacy/GetRunPeriodComparison.C` | `gxana run systematics --steps compare` (`run_compare`), `--steps runperiod` | not ported — draft of the run-period check |
| `TestUMLRecursive.C`, `TestUMLRecursive_1.C` | `systematics_legacy/TestUMLRecursive{,_1}.C` | none | not ported — UML fit tests |
| `track_efficiency/get_hists.C` | `systematics_legacy/track_efficiency/get_hists.C` | `gxana_syst_track` (`TrackHists.h` `MakeKinematics`); `gxana run systematics --steps track`; `systematics.studies.track` | `tests/golden/test_systematics_track_golden.py` |
| `track_efficiency/get_track_efficiency.C` | `systematics_legacy/track_efficiency/get_track_efficiency.C` | `gxana_syst_track` (`CountAndDraw`) + `gxana_systematics.track`; `--steps track` | `tests/golden/test_systematics_track_golden.py` |
| `track_efficiency/get_track_efficieny.C` | `systematics_legacy/track_efficiency/get_track_efficieny.C` | `get_track_efficiency.C` port (row above) | not ported — earlier misspelled draft |
| `_workdir/AnalysisNote/xsection/PlotComboComparison.C` | `systematics_legacy/comparisons/PlotComboComparison.C` | `gxana_syst_plot` layouts `grid3`, `pair_band` (`PlotSpread.h`) + `gxana_systematics.spread`; study `accidentals`, `--steps spread` | `tests/golden/test_systematics_plot_golden.py`, `test_systematics_numbers_golden.py` |
| `_workdir/AnalysisNote/xsection/PlotFitComparison.C` | `systematics_legacy/comparisons/PlotFitComparison.C` | `gxana_syst_plot` `grid3`, `all_band` + `gxana_systematics.spread`; study `fit` | `tests/golden/test_systematics_plot_golden.py`, `test_systematics_chain_golden.py` |
| `_workdir/AnalysisNote/xsection/PlotRunComparison.C` | `systematics_legacy/comparisons/PlotRunComparison.C` | `gxana_syst_plot` `run_grid`, `stddev_band` + `gxana_systematics.runcompare`; study `run_compare`, `--steps compare` | `tests/golden/test_systematics_plot_golden.py`; `packages/systematics/tests/python/test_runcompare.py` |
| `_workdir/AnalysisNote/xsection/PlotQValueComparison.C` | `systematics_legacy/comparisons/PlotQValueComparison.C` | `gxana_syst_plot` `grid2`; study `qval_yield`, `--steps compare` | `tests/golden/test_systematics_plot_golden.py` |
| `_workdir/AnalysisNote/xsection/PlotBunchComparison.C` | `systematics_legacy/comparisons/PlotBunchComparison.C` | `gxana_syst_plot` `grid2` (weighted overlay only); study `bunch`, `--steps compare` | no golden — label `oneRfBunch` not in the variant pool |
| `_workdir/AnalysisNote/xsection/PlotFitBkgdComparison.C` | `systematics_legacy/comparisons/PlotFitBkgdComparison.C` | `gxana_syst_plot` `grid2`; study `bkgd`, `--steps compare` | no golden — label `bkgd` not in the variant pool |
| `_workdir/AnalysisNote/xsection/PlotRestVComparison.C` | `systematics_legacy/comparisons/PlotRestVComparison.C` | none | not ported — REST-version check omitted pending re-evaluation (KNOWN_ISSUES §6) |

**`root_macros/`**

| Legacy original | Archive path | Replaced by | Verified by |
|---|---|---|---|
| `_workdir/AnalysisNote/utilities/AcceptanceCorrect.C` | `root_macros/AcceptanceCorrect.C` | `gxana/common/AcceptanceCorrect.h` (`Acceptance`, `AcceptanceCorrect`, `MergeCorrected`, `LostBins`) | `tests/golden/test_sampling_golden.py`; ctest `test_acceptance.cxx` |
| `_workdir/AnalysisNote/analysis/event_selection/CutAnalysisRF.C` | `root_macros/CutAnalysisRF.C` | `gxana_study_cutscan` (`CutScan.h`); `gxana run studies --study chisqndf_scan,mm2_scan` | `packages/studies/tests/python/test_cutscan_equivalence.py` (frozen copy `packages/studies/tests/legacy/CutAnalysisRF.C`) |
| `_workdir/AnalysisNote/analysis/event_selection/CutAnalysis.C` | `root_macros/CutAnalysis.C` | the `cutscan` study (`gxana run studies`) | fit function: `packages/fit/tests/python/test_legacy_sites.py`; no golden — inputs not preserved |
| `_workdir/AnalysisNote/analysis/CutAnalysis.C` | `root_macros/CutAnalysis_old_draft.C` | the `cutscan` study | not ported — old draft (different binning) |
| `_workdir/AnalysisNote/analysis/GetKinematicsDataMC_RF.C` | `root_macros/GetKinematicsDataMC_RF.C` | `gxana_study_datamc` (`DataMC.h`, `FillPeriodHists`); `gxana run studies --study kinematics` | `packages/studies/tests/python/test_kinematics_equivalence.py` (golden marker; frozen copy in `packages/studies/tests/legacy/`) |
| `_workdir/AnalysisNote/analysis/GetKinematicsDataMC.C` | `root_macros/GetKinematicsDataMC.C` | the `datamc` study | no golden — reads MC samples that are not preserved |
| `_workdir/AnalysisNote/analysis/analysis/cascade_properties/GetXimProperties.C` | `root_macros/GetXimProperties.C` | `analyses/kpkpxim/measurements/{mass/PrepMass,mass/FitMass,lifetime/PrepLifetime,lifetime/FitLifetime,spin/PrepSpinData}.C` + `common/XimInputs.h`; `gxana::fit::Moments`; `gxana run measurements` (`measurements.yaml`) | `tests/golden/test_measurements_golden.py`, `test_measurements_stage_golden.py` |
| `_workdir/AnalysisNote/utilities/flatTreePrepQVal_old.C` | `root_macros/flatTreePrepQVal_old.C` | `analyses/kpkpxim/selection/flatTreePrepQVal.C`, `flatTreePrep.C` | not ported — older copy |
| `_workdir/AnalysisNote/analysis/event_selection/lambda_vertex_cut/old/*` (4: `get_data_hists{,_old}.C`, `get_data_vertexdiff.C`, `make_plot.C`) | `root_macros/lambda_vertex_cut_old/` | `analyses/kpkpxim/selection/cut_studies/lambda_vertex_cut/` | not ported — superseded study |
| `_workdir/MakeHistos.C` | `root_macros/MakeHistos.C` | none | not ported — pre-AnalysisNote home-dir macro |
| `_workdir/PlotfromFlatTree.C`, `_workdir/PlotfromFlatTreeMC.C` | `root_macros/PlotfromFlatTree{,MC}.C` | none | not ported — pre-AnalysisNote home-dir macros |
| `_workdir/AnalysisNote/utilities/MakeXim1320_IM_Volker.C` | `root_macros/MakeXim1320_IM_Volker.C` | `analyses/kpkpxim/measurements/mass/MakeXim1320_IM.C` | not ported — variant of the mass-figure macro |

**`mc_legacy/`** (`jlab/` = the author's ifarm MC area; `local/` = `_workdir/AnalysisNote/MC/`)

| Legacy original | Archive path | Replaced by | Verified by |
|---|---|---|---|
| ifarm `gen_amp/runAllMC.sh` | `mc_legacy/jlab/gen_amp/runAllMC.sh` | `gxana run mc` (`gxana/stages/mc.py`) + `config/mc.yaml` (D26) | `packages/common/tests/python/test_mc_stage.py`, `test_simulation_config.py` |
| ifarm `gen_amp/xim_jlab_MC_gen_amp_201808_nobkg.conf` | `mc_legacy/jlab/gen_amp/xim_jlab_MC_gen_amp_201808_nobkg.conf` | `mc.yaml` sample `gen_amp_V2_nobkg` (`overrides: {BKG: None}`) on `simulation/mcwrapper/xim_jlab_MC_gen_amp_201808.conf` | `packages/common/tests/python/test_mc_stage.py` |
| ifarm `gen_amp/xim_jlab_MC_gen_amp_{2017_rest3,201808_l1}.conf` (2) | `mc_legacy/jlab/gen_amp/` | thesis confs `analyses/kpkpxim/simulation/mcwrapper/xim_jlab_MC_gen_amp_{2017,201801,201808}.conf` | not ported — pre-thesis (ana45 REST 3, `_l1`) productions |
| ifarm `gen_amp/gen_amp_cfg/*.cfg` (15) | `mc_legacy/jlab/gen_amp/gen_amp_cfg/` | thesis cfgs `analyses/kpkpxim/simulation/gen_amp_cfg/kpkpxim_2dhist_{ac,noac}_YstarRest.cfg` | not ported — non-thesis iterations (2D v2/ac1, 3D, π⁰, Λ(1520), early AmpTools) |
| ifarm `gen_amp/MakeMC.csh`, `MakeMC_old.csh` (2) | `mc_legacy/jlab/gen_amp/` | patched upstream `MakeMC.sh`/`.csh` (`packages/montecarlo/patches/gluex_MCwrapper/0001-*`, `0002-*`; `gxana externals fetch gluex_MCwrapper`) | not ported — loose copies; the locked sha256 is the MCwrapper checkout's file |
| ifarm `gen_amp/xim_hd_root_test.conf`, `hd_root_sigma.config`, `hd_root_xim13.config` (3) | `mc_legacy/jlab/` | `analyses/kpkpxim/simulation/hd_root/` | not ported — test and Σ configs; the kept `hd_root_xim13.config` is the `AnalysisNote/MC` copy |
| ifarm `MC.config`, `xim_jlab_MC.config`, `xim_jlab_MC_amptools.config`, `version.xml` (4) | `mc_legacy/jlab/` | `analyses/kpkpxim/simulation/mcwrapper/*.conf`, `mc.yaml`, `env/version_sets/*.xml.in` | not ported — pre-thesis MCwrapper configs and version set |
| ifarm `gen_amp_mod.cc` | `mc_legacy/jlab/gen_amp_mod.cc` | `packages/montecarlo/patches/halld_sim/0001-*` to `0004-*` | not ported — unbuilt draft of the gen_amp patch |
| ifarm `genr8/*.input` (14), `ystar_inputs/*.input` (12) | `mc_legacy/jlab/{genr8,ystar_inputs}/` | none (genr8 reference inputs kept in `analyses/kpkpxim/simulation/genr8/`) | not ported — pre-thesis genr8 Y* productions |
| `_workdir/AnalysisNote/MC/{getHist2D,getHist2D_s17_v3,getHist2D_test,exampleHist2D}.C` (4) | `mc_legacy/local/` | `analyses/kpkpxim/simulation/sampling/PrepSampling.C` (from `MC/getHist2D_gen_amp.C`) | not ported — earlier sampling macros |
| `_workdir/AnalysisNote/MC/gen_amp/{amptools,amptools_delta,kpkpxim_amptools}.cfg` (3), `MC/genr8/{KLamb,pimpipkplamb}.input` (2) | `mc_legacy/local/{gen_amp,genr8}/` | none | not ported — early generator configs |
| `_workdir/AnalysisNote/MC/MakeMC.csh` | `mc_legacy/local/MakeMC.csh` | patched upstream MCwrapper (`external.lock`) | not ported — copy of an unpatched MCwrapper script |
| `_workdir/AnalysisNote/MC/{MC.config,xim_jlab_MC.config}` (2) | `mc_legacy/local/` | `simulation/mcwrapper/*.conf`, `mc.yaml` | not ported — FSU-era configs |
| `_workdir/AnalysisNote/MC/version.xml` | `mc_legacy/local/version.xml` | upstream `version_5.12.0.xml` via `source env/setup.sh --gluex` (§7.2) | not ported — FSU copy of the 5.12.0 analysis set |

Count: 194 files; 58 in individual rows, 136 in 28 grouped rows. No file is
unmapped or marked unclear.

### 15.2 Legacy scripts migrated into `analyses/` that now call package code

These scripts stayed scripts (§5.2) but call package code in place of their
own copies. Macros that only resolve paths with `gxana::EnvPath` are not listed.
How each adoption was checked: `docs/PORT_NOTES.md` §8 (acceptance), §12 and
`tests/macros/test_macro_styles.py` (styles), §15 and
`packages/fit/tests/python/test_legacy_sites.py` (fits), §16 (period histograms,
overlays).

| Legacy path (`_workdir/`) | Live path (`analyses/`) | Package code it now uses |
|---|---|---|
| `kpkpkmlamb/FitXimStar.C`, `FitXimStarCuts.C` | `kpkpkmlamb/measurements/FitXimStar.C` | `gxana::fit` (`BreitWigner`, `Chebychev`, `Sum`, `BuildModel`, `RunFit`), `ApplyStyle(FitStyle)`; `gxana run measurements --channel kpkpkmlamb` |
| `AnalysisNote/utilities/KstarFit.C` | `kpkpxim/backgrounds/KstarFit.C` | `gxana::fit` (`Voigtian`, `Chebychev`, `Sum`, `BuildModel`, `RunFit`) |
| `AnalysisNote/utilities/YstarBWFitsData.C` | `kpkpxim/backgrounds/YstarBWFitsData.C` | `gxana::fit` (`BreitWigner`, `Sum`, `BuildModel`, `RunFit`), `ApplyStyle(CutStudyStyle)` |
| `AnalysisNote/utilities/MakeXim1320_IM.C`, `MakeXim1320_IM_Res.C` | `kpkpxim/measurements/mass/MakeXim1320_IM{,_Res}.C` | `gxana::fit` (`Johnson`, `Gaussian`, `Chebychev`, `Sum`, `BuildModel`, `RunFit`), `ApplyStyle(FitStyle)` |
| `AnalysisNote/analysis/analysis/cascade_properties/GetXimProperties.C` (split) | `kpkpxim/measurements/{mass,lifetime,spin}/Prep*.C`, `mass/FitMass.C`, `common/XimInputs.h` | `gxana::fit` (`Johnson`, `Gaussian`, `Threshold`, `Moments`, `JohnsonMoments`, `RunFit`), `AcceptanceCorrect`, `MergeCorrected`, `LostBins`, `ChannelInfo`, `gxana::xsec::SetFitStyle`; `gxana run measurements` |
| `AnalysisNote/analysis/analysis/cascade_properties/PlotGlueXSpin.C` | `kpkpxim/measurements/spin/PlotGlueXSpin.C` (with `PrepSpinData.C`) | `AcceptanceCorrect`, `MergeCorrected` (in `PrepSpinData.C`); `gxana run measurements` |
| `AnalysisNote/xsection/OneUMLFit.C`, `SingleGaussianFit.C`, `DoubleGaussianFit.C` | `kpkpxim/signal_extraction/lineshape/` | `gxana::fit` (`ImportTree`, `FirstPopulatedEdge`, `Gaussian`, `Voigtian`, `Threshold`, `Chebychev`, `Sum`, `BuildModel`, `RunFit`), `gxana::xsec::AttemptFit`, `AttemptFitMC`, `constructFitString` (`OneUMLFit.C`), `ApplyStyle(FitStyle)` |
| `AnalysisNote/QFactors/scripts/GetQvalueSum.C` | `kpkpxim/signal_extraction/qfactors/scripts/GetQvalueSum.C` | `gxana::fit` (`Johnson`, `Chebychev`, `FirstPopulatedEdge`, `BuildModel`, `RunFit`), `ApplyStyle(CutStudyStyle)` |
| `AnalysisNote/analysis/event_selection/qfactors/XimMassQVal.C` | `kpkpxim/selection/XimMassQVal.C` | `ApplyStyle(FitStyle)` |
| `AnalysisNote/utilities/flatTreeCuts.C` | `kpkpxim/selection/flatTreeCuts.C` | `ApplyStyle(CutStudyStyle)` |
| `AnalysisNote/analysis/event_selection/{accidentals,chisqndf_cut,kaon_selection,lambda_vertex_cut,mm2_cut,xim_vertex_cuts}/make_plot*.C`, `accidentals/get_data_hists.C` | `kpkpxim/selection/cut_studies/<cut>/` | `ApplyStyle(CutStudyStyle)` or `ApplyStyle(DistributionStyle)` |
| `AnalysisNote/analysis/event_selection/rapidity_cuts/Plot*.C` (8) | `kpkpxim/selection/cut_studies/rapidity_cuts/` | `ApplyStyle(DistributionStyle)` |
| `AnalysisNote/analysis/analysis/mc_studies/get_data_hists_RF.C` | `kpkpxim/selection/mc_studies/get_data_hists_RF.C` | `Acceptance`, `AcceptanceCorrect`, `ChannelInfo`, `MakePeriods`, `FillPeriodHists`, `GetPeriodHists`, `MergeHists` |
| `AnalysisNote/analysis/analysis/mc_studies/get_data_hists.C` | `kpkpxim/selection/mc_studies/get_data_hists.C` | `Acceptance`, `AcceptanceCorrect` |
| `AnalysisNote/analysis/analysis/mc_studies/{make_plot,make_plot_RF,make_plot_acceptcorr}.C` | `kpkpxim/selection/mc_studies/` | `ApplyStyle(CutStudyStyle)` |
| `AnalysisNote/MC/getHist2D_gen_amp.C` | `kpkpxim/simulation/sampling/PrepSampling.C` | `AcceptanceCorrect`, `MergeCorrected`, `LostBins`, `ChannelInfo`, `MakePeriods`, `FillPeriodHists`, `GetPeriodHists`, `MergeHists` |
| `AnalysisNote/MC/getHist3D.C`, `getHist3D_F18.C` | `kpkpxim/simulation/sampling/` | `Acceptance`, `AcceptanceCorrect` (acceptance functions only) |
| `AnalysisNote/MC/compare_iters.C`, `compare_iters_2D.C`, `MC/tree_3d/in_out_test.C` | `kpkpxim/simulation/validation/` | `DrawOverlay` (`Overlay.h`); `ChannelInfo` (`in_out_test.C`) |
| `AnalysisNote/MC/tree_3d/get_data_hists_RF.C`, `make_plot_RF.C` | `kpkpxim/simulation/validation/` | `Acceptance`, `AcceptanceCorrect`, `ChannelInfo`, period fills (`get_data_hists_RF.C`); `DrawOverlay`, `ApplyStyle(CutStudyStyle)` (`make_plot_RF.C`) |
| `AnalysisNote/systematics/mc_weight_variations/get_data_hists.C`, `WeightMC.C` | `kpkpxim/systematics/mc_weight_variations/` | `Acceptance`, `AcceptanceCorrect`, `ChannelInfo`, `FillPeriodHists`, `GetPeriodHists` (`get_data_hists.C`); `ApplyStyle(ComparisonStyle)` (`WeightMC.C`) |
| `AnalysisNote/systematics/GetRunPeriodPctSig.C` | `kpkpxim/systematics/GetRunPeriodPctSig.C` | `ApplyStyle(BarlowStyle)`; run by `gxana run systematics --steps runperiod` (golden `tests/golden/test_runperiod_golden.py` against `tests/golden/legacy/runperiod/GetRunPeriodPctSig.C`) |
| `AnalysisNote/xsection/PlotDiffXSec.C`, `PlotComponents.C` | `kpkpxim/xsection/` | `gxana::xsec::plotDiffXSec`, `plotWeightedXSec`, `plotOneWeightedXSec`, `plotFinalWeightedXSec`, `SetPlotDir`; `CreateTGraphErrorsFromTxt`, `SetStyle` |
| `AnalysisNote/xsection/PlotTotXsecWithClas.C`, `PlotXSecComponents.C` | `kpkpxim/xsection/` | `ApplyStyle(ThesisStyle)`, `ApplyStyle(ComparisonStyle)` |
