#include "gxana/common/Paths.h"
void RooFitHist(TH1* hist, const char* histTitle, string delim);
void setStyle();
void GetXim1320_IM(string delim="_allCuts", int n_threads = 8);
  
int MakeXim1320_IM()
{
  setStyle();
  GetXim1320_IM("_ximVertexCut");
  //GetXim1320_IM();
  GetXim1320_IM("_kphighrap");
  //GetXim1320_IM("_rapidityCuts");
  
  return 0;
}

void GetXim1320_IM(string delim="_allCuts", int n_threads = 4) {
	// Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
    // Initialize variables
    string tree_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
	// Branches you want to use get
    std::vector<std::string> branches = {"decayxim_M","hybrid_combo"};
    
	// make data frame
	// format : tree name, file name, branches to open
	auto df = ROOT::RDataFrame("flatTree_kpkpxim", (tree_dir+"flatTree_kpkpxim__M23_2017-01_ana56_nominal"+delim+".root").c_str(), branches);
    auto df1 = ROOT::RDataFrame("flatTree_kpkpxim", (tree_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+".root").c_str(), branches);
	auto df2 = ROOT::RDataFrame("flatTree_kpkpxim", (tree_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+".root").c_str(), branches);

    // make histos and merge
    auto h = df.Histo1D({""," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 60,1.25,1.47}, "decayxim_M", "hybrid_combo");
    auto h1 = df1.Histo1D({""," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 60,1.25,1.47}, "decayxim_M", "hybrid_combo");
    auto h2 = df2.Histo1D({""," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 60,1.25,1.47}, "decayxim_M", "hybrid_combo");
    double binWidth = h->GetXaxis()->GetBinWidth(1);
    char histTitle[100];
    sprintf(histTitle, " ;M(#Lambda#pi^{-}) (GeV); Events / %.2f MeV", binWidth*1000);
    //add histos to list 
    TList *list = new TList;
    list->Add(h.GetPtr());
    list->Add(h1.GetPtr());
    list->Add(h2.GetPtr());
    //merge histograms and fit
    TH1F *hist_merged = (TH1F*)h1->Clone("XiMinus_PhaseI");
    hist_merged->Reset();
    hist_merged->Merge(list);
    gStyle->SetOptFit(1);
    hist_merged->GetXaxis()->SetTitleOffset(0.92);
    hist_merged->GetYaxis()->SetTitleOffset(0.73);
    //hist_merged->GetYaxis()->SetNdivisions(-5, kFALSE);
    hist_merged->GetYaxis()->SetMaxDigits(3);
    
    RooFitHist(hist_merged, histTitle, delim);
}
 
void RooFitHist(TH1* hist, const char* histTitle, string delim)
{
  RooWorkspace* w = new RooWorkspace(histTitle);
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.45);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  RooPlot* massframe = mass.frame(RooFit::Title(histTitle));
  TCanvas *fitCan = new TCanvas("fitCan"," c", 800, 700);

  w->import(RooArgSet(mass));
  
  //Build model and Fit data
  w->factory("Chebychev::bkgd(mass,{a0[0.8,1.e-2,2],a1[-0.2,-2,-1e-2]})");//,a1[-0.1,-1.e2,-1e-2]
  w->factory("Gaussian::sigma(mass,mean[1.385,1.383,1.388],sig[0.0394,0.01,0.05])");
  //w->factory("Voigtian::sigma(mass,mean[1.387,1.383,1.39],width[0.0394,0.034,0.042], sig[0.007])");
  w->factory("Johnson::xigaus(mass,mu[1.32171,1.321,1.323],lambda[0.005,0.003,0.007], gamma[0,-1,1], delta[1.1,0.1,10])");
   
  //Create model and fit to data
  w->factory("SUM::model( nsigma[0,0,1e6]*sigma, nbkgd[1000,1,1e6]*bkgd, nxi[10000,1,1e5]*xigaus)");//nbkgd[200,1,1e6]*bkgd,
  w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));
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

  auto legend = new TLegend(0.145,0.75,0.35,0.925); //top right corner
  legend->SetTextSize(0.055);
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
  latex.SetTextSize(0.057);
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
  fitCan->SetGrid();
  fitCan->SaveAs( (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/prod_plots/Xim_InvariantMassFit_Phase1")+delim+".pdf").c_str());
  //fitCan->SaveAs( (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/plots/Xim_InvariantMassFit_Phase1")+delim+".pdf").c_str());
}

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
