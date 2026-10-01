// Recompute the 2-D sampling histogram from the raw per-period histograms stored in the
// preserved sampling file, with gxana::AcceptanceCorrect, and report the largest bin
// differences against what PrepSampling.C stored.
#include "gxana/common/AcceptanceCorrect.h"
#include <cmath>
#include <limits>

// Largest bin difference; NaN if any compared value is non-finite (a NaN must fail the test,
// std::max would silently drop it). A cell-count mismatch returns 1 after a marker line.
static double maxDiff(const TH1* a, const TH1* b, bool errors, const char* label = "")
{
    if (a->GetNcells() != b->GetNcells()) {
        printf("MAXDIFF binning_mismatch_%s 1\n", label);
        return 1;
    }
    double m = 0;
    auto upd = [&m](double x, double y) {
        if (!std::isfinite(x) || !std::isfinite(y)) { m = std::numeric_limits<double>::quiet_NaN(); return; }
        if (!std::isnan(m)) m = std::max(m, std::fabs(x - y));
    };
    for (int i = 0; i < a->GetNcells(); ++i) {
        upd(a->GetBinContent(i), b->GetBinContent(i));
        if (errors) upd(a->GetBinError(i), b->GetBinError(i));
    }
    return m;
}

void sampling_recompute(const char* path)
{
    TFile f(path, "READ");
    if (f.IsZombie()) { printf("MAXDIFF open_failed 1\n"); return; }
    std::vector<const TH1*> corrC;
    for (auto d : {"Spring_2017", "Spring_2018", "Fall_2018"}) {
        auto data = (TH2D*)f.Get(Form("%s/ResMassVsCosTheta_qval", d));
        auto mc = (TH2D*)f.Get(Form("%s/ResMassVsCosTheta_mc", d));
        auto thrown = (TH2D*)f.Get(Form("%s/ResMassVsCosTheta_thrown", d));
        auto oldAcc = (TH2D*)f.Get(Form("%s/costhetahf_ystar_acceptance", d));
        auto oldCorr = (TH2D*)f.Get(Form("%s/costhetahf_ystar_acceptcorr", d));
        if (!data || !mc || !thrown || !oldAcc || !oldCorr) { printf("MAXDIFF missing_%s 1\n", d); return; }
        TH1* acc = nullptr;
        TH1* c = gxana::AcceptanceCorrect(*data, *mc, *thrown, "c", gxana::AccErrors::PlainNoSumw2, &acc);
        printf("MAXDIFF acceptance_%s %.3e\n", d, maxDiff(acc, oldAcc, false, d));
        printf("MAXDIFF acceptcorr_%s %.3e\n", d, maxDiff(c, oldCorr, false, d));
        printf("MAXDIFF acceptcorr_err_%s %.3e\n", d, maxDiff(c, oldCorr, true, d));
        printf("LOST %s %d\n", d, gxana::LostBins(*data, *acc));
        corrC.push_back(c);
    }
    TH1* merged = gxana::MergeCorrected(corrC, {}, nullptr, false, "merged");
    auto oldMerged = (TH2D*)f.Get("ResMassVsCosTheta_Phase1_ac");
    printf("MAXDIFF phase1_ac %.3e\n", maxDiff(merged, oldMerged, false, "phase1_ac"));
    printf("MAXDIFF phase1_ac_err %.3e\n", maxDiff(merged, oldMerged, true, "phase1_ac"));
}
