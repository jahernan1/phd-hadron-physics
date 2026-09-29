// Draw the Barlow plots of one variation family (port of PlotXSecBarlow*.C).
#include "CliArgs.h"
#include "gxana/barlow/Barlow.h"
#include "gxana/barlow/VariationTrees.h"

#include <TROOT.h>

#include <iostream>
#include <stdexcept>
#include <string>

namespace {
const char* kUsage =
    "usage: gxana_barlow_plot --nominal-dir DIR --var-dir DIR --out-dir DIR --family NAME --label TEXT\n"
    "                         --variation ID=VALUE [...] --energy EMIN:EMAX [...]\n"
    "                         --canvas W,H|default --legend-diff X1,Y1,X2,Y2 --legend-tot X1,Y1,X2,Y2\n"
    "                         --y-floor N --y-pad-diff N --canvas-def-w N --title-offset-y N\n"
    "                         --title-offsets-diff X,Y --title-offsets-tot X,Y --tot-y-ndiv 0|1\n"
    "                         [--threshold N]\n"
    "  Reads DIR/totxsec_weighted_output.txt and weighted_diffxsec_emin_EMIN_emax_EMAX.txt (nominal),\n"
    "  weighted_totxsec_vary_ID.txt and weighted_diffxsec_vary_ID_emin_EMIN_emax_EMAX.txt (variations);\n"
    "  writes barlow_weighted_{totxsec,diffxsec}_vary_NAME*.pdf and .txt (sigma_B per point).\n";

std::vector<double> Numbers(const std::string& option, const std::string& text, size_t count)
{
    const auto values = gxana::cli::ParseDoubleList(text);
    if (values.size() != count)
        throw std::invalid_argument(option + " needs " + std::to_string(count) + " comma-separated numbers: '"
                                    + text + "'");
    return values;
}
} // namespace

int main(int argc, char** argv)
{
    gROOT->SetBatch(true);
    gxana::barlow::BarlowPlotSpec spec;
    auto& st = spec.style;
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
            if (arg == "--nominal-dir") spec.nominalDir = value;
            else if (arg == "--var-dir") spec.varDir = value;
            else if (arg == "--out-dir") spec.outDir = value;
            else if (arg == "--family") spec.family = value;
            else if (arg == "--label") spec.label = value;
            else if (arg == "--variation") spec.variations.push_back(gxana::barlow::SplitAssign(value));
            else if (arg == "--energy") {
                const auto parts = gxana::cli::Split(value, ':');
                if (parts.size() != 2 || parts[0].empty() || parts[1].empty())
                    throw std::invalid_argument("--energy needs EMIN:EMAX: '" + value + "'");
                spec.energies.emplace_back(parts[0], parts[1]);
            } else if (arg == "--canvas") {
                if (value != "default") {
                    const auto wh = Numbers(arg, value, 2);
                    st.canvasW = static_cast<int>(wh[0]);
                    st.canvasH = static_cast<int>(wh[1]);
                }
            } else if (arg == "--legend-diff") st.legendDiff = Numbers(arg, value, 4);
            else if (arg == "--legend-tot") st.legendTot = Numbers(arg, value, 4);
            else if (arg == "--y-floor") st.yFloor = gxana::cli::ParseDouble(value);
            else if (arg == "--y-pad-diff") st.yPadDiff = gxana::cli::ParseDouble(value);
            else if (arg == "--canvas-def-w") st.canvasDefW = static_cast<int>(gxana::cli::ParseDouble(value));
            else if (arg == "--title-offset-y") st.titleOffsetY = gxana::cli::ParseDouble(value);
            else if (arg == "--title-offsets-diff") st.titleOffsetsDiff = Numbers(arg, value, 2);
            else if (arg == "--title-offsets-tot") st.titleOffsetsTot = Numbers(arg, value, 2);
            else if (arg == "--tot-y-ndiv") {
                if (value != "0" && value != "1")
                    throw std::invalid_argument("--tot-y-ndiv must be 0 or 1: '" + value + "'");
                st.totYNdiv = value == "1";
            } else if (arg == "--threshold") spec.threshold = gxana::cli::ParseDouble(value);
            else
                throw std::invalid_argument("unknown option " + arg);
        }
        if (spec.nominalDir.empty() || spec.varDir.empty() || spec.outDir.empty() || spec.family.empty()
            || spec.label.empty() || spec.variations.empty() || st.legendDiff.empty() || st.legendTot.empty())
            throw std::invalid_argument("missing arguments");
    } catch (const std::invalid_argument& err) {
        std::cerr << "gxana_barlow_plot: " << err.what() << "\n" << kUsage;
        return 2;
    }
    try {
        gxana::barlow::PlotBarlow(spec);
    } catch (const std::exception& err) {
        std::cerr << "gxana_barlow_plot: " << err.what() << "\n";
        return 1;
    }
    return 0;
}
