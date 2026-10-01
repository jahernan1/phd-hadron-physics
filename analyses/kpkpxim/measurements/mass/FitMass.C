#include "../common/XimInputs.h"
#include "gxana/fit/Fit.h"
#include "gxana/fit/Johnson.h"
#include "gxana/fit/Model.h"

#include <RooRealVar.h>
#include <RooDataHist.h>
#include <RooPlot.h>
#include <RooWorkspace.h>
#include <RooAbsPdf.h>
#include <RooMsgService.h>
#include <TLine.h>
#include <TLegend.h>
#include <TCanvas.h>

using namespace RooFit;

// Mass fit: Johnson-SU signal on the reconstructed MC gives the shift (PDG mass minus MC
// mean); the data fit (Johnson + Gaussian Xi(1530) + Chebychev) is reported as
// mean_data + shift. Reads xim_mass.root (PrepMass.C), writes the fit canvases back into
// it and XimIM_{mc,dataCorr}_<stem>.pdf to $GXANA_OUTPUT/kpkpxim/prod_plots/.
void RooFitHist(TH1* hist, const char* histTitle, std::string delim, double *correction, std::vector<double> params)
{
  RooWorkspace* w = new RooWorkspace(histTitle);
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.45);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  RooPlot* massframe = mass.frame(RooFit::Title(histTitle));
  TCanvas *fitCan = new TCanvas("fitCan"," c", 800, 700);
  double maxY = hist->GetMaximum();
  
  w->import(RooArgSet(mass));
  
  //Build model and Fit data
  gxana::fit::BuildModel(*w, {
      gxana::fit::Chebychev("bkgd", "mass", {{"a0", "0.8,1.e-2,2"}, {"a1", "-0.2,-2,-1e-2"}}),//,a1[-0.1,-1.e2,-1e-2]
      gxana::fit::Gaussian("sigma", "mass", {"mean", "1.385,1.383,1.388"}, {"sig", "0.0394,0.01,0.05"}),
      //w->factory("Voigtian::sigma(mass,mean[1.387,1.383,1.39],width[0.0394,0.034,0.042], sig[0.007])");
      gxana::fit::Johnson("xisignal", "mass", {"mu", gxana::fit::Fx(params[0]) + ",1.32,1.33"},
                          {"lambda", gxana::fit::Fx(params[1]) + "," + gxana::fit::Fx(params[1]) + ",0.008"},
                          {"gamma", gxana::fit::Fx(params[2])}, {"delta", gxana::fit::Fx(params[3])}),
      //Create model and fit to data
      gxana::fit::Sum("model", {{{"nsigma", "0,1,1e6"}, "sigma"}, {{"nbkgd", "1000,1,1e6"}, "bkgd"},
                                {{"nxi", "10000,1,1e6"}, "xisignal"}})});//nbkgd[200,1,1e6]*bkgd,
  gxana::fit::RunFit(*w->pdf("model"), *data, RooFit::Extended(true), RooFit::PrintLevel(-1),
                     RooFit::PrintEvalErrors(-1), RooFit::Verbose(false), RooFit::Warnings(false));
  //Plot model and data 
  data->plotOn(massframe, RooFit::Name("data"));
  //w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.51, 0.95, 0.92) );
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(3), RooFit::Name("model"));
  w->pdf("xisignal")->plotOn(massframe,RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(3001),RooFit::MoveToBack(), RooFit::Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), RooFit::Name("xisignal"));
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
  double xiMuErr = w->var("mu")->getError();
  gxana::fit::JohnsonMoments xiMoments = gxana::fit::Moments(*w->var("mu"), *w->var("lambda"), *w->var("gamma"), *w->var("delta"));
  double xiMean = xiMoments.mean;
  double xiMass = xiMean + *correction;
  double xiMeanErr = xiMoments.meanErr;
      //xiMean * sqrt( pow( xiMuErr/xiMu, 2) + pow( xiLambdaErr/ xiLambda, 2) + pow( xiDelta/ xiDeltaErr, 2) + pow( xiGammaErr/xiGamma, 2) );
  double xiSigma = xiMoments.sigma;
  double xiSigmaErr = xiMoments.sigmaErr;
  printf("Mean and Sigma: %f +/- %f, %f  +/- %f\n", xiMean, xiMeanErr, xiSigma, xiSigmaErr);
  printf("FITRESULT mass_data %s mean=%.8f mean_err=%.8f sigma=%.8f yield=%.4f yield_err=%.4f mass=%.8f chi2ndf=%.6f\n", hist->GetName(), xiMean, xiMeanErr, xiSigma, yield, yield_err, xiMean + *correction, chiSqNdf);
  //Get FWHM
  auto fwhm = xiSigma*2.35482004503;
  std::cout << "FWHM: " << fwhm << std::endl;
  
  printf("Yield for kpkpxim hist: %s\n", hist->GetName());
  printf("%f +/- %f\n", yield, yield_err);

  auto legend = new TLegend(0.145,0.75,0.35,0.925); //top right corner
  legend->SetTextSize(0.055);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  legend->AddEntry(massframe->findObject("data") ,"Data","lep");
  legend->AddEntry(massframe->findObject("model"),"Fit","l");
  legend->AddEntry(massframe->findObject("xisignal"),"Signal","f");
 
  TLatex latex;
  char str[100]; char str_fwhm[100];
  char str_mean[100]; char str_mass[100];
  char str_sigma[100];
  char str_chisq[100];
  std::sprintf(str, "#color[4]{N_{#Xi^{-}} = %.0f #pm %.0f}", yield, yield_err);
  printf("Mu: %f +/- %f\n", xiMean*1e3, xiMuErr*1e3);
  std::sprintf(str_mean, "#color[4]{Mean = %.2f MeV}", xiMean*1e3);
  std::sprintf(str_mass, "#color[4]{M_{#Xi^{-}} = %.2f MeV}", xiMass*1e3);
  std::sprintf(str_sigma, "#color[4]{#sigma_{#Xi^{-}} = %.2f MeV}", xiSigma*1e3);
  std::sprintf(str_chisq, "#color[4]{#chi^{2}_{#nu} = %.3f}",chiSqNdf );
  std::sprintf(str_fwhm, "#color[4]{FWHM = %.3f MeV}",fwhm*1e3 );
  latex.SetTextSize(0.057);
  //latex.DrawLatex(1.352, maxY, "GlueX#lower[-0.15]{-}#kern[0.2]{I}");
  latex.DrawLatex(1.352, maxY, str);
  latex.DrawLatex(1.352, maxY*0.9, str_mean);
  latex.DrawLatex(1.352, maxY*0.8, str_mass);
  latex.DrawLatex(1.352, maxY*0.7, str_sigma);
  latex.DrawLatex(1.352, maxY*0.6, str_fwhm);
  latex.DrawLatex(1.352, maxY*0.5, str_chisq);
  
  double left_2sigma = xiMean-2*xiSigma;
  double right_2sigma = xiMean+2*xiSigma;
  TLine* left = new TLine(left_2sigma,0,left_2sigma,hist->GetMaximum()/2);
  left->SetLineColor(kAzure); left->SetLineWidth(2); left->SetLineStyle(kDashed);
  left->Draw();
  TLine* right = new TLine(right_2sigma,0,right_2sigma,hist->GetMaximum()/2);
  right->SetLineColor(kAzure); right->SetLineWidth(2); right->SetLineStyle(kDashed);
  right->Draw();
  
  legend->Draw("same");
  //fitCan->SetGrid();
  fitCan->SaveAs( (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/prod_plots/XimIM_dataCorr_")+delim+".pdf").c_str());
  fitCan->Write();
}

void RooFitHistMC(TH1* hist, const char* histTitle, std::string delim, double *correction, std::vector<double> &params)
{
    RooWorkspace* w = new RooWorkspace(histTitle);
    RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.285, 1.38);
    RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
    RooPlot* massframe = mass.frame(RooFit::Title(histTitle));
    TCanvas *fitCan = new TCanvas("fitCanMC"," mc", 800, 700);
    double thrown_mass = 1.32171; double maxY = hist->GetMaximum();
  
    w->import(RooArgSet(mass));
  
    //Build model and Fit data
    // Build model with initial parameters from params vector
    gxana::fit::BuildModel(*w, {
        gxana::fit::Johnson("xisignal", "mass", {"mu", gxana::fit::Fx(params[0]) + ",1.32,1.33"},
                            {"lambda", gxana::fit::Fx(params[1]) + ",0.002,0.007"},
                            {"gamma", gxana::fit::Fx(params[2]) + ", -1,1"}, {"delta", gxana::fit::Fx(params[3]) + ",0.2,5."}),
        gxana::fit::Threshold("bkgd", "mass", {"m0", "1.2602,1.255,1.275"}, {"b", "-22,-40.,-5."}, {"p", "2"}),
        //Create model and fit to data
        gxana::fit::Sum("model", {{{"nxi", "10000,1,1e6"}, "xisignal"}, {{"nbkgd", "2000,1,1e6"}, "bkgd"}})});//nbkgd[200,1,1e6]*bkgd,
    gxana::fit::RunFit(*w->pdf("model"), *data, RooFit::Extended(true), RooFit::PrintLevel(-1),
                       RooFit::PrintEvalErrors(-1), RooFit::Verbose(false), RooFit::Warnings(false));
    //Plot model and data 
    data->plotOn(massframe, RooFit::Name("data"));
    //w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.51, 0.95, 0.92) );
    w->pdf("model")->plotOn(massframe, RooFit::LineWidth(3), RooFit::Name("model"));
    w->pdf("xisignal")->plotOn(massframe,RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(3001),RooFit::MoveToBack(), RooFit::Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), RooFit::Name("xisignal"));
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
    double xiMuErr = w->var("mu")->getError();
    gxana::fit::JohnsonMoments xiMoments = gxana::fit::Moments(*w->var("mu"), *w->var("lambda"), *w->var("gamma"), *w->var("delta"));
    double xiMean = xiMoments.mean;
    double xiMeanErr = xiMoments.meanErr;
    //        xiMean * sqrt( pow( xiMuErr/xiMu, 2) + pow( xiLambdaErr/ xiLambda, 2) + pow( xiDelta/ xiDeltaErr, 2) + pow( xiGammaErr/xiGamma, 2) );
    double xiSigma = xiMoments.sigma;
    double xiSigmaErr = xiMoments.sigmaErr;
    printf("Mean and Sigma: %f +/- %f, %f  +/- %f\n", xiMean, xiMeanErr, xiSigma, xiSigmaErr);
    printf("FITRESULT mass_mc %s mean=%.8f mean_err=%.8f sigma=%.8f yield=%.4f yield_err=%.4f correction=%.8f chi2ndf=%.6f\n", hist->GetName(), xiMean, xiMeanErr, xiSigma, yield, yield_err, thrown_mass - xiMean, chiSqNdf);
    //Get FWHM
    auto fwhm = xiSigma*2.35482004503;
    std::cout << "FWHM: " << fwhm << std::endl;
    //correction to data
    *correction = thrown_mass - xiMean;
        
    printf("Yield for kpkpxim hist: %s\n", hist->GetName());
    printf("%f +/- %f\n", yield, yield_err);
    
    auto legend = new TLegend(0.145,0.75,0.35,0.925); //top right corner
    legend->SetTextSize(0.055);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    
    legend->AddEntry(massframe->findObject("data") ,"Data","lep");
    legend->AddEntry(massframe->findObject("model"),"Fit","l");
    legend->AddEntry(massframe->findObject("xisignal"),"Signal","f");

    TLatex latex;
    char str[100]; char str_fwhm[100];
    char str_mean[100]; char str_corr[100];
    char str_sigma[100];
    char str_chisq[100];
    std::sprintf(str, "#color[4]{N_{#Xi^{-}} = %.0f #pm %.0f}", yield, yield_err);
    printf("Mu: %f +/- %f\n", xiMean*1e3, xiMuErr*1e3);
    std::sprintf(str_mean, "#color[4]{M_{#Xi^{-}} = %.2f MeV}", xiMean*1e3);
    std::sprintf(str_corr, "#color[4]{Correction = %.2f MeV}", *correction*1e3);
    std::sprintf(str_sigma, "#color[4]{#sigma_{#Xi^{-}} = %.2f MeV}", xiSigma*1e3);
    std::sprintf(str_chisq, "#color[4]{#chi^{2}_{#nu} = %.3f}",chiSqNdf );
    std::sprintf(str_fwhm, "#color[4]{FWHM = %.3f MeV}",fwhm*1e3 );
    latex.SetTextSize(0.057);
    //latex.DrawLatex(1.33, maxY, "GlueX#lower[-0.15]{-}#kern[0.2]{I}");
    latex.DrawLatex(1.33, maxY, str);
    latex.DrawLatex(1.33, maxY*0.9, str_mean);
    latex.DrawLatex(1.33, maxY*0.8, str_corr);
    latex.DrawLatex(1.33, maxY*0.7, str_sigma);
    latex.DrawLatex(1.33, maxY*0.6, str_fwhm);
    latex.DrawLatex(1.33, maxY*0.5, str_chisq);
  
    double left_2sigma = xiMean-2*xiSigma;
    double right_2sigma = xiMean+2*xiSigma;
    TLine* left = new TLine(left_2sigma,0,left_2sigma,hist->GetMaximum()/2);
    left->SetLineColor(kAzure); left->SetLineWidth(2); left->SetLineStyle(kDashed);
    left->Draw();
    TLine* right = new TLine(right_2sigma,0,right_2sigma,hist->GetMaximum()/2);
    right->SetLineColor(kAzure); right->SetLineWidth(2); right->SetLineStyle(kDashed);
    right->Draw();
    TLine* massT = new TLine(1.32171,0,1.32171,hist->GetMaximum());
    massT->SetLineColor(kRed+1); massT->SetLineWidth(4);
    massT->Draw();
    
    legend->Draw("same");
    //fitCan->SetGrid();
    fitCan->SaveAs( (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/prod_plots/XimIM_mc_")+delim+".pdf").c_str());
    fitCan->Write();
}
void GetMassAnalysis(std::vector<TH1D*> hists, std::string name)
{
    double pdg_mass = 1.32171;// mass_err=0.00007
    double thrown_mass = hists[2]->GetBinCenter(hists[2]->GetMaximumBin());
    double mass_correction;
    vector<double> xiParams = {1.3217,0.004,-0.01,1.2};
    double binWidth = hists[0]->GetXaxis()->GetBinWidth(1);
    char histTitle[100];
    sprintf(histTitle, " ;M(#Lambda#pi^{-}) (GeV); Events / %.2f MeV", binWidth*1000);
    cout << "Thrown Value: " << thrown_mass << " GeV" << endl;

    RooFitHistMC(hists[1], histTitle, name, &mass_correction, xiParams);
    RooFitHist(hists[0], histTitle, name, &mass_correction, xiParams);
}

int FitMass(int n_threads = 4)
{
    // The original fitted in the same process as the histogram filling, with implicit
    // multithreading on; give the fits the same process state.
    if (n_threads > 0) ROOT::EnableImplicitMT(n_threads);
    setStyle();
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    TFile* f = XimOpen("xim_mass.root", "UPDATE");
    for (const auto& p : XimPeriods()) {
        auto get = [&](const char* n) {
            auto h = (TH1D*)f->Get((p.dir + "/" + n).c_str());
            if (!h) throw std::runtime_error("xim_mass.root: missing " + p.dir + "/" + n);
            return h;
        };
        f->cd(p.dir.c_str());
        GetMassAnalysis({get("cascade_mass"), get("cascade_mass_mc"), get("cascade_mass_thrown")}, p.stem);
    }
    f->Close();
    return 0;
}
