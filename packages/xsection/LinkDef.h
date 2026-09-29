#ifdef __CLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ namespace gxana::xsec;
#pragma link C++ function gxana::xsec::BinEdgeLabel;
#pragma link C++ function gxana::xsec::EnergyBinName;
#pragma link C++ function gxana::xsec::BinName;
#pragma link C++ function gxana::xsec::EdgesToBins;
#pragma link C++ function gxana::xsec::divideNominalIntoBins;
#pragma link C++ function gxana::xsec::divideThrownIntoBins;
#pragma link C++ function gxana::xsec::divideVariationTreesIntoBins;
#pragma link C++ function gxana::xsec::SetFitPlotDir;
#pragma link C++ function gxana::xsec::GetFitPlotDir;
#pragma link C++ function gxana::xsec::OrderedFitParams;
#pragma link C++ function gxana::xsec::constructFitString;
#pragma link C++ function gxana::xsec::constructFitStringData;
#pragma link C++ function gxana::xsec::AttemptFitMC;
#pragma link C++ function gxana::xsec::AttemptFit;
#pragma link C++ function gxana::xsec::RooFitMC;
#pragma link C++ function gxana::xsec::RooFitData;
#pragma link C++ function gxana::xsec::SetFitStyle;
#pragma link C++ function gxana::xsec::GetFluxHist;
#pragma link C++ function gxana::xsec::GetDiffXSecFile;
#pragma link C++ function gxana::xsec::GetTotXSecFile;
#pragma link C++ function gxana::xsec::WriteXSecTables;
#pragma link C++ function gxana::xsec::SetPlotDir;
#pragma link C++ function gxana::xsec::PlotDir;
#pragma link C++ function gxana::xsec::plotDiffXSec;
#pragma link C++ function gxana::xsec::plotWeightedXSec;
#pragma link C++ function gxana::xsec::plotOneWeightedXSec;
#pragma link C++ function gxana::xsec::plotFinalWeightedXSec;
#endif
