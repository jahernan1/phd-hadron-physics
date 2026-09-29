#ifndef GXANA_XSECTION_XSEC_H
#define GXANA_XSECTION_XSEC_H

#include "gxana/xsection/YieldFit.h"

#include <TAxis.h>
#include <TH1D.h>
#include <TTree.h>

#include <fstream>
#include <string>
#include <vector>

namespace gxana {
namespace xsec {

// ROOT 6.24 TAxis::FindFixBin formula (fixed-width axis): bin =
// 1 + int(nbins*(x-xmin)/(xmax-xmin)), underflow 0 below xmin, overflow
// nbins+1 at/above xmax. ROOT >= 6.32's FindBin resolves exact bin-edge
// values one bin higher in some cases (e.g. the flux histogram's 8.68, 9.26,
// 10.18 edges); the thesis flux windows were integrated with the 6.24
// formula, so reproducing them on any ROOT version requires this helper
// instead of TAxis::FindBin.
int LegacyFindBin(const TAxis* axis, double x);

// One (E_gamma, -t) bin (delim[2] = its tree name, e.g.
// "emin_6.40_emax_7.40_tmin_0.10_tmax_0.35"): fit data (trees[0]) and MC
// (trees[1]), count thrown (trees[2]), integrate flux over the energy bin, and
// append a row to outputFile (t center, half width, yields, acceptance, flux)
// and to xsecFile (t center, dsigma/dt [nb/GeV^2], half width, error). Bins
// with <= 10 data entries in 1.30-1.35 GeV (<= 25 for JohnsonMCShape) get
// cross section 0, and NaN in the MC, thrown and acceptance columns.
// Verbatim from AnalysisNote/xsection/FitFunctions.cpp (JohnsonMCShape:
// MakeXSecFiles.C, whose fits use a fresh copy of xiParamRange per bin);
// n_threads is unused.
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
// Directory mode: when the top-level keys of dataFile are TDirectoryFile (the
// output of divideVariationTreesIntoBins, legacy GetXSecFilesUML.C), each
// directory not ending in "_mc" (vary_<cut>_<value>) is one variation: its data
// trees come from that directory, its reconstructed-MC trees from the directory
// <dir>_mc of mcFile (dataFile itself for the systematics stage), and its
// thrown trees from the top level of thrownFile. Tables are then named
// totout_/totxsec_<name>_<dir>.txt and diffout_/diffxsec_<name>_<dir>_<energy bin>.txt.
// Throws std::runtime_error if a file cannot be opened or a tree is missing.
void WriteXSecTables(const std::string& dataFile, const std::string& mcFile, const std::string& thrownFile,
                     TH1D* flux, const std::string& name, const std::string& label,
                     const std::string& fitType, FitParams& params, const std::string& logDir,
                     const std::string& weight = "hybrid_combo", int chebyOrder = 2);

} // namespace xsec
} // namespace gxana

#endif
