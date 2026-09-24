#include "gxana/common/Paths.h"
void setStyle()
{
    //gStyle->SetCanvasPreferGL(true);
    gStyle->SetCanvasColor(0);
    gStyle->SetCanvasBorderSize(10);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasDefH(600);
    gStyle->SetCanvasDefW(700);

    gStyle->SetPadColor       (0);
    gStyle->SetPadBorderSize  (10);
    gStyle->SetPadBorderMode  (0);
    gStyle->SetPadBottomMargin(0.15);
    gStyle->SetPadTopMargin   (0.06);
    gStyle->SetPadLeftMargin  (0.12);
    gStyle->SetPadRightMargin (0.05);
    gStyle->SetPadGridX       (0);
    gStyle->SetPadGridY       (0);
    gStyle->SetPadTickX       (0);
    gStyle->SetPadTickY       (0);

    gStyle->SetFrameFillStyle ( 0);
    gStyle->SetFrameFillColor ( 0);
    gStyle->SetFrameLineColor ( 1);
    gStyle->SetFrameLineStyle ( 0);
    gStyle->SetFrameLineWidth ( 1);
    gStyle->SetFrameBorderSize(10);
    gStyle->SetFrameBorderMode( 0);

    gStyle->SetNdivisions(505);

    gStyle->SetLineWidth(2);
    gStyle->SetHistLineWidth(2);
    gStyle->SetFrameLineWidth(2);
    //gStyle->SetLegendFillColor(1);
  
    gStyle->SetLegendBorderSize(0);
    gStyle->SetLegendFont(132);
    gStyle->SetLegendTextSize(0.06);
    gStyle->SetMarkerSize(1.2);
    gStyle->SetMarkerStyle(20);

    gStyle->SetLabelSize(0.055,"X");
    gStyle->SetLabelSize(0.055,"Y");

    gStyle->SetLabelOffset(0.010,"X");
    gStyle->SetLabelOffset(0.010,"Y");

    gStyle->SetLabelFont(132,"X");
    gStyle->SetLabelFont(132,"Y");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetTitleFont(132);
    gStyle->SetTitleFont(132,"X");
    gStyle->SetTitleFont(132,"Y");

    gStyle->SetTitleSize(0.08,"X");
    gStyle->SetTitleSize(0.08,"Y");

    gStyle->SetTitleOffset(0.9,"X");
    gStyle->SetTitleOffset(0.65,"Y");

    gStyle->SetTextSize(0.08);
    gStyle->SetTextFont(132);

    gStyle->SetOptStat(0);

    gROOT->ForceStyle();

    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextFont(132);
    latex->SetTextSize(0.08);
    latex->SetTextAlign(32);
    gROOT->ForceStyle();
}

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
  w->factory("Chebychev::bkgd(mass,{a0[0.8,1.e-2,2],a1[-0.2,-2,-1e-2]})");//,a1[-0.1,-1.e2,-1e-2]
  w->factory("Gaussian::sigma(mass,mean[1.385,1.383,1.388],sig[0.0394,0.01,0.05])");
  //w->factory("Voigtian::sigma(mass,mean[1.387,1.383,1.39],width[0.0394,0.034,0.042], sig[0.007])");
 w->factory(Form("Johnson::xisignal(mass, mu[%f,1.32,1.33], lambda[%f,%f,0.008], gamma[%f], delta[%f])",params[0], params[1], params[1], params[2], params[3]));   
  //Create model and fit to data
  w->factory("SUM::model( nsigma[0,1,1e6]*sigma, nbkgd[1000,1,1e6]*bkgd, nxi[10000,1,1e6]*xisignal)");//nbkgd[200,1,1e6]*bkgd,
  w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));
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
  double xiMu = w->var("mu")->getVal(); double xiMuErr = w->var("mu")->getError();
  double xiLambda = w->var("lambda")->getVal(); double xiLambdaErr = w->var("lambda")->getError();
  double xiDelta = w->var("delta")->getVal(); double xiDeltaErr = w->var("delta")->getError();
  double xiGamma = w->var("gamma")->getVal(); double xiGammaErr = w->var("gamma")->getError();
  double xiMean = xiMu - xiLambda * exp(1 / (2*pow(xiDelta,2)) ) * sinh(xiGamma/xiDelta);
  double xiMass = xiMean + *correction; 
  double xiMeanErr = sqrt( pow(xiMuErr,2)
                           + pow( -1* xiLambdaErr*exp(1 / (2*pow(xiDelta,2)))*sinh(xiGamma/xiDelta),2 )
                           + pow( -1* xiGammaErr* xiLambda *exp(1 / (2*pow(xiDelta,2)))*cosh(xiGamma/xiDelta) / xiDelta,2)
                           + pow(xiDeltaErr*xiLambda*exp(1 / (2*pow(xiDelta,2)))*(sinh(xiGamma/xiDelta) + xiGamma*xiDelta*cosh(xiGamma/xiDelta)) / pow(xiDelta,3) ,2));
      //xiMean * sqrt( pow( xiMuErr/xiMu, 2) + pow( xiLambdaErr/ xiLambda, 2) + pow( xiDelta/ xiDeltaErr, 2) + pow( xiGammaErr/xiGamma, 2) ); 
  double xiSigma = sqrt( pow(xiLambda, 2)/2*(exp(pow(xiDelta, -2) ) - 1 )*(exp(pow(xiDelta, -2) )*cosh(2*xiGamma/xiDelta )+1));
  double xiSigmaErr = xiSigma * sqrt( pow( xiDelta/ xiDeltaErr, 2) + pow( xiGammaErr/xiGamma, 2) );
  printf("Mean and Sigma: %f +/- %f, %f  +/- %f\n", xiMean, xiMeanErr, xiSigma, xiSigmaErr);
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
    w->factory(Form("Johnson::xisignal(mass, mu[%f,1.32,1.33], lambda[%f,0.002,0.007], gamma[%f, -1,1], delta[%f,0.2,5.])", params[0], params[1], params[2], params[3]));
    w->factory("EXPR::bkgd('(mass)*(((mass)/m0)**2-1.0)**p*exp(b*(((mass)/m0)**2-1.0))',mass, m0[1.2602,1.255,1.275], b[-22,-40.,-5.], p[2])");
    //Create model and fit to data

    w->factory("SUM::model(nxi[10000,1,1e6]*xisignal, nbkgd[2000,1,1e6]*bkgd)");//nbkgd[200,1,1e6]*bkgd,
    w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));
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
    double xiMu = w->var("mu")->getVal(); double xiMuErr = w->var("mu")->getError();
    double xiLambda = w->var("lambda")->getVal(); double xiLambdaErr = w->var("lambda")->getError();
    double xiDelta = w->var("delta")->getVal(); double xiDeltaErr = w->var("delta")->getError();
    double xiGamma = w->var("gamma")->getVal(); double xiGammaErr = w->var("gamma")->getError();
    double xiMean = xiMu - xiLambda * exp(1 / (2*pow(xiDelta,2)) ) * sinh(xiGamma/xiDelta);
    double xiMeanErr = sqrt( pow(xiMuErr,2)
                           + pow( -1* xiLambdaErr*exp(1 / (2*pow(xiDelta,2)))*sinh(xiGamma/xiDelta),2 )
                           + pow( -1* xiGammaErr* xiLambda *exp(1 / (2*pow(xiDelta,2)))*cosh(xiGamma/xiDelta) / xiDelta,2)
                           + pow(xiDeltaErr*xiLambda*exp(1 / (2*pow(xiDelta,2)))*(sinh(xiGamma/xiDelta) + xiGamma*xiDelta*cosh(xiGamma/xiDelta)) / pow(xiDelta,3) ,2));
    //        xiMean * sqrt( pow( xiMuErr/xiMu, 2) + pow( xiLambdaErr/ xiLambda, 2) + pow( xiDelta/ xiDeltaErr, 2) + pow( xiGammaErr/xiGamma, 2) ); 
    double xiSigma = sqrt( pow(xiLambda, 2)/2*(exp(pow(xiDelta, -2) ) - 1 )*(exp(pow(xiDelta, -2) )*cosh(2*xiGamma/xiDelta )+1));
    double xiSigmaErr = xiSigma * sqrt( pow( xiDelta/ xiDeltaErr, 2) + pow( xiGammaErr/xiGamma, 2) );
    printf("Mean and Sigma: %f +/- %f, %f  +/- %f\n", xiMean, xiMeanErr, xiSigma, xiSigmaErr);
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

TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon)
{
    TH1D* hist_accept = (TH1D*)hist_recon->Clone();
    hist_accept->GetYaxis()->SetTitle("Acceptance, #epsilon");
    hist_accept->Divide(hist_accept,hist_genr,1,1,"B");
    
    printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
    return hist_accept;
}

//`vect_hist` [0](data),[1](recon),[2](generated)
TH1D* GetAcceptanceCorrHist1D(std::vector<TH1D*> vec_hist, TH1D* hist_accept)
{
    //get the acceptance
    if(hist_accept->GetEntries()==0)
        hist_accept = (TH1D*)GetAcceptanceHist1D(vec_hist[2],vec_hist[1])->Clone();
    
    char newName[150];
    double binwidth = vec_hist[0]->GetBinWidth(1);
    sprintf(newName,"Events/#epsilon / %.3f GeV", binwidth);
  
    vec_hist[0]->GetYaxis()->SetTitle(newName);
    TH1D* hist_data_acccorr = (TH1D*)vec_hist[0]->Clone( ((string)vec_hist[0]->GetName()+"_accCorr").c_str());
    //hist_data_acccorr->Divide(vec_hist[0],hist_accept,1,1,"B");
    hist_data_acccorr->Divide(hist_accept);
    hist_data_acccorr->Write(hist_data_acccorr->GetName(),TObject::kOverwrite);
    hist_accept->Write( ((string)vec_hist[0]->GetName()+"_accept").c_str(),TObject::kOverwrite);
  
    return hist_data_acccorr;
}

void GetLifetimeAnalysis(std::vector<TH1D*> hists, std::string name)
{
    double pdg_lifetime = 0.1639;//(ns) lifetime_err=0.0015
    double thrown_lifetime = hists[2]->GetMean();
    double mc_lifetime = hists[1]->GetMean();
    double correction = pdg_lifetime - mc_lifetime;
    double binWidth = hists[0]->GetXaxis()->GetBinWidth(1);
    char histTitle[100];
    sprintf(histTitle, " ;#Xi^{-} Lifetime; arb. units  / %.3f ns", binWidth);
    cout << "Thrown Value: " << thrown_lifetime << " ns" << endl;

    //
    cout << "Thrown Mean: " << hists[2]->GetMean() << endl;
    cout << "MC Mean: " << hists[1]->GetMean() << endl;

    TH1D* acceptance = (TH1D*)GetAcceptanceHist1D(hists[2],hists[1]);
    TH1D* accepted_hist = (TH1D*)GetAcceptanceCorrHist1D(hists, acceptance)->Clone();
    acceptance->SetTitle(" ; ; acceptance, #varepsilon");
    acceptance->GetYaxis()->RotateTitle(true);
    acceptance->SetMarkerStyle(24);
    acceptance->SetMarkerSize(1.1);
    acceptance->SetMarkerColor(kRed+1);
    acceptance->SetLineColor(kRed+1);
    acceptance->SetLineWidth(2);
    acceptance->SetFillStyle(0);  
    acceptance->GetYaxis()->SetAxisColor(kRed+1);
    acceptance->GetYaxis()->SetTitleColor(kRed+1);
    acceptance->GetYaxis()->SetLabelColor(kRed+1);
    acceptance->GetYaxis()->SetMaxDigits(2);
    acceptance->GetYaxis()->SetNdivisions(505);
    //acceptance->GetYaxis()->CenterTitle(true);
    acceptance->GetYaxis()->SetTitleOffset(0.9);
       
    accepted_hist->GetYaxis()->SetMaxDigits(3);
    accepted_hist->GetYaxis()->SetNdivisions(505);
    accepted_hist->SetTitle(histTitle);
    
    TF1* fit = new TF1("fit","expo",0.02,0.7);
    fit->SetLineColor(kAzure);
    fit->SetLineWidth(4);
    TCanvas *c = new TCanvas("c",name.c_str());
    TPad *pad1 = new TPad((name+"_Pad1").c_str(),"",0,0,1,1);
    TPad *pad2 = new TPad((name+"_Pad2").c_str(),"",0,0,1,1);
    pad2->SetFillStyle(4000); //will be transparent
    pad2->SetFrameFillStyle(0);
    pad1->SetRightMargin(0.15);
    pad2->SetRightMargin(0.15);
    //pad1->SetLogx();
    //pad2->SetLogx();
    //pad2->SetFrameLineColor(kRed+1);
    //pad2->GetFrame()->SetLineWidth(10);
    pad1->Draw();
    pad1->cd();
    accepted_hist->Draw("PL");
    accepted_hist->Fit("fit","WLR");
    double slope = -1/fit->GetParameter(1);
    double chisqndf = fit->GetChisquare() / fit->GetNDF();
    TLatex l;
    l.SetTextSize(0.06);
    l.DrawLatex(0.45,accepted_hist->GetMaximum(),Form("#tau_{#Xi} = %.4f ns",slope));
    //l.DrawLatex(0.3,accepted_hist->GetMaximum()*0.85,Form("Correction = %.4f ns",correction));
    l.DrawLatex(0.45,accepted_hist->GetMaximum()*0.85,Form("#chi^{2}_{#nu} = %.2f",chisqndf));
    
    pad2->Draw();
    pad2->cd();
    acceptance->GetYaxis()->RotateTitle(true);
    acceptance->Draw("histY+");
    
    string plotDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/prod_plots/");
    c->SaveAs( (plotDir+"accepted_ximlifetime_"+name+".pdf").c_str());    
    delete c, pad1, pad2;
}

void GetSpinAnalysis(std::vector<TH1D*> hists, std::string name)
{
    double binWidth = hists[0]->GetXaxis()->GetBinWidth(1);
    char histTitle[100];
    sprintf(histTitle, " ;cos#vartheta^{#pi^{-}}_{#it{h}} ; arb. units  / %.3f ", binWidth);
    TH1D *acceptance, *accepted_hist;

    if(hists.size()>2){
        acceptance = (TH1D*)GetAcceptanceHist1D(hists[2],hists[1]);    
        accepted_hist = (TH1D*)GetAcceptanceCorrHist1D(hists, acceptance)->Clone();
    }
    else{
        acceptance = (TH1D*)hists[1]->Clone();
        accepted_hist = (TH1D*)hists[0]->Clone();
    }
        
    acceptance->SetTitle(" ; ; acceptance, #varepsilon");
    acceptance->SetMarkerStyle(24);
    acceptance->SetMarkerSize(1.1);
    acceptance->SetMarkerColor(kRed+1);
    acceptance->SetLineColor(kRed+1);
    acceptance->SetLineWidth(2);
    acceptance->SetFillStyle(0);  
    acceptance->GetYaxis()->SetAxisColor(kRed+1);
    acceptance->GetYaxis()->SetTitleColor(kRed+1);
    acceptance->GetYaxis()->SetLabelColor(kRed+1);
    acceptance->GetYaxis()->SetMaxDigits(2);
    acceptance->GetYaxis()->SetNdivisions(505);
    //acceptance->GetYaxis()->CenterTitle(true);
    acceptance->GetYaxis()->SetTitleOffset(0.9);

    accepted_hist->GetYaxis()->SetTitleOffset(0.9);
    accepted_hist->GetYaxis()->SetMaxDigits(3);
    accepted_hist->GetYaxis()->SetNdivisions(505);
    accepted_hist->SetTitle(histTitle);

    TCanvas *c = new TCanvas("c",name.c_str());
    TPad *pad1 = new TPad((name+"_Pad1").c_str(),"",0,0,1,1);
    TPad *pad2 = new TPad((name+"_Pad2").c_str(),"",0,0,1,1);
    pad2->SetFillStyle(4000); //will be transparent
    pad2->SetFrameFillStyle(0);
    pad1->SetRightMargin(0.15);
    pad2->SetRightMargin(0.15);
    pad1->SetBottomMargin(0.17);
    pad2->SetBottomMargin(0.17);
    pad1->SetLeftMargin(0.15);
    pad2->SetLeftMargin(0.15);
    pad1->SetLogy();
    //pad2->SetLogy();
    //pad2->SetFrameLineColor(kRed+1);
    //pad2->GetFrame()->SetLineWidth(10);
    pad1->Draw();
    pad1->cd();
    accepted_hist->Draw("PL");

    TF1* fit = new TF1("fit","pol0",-1,1);
    fit->SetLineColor(kAzure);
    fit->SetLineWidth(4);
    fit->SetLineStyle(kDashed);
    accepted_hist->Fit("fit","WLR");
    
    TF1* fit_beta = new TF1("fit_beta","[Const]*(1+[Beta]*x)",-1,1);
    fit_beta->SetParameter(1,fit->GetParameter(0));
    fit_beta->SetParLimits(0,0,1);
    fit_beta->SetLineColor(kAzure);
    fit_beta->SetLineWidth(4);    
    accepted_hist->Fit("fit_beta","WLR+");

    double beta = fit_beta->GetParameter(0);
    double beta_err = fit_beta->GetParError(0);

    TF1* fit1 = new TF1("fit1","[C]*(1+3*x^2)",-1,1);
    fit1->SetLineColor(kMagenta);
    fit1->SetLineWidth(4);
    fit1->SetLineStyle(kDashed);
    
    TF1* fit1_beta = new TF1("fit1_beta","[C]*(1 + 3*x^2 + [B]*x*(5-9*x^2))",-1,1);
    fit1_beta->FixParameter(0, beta);
    fit1_beta->SetParError(0, beta_err);
    fit1_beta->SetLineColor(kMagenta);
    fit1_beta->SetLineWidth(4);
    accepted_hist->Fit("fit1","WLR+");
    accepted_hist->Fit("fit1_beta","BWLR+");
    double chisqndf = fit->GetChisquare() / fit->GetNDF();
    double chisqndf_beta = fit_beta->GetChisquare() / fit_beta->GetNDF();
    double chisqndf1 = fit1_beta->GetChisquare() / fit1_beta->GetNDF();
    TLatex l;
    l.SetTextSize(0.055);
    l.DrawLatex(0.35,accepted_hist->GetMinimum()*0.85,Form("#bf{#color[4]{#chi^{2}_{#nu} = %.2f}}",chisqndf_beta));
    l.DrawLatex(0.35,accepted_hist->GetMinimum()*0.63,Form("#color[4]{#chi^{2}_{#nu} = %.2f}",chisqndf));
    l.DrawLatex(0.35,accepted_hist->GetMinimum()*0.46,Form("#color[6]{#chi^{2}_{#nu} = %.2f}",chisqndf1));
    
    pad2->Draw();
    pad2->cd();
    acceptance->GetYaxis()->RotateTitle(true);
    acceptance->Draw("histY+");

    cout << "Beta= " << beta << endl;
    string plotDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/prod_plots/");
    c->SaveAs( (plotDir+"accepted_pimcostheta_"+name+".pdf").c_str());    
    //delete c, pad1, pad2, acceptance, accepted_hist;
}

void GetXimHistos( std::string rootName, std::string delim="kphighrap", int n_threads=4 ) {
	// Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
    // Initialize variables
    std::string tree_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
	// Branches you want to use get
    std::vector<std::string> branches = {"decayxim_M","hybrid_combo"};
    
	// make data frame
	// format : tree name, file name, branches to open
	auto df = ROOT::RDataFrame("flatTree_kpkpxim", (tree_dir+"flatTree_"+rootName+"_nominal_"+delim+".root").c_str());
    auto df1 = ROOT::RDataFrame("flatTree_kpkpxim", gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/")+rootName+"_nominal_kphighrap_1111111/postQVal_flatTree_"+rootName+"_nominal_kphighrap_1111111.root")
        .Define("qvalue_acc","hybrid_combo*qvalue_decayxim_M");
    auto dfmc = ROOT::RDataFrame("flatTree_kpkpxim", (tree_dir+"flatTree_"+rootName+"_gen_amp_V2_ac_YstarRest_nominal_"+delim+".root").c_str());
	auto dfT = ROOT::RDataFrame("flatTree_thrown_kpkpxim", (tree_dir+"flatTree_thrown_"+rootName+"_gen_amp_V2_ac_YstarRest.root").c_str())
        .Define("decayxim_M","decayxim_p4.M()");

    // make histos and merge
    auto mass_data = df.Histo1D({"cascade_mass"," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 60,1.25,1.47}, "decayxim_M", "hybrid_combo");
    auto mass_mc = dfmc.Histo1D({"cascade_mass_mc"," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 60,1.25,1.47}, "decayxim_M", "hybrid_combo");
    auto mass_t = dfT.Histo1D({"cascade_mass_thrown"," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 60,1.25,1.47}, "decayxim_M");

    mass_data->Write(mass_data->GetName(), TObject::kOverwrite);
    mass_mc->Write(mass_mc->GetName(), TObject::kOverwrite);
    mass_t->Write(mass_t->GetName(), TObject::kOverwrite);
    
    GetMassAnalysis({mass_data.GetPtr(),mass_mc.GetPtr(),mass_t.GetPtr()}, rootName);

    // xim lifetime
    auto lifetime_data = df1.Histo1D({"cascade_lifetime"," ;#tau_{#Xi^{-}} (ns); arb. units", 60,0.02,0.8}, "xim_lifetime_restframe", "qvalue_acc");        
    auto lifetime_mc = dfmc.Histo1D({"cascade_lifetime_mc"," ;#tau_{#Xi^{-}} (ns); arb. units", 60,0.02,0.8}, "xim_lifetime_restframe", "hybrid_combo");
    auto lifetime_t = dfT.Histo1D({"cascade_lifetime_thrown"," ;#tau_{#Xi^{-}} (ns); arb. units", 60,0.02,0.8}, "xim_lifetime_restframe");

    lifetime_data->Write(lifetime_data->GetName(), TObject::kOverwrite);
    lifetime_mc->Write(lifetime_mc->GetName(), TObject::kOverwrite);
    lifetime_t->Write(lifetime_t->GetName(), TObject::kOverwrite);

    GetLifetimeAnalysis({lifetime_data.GetPtr(), lifetime_mc.GetPtr(), lifetime_t.GetPtr()}, rootName);
    
    //Xim spin correction
    auto pim_data = df1.Histo1D({"piminus_costheta_hf"," ; cos#vartheta^{#pi^{-}}_{#it{h}}; arb. units", 60,-1,1}, "pim1_costheta_hf", "qvalue_acc");
    auto pim_mc = dfmc.Histo1D({"piminus_costheta_hf_mc"," ; cos#vartheta^{#pi^{-}}_{#it{h}}; arb. units", 60,-1,1}, "pim1_costheta_hf", "hybrid_combo");
    auto pim_t = dfT.Histo1D({"piminus_costheta_hf_thrown"," ; cos#vartheta^{#pi^{-}}_{#it{h}}; arb. units", 60,-1,1}, "pim1_costheta_hf");

    pim_data->Write(pim_data->GetName(), TObject::kOverwrite);
    pim_mc->Write(pim_mc->GetName(), TObject::kOverwrite);
    pim_t->Write(pim_t->GetName(), TObject::kOverwrite);

    GetSpinAnalysis({pim_data.GetPtr(), pim_mc.GetPtr(), pim_t.GetPtr()}, rootName);
}

using namespace RooFit;

int GetXimProperties() {
    setStyle();
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    //Make root file to save histos
    vector< string> fileDir = {"Spring_2017", "Spring_2018", "Fall_2018"};
    TFile *f =  TFile::Open("xim1320_properties.root", "RECREATE");
    for(const auto& dir : fileDir){
        f->cd();
        gDirectory->mkdir(dir.c_str());
    }
    // Perform individual analysis
    f->cd("Spring_2017");
    GetXimHistos("kpkpxim__M23_2017-01_ana56");
    f->cd("Spring_2018");
    GetXimHistos("kpkpxim__B4_M23_2018-01_ana03");
    f->cd("Fall_2018");
    GetXimHistos("kpkpxim__B4_M23_2018-08_ana02");

    // Combine the 3 measurments after acceptance correction 
    f->ReOpen("READ");
    f->cd();
    TH1D* merged_spin = (TH1D*)f->Get( (fileDir[0]+"/piminus_costheta_hf_accCorr").c_str())->Clone();
    TH1D* merged_spin_accept = (TH1D*)f->Get( (fileDir[0]+"/piminus_costheta_hf_accept").c_str())->Clone();
    merged_spin->Draw();
    //Set histos up for merging (averaging for the efficiency)
    merged_spin->Sumw2();merged_spin_accept->SetBit(TH1::kIsAverage);
    //merge
    for(int j=1; j<fileDir.size();++j){
        merged_spin->Add( (TH1D*)f->Get( (fileDir[j]+"/piminus_costheta_hf_accCorr").c_str()));
        merged_spin_accept->Add( (TH1D*)f->Get( (fileDir[j]+"/piminus_costheta_hf_accept").c_str()));    
    }

    f->ReOpen("UPDATE");
    f->cd();
    merged_spin->Write("pim_costheta_hf_phase1",TObject::kOverwrite);
    merged_spin_accept->Write("pim_costheta_hf_avg_accept_phase1",TObject::kOverwrite);
    
    return 0;
}
