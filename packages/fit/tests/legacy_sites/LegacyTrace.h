// Trace shim for the frozen legacy fit functions in this directory and for instrumented
// copies of original macros: prints what gxana::fit prints under GXANA_FIT_TRACE, from the
// legacy calls (freeze.py rewrites "w->factory(" to "LegacyFactory(w, " and adds
// "LegacyFitDone(w, data);" after each fitTo statement).
#ifndef GXANA_FIT_LEGACY_TRACE_H
#define GXANA_FIT_LEGACY_TRACE_H

#include "gxana/fit/Fit.h"

#include <RooAbsData.h>
#include <RooWorkspace.h>

#include <iostream>

inline RooAbsArg* LegacyFactory(RooWorkspace* w, const char* statement)
{
    std::cout << "FACTORY " << statement << std::endl;
    return w->factory(statement);
}

inline void LegacyFitDone(RooWorkspace* w, RooAbsData* data)
{
    gxana::fit::TraceFit(std::cout, *w->pdf("model"), *data);
}

#endif
