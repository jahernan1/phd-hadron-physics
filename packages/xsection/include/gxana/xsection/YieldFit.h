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

// params in RooFit factory argument order: Johnson and JohnsonMCShape (mu, lambda,
// gamma, delta), Gaussian (mean, sigma), Voigtian (mean, width, sigma); other fit types, or
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

// Fit type of the thesis tables (legacy MakeXSecFiles.C, run per combo weight
// into data/<weight>/): per bin, a Johnson fit to MC sets the signal shape, then data
// is fit with that shape (gamma, delta fixed; lambda >= MC value) plus a
// 2nd-order Chebychev. Parameters mu, lambda, gamma, delta are {start, min, max}
// of the MC fit; every bin restarts from them (no carry-over between bins).
extern const char* const kJohnsonMCShape;
// Fit type of the Barlow cut-variation tables (legacy GetXSecFilesUML.C): the
// same structure as JohnsonMCShape with different literals and error
// conventions; the list of differences is above MCShapeLiterals in YieldFit.cxx.
extern const char* const kJohnsonMCShapeSyst;
// True for kJohnsonMCShape and kJohnsonMCShapeSyst.
bool IsMCShapeFit(const std::string& fitType);

// MakeXSecFiles.C factory strings (start values printed with "%f"). fitType is
// kJohnsonMCShape or kJohnsonMCShapeSyst.
std::string constructFitStringMCShape(const FitParams& params, const std::string& fitType = kJohnsonMCShape);
std::string constructFitStringDataMCShape(const FitParams& params, const std::string& fitType = kJohnsonMCShape);

// MakeXSecFiles.C RooFitHistMC: yield = sum of weights, set only if the MC fit
// converges (NaN otherwise; legacy left it uninitialized). Updates params[*][0].
void RooFitMCShapeSeed(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double* yield,
                       double* yield_err, FitParams& params, std::string hist_weight = "hybrid_combo",
                       int max_retries = 10, const std::string& fitType = kJohnsonMCShape);
// MakeXSecFiles.C RooFitHist: extended fit of data, signal shape from params
// (as left by RooFitMCShapeSeed) + Chebychev(a0, a1); yield = fitted signal events.
void RooFitDataMCShape(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double* yield,
                       double* yield_err, FitParams& params, std::string hist_weight = "hybrid_combo",
                       int max_retries = 10, const std::string& fitType = kJohnsonMCShape);

// Fit type of the dissertation mcPdf / mcPdf_cheby1 labels (legacy
// MakeXSecFitMC.C): the signal shape is the binned MC mass histogram (RooHistPdf),
// data is fit with it plus a Chebychev of order 1 or 2.
extern const char* const kMCPdf;
bool IsMCPdfFit(const std::string& fitType);
// MakeXSecFitMC.C getHistogramPdf + RooFitHist for one bin. yieldMC = sum of MC
// weights (err sqrt(N)); yield = fitted signal events (err sqrt(N), as legacy).
void RooFitMCPdf(TTree* mcTree, TTree* dataTree, std::string histTitle, std::vector<std::string> delim,
                 double* yieldMC, double* yieldMC_err, double* yield, double* yield_err,
                 std::string weight = "hybrid_combo", int chebyOrder = 2);

// Plot style of the legacy fit code (FitFunctions.cpp setStyle; not gxana::SetStyle).
void SetFitStyle();

} // namespace xsec
} // namespace gxana

#endif
