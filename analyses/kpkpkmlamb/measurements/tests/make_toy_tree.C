// Writes a toy merged kpkpkmlamb tree (Ξ(1820), Ξ(1690) Breit-Wigner peaks on a
// flat background in M(ΛK⁻), 1.6-2.6 GeV) so FitXimStar.C can be run without data.
#include <ROOT/RDataFrame.hxx>
#include <TLorentzVector.h>
#include <TRandom3.h>

void make_toy_tree(const char* out)
{
    auto rng = std::make_shared<TRandom3>(1234);
    ROOT::RDataFrame(4000)
        .Define("i", [](ULong64_t e) { return int(e); }, {"rdfentry_"})
        .Define("ximstar_M", [rng](int i) {
            double m = i < 1200 ? rng->BreitWigner(1.823, 0.024) : i < 1800 ? rng->BreitWigner(1.690, 0.020) : rng->Uniform(1.6, 2.6);
            return m; }, {"i"})
        .Define("kphigh_p4", [](int i) { TLorentzVector v; v.SetXYZM(0.1, 0.0, i % 10 == 0 ? -1.0 : 3.0, 0.493677); return v; }, {"i"})
        .Define("t_dist", [rng](int i) { return rng->Uniform(0.0, 3.0); }, {"i"})
        .Snapshot("flatTree_kpkpkmlamb", out, {"ximstar_M", "kphigh_p4", "t_dist"});
}
