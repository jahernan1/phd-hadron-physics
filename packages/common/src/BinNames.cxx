#include "gxana/common/BinNames.h"

#include <stdexcept>

namespace gxana {

namespace {

// The label after the five-character key at `at`, up to the next '_' or the end of text.
std::string Label(const std::string& text, std::string::size_type at)
{
    const std::string rest = text.substr(at + 5);
    return rest.substr(0, rest.find('_'));
}

} // namespace

std::string BinEdgeLabel(double edge)
{
    std::string label = std::to_string(edge);
    return label.substr(0, label.find(".") + 3);
}

std::string EnergyBinName(double lowE, double highE)
{
    return EnergyBinName(BinEdgeLabel(lowE), BinEdgeLabel(highE));
}

std::string EnergyBinName(const std::string& lowE, const std::string& highE)
{
    return "emin_" + lowE + "_emax_" + highE;
}

std::string BinName(double lowE, double highE, double lowT, double highT)
{
    return EnergyBinName(lowE, highE) + "_tmin_" + BinEdgeLabel(lowT) + "_tmax_" + BinEdgeLabel(highT);
}

std::string EnergyBinTitle(const std::string& lowE, const std::string& highE)
{
    return "#bf{E_{#gamma} (GeV): (" + lowE + ", " + highE + ")}";
}

BinNameParts ParseBinName(const std::string& text)
{
    const auto emin = text.find("emin_");
    const auto emax = text.find("emax_");
    if (emin == std::string::npos || emax == std::string::npos)
        throw std::invalid_argument("not a bin name (needs emin_ and emax_): '" + text + "'");
    BinNameParts parts;
    parts.emin = Label(text, emin);
    parts.emax = Label(text, emax);
    const auto tmin = text.find("tmin_");
    if (tmin != std::string::npos) {
        const auto tmax = text.find("tmax_");
        if (tmax == std::string::npos)
            throw std::invalid_argument("bin name has tmin_ but no tmax_: '" + text + "'");
        parts.tmin = Label(text, tmin);
        parts.tmax = text.substr(tmax + 5);
        parts.hasT = true;
    }
    return parts;
}

} // namespace gxana
