#ifndef GXANA_COMMON_STACKEDHIST_H
#define GXANA_COMMON_STACKEDHIST_H

#include <string>
#include <vector>

class TH1D;

namespace gxana {

// Legacy MakeStackedHist (AnalysisNote/analysis/GetKinematicsDataMC_RF.C),
// one of 11 near-identical copies across the thesis macros. The copies
// disagree on default leg_pos/leg_title, so none are given here. plot_dir
// replaces each copy's hardcoded plotdir; saves
// "<plot_dir>/<arr_hist[0] name>_<identifier>_ac.pdf".
void MakeStackedHist(std::vector<TH1D*> arr_hist, std::string stack_title, std::string identifier,
                     std::string leg_pos, std::string leg_title, std::string plot_dir);

} // namespace gxana

#endif
