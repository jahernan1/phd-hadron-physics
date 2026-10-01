#include "../common/XimInputs.h"
#include "gxana/common/AcceptanceCorrect.h"

// Lifetime prep: fill data / reconstructed MC / thrown MC rest-frame lifetime per period,
// acceptance (reco/thrown, binomial), and the acceptance-corrected data.
// Output: xim_lifetime.root in the current directory.
int PrepLifetime(int n_threads = 4)
{
    if (n_threads > 0) ROOT::EnableImplicitMT(n_threads);
    TFile* out = XimOpen("xim_lifetime.root", "RECREATE");
    for (const auto& p : XimPeriods()) {
        out->cd();
        TDirectory* dir = out->mkdir(p.dir.c_str());
        dir->cd();
        auto df = XimQvalFrame(p.stem);
        auto dfmc = XimMcFrame(p.stem);
        auto dfT = XimThrownFrame(p.stem);

        auto lifetime_data = df.Histo1D({"cascade_lifetime"," ;#tau_{#Xi^{-}} (ns); arb. units", 60,0.02,0.8}, "xim_lifetime_restframe", "qvalue_acc");
        auto lifetime_mc = dfmc.Histo1D({"cascade_lifetime_mc"," ;#tau_{#Xi^{-}} (ns); arb. units", 60,0.02,0.8}, "xim_lifetime_restframe", "hybrid_combo");
        auto lifetime_t = dfT.Histo1D({"cascade_lifetime_thrown"," ;#tau_{#Xi^{-}} (ns); arb. units", 60,0.02,0.8}, "xim_lifetime_restframe");
        lifetime_data->Write(lifetime_data->GetName(), TObject::kOverwrite);
        lifetime_mc->Write(lifetime_mc->GetName(), TObject::kOverwrite);
        lifetime_t->Write(lifetime_t->GetName(), TObject::kOverwrite);

        TH1* acc = nullptr;
        TH1* corr = gxana::AcceptanceCorrect(*lifetime_data, *lifetime_mc, *lifetime_t, "cascade_lifetime_accCorr",
                                             gxana::AccErrors::Binomial, &acc);
        acc->GetYaxis()->SetTitle("Acceptance, #epsilon");
        acc->SetName(lifetime_mc->GetName()); // the original epsilon was a clone of the MC histogram, keeping its name
        char ytitle[150];
        snprintf(ytitle, sizeof(ytitle), "Events/#epsilon / %.3f GeV", lifetime_data->GetBinWidth(1)); // as the original
        corr->GetYaxis()->SetTitle(ytitle);
        printf("%s: acceptance bins %d, populated data bins with zero acceptance %d\n", p.dir.c_str(),
               acc->GetNbinsX(), gxana::LostBins(*lifetime_data, *acc));
        acc->Write("cascade_lifetime_accept", TObject::kOverwrite);
        corr->Write("cascade_lifetime_accCorr", TObject::kOverwrite);
        delete acc;
        delete corr;
    }
    out->Close();
    return 0;
}
