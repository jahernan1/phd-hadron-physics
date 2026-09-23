#ifndef GXANA_XSECTION_FLUX_H
#define GXANA_XSECTION_FLUX_H

#include <TH1D.h>

#include <string>

namespace gxana {
namespace xsec {

// Tagged photon flux histogram histName from fluxFile, detached from the file
// (the caller owns it). Throws std::runtime_error if the file or histogram is
// missing. Legacy FitFunctions.cpp chose the file by run period:
//   2017-01 flux_30274_31057_r4.root, 2018-01 flux_40856_42559.root,
//   2018-08 flux_50685_51768.root
TH1D* GetFluxHist(const std::string& fluxFile, const std::string& histName = "tagged_flux");

} // namespace xsec
} // namespace gxana

#endif
