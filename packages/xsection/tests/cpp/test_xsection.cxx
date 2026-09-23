#include "CliArgs.h"
#include "gxana/xsection/Binning.h"

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
    CHECK(Throws([] { gxana::cli::ParseJob("n:d:m:t"); }));

    if (failures == 0) std::cout << "test_xsection: all checks passed\n";
    return failures == 0 ? 0 : 1;
}
