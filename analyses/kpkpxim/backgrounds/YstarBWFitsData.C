#include "gxana/common/Paths.h"
//#include <RooAbsDataHelper.h>

void setStyle();
void rooFitHist(TH1* hist,char* histTitle, char* ws_name, double* yield, double* yield_err);
void FitYstarMass(string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111", int n_threads = 8);
void rooFitHist(TH1D* hist, string histTitle);

//main
int YstarBWFitsData()
{
  vector<string> rootFile = {"kpkpxim__B4_M23_2018-08_ana02_nominal_allKaonSep_1111111"
  };
  //run cut analysis
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  for(int loc_i=0; loc_i < rootFile.size(); loc_i++)
    {
      FitYstarMass(rootFile[loc_i]);
      //GetCutAnalysis(mm2Bins, rootFile[loc_i], 0.04 , "total_mm22", "|MM(#gammapK^{+}K^{+}#Xi^{-})|^{2} (GeV^{2}/c^{4})" );
      //GetCutAnalysis(xifsBins, rootFile[loc_i], 2 , "xim_pathlensig", "#Xi Flight Significance" );
    }

  return 0;
}

void FitYstarMass(string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111", int n_threads = 8) {
	// Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	//setStyle();

	// Branches you want to use get
	std::vector<std::string> branches = {"ystar_M",
                                         "decayxim_M",
                                         "qvalue_decayxim_M",
                                         "acc_weight",
                                         "beam_E"
                                         
	};
    // format : tree name, file name, branches to open	
    // make data frame
    auto df = ROOT::RDataFrame("flatTree_kpkpxim", (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/")+root_file_name+"/postQVal_flatTree_"+root_file_name+".root").c_str(), branches);
    //setect data points and make signal and bkg weights
    auto nanCut = [](float f) {return !isnan(f);};
    auto df1 = df.Filter(nanCut, {"qvalue_decayxim_M"})
      .Filter("beam_E > 6.4 && beam_E < 11.4")
      .Define("qacc_decayxim_M", "acc_weight*qvalue_decayxim_M")
      .Define("qbkg_decayxim_M", "acc_weight*(1-qvalue_decayxim_M)");
    
    // Print Yield Value
    cout << "Q-Value Tree: " << root_file_name.c_str() << endl;
    cout << "Sum of Weighted Q-Values: " << df1.Sum("qacc_decayxim_M").GetValue() << endl;
    cout << endl;
    
    auto h = df1.Histo2D({"","weights:qacc_decayxim_M; ystar_M; decayxim_M", 60, 1.7,4.1, 125, 1.25, 1.5},"ystar_M", "decayxim_M", "qacc_decayxim_M");
    auto h1 = df1.Histo2D({"","weights:qacc_decayxim_M; M(K^{+}#Xi^{-}) (GeV/c^{2}); E_{#gamma} (GeV)", 120, 1.7, 4.1, 60, 6, 12},"ystar_M","beam_E", "qacc_decayxim_M");
    auto hist_YstarMass = df1.Histo1D("ystar_M", "qacc_decayxim_M");
    hist_YstarMass->RebinX();
    
    new TCanvas;
    hist_YstarMass->DrawClone("hist");
    
    new TCanvas;
    //hist_YstarMass->DrawClone("hist");
    rooFitHist(hist_YstarMass.GetPtr(),"");
    
    new TCanvas;
    h->DrawClone("colz");
	new TCanvas;
    h1->DrawClone("colz");
    
    //df2.Snapshot("kpkpxim_flatTree", gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_postQVal")+root_file_name+".root");
}

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

  w->factory("BreitWigner::bw1(mass,mean1[2.1,1.9,2.2], width1[0.15, 0.1,0.2])");
  w->factory("BreitWigner::bw2(mass,mean2[2.3,2.2,2.5], width2[0.4, 0.15,0.6])");
  w->factory("BreitWigner::bw3(mass,mean3[2.8, 2.6,3], width3[0.6,0.3,1])");

  //Create model and fit to data
  w->factory("SUM::model( nbw1[0]*bw1, nbw2[300,1,1e6]*bw2, nbw3[200,1,1e6]*bw3)");
  
  w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::SumW2Error(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.6, 0.85, 0.95));
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(3));
  w->pdf("bw1")->plotOn(massframe,RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbw1")->getVal(), RooAbsReal::NumEvent));
  w->pdf("bw2")->plotOn(massframe, RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbw2")->getVal(), RooAbsReal::NumEvent));
  w->pdf("bw3")->plotOn(massframe, RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbw3")->getVal(), RooAbsReal::NumEvent));
  //RooFit::DrawOption("F"), RooFit::FillColor(kAzure-1),RooFit::FillStyle(4010),RooFit::MoveToBack()

  massframe->Draw();
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
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
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

  //gStyle->SetLineWidth(1);
  //gStyle->SetHistLineWidth(1);
  //gStyle->SetFrameLineWidth(2);
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.07,"X");
  gStyle->SetLabelSize(0.07,"Y");

  gStyle->SetLabelOffset(0.008,"X");
  gStyle->SetLabelOffset(0.008,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132,"T");
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");
  
  gStyle->SetTitleSize(0.07,"T");
  gStyle->SetTitleSize(0.08,"X");
  gStyle->SetTitleSize(0.08,"Y");
  
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(1.0,"Y");

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
