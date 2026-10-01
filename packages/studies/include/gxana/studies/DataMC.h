#ifndef GXANA_STUDIES_DATAMC_H
#define GXANA_STUDIES_DATAMC_H

// Data/MC kinematic comparison: the drawing half of archive/root_macros/GetKinematicsDataMC_RF.C
// (MakeStackedHist, setStyle). The histograms are filled by gxana::FillPeriodHists.

#include <TH1D.h>

#include <string>

namespace gxana {
namespace studies {

// MakeStackedHist (GetKinematicsDataMC_RF.C:182-265) for two histograms, statement for
// statement (the unused `entries`/`sprintf` is omitted): both rebinned by 2, the second scaled by the integral ratio in the direction its
// maximum picks, THStack "nostack", legend top left ("tl") or top right; legTitle "Data" draws the
// first as points ("lep"), anything else as a grey filled histogram ("f"). Saved as
// <outDir>/<first's name>_<identifier>_ac.pdf. Both histograms are modified.
void DrawStacked(TH1D* first, TH1D* second, const std::string& title, const std::string& identifier,
                 const std::string& legPos, const std::string& legTitle, const std::string& outDir);

// setStyle() of GetKinematicsDataMC_RF.C.
void ApplyDataMCStyle();

} // namespace studies
} // namespace gxana

#endif
