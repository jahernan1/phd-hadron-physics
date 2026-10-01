#ifndef GXANA_SYSTEMATICS_PLOTSPREAD_H
#define GXANA_SYSTEMATICS_PLOTSPREAD_H

#include <TGraphErrors.h>

#include <string>
#include <vector>

namespace gxana {
namespace systematics {

// One legend line: "text|opt" (TLegend::AddEntry option, e.g. lep or f).
struct LegendEntry {
    std::string text;
    std::string option;
};

// One TLatex line per pad: "avg:N" or "max:N", the average or maximum percent
// difference of input N against input 0.
struct Annotation {
    std::string what;
    int graph = 1;
};

// The per-figure differences of the legacy comparison macros, as data.
struct PlotSpec {
    std::string layout;                // grid3 | pair_band | all_band | grid2 | run_grid | stddev_band
    std::string name;                  // output <outDir>/<name>.pdf
    std::string outDir;
    std::vector<std::string> inputs;   // label directories (weighted_diffxsec_emin_*.txt) or per-period files
    std::string band;                  // stats file for *_band layouts
    std::vector<LegendEntry> legend;
    std::string legendHeader;
    std::string firstStyle = "points"; // points | band
    std::vector<Annotation> annotate;
    int axisFormat = 305;              // TGaxis ndiv of the y axes
    double xmax = 2.5, ymax = 9;
};

// Legacy style_format() of the comparison macros.
void StyleFormat();

// One graph per energy bin of a label directory (weighted_diffxsec_emin_E1_emax_E2.txt),
// in ascending emin, titled as MakeWeightedDiffXSecTGraphs.C titles them.
std::vector<TGraphErrors*> ReadLabelGraphs(const std::string& dir);

// The spread band of a stats file (XVal XErr YMean StdDev): one graph per block of
// shape[i]->GetN() rows, y = YMean, ey = StdDev.
std::vector<TGraphErrors*> ReadBandGraphs(const std::string& stats, const std::vector<TGraphErrors*>& shape);

struct GraphStats {
    double mean_diff;    // Average of all y-values
    double std_dev_diff; // Standard deviation
    double paired_ttest;
    double max_pct_diff;
    double avg_pct_diff;
    double signif;
};
// Legacy GetAvgStdDev (paired t-test index bug fixed).
GraphStats GetAvgStdDev(const TGraphErrors& graph1, const TGraphErrors& graph2);

// Draw spec.layout to <outDir>/<name>.pdf.
void Plot(const PlotSpec& spec);

} // namespace systematics
} // namespace gxana

#endif
