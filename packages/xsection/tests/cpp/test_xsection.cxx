#include "CliArgs.h"
#include "gxana/xsection/Binning.h"
#include "gxana/xsection/Flux.h"
#include "gxana/xsection/YieldFit.h"
#include "gxana/xsection/XSec.h"
#include "gxana/xsection/Plotting.h"

#include <TAxis.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TH1D.h>
#include <TString.h>
#include <TSystem.h>

#include <TRandom3.h>
#include <TTree.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
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

template <typename F>
static std::string ErrorText(F f)
{
    try { f(); } catch (const std::invalid_argument& e) { return e.what(); }
    return "";
}

int main()
{
    using namespace gxana::xsec;
    // The kpkpxim fit observable and mass windows (analyses/kpkpxim/config).
    const Observable kObs{"decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})"};
    const MassWindows kWin{1.27, 1.40, 1.38, 1.42, 1.45, 1.28, 1.275};
    const XSecPhysics kPhysics{"(hybrid_combo)*(decayxim_M>1.3&&decayxim_M<1.35)", "qvalue_decayxim_M", 0.641, 0.005,
                               {50.4, 79.1, 0.07008, 2.01588, 2}, kObs, kWin};

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
    CHECK(!divideThrownIntoBins("/nonexistent/in.root", "/nonexistent/out.root", bins, bins, "t"));

    // LegacyFindBin: ROOT 6.24 TAxis::FindFixBin formula on the staged flux
    // binning (500 bins, [6.4, 11.4], from gluex_analysis_data/kpkpxim/flux).
    {
        TAxis fluxAxis(500, 6.4, 11.4);
        CHECK(LegacyFindBin(&fluxAxis, 8.68) == 228);
        CHECK(LegacyFindBin(&fluxAxis, 9.26) == 286);
        CHECK(LegacyFindBin(&fluxAxis, 10.18) == 378);
        // Points where LegacyFindBin and the 6.40 TAxis::FindBin agree (8.19 and
        // 8.45 still land one bin low in both; see the window checks below):
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

        // The thesis flux windows: every energy edge from 8.19 GeV up sits on a
        // flux-bin edge and rounds one 10 MeV bin low, so e.g. 7.86-8.19 integrates
        // bins 147..178 = [7.86, 8.18) while the data cut is beam_E < 8.19
        // (docs/KNOWN_ISSUES.md, flux windows). Aligned windows would end at 179.
        CHECK(LegacyFindBin(&fluxAxis, 8.19) - 1 == 178);
        CHECK(fluxAxis.FindBin(8.19 - 1e-6) == 179);
        CHECK(LegacyFindBin(&fluxAxis, 8.45) == 205);
        CHECK(fluxAxis.FindBin(8.45 + 1e-6) == 206);

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
    // Generic parsers: results and exact messages (the executables print them).
    CHECK(gxana::cli::Split("a,,b,", ',') == std::vector<std::string>({"a", "", "b", ""}));
    CHECK(gxana::cli::Split("", ',').empty());
    CHECK(gxana::cli::ParseDouble("1.5") == 1.5);
    CHECK(ErrorText([] { gxana::cli::ParseDouble("x"); }) == "not a number: 'x'");
    CHECK(ErrorText([] { gxana::cli::ParseDouble(""); }) == "not a number: ''");
    CHECK(ErrorText([] { gxana::cli::ParseDouble("1.5abc"); }) == "not a number: '1.5abc'");
    CHECK(ErrorText([] { gxana::cli::ParseDoubleList("6.4,"); }) == "not a number: ''");
    CHECK(ErrorText([] { gxana::cli::ParseParam("mu=1,2"); }) == "expected NAME=INIT,MIN,MAX: 'mu=1,2'");
    CHECK(ErrorText([] { gxana::cli::ParseParam("=1,2,3"); }) == "expected NAME=INIT,MIN,MAX: '=1,2,3'");
    CHECK(ErrorText([] { gxana::cli::ParseParam("mu"); }) == "expected NAME=INIT,MIN,MAX: 'mu'");
    auto job = gxana::cli::ParseJob("n:d.root:m.root:t.root:f.root");
    CHECK(job.name == "n" && job.data == "d.root" && job.thrown == "t.root" && job.flux == "f.root");
    CHECK(job.label.empty() && job.chebyOrder == 2); // defaults when no --label/--cheby precede it
    CHECK(Throws([] { gxana::cli::ParseJob("n:d:m:t"); }));

    // JOBs record whatever --label/--cheby is in effect when they are parsed, so
    // gxana_xsec_tables can chain e.g. johnson -> johnson_cheby1 in one process.
    auto job1 = gxana::cli::ParseJob("n1:d:m:t:f", "johnson", 2);
    auto job2 = gxana::cli::ParseJob("n2:d:m:t:f", "johnson_cheby1", 1);
    CHECK(job1.label == "johnson" && job1.chebyOrder == 2);
    CHECK(job2.label == "johnson_cheby1" && job2.chebyOrder == 1);
    // --weight is per JOB too (combo-selection study: one fit, three weights).
    CHECK(job1.weight.empty()); // no default weight: gxana_xsec_tables requires --weight
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
    CHECK(constructFitString("Johnson", johnson, kObs) ==
          "Johnson::xisignal(decayxim_M, mu[1.3217, 1.31, 1.33], lambda[0.004, 0.003, 0.01], "
          "gamma[0, -0.5, 0.5], delta[1, 0.2, 1.5])");
    CHECK(constructFitStringData("Johnson", johnson, kObs) ==
          "Johnson::xisignal(decayxim_M, mu[1.3217, 1.31, 1.33], lambda[-0.003, 0.004, 0.01], "
          "gamma[0], delta[-0.25, 1, 1.5])");
    FitParams voigt;
    voigt["sigma"] = {0.002, 0.001, 0.018};
    voigt["width"] = {0.004, 0.001, 0.008};
    voigt["mean"] = {1.3217, 1.32, 1.33};
    CHECK(constructFitStringData("Voigtian", voigt, kObs) ==
          "Voigtian::xisignal(decayxim_M, mean[1.3217, 1.32, 1.33], width[0.004], sigma[0.002, 0.002, 0.018])");
    FitParams gaus;
    gaus["sigma"] = {0.005, 0.003, 0.01};
    gaus["mean"] = {1.3217, 1.32, 1.33};
    CHECK(constructFitString("Gaussian", gaus, kObs) ==
          "Gaussian::xisignal(decayxim_M, mean[1.3217, 1.32, 1.33], sigma[0.005, 0.003, 0.01])");
    FitParams other;
    other["c"] = {1, 0, 2};
    CHECK(constructFitString("Exponential", other, kObs) == "Exponential::xisignal(decayxim_M, c[1, 0, 2])");
    // Another channel's observable replaces the branch, nothing else.
    const Observable other_obs{"ximstar_M", "M(#LambdaK^{-}) (GeV/c^{2})"};
    CHECK(constructFitString("Gaussian", gaus, other_obs) ==
          "Gaussian::xisignal(ximstar_M, mean[1.3217, 1.32, 1.33], sigma[0.005, 0.003, 0.01])");
    CHECK(constructFitStringData("Johnson", johnson, other_obs) ==
          "Johnson::xisignal(ximstar_M, mu[1.3217, 1.31, 1.33], lambda[-0.003, 0.004, 0.01], "
          "gamma[0], delta[-0.25, 1, 1.5])");
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
    CHECK(constructFitStringMCShape(mcShape, kObs) ==
          "Johnson::xisignal(decayxim_M, mu[1.321700,1.3200000000000001,1.3300000000000001], "
          "lambda[0.004000,0.002,0.0070000000000000001], gamma[-0.010000,-1,1], delta[1.200000,0.20000000000000001,5])");
    mcShape["lambda"][0] = 0.00412345678;  // as left by the MC fit
    CHECK(constructFitStringDataMCShape(mcShape, kObs) ==
          "Johnson::xisignal(decayxim_M, mu[1.321700,1.3200000000000001,1.3300000000000001], "
          "lambda[0.004123,0.004123,0.008], gamma[-0.010000], delta[1.200000])");
    CHECK(OrderedFitParams(kJohnsonMCShape, mcShape).front().first == "mu");

    // JohnsonMCShapeSyst: legacy GetXSecFilesUML.C variation fit (same MC
    // string, data fit with mu in [1.31,1.33] and lambda in [MC lambda,0.01]).
    CHECK(std::string(kJohnsonMCShapeSyst) == "JohnsonMCShapeSyst");
    CHECK(IsMCShapeFit(kJohnsonMCShape) && IsMCShapeFit(kJohnsonMCShapeSyst) && !IsMCShapeFit("Johnson"));
    CHECK(constructFitStringMCShape(mcShape, kObs, kJohnsonMCShapeSyst) == constructFitStringMCShape(mcShape, kObs));
    CHECK(constructFitStringDataMCShape(mcShape, kObs, kJohnsonMCShapeSyst) ==
          "Johnson::xisignal(decayxim_M, mu[1.321700,1.3100000000000001,1.3300000000000001], "
          "lambda[0.004123,0.004123,0.01], gamma[-0.010000], delta[1.200000])");
    CHECK(OrderedFitParams(kJohnsonMCShapeSyst, mcShape).front().first == "mu");
    CHECK(constructFitStringDataMCShape(mcShape, other_obs, kJohnsonMCShapeSyst) ==
          "Johnson::xisignal(ximstar_M, mu[1.321700,1.3100000000000001,1.3300000000000001], "
          "lambda[0.004123,0.004123,0.01], gamma[-0.010000], delta[1.200000])");
    CHECK(Throws([&] { constructFitStringDataMCShape(mcShape, kObs, "Johnson"); }));
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
                        "Johnson", johnson, fluxDir + "/tables", kPhysics, "hybrid_combo");
    }));

    // Directory mode: a variation file (one vary_<cut>_<value> directory plus its
    // _mc sibling) gives tables named <name>_<directory>; empty trees fail the
    // entry gate, so each table holds one zero row without any fit running.
    {
        const std::string vdir = fluxDir + "/vartables";
        gSystem->mkdir(vdir.c_str(), true);
        const std::vector<std::string> bins{"emin_6.40_emax_7.40", "emin_6.40_emax_7.40_tmin_0.10_tmax_0.35"};
        auto writeTrees = [&](TDirectory* d) {
            for (const auto& bin : bins) {
                d->cd();
                Double_t mass = 1.32, w = 1, q = 1;
                TTree tree(bin.c_str(), bin.c_str());
                tree.Branch("decayxim_M", &mass);
                tree.Branch("hybrid_combo", &w);
                tree.Branch("qvalue_decayxim_M", &q);
                tree.Write();
            }
        };
        {
            TFile variations((vdir + "/variations.root").c_str(), "RECREATE");
            for (const char* dirName : {"vary_chisqndf_5", "vary_chisqndf_5_mc", "vary_chisqndf_7", "vary_chisqndf_7_mc"})
                writeTrees(variations.mkdir(dirName));
            variations.Write();
        }
        {
            TFile thrownFile((vdir + "/thrown.root").c_str(), "RECREATE");
            writeTrees(&thrownFile);
            thrownFile.Write();
        }
        FitParams syst = mcShape;
        WriteXSecTables(vdir + "/variations.root", vdir + "/variations.root", vdir + "/thrown.root", flux,
                        "flatTree_x", "lab", kJohnsonMCShapeSyst, syst, vdir + "/out", kPhysics, "hybrid_combo");
        auto lines = [](const std::string& path) {
            std::ifstream in(path);
            int n = 0;
            std::string line;
            while (std::getline(in, line)) ++n;
            return in.good() || n > 0 ? n : -1;
        };
        for (const char* v : {"vary_chisqndf_5", "vary_chisqndf_7"}) {
            const std::string stem = std::string("flatTree_x_") + v;
            CHECK(lines(vdir + "/out/totxsec_" + stem + ".txt") == 2);
            CHECK(lines(vdir + "/out/totout_" + stem + ".txt") == 2);
            CHECK(lines(vdir + "/out/diffxsec_" + stem + "_emin_6.40_emax_7.40.txt") == 2);
            CHECK(lines(vdir + "/out/diffout_" + stem + "_emin_6.40_emax_7.40.txt") == 2);
        }
        CHECK(gSystem->AccessPathName((vdir + "/out/totxsec_flatTree_x_vary_chisqndf_5_mc.txt").c_str()));
        CHECK(gSystem->AccessPathName((vdir + "/out/totxsec_flatTree_x.txt").c_str()));
    }


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

    // MCPdf: signal shape from an MC tree, data = same signal + flat background.
    {
        TRandom3 rng(7);
        auto makeTree = [&](const char* name, int nSig, int nBkg) {
            auto* t = new TTree(name, name);
            double m = 0, w = 1;
            t->Branch("decayxim_M", &m);
            t->Branch("hybrid_combo", &w);
            for (int i = 0; i < nSig; ++i) { m = rng.Gaus(1.3217, 0.006); if (m > 1.275 && m < 1.45) t->Fill(); }
            for (int i = 0; i < nBkg; ++i) { m = rng.Uniform(1.275, 1.45); t->Fill(); }
            return t;
        };
        TTree* mc = makeTree("mc", 20000, 0);
        TTree* data = makeTree("data", 2000, 1000);
        double yMC = 0, yMCe = 0, y = 0, ye = 0;
        RooFitMCPdf(mc, data, "unit", {"mcPdf", "unit", "bin"}, &yMC, &yMCe, &y, &ye, kObs, kWin, "hybrid_combo", 2);
        CHECK(std::abs(yMC - mc->GetEntries()) < 1e-6);
        CHECK(std::abs(y - 2000) < 150);
        CHECK(std::abs(ye - std::sqrt(y)) < 1e-9); // legacy: sqrt(N), not the fit error
        double y1 = 0, y1e = 0, a = 0, b = 0;
        RooFitMCPdf(mc, data, "unit", {"mcPdf_cheby1", "unit", "bin"}, &a, &b, &y1, &y1e, kObs, kWin, "hybrid_combo", 1);
        CHECK(std::abs(y1 - 2000) < 150);
        CHECK(IsMCPdfFit("MCPdf") && !IsMCPdfFit("Johnson"));
    }

    // MCPdf guards: empty trees and non-positive weight sums give zeros, no fit.
    {
        TRandom3 rng(11);
        auto makeW = [&](const char* name, int n, double wt) {
            auto* t = new TTree(name, name);
            double m = 0, w = wt;
            t->Branch("decayxim_M", &m);
            t->Branch("hybrid_combo", &w);
            for (int i = 0; i < n; ++i) { m = rng.Uniform(1.28, 1.44); t->Fill(); }
            return t;
        };
        auto allZero = [](double a, double b, double c, double d) {
            return a == 0 && b == 0 && c == 0 && d == 0;
        };
        TTree* good = makeW("g_mc", 5000, 1.0);
        TTree* dgood = makeW("g_data", 1000, 1.0);
        double a = -1, b = -1, c = -1, d = -1;
        RooFitMCPdf(makeW("e_mc", 0, 1.0), dgood, "unit", {"mcPdf_g", "unit", "bin"}, &a, &b, &c, &d, kObs, kWin, "hybrid_combo", 2);
        CHECK(allZero(a, b, c, d));
        a = b = c = d = -1;
        RooFitMCPdf(good, makeW("e_data", 0, 1.0), "unit", {"mcPdf_g", "unit", "bin"}, &a, &b, &c, &d, kObs, kWin, "hybrid_combo", 2);
        CHECK(allZero(a, b, c, d));
        a = b = c = d = -1;
        RooFitMCPdf(makeW("neg_mc", 500, -1.0), dgood, "unit", {"mcPdf_g", "unit", "bin"}, &a, &b, &c, &d, kObs, kWin, "hybrid_combo", 2);
        CHECK(allZero(a, b, c, d));
        // Weighted MC: yieldMC is the weight sum.
        a = b = c = d = -1;
        TTree* half = makeW("half_mc", 4000, 0.5);
        RooFitMCPdf(half, dgood, "unit", {"mcPdf_g", "unit", "bin"}, &a, &b, &c, &d, kObs, kWin, "hybrid_combo", 2);
        CHECK(std::abs(a - 2000.0) < 1e-6);
        CHECK(std::abs(b - std::sqrt(2000.0)) < 1e-6);
    }

    // MCPdf through WriteXSecTables: no fit parameters; a finite positive sigma on
    // synthetic trees, and a zero row (not NaN/inf) when the MC has no usable weight.
    {
        const std::string mdir = fluxDir + "/mcpdftables";
        gSystem->mkdir(mdir.c_str(), true);
        const std::vector<std::string> bins{"emin_6.40_emax_7.40", "emin_6.40_emax_7.40_tmin_0.10_tmax_0.35"};
        TRandom3 rng(23);
        auto writeTrees = [&](const std::string& path, int nSig, int nBkg, double wt, int nOut) {
            TFile f(path.c_str(), "RECREATE");
            for (const auto& bin : bins) {
                TTree tree(bin.c_str(), bin.c_str());
                Double_t m = 0, w = wt, q = 1;
                tree.Branch("decayxim_M", &m);
                tree.Branch("hybrid_combo", &w);
                tree.Branch("qvalue_decayxim_M", &q);
                for (int i = 0; i < nSig; ++i) { m = rng.Gaus(1.3217, 0.006); if (m > 1.275 && m < 1.45) tree.Fill(); }
                for (int i = 0; i < nBkg; ++i) { m = rng.Uniform(1.275, 1.45); tree.Fill(); }
                m = 0; // outside the fit window
                for (int i = 0; i < nOut; ++i) tree.Fill();
                tree.Write();
            }
        };
        writeTrees(mdir + "/data.root", 2000, 1000, 1.0, 0);
        writeTrees(mdir + "/mc.root", 5000, 0, 1.0, 0);
        writeTrees(mdir + "/mcneg.root", 0, 0, -1.0, 500);
        writeTrees(mdir + "/thrown.root", 0, 0, 1.0, 50000);
        TH1D bigFlux("f", "", 10, 6.4, 11.4);
        for (int i = 1; i <= 10; ++i) bigFlux.SetBinContent(i, 1.0e6);
        auto sigmaOf = [](const std::string& path) {
            std::ifstream in(path);
            std::string line;
            std::getline(in, line); // header
            double a = 0, sigma = -1;
            in >> a >> sigma;
            return sigma;
        };
        FitParams none;
        WriteXSecTables(mdir + "/data.root", mdir + "/mc.root", mdir + "/thrown.root", &bigFlux, "n", "mcPdf",
                        kMCPdf, none, mdir + "/ok", kPhysics, "hybrid_combo");
        const double tot = sigmaOf(mdir + "/ok/totxsec_n.txt");
        const double diff = sigmaOf(mdir + "/ok/diffxsec_n_emin_6.40_emax_7.40.txt");
        CHECK(std::isfinite(tot) && tot > 0);
        CHECK(std::isfinite(diff) && diff > 0);
        WriteXSecTables(mdir + "/data.root", mdir + "/mcneg.root", mdir + "/thrown.root", &bigFlux, "n", "mcPdf",
                        kMCPdf, none, mdir + "/zero", kPhysics, "hybrid_combo");
        CHECK(sigmaOf(mdir + "/zero/totxsec_n.txt") == 0);
        CHECK(sigmaOf(mdir + "/zero/diffxsec_n_emin_6.40_emax_7.40.txt") == 0);
    }

    // Number fidelity: the channel values reach the apps as the text the stage writes
    // (python str() of the YAML float); std::stod must give back the double of the old
    // C++ literal, bit for bit, and the target density must equal the legacy expression.
    {
        auto same = [](double a, double b) { return std::memcmp(&a, &b, sizeof a) == 0; };
        const std::vector<std::pair<std::string, double>> texts{
            {"0.641", 0.641}, {"0.005", 0.005}, {"50.4", 50.4}, {"79.1", 79.1}, {"0.07008", 70.08e-3},
            {"2.01588", 2.01588}, {"2", 2}};
        for (const auto& t : texts)
            CHECK(same(gxana::cli::ParseDouble(t.first), t.second));
        Double_t Na = 6.022e23; Double_t tar_Len = 79.1-50.4; Double_t tar_Den = 70.08e-3; Double_t hy_MM = 2.01588;
        const double legacy = 2 * Na * tar_Len * tar_Den * pow(10,-24) / hy_MM;
        CHECK(same(TargetDensity(gxana::cli::ParseTarget("50.4,79.1,0.07008,2.01588,2")), legacy));
        double br = 0, brErr = 0;
        gxana::cli::ParseBranchingRatio("0.641,0.005", br, brErr);
        CHECK(same(br, 0.641) && same(brErr, 0.005));
        CHECK(Throws([] { gxana::cli::ParseTarget("79.1,50.4,0.07008,2.01588,2"); }));
        CHECK(Throws([] { gxana::cli::ParseTarget("50.4,79.1,0.07008,2.01588"); }));
        CHECK(Throws([] { double a, b; gxana::cli::ParseBranchingRatio("0.641", a, b); }));
        CHECK(gxana::cli::ParseQValueBranch("none").empty());
        const std::vector<std::pair<std::string, double>> windowTexts{
            {"lo=1.27", 1.27}, {"mc_hi=1.4", 1.40}, {"mc_signal_hi=1.38", 1.38}, {"mc_plot_hi=1.42", 1.42},
            {"data_hi=1.45", 1.45}, {"data_edge=1.28", 1.28}, {"mcpdf_data_lo=1.275", 1.275}};
        MassWindows w{};
        for (const auto& t : windowTexts)
            gxana::cli::SetMassWindow(w, t.first);
        CHECK(same(w.lo, kWin.lo) && same(w.mcHi, kWin.mcHi) && same(w.mcSignalHi, kWin.mcSignalHi)
              && same(w.mcPlotHi, kWin.mcPlotHi) && same(w.dataHi, kWin.dataHi) && same(w.dataEdge, kWin.dataEdge)
              && same(w.mcPdfDataLo, kWin.mcPdfDataLo));
        CHECK(same(kWin.mcHi, 1.40) && same(kWin.mcPdfDataLo, 1.275));
        CHECK(Throws([&] { gxana::cli::SetMassWindow(w, "hi=1.5"); }));
        CHECK(Throws([&] { gxana::cli::SetMassWindow(w, "lo=x"); }));
        CHECK(gxana::cli::ParseQValueBranch("qvalue_x") == "qvalue_x");
    }

    // Another channel: its own observable branch and weight, and no Q-factors (empty
    // qvalueBranch): the trees need no Q-value branch and the qval columns are nan.
    {
        const std::string qdir = fluxDir + "/noqvalue";
        gSystem->mkdir(qdir.c_str(), true);
        const std::vector<std::string> bins{"emin_6.40_emax_7.40", "emin_6.40_emax_7.40_tmin_0.10_tmax_0.35"};
        TRandom3 rng(29);
        auto writeTrees = [&](const std::string& path, int nSig, int nOut) {
            TFile f(path.c_str(), "RECREATE");
            for (const auto& bin : bins) {
                TTree tree(bin.c_str(), bin.c_str());
                Double_t m = 0, w = 1;
                tree.Branch("mass_x", &m);
                tree.Branch("w_x", &w);
                for (int i = 0; i < nSig; ++i) { m = rng.Gaus(1.3217, 0.006); if (m > 1.275 && m < 1.45) tree.Fill(); }
                m = 0;
                for (int i = 0; i < nOut; ++i) tree.Fill();
                tree.Write();
            }
        };
        writeTrees(qdir + "/data.root", 3000, 0);
        writeTrees(qdir + "/mc.root", 5000, 0);
        writeTrees(qdir + "/thrown.root", 0, 50000);
        TH1D bigFlux("fq", "", 10, 6.4, 11.4);
        for (int i = 1; i <= 10; ++i) bigFlux.SetBinContent(i, 1.0e6);
        XSecPhysics physics{"(w_x)*(mass_x>1.3&&mass_x<1.35)", "", 0.641, 0.005, {50.4, 79.1, 0.07008, 2.01588, 2},
                            {"mass_x", "M (GeV)"}, kWin};
        FitParams none;
        WriteXSecTables(qdir + "/data.root", qdir + "/mc.root", qdir + "/thrown.root", &bigFlux, "n", "mcPdf",
                        kMCPdf, none, qdir + "/out", physics, "w_x", 2);
        std::ifstream in(qdir + "/out/diffout_n_emin_6.40_emax_7.40.txt");
        std::string header, t, terr, y, ye, qv, qve;
        std::getline(in, header);
        in >> t >> terr >> y >> ye >> qv >> qve;
        CHECK(qv == "nan" && qve == "nan");
        CHECK(std::stod(y) > 0);
    }

    if (failures == 0) std::cout << "test_xsection: all checks passed\n";
    gSystem->Exec(("rm -rf " + fluxDir).c_str());
    return failures == 0 ? 0 : 1;
}
