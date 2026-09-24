void RooFitHist(TTree* TreeData, const char* histTitle, string delim);
void setStyle();
void GetXim1320_IM(string delim="_allCuts", int n_threads = 8);
using namespace RooFit;

int TestUMLFits()
{
  setStyle();
  //  GetXim1320_IM("_vertexCuts");
  //GetXim1320_IM();
  GetXim1320_IM("_allKaonSep");
  
  return 0;
}

void GetXim1320_IM(string delim="_allCuts", int n_threads = 4) {
	// Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
    // Initialize variables
    string tree_dir = "/d/grid17/hjesse/AnalysisNote/xsection/prepped_ttrees/gen_amp_V2_2/";
	// Branches you want to use get
    std::vector<std::string> branches = {"decayxim_M"};
    
	// make data frame
	// format : tree name, file name, branches to open
    TFile* dataFile = TFile::Open((tree_dir+"kpkpxim__B4_M23_2018-08_ana02_nominal_allKaonSep_Emin-8.19_Emax-8.45_Tmin-0.92_Tmax-1.19.root").c_str(), "READ");
    TTree* dataTree =  (TTree*)dataFile->Get("flatTree_kpkpxim");

    char histTitle[100];
    sprintf(histTitle, " ;M(#Lambda#pi^{-}) (GeV); Events");

    
    RooFitHist(dataTree, histTitle, delim);
}
 
void RooFitHist(TTree* treeData, const char* histTitle, string delim)
{
  //Import dataset to plot
  RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.26, 1.48);
  //RooRealVar weight("mc_weight", "weight", 1e-4, 10);
  RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass), Import(*treeData));
  TH1* dataHist = (TH1*)data->createHistogram("decayxim_M");
  Double_t min_mass = dataHist->GetXaxis()->GetBinLowEdge(dataHist->FindFirstBinAbove(0,1,1, dataHist->FindBin(1.3)));
  mass.setRange("fitrange", 1.27, 1.47);
  //Set up workspace
  RooWorkspace* w = new RooWorkspace(histTitle);
  RooPlot* massframe = mass.frame(Title(histTitle));
  TCanvas* fitCan = new TCanvas("fitCan"," c", 800, 700);
  //fitCan->SetLogy();
  w->import(RooArgSet(mass));
    
  //Build model and Fit data
  w->factory("Chebychev::bkgd(decayxim_M,{a0[0.8,0.01,3.],a1[-0.2,-3,-0.01]})");//,a1[-0.1,-2,-1e-2]
  w->factory("Johnson::xisignal(decayxim_M,mu[1.3217,1.31,1.33],lambda[0.006,0.003,0.008], gamma[0], delta[1.2,1.,3.])");
  w->factory("SUM::model( nxi[100,1,1e6]*xisignal, nbkgd[100,1,1e6]*bkgd)");// nbkgd[200,1,1e6]*bkgd,
  w->pdf("model")->fitTo(*data, Extended(true),
                         SumW2Error(false),
						//RooFit::AsymptoticError(true), 
                         Hesse(false),
                         PrintLevel(-1),PrintEvalErrors(-1),Verbose(false),Warnings(false));
  
  //Plot model and data 
  data->plotOn(massframe, Name("datapnts"), Binning(50, 1.25, 1.48));
  w->pdf("model")->paramOn(massframe, Format("NE",AutoPrecision(1)), Layout(0.55, 0.95, 0.92), Range("fitrange"));//, Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nbkgd")))
  w->pdf("model")->plotOn(massframe, LineWidth(4), Name("model"), Range("fitrange"));
  w->pdf("xisignal")->plotOn(massframe,DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Name("xisignal"), Range("fitrange"));//
  w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("fitrange"));
  //set up specific frame styling
  massframe->GetYaxis()->SetMaxDigits(2);
  massframe->GetYaxis()->SetNdivisions(505, kFALSE);
  //massframe->SetMinimum(0.1);
  massframe->Draw();

  fitCan->SetGrid();
  //fitCan->SaveAs( ("/d/grid17/hjesse/AnalysisNote/analysis/plots/Xim_InvariantMassFit_Phase1"+delim+".pdf").c_str());
}

void RooFitHistMC(TTree* treeData, const char* histTitle, string delim)
{
  //Set up workspace
  RooWorkspace* w = new RooWorkspace(histTitle);
  RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.25, 1.42);
  
  RooRealVar weight("mc_weight", "weight", 1e-4, 10);
  // 
  RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
  TH1* dataHist = (TH1*)data->createHistogram("decayxim_M");
  Double_t min_mass = dataHist->GetXaxis()->GetBinLowEdge(dataHist->FindFirstBinAbove(0,1,1, dataHist->FindBin(1.3)));
  mass.setRange("signal", min_mass, 1.37);
  RooPlot* massframe = mass.frame(Title(histTitle));
  TCanvas* fitCan = new TCanvas("fitCan"," c", 800, 700);
  fitCan->SetLogy();
  w->import(RooArgSet(mass));
    
  //Build model and Fit data
  w->factory("EXPR::bkgd('(decayxim_M)*(((decayxim_M)/m0)**2-1.0)**p*exp(b*(((decayxim_M)/m0)**2-1.0))',decayxim_M, m0[1.2602,1.255,1.275], b[-22,-40.,-5.], p[2])");
  //w->factory("Gaussian::xisignal(decayxim_M,mean1[1.32,1.31,1.33], sigma[0.005,0.003,0.007])");
  w->factory("Johnson::xisignal(decayxim_M,mu[1.3217,1.32,1.33],lambda[0.0045,0.003,0.006], gamma[0], delta[1.2,0.2,3.])");
  w->factory("SUM::model( nxi[1000,1,1e6]*xisignal, nbkgd[100,1,1e6]*bkgd)");// nbkgd[200,1,1e6]*bkgd,
  w->pdf("model")->fitTo(*data,SumW2Error(true),PrintLevel(-1), Range("signal"));
  
  //Plot model and data 
  data->plotOn(massframe, Name("datapnts"));
  w->pdf("model")->paramOn(massframe, Format("NE",AutoPrecision(1)), Layout(0.55, 0.95, 0.92), Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nbkgd"))));
  w->pdf("model")->plotOn(massframe, LineWidth(3), Name("model"), Range("signal"));
  w->pdf("xisignal")->plotOn(massframe,DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Name("xisignal"), Range("signal"));//
  w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("signal"));
  //set up specific frame styling
  massframe->GetYaxis()->SetMaxDigits(2);
  massframe->GetYaxis()->SetNdivisions(505, kFALSE);
  massframe->SetMinimum(0.1);
  massframe->Draw();

  fitCan->SetGrid();
  //fitCan->SaveAs( ("/d/grid17/hjesse/AnalysisNote/analysis/plots/Xim_InvariantMassFit_Phase1"+delim+".pdf").c_str());
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
