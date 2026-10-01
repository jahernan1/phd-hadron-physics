// FROZEN LEGACY COPY for the gxana::fit equivalence checks. Do not edit.
// Origin: analyses/kpkpxim/measurements/mass/MakeXim1320_IM_Res.C at 4563248 (from AnalysisNote/utilities/MakeXim1320_IM_Res.C) lines 1:2 57:204
// Made by packages/fit/tests/legacy_sites/freeze.py: "w->factory(" -> "LegacyFactory(w, " (5, comments included),
// "LegacyFitDone(w, data);" after each fitTo statement (1). Nothing else changed.
#include "LegacyTrace.h"
#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
void RooFitHist(TH1* hist, const char* histTitle, string delim)
{
  RooWorkspace* w = new RooWorkspace(histTitle);
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.45);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  RooPlot* massframe = mass.frame(RooFit::Title(histTitle));
  TCanvas *fitCan = new TCanvas("fitCan"," c", 700, 700);
  TPad* topPad = new TPad("topPad", "Top Pad", 0.0, 0.3, 1.0, 1.0);
  TPad* bottomPad = new TPad("bottomPad", "Bottom Pad", 0.0, 0.0, 1.0, 0.3);
  topPad->SetBottomMargin(0.02);
  topPad->SetGrid();
  bottomPad->SetTopMargin(0.015);
  bottomPad->SetBottomMargin(0.4);
  bottomPad->SetGrid(1,0);
  
  fitCan->cd();
  topPad->Draw();
  bottomPad->Draw();

  // Draw graphs in the top pad
  topPad->cd();
    
  w->import(RooArgSet(mass));
  
  //Build model and Fit data
  LegacyFactory(w, "Chebychev::bkgd(mass,{a0[0.8,1.e-2,2],a1[-0.2,-2,-1e-2]})");//,a1[-0.1,-1.e2,-1e-2]
  LegacyFactory(w, "Gaussian::sigma(mass,mean[1.385,1.383,1.388],sig[0.0394,0.01,0.05])");
  //LegacyFactory(w, "Voigtian::sigma(mass,mean[1.387,1.383,1.39],width[0.0394,0.034,0.042], sig[0.007])");
  LegacyFactory(w, "Johnson::xigaus(mass,mu[1.32171,1.321,1.323],lambda[0.005,0.003,0.007], gamma[0,-1,1], delta[1.1,0.1,10])");
   
  //Create model and fit to data
  LegacyFactory(w, "SUM::model( nsigma[0,0,1e6]*sigma, nbkgd[1000,1,1e6]*bkgd, nxi[10000,1,1e5]*xigaus)");//nbkgd[200,1,1e6]*bkgd,
  //
  w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));
  LegacyFitDone(w, data);
  //Plot model and data 
  data->plotOn(massframe, RooFit::Name("data"));
  //w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.51, 0.95, 0.92) );
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(3), RooFit::Name("model"));
  w->pdf("xigaus")->plotOn(massframe,RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(3001),RooFit::MoveToBack(), RooFit::Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), RooFit::Name("xigaus"));
  w->pdf("sigma")->plotOn(massframe, RooFit::DrawOption("F"),RooFit::LineColor(kMagenta),RooFit::FillColor(kMagenta), RooFit::FillStyle(3001), RooFit::Normalization(w->var("nsigma")->getVal(), RooAbsReal::NumEvent), RooFit::Name("sigma"));
  w->pdf("bkgd")->plotOn(massframe, RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), RooFit::Name("bkgd"));
  //set up specific frame styling
  massframe->GetYaxis()->SetMaxDigits(2);
  massframe->GetYaxis()->SetNdivisions(505);
    
  massframe->Draw();
  //Get approxmate chisq of model and data fit
  // gxana: RooChi2Var.h has no public header in ROOT 6.40; the chi2 object was unused (next line already commented out), dropped.
  //Double_t chi2_val = chi2.getVal()/(100-10);
  Double_t chiSqNdf = massframe->chiSquare("model","data",10);
  printf("ChiSqNdf: %f\n", chiSqNdf);
  
  //Get yield and error
  double yield = w->var("nxi")->getVal();
  //*yield_err = TMath::Sqrt(*yield);
  double yield_err = w->var("nxi")->getError();
  //Get mean and variance
  double xiMu = w->var("mu")->getVal(); double xiMuErr = w->var("mu")->getError();
  double xiLambda = w->var("lambda")->getVal(); double xiLambdaErr = w->var("lambda")->getError();
  double xiDelta = w->var("delta")->getVal(); double xiDeltaErr = w->var("delta")->getError();
  double xiGamma = w->var("gamma")->getVal(); double xiGammaErr = w->var("gamma")->getError();
  double xiMean = xiMu - xiLambda * exp(1 / (2*pow(xiDelta,2)) ) * sinh(xiGamma/xiDelta);
  double xiMeanErr = xiMean * sqrt( pow( xiMuErr/xiMu, 2) + pow( xiLambdaErr/ xiLambda, 2) + pow( xiDelta/ xiDeltaErr, 2) + pow( xiGammaErr/xiGamma, 2) ); 
  double xiSigma = sqrt( pow(xiLambda, 2)/2*(exp(pow(xiDelta, -2) ) - 1 )*(exp(pow(xiDelta, -2) )*cosh(2*xiGamma/xiDelta )+1));
  double xiSigmaErr = xiSigma * sqrt( pow( xiDelta/ xiDeltaErr, 2) + pow( xiGammaErr/xiGamma, 2) );
  printf("Mean and Sigma: %f, %f \n", xiMean, xiSigma);
  //Get FWHM
  auto fwhm = xiSigma*2.35482004503;
  cout << "FWHM: " << fwhm << endl;
  
  double pxmax = fitCan->GetUxmax();
  double pxmin = fitCan->GetUxmax();
  double pymax = fitCan->GetUymax();
  double pymin = fitCan->GetUymax();

  printf("Yield for kpkpxim hist: %s\n", hist->GetName());
  printf("%f +/- %f\n", yield, yield_err);

  auto legend = new TLegend(0.147,0.73,0.37,0.92); //top right corner
  legend->SetTextSize(0.065);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  //legend->AddEntry(h1,"Histogram filled with random numbers","f");
    
  legend->AddEntry(massframe->findObject("data") ,"Data","lep");
  legend->AddEntry(massframe->findObject("model"),"Fit","l");
  legend->AddEntry(massframe->findObject("xigaus"),"Signal","f");
  //legend->AddEntry(massframe->findObject("sigma"),"Gaussian","l");
  //legend->AddEntry(massframe->findObject("bkgd"), "Background", "l");
 
  TLatex latex;
  char str[100];
  char str_mean[100];
  char str_sigma[100];
  char str_chisq[100];
  std::sprintf(str, "#color[4]{N_{#Xi^{-}} = %.0f #pm %.0f}", yield, yield_err);
  printf("Mu: %f +/- %f\n", xiMean*1e3, xiMuErr*1e3);
  std::sprintf(str_mean, "#color[4]{M_{#Xi^{-}} = %.2f MeV}", xiMean*1e3);
  std::sprintf(str_sigma, "#color[4]{#sigma_{#Xi^{-}} = %.2f MeV}", xiSigma*1e3);
  std::sprintf(str_chisq, "#color[4]{#chi^{2}_{#nu} = %.3f}",chiSqNdf );
  latex.SetTextSize(0.065);
  latex.DrawLatex(1.352, hist->GetMaximum(), "GlueX#lower[-0.15]{-}#kern[0.2]{I}");
  latex.DrawLatex(1.352, hist->GetMaximum()-300, str);
  latex.DrawLatex(1.3505, hist->GetMaximum()-620, str_mean);
  latex.DrawLatex(1.353, hist->GetMaximum()-940, str_sigma);
  latex.DrawLatex(1.356, hist->GetMaximum()-1260, str_chisq);
  //latex.DrawLatex(1.395, 2000, "Events");

  double left_2sigma = xiMean-2*xiSigma;
  double right_2sigma = xiMean+2*xiSigma;
  TLine* left = new TLine(left_2sigma,0,left_2sigma,hist->GetMaximum()/2);
  left->SetLineColor(kAzure); left->SetLineWidth(2); left->SetLineStyle(kDashed);
  left->Draw();
  TLine* right = new TLine(right_2sigma,0,right_2sigma,hist->GetMaximum()/2);
  right->SetLineColor(kAzure); right->SetLineWidth(2); right->SetLineStyle(kDashed);
  right->Draw();
  
  legend->Draw("same");

  // Get the residual plot
  bottomPad->cd();
  bottomPad->SetGrid();
  RooHist* resPlot = massframe->residHist("data","model",true);
  resPlot->SetTitle(Form("; %s;#sigma_{#scale[0.7]{pull}}",massframe->GetXaxis()->GetTitle()));
  //resPlot->SetMarkerStyle(24);
  resPlot->GetYaxis()->CenterTitle(true);
  resPlot->GetYaxis()->SetNdivisions(505);
  resPlot->GetXaxis()->SetLabelSize(0.13);
  resPlot->GetYaxis()->SetLabelSize(0.13);
  resPlot->GetXaxis()->SetTitleSize(0.18);
  resPlot->GetYaxis()->SetTitleSize(0.18);
  resPlot->GetXaxis()->SetTitleOffset(0.9);
  resPlot->GetYaxis()->SetTitleOffset(0.25);
  resPlot->GetYaxis()->SetRangeUser(-5,5);
  resPlot->DrawClone("ap");

  double xmin = TMath::MinElement(resPlot->GetN(), resPlot->GetX());
  double xmax = TMath::MaxElement(resPlot->GetN(), resPlot->GetX());
  cout << xmin << "\t" << xmax << endl;
  TLine zeroline(xmin,0,xmax,0);
  zeroline.SetLineWidth(4);
  zeroline.SetLineStyle(1);
  zeroline.DrawClone();
      
  fitCan->SaveAs( (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/prod_plots/Xim_InvariantMassFit_Phase1_residual")+delim+".pdf").c_str());
  //fitCan->SaveAs( (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/plots/Xim_InvariantMassFit_Phase1")+delim+".pdf").c_str());
}
