#ifndef GXANA_COMMON_BINNAMES_H
#define GXANA_COMMON_BINNAMES_H

#include <string>

namespace gxana {

// Bin-edge text used in tree, table and plot names and in the RDataFrame filters:
// std::to_string cut two digits after the point (6.4 -> "6.40", 11.4 -> "11.40").
// Truncates, never rounds (6.405 -> "6.40", 0.375 -> "0.37").
std::string BinEdgeLabel(double edge);
// "emin_6.40_emax_7.40"
std::string EnergyBinName(double lowE, double highE);
// "emin_<lowE>_emax_<highE>" with the labels as given (e.g. the strings of a config file).
std::string EnergyBinName(const std::string& lowE, const std::string& highE);
// "emin_6.40_emax_7.40_tmin_0.10_tmax_0.35"
std::string BinName(double lowE, double highE, double lowT, double highT);
// "#bf{E_{#gamma} (GeV): (<lowE>, <highE>)}", the panel title of the per-energy plots.
std::string EnergyBinTitle(const std::string& lowE, const std::string& highE);

struct BinNameParts {
    std::string emin, emax, tmin, tmax;
    bool hasT = false;
};
// The labels of a name that contains "emin_<a>_emax_<b>" and optionally "_tmin_<c>_tmax_<d>"
// anywhere (e.g. a tree name or a table file stem), as written (no reformatting). emin, emax
// and tmin end at the next '_' (or the end of the text); tmax runs to the end of the text.
// Throws std::invalid_argument when emin_ or emax_ is missing, or tmin_ without tmax_.
BinNameParts ParseBinName(const std::string& text);

} // namespace gxana

#endif
