// Write the Barlow variation trees (port of GetVariationTreesUML.C, snapshot part).
#include "gxana/common/Cli.h"
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
    "       gxana_barlow_trees --check --tree NAME --out FILE --variation TREE=CUT [...]\n"
    "                          --nominal DATA.root --nominal-mc MC.root --name NAME --weight BRANCH\n"
    "                          --yields FILE --fit-dir DIR\n"
    "                          [--observable BRANCH] [--observable-title TITLE] [--mass-window NAME=GEV ...]\n"
    "  --check  fit the nominal and TREE/TREE_mc yields in FILE (legacy side check) and append\n"
    "           NAME, id, nom, var, pct, nomMC, varMC, pctMC to --yields\n"
    "  --observable  fitted mass branch; --observable-title its axis title\n"
    "  --mass-window  lo, mc_hi, mc_signal_hi, mc_plot_hi, data_lo, data_hi, scan_start\n"
    "                (default: the kpkpxim values until the channel config passes them)\n"
    "  --threads N  N > 0 enables ROOT implicit multithreading (default 0: off)\n";

// --mass-window NAME=VALUE (GeV) of the check fits; returns NAME.
std::string SetCheckWindow(gxana::barlow::CheckWindows& windows, const std::string& text)
{
    const auto assign = gxana::cli::SplitAssign(text);
    const double value = gxana::cli::ParseDouble(assign.second);
    const std::string& name = assign.first;
    if (name == "lo") windows.lo = value;
    else if (name == "mc_hi") windows.mcHi = value;
    else if (name == "mc_signal_hi") windows.mcSignalHi = value;
    else if (name == "mc_plot_hi") windows.mcPlotHi = value;
    else if (name == "data_lo") windows.dataLo = value;
    else if (name == "data_hi") windows.dataHi = value;
    else if (name == "scan_start") windows.scanStart = value;
    else
        throw std::invalid_argument("unknown --mass-window " + name + " (lo, mc_hi, mc_signal_hi, mc_plot_hi, "
                                    "data_lo, data_hi, scan_start)");
    return name;
}

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
    gxana::barlow::CheckSpec check;
    bool checkMode = false;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "-h" || arg == "--help") {
                std::cout << kUsage;
                return 0;
            }
            if (arg == "--check") {
                checkMode = true;
                continue;
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
            else if (arg == "--nominal") check.nominal = value;
            else if (arg == "--nominal-mc") check.nominalMC = value;
            else if (arg == "--name") check.name = value;
            else if (arg == "--weight") check.weight = value;
            else if (arg == "--yields") check.yields = value;
            else if (arg == "--fit-dir") check.fitDir = value;
            else if (arg == "--observable") check.obs.branch = value;
            else if (arg == "--observable-title") check.obs.title = value;
            else if (arg == "--mass-window") SetCheckWindow(check.windows, value);
            else if (arg == "--variation") {
                const auto v = gxana::barlow::SplitAssign(value);
                spec.variations.push_back({v.first, v.second});
            } else
                throw std::invalid_argument("unknown option " + arg);
        }
        if (checkMode) {
            check.tree = spec.tree;
            check.out = spec.out;
            check.variations = spec.variations;
            if (check.tree.empty() || check.out.empty() || check.nominal.empty() || check.nominalMC.empty()
                || check.name.empty() || check.weight.empty() || check.yields.empty() || check.fitDir.empty()
                || check.variations.empty())
                throw std::invalid_argument("missing arguments");
        } else if (spec.tree.empty() || spec.input.empty() || spec.inputMC.empty() || spec.out.empty()
            || spec.branches.empty() || spec.variations.empty())
            throw std::invalid_argument("missing arguments");
    } catch (const std::invalid_argument& err) {
        std::cerr << "gxana_barlow_trees: " << err.what() << "\n" << kUsage;
        return 2;
    }
    try {
        if (checkMode)
            gxana::barlow::CheckVariationYields(check);
        else
            gxana::barlow::WriteVariationTrees(spec);
    } catch (const std::exception& err) {
        std::cerr << "gxana_barlow_trees: " << err.what() << "\n";
        return 1;
    }
    return 0;
}
