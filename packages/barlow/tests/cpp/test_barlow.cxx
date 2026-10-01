#include "gxana/barlow/Barlow.h"
#include "gxana/barlow/VariationTrees.h"

#include <ROOT/RDataFrame.hxx>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TKey.h>
#include <TRandom3.h>
#include <TSystem.h>
#include <TTree.h>

#include <sys/wait.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
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

using namespace gxana::barlow;

static void TestCalcBarlow()
{
    // Moved from packages/xsection/tests/cpp/test_xsection.cxx.
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

    // Signed: variation above nominal gives a negative significance.
    TGraphErrors n1(1), v1(1);
    n1.SetPoint(0, 7.0, 4.0); n1.SetPointError(0, 0.5, 0.3);
    v1.SetPoint(0, 7.0, 5.0); v1.SetPointError(0, 0.5, 0.5);
    TGraphErrors* neg = calc_barlow(&n1, &v1);
    CHECK(std::fabs(neg->GetPointY(0) - (-1.0 / 0.4)) < 1e-12);

    // sigma = 0 with a nonzero difference still gives 0 (legacy guard), not inf.
    TGraphErrors n2(1), v2(1);
    n2.SetPoint(0, 7.0, 4.0); n2.SetPointError(0, 0.5, 0.0);
    v2.SetPoint(0, 7.0, 9.0); v2.SetPointError(0, 0.5, 0.0);
    CHECK(calc_barlow(&n2, &v2)->GetPointY(0) == 0.0);
}

static void TestStdDev()
{
    TGraphErrors a(2), b(2);
    a.SetPoint(0, 0.225, 4.0); a.SetPoint(1, 0.44, 6.0);
    b.SetPoint(0, 0.225, 6.0); b.SetPoint(1, 0.44, 6.0);
    TGraphErrors* spread = calculateStdDevGraph({&a, &b});
    CHECK(std::fabs(spread->GetPointY(0) - 1.0) < 1e-12);
    CHECK(spread->GetPointY(1) == 0.0 && spread->GetPointX(1) == 0.44);
    CHECK(calculateStdDevGraph({}) == nullptr);
}

static int Run(const std::string& cmd)
{
    const int status = std::system(cmd.c_str());
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static std::string Quote(const std::string& s) { return "'" + s + "'"; }

// Synthetic flat tree: 20 entries, i = 0..19. beam_E = 6.05 + 0.3 i passes the
// 6.4-11.4 filter for i = 2..17; total_mm2 = -0.03 when i % 3 == 0 else 0.01.
static void WriteSyntheticTree(const std::string& path, bool mc)
{
    ROOT::RDataFrame df(20);
    auto d = df.Define("i", [](ULong64_t e) { return static_cast<int>(e); }, {"rdfentry_"})
                 .Define("beam_E", [](int i) { return 6.05 + 0.3 * i; }, {"i"})
                 .Define("beam_vertexZ", [] { return 60.0; })
                 .Define("chisqndf", [](int i) { return static_cast<double>(i % 10); }, {"i"})
                 .Define("total_mm2", [](int i) { return i % 3 == 0 ? -0.03 : 0.01; }, {"i"});
    if (mc)
        d.Define("beam_E_Truth", [](double e) { return e; }, {"beam_E"})
            .Snapshot("flatTree_test", path);
    else
        d.Snapshot("flatTree_test", path);
}

static long HandCount(double chisqMax)
{
    long n = 0;
    for (int i = 0; i < 20; ++i) {
        const double beamE = 6.05 + 0.3 * i;
        const double mm2 = i % 3 == 0 ? 0.03 : 0.01;
        if (beamE > 6.4 && beamE < 11.4 && (i % 10) < chisqMax && mm2 < 0.02) ++n;
    }
    return n;
}

static void TestTreesApp(const std::string& exe)
{
    const std::string dir = std::string(gSystem->TempDirectory()) + "/gxana_barlow_trees_test";
    gSystem->Exec(("rm -rf " + dir).c_str());
    gSystem->mkdir(dir.c_str(), true);
    WriteSyntheticTree(dir + "/data.root", false);
    WriteSyntheticTree(dir + "/mc.root", true);
    const std::string out = dir + "/vt/variations.root"; // vt/ does not exist yet
    const std::string cmd = exe + " --tree flatTree_test --input " + dir + "/data.root --input-mc " + dir + "/mc.root"
        + " --define " + Quote("total_mm2_abs=fabs(total_mm2)")
        + " --filter-data " + Quote("beam_E>6.4&&beam_E<11.4")
        + " --filter-mc " + Quote("beam_E_Truth>6.4&&beam_E_Truth<11.4")
        + " --filter-data " + Quote("beam_vertexZ>50.4&&beam_vertexZ<79.1")
        + " --filter-mc " + Quote("beam_vertexZ>50.4&&beam_vertexZ<79.1")
        + " --branch beam_E --branch chisqndf --branch total_mm2_abs"
        + " --variation " + Quote("vary_chisqndf_5=chisqndf<5&&total_mm2_abs<0.02")
        + " --variation " + Quote("vary_chisqndf_8=chisqndf<8&&total_mm2_abs<0.02")
        + " --out " + out + " --threads 0";
    for (int pass = 0; pass < 2; ++pass) { // a rerun must leave the same trees
        CHECK(Run(cmd) == 0);
        TFile f(out.c_str(), "READ");
        CHECK(!f.IsZombie());
        std::vector<std::string> keys;
        for (auto* obj : *f.GetListOfKeys())
            keys.push_back(std::string(obj->GetName()) + ";" + std::to_string(static_cast<TKey*>(obj)->GetCycle()));
        std::sort(keys.begin(), keys.end());
        CHECK((keys == std::vector<std::string>{"vary_chisqndf_5;1", "vary_chisqndf_5_mc;1",
                                                "vary_chisqndf_8;1", "vary_chisqndf_8_mc;1"}));
        for (const auto& [name, expected] : std::vector<std::pair<std::string, long>>{
                 {"vary_chisqndf_5", HandCount(5)}, {"vary_chisqndf_5_mc", HandCount(5)},
                 {"vary_chisqndf_8", HandCount(8)}, {"vary_chisqndf_8_mc", HandCount(8)}}) {
            auto* tree = f.Get<TTree>(name.c_str());
            CHECK(tree && tree->GetEntries() == expected);
            CHECK(tree && tree->GetBranch("total_mm2_abs") && tree->GetBranch("chisqndf") && tree->GetBranch("beam_E"));
            CHECK(tree && !tree->GetBranch("total_mm2")); // only --branch columns are written
        }
    }
    CHECK(HandCount(5) == 6 && HandCount(8) == 10); // the hand count itself
    CHECK(Run(exe + " --tree t --input " + dir + "/absent.root --input-mc " + dir + "/mc.root --branch b"
              + " --variation v=x --out " + dir + "/o.root") == 1);
    gSystem->Exec(("rm -rf " + dir).c_str());
}

// Xi-like peak: 80% Gaussian(1.3217, 0.004), 20% flat 1.26-1.45, weight 1.
static void WriteMassTree(const std::string& path, const std::string& name, const std::string& mode)
{
    TRandom3 rng(12345); // single-threaded event loop: the shared generator is safe
    ROOT::RDF::RSnapshotOptions opts;
    opts.fMode = mode;
    ROOT::RDataFrame df(4000);
    df.Define("decayxim_M", [&rng](ULong64_t e) {
          return e % 5 == 0 ? rng.Uniform(1.26, 1.45) : rng.Gaus(1.3217, 0.004); }, {"rdfentry_"})
        .Define("hybrid_combo", [] { return 1.0; })
        .Snapshot(name, path, {"decayxim_M", "hybrid_combo"}, opts);
}

static void TestCheckMode(const std::string& exe)
{
    const std::string dir = std::string(gSystem->TempDirectory()) + "/gxana_barlow_check_test";
    gSystem->Exec(("rm -rf " + dir).c_str());
    gSystem->mkdir(dir.c_str(), true);
    WriteMassTree(dir + "/nominal.root", "flatTree_test", "RECREATE");
    WriteMassTree(dir + "/nominal_mc.root", "flatTree_test", "RECREATE");
    WriteMassTree(dir + "/variations.root", "vary_x_1", "RECREATE");
    WriteMassTree(dir + "/variations.root", "vary_x_1_mc", "UPDATE");
    const std::string yields = dir + "/output_yields.txt";
    const std::string cmd = exe + " --check --tree flatTree_test --out " + dir + "/variations.root"
        + " --nominal " + dir + "/nominal.root --nominal-mc " + dir + "/nominal_mc.root"
        + " --name flatTree_test --weight hybrid_combo --yields " + yields + " --fit-dir " + dir + "/fits"
        + " --variation " + Quote("vary_x_1=x<1");
    CHECK(Run(cmd) == 0);
    std::ifstream in(yields);
    std::string line;
    std::vector<std::string> rows;
    while (std::getline(in, line)) rows.push_back(line);
    CHECK(rows.size() == 1);
    if (rows.size() == 1) {
        std::vector<std::string> fields;
        std::stringstream ss(rows[0]);
        std::string field;
        while (std::getline(ss, field, '\t')) fields.push_back(field);
        CHECK(fields.size() == 8);
        CHECK(fields.size() == 8 && fields[0] == "flatTree_test" && fields[1] == "x_1");
        // Same events for nominal and variation: identical fits, 0% difference.
        CHECK(fields.size() == 8 && std::stod(fields[2]) > 0 && std::fabs(std::stod(fields[4])) < 1e-6);
        CHECK(fields.size() == 8 && std::stod(fields[5]) > 0 && std::fabs(std::stod(fields[7])) < 1e-6);
    }
    CHECK(!gSystem->AccessPathName((dir + "/fits/recon_flatTree_test_x_1.pdf").c_str()));
    CHECK(!gSystem->AccessPathName((dir + "/fits/data_flatTree_test_x_1.pdf").c_str()));
    // A second run appends (the stage truncates the file once before the step).
    CHECK(Run(cmd) == 0);
    std::ifstream again(yields);
    int count = 0;
    while (std::getline(again, line)) ++count;
    CHECK(count == 2);
    CHECK(Run(exe + " --check --tree flatTree_test --out " + dir + "/variations.root --nominal " + dir
              + "/absent.root --nominal-mc " + dir + "/nominal_mc.root --name n --weight hybrid_combo --yields "
              + yields + " --fit-dir " + dir + "/fits --variation v=x") == 1);
    gSystem->Exec(("rm -rf " + dir).c_str());
}

static void WriteText(const std::string& path, const std::string& text)
{
    std::ofstream(path) << text;
}

struct Row { std::string id; double x, yn, eyn, yv, eyv, sb; };

static std::vector<Row> ReadSigmaB(const std::string& path)
{
    std::vector<Row> rows;
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        Row r;
        ss >> r.id >> r.x >> r.yn >> r.eyn >> r.yv >> r.eyv >> r.sb;
        rows.push_back(r);
    }
    return rows;
}

static double Expected(double yn, double eyn, double yv, double eyv)
{
    const double sigma = std::sqrt(std::fabs(eyn * eyn - eyv * eyv));
    return sigma != 0.0 ? (yn - yv) / sigma : 0.0;
}

static void TestPlotApp(const std::string& exe)
{
    const std::string dir = std::string(gSystem->TempDirectory()) + "/gxana_barlow_plot_test";
    gSystem->Exec(("rm -rf " + dir).c_str());
    gSystem->mkdir((dir + "/nom").c_str(), true);
    gSystem->mkdir((dir + "/var").c_str(), true);
    // Nominal tables have the package header (not '#') and a 5th column, as
    // xsection/weighted_data/johnson; variations the legacy 4-column form.
    WriteText(dir + "/nom/totxsec_weighted_output.txt",
              "-t d\\sigma/dt \\delta_x \\delta_y S\n6.9 7.0 0.5 0.3 0.6\n7.63 6.8 0.23 0.25 0.2\n");
    WriteText(dir + "/var/weighted_totxsec_vary_f_1.txt", "# X Y_weighted EX EY_weighted\n6.9 7.4 0.5 0.2\n7.63 6.8 0.23 0.25\n");
    WriteText(dir + "/var/weighted_totxsec_vary_f_2.txt", "# X Y_weighted EX EY_weighted\n6.9 6.5 0.5 0.4\n7.63 7.1 0.23 0.1\n");
    WriteText(dir + "/nom/weighted_diffxsec_emin_6.40_emax_7.40.txt", "-t d\\sigma/dt \\delta_x \\delta_y S\n0.2 3.0 0.1 0.3 1\n");
    WriteText(dir + "/var/weighted_diffxsec_vary_f_1_emin_6.40_emax_7.40.txt", "# X Y_weighted EX EY_weighted\n0.2 2.5 0.1 0.2\n");
    WriteText(dir + "/var/weighted_diffxsec_vary_f_2_emin_6.40_emax_7.40.txt", "# X Y_weighted EX EY_weighted\n0.2 3.2 0.1 0.1\n");
    const std::string base = exe + " --nominal-dir " + dir + "/nom --var-dir " + dir + "/var --family f"
        + " --label " + Quote("#chi^{2}_{#nu} < ") + " --variation f_1=1 --variation f_2=2 --energy 6.40:7.40"
        + " --canvas 800,800 --legend-diff 0.72,0.5,0.93,0.9 --legend-tot 0.72,0.5,0.93,0.9 --y-floor 8"
        + " --y-pad-diff 0 --canvas-def-w 600 --title-offset-y 0.8 --title-offsets-diff 0.9,0.3"
        + " --title-offsets-tot 0.9,0.3 --tot-y-ndiv 1 --threshold 4 --out-dir " + dir + "/plots";
    CHECK(Run(base) == 0);
    for (const std::string stem : {"barlow_weighted_totxsec_vary_f", "barlow_weighted_diffxsec_vary_f_emin_6.40_emax_7.40"}) {
        CHECK(!gSystem->AccessPathName((dir + "/plots/" + stem + ".pdf").c_str()));
        CHECK(!gSystem->AccessPathName((dir + "/plots/" + stem + ".txt").c_str()));
    }
    const auto tot = ReadSigmaB(dir + "/plots/barlow_weighted_totxsec_vary_f.txt");
    CHECK(tot.size() == 4);
    if (tot.size() == 4) {
        CHECK(tot[0].id == "f_1" && tot[2].id == "f_2" && tot[1].x == 7.63);
        CHECK(std::fabs(tot[0].sb - Expected(7.0, 0.3, 7.4, 0.2)) < 1e-9);
        CHECK(tot[1].sb == 0.0); // equal errors
        CHECK(std::fabs(tot[3].sb - Expected(6.8, 0.25, 7.1, 0.1)) < 1e-9);
    }
    const auto diff = ReadSigmaB(dir + "/plots/barlow_weighted_diffxsec_vary_f_emin_6.40_emax_7.40.txt");
    CHECK(diff.size() == 2 && std::fabs(diff[1].sb - Expected(3.0, 0.3, 3.2, 0.1)) < 1e-9);

    // A variation with a missing point, or a shifted x, is an error.
    WriteText(dir + "/var/weighted_totxsec_vary_f_2.txt", "# X Y_weighted EX EY_weighted\n6.9 6.5 0.5 0.4\n");
    CHECK(Run(base) == 1);
    WriteText(dir + "/var/weighted_totxsec_vary_f_2.txt", "# X Y_weighted EX EY_weighted\n6.9 6.5 0.5 0.4\n7.7 7.1 0.23 0.1\n");
    CHECK(Run(base) == 1);
    // A missing input is an error too.
    gSystem->Unlink((dir + "/var/weighted_totxsec_vary_f_2.txt").c_str());
    CHECK(Run(base) == 1);
    gSystem->Exec(("rm -rf " + dir).c_str());
}

// NAME=VALUE split of --define / --variation: result and exact message.
static void TestSplitAssign()
{
    CHECK(SplitAssign("n=x<=1") == std::make_pair(std::string("n"), std::string("x<=1")));
    CHECK(SplitAssign("n=") == std::make_pair(std::string("n"), std::string("")));
    for (const std::string bad : {"=x", "nox"}) {
        std::string what;
        try { SplitAssign(bad); } catch (const std::invalid_argument& e) { what = e.what(); }
        CHECK(what == "expected NAME=VALUE: '" + bad + "'");
    }
}

int main(int argc, char** argv)
{
    TestSplitAssign();
    TestCalcBarlow();
    TestStdDev();
    if (argc > 1) { TestTreesApp(argv[1]); TestCheckMode(argv[1]); }
    else { std::cerr << "test_barlow: no gxana_barlow_trees path given\n"; ++failures; }
    if (argc > 2) TestPlotApp(argv[2]);
    else { std::cerr << "test_barlow: no gxana_barlow_plot path given\n"; ++failures; }
    if (failures == 0) std::cout << "test_barlow: all checks passed\n";
    return failures == 0 ? 0 : 1;
}
