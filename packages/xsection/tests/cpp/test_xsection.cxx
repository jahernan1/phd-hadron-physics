#include "CliArgs.h"
#include "gxana/xsection/Binning.h"
#include "gxana/xsection/Flux.h"
#include "gxana/xsection/YieldFit.h"
#include "gxana/xsection/XSec.h"
#include "gxana/xsection/Barlow.h"
#include "gxana/xsection/Plotting.h"

#include <TAxis.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TH1D.h>
#include <TString.h>
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

    // LegacyFindBin: ROOT 6.24 TAxis::FindFixBin formula on the staged flux
    // binning (500 bins, [6.4, 11.4], from gluex_analysis_data/kpkpxim/flux).
    {
        TAxis fluxAxis(500, 6.4, 11.4);
        CHECK(LegacyFindBin(&fluxAxis, 8.68) == 228);
        CHECK(LegacyFindBin(&fluxAxis, 9.26) == 286);
        CHECK(LegacyFindBin(&fluxAxis, 10.18) == 378);
        // Interior points where FP rounding does not shift the formula:
        // both LegacyFindBin and the raw formula 1+int(n*(x-xmin)/(xmax-xmin))
        // must agree with each other and with TAxis::FindBin.
        for (double x : {6.40, 7.40, 7.86, 8.19, 8.45, 11.40}) {
            const int legacy = 1 + static_cast<int>(500 * (x - 6.4) / (11.4 - 6.4));
            CHECK(LegacyFindBin(&fluxAxis, x) == legacy);
            CHECK(LegacyFindBin(&fluxAxis, x) == fluxAxis.FindBin(x));
        }
        CHECK(LegacyFindBin(&fluxAxis, 6.0) == 0);     // underflow
        CHECK(LegacyFindBin(&fluxAxis, 11.4) == 501);  // overflow (n+1)
        // NaN must go to overflow like ROOT 6.24 FindFixBin's `!(x < xmax)`,
        // not through a UB static_cast<int>(NaN).
        CHECK(LegacyFindBin(&fluxAxis, std::nan("")) == 501);

        // Variable-bin axis: not the fixed-formula case.
        double edges[4] = {6.4, 7.4, 9.4, 11.4};
        TAxis varAxis(3, edges);
        CHECK(Throws([&] { LegacyFindBin(&varAxis, 8.0); }));
    }

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
    // --weight is per JOB too (combo-selection study: one fit, three weights).
    CHECK(job1.weight == "hybrid_combo");
    CHECK(gxana::cli::ParseJob("n:d:m:t:f", "best_combo", 2, "best_combo").weight == "best_combo");
    CHECK(gxana::cli::ParseChebyOrder("1") == 1);
    CHECK(gxana::cli::ParseChebyOrder("2") == 2);
    CHECK(Throws([] { gxana::cli::ParseChebyOrder("3"); }));
    CHECK(Throws([] { gxana::cli::ParseChebyOrder("1.7"); }));
    // Each JOB's tables go to <out>/<label>/ (legacy MakeXSecFitVariations.C
    // wrote data/<variation>/), so chained labels never overwrite each other.
    CHECK(gxana::cli::LabelOutDir("out/data", "johnson") == "out/data/johnson");
    CHECK(gxana::cli::LabelOutDir("out/data/", "johnson_cheby1") == "out/data/johnson_cheby1");

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

    // JohnsonMCShape: legacy MakeXSecFiles.C factory strings, start values "%f".
    FitParams mcShape;
    mcShape["mu"] = {1.3217, 1.32, 1.33};
    mcShape["lambda"] = {0.004, 0.002, 0.007};
    mcShape["gamma"] = {-0.01, -1, 1};
    mcShape["delta"] = {1.2, 0.2, 5};
    CHECK(std::string(kJohnsonMCShape) == "JohnsonMCShape");
    CHECK(constructFitStringMCShape(mcShape) ==
          "Johnson::xisignal(decayxim_M, mu[1.321700,1.3200000000000001,1.3300000000000001], "
          "lambda[0.004000,0.002,0.0070000000000000001], gamma[-0.010000,-1,1], delta[1.200000,0.20000000000000001,5])");
    mcShape["lambda"][0] = 0.00412345678;  // as left by the MC fit
    CHECK(constructFitStringDataMCShape(mcShape) ==
          "Johnson::xisignal(decayxim_M, mu[1.321700,1.3200000000000001,1.3300000000000001], "
          "lambda[0.004123,0.004123,0.008], gamma[-0.010000], delta[1.200000])");
    CHECK(OrderedFitParams(kJohnsonMCShape, mcShape).front().first == "mu");
    const std::vector<gxana::cli::XSecJob> cheby2Jobs{gxana::cli::ParseJob("n:d:m:t:f", "hybrid_combo", 2)};
    gxana::cli::CheckMCShapeArgs(mcShape, cheby2Jobs);
    CHECK(Throws([&] { gxana::cli::CheckMCShapeArgs(mcShape, {gxana::cli::ParseJob("n:d:m:t:f", "x", 1)}); }));
    CHECK(Throws([&] { gxana::cli::CheckMCShapeArgs(partial, cheby2Jobs); }));
    FitParams extra = mcShape;
    extra["sigma"] = {1, 0, 2};
    CHECK(Throws([&] { gxana::cli::CheckMCShapeArgs(extra, cheby2Jobs); }));

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

    // Barlow significance and spread of variations.
    TGraphErrors nominal(2), variation(2);
    nominal.SetPoint(0, 0.225, 5.0);   nominal.SetPointError(0, 0.125, 0.5);
    nominal.SetPoint(1, 0.44, 6.0);    nominal.SetPointError(1, 0.09, 0.4);
    variation.SetPoint(0, 0.225, 4.0); variation.SetPointError(0, 0.125, 0.3);
    variation.SetPoint(1, 0.44, 6.5);  variation.SetPointError(1, 0.09, 0.4);
    TGraphErrors* barlow = calc_barlow(&nominal, &variation);
    CHECK(barlow->GetN() == 2);
    CHECK(std::fabs(barlow->GetPointY(0) - 1.0 / 0.4) < 1e-12); // sqrt(0.25 - 0.09) = 0.4
    CHECK(barlow->GetPointY(1) == 0.0);                           // equal errors: sigma 0
    CHECK(barlow->GetPointX(1) == 0.44 && barlow->GetErrorX(0) == 0.125 && barlow->GetErrorY(0) == 0.0);
    TGraphErrors a(2), b(2);
    a.SetPoint(0, 0.225, 4.0); a.SetPoint(1, 0.44, 6.0);
    b.SetPoint(0, 0.225, 6.0); b.SetPoint(1, 0.44, 6.0);
    TGraphErrors* spread = calculateStdDevGraph({&a, &b});
    CHECK(std::fabs(spread->GetPointY(0) - 1.0) < 1e-12);
    CHECK(spread->GetPointY(1) == 0.0 && spread->GetPointX(1) == 0.44);
    CHECK(calculateStdDevGraph({}) == nullptr);

    // Plotting: plot*XSec save PlotDir()/<saveName>.pdf.
    {
        // Nested and absent: SetPlotDir must create it (PlotDiffXSec.C points it at a fresh
        // $GXANA_OUTPUT/kpkpxim/xsection/plots).
        std::string plotRoot = std::string(gSystem->TempDirectory()) + "/gxana_plot_test";
        std::string plotDir = plotRoot + "/plots";
        gSystem->Exec(("rm -rf " + plotRoot).c_str());
        CHECK(PlotDir() == "."); // unset default: current directory, like SetFitPlotDir
        SetPlotDir(plotDir);
        CHECK(PlotDir() == plotDir);
        CHECK(!gSystem->AccessPathName(plotDir.c_str()));

        auto makeGraph = [](const std::string& name) {
            auto* g = new TGraphErrors(3);
            for (int j = 0; j < 3; ++j) {
                g->SetPoint(j, 0.3 + 0.4 * j, 5.0 - j);
                g->SetPointError(j, 0.1, 0.5);
            }
            g->SetName(name.c_str());
            return g;
        };

        // plotWeightedXSec: legacy loops over 8 energy bins.
        std::vector<TGraphErrors*> weighted;
        for (int i = 0; i < 8; ++i) weighted.push_back(makeGraph(Form("diffxsec_emin_%d", i)));
        plotWeightedXSec(weighted, 2.5, 20, "unit_weighted");
        CHECK(!gSystem->AccessPathName((plotDir + "/unit_weighted.pdf").c_str()));

        // plotOneWeightedXSec: exercises the loop on a short (2-graph) vector.
        std::vector<TGraphErrors*> onePair = {makeGraph("g0"), makeGraph("g1")};
        plotOneWeightedXSec(onePair, 2.5, 20, "unit_one_weighted");
        CHECK(!gSystem->AccessPathName((plotDir + "/unit_one_weighted.pdf").c_str()));

        // plotDiffXSec: arrGraphs[run period][energy bin], 3 periods x 2 bins.
        std::vector<std::vector<TGraphErrors*>> diffGraphs;
        for (int p = 0; p < 3; ++p) {
            std::vector<TGraphErrors*> period;
            for (int b = 0; b < 2; ++b) period.push_back(makeGraph(Form("p%d_b%d", p, b)));
            diffGraphs.push_back(period);
        }
        plotDiffXSec(diffGraphs, 2.5, 20, "unit_diff");
        CHECK(!gSystem->AccessPathName((plotDir + "/unit_diff.pdf").c_str()));

        // plotFinalWeightedXSec: arrGraphs[nominal/systematic][energy bin], 2 x 2.
        std::vector<std::vector<TGraphErrors*>> finalGraphs;
        for (int p = 0; p < 2; ++p) {
            std::vector<TGraphErrors*> group;
            for (int b = 0; b < 2; ++b) group.push_back(makeGraph(Form("f%d_b%d", p, b)));
            finalGraphs.push_back(group);
        }
        plotFinalWeightedXSec(finalGraphs, 2.5, 20, "unit_final_weighted");
        CHECK(!gSystem->AccessPathName((plotDir + "/unit_final_weighted.pdf").c_str()));

        // Labels with no text files give empty graph vectors (PlotDiffXSec.C loops over every
        // legacy label); each plotter must skip them instead of indexing element 0.
        plotWeightedXSec({}, 2.5, 20, "unit_empty_weighted");
        CHECK(gSystem->AccessPathName((plotDir + "/unit_empty_weighted.pdf").c_str()));
        plotOneWeightedXSec({}, 2.5, 20, "unit_empty_one");
        CHECK(gSystem->AccessPathName((plotDir + "/unit_empty_one.pdf").c_str()));
        plotDiffXSec({{}, {}, {}}, 2.5, 20, "unit_empty_diff");
        CHECK(gSystem->AccessPathName((plotDir + "/unit_empty_diff.pdf").c_str()));
        // Nominal present, systematic missing (no syst_weighted_* files).
        plotFinalWeightedXSec({finalGraphs[0], {}}, 2.5, 20, "unit_empty_final");
        CHECK(gSystem->AccessPathName((plotDir + "/unit_empty_final.pdf").c_str()));

        gSystem->Exec(("rm -rf " + plotRoot).c_str());
    }

    if (failures == 0) std::cout << "test_xsection: all checks passed\n";
    gSystem->Exec(("rm -rf " + fluxDir).c_str());
    return failures == 0 ? 0 : 1;
}
