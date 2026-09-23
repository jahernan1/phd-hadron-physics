#include "CliArgs.h"
#include "gxana/xsection/Binning.h"
#include "gxana/xsection/Flux.h"
#include "gxana/xsection/YieldFit.h"
#include "gxana/xsection/XSec.h"

#include <TFile.h>
#include <TH1D.h>
#include <TSystem.h>

#include <cstdlib>
#include <unistd.h>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static int failures = 0;
#define CHECK(cond)                                                                   \
    do {                                                                              \
        if (!(cond)) {                                                                \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #cond "\n"; \
            ++failures;                                                               \
        }                                                                             \
    } while (0)

template <typename F>
static bool Throws(F f)
{
    try { f(); } catch (const std::exception&) { return true; }
    return false;
}

int main()
{
    using namespace gxana::xsec;

    // Binning names (legacy MakeBinnedTrees.cpp label rule)
    CHECK(BinEdgeLabel(6.4) == "6.40");
    CHECK(BinEdgeLabel(11.4) == "11.40");
    CHECK(BinEdgeLabel(10.18) == "10.18");
    CHECK(BinEdgeLabel(0.1) == "0.10");
    CHECK(BinEdgeLabel(6.405) == "6.40"); // truncates, never rounds
    CHECK(EnergyBinName(6.4, 7.4) == "emin_6.40_emax_7.40");
    CHECK(BinName(10.18, 11.4, 1.53, 2.4) == "emin_10.18_emax_11.40_tmin_1.53_tmax_2.40");
    BinRanges bins = EdgesToBins({6.4, 7.4, 7.86});
    CHECK(bins.size() == 2 && bins[1].first == 7.4 && bins[1].second == 7.86);
    CHECK(Throws([] { EdgesToBins({6.4}); }));
    CHECK(Throws([] { EdgesToBins({7.4, 6.4}); }));
    CHECK(!divideThrownIntoBins("/nonexistent/in.root", "/nonexistent/out.root", bins, bins));

    // Executable argument parsing
    CHECK(gxana::cli::ParseDoubleList("6.4,11.4") == std::vector<double>({6.4, 11.4}));
    CHECK(Throws([] { gxana::cli::ParseDoubleList("6.4,x"); }));
    CHECK(Throws([] { gxana::cli::ParseDoubleList("6.4,"); }));
    auto param = gxana::cli::ParseParam("mu=1.3217,1.31,1.33");
    CHECK(param.first == "mu" && param.second == std::vector<double>({1.3217, 1.31, 1.33}));
    CHECK(Throws([] { gxana::cli::ParseParam("mu=1,2"); }));
    CHECK(Throws([] { gxana::cli::ParseParam("=1,2,3"); }));
    auto job = gxana::cli::ParseJob("n:d.root:m.root:t.root:f.root");
    CHECK(job.name == "n" && job.data == "d.root" && job.thrown == "t.root" && job.flux == "f.root");
    CHECK(job.label.empty() && job.chebyOrder == 2); // defaults when no --label/--cheby precede it
    CHECK(Throws([] { gxana::cli::ParseJob("n:d:m:t"); }));

    // JOBs record whatever --label/--cheby is in effect when they are parsed, so
    // gxana_xsec_tables can chain e.g. johnson -> johnson_cheby1 in one process (spec D20).
    auto job1 = gxana::cli::ParseJob("n1:d:m:t:f", "johnson", 2);
    auto job2 = gxana::cli::ParseJob("n2:d:m:t:f", "johnson_cheby1", 1);
    CHECK(job1.label == "johnson" && job1.chebyOrder == 2);
    CHECK(job2.label == "johnson_cheby1" && job2.chebyOrder == 1);
    CHECK(gxana::cli::ParseChebyOrder("1") == 1);
    CHECK(gxana::cli::ParseChebyOrder("2") == 2);
    CHECK(Throws([] { gxana::cli::ParseChebyOrder("3"); }));
    CHECK(Throws([] { gxana::cli::ParseChebyOrder("1.7"); }));

    // Fit model strings: legacy MakeXSecFitVariations.C parameter sets.
    FitParams johnson;
    johnson["delta"] = {1., 0.2, 1.5};
    johnson["gamma"] = {0., -0.5, 0.5};
    johnson["lambda"] = {0.004, 0.003, 0.01};
    johnson["mu"] = {1.3217, 1.31, 1.33};
    CHECK(constructFitString("Johnson", johnson) ==
          "Johnson::xisignal(decayxim_M, mu[1.3217, 1.31, 1.33], lambda[0.004, 0.003, 0.01], "
          "gamma[0, -0.5, 0.5], delta[1, 0.2, 1.5])");
    CHECK(constructFitStringData("Johnson", johnson) ==
          "Johnson::xisignal(decayxim_M, mu[1.3217, 1.31, 1.33], lambda[-0.003, 0.004, 0.01], "
          "gamma[0], delta[-0.25, 1, 1.5])");
    FitParams voigt;
    voigt["sigma"] = {0.002, 0.001, 0.018};
    voigt["width"] = {0.004, 0.001, 0.008};
    voigt["mean"] = {1.3217, 1.32, 1.33};
    CHECK(constructFitStringData("Voigtian", voigt) ==
          "Voigtian::xisignal(decayxim_M, mean[1.3217, 1.32, 1.33], width[0.004], sigma[0.002, 0.002, 0.018])");
    FitParams gaus;
    gaus["sigma"] = {0.005, 0.003, 0.01};
    gaus["mean"] = {1.3217, 1.32, 1.33};
    CHECK(constructFitString("Gaussian", gaus) ==
          "Gaussian::xisignal(decayxim_M, mean[1.3217, 1.32, 1.33], sigma[0.005, 0.003, 0.01])");
    FitParams other;
    other["c"] = {1, 0, 2};
    CHECK(constructFitString("Exponential", other) == "Exponential::xisignal(decayxim_M, c[1, 0, 2])");
    FitParams partial;
    partial["mu"] = {1.3, 1.2, 1.4};
    CHECK(OrderedFitParams("Johnson", partial).size() == 1);

    CHECK(GetFitPlotDir().empty());
    SetFitPlotDir("/tmp/plots");
    CHECK(GetFitPlotDir() == "/tmp/plots");
    SetFitPlotDir("");

    // Flux histogram from a file written here.
    char fluxTmpl[] = "/tmp/gxana_flux_XXXXXX";
    const std::string fluxDir = mkdtemp(fluxTmpl);
    {
        TFile f((fluxDir + "/flux.root").c_str(), "RECREATE");
        TH1D h("tagged_flux", "", 10, 6.4, 11.4);
        h.SetBinContent(3, 5.0);
        h.Write();
    }
    TH1D* flux = GetFluxHist(fluxDir + "/flux.root");
    CHECK(flux != nullptr && flux->GetBinContent(3) == 5.0 && flux->GetDirectory() == nullptr);
    CHECK(Throws([&] { GetFluxHist(fluxDir + "/absent.root"); }));
    CHECK(Throws([&] { GetFluxHist(fluxDir + "/flux.root", "nope"); }));

    // WriteXSecTables reports unreadable inputs instead of crashing.
    CHECK(Throws([&] {
        WriteXSecTables("/nonexistent/d.root", "/nonexistent/m.root", "/nonexistent/t.root", flux, "n", "l",
                        "Johnson", johnson, fluxDir + "/tables");
    }));

    if (failures == 0) std::cout << "test_xsection: all checks passed\n";
    gSystem->Exec(("rm -rf " + fluxDir).c_str());
    return failures == 0 ? 0 : 1;
}
