// Split flat trees into the cross-section (E_gamma, -t) bins (gxana::xsec Binning).
#include "CliArgs.h"
#include "gxana/xsection/Binning.h"

#include <TROOT.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
const char* kUsage =
    "usage: gxana_xsec_bin MODE INPUT OUTPUT --energy E0,E1,... --t T0,T1,... [--tree NAME]\n"
    "  MODE  data (nominal tree + qvalue_decayxim_M), mc (nominal tree), thrown,\n"
    "        variation (every tree in INPUT, all branches)\n"
    "  --energy/--t take contiguous bin edges, e.g. --energy 6.4,11.4 for one bin\n";
}

int main(int argc, char** argv)
{
    gROOT->SetBatch(true);
    std::vector<std::string> positional;
    std::string energy, tEdges, tree;
    gxana::xsec::BinRanges en, t;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "-h" || arg == "--help") {
                std::cout << kUsage;
                return 0;
            }
            if (arg == "--energy" || arg == "--t" || arg == "--tree") {
                if (i + 1 >= argc)
                    throw std::invalid_argument(arg + " needs a value");
                const std::string value = argv[++i];
                if (arg == "--energy")
                    energy = value;
                else if (arg == "--t")
                    tEdges = value;
                else
                    tree = value;
            } else {
                positional.push_back(arg);
            }
        }
        if (positional.size() != 3 || energy.empty() || tEdges.empty())
            throw std::invalid_argument("missing arguments");
        en = gxana::xsec::EdgesToBins(gxana::cli::ParseDoubleList(energy));
        t = gxana::xsec::EdgesToBins(gxana::cli::ParseDoubleList(tEdges));
    } catch (const std::invalid_argument& err) {
        std::cerr << "gxana_xsec_bin: " << err.what() << "\n" << kUsage;
        return 2;
    }

    const std::string& mode = positional[0];
    const std::string& in = positional[1];
    const std::string& out = positional[2];
    try {
        bool ok = false;
        if (mode == "data" || mode == "mc")
            ok = tree.empty() ? gxana::xsec::divideNominalIntoBins(in, out, en, t, mode == "data")
                              : gxana::xsec::divideNominalIntoBins(in, out, en, t, mode == "data", tree);
        else if (mode == "thrown")
            ok = tree.empty() ? gxana::xsec::divideThrownIntoBins(in, out, en, t)
                              : gxana::xsec::divideThrownIntoBins(in, out, en, t, tree);
        else if (mode == "variation")
            ok = gxana::xsec::divideVariationTreesIntoBins(in, out, en, t);
        else {
            std::cerr << "gxana_xsec_bin: unknown MODE " << mode << "\n" << kUsage;
            return 2;
        }
        return ok ? 0 : 1;
    } catch (const std::exception& err) {
        std::cerr << "gxana_xsec_bin: " << err.what() << "\n";
        return 1;
    }
}
