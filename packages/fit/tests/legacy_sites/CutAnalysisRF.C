// FROZEN LEGACY COPY for the gxana::fit equivalence checks. Do not edit.
// Origin: analyses/kpkpxim/selection/CutAnalysisRF.C at dbeb86c (from AnalysisNote/analysis/event_selection/CutAnalysisRF.C) lines 1:17 241:322
// Made by packages/fit/tests/legacy_sites/freeze.py: "w->factory(" -> "LegacyFactory(w, " (6, comments included),
// "LegacyFitDone(w, data);" after each fitTo statement (1). Nothing else changed.
#include "LegacyTrace.h"
#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
#include "TF1.h"
#include "TH1.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include "TMath.h"
#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
using namespace RooFit;
void rooFitHist(TH1* hist, string histTitle, double* sigYield, double* sigYieldErr, double* bkgYield, double* bkgYieldErr)
{
    Double_t min_mass = hist->GetXaxis()->GetBinLowEdge(hist->FindFirstBinAbove(0,1,5, hist->FindBin(1.3)));
    if(min_mass < 1.28) min_mass = 1.28;
  
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.28, 1.45);
    RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
    RooPlot* massframe = mass.frame(RooFit::Title(histTitle.c_str()));
    w->import(RooArgSet(mass));
    //Style the histogram
    massframe->GetYaxis()->SetMaxDigits(3);
    massframe->SetNdivisions(505);
    
    //Define pdfs
    LegacyFactory(w, "Chebychev::bkgd(mass,{a0[0.8,0.1,1.5],a1[-0.2,-1,-0.1]})");//,a1[-0.1,-2,-1e-2]
    //LegacyFactory(w, "Voigtian::sigma(mass,mean[1.385,1.383,1.388],sig[0.0055. 0.004, 0.006], width[0.015, 0.01, 0.042])");
    //LegacyFactory(w, "Gaussian::sigma(mass,mean[1.385,1.383,1.390],sig[0.019,0.018,0.022])");
    LegacyFactory(w, "Johnson::xigaus(mass,mu[1.3217,1.32,1.33],lambda[0.0055,0.004,0.006], gamma[0], delta[1.3,1.,2.])");
    //LegacyFactory(w, "Gaussian::xigaus(mass,mean_xi[1.322,1.31,1.33],sigma_xi[0.0055,0.004,0.008])");
    
    //Create model and fit to data
    LegacyFactory(w, "SUM::model(  nbkgd[2000,1,1e6]*bkgd, nxi[1000,1,1e6]*xigaus)");//nsigma[300,1,1e6]*sigma,
    w->pdf("model")->fitTo(*data,Extended(true),SumW2Error(true),PrintLevel(-1),PrintEvalErrors(-1),Verbose(false),Warnings(false));
    LegacyFitDone(w, data);
    //Plot params and data and fit
    data->plotOn(massframe,MarkerStyle(24),MarkerSize(0.4));
    w->pdf("model")->paramOn(massframe, Format("N",AutoPrecision(0)), Layout(0.45, 0.9, 0.85) ,Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nbkgd"))));// *w->var("nsigma"), *w->var("mean"), *w->var("sig")
    massframe->getAttText()->SetTextSize(0.08) ;
    massframe->getAttFill()->SetFillStyle(0) ;
    massframe->getAttLine()->SetLineColor(0) ;
    w->pdf("model")->plotOn(massframe, LineWidth(1));
    w->pdf("xigaus")->plotOn(massframe,LineWidth(1), LineStyle(kDotted), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent));
    //w->pdf("sigma")->plotOn(massframe, LineWidth(1), LineStyle(kDotted), Normalization(w->var("nsigma")->getVal(), RooAbsReal::NumEvent));
    w->pdf("bkgd")->plotOn(massframe,LineWidth(1), LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent));
  
    //Get background under signal region
    double xiMu = w->var("mu")->getVal();
    double xiLambda = w->var("lambda")->getVal();
    double xiDelta = w->var("delta")->getVal();
    double xiGamma = w->var("gamma")->getVal();
    double xiMean = xiMu - xiLambda*exp(1 / (2*pow(xiDelta,2)) )*sinh(xiGamma/xiDelta);
    double xiSigma = sqrt( pow(xiLambda, 2)/2*(exp(pow(xiDelta, -2) ) - 1 )*(exp(pow(xiDelta, -2) )*cosh(2*xiGamma/xiDelta )+1));
    //For Gaussian signal
    // double xiMean = w->var("mean_xi")->getVal();
    // double xiSigma = w->var("sigma_xi")->getVal();
  
    printf("Mean and Sigma: %f, %f \n", xiMean, xiSigma);
    double xCutL = xiMean - 2*xiSigma;
    double xCutR = xiMean + 2*xiSigma;
    w->var("mass")->setRange("signal", xCutL, xCutR);
  
    RooAbsReal* sig_sig = w->pdf("xigaus")->createIntegral(mass, RooFit::NormSet(mass), RooFit::Range("signal"));
    printf("Signal Fraction in 2Sigma Window: %f \n", sig_sig->getVal());
    printf("Signal Yield in 2Sigma Window: %f \n", w->var("nxi")->getVal()*sig_sig->getVal());

    *sigYield = w->var("nxi")->getVal()*sig_sig->getVal();
    *sigYieldErr = w->var("nxi")->getError()*sig_sig->getVal();
  
    RooAbsReal* bkg_sig = w->pdf("bkgd")->createIntegral(mass, RooFit::NormSet(mass), RooFit::Range("signal"));
    //RooAbsReal* bkg_sigma = w->pdf("sigma")->createIntegral(mass, RooFit::NormSet(mass), RooFit::Range("signal"));
    printf("Background Fraction of 2Sigma Window: %f \n", bkg_sig->getVal());//,bkg_sigma->getVal());
    printf("Background Yield in 2Sigma Window: %f \n", w->var("nbkgd")->getVal()*bkg_sig->getVal());//+w->var("nsigma")->getVal()*bkg_sigma->getVal());
  
    *bkgYield = w->var("nbkgd")->getVal()*bkg_sig->getVal();//+w->var("nsigma")->getVal()*bkg_sigma->getVal();
    *bkgYieldErr = w->var("nbkgd")->getError()*bkg_sig->getVal();

    //Draw cut lines for signal region
    //massframe->SetMaximum(massframe->GetMaximum()+100);
    double ypadmax = massframe->GetMaximum();
    TLine* cutLineL = new TLine(xCutL, 0.0, xCutL, ypadmax);
    cutLineL->SetLineWidth(1);
    cutLineL->SetLineColor(kRed+1);
    massframe->addObject(cutLineL, " ");
    TLine* cutLineR = new TLine(xCutR, 0.0, xCutR, ypadmax);
    cutLineR->SetLineWidth(1);
    cutLineR->SetLineColor(kRed+1);
    massframe->addObject(cutLineR, " ");

    massframe->Draw();
    massframe->SetMarkerStyle(24);
    massframe->SetMarkerSize(0.2);
}
