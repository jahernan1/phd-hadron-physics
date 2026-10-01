#include "gxana/common/AcceptanceCorrect.h"

#include <TAxis.h>
#include <TH1.h>

#include <cmath>
#include <stdexcept>
#include <string>

namespace {

bool sameAxis(const TAxis* a, const TAxis* b)
{
    return a->GetNbins() == b->GetNbins() && std::fabs(a->GetXmin() - b->GetXmin()) < 1e-9 &&
           std::fabs(a->GetXmax() - b->GetXmax()) < 1e-9;
}

void requireSameBinning(const TH1& a, const TH1& b, const char* who)
{
    bool ok = a.GetDimension() == b.GetDimension() && sameAxis(a.GetXaxis(), b.GetXaxis());
    if (ok && a.GetDimension() > 1) ok = sameAxis(a.GetYaxis(), b.GetYaxis());
    if (ok && a.GetDimension() > 2) ok = sameAxis(a.GetZaxis(), b.GetZaxis());
    if (!ok)
        throw std::invalid_argument(std::string(who) + ": binning mismatch between '" + a.GetName() + "' and '" +
                                    b.GetName() + "'");
}

TH1* cloneDetached(const TH1& h, const char* name)
{
    TH1* c = static_cast<TH1*>(h.Clone(name));
    c->SetDirectory(nullptr);
    return c;
}

} // namespace

namespace gxana {

TH1* Acceptance(const TH1& thrown, const TH1& reco, const char* name, AccErrors errors)
{
    requireSameBinning(thrown, reco, "gxana::Acceptance");
    TH1* acc = cloneDetached(reco, name);
    if (errors == AccErrors::Binomial) {
        acc->Divide(&reco, &thrown, 1, 1, "B");
    } else {
        if (errors == AccErrors::PlainNoSumw2) acc->Sumw2(false);
        acc->Divide(&thrown);
    }
    return acc;
}

TH1* AcceptanceCorrect(const TH1& data, const TH1& acc, const char* name, AccErrors errors)
{
    requireSameBinning(data, acc, "gxana::AcceptanceCorrect");
    TH1* corr = cloneDetached(data, name);
    if (errors == AccErrors::PlainNoSumw2) corr->Sumw2(false);
    corr->Divide(&acc);
    return corr;
}

TH1* AcceptanceCorrect(const TH1& data, const TH1& reco, const TH1& thrown, const char* name, AccErrors errors,
                       TH1** accOut)
{
    TH1* acc = Acceptance(thrown, reco, (std::string(name) + "_acceptance").c_str(), errors);
    TH1* corr = AcceptanceCorrect(data, *acc, name, errors);
    if (accOut) *accOut = acc;
    else delete acc;
    return corr;
}

int LostBins(const TH1& data, const TH1& acc)
{
    requireSameBinning(data, acc, "gxana::LostBins");
    int lost = 0;
    for (int b = 0; b < data.GetNcells(); ++b) {
        if (data.IsBinUnderflow(b) || data.IsBinOverflow(b)) continue;
        if (data.GetBinContent(b) != 0.0 && acc.GetBinContent(b) == 0.0) ++lost;
    }
    return lost;
}

TH1* MergeCorrected(const std::vector<const TH1*>& corr, const std::vector<const TH1*>& acc, TH1** avgAcc,
                    bool sumw2, const char* name)
{
    if (corr.empty()) throw std::invalid_argument("gxana::MergeCorrected: nothing to merge");
    if (!acc.empty() && acc.size() != corr.size())
        throw std::invalid_argument("gxana::MergeCorrected: corrected and acceptance lists differ in size");
    TH1* merged = cloneDetached(*corr[0], (name && *name) ? name : corr[0]->GetName());
    if (sumw2) merged->Sumw2();
    for (size_t i = 1; i < corr.size(); ++i) {
        requireSameBinning(*merged, *corr[i], "gxana::MergeCorrected");
        merged->Add(corr[i]);
    }
    if (avgAcc) {
        *avgAcc = nullptr;
        if (!acc.empty()) {
            TH1* a = cloneDetached(*acc[0], (std::string(acc[0]->GetName()) + "_merged").c_str());
            a->SetBit(TH1::kIsAverage);
            for (size_t i = 1; i < acc.size(); ++i) {
                requireSameBinning(*a, *acc[i], "gxana::MergeCorrected");
                a->Add(acc[i]);
            }
            *avgAcc = a;
        }
    }
    return merged;
}

} // namespace gxana
