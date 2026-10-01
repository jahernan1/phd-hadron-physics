// Bin names: the common formatters and ParseBinName against frozen copies of the code they
// replace (the gxana::xsec formatters of Binning.cxx and the substr parses that XSec.cxx
// took over from AnalysisNote/xsection/FitFunctions.cpp), over every configured bin
// (analyses/kpkpxim/config/binning.yaml) and the truncation edge cases.
#include "gxana/common/BinNames.h"

#include <cstdio>
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

namespace frozen {

// packages/xsection/src/Binning.cxx:85-99 before S1.
std::string BinEdgeLabel(double edge)
{
    std::string label = std::to_string(edge);
    return label.substr(0, label.find(".") + 3);
}
std::string EnergyBinName(double lowE, double highE)
{
    return "emin_" + BinEdgeLabel(lowE) + "_emax_" + BinEdgeLabel(highE);
}
std::string BinName(double lowE, double highE, double lowT, double highT)
{
    return EnergyBinName(lowE, highE) + "_tmin_" + BinEdgeLabel(lowT) + "_tmax_" + BinEdgeLabel(highT);
}

// AnalysisNote/xsection/FitFunctions.cpp:80-86 (GetDiffXSecFile; packages/xsection/src/XSec.cxx:66-72
// before S1), statements verbatim.
std::vector<std::string> ParseDiff(const std::string& treeName)
{
    std::string emin = treeName.substr(treeName.find("emin")+5);
    emin = emin.substr(0,emin.find("_"));
    std::string emax = treeName.substr(treeName.find("emax")+5);
    emax = emax.substr(0,emax.find("_"));
    std::string tmin = treeName.substr(treeName.find("tmin")+5);
    tmin = tmin.substr(0,tmin.find("_"));
    std::string tmax = treeName.substr(treeName.find("tmax")+5);
    return {emin, emax, tmin, tmax};
}

// AnalysisNote/xsection/FitFunctions.cpp:175-177 (GetTotXSecFile; packages/xsection/src/XSec.cxx:187-189
// before S1), statements verbatim.
std::vector<std::string> ParseTot(const std::string& treeName)
{
    std::string emin = treeName.substr(treeName.find("emin")+5);
    emin = emin.substr(0,emin.find("_"));
    std::string emax = treeName.substr(treeName.find("emax")+5);
    return {emin, emax};
}

} // namespace frozen

int main()
{
    // analyses/kpkpxim/config/binning.yaml
    const std::vector<double> energy{6.40, 7.40, 7.86, 8.19, 8.45, 8.68, 9.26, 10.18, 11.40};
    const std::vector<std::pair<double, double>> tBins{{0.10, 0.35}, {0.35, 0.53}, {0.53, 0.71}, {0.71, 0.92},
                                                       {0.92, 1.19}, {1.19, 1.53}, {1.53, 2.40}};
    int names = 0;
    for (size_t e = 0; e + 1 < energy.size(); ++e) {
        const double lo = energy[e], hi = energy[e + 1];
        const std::string en = gxana::EnergyBinName(lo, hi);
        CHECK(en == frozen::EnergyBinName(lo, hi));
        CHECK(en == gxana::EnergyBinName(gxana::BinEdgeLabel(lo), gxana::BinEdgeLabel(hi)));
        const gxana::BinNameParts ep = gxana::ParseBinName(en);
        const auto et = frozen::ParseTot(en);
        CHECK(!ep.hasT && ep.emin == et[0] && ep.emax == et[1]);
        CHECK(gxana::EnergyBinTitle(ep.emin, ep.emax) == "#bf{E_{#gamma} (GeV): (" + et[0] + ", " + et[1] + ")}");
        ++names;
        for (const auto& t : tBins) {
            const std::string bn = gxana::BinName(lo, hi, t.first, t.second);
            CHECK(bn == frozen::BinName(lo, hi, t.first, t.second));
            const gxana::BinNameParts p = gxana::ParseBinName(bn);
            const auto d = frozen::ParseDiff(bn);
            CHECK(p.hasT && p.emin == d[0] && p.emax == d[1] && p.tmin == d[2] && p.tmax == d[3]);
            ++names;
        }
    }
    CHECK(names == 8 + 8 * 7);

    // Truncation, not rounding (pinned by test_xsection.cxx too), and a dense sweep.
    CHECK(gxana::BinEdgeLabel(0.375) == "0.37");
    CHECK(gxana::BinEdgeLabel(6.405) == "6.40");
    CHECK(gxana::BinEdgeLabel(10.18) == "10.18");
    for (int i = 0; i <= 12000; ++i) CHECK(gxana::BinEdgeLabel(i * 0.001) == frozen::BinEdgeLabel(i * 0.001));

    // Names with a prefix or suffix, as table stems carry them.
    const gxana::BinNameParts stem = gxana::ParseBinName("diffxsec_flatTree_x_emin_6.40_emax_7.40");
    CHECK(stem.emin == "6.40" && stem.emax == "7.40" && !stem.hasT);
    const gxana::BinNameParts file = gxana::ParseBinName("weighted_diffxsec_emin_10.18_emax_11.40.txt");
    CHECK(file.emin == "10.18" && file.emax == "11.40.txt");

    // A name without the keys is rejected (the substr code read from position 4 instead).
    auto throws = [](const std::string& text) {
        try {
            gxana::ParseBinName(text);
        } catch (const std::invalid_argument&) {
            return true;
        }
        return false;
    };
    CHECK(throws("flatTree_kpkpxim"));
    CHECK(throws("emin_6.40"));
    CHECK(throws("emin_6.40_emax_7.40_tmin_0.10"));
    CHECK(!throws("emin_6.40_emax_7.40"));

    if (failures == 0) std::cout << "test_binnames: all checks passed (" << names << " configured names)\n";
    return failures == 0 ? 0 : 1;
}
