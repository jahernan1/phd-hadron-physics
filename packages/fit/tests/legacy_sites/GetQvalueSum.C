// FROZEN LEGACY COPY for the gxana::fit equivalence checks. Do not edit.
// Origin: analyses/kpkpxim/signal_extraction/qfactors/scripts/GetQvalueSum.C at 5eee8cd (from AnalysisNote/QFactors/scripts/GetQvalueSum.C) lines 1:2 137:169
// Made by packages/fit/tests/legacy_sites/freeze.py: "w->factory(" -> "LegacyFactory(w, " (7, comments included),
// "LegacyFitDone(w, data);" after each fitTo statement (1). Nothing else changed.
#include "LegacyTrace.h"
#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
void rooFitHist(TH1* hist,char* histTitle, char* ws_name, double* yield, double* yield_err)
{
  Double_t min_mass = hist->GetXaxis()->GetBinLowEdge(hist->FindFirstBinAbove(0,1,1, hist->FindBin(1.3)));
  if(min_mass < 1.28) min_mass = 1.28;

  RooWorkspace* w = new RooWorkspace(ws_name);
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", min_mass, 1.45);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  //XiMassKinFit_Ebin_accsub->Print();
  w->import(RooArgSet(mass)); 
  
  LegacyFactory(w, "Chebychev::bkgd(mass,{a0[0.8,0.1,1.2]})");//,a1[-0.1,-0.5,-0.05]
  //LegacyFactory(w, "CBShape::xigaus(mass,xi_mean[1.322],xi_sig[0.005,0.003,0.01], alpha[5,1,20], n[5,0,10])");
  LegacyFactory(w, "Johnson::xigaus(mass,mu[1.322,1.31, 1.33],lambda[0.006, 0.005, 0.01], gamma[0], delta[1.5, 1, 2])");
  //LegacyFactory(w, "Gaussian::xigaus(mass,xi_mean[1.322],xi_sig[0.005, 0.001, 0.01])");
  //LegacyFactory(w, "Voigtian::sigma(mass,mean[1.387],sig[0.006, 0.005,0.01], width[0.0394, 0.0373, 0.0415])");
  //LegacyFactory(w, "Gaussian::siggaus(mass,sigma_mean[1.385],sigma_sig[0.016])");
  LegacyFactory(w, "SUM::model(nbkgd[500,0,1e6]*bkgd, nxi[100,1,1e6]*xigaus)");

  RooPlot* massframe = mass.frame(RooFit::Title(histTitle));
  w->pdf("model")->fitTo(*data,RooFit::Extended(kTRUE),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));
  LegacyFitDone(w, data);
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.5, 0.95, 0.92) );
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(2) );
  w->pdf("xigaus")->plotOn(massframe,RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(4010),RooFit::MoveToBack(), RooFit::Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent) );
  //w->pdf("sigma")->plotOn(massframe, RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(4010),RooFit::MoveToBack() , RooFit::Normalization(w->var("nsigma")->getVal(), RooAbsReal::NumEvent));
  w->pdf("bkgd")->plotOn(massframe, RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent) );
  
  *yield = w->var("nxi")->getVal();
  *yield_err = w->var("nxi")->getError();
  
  massframe->Draw();
}
