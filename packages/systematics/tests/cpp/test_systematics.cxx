#include "gxana/systematics/PlotSpread.h"

#include <TGraphErrors.h>
#include <TSystem.h>

#include <sys/wait.h>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
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

using namespace gxana::systematics;

static std::string TempDir(const std::string& name)
{
    const std::string dir = std::string(gSystem->TempDirectory()) + "/" + name;
    gSystem->Exec(("rm -rf " + dir).c_str());
    gSystem->mkdir(dir.c_str(), true);
    return dir;
}

static void WriteText(const std::string& path, const std::string& text)
{
    std::ofstream(path) << text;
}

static void TestReadLabelGraphs()
{
    const std::string dir = TempDir("gxana_systematics_label");
    WriteText(dir + "/weighted_diffxsec_emin_10.18_emax_11.40.txt",
              "# X Y_weighted EX EY_weighted\n0.2 3.5 0.1 0.3\n0.6 2.5 0.1 0.2\n");
    WriteText(dir + "/weighted_diffxsec_emin_6.40_emax_7.40.txt",
              "# X Y_weighted EX EY_weighted\n0.2 5.25 0.1 0.4\n0.6 4.0 0.1 0.3\n");
    WriteText(dir + "/syst_weighted_diffxsec_emin_6.40_emax_7.40.txt", "0.2 99 0.1 1\n");
    const auto graphs = ReadLabelGraphs(dir);
    CHECK(graphs.size() == 2);
    if (graphs.size() == 2) {
        CHECK(graphs[0]->GetN() == 2 && graphs[1]->GetN() == 2);
        CHECK(graphs[0]->GetY()[0] == 5.25 && graphs[1]->GetY()[0] == 3.5);
        CHECK(std::string(graphs[0]->GetTitle()) == "#bf{E_{#gamma} (GeV): (6.40, 7.40)}");
    }
    gSystem->Exec(("rm -rf " + dir).c_str());
}

static void TestReadBandGraphs()
{
    const std::string dir = TempDir("gxana_systematics_band");
    WriteText(dir + "/stats.txt", "XVal XErr YMean StdDev\n0.2 0.1 5 0.5\n0.6 0.1 4 0.25\n0.2 0.1 3 0.125\n0.6 0.1 2 1\n");
    TGraphErrors a(2), b(2);
    const auto band = ReadBandGraphs(dir + "/stats.txt", {&a, &b});
    CHECK(band.size() == 2);
    if (band.size() == 2) {
        CHECK(band[0]->GetN() == 2 && band[1]->GetN() == 2);
        CHECK(band[0]->GetEY()[0] == 0.5 && band[0]->GetEY()[1] == 0.25);
        CHECK(band[1]->GetEY()[0] == 0.125 && band[1]->GetEY()[1] == 1.0);
        CHECK(band[1]->GetY()[0] == 3.0 && band[1]->GetX()[1] == 0.6 && band[1]->GetEX()[1] == 0.1);
    }
    gSystem->Exec(("rm -rf " + dir).c_str());
}

static void TestGetAvgStdDev()
{
    TGraphErrors a(3), b(3);
    const double y1[] = {1, 2, 3}, y2[] = {2, 2, 2};
    for (int i = 0; i < 3; ++i) {
        a.SetPoint(i, i, y1[i]); a.SetPointError(i, 0, 0.1);
        b.SetPoint(i, i, y2[i]); b.SetPointError(i, 0, 0.1);
    }
    const GraphStats s = GetAvgStdDev(a, b);
    // legacy: totalPercentDifference/nPoints of ((y2 - y1) / y1) * 100.0
    double total = 0, maxPct = 0;
    for (int i = 0; i < 3; ++i) {
        const double pct = ((y2[i] - y1[i]) / y1[i]) * 100.0;
        if (std::abs(pct) > std::abs(maxPct)) maxPct = pct;
        total += pct;
    }
    CHECK(std::fabs(s.avg_pct_diff - total / 3) < 1e-12);
    CHECK(std::fabs(s.max_pct_diff - maxPct) < 1e-12 && maxPct == 100.0);
    CHECK(s.mean_diff == 0.0); // differences -1, 0, 1
}

static int RunCapture(const std::string& cmd, std::string& out)
{
    out.clear();
    FILE* pipe = popen((cmd + " 2>&1").c_str(), "r");
    if (!pipe) return -1;
    char buf[512];
    while (fgets(buf, sizeof(buf), pipe)) out += buf;
    const int status = pclose(pipe);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static void TestApp(const std::string& exe)
{
    std::string out;
    CHECK(RunCapture(exe + " --layout grid3 --name x --out-dir /tmp --input /nonexistent", out) != 0);
    CHECK(out.find("needs at least 2") != std::string::npos);
    CHECK(RunCapture(exe + " --layout grid3 --name x --out-dir /tmp --input a --input b --legend nobar", out) != 0);
    CHECK(out.find("text|opt") != std::string::npos);
    CHECK(RunCapture(exe + " --layout pair_band --name x --out-dir /tmp --input a --input b", out) != 0);
    CHECK(out.find("needs --band") != std::string::npos);
    CHECK(RunCapture(exe + " --help", out) == 0 && out.find("usage:") != std::string::npos);
}

int main(int argc, char** argv)
{
    TestReadLabelGraphs();
    TestReadBandGraphs();
    TestGetAvgStdDev();
    if (argc > 1) TestApp(argv[1]);
    else { std::cerr << "test_systematics: no gxana_syst_plot path given\n"; ++failures; }
    if (failures == 0) std::cout << "test_systematics: all checks passed\n";
    return failures == 0 ? 0 : 1;
}
