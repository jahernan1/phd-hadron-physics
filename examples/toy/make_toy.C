// Toy inputs of one run period for the walkthrough in examples/toy/README.md.
// examples/toy/make_toy.py calls it once per period with numbers derived from
// the toy channel config; nothing here is GlueX data.
//
// Writes:
//   fluxPath    TH1D "tagged_flux": photons per 0.05 GeV beam-energy bin, shape 1/E on
//               6.0-12.0 GeV, integral fluxTotal
//   dataPath    tree `tree`: signal (Gaussian in mass) + background (linear in mass)
//   mcPath      tree `tree`: reconstructed signal MC
//   thrownPath  tree `thrownTree`: generated signal MC (beam_E, t_dist)
// Signal events follow dsigma/dt ~ exp(-b t) on tMin < -t < tMax, flat in beam energy,
// so the event count per energy bin follows the flux. A signal event is reconstructed
// with probability Efficiency(t); reconstructed t and beam_E equal the generated ones.
#include <cmath>
#include <iostream>
#include <memory>

#include "TFile.h"
#include "TH1D.h"
#include "TRandom3.h"
#include "TTree.h"

namespace toy {

double Efficiency(double t) { return 0.55 - 0.10 * t; }

// -t from exp(-b t) on [tMin, tMax] by inverting the cumulative distribution.
double DrawT(TRandom3& rng, double b, double tMin, double tMax)
{
    const double a = std::exp(-b * tMin), c = std::exp(-b * tMax);
    return -std::log(a - rng.Rndm() * (a - c)) / b;
}

// Mass from 1 + slope * x on [lo, hi] (x mapped to [-1, 1]), by accept-reject.
double DrawBackgroundMass(TRandom3& rng, double lo, double hi, double slope)
{
    while (true) {
        const double m = rng.Uniform(lo, hi);
        const double x = 2 * (m - lo) / (hi - lo) - 1;
        if (rng.Rndm() * (1 + std::abs(slope)) < 1 + slope * x)
            return m;
    }
}

struct Event {
    double beam_E = 0, t_dist = 0, mass = 0, weight = 1;
};

std::unique_ptr<TTree> MakeTree(const char* name, Event& e, bool withMass)
{
    auto tree = std::make_unique<TTree>(name, "toy flat tree");
    tree->Branch("beam_E", &e.beam_E, "beam_E/D");
    tree->Branch("t_dist", &e.t_dist, "t_dist/D");
    if (withMass) {
        tree->Branch("mass", &e.mass, "mass/D");
        tree->Branch("weight", &e.weight, "weight/D");
    }
    return tree;
}

} // namespace toy

int make_toy(const char* fluxPath, const char* dataPath, const char* mcPath, const char* thrownPath,
             const char* tree, const char* thrownTree, double fluxTotal, double nSignal, double nBackground,
             int nThrown, double b, double tMin, double tMax, double mean, double sigma, double massLo,
             double massHi, int seed)
{
    TRandom3 rng(seed);

    // Flux: 1/E shape, 120 bins of 0.05 GeV (fixed width, edges on the analysis bin edges).
    TFile fluxFile(fluxPath, "RECREATE");
    TH1D flux("tagged_flux", "toy tagged flux;E_{#gamma} (GeV);photons", 120, 6.0, 12.0);
    for (int i = 1; i <= flux.GetNbinsX(); ++i)
        flux.SetBinContent(i, 1.0 / flux.GetBinCenter(i));
    flux.Scale(fluxTotal / flux.Integral());
    for (int i = 1; i <= flux.GetNbinsX(); ++i)
        flux.SetBinError(i, 1e-3 * flux.GetBinContent(i));
    flux.Write();
    fluxFile.Close();

    toy::Event e;
    // Data: Poisson signal and background counts.
    {
        TFile f(dataPath, "RECREATE");
        auto t = toy::MakeTree(tree, e, true);
        const int nSig = rng.Poisson(nSignal), nBkg = rng.Poisson(nBackground);
        int kept = 0;
        for (int i = 0; i < nSig; ++i) {
            e.beam_E = flux.GetRandom(&rng);
            e.t_dist = toy::DrawT(rng, b, tMin, tMax);
            if (rng.Rndm() >= toy::Efficiency(e.t_dist))
                continue;
            e.mass = rng.Gaus(mean, sigma);
            if (e.mass < massLo || e.mass > massHi)
                continue;
            t->Fill();
            ++kept;
        }
        for (int i = 0; i < nBkg; ++i) {
            e.beam_E = flux.GetRandom(&rng);
            e.t_dist = toy::DrawT(rng, 0.7, tMin, tMax);
            e.mass = toy::DrawBackgroundMass(rng, massLo, massHi, 0.3);
            t->Fill();
        }
        t->Write();
        std::cout << dataPath << ": " << kept << " signal + " << nBkg << " background events (" << nSig
                  << " signal produced)" << std::endl;
    }
    // Signal MC: thrown (every generated event) and reconstructed (accepted ones).
    {
        TFile fm(mcPath, "RECREATE");
        auto reco = toy::MakeTree(tree, e, true);
        TFile ft(thrownPath, "RECREATE");
        auto thrown = toy::MakeTree(thrownTree, e, false);
        for (int i = 0; i < nThrown; ++i) {
            e.beam_E = flux.GetRandom(&rng);
            e.t_dist = toy::DrawT(rng, b, tMin, tMax);
            thrown->Fill();
            if (rng.Rndm() < toy::Efficiency(e.t_dist)) {
                e.mass = rng.Gaus(mean, sigma);
                reco->Fill();
            }
        }
        std::cout << mcPath << ": " << reco->GetEntries() << " reconstructed of " << nThrown << " thrown"
                  << std::endl;
        ft.cd();
        thrown->Write();
        fm.cd();
        reco->Write();
        thrown.reset();
        reco.reset();
    }
    return 0;
}
