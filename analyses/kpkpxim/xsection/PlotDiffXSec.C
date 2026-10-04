#include "gxana/common/Style.h"
#include "gxana/common/GraphIO.h"
#include "gxana/xsection/Plotting.h"
#include "gxana/common/Paths.h"

// Differential cross-section figures of one fit label (dissertation chapter 6):
//   diffxsec_runs_<label>.pdf                the three run periods, <xsecDir>/data/<label>/diffxsec*<period>*
//   diffxsec_phase1_weighted_<label>.pdf     the weighted average, <xsecDir>/weighted_data/<label>/weighted_diffxsec*
//   diffxsec_phase1_systematics_<label>.pdf  the weighted average (statistical bars) with the systematic band of
//                                            syst_weighted_diffxsec* (gxana_xsection.syst_tables)
// `gxana run xsection --steps figures` runs it (xsection.figures). The drawn graphs, one per energy bin in
// panel order (ascending emin), are also written to <plotDir>/{Weighted,SystWeighted}DiffXSecTGraphs_<label>.root
// and DiffXSecTGraphs_<period>_<label>.root. Returns 1 if the weighted or systematic tables are missing.
int PlotDiffXSec(std::string xsecDir = "", std::string label = "johnson", std::string plotDir = "")
{
    if (xsecDir.empty()) xsecDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection");
    if (plotDir.empty()) plotDir = xsecDir + "/figures";
    const std::string wdir = xsecDir + "/weighted_data/" + label;
    const std::string ddir = xsecDir + "/data/" + label;
    gSystem->mkdir(plotDir.c_str(), true);
    gxana::SetStyle();
    gxana::xsec::SetPlotDir(plotDir);

    std::vector<TGraphErrors*> weighted = gxana::CreateTGraphErrorsFromTxt(
        wdir, "weighted_diffxsec*", plotDir + "/WeightedDiffXSecTGraphs_" + label + ".root");
    std::vector<TGraphErrors*> syst = gxana::CreateTGraphErrorsFromTxt(
        wdir, "syst_weighted_diffxsec*", plotDir + "/SystWeightedDiffXSecTGraphs_" + label + ".root");
    if (weighted.empty() || syst.size() != weighted.size()) {
        std::cerr << "PlotDiffXSec: need one syst_weighted_diffxsec* table per weighted_diffxsec* table in "
                  << wdir << " (found " << weighted.size() << " weighted and " << syst.size()
                  << " systematic; gxana run xsection --steps figures writes them)" << std::endl;
        return 1;
    }
    std::vector<std::vector<TGraphErrors*>> runs;
    for (const std::string period : {"2017-01", "2018-01", "2018-08"})
        runs.push_back(gxana::CreateTGraphErrorsFromTxt(
            ddir, "diffxsec*" + period + "*", plotDir + "/DiffXSecTGraphs_" + period + "_" + label + ".root"));

    if (weighted.size() > 1) {
        gxana::xsec::plotWeightedXSec(weighted, 2.5, 20, "diffxsec_phase1_weighted_" + label);
        gxana::xsec::plotDiffXSec(runs, 2.5, 20, "diffxsec_runs_" + label);
    } else {
        gxana::xsec::plotOneWeightedXSec(weighted, 2.5, 20, "diffxsec_phase1_weighted_" + label);
    }
    gxana::xsec::plotFinalWeightedXSec({weighted, syst}, 2.5, 20, "diffxsec_phase1_systematics_" + label);
    return 0;
}
