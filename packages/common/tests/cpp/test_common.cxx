#include "gxana/common/GraphIO.h"
#include "gxana/common/Paths.h"
#include "gxana/common/StackedHist.h"
#include "gxana/common/Strings.h"
#include "gxana/common/Style.h"

#include <TGraphErrors.h>
#include <TH1D.h>
#include <TStyle.h>
#include <TSystem.h>

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>

static int failures = 0;
#define CHECK(cond)                                                                   \
    do {                                                                              \
        if (!(cond)) {                                                                \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #cond "\n"; \
            ++failures;                                                               \
        }                                                                             \
    } while (0)

int main()
{
    CHECK(gxana::NumericCompare("diffxsec_emin_7.40_emax_7.86.txt", "diffxsec_emin_10.18_emax_11.40.txt"));
    CHECK(!gxana::NumericCompare("diffxsec_emin_10.18.txt", "diffxsec_emin_7.40.txt"));
    CHECK(!gxana::NumericCompare("x_emin_8.19.txt", "y_emin_8.45.txt")); // equal integer parts
    bool threw = false;
    try { gxana::NumericCompare("no_marker.txt", "x_emin_7.txt"); } catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);

    setenv("GXANA_DATA", "/data/", 1);
    CHECK(gxana::EnvPath("GXANA_DATA", "Trees/x.root") == "/data/Trees/x.root");
    setenv("GXANA_DATA", "/data", 1);
    CHECK(gxana::EnvPath("GXANA_DATA", "Trees") == "/data/Trees");
    CHECK(gxana::EnvPath("GXANA_DATA") == "/data");
    unsetenv("GXANA_DATA");
    threw = false;
    try { gxana::EnvPath("GXANA_DATA", "x"); } catch (const std::runtime_error&) { threw = true; }
    CHECK(threw);

    gxana::SetStyle();
    CHECK(gStyle->GetOptStat() == 0);
    CHECK(std::fabs(gStyle->GetPadLeftMargin() - 0.13f) < 1e-6);
    CHECK(gStyle->GetCanvasDefW() == 700);

    // GraphIO: text tables -> TGraphErrors, ROOT file round trip.
    char tmpl[] = "/tmp/gxana_graphio_XXXXXX";
    const std::string dir = mkdtemp(tmpl);
    for (const char* name : {"diffxsec_P_emin_10.18_emax_11.40.txt", "diffxsec_P_emin_7.40_emax_7.86.txt"}) {
        std::ofstream(dir + "/" + name) << "tBinCenter\tdsigmadt\ttBinWidth\tYerr\n"
                                           "0.225  4.5  0.125  0.4\n0.44  6.4  0.09  0.5\n";
    }
    std::ofstream(dir + "/totxsec_P.txt") << "x\n1 2 3 4\n"; // not matched by the pattern
    auto graphs = gxana::CreateTGraphErrorsFromTxt(dir, "diffxsec*", dir + "/graphs.root");
    CHECK(graphs.size() == 2);
    CHECK(std::string(graphs[0]->GetName()) == "Graph_diffxsec_P_emin_7.40_emax_7.86");
    CHECK(std::string(graphs[0]->GetTitle()) == "#bf{E_{#gamma} (GeV): (7.40, 7.86)}");
    CHECK(graphs[0]->GetN() == 2);
    CHECK(std::fabs(graphs[0]->GetErrorY(1) - 0.5) < 1e-12);
    CHECK(gxana::GetAllTGraphErrors((dir + "/graphs.root").c_str()).size() == 2);
    CHECK(gxana::CreateTGraphErrorsFromTxt(dir, "diffxsec*").size() == 2); // writes no file
    CHECK(gxana::CreateTGraphErrorsFromTxt(dir, "nomatch*").empty());
    CHECK(gxana::GetAllTGraphErrors((dir + "/absent.root").c_str()).empty());
    gSystem->Exec(("rm -rf " + dir).c_str());

    // MakeStackedHist: writes "<plot_dir>/<arr_hist[0] name>_<identifier>_ac.pdf".
    {
        std::string stackDir = std::string(gSystem->TempDirectory()) + "/gxana_stack_test";
        gSystem->mkdir(stackDir.c_str(), true);
        TH1D data("d", "d", 10, 0, 1), mc("m", "m", 10, 0, 1);
        data.FillRandom("gaus", 100);
        mc.FillRandom("gaus", 100);
        gxana::MakeStackedHist({&data, &mc}, "stack", "unit", "tr", "Data", stackDir);
        CHECK(!gSystem->AccessPathName((stackDir + "/d_unit_ac.pdf").c_str()));
        gSystem->Exec(("rm -rf " + stackDir).c_str());
    }

    if (failures == 0) std::cout << "test_common: all checks passed\n";
    return failures == 0 ? 0 : 1;
}
