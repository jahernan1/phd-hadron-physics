#include "../common/XimInputs.h"

// Mass prep: M(Lambda pi-) for data (hybrid_combo), reconstructed MC (hybrid_combo) and thrown
// MC per period. No acceptance correction (the mass distribution cannot be acceptance
// corrected; FitMass.C corrects the mean with the generated-minus-reconstructed MC shift).
// Output: xim_mass.root in the current directory.
int PrepMass(int n_threads = 4)
{
    if (n_threads > 0) ROOT::EnableImplicitMT(n_threads);
    // As the original: histograms filled after setStyle() store its attributes.
    setStyle();
    TFile* out = XimOpen("xim_mass.root", "RECREATE");
    for (const auto& p : XimPeriods()) {
        out->cd();
        TDirectory* dir = out->mkdir(p.dir.c_str());
        dir->cd();
        auto df = XimDataFrame(p.stem);
        auto dfmc = XimMcFrame(p.stem);
        auto dfT = XimThrownFrame(p.stem);
        auto mass_data = df.Histo1D({"cascade_mass"," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 60,1.25,1.47}, "decayxim_M", "hybrid_combo");
        auto mass_mc = dfmc.Histo1D({"cascade_mass_mc"," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 60,1.25,1.47}, "decayxim_M", "hybrid_combo");
        auto mass_t = dfT.Histo1D({"cascade_mass_thrown"," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 60,1.25,1.47}, "decayxim_M");
        mass_data->Write(mass_data->GetName(), TObject::kOverwrite);
        mass_mc->Write(mass_mc->GetName(), TObject::kOverwrite);
        mass_t->Write(mass_t->GetName(), TObject::kOverwrite);
    }
    out->Close();
    return 0;
}
