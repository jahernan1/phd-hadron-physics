// Seeded toy raw flat tree with every branch CutAnalysis.C / CutAnalysisRF.C read.
// usage: root -l -b -q 'make_cutscan_toy.C("out.root", seed, nEvents)'
#include <TFile.h>
#include <TLorentzVector.h>
#include <TRandom3.h>
#include <TTree.h>

void make_cutscan_toy(const char* out, unsigned seed, int n)
{
    TRandom3 r(seed);
    TFile f(out, "RECREATE");
    TTree t("flatTree_kpkpxim", "toy");
    double decayxim_M, chisqndf, total_mm2, beam_E, beam_vertexZ, xim_pathlensig, lambda_pathlensig, acc_weight,
        kp_highp_P3, kp_lowp_P3;
    int best_combo_rf, best_combo;
    TLorentzVector* kphigh_p4 = new TLorentzVector();
    t.Branch("decayxim_M", &decayxim_M);
    t.Branch("chisqndf", &chisqndf);
    t.Branch("total_mm2", &total_mm2);
    t.Branch("beam_E", &beam_E);
    t.Branch("beam_vertexZ", &beam_vertexZ);
    t.Branch("xim_pathlensig", &xim_pathlensig);
    t.Branch("lambda_pathlensig", &lambda_pathlensig);
    t.Branch("acc_weight", &acc_weight);
    t.Branch("kp_highp_P3", &kp_highp_P3);
    t.Branch("kp_lowp_P3", &kp_lowp_P3);
    t.Branch("best_combo_rf", &best_combo_rf);
    t.Branch("best_combo", &best_combo);
    t.Branch("kphigh_p4", &kphigh_p4);
    for (int i = 0; i < n; ++i) {
        const bool sig = r.Uniform() < 0.45;
        decayxim_M = sig ? r.Gaus(1.3217, 0.0045) : r.Uniform(1.25, 1.45);
        chisqndf = sig ? r.Exp(2.5) : r.Exp(6.0);
        total_mm2 = sig ? r.Gaus(0, 0.008) : r.Gaus(0, 0.03);
        beam_E = r.Uniform(6.0, 12.0);
        beam_vertexZ = r.Uniform(45, 85);
        xim_pathlensig = r.Exp(sig ? 8.0 : 3.0);
        lambda_pathlensig = r.Gaus(3, 3);
        acc_weight = r.Uniform() < 0.9 ? 1.0 : -0.25;
        best_combo_rf = r.Uniform() < 0.85 ? 1 : 0;
        best_combo = r.Uniform() < 0.8 ? 1 : 0;
        const double pz = r.Uniform(1.0, 9.0);
        kphigh_p4->SetXYZM(r.Gaus(0, 0.3), r.Gaus(0, 0.3), pz, 0.493677);
        kp_highp_P3 = kphigh_p4->P();
        kp_lowp_P3 = r.Uniform(0.3, 4.0);
        t.Fill();
    }
    t.Write();
    f.Close();
}
