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
| `XimMassQVal.C` | Ξ⁻ mass and χ²/ndf with Q-value weights; `xim_qacc_*.pdf` and `chisqndf_qvalue_*.pdf` in the current directory. |
| `CompareFromTree.C` | Compares `decayxim_M` of the `gen_amp_V2_ac_YstarRest` and `gen_amp_V2_nobkg` MC flat trees. |

`CutAnalysis.C` and `GetKinematicsDataMC.C` were archived (in `archive/root_macros/`) on 2026-10-02: they are superseded by the `cutscan` and `datamc` studies (`gxana run studies`), and their inputs are not preserved.

`flatTreeCuts.C`, `flatTreeCutsMC.C`, `flatTreePlots.C` (early cut-and-fit macros, superseded by `flatTreePrep.C` and `cut_studies/`), `flatTreePrepQVal.C` (early variant of `flatTreePrep.C`) and `weighted_unbinned_fit.C` (scratch test on a local `test_tree.root`) were archived (in `archive/root_macros/`) on 2026-10-04.

## Subdirectories

- [`cut_studies/`](cut_studies/README.md): chapter 4 cut studies.
- [`mc_studies/`](mc_studies/README.md): chapter 5 data versus
  acceptance-corrected MC.
- `tests/make_raw_tree.C`: `make_raw_tree(const char* out)` writes a synthetic
  20-event raw flat tree with the branches `flatTreePrep.C` reads, so the prep
  can be tested without GlueX data.
