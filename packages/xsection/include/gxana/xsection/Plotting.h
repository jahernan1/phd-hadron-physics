#ifndef GXANA_XSECTION_PLOTTING_H
#define GXANA_XSECTION_PLOTTING_H

#include <string>
#include <vector>

class TGraphErrors;

namespace gxana {
namespace xsec {

// Directory the plot* functions below save into (created if missing); "" (default)
// saves under the current directory, matching SetFitPlotDir/GetFitPlotDir in YieldFit.h.
// Each plot* function skips (prints, saves nothing) when its graph vectors are empty.
void SetPlotDir(const std::string& dir);
std::string PlotDir();

// Legacy AnalysisNote/xsection/PlotFunctions.cpp:195-322. arrGraphs[run
// period][energy bin]; saves PlotDir() + "/" + saveName + ".pdf".
void plotDiffXSec(std::vector<std::vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, std::string saveName);

// Legacy PlotFunctions.cpp:324-429.
void plotWeightedXSec(std::vector<TGraphErrors*> arrGraphs, double xmax, double ymax, std::string saveName);

// Legacy PlotFunctions.cpp:431-494.
void plotOneWeightedXSec(std::vector<TGraphErrors*> arrGraphs, double xmax, double ymax, std::string saveName);

// Legacy PlotFunctions.cpp:496-611.
void plotFinalWeightedXSec(std::vector<std::vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, std::string saveName);

} // namespace xsec
} // namespace gxana

#endif
