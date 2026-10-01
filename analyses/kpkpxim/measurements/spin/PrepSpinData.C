#include "../common/XimInputs.h"
#include "gxana/common/AcceptanceCorrect.h"

// Spin prep: pi- cos(theta) in the helicity frame for data (Q-weighted), reconstructed MC and
// thrown MC per period, the acceptance and the corrected data, then the three-period merge.
// Output: xim_spin.root in the current directory.
// Written objects keep the original macro's in-object names and axis titles: the acceptance was
// a clone of the MC histogram (name piminus_costheta_hf_mc), and the merged histograms were
// clones of the Spring_2017 ones.
int PrepSpinData(int n_threads = 4)
{
    if (n_threads > 0) ROOT::EnableImplicitMT(n_threads);
    // As the original: histograms filled after setStyle() store its attributes, which the merged plot uses.
    setStyle();
    TFile* out = XimOpen("xim_spin.root", "RECREATE");
    std::vector<TH1*> corr, acc;
    for (const auto& p : XimPeriods()) {
        out->cd();
        TDirectory* dir = out->mkdir(p.dir.c_str());
        dir->cd();
        auto df = XimQvalFrame(p.stem);
        auto dfmc = XimMcFrame(p.stem);
        auto dfT = XimThrownFrame(p.stem);

        auto pim_data = df.Histo1D({"piminus_costheta_hf"," ; cos#vartheta^{#pi^{-}}_{#it{h}}; arb. units", 60,-1,1}, "pim1_costheta_hf", "qvalue_acc");
        auto pim_mc = dfmc.Histo1D({"piminus_costheta_hf_mc"," ; cos#vartheta^{#pi^{-}}_{#it{h}}; arb. units", 60,-1,1}, "pim1_costheta_hf", "hybrid_combo");
        auto pim_t = dfT.Histo1D({"piminus_costheta_hf_thrown"," ; cos#vartheta^{#pi^{-}}_{#it{h}}; arb. units", 60,-1,1}, "pim1_costheta_hf");
        pim_data->Write(pim_data->GetName(), TObject::kOverwrite);
        pim_mc->Write(pim_mc->GetName(), TObject::kOverwrite);
        pim_t->Write(pim_t->GetName(), TObject::kOverwrite);

        TH1* a = nullptr;
        TH1* c = gxana::AcceptanceCorrect(*pim_data, *pim_mc, *pim_t, "piminus_costheta_hf_accCorr",
                                          gxana::AccErrors::Binomial, &a);
        a->GetYaxis()->SetTitle("Acceptance, #epsilon");
        a->SetName(pim_mc->GetName()); // the original epsilon was a clone of the MC histogram, keeping its name
        char ytitle[150];
        snprintf(ytitle, sizeof(ytitle), "Events/#epsilon / %.3f GeV", pim_data->GetBinWidth(1)); // as the original
        c->GetYaxis()->SetTitle(ytitle);
        printf("%s: populated data bins with zero acceptance %d\n", p.dir.c_str(), gxana::LostBins(*pim_data, *a));
        a->Write("piminus_costheta_hf_accept", TObject::kOverwrite);
        c->Write("piminus_costheta_hf_accCorr", TObject::kOverwrite);
        corr.push_back(c);
        acc.push_back(a);
    }
    // Combine the 3 measurements after acceptance correction, as the original: clone of the first
    // period (Sumw2), Add the others; the acceptance clone carries kIsAverage but the others do not,
    // so TH1::Add sums the three acceptances (kept as the original made it; it is only drawn).
    TH1* avg = nullptr;
    std::vector<const TH1*> corr_c(corr.begin(), corr.end()), acc_c(acc.begin(), acc.end());
    TH1* merged = gxana::MergeCorrected(corr_c, acc_c, &avg, true, corr[0]->GetName());
    avg->SetName(acc[0]->GetName());
    out->cd();
    merged->Write("pim_costheta_hf_phase1", TObject::kOverwrite);
    avg->Write("pim_costheta_hf_avg_accept_phase1", TObject::kOverwrite);
    out->Close();
    return 0;
}
