#ifndef GXANA_XSECTION_XSEC_H
#define GXANA_XSECTION_XSEC_H

#include "gxana/xsection/YieldFit.h"

#include <TH1D.h>
#include <TTree.h>

#include <fstream>
#include <string>
#include <vector>

namespace gxana {
namespace xsec {

// One (E_gamma, -t) bin (delim[2] = its tree name, e.g.
// "emin_6.40_emax_7.40_tmin_0.10_tmax_0.35"): fit data (trees[0]) and MC
// (trees[1]), count thrown (trees[2]), integrate flux over the energy bin, and
// append a row to outputFile (t center, half width, yields, acceptance, flux)
// and to xsecFile (t center, dsigma/dt [nb/GeV^2], half width, error). Bins
// with <= 10 data entries in 1.30-1.35 GeV get cross section 0.
// Verbatim from AnalysisNote/xsection/FitFunctions.cpp; n_threads is unused.
void GetDiffXSecFile(std::vector<TTree*> trees, TH1D* flux, std::vector<std::string> delim,
                     std::string fitType, FitParams& xiParamRange,
                     std::ofstream& outputFile, std::ofstream& xsecFile,
                     std::string weight = "hybrid_combo", int chebyOrder = 2, int n_threads = 4);
// Same for a whole energy bin ("emin_6.40_emax_7.40"): total cross section [nb].
void GetTotXSecFile(std::vector<TTree*> trees, TH1D* flux, std::vector<std::string> delim,
                    std::string fitType, FitParams& xiParamRange,
                    std::ofstream& outputFile, std::ofstream& xsecFile,
                    std::string weight = "hybrid_combo", int chebyOrder = 2, int n_threads = 4);

// Legacy MakeXSecFitVariations.C getXSecFiles() with explicit paths. For each
// tree of dataFile (an energy-bin tree followed by its -t-bin trees, as
// written by divideNominalIntoBins) fits data and MC and writes to logDir:
//   totout_<name>.txt  totxsec_<name>.txt
//   diffout_<name>_<energy bin>.txt  diffxsec_<name>_<energy bin>.txt
// mcFile and thrownFile must contain trees of the same names. label is
// delim[0] (fit-plot subdirectory). params are updated by every fit and carry
// over to the next call, as in the legacy loop over run periods.
// Throws std::runtime_error if a file cannot be opened or a tree is missing.
void WriteXSecTables(const std::string& dataFile, const std::string& mcFile, const std::string& thrownFile,
                     TH1D* flux, const std::string& name, const std::string& label,
                     const std::string& fitType, FitParams& params, const std::string& logDir,
                     const std::string& weight = "hybrid_combo", int chebyOrder = 2);

} // namespace xsec
} // namespace gxana

#endif
