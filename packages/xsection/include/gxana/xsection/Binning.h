#ifndef GXANA_XSECTION_BINNING_H
#define GXANA_XSECTION_BINNING_H

#include <string>
#include <utility>
#include <vector>

namespace gxana {
namespace xsec {

using BinRanges = std::vector<std::pair<double, double>>;

// Forwarders to gxana::BinEdgeLabel / EnergyBinName / BinName (gxana/common/BinNames.h).
// Bin-edge text used in tree names and in the RDataFrame filters: std::to_string
// cut two digits after the point (6.4 -> "6.40", 11.4 -> "11.40", 6.405 -> "6.40").
std::string BinEdgeLabel(double edge);
// "emin_6.40_emax_7.40"
std::string EnergyBinName(double lowE, double highE);
// "emin_6.40_emax_7.40_tmin_0.10_tmax_0.35"
std::string BinName(double lowE, double highE, double lowT, double highT);
// {e0, e1, e2} -> {{e0, e1}, {e1, e2}}; throws std::invalid_argument unless
// there are at least two strictly increasing edges.
BinRanges EdgesToBins(const std::vector<double>& edges);

// From AnalysisNote/xsection/MakeBinnedTrees.cpp. Splits tree `treeName` of
// filePath into one tree per energy bin and one per (energy, -t) bin, all
// written to outputFilePath (recreated), keeping the legacy branch list (+
// qvalue_decayxim_M when data). The energy-only trees carry no t_dist cut:
// the thesis total cross sections were computed from energy bins without
// t_dist < 2.4 (author decision 2026-09-24); the (energy, -t) trees are
// unaffected since their own t filter never exceeds 2.4. Returns false if a
// file cannot be opened.
bool divideNominalIntoBins(const std::string& filePath, const std::string& outputFilePath,
                           const BinRanges& enRange, const BinRanges& tRange,
                           bool data = true, const std::string& treeName = "flatTree_kpkpxim");
// Same for the thrown tree: branches t_dist and beam_E, no t_dist < 2.4 cut.
bool divideThrownIntoBins(const std::string& filePath, const std::string& outputFilePath,
                          const BinRanges& enRange, const BinRanges& tRange,
                          const std::string& treeName = "flatTree_thrown_kpkpxim");
// From AnalysisNote/systematics/SplitVariationTrees.C (divideDataIntoBins):
// every TTree <T> in filePath -> <T>/<bin name> for each bin, all branches,
// no t_dist < 2.4 cut.
bool divideVariationTreesIntoBins(const std::string& filePath, const std::string& outputFilePath,
                                  const BinRanges& enRange, const BinRanges& tRange);

} // namespace xsec
} // namespace gxana

#endif
