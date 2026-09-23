#ifndef GXANA_XSECTION_YIELDFIT_H
#define GXANA_XSECTION_YIELDFIT_H

#include <RooDataSet.h>
#include <RooWorkspace.h>
#include <TTree.h>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gxana {
namespace xsec {

// Signal-shape parameters: name -> {initial value, min, max}. Successful fits
// overwrite [0] with the fitted value, so one map carries over between fits.
using FitParams = std::unordered_map<std::string, std::vector<double>>;

// Directory for fit PDFs; "" (default) saves none. Plots go to
// <dir>/<delim[0]>/recon_<delim[1]>_<delim[2]>.pdf (MC) and data_... (data).
void SetFitPlotDir(const std::string& dir);
const std::string& GetFitPlotDir();

// params in RooFit factory argument order: Johnson (mu, lambda, gamma, delta),
// Gaussian (mean, sigma), Voigtian (mean, width, sigma); other fit types, or
// maps with other names, in map iteration order (legacy behavior).
std::vector<std::pair<std::string, std::vector<double>>> OrderedFitParams(const std::string& fitType,
                                                                          const FitParams& params);

// "<fitType>::xisignal(decayxim_M, name[init, min, max], ...)" for the MC fit.
std::string constructFitString(const std::string& fitType, const FitParams& params);
// Data-fit variant: Johnson fixes gamma and starts delta/lambda at (init-max)/2
// in [init, max]; Voigtian fixes width and uses [init, init, max] for sigma.
std::string constructFitStringData(const std::string& fitType, const FitParams& params);

bool AttemptFitMC(RooWorkspace* w, RooDataSet* data, FitParams& params);
bool AttemptFit(RooWorkspace* w, RooDataSet* data, FitParams& params, double lowerBound, double upperBound);

// Weighted fit of the Xi- mass (decayxim_M) in MC: yield = sum of weights.
// delim = {plot subdir, name, bin tree name}.
void RooFitMC(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double* yield,
              double* yield_err, std::string fitType, FitParams& params,
              std::string hist_weight = "hybrid_combo", int max_retries = 10);
// Extended weighted fit of data: signal + Chebychev background (order 1 or 2);
// yield = fitted signal events.
void RooFitData(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double* yield,
                double* yield_err, std::string fitType, FitParams& params, int chebyOrder = 2,
                std::string weight_name = "hybrid_combo", int max_retries = 10);

// Plot style of the legacy fit code (FitFunctions.cpp setStyle; not gxana::SetStyle).
void SetFitStyle();

} // namespace xsec
} // namespace gxana

#endif
