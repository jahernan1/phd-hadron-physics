// Write the Barlow variation trees (port of GetVariationTreesUML.C, snapshot part).
#include "CliArgs.h"
#include "gxana/barlow/VariationTrees.h"

#include <TROOT.h>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
const char* kUsage =
    "usage: gxana_barlow_trees --tree NAME --input DATA.root --input-mc MC.root --out FILE\n"
    "                          --branch B [--branch ...] --variation TREE=CUT [--variation ...]\n"
    "                          [--define NAME=EXPR ...] [--filter-data EXPR ...] [--filter-mc EXPR ...]\n"
    "                          [--threads N]\n"
    "  Defines, then filters, then each variation's cut on the data and MC trees NAME;\n"
    "  writes TREE (data) and TREE_mc (MC) with the --branch columns to FILE (recreated).\n"
    "  --threads N  N > 0 enables ROOT implicit multithreading (default 0: off)\n";

int ParseThreads(const std::string& text)
{
    const double value = gxana::cli::ParseDouble(text);
    if (value < 0 || value != std::floor(value))
        throw std::invalid_argument("--threads must be a whole number >= 0: '" + text + "'");
    return static_cast<int>(value);
}
} // namespace

int main(int argc, char** argv)
{
    gROOT->SetBatch(true);
    gxana::barlow::VariationTreesSpec spec;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "-h" || arg == "--help") {
                std::cout << kUsage;
                return 0;
            }
            if (i + 1 >= argc)
                throw std::invalid_argument(arg + " needs a value");
            const std::string value = argv[++i];
            if (arg == "--tree") spec.tree = value;
            else if (arg == "--input") spec.input = value;
            else if (arg == "--input-mc") spec.inputMC = value;
            else if (arg == "--out") spec.out = value;
            else if (arg == "--branch") spec.branches.push_back(value);
            else if (arg == "--define") spec.defines.push_back(gxana::barlow::SplitAssign(value));
            else if (arg == "--filter-data") spec.filtersData.push_back(value);
            else if (arg == "--filter-mc") spec.filtersMC.push_back(value);
            else if (arg == "--threads") spec.threads = ParseThreads(value);
            else if (arg == "--variation") {
                const auto v = gxana::barlow::SplitAssign(value);
                spec.variations.push_back({v.first, v.second});
            } else
                throw std::invalid_argument("unknown option " + arg);
        }
        if (spec.tree.empty() || spec.input.empty() || spec.inputMC.empty() || spec.out.empty()
            || spec.branches.empty() || spec.variations.empty())
            throw std::invalid_argument("missing arguments");
    } catch (const std::invalid_argument& err) {
        std::cerr << "gxana_barlow_trees: " << err.what() << "\n" << kUsage;
        return 2;
    }
    try {
        gxana::barlow::WriteVariationTrees(spec);
    } catch (const std::exception& err) {
        std::cerr << "gxana_barlow_trees: " << err.what() << "\n";
        return 1;
    }
    return 0;
}
