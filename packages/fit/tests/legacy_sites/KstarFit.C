// FROZEN LEGACY COPY for the gxana::fit equivalence checks. Do not edit.
// Origin: analyses/kpkpxim/backgrounds/KstarFit.C at 1112839 (from AnalysisNote/utilities/KstarFit.C) lines 1:3 116:148
// Made by packages/fit/tests/legacy_sites/freeze.py: "w->factory(" -> "LegacyFactory(w, " (5, comments included),
// "LegacyFitDone(w, data);" after each fitTo statement (1). Nothing else changed.
#include "LegacyTrace.h"
#include "gxana/common/Paths.h"
// Fit the kstar that is seen in data gamma p -> K+ Y*->K+ (K*Lambda)
#include<RooPlot.h>
void rooFitHist(TH1D* hist, string histTitle)
{
  //setStyle();
  Double_t min_mass = hist->GetXaxis()->GetBinLowEdge(hist->FindFirstBinAbove());
  //if(min_mass < 1.26) min_mass = 1.26;

  RooWorkspace* w = new RooWorkspace("ws1");
  RooRealVar mass("mass", "M(K^{+}_{slow}#pi^{-}) (GeV/c^{2})", min_mass, 1.8);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  RooPlot* massframe = mass.frame(RooFit::Title(histTitle.c_str()));
  w->import(RooArgSet(mass));

  if(!data)
    cout << "Null data is the problem" << endl;

  LegacyFactory(w, "Voigtian::signal(mass,mean[0.89555, 0.89535, 0.89575], width[0.0473, 0.0468, 0.0481], sigma[0])");
  //LegacyFactory(w, "Gaussian::signal(mass,mean[0.89555, 0.89535, 0.89575], sigma[0.05, 0.01,0.09])");
  LegacyFactory(w, "Chebychev::bkgd(mass,{a0[-0.8,-5,-0.1], a1[-0.4, -5,-0.1], a2[0.5, 0.1,5]})");
  //LegacyFactory(w, "ArgusBG::bkgd(mass,mo[1.9, 1.8, 2.0], c[0.5, 0, 2.0], p[2])");
  
  //Create model and fit to data
  LegacyFactory(w, "SUM::model( nkstar[200,1,1e6]*signal, nbkgd[200,1,1e6]*bkgd)");
  
  w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::SumW2Error(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));//,RooFit::Range(0.75,1.8)
  LegacyFitDone(w, data);
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.55, 0.9, 0.9));
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(3));
  w->pdf("signal")->plotOn(massframe,RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nkstar")->getVal(), RooAbsReal::NumEvent));
  w->pdf("bkgd")->plotOn(massframe, RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent));
  //RooFit::DrawOption("F"), RooFit::FillColor(kAzure-1),RooFit::FillStyle(4010),RooFit::MoveToBack()

  massframe->Draw();
}
