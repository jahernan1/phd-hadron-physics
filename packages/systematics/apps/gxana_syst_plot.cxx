// Draw one systematics comparison figure (port of the PlotComboComparison.C /
// PlotFitComparison.C drawing; see gxana/systematics/PlotSpread.h).
#include "CliArgs.h"
#include "gxana/systematics/PlotSpread.h"

#include <TROOT.h>

#include <iostream>
#include <stdexcept>
#include <string>

namespace {
const char* kUsage =
    "usage: gxana_syst_plot --layout L --name N --out-dir D --input DIR [--input ...] [--band FILE]\n"
    "                       [--legend \"text|opt\" ...] [--legend-header T] [--first-style points|band]\n"
    "                       [--annotate avg:N|max:N ...] [--axis-format N] [--xmax X] [--ymax Y]\n"
    "  Layouts: grid3 (3 inputs), pair_band (2 inputs + band), all_band (2-8 inputs + band).\n"
    "  Reads DIR/weighted_diffxsec_emin_*_emax_*.txt (one panel per energy bin) and, for *_band\n"
    "  layouts, the spread stats FILE (XVal XErr YMean StdDev); writes D/N.pdf.\n";

gxana::systematics::LegendEntry ParseLegend(const std::string& value)
{
    const auto bar = value.rfind('|');
    if (bar == std::string::npos)
        throw std::invalid_argument("--legend needs \"text|opt\": '" + value + "'");
    return {value.substr(0, bar), value.substr(bar + 1)};
}

gxana::systematics::Annotation ParseAnnotation(const std::string& value)
{
    const auto parts = gxana::cli::Split(value, ':');
    if (parts.size() != 2 || (parts[0] != "avg" && parts[0] != "max"))
        throw std::invalid_argument("--annotate needs avg:N or max:N: '" + value + "'");
    const double n = gxana::cli::ParseDouble(parts[1]);
    if (n < 1 || n != static_cast<int>(n))
        throw std::invalid_argument("--annotate needs avg:N or max:N with N >= 1: '" + value + "'");
    return {parts[0], static_cast<int>(n)};
}
} // namespace

int main(int argc, char** argv)
{
    gROOT->SetBatch(true);
    gxana::systematics::PlotSpec spec;
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
            if (arg == "--layout") spec.layout = value;
            else if (arg == "--name") spec.name = value;
            else if (arg == "--out-dir") spec.outDir = value;
            else if (arg == "--input") spec.inputs.push_back(value);
            else if (arg == "--band") spec.band = value;
            else if (arg == "--legend") spec.legend.push_back(ParseLegend(value));
            else if (arg == "--legend-header") spec.legendHeader = value;
            else if (arg == "--first-style") {
                if (value != "points" && value != "band")
                    throw std::invalid_argument("--first-style must be points or band: '" + value + "'");
                spec.firstStyle = value;
            } else if (arg == "--annotate") spec.annotate.push_back(ParseAnnotation(value));
            else if (arg == "--axis-format") {
                const double n = gxana::cli::ParseDouble(value);
                if (n != static_cast<int>(n))
                    throw std::invalid_argument("--axis-format needs an integer: '" + value + "'");
                spec.axisFormat = static_cast<int>(n);
            } else if (arg == "--xmax") spec.xmax = gxana::cli::ParseDouble(value);
            else if (arg == "--ymax") spec.ymax = gxana::cli::ParseDouble(value);
            else
                throw std::invalid_argument("unknown option " + arg);
        }
        if (spec.layout.empty() || spec.name.empty() || spec.outDir.empty())
            throw std::invalid_argument("missing arguments: --layout, --name and --out-dir are required");
        if (spec.inputs.size() < 2)
            throw std::invalid_argument("--input needs at least 2 values, got " + std::to_string(spec.inputs.size()));
        const bool band = spec.layout == "pair_band" || spec.layout == "all_band" || spec.layout == "stddev_band";
        if (band && spec.band.empty())
            throw std::invalid_argument("layout " + spec.layout + " needs --band");
    } catch (const std::invalid_argument& err) {
        std::cerr << "gxana_syst_plot: error: " << err.what() << "\n" << kUsage;
        return 2;
    }
    try {
        gxana::systematics::Plot(spec);
    } catch (const std::exception& err) {
        std::cerr << "gxana_syst_plot: error: " << err.what() << "\n";
        return 1;
    }
    return 0;
}
