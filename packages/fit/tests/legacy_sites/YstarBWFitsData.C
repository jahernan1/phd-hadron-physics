// FROZEN LEGACY COPY for the gxana::fit equivalence checks. Do not edit.
// Origin: analyses/kpkpxim/backgrounds/YstarBWFitsData.C at 442121a (from AnalysisNote/utilities/YstarBWFitsData.C) lines 1:2 75:104
// Made by packages/fit/tests/legacy_sites/freeze.py: "w->factory(" -> "LegacyFactory(w, " (4, comments included),
// "LegacyFitDone(w, data);" after each fitTo statement (1). Nothing else changed.
#include "LegacyTrace.h"
#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
void rooFitHist(TH1D* hist, string histTitle)
{
  //setStyle();
  Double_t min_mass = hist->GetXaxis()->GetBinLowEdge(hist->FindFirstBinAbove());
  //if(min_mass < 1.26) min_mass = 1.26;

  RooWorkspace* w = new RooWorkspace("ws");
  RooRealVar mass("mass", "M(K^{+}_{slow}#Xi^{-}) (GeV/c^{2})", min_mass, 4.0);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  RooPlot* massframe = mass.frame(RooFit::Title(histTitle.c_str()));
  w->import(RooArgSet(mass));

  LegacyFactory(w, "BreitWigner::bw1(mass,mean1[2.1,1.9,2.2], width1[0.15, 0.1,0.2])");
  LegacyFactory(w, "BreitWigner::bw2(mass,mean2[2.3,2.2,2.5], width2[0.4, 0.15,0.6])");
  LegacyFactory(w, "BreitWigner::bw3(mass,mean3[2.8, 2.6,3], width3[0.6,0.3,1])");

  //Create model and fit to data
  LegacyFactory(w, "SUM::model( nbw1[0]*bw1, nbw2[300,1,1e6]*bw2, nbw3[200,1,1e6]*bw3)");
  
  w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::SumW2Error(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));
  LegacyFitDone(w, data);
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.6, 0.85, 0.95));
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(3));
  w->pdf("bw1")->plotOn(massframe,RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbw1")->getVal(), RooAbsReal::NumEvent));
  w->pdf("bw2")->plotOn(massframe, RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbw2")->getVal(), RooAbsReal::NumEvent));
  w->pdf("bw3")->plotOn(massframe, RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbw3")->getVal(), RooAbsReal::NumEvent));
  //RooFit::DrawOption("F"), RooFit::FillColor(kAzure-1),RooFit::FillStyle(4010),RooFit::MoveToBack()

  massframe->Draw();
}
