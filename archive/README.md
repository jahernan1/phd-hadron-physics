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
| `systematics_legacy/` (20 files) | `GetVariationTrees.C`, `GetXSecFiles.C`, `GetDiffXSec.C`, `GetRunPeriodComparison.C`, `TestUMLRecursive{,_1}.C`, `track_efficiency/get_track_efficieny.C`, `GetXSecFitVariations.C` (retired draft; see Task 10), and the legacy UML variation chain now replaced by `gxana run barlow`: `GetVariationTreesUML.C`, `PlotXSecBarlow{ChiSqNdf,MissingMass,XimFlightSig,LambdaFlightSig,KHighRapidity,KLowRapidity}.C`, `SplitVariationTrees.C`, `GetXSecFilesUML.C`, `GetWeightedXsecFile.py`, `run.sh`, `GetBarlowResults.C` (read `xsection/data_files` names nothing produces) | pre-package | UML pipeline in `packages/barlow`; the UML chain by `gxana run barlow` (`packages/barlow`: `gxana_barlow_trees`, `gxana_barlow_plot`, `gxana_xsec_bin variation`, `gxana_xsec_tables --fit JohnsonMCShapeSyst`, `gxana_xsection.weighted_average`) |
| `selectors/` (2 files) | `DSelector_kpkpxim_legacy.C`, `DSelector_pi0kpkpxim_1.C` | superseded selector drafts | the main `DSelector_kpkpxim*` family |
| `root_macros/` (19 files) | `MakeHistos.C`, `MakeHistoQVal.C`, `PlotfromFlatTree{,MC}.C`, `CutAnalysis_old_draft.C` (old draft of `analysis/CutAnalysis.C`), `flatTreePrepQVal_old.C`, `MakeXim1320_IM_Volker.C`, `AcceptanceCorrect.C`, `lambda_vertex_cut_old/`, kpkpkmlamb_get_data_hists.C (a kpkpxim χ²/ndf histogram macro that sat in the legacy kpkpkmlamb folder); the four Ξ⁻(1320) property macros listed under Notable files | AnalysisNote-era pipeline | current `analyses/kpkpxim` macros |
| `gx1_export/` (5 files) | Files from an earlier export of this analysis (`kpkpxim_hjesse_gx1_analysis`) that are not byte-identical to any migrated or `AnalysisNote` file: `PrepFlatTrees.C`, `runDSelector.sh`, `data_analysis/analysis/mc_studies/{get_data_hists_RF.C,make_plot_RF.C}`, `data_analysis/event_selection/chisqndf_cut/get_data_hists_RF.C` | export snapshot | this repo (byte-identical files were dropped as duplicates) |
| `env_fsu/` (3 files) | `set_gluexenv.sh`, `runDSelector.sh`, `runMultiDSelector.sh` | FSU-cluster environment scripts | `env/` + `gxana run select` |
| `mc_legacy/` (68 files) | Pre-thesis MC: early gen_amp/AmpTools cfgs, non-thesis `gen_amp_cfg` iterations (2D v2/ac1/3D/pi0/L1520), MCwrapper confs (ana45 `_rest3`, `_l1`, `_nobkg` — now a `BKG` override in `config/mc.yaml`), genr8 and `ystar_inputs/`, early `getHist2D*`, `exampleHist2D.C`, `gen_amp_mod.cc` (unbuilt draft of the gen_amp patch), version-set XMLs. `local/` = FSU copy, `jlab/` = the author's JLab ifarm MC area | pre-thesis | `analyses/kpkpxim/simulation` + `packages/montecarlo` |

## Notable files

- `xsection_legacy/RunAnalysis.sh` and `xsection_legacy/RunXSec.py` never
  ran as checked in (their logic was ported piecemeal into the packages
  during earlier tasks); kept here only for provenance.
- `root_macros/CutAnalysis_old_draft.C` differs from the migrated
  `analyses/kpkpxim/selection/CutAnalysis.C` in binning.
- `root_macros/CutAnalysisRF.C`: the RF cut scans of the thesis selection; replaced by the
  `cutscan` studies `chisqndf_scan` and `mm2_scan` (`gxana run studies`, `packages/studies`).
  Tables, printed fit lines and PDFs are identical on seeded toy raw trees
  (`packages/studies/tests/python/test_cutscan_equivalence.py`, against a frozen copy in
  `packages/studies/tests/legacy/`).
- `root_macros/GetKinematicsDataMC_RF.C`: the thesis data/MC kinematics figures; replaced by the
  `datamc` study `kinematics` (`gxana run studies`, `packages/studies`). On the preserved kphighrap
  trees every histogram, the PDF list and the PDFs are identical (checked single-threaded when it was
  archived; `docs/KNOWN_ISSUES.md` §22).
- `root_macros/GetXimProperties.C`: the combined Ξ⁻(1320) mass, lifetime and
  spin macro as migrated (paths already on `gxana::EnvPath`); replaced by the
  prep and fit macros in `analyses/kpkpxim/measurements/{mass,lifetime,spin}/`.
- `root_macros/Xim1320Properties.cpp`: an older near-duplicate of
  `GetXimProperties.C`; replaced by `analyses/kpkpxim/measurements/{mass,lifetime,spin}/`.
- `root_macros/Xim1320Properties.h`: the header of `Xim1320Properties.cpp`;
  replaced by `analyses/kpkpxim/measurements/{mass,lifetime,spin}/`.
- `root_macros/PlotXim1320Properties.C`: calls `GetXim1320_IM`, declared in
  `Xim1320Properties.h` but never defined, so it never ran; replaced by
  `analyses/kpkpxim/measurements/{mass,lifetime,spin}/`.
