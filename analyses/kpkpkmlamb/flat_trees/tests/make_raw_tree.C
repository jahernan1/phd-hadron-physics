// Writes a small raw kpkpkmlamb flat tree with the branches flatTreePrep.C
// cuts on, so the prep can be tested without GlueX data. Branch types follow
// DSelector_kpkpkmlamb.C. Of the 20 events exactly 4 pass the nominal cuts:
// i odd fails best_combo; i%4==0 fails the K+K- mass; i==6 fails beam_E;
// i==10 has lambda_pathlensig=1 (passes the as-run cut > 0, would fail > 2).
#include <ROOT/RDataFrame.hxx>
#include <TLorentzVector.h>

void make_raw_tree(const char* out)
{
    ROOT::RDataFrame(20)
        .Define("i", [](ULong64_t e) { return int(e); }, {"rdfentry_"})
        .Define("beam_E", [](int i) { return i == 6 ? 12.0 : 8.5; }, {"i"})
        .Define("best_combo", [](int i) -> Int_t { return i % 2 == 0 ? 1 : 0; }, {"i"})
        .Define("total_mm2", [](int i) { return 0.001; }, {"i"})
        .Define("beam_vertexZ", [](int i) { return 65.0; }, {"i"})
        .Define("chisqndf", [](int i) { return 2.0; }, {"i"})
        .Define("lambda_pathlensig", [](int i) { return i == 10 ? 1.0 : 5.0; }, {"i"})
        .Define("kp1_km_p4", [](int i) { TLorentzVector v; v.SetXYZM(0.1, 0.0, 2.0, i % 4 == 0 ? 1.0 : 1.3); return v; }, {"i"})
        .Define("kp2_km_p4", [](int i) { TLorentzVector v; v.SetXYZM(0.0, 0.1, 2.0, 1.3); return v; }, {"i"})
        .Define("lambda_p4", [](int i) { TLorentzVector v; v.SetXYZM(0.0, 0.0, 1.5, 1.1157); return v; }, {"i"})
        .Snapshot("flatTree_kpkpkmlamb", out, {"beam_E", "best_combo", "total_mm2", "beam_vertexZ", "chisqndf",
                                               "lambda_pathlensig", "kp1_km_p4", "kp2_km_p4", "lambda_p4"});
}
