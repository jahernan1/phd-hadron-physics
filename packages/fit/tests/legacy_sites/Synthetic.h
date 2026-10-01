// Seeded synthetic inputs for the legacy-site equivalence checks: a mass histogram (or a
// weighted tree) with Gaussian or Breit-Wigner peaks on a flat background.
#ifndef GXANA_FIT_SYNTHETIC_H
#define GXANA_FIT_SYNTHETIC_H

#include <TH1D.h>
#include <TRandom3.h>
#include <TTree.h>

#include <vector>

struct SynPeak {
    double mean, width;
    int n;
    bool breitWigner;
};

inline double SynDraw(TRandom3& r, const SynPeak& p)
{
    return p.breitWigner ? r.BreitWigner(p.mean, p.width) : r.Gaus(p.mean, p.width);
}

// Peaks plus nBkg flat entries in [lo, hi]; each entry has weight 1 (weighted = false) or a
// weight drawn uniformly in [0.5, 1.5] (weighted = true, so Sumw2 differs from the counts).
inline TH1D* SynHist(unsigned seed, const char* name, int nbins, double lo, double hi,
                     const std::vector<SynPeak>& peaks, int nBkg, bool weighted)
{
    TRandom3 r(seed);
    auto* h = new TH1D(name, " ;M (GeV/c^{2});Counts", nbins, lo, hi);
    h->Sumw2();
    for (const auto& p : peaks)
        for (int i = 0; i < p.n; ++i) {
            const double x = SynDraw(r, p);
            h->Fill(x, weighted ? r.Uniform(0.5, 1.5) : 1.0);
        }
    for (int i = 0; i < nBkg; ++i) {
        const double x = r.Uniform(lo, hi);
        h->Fill(x, weighted ? r.Uniform(0.5, 1.5) : 1.0);
    }
    return h;
}

// The same entries as a TTree with branches massBranch and weightBranch (double).
inline TTree* SynTree(unsigned seed, const char* massBranch, const char* weightBranch, double lo, double hi,
                      const std::vector<SynPeak>& peaks, int nBkg)
{
    TRandom3 r(seed);
    auto* t = new TTree("syn", "syn");
    t->SetDirectory(nullptr);
    double m = 0, w = 0;
    t->Branch(massBranch, &m);
    t->Branch(weightBranch, &w);
    for (const auto& p : peaks)
        for (int i = 0; i < p.n; ++i) { m = SynDraw(r, p); w = r.Uniform(0.5, 1.5); t->Fill(); }
    for (int i = 0; i < nBkg; ++i) { m = r.Uniform(lo, hi); w = r.Uniform(0.5, 1.5); t->Fill(); }
    return t;
}

#endif
