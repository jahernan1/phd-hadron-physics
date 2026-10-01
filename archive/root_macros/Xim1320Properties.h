#ifndef XIM1320PROPERTIES_H
#define XIM1320PROPERTIES_H

#include <string>
#include <vector>
#include <TH1.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TLine.h>
#include <RooWorkspace.h>
#include <RooRealVar.h>
#include <RooDataHist.h>
#include <RooPlot.h>
#include <RooFit.h>
#include <RooAbsCollection.h>
#include <RooAbsPdf.h>
// gxana: RooChi2Var.h has no public header in ROOT 6.40 and is unused here.
#include <ROOT/RDataFrame.hxx>

// gxana: GetXim1320_IM is declared but never defined in Xim1320Properties.cpp
// (legacy PlotXim1320Properties.C called it this way; kept for load-only use).
void GetXim1320_IM(const std::string rootName, const std::string delim = "hybrid_combo", int n_threads = 4);
void setStyle();
void RooFitHist(TH1* hist, const char* histTitle, std::string delim, double* correction, std::vector<double> params);
void RooFitHistMC(TH1* hist, const char* histTitle, std::string delim, double* correction, std::vector<double>& params);
void GetMassAnalysis(std::vector<TH1D*> hists, std::string name);
TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon);
TH1D* GetAcceptanceCorrHist1D(std::vector<TH1D*> vec_hist, TH1D* hist_accept);
void GetLifetimeAnalysis(std::vector<TH1D*> hists, std::string name);
void GetSpinAnalysis(std::vector<TH1D*> hists, std::string name);
void GetXimHistos(std::string rootName, std::string delim = "kphighrap", int n_threads = 4);

#endif // XIM1320PROPERTIES_H
