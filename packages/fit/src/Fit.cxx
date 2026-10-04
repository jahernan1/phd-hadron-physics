#include "gxana/fit/Fit.h"

#include <RooArgSet.h>
#include <RooDataSet.h>
#include <RooRealVar.h>
#include <TH1.h>
#include <TTree.h>

#include <Math/MinimizerOptions.h>
#include <RVersion.h>
#include <RooGlobalFunc.h>

#include <cstdio>
#include <memory>

namespace gxana {
namespace fit {

RooDataSet* ImportTree(TTree& tree, RooRealVar& obs, const std::string& weightName)
{
    RooRealVar weight(weightName.c_str(), "weight", -10, 10);
    return new RooDataSet("data", "Dataset of mass", RooArgSet(obs, weight), RooFit::Import(tree),
                          RooFit::WeightVar(weight));
}

void TraceFit(std::ostream& os, const RooAbsPdf& model, const RooAbsData& data)
{
    std::unique_ptr<RooArgSet> pars{model.getParameters(data)};
    os << "FITRESULT trace " << model.GetName();
    char buf[256];
    for (const auto* arg : *pars) {
        const auto* v = dynamic_cast<const RooRealVar*>(arg);
        if (!v) continue;
        std::snprintf(buf, sizeof(buf), " %s=%.17g %s_err=%.17g", v->GetName(), v->getVal(), v->GetName(),
                      v->getError());
        os << buf;
    }
    os << std::endl;
}

double FirstPopulatedEdge(TH1& h, double threshold, int firstBin, double lastX, int binOffset)
{
    return h.GetXaxis()->GetBinLowEdge(h.FindFirstBinAbove(threshold, 1, firstBin, h.FindBin(lastX)) + binOffset);
}

void UseThesisMinimizer()
{
    ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit", "Migrad");
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 32, 0)
    RooFit::EvalBackend::defaultValue() = RooFit::EvalBackend::Value::Legacy;
#endif
}

} // namespace fit
} // namespace gxana
