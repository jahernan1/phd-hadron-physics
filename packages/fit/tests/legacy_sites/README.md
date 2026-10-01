# Legacy fit sites (equivalence checks for gxana::fit)

Each `*.C` here except `run_site.C` is a FROZEN copy of an analysis macro's
fit function as it was before it moved to `gxana::fit`, made by `freeze.py`
from the macro at the commit before the port (the header names the origin
and the line ranges). The copy is the original text with two mechanical
changes: `w->factory(` becomes `LegacyFactory(w, ` and
`LegacyFitDone(w, data);` follows every `fitTo` statement
(`LegacyTrace.h`). Both print what `gxana::fit` prints under
`GXANA_FIT_TRACE=1`, so `../python/test_legacy_sites.py` can compare the
frozen copy and the adopted macro on the same seeded synthetic input
(`Synthetic.h`), each in its own ROOT process (`run_site.C`): `FITRESULT`
lines identical, `FACTORY` statements identical once blanks are removed,
all other output identical. Do not edit the frozen copies.

Adding a site: `python3 freeze.py <macro> <site>.C "<macro path> at <commit> (from AnalysisNote/...)" 1:<last include line> <first>:<last line of the fit function>`,
then add its entry to `SITES` in `../python/test_legacy_sites.py` and run
the test before porting the macro (it must pass against the unported
macro too).
