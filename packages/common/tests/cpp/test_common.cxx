#include "gxana/common/GraphIO.h"
#include "gxana/common/Paths.h"
#include "gxana/common/Strings.h"
#include "gxana/common/Style.h"

#include <TGraphErrors.h>
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
    CHECK(gxana::NumericCompare("x_emin_8.19.txt", "y_emin_8.45.txt"));  // full value, not the integer part
    CHECK(!gxana::NumericCompare("y_emin_8.45.txt", "x_emin_8.19.txt"));
    CHECK(gxana::NumericCompare("a_emin_7.40.txt", "b_emin_7.86.txt"));
    CHECK(gxana::NumericCompare("a_emin_8.19.txt", "b_emin_8.19.txt"));  // equal value: by name
    CHECK(!gxana::NumericCompare("b_emin_8.19.txt", "a_emin_8.19.txt"));
    CHECK(!gxana::NumericCompare("a_emin_8.19.txt", "a_emin_8.19.txt")); // irreflexive
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
    // Panels with equal integer parts of emin come out in ascending emin order.
    const std::string sub = dir + "/order";
    gSystem->Exec(("mkdir -p " + sub).c_str());
    for (const char* name : {"diffxsec_emin_8.68_emax_9.26.txt", "diffxsec_emin_8.19_emax_8.45.txt",
                             "diffxsec_emin_8.45_emax_8.68.txt"}) {
        std::ofstream(sub + "/" + name) << "tBinCenter\tdsigmadt\ttBinWidth\tYerr\n"
                                           "0.225  4.5  0.125  0.4\n";
    }
    auto g3 = gxana::CreateTGraphErrorsFromTxt(sub, "diffxsec*");
    CHECK(g3.size() == 3);
    if (g3.size() == 3) {
        CHECK(std::string(g3[0]->GetTitle()).find("(8.19, 8.45)") != std::string::npos);
        CHECK(std::string(g3[1]->GetTitle()).find("(8.45, 8.68)") != std::string::npos);
        CHECK(std::string(g3[2]->GetTitle()).find("(8.68, 9.26)") != std::string::npos);
    }
    gSystem->Exec(("rm -rf " + dir).c_str());

    if (failures == 0) std::cout << "test_common: all checks passed\n";
    return failures == 0 ? 0 : 1;
}
