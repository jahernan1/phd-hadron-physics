# Legacy fixtures of packages/studies

- `CutAnalysisRF.C`: FROZEN copy of the cut-scan macro the `cutscan` studies replace (the
  header names the commit). `../python/test_cutscan_equivalence.py` runs it and
  `gxana run studies --study chisqndf_scan,mm2_scan` on the same seeded toy raw trees and
  requires identical tables, printed fit lines and PDFs. Its `rooFitHist` is also the adopted
  side of the `CutAnalysisRF` entry of `packages/fit/tests/python/test_legacy_sites.py`, and its
  `setStyle()` a site of `tests/macros/test_macro_styles.py`. Do not edit.
- `GetKinematicsDataMC_RF.C`: FROZEN copy of the kinematics macro the `kinematics` study replaces
  (the header names the commit; the body is the macro unchanged). `../python/test_kinematics_equivalence.py`
  calls its `GetDataMCPlots` for the 2018-08 period with `n_threads = 0` on the preserved trees and runs
  the `kinematics` study planned for that period (`--threads 0`, sequential: RDataFrame's automatic
  binning of the data histogram differs between a sequential and an implicit-MT fill), and requires
  the same 36 PDF files, the same printed lines in the same order and identical rasters of four PDFs. Do not edit.
- `make_cutscan_toy.C`: `make_cutscan_toy(out, seed, n)` writes a seeded toy raw flat tree
  (`flatTree_kpkpxim`) with every branch the macro reads: a Ξ⁻ peak on a flat background,
  accidental weights of -0.25, `best_combo_rf` as an int as in the real trees.
