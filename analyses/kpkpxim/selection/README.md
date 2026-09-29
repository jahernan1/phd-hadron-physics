# selection

Event selection after the DSelectors: nominal cuts, flat-tree production and
the cut and MC studies of dissertation chapters 4 and 5.

## Pipeline macro

| Macro | Purpose |
|---|---|
| `flatTreePrep.C` | Pipeline step 2. `flatTreePrep("flatTree_<stem>")` reads `$GXANA_DATA/Trees/flatTree/rawTrees/<name>.root`, applies the nominal cuts and writes `$GXANA_DATA/flatTrees/<name>_nominal.root` plus the variants `_nominal_ximVertexCut`, `_nominal_kphighrap` and `_nominal_rapidityCuts` (the `_allKaonSep` and `_momCut` variants are commented out). `flatTreePrepAll()` lists every data and MC stem. Run: `root -l -b -q $GXANA_ROOT/rootlogon.C '$GXANA_ROOT/analyses/kpkpxim/selection/flatTreePrep.C("flatTree_<stem>")'` |

## Studies and helper macros (not part of the pipeline)

| Macro | Purpose |
|---|---|
| `CutAnalysis.C`, `CutAnalysisRF.C` | Ξ⁻ mass fits on the raw trees for a scan of one cut at a time (χ²/ndf, missing mass², Ξ⁻ path-length significance around the nominal values 8, 0.02, 2), with the signal-yield ratio versus cut value; PDFs into `$GXANA_OUTPUT/kpkpxim/cut_analysis_plots/results/`. The `RF` version is the `kphighrap` (gen_amp_V2) variant. |
| `GetKinematicsDataMC.C`, `GetKinematicsDataMC_RF.C` | Data (Q-factor weighted) versus MC kinematic distributions; PDFs into `$GXANA_OUTPUT/kpkpxim/data_mc_kinematics/` (`_ac.pdf` suffix for the RF version). Their default arguments name legacy MC stems; pass the gen_amp_V2 stems. |
| `XimMassQVal.C` | Ξ⁻ mass and χ²/ndf with Q-value weights; `xim_qacc_*.pdf` and `chisqndf_qvalue_*.pdf` in the current directory. |
| `CompareFromTree.C` | Compares `decayxim_M` of the `gen_amp_V2_ac_YstarRest` and `gen_amp_V2_nobkg` MC flat trees. |
| `flatTreeCuts.C`, `flatTreeCutsMC.C`, `flatTreePlots.C` | Early cut-and-fit macros (chisqndf < 4, xim_pathlensig > 2, ...) on the `Trees/flatTree/` layout of the legacy AnalysisNote; `flatTreeCutsMC.C` reads legacy genr8 files. Superseded by `flatTreePrep.C` and `cut_studies/`. |
| `flatTreePrepQVal.C` | Early variant of the flat-tree preparation (older cut values, best-combo selection, Ξ⁻ mass window snapshot `_xiMassCut` commented out); superseded by `flatTreePrep.C`. |
| `weighted_unbinned_fit.C` | Scratch test of a weighted unbinned fit on a local `test_tree.root`. |

## Subdirectories

- [`cut_studies/`](cut_studies/README.md): chapter 4 cut studies.
- [`mc_studies/`](mc_studies/README.md): chapter 5 data versus
  acceptance-corrected MC.
- `tests/make_raw_tree.C`: `make_raw_tree(const char* out)` writes a synthetic
  20-event raw flat tree with the branches `flatTreePrep.C` reads, so the prep
  can be tested without GlueX data.
