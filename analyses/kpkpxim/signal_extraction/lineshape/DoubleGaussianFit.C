#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
#include "gxana/fit/Fit.h"
#include "gxana/fit/Model.h"
void RooFitHist(TTree* TreeData, string histTitle, string delim,  vector<double> params);
void RooFitHistMC(TTree* treeData, string histTitle, string delim, vector<double> &params);
void setStyle();
void GetXim1320_IM(string filename, string treename);
using namespace RooFit;

int DoubleGaussianFit()
{
  
    GetXim1320_IM("flatTree_kpkpxim__B4_M23_2018-01_ana03", "emin_7.40_emax_7.86_tmin_1.53_tmax_2.40");
  
  return 0;
}

void GetXim1320_IM(string filename, string treename) {
    setStyle();
    // Initialize variables
    string tree_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
    vector<double> xiParams = {1.3217, 0.004, 0.01};
    // Branches you want to use get
    std::vector<std::string> branches = {"decayxim_M"};
    
	//make data frame
	//format : tree name, file name, branches to open
    TFile* dataFile = TFile::Open((tree_dir+"binned_"+filename+"_nominal_kphighrap.root").c_str(), "READ");
    TFile* dataFileMC = TFile::Open((tree_dir+"binned_"+filename+"_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root").c_str(), "READ");
    TTree* dataTree =  (TTree*)dataFile->Get(treename.c_str());
    TTree* dataTreeMC =  (TTree*)dataFileMC->Get(treename.c_str());
 
    //Fit Data and MC for yields
    string tmin = treename.substr(treename.find("tmin")+5,4);
    string tmax = treename.substr(treename.find("tmax")+5,4);
    string histTitle = "#bf{-t: ("+tmin+", "+tmax+")} ;M(#Lambda#pi^{-}) (GeV); Events";

    RooFitHistMC(dataTreeMC, histTitle, "recon_"+filename.substr(0,filename.find_last_of(".")), xiParams);
    cout << "MC params extracted (mean, sigma1, sigma2): (" << xiParams[0] << ", " << xiParams[1] << ", " << xiParams[2] << ")" << endl;
    RooFitHist(dataTree, histTitle, filename.substr(0,filename.find_last_of(".")), xiParams);
}
 
void RooFitHist(TTree* treeData, string histTitle, string delim, vector<double> params)
{
  gStyle->SetTitleAlign(33);
  gStyle->SetTitleX(.95);
  //Import dataset to plot
  RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.26, 1.46);
  RooDataSet* data = gxana::fit::ImportTree(*treeData, mass, "hybrid_combo");
  // RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass), Import(*treeData));
  TH1* dataHist = (TH1*)data->createHistogram("decayxim_M")->Clone(delim.c_str());
  Double_t min_mass = gxana::fit::FirstPopulatedEdge(*dataHist, 0, 1, 1.32);
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
  gxana::fit::BuildModel(*w, {
      gxana::fit::Chebychev("bkgd", "decayxim_M", {{"a0", "0.8,0.01,3."}, {"a1", "-0.2,-3.,-0.01"}}),//,a1[-0.1,-2,-1e-2]
      // w->factory("Johnson::xisignal(decayxim_M,mu[1.322,1.321,1.323],lambda[0.006,0.003,0.008], gamma[0,-0.03,0.3], delta[1.5,1.,3.])");
      gxana::fit::Gaussian("xisignal1", "decayxim_M", {"mean", gxana::fit::Fx(params[0]) + ",1.32,1.33"},
                           {"sigma1", gxana::fit::Fx(params[1]) + "," + gxana::fit::Fx(params[1]) + ",0.01"}),
      gxana::fit::Gaussian("xisignal2", "decayxim_M", {"mean", ""}, {"sigma2", gxana::fit::Fx(params[2])}),
      gxana::fit::Sum("model", {{{"nxi1", "1,1,1e6"}, "xisignal1"}, {{"nxi2", "1,1,1e6"}, "xisignal2"},
                                {{"nbkgd", "1,0,1e6"}, "bkgd"}})});// nbkgd[200,1,1e6]*bkgd,
  gxana::fit::RunFit(*w->pdf("model"), *data, Extended(true), //EvalErrorWall(false),
                     SumW2Error(false),
                     //RooFit::AsymptoticError(true),
                     Hesse(false),
                     Range("fitrange"),RecoverFromUndefinedRegions(10),
                     PrintLevel(-1),PrintEvalErrors(-1),Verbose(false));
  
  //Plot model and data 
  data->plotOn(massframe, Name("datapnts"), Binning(60, 1.26, 1.46));
  w->pdf("model")->paramOn(massframe, Format("NE",AutoPrecision(1)), Layout(0.55, 0.95, 0.92), Range("fitrange"));//, Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nbkgd")))
  w->pdf("model")->plotOn(massframe, LineWidth(4), Name("model"), Range("fitrange"));
  w->pdf("model")->plotOn(massframe, Components("xisignal1,xisignal2"),LineStyle(kDashed), Name("xisignal1"), Range("fitrange"));
  //w->pdf("model")->plotOn(massframe, Components("xisignal2"),LineStyle(kDashed),LineColor(kMagenta), Name("xisignal2"), Range("fitrange")); 
  
  w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("fitrange"));
  //set up specific frame styling
  massframe->GetYaxis()->SetMaxDigits(3);
  massframe->GetYaxis()->SetNdivisions(505);
  //massframe->SetMinimum(0.1);
  massframe->Draw();

  fitCan->SetGrid();
  //fitCan->SaveAs( (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/ml_fits/corrected/")+delim+".pdf").c_str());
}

void RooFitHistMC(TTree* treeData, string histTitle, string delim, vector<double> &params)
{
    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    //Set up workspace
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.26, 1.42);
  
    RooDataSet* data = gxana::fit::ImportTree(*treeData, mass, "hybrid_combo");
    TH1* dataHist = (TH1*)data->createHistogram("decayxim_M");
    Double_t min_mass = gxana::fit::FirstPopulatedEdge(*dataHist, 0, 1, 1.32, -1);
    mass.setRange("signal", min_mass, 1.37);
    RooPlot* massframe = mass.frame(Title(histTitle.c_str()));
    TCanvas* fitCan = new TCanvas(delim.c_str(),delim.c_str(), 800, 700);
    //fitCan->SetLogy();
    w->import(RooArgSet(mass));
    
    //Build model and Fit data
    gxana::fit::BuildModel(*w, {
        //w->factory("EXPR::bkgd('(decayxim_M)*(((decayxim_M)/m0)**2-1.0)**p*exp(b*(((decayxim_M)/m0)**2-1.0))',decayxim_M, m0[1.2602,1.255,1.275], b[-22,-40.,-5.], p[2])");
        gxana::fit::Gaussian("xisignal1", "decayxim_M", {"mean", gxana::fit::Fx(params[0]) + ",1.32,1.33"},
                             {"sigma1", gxana::fit::Fx(params[1]) + ",0.002,0.01"}),
        gxana::fit::Gaussian("xisignal2", "decayxim_M", {"mean", ""}, {"sigma2", gxana::fit::Fx(params[2]) + ",0.005,0.02"}),
        // the original statement as text: Sum() has no form for its trailing comma
        "SUM::model( nxi1[1000,1,1e6]*xisignal1, nxi2[1000,1,1e6]*xisignal2,  )"});// nbkgd[200,1,1e6]*bkgd,
    gxana::fit::RunFit(*w->pdf("model"), *data, SumW2Error(false), PrintLevel(-1), Range("signal"));
  
    //Plot model and data 
    data->plotOn(massframe, Name("datapnts"));
    w->pdf("model")->paramOn(massframe, Format("NE",AutoPrecision(1)), Layout(0.55, 0.95, 0.92));//, Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nbkgd"))));
    w->pdf("model")->plotOn(massframe, LineWidth(3), Name("model"), Range("signal"));
    w->pdf("model")->plotOn(massframe, Components("xisignal1,xisignal2"),LineStyle(kDashed), MoveToBack(), Range("signal"));//DrawOption("F"), FillColor(kBlue - 9), FillStyle(3001)
    //w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("signal"));
    //set up specific frame styling
    massframe->GetYaxis()->SetMaxDigits(3);
    massframe->GetYaxis()->SetNdivisions(505);
    massframe->SetMinimum(0.1);
    massframe->Draw();

    params[0] = w->var("mean")->getVal();
    params[1] = w->var("sigma1")->getVal();
    params[2] = w->var("sigma2")->getVal();

    fitCan->SetGrid();
    //fitCan->SaveAs( (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/ml_fits/corrected/")+delim+".pdf").c_str());
    //delete fitCan, data, w;
}

void setStyle()
{
    gxana::ApplyStyle(gxana::FitStyle());
}
