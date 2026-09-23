#ifndef GXANA_XSECTION_BARLOW_H
#define GXANA_XSECTION_BARLOW_H

#include <TGraphErrors.h>

#include <vector>

namespace gxana {
namespace xsec {

// Barlow significance per point: (y_nominal - y_variation) / sqrt(|sigma_n^2 - sigma_v^2|),
// 0 where that sigma is 0; x errors from variation, y errors 0. Signed, as in
// systematics/PlotXSecBarlow*.C (GetBarlowResults.C used the absolute value).
TGraphErrors* calc_barlow(TGraphErrors* nominal, TGraphErrors* variation);

// Population standard deviation of y across graphs at each point (x from the
// first graph, zero errors, red markers); nullptr if graphs is empty.
TGraphErrors* calculateStdDevGraph(const std::vector<TGraphErrors*>& graphs);

} // namespace xsec
} // namespace gxana

#endif
