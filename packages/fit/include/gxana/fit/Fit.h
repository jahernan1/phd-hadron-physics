#ifndef GXANA_FIT_FIT_H
#define GXANA_FIT_FIT_H

// Data import, the fit call and the fit-window edge shared by the analysis macros.

#include <RooAbsData.h>
#include <RooAbsPdf.h>
#include <RooFitResult.h> // complete type for the fitTo return value (ROOT >= 6.30)

#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>

class RooDataSet;
class RooRealVar;
class TH1;
class TTree;

namespace gxana {
namespace fit {

// new RooDataSet("data", "Dataset of mass", RooArgSet(obs, w), Import(tree), WeightVar(w)) with
// w = RooRealVar(weightName, "weight", -10, 10): the import of every tree-based macro fit.
// Entries with |weight| > 10 or obs outside its range are dropped (RooFit import). The caller
// owns the result.
RooDataSet* ImportTree(TTree& tree, RooRealVar& obs, const std::string& weightName);

// One line "FITRESULT trace <model> <par>=<v> <par>_err=<e> ..." (values "%.17g") over
// model.getParameters(data), constants included, in that set's order.
void TraceFit(std::ostream& os, const RooAbsPdf& model, const RooAbsData& data);

// model.fitTo(data, args...) with exactly the arguments given, in the given order; nothing is
// added (no Save; the minimiser and backend come from UseThesisMinimizer's process-wide
// defaults when the caller has set them). With GXANA_FIT_TRACE set (non-empty) it
// then prints TraceFit to stdout. ROOT 6.24 accepts at most 8 arguments here.
template <typename... Args>
void RunFit(RooAbsPdf& model, RooAbsData& data, const Args&... args)
{
    model.fitTo(data, args...);
    const char* trace = std::getenv("GXANA_FIT_TRACE");
    if (trace && *trace) TraceFit(std::cout, model, data);
}

// h.GetXaxis()->GetBinLowEdge(h.FindFirstBinAbove(threshold, 1, firstBin, h.FindBin(lastX)) + binOffset):
// low edge of the first bin above threshold between bin firstBin and the bin of lastX, moved by
// binOffset bins. No guard: when no bin qualifies FindFirstBinAbove returns -1, as in the macros.
double FirstPopulatedEdge(TH1& h, double threshold, int firstBin, double lastX, int binOffset = 0);

// Process-wide fit defaults of ROOT 6.24, the version the thesis results were produced with:
// ROOT::Math::MinimizerOptions default minimiser "Minuit" (TMinuit) with "Migrad", which
// RooAbsPdf::fitTo uses when no Minimizer(...) argument is given and TH1::Fit/TGraph::Fit use;
// on ROOT >= 6.32 also RooFit's legacy likelihood evaluation backend (the 6.24 evaluator).
// Newer ROOT defaults to Minuit2 and the new backend. Called by rootlogon.C and by the apps
// that fit; affects every later fit in the process.
void UseThesisMinimizer();

} // namespace fit
} // namespace gxana

#endif
