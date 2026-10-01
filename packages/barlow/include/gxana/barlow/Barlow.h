#ifndef GXANA_BARLOW_BARLOW_H
#define GXANA_BARLOW_BARLOW_H

#include <TGraphErrors.h>

#include <string>
#include <utility>
#include <vector>

namespace gxana {
namespace barlow {

// Barlow significance per point: (y_nominal - y_variation) / sqrt(|sigma_n^2 - sigma_v^2|),
// 0 where that sigma is 0; x errors from variation, y errors 0. Signed, as in
// systematics/PlotXSecBarlow*.C (GetBarlowResults.C used the absolute value).
TGraphErrors* calc_barlow(TGraphErrors* nominal, TGraphErrors* variation);

// Population standard deviation of y across graphs at each point (x from the
// first graph, zero errors, red markers); nullptr if graphs is empty.
TGraphErrors* calculateStdDevGraph(const std::vector<TGraphErrors*>& graphs);

// Per-family drawing differences of the PlotXSecBarlow*.C macros, as data.
struct BarlowPlotStyle {
    int canvasW = 0, canvasH = 0;          // TCanvas size; 0 = TCanvas default (gStyle canvasDefW x 800)
    std::vector<double> legendDiff, legendTot; // TLegend x1,y1,x2,y2 of the dsigma/dt and total plots
    double yFloor = 6;                      // minimum sigma_B half-range (ysym floor)
    double yPadDiff = 1;                    // dsigma/dt sigma_B axis is +-(ysym + yPadDiff)
    int canvasDefW = 700;                   // gStyle->SetCanvasDefW
    double titleOffsetY = 0.85;             // gStyle->SetTitleOffset(v, "Y")
    std::vector<double> titleOffsetsDiff{0.9, 0.35}, titleOffsetsTot{1.1, 0.35}; // sigma_B pad x, y
    bool totYNdiv = true;                   // total plot sigma_B axis SetNdivisions(505)
};

struct BarlowPlotSpec {
    std::string nominalDir, varDir, family, label, outDir;
    std::vector<std::pair<std::string, std::string>> variations; // id, legend value
    std::vector<std::pair<std::string, std::string>> energies;   // emin, emax as in file names
    double threshold = 4;                                         // shaded band +-threshold
    BarlowPlotStyle style;
    // Channel values (physics.reaction_title, barlow.plot); S6 transition defaults = kpkpxim.
    std::string reactionTitle = "#gamma p#rightarrow K^{+}K^{+}#Xi^{-}"; // total plot y title #sigma(<this>) (nb)
    std::vector<double> tLimits{0, 2.5};       // dsigma/dt plot x range (-t, GeV^2)
    std::vector<double> energyLimits{6.2, 11.6}; // total plot x range (E_gamma, GeV)
    std::vector<double> graphLimits{6, 12};    // SetLimits of the input graphs (and total sigma_B graphs)
};

// The macros' SetStyle() with the per-family canvas width and Y title offset.
void SetBarlowStyle(const BarlowPlotStyle& style);

// plotTotXSecAndBarlow + plotDiffXSecAndBarlow of PlotXSecBarlow*.C for one family:
// barlow_weighted_totxsec_vary_<family>.pdf and one
// barlow_weighted_diffxsec_vary_<family>_emin_X_emax_Y.pdf per energy bin, each with
// a .txt of "id x y_nom ey_nom y_var ey_var sigma_B". Throws std::runtime_error on
// missing inputs or a variation whose points differ from the nominal's.
void PlotBarlow(const BarlowPlotSpec& spec);

} // namespace barlow
} // namespace gxana

#endif
