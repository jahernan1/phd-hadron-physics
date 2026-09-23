#ifndef GXANA_COMMON_GRAPHIO_H
#define GXANA_COMMON_GRAPHIO_H

#include <string>
#include <vector>

class TGraphErrors;

namespace gxana {

// All TGraphErrors at the top level of fileName; empty if the file cannot be
// opened. Verbatim from AnalysisNote/xsection/Plot*Comparison.C (8 copies).
std::vector<TGraphErrors*> GetAllTGraphErrors(const char* fileName);

// One TGraphErrors per "*.txt" file in dir whose name matches the fnmatch
// pattern (columns x y ex ey; lines that do not parse, e.g. headers, are
// skipped), ordered by NumericCompare -- so every matched name must contain
// "emin" or std::out_of_range is thrown (legacy). Graphs are named
// "Graph_<file stem>" and titled "#bf{E_{#gamma} (GeV): (<emin>, <emax>)}".
// If outFile is non-empty the graphs are also written there (RECREATE).
// From AnalysisNote/xsection/PlotFunctions.cpp, which wrote into the cwd
// under a name derived from dir; callers now choose the path.
std::vector<TGraphErrors*> CreateTGraphErrorsFromTxt(const std::string& dir, const std::string& pattern,
                                                     const std::string& outFile = "");

} // namespace gxana

#endif
