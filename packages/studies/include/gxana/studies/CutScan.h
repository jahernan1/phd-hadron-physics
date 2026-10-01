#ifndef GXANA_STUDIES_CUTSCAN_H
#define GXANA_STUDIES_CUTSCAN_H

// Cut scan with a figure of merit: the fit and plot halves of archive/root_macros/CutAnalysisRF.C
// (GetCutAnalysis, rooFitHist, plotRatio), statement for statement. The mass-vs-cut TH2 is
// filled by gxana::FillHists (packages/common, PeriodHists.h). Model: Chebychev(a0,a1) "bkgd"
// + Johnson(mu,lambda,gamma,delta) "xigaus", extended SUM "model" with yields nbkgd, nxi; the
// parameter names are those of the legacy macro (they appear in the fit panels' parameter box).

#include <TH2.h>

#include <map>
#include <string>
#include <vector>

namespace gxana {
namespace studies {

struct CutScanFit {
    std::string massTitle;                     // observable title (fit panel x axis)
    double lo = 0, hi = 0;                     // observable range
    std::map<std::string, std::string> params; // a0 a1 mu lambda gamma delta nbkgd nxi -> factory bracket text
    int firstBin = 1;                          // first y bin fitted (cumulative projection over y bins 1..bin)
    std::string panelLabel;                    // panel title: "<panelLabel> < <cut>"
    std::string tables;                        // path with "{what}" -> FOM, SB, Yield
    std::string gridPdf;                       // fit grid
};

// The names FitCutScan requires in CutScanFit::params, in factory order.
const std::vector<std::string>& CutScanParams();

// Fits every cumulative projection, writes the three tables ("%f \t %f \n") and the fit grid.
void FitCutScan(const TH2& hist, const CutScanFit& fit);

// plotRatio: FOM table (TGraphErrors "%lg %lg") with the S/B table on a right-hand axis, a cut
// line and arrow at `cut`; saved to every path in `pdfs`. `name` names the canvas and pads.
void PlotCutScan(const std::string& tables, const std::string& title, double cut, const std::string& name,
                 const std::vector<std::string>& pdfs);

// setStyle() of CutAnalysisRF.C.
void ApplyCutScanStyle();
// The five gStyle writes GetCutAnalysis makes before plotRatio.
void ApplyCutScanPlotMargins();

} // namespace studies
} // namespace gxana

#endif
