void RooFitHist(TTree* TreeData, string histTitle, string delim,  vector<double> params);
void RooFitHistMC(TTree* treeData, string histTitle, string delim, vector<double> &params);
void setStyle();
void GetXim1320_IM(string filename);
using namespace RooFit;

int TestUMLFitsRF()
{
  
  GetXim1320_IM("kpkpxim__B4_M23_2018-01_ana03_nominal_tCut_Emin-7.40_Emax-7.86.root");
  GetXim1320_IM("kpkpxim__B4_M23_2018-01_ana03_nominal_rapidityCuts_Emin-7.40_Emax-7.86.root");
  
  return 0;
}

void GetXim1320_IM(string filename) {
    setStyle();
    // Initialize variables
    string tree_dir = "/d/grid17/hjesse/AnalysisNote/xsection/prepped_ttrees/gen_amp_V2_2D_ac/";
    vector<double> xiParams(3);
    // Branches you want to use get
    std::vector<std::string> branches = {"decayxim_M"};
    
	//make data frame
	//format : tree name, file name, branches to open
    // string filename = "kpkpxim__B4_M23_2018-08_ana02_nominal_allKaonSep_Emin-7.86_Emax-8.19_Tmin-1.19_Tmax-1.53.root";
    // string filename = "kpkpxim__B4_M23_2018-08_ana02_nominal_allKaonSep_Emin-8.45_Emax-8.68_Tmin-0.10_Tmax-0.35.root";
    // string filename = "kpkpxim__B4_M23_2018-08_ana02_nominal_allKaonSep_Emin-9.26_Emax-10.18_Tmin-0.92_Tmax-1.19.root";
    //filename = "kpkpxim__M23_2017-01_ana56_nominal_tCut_Emin-6.40_Emax-7.40_Tmin-0.10_Tmax-0.35.root";
    TFile* dataFile = TFile::Open((tree_dir+filename).c_str(), "READ");
    TFile* dataFileMC = TFile::Open((tree_dir+"recon_"+filename).c_str(), "READ");
    TTree* dataTree =  (TTree*)dataFile->Get("flatTree_kpkpxim");
    TTree* dataTreeMC =  (TTree*)dataFileMC->Get("flatTree_kpkpxim");
 
    //Fit Data and MC for yields
    string tmin = filename.substr(filename.find("Tmin")+5,4);
    string tmax = filename.substr(filename.find("Tmax")+5,4);
    string histTitle = "#bf{-t: ("+tmin+", "+tmax+")} ;M(#Lambda#pi^{-}) (GeV); Events";

    RooFitHistMC(dataTreeMC, histTitle, "recon_"+filename.substr(0,filename.find_last_of(".")), xiParams);
    cout << "MC params extracted (lambda, delta): (" << xiParams[0] << ", " << xiParams[1] << ")" << endl;
    RooFitHist(dataTree, histTitle, filename.substr(0,filename.find_last_of(".")), xiParams);
}
 
void RooFitHist(TTree* treeData, string histTitle, string delim, vector<double> params)
{
  gStyle->SetTitleAlign(33);
  gStyle->SetTitleX(.95);
  //Import dataset to plot
  RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.26, 1.46);
  RooRealVar weight("hybrid_combo", "weight", -10, 10);
  RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
  // RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass), Import(*treeData));
  TH1* dataHist = (TH1*)data->createHistogram("decayxim_M")->Clone(delim.c_str());
  Double_t min_mass = dataHist->GetXaxis()->GetBinLowEdge(dataHist->FindFirstBinAbove(0,1,1, dataHist->FindBin(1.32)));
  double max_mass=1.44;
  // if (min_mass < 1.3) 
  //   min_mass=1.298;
  while(dataHist->GetBinContent(dataHist->FindBin(max_mass)) < 0)
   max_mass = max_mass - dataHist->GetBinWidth(1);
 
  mass.setRange("fitrange", min_mass, max_mass);
  //Set up workspace
  RooWorkspace* w = new RooWorkspace(histTitle.c_str());
  RooPlot* massframe = mass.frame( Title(histTitle.c_str()));
  TCanvas* fitCan = new TCanvas(delim.c_str(),delim.c_str(), 800, 700);
  //fitCan->SetLogy();
  w->import(RooArgSet(mass));
  //Build model and Fit data
  w->factory("Chebychev::bkgd(decayxim_M,{a0[0.8,0.01,3.],a1[-0.2,-3.,-0.01]})");//,a1[-0.1,-2,-1e-2]
  // w->factory("Johnson::xisignal(decayxim_M,mu[1.322,1.321,1.323],lambda[0.006,0.003,0.008], gamma[0,-0.03,0.3], delta[1.5,1.,3.])");
  w->factory(("Johnson::xisignal(decayxim_M,mu[1.322,1.3213,1.323],lambda["+to_string(params[0])+","+to_string(params[0])+",0.008], gamma["+to_string(params[2])+"], delta["+to_string(params[1])+"])").c_str());
  w->factory(("SUM::model( nxi[1,0,1e6]*xisignal, nbkgd[1,0,1e6]*bkgd)"));// nbkgd[200,1,1e6]*bkgd,
  w->pdf("model")->fitTo(*data, Extended(true), //EvalErrorWall(false),
                         SumW2Error(false),
						//RooFit::AsymptoticError(true), 
                         Hesse(false),
                         Range("fitrange"),RecoverFromUndefinedRegions(10),
                         PrintLevel(-1),PrintEvalErrors(-1),Verbose(false));
  
  //Plot model and data 
  data->plotOn(massframe, Name("datapnts"), Binning(60, 1.26, 1.46));
  w->pdf("model")->paramOn(massframe, Format("NE",AutoPrecision(1)), Layout(0.55, 0.95, 0.92), Range("fitrange"));//, Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nbkgd")))
  w->pdf("model")->plotOn(massframe, LineWidth(4), Name("model"), Range("fitrange"));
  w->pdf("xisignal")->plotOn(massframe,DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Name("xisignal"), Range("fitrange"));//
  w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("fitrange"));
  //set up specific frame styling
  massframe->GetYaxis()->SetMaxDigits(3);
  massframe->GetYaxis()->SetNdivisions(505);
  //massframe->SetMinimum(0.1);
  massframe->Draw();

  fitCan->SetGrid();
  fitCan->SaveAs( ("/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/corrected/"+delim+".pdf").c_str());
}

void RooFitHistMC(TTree* treeData, string histTitle, string delim, vector<double> &params)
{
  TH1::AddDirectory(kFALSE);
  gStyle->SetTitleAlign(33);
  gStyle->SetTitleX(.95);
  //Set up workspace
  RooWorkspace* w = new RooWorkspace(histTitle.c_str());
  RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.26, 1.42);
  
  RooRealVar weight("hybrid_combo", "weight", -10, 10);
  // 
  RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
  TH1* dataHist = (TH1*)data->createHistogram("decayxim_M");
  Double_t min_mass = dataHist->GetXaxis()->GetBinLowEdge(dataHist->FindFirstBinAbove(0,1,1, dataHist->FindBin(1.32))-1);
  mass.setRange("signal", min_mass, 1.37);
  RooPlot* massframe = mass.frame(Title(histTitle.c_str()));
  TCanvas* fitCan = new TCanvas(delim.c_str(),delim.c_str(), 800, 700);
  //fitCan->SetLogy();
  w->import(RooArgSet(mass));
    
  //Build model and Fit data
  //w->factory("EXPR::bkgd('(decayxim_M)*(((decayxim_M)/m0)**2-1.0)**p*exp(b*(((decayxim_M)/m0)**2-1.0))',decayxim_M, m0[1.2602,1.255,1.275], b[-22,-40.,-5.], p[2])");
  //w->factory("Gaussian::xisignal(decayxim_M,mean1[1.32,1.31,1.33], sigma[0.005,0.003,0.007])");
  w->factory("Johnson::xisignal(decayxim_M,mu[1.3217,1.32,1.33],lambda[0.0045,0.003,0.006], gamma[0,-1,1], delta[1.2,0.2,3.])");
  w->factory("SUM::model( nxi[1000,1,1e6]*xisignal)");// nbkgd[200,1,1e6]*bkgd,
  w->pdf("model")->fitTo(*data,SumW2Error(false),PrintLevel(-1), Range("signal"));
  
  //Plot model and data 
  data->plotOn(massframe, Name("datapnts"));
  w->pdf("model")->paramOn(massframe, Format("NE",AutoPrecision(1)), Layout(0.55, 0.95, 0.92));//, Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nbkgd"))));
  w->pdf("model")->plotOn(massframe, LineWidth(3), Name("model"), Range("signal"));
  w->pdf("xisignal")->plotOn(massframe,DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Name("xisignal"), Range("signal"));//
  //w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("signal"));
  //set up specific frame styling
  massframe->GetYaxis()->SetMaxDigits(3);
  massframe->GetYaxis()->SetNdivisions(505);
  massframe->SetMinimum(0.1);
  massframe->Draw();

  params[0] = w->var("lambda")->getVal();
  params[1] = w->var("delta")->getVal();
  params[2] = w->var("gamma")->getVal();

  fitCan->SetGrid();
  //fitCan->SaveAs( ("/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/corrected/"+delim+".pdf").c_str());
  //delete fitCan, data, w;
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
