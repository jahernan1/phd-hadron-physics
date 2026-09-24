# Archive

Code-only copies of superseded legacy code, kept for provenance. No outputs
(`.root .pdf .png .eps .ps .so .o .d .pcm` etc.) and nothing over 1 MiB is
archived — see `scripts/archive_copy.sh`.

Archived code is kept verbatim for provenance; it contains legacy site
paths and is not built, loaded or tested. It is excluded from
`tests/test_no_legacy_paths.py` and `tests/macros/test_macros_load.py`
(both scan only `analyses/`, `packages/`, `env/`, `scripts/`).

| Dir | Content | Era | Superseded by |
|---|---|---|---|
| `mc_weights/` (8 files) | `AnalysisNote/mc_weights/**` incl. its own `archive/` subdir (genr8 reweighting scheme) | pre-thesis | not used in thesis |
| `xsection_old/` (28 files) | `AnalysisNote/xsection_old/**` | early cross-section pipeline | `packages/xsection` |
| `xsection_legacy/` (30 files) | Pre-package xsection macros: `MakeXSec.C`, `MakeXSecCompare.C`, `MakeXSecComponents.C`, `MakeCutComparison.C`, `MakeComponentComparison.C`, `RunXSec.py`, `MakeXSecFiles.C`, `MakeXSecFit{Bkgd,SingleGaus,DoubleGaus,Voigtian,MC}.C`, `FitFunctions_bak.cpp`, `RooHistPdfFitTest{,_1}.C`, `MakeBinnedTrees.{C,cpp,h}`, `MakeXSecFitVariations.C`, `FitFunctions.{cpp,h}`, `PlotFunctions.{cpp,h}`, `GetWeightedXsecFile.py`, `GetXSecComponentFiles.py`, `MakeQValXSecFile.py`, `MakeXsecTexTable{,1,Scale}.py`, `RunAnalysis.sh` | pre-package | `FitFunctions`/`PlotFunctions` in `packages/xsection` + `MakeXSecFitVariations` |
| `systematics_legacy/` (8 files) | `GetVariationTrees.C`, `GetXSecFiles.C`, `GetDiffXSec.C`, `GetRunPeriodComparison.C`, `TestUMLRecursive{,_1}.C`, `track_efficiency/get_track_efficieny.C`, `GetXSecFitVariations.C` (retired draft; see Task 10) | pre-package | UML pipeline in `packages/systematics` |
| `selectors/` (2 files) | `DSelector_kpkpxim_legacy.C`, `DSelector_pi0kpkpxim_1.C` | superseded selector drafts | the main `DSelector_kpkpxim*` family |
| `root_macros/` (12 files) | `MakeHistos.C`, `MakeHistoQVal.C`, `PlotfromFlatTree{,MC}.C`, `CutAnalysis_old_draft.C` (old draft of `analysis/CutAnalysis.C`), `flatTreePrepQVal_old.C`, `MakeXim1320_IM_Volker.C`, `AcceptanceCorrect.C`, `lambda_vertex_cut_old/` | AnalysisNote-era pipeline | current `analyses/kpkpxim` macros |
| `gx1_export/` (5 files) | Files from an earlier export of this analysis (`kpkpxim_hjesse_gx1_analysis`) that are not byte-identical to any migrated or `AnalysisNote` file: `PrepFlatTrees.C`, `runDSelector.sh`, `data_analysis/analysis/mc_studies/{get_data_hists_RF.C,make_plot_RF.C}`, `data_analysis/event_selection/chisqndf_cut/get_data_hists_RF.C` | export snapshot | this repo (byte-identical files were dropped as duplicates) |
| `env_fsu/` (3 files) | `set_gluexenv.sh`, `runDSelector.sh`, `runMultiDSelector.sh` | FSU-cluster environment scripts | `env/` + `gxana run select` |

`mc_legacy/` (early gen_amp cfgs, genr8 non-Ξ inputs, `ystar_inputs/`, MC
config files) is **not populated here** — it depends on the simulation
migration and is deferred to Plan 4.

## Notable files

- `xsection_legacy/RunAnalysis.sh` and `xsection_legacy/RunXSec.py` never
  ran as checked in (their logic was ported piecemeal into the packages
  during earlier tasks); kept here only for provenance.
- `root_macros/CutAnalysis_old_draft.C` differs from the migrated
  `analyses/kpkpxim/selection/CutAnalysis.C` in binning.
