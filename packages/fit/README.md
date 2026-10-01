# packages/fit

- `include/gxana/fit/`, `src/` — `GxanaFit` C++/ROOT library (namespace
  `gxana::fit`) for the analysis macros' RooFit lineshape fits:
  factory-statement builders (`Johnson`, `Gaussian`, `Voigtian`,
  `BreitWigner`, `Chebychev`, `Threshold`, `Sum`; the numbers are the
  caller's own text, `Fx` writes a double as `%f`), `BuildModel` (issues
  the statements in order), `RunFit` (`fitTo` with exactly the given
  arguments), `ImportTree` (weighted tree import, weights within ±10),
  `FirstPopulatedEdge` (fit-window edge) and `Moments` (Johnson mean and
  σ with the mass macro's error expressions). It changes no global ROOT
  state and adds no fit argument. Loaded into ROOT by the repository's
  `rootlogon.C`.
- Tracing: with `GXANA_FIT_TRACE=1`, `BuildModel` prints
  `FACTORY <statement>` and `RunFit` prints
  `FITRESULT trace <model> <par>=<value> <par>_err=<error> ...` (`%.17g`).
  Off by default.
- `tests/` — ctest `fit.unit` (`tests/cpp/test_fit.cxx`) and `fit.cling`
  (the library from the interpreter; compiled and interpreted Johnson
  moments are bit-identical).
