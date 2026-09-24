// Writes a small raw flat tree with the branches flatTreePrep.C reads, so the
// prep can be tested without GlueX data. Values are chosen to pass the
// nominal cuts; kaons have mass 0.4937 so rapidity != pseudorapidity.
#include <ROOT/RDataFrame.hxx>
#include <TLorentzVector.h>
#include <TMath.h>

void make_raw_tree(const char* out)
{
    const double mK = 0.493677;
    ROOT::RDataFrame(20)
        .Define("i", [](ULong64_t e) { return double(e); }, {"rdfentry_"})
        .Define("kphigh_p4", [mK](double i) { TLorentzVector v; v.SetXYZM(0.05, 0.02, 4.0 + 0.1 * i, mK); return v; }, {"i"})
        .Define("kplow_p4", [mK](double i) { TLorentzVector v; v.SetXYZM(0.2, 0.1, 1.0 + 0.05 * i, mK); return v; }, {"i"})
        .Define("ystar_p4", [](double i) { TLorentzVector v; v.SetXYZM(0.1, 0.0, 3.0 + 0.1 * i, 1.8); return v; }, {"i"})
        .Define("beam_p4_truth", [](double i) { TLorentzVector v; v.SetXYZM(0, 0, 8.5, 0); return v; }, {"i"})
        .Define("beam_E", [](double i) { return 8.5; }, {"i"})
        .Define("beam_vertexZ", [](double i) { return 65.0; }, {"i"})
        .Define("chisqndf", [](double i) { return 2.0; }, {"i"})
        .Define("total_mm2", [](double i) { return 0.001; }, {"i"})
        .Define("xim_pathlensig", [](double i) { return 5.0; }, {"i"})
        .Define("lambda_pathlensig", [](double i) { return 5.0; }, {"i"})
        .Define("kp_highp_P3", [](double i) { return 4.0 + 0.1 * i; }, {"i"})
        .Define("kp_lowp_P3", [](double i) { return 1.0 + 0.05 * i; }, {"i"})
        .Define("t_dist", [](double i) { return 0.5; }, {"i"})
        .Define("decayxim_M", [](double i) { return 1.3217; }, {"i"})
        .Define("best_combo_rf", [](double i) -> int { return 1; }, {"i"})
        .Define("acc_weight", [](double i) { return 1.0; }, {"i"})
        .Define("uniq_weight", [](double i) -> int { return 1; }, {"i"})
        .Snapshot("flatTree_kpkpxim", out);
}
