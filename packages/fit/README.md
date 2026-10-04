# packages/fit

- `include/gxana/fit/`, `src/` — `GxanaFit` C++/ROOT library (namespace
  `gxana::fit`) for the analysis macros' RooFit lineshape fits:
  factory-statement builders (`Johnson`, `Gaussian`, `Voigtian`,
  `BreitWigner`, `Chebychev`, `Threshold`, `Sum`; the numbers are the
  caller's own text, `Fx` writes a double as `%f`), `BuildModel` (issues
  the statements in order), `RunFit` (`fitTo` with exactly the given
  arguments), `ImportTree` (weighted tree import, weights within ±10),
  `FirstPopulatedEdge` (fit-window edge) and `Moments` (Johnson mean and
  σ with the mass macro's error expressions), and `UseThesisMinimizer`.
  No function adds a fit argument. Loaded into ROOT by the repository's
  `rootlogon.C`.
- Minimiser pin: `UseThesisMinimizer()` sets the process-wide fit defaults
  of ROOT 6.24, the version of the thesis results: default minimiser
  `Minuit`/`Migrad` (TMinuit; newer ROOT defaults to Minuit2), used by
  `fitTo` calls without a `Minimizer(...)` argument and by `TH1::Fit`/
  `TGraph::Fit`, and on ROOT ≥ 6.32 RooFit's legacy evaluation backend.
  `rootlogon.C` calls it after loading the libraries, and so do the apps
  that fit (`gxana_xsec_tables`, `gxana_barlow_trees`,
  `gxana_study_cutscan`). TMinuit keeps one static instance, so a fit
  starts from the state the previous fit in the same process left (as in
  ROOT 6.24); the cut-scan app therefore fits all blocks in one process.
  Not pinned: macros run without `rootlogon.C` (`root -n`) or without
  `GXANA_ROOT` set, and the Q-factor fork (`packages/qfactors`, which
  passes `Minimizer("Minuit","migrad")` itself but keeps the newer
  backend).
- Tracing: with `GXANA_FIT_TRACE=1`, `BuildModel` prints
  `FACTORY <statement>` and `RunFit` prints
  `FITRESULT trace <model> <par>=<value> <par>_err=<error> ...` (`%.17g`).
  Off by default.
- `tests/` — ctest `fit.unit` (`tests/cpp/test_fit.cxx`) and `fit.cling`
  (the library from the interpreter; compiled and interpreted Johnson
  moments are bit-identical). Pytest `tests/python/test_legacy_sites.py` checks each adopted macro without preserved inputs against a frozen copy of its original fit function (`tests/legacy_sites/`, see its README) on seeded synthetic histograms. Pytest `tests/python/test_minimizer_pin.py` checks that `rootlogon.C` sets the pin (RooFit default, backend, `TH1::Fit`) and that the fitting apps call it.
