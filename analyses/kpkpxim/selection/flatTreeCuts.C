#include "gxana/common/Paths.h"
#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

void rooFitHist(TH1 &hist, double* yield, double* yield_err, std::string canName="c", bool sigmabool = true);
void PlotFromFlat(int n_threads = 4, string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111");
void setStyle();

int flatTreeCuts()
{
  vector<string> rootFile = {"flatTree_kpkpxim__M23_2017-01_ver45", "flatTree_kpkpxim__B4_M23_2018-01_ver03", "flatTree_kpkpxim__B4_M23_2018-08_ver02","kpkpxim_2018-08_ana-05_LE","kpkpxim_2019-11_ana-04_b05_b12"};
 
  //run cut analysis
  for(int loc_i=0; loc_i < 3; loc_i++)
    {
      PlotFromFlat(4, rootFile[loc_i] );
      //GetCutAnalysis(mm2Bins, rootFile[loc_i], 0.04 , "total_mm22", "|MM(#gammapK^{+}K^{+}#Xi^{-})|^{2} (GeV^{2}/c^{4})" );
      //GetCutAnalysis(xifsBins, rootFile[loc_i], 2 , "xim_pathlensig", "#Xi Flight Significance" );
    }

  return 0;
}

void PlotFromFlat(int n_threads = 4, string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111") {
	// Parallelize with n threads
        if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M",
                                         "xim_vertexZ",
                                         "lambda_vertexZ",
                                         "xim_pathlensig",
                                         "lambda_pathlensig",
                                         "acc_weight",
                                         "chisqndf",
                                         "confidencelvl",
                                         "total_mm2",
                                         "kp1_P3",
                                         "kp_highp_P3",
                                         "kp_highp_CosTheta",
                                         "kp2_P3",
                                         "kp_lowp_P3",
                                         "kp_lowp_Ystar_CosTheta",
                                         "beam_E",
                                         "t_dist",                           
	};

	// make data frame
	// format : tree name, file name, branches to open
	// auto df = ROOT::RDataFrame("kpkpxim_flatTree", (gxana::EnvPath("GXANA_OUTPUT", "legacy_macros/QFactors/logs/")+root_file_name+"/postQVal_flatTree_"+root_file_name+".root").c_str(), branches);
	auto df = ROOT::RDataFrame("flatTree_kpkpxim", gxana::EnvPath("GXANA_DATA", "Trees/flatTree/")+root_file_name+".root", branches);
        
    //auto accCut = [](double f, double e) {return f == 1. && e >= 6.4;};
	auto df1 = df.Filter("beam_E > 6.4 && beam_E < 11.4");
    auto cnt1 = df1.Count();
    auto df2 = df1.Filter("chisqndf < 4.0");
    auto cnt2 = df2.Count();
    auto df5 = df2.Filter("xim_vertexZ < lambda_vertexZ");
    auto cnt5 = df5.Count();
    auto df6 = df5.Filter("xim_pathlensig > 2.0");
    auto cnt6 = df6.Count();
    auto df3 = df2.Filter("abs(total_mm2) < 0.04");
    auto cnt3 = df3.Count();
	auto df4 = df2.Filter("kp_highp_P3 >= 3 && kp_lowp_P3 < 3");
    auto cnt4 = df4.Count();
    auto df7 = df2.Filter("decayxim_M > 1.308 && decayxim_M < 1.338");
    auto df71 = df6.Filter("decayxim_M > 1.308 && decayxim_M < 1.338");
    
    // Draw 
	auto h = df2.Histo2D({"","; M(#Lambda#pi^{-}) (GeV); Pathlength Significance",500,1.25,1.5,200,0,50},"decayxim_M", "xim_pathlensig", "acc_weight");
	auto h1 = df2.Histo2D({"","; M(#Lambda#pi^{-}) (GeV); #vec{#bf{p}}(K^{+}_{1}) (GeV/c)",250,1.25,1.5,100,0,10},"decayxim_M", "kp2_P3");
	//auto h2 = df2.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "kp_lowp_P3");
	//auto h22 = df2.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "kp_highp_P3");
	
	auto h2 = df2.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
	auto h3 = df3.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    auto h4 = df4.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    auto h5 = df5.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    auto h6 = df6.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    
    auto h7 = df2.Histo1D({"","ChiSqNdf > 4; Z_{#Xi} (cm); Events", 90, 20, 200},"xim_vertexZ", "acc_weight");
    auto h71 = df6.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; Z_{#Xi} (cm); Events", 90, 20, 200},"xim_vertexZ", "acc_weight");
    auto h8 = df7.Histo1D({"","ChiSqNdf > 4; cos#theta_{K^{+}_{Prod}}; Events", 50, 0.5, 1},"kp_highp_CosTheta", "acc_weight");
    auto h81 = df71.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; cos#theta_{K^{+}_{Prod}}; Events", 50, 0.5, 1},"kp_highp_CosTheta", "acc_weight");
    auto h91 = df71.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #overrightarrow{p}({K^{+}_{Prod}}) (GeV/c); Events", 100, 0.0, 10},"kp_highp_P3", "acc_weight");
   
    auto h10 = df71.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #overrightarrow{p}({K^{+}_{Prod}}) (GeV/c); Events", 100, 0.0, 10},"kp_lowp_P3", "acc_weight");
   
    auto h11 = df71.Histo1D({"","; -t_{#gamma K^{+}_{Prod}} (GeV^{2}/c^{4}); Events", 120, 0, 12},"t_dist", "acc_weight");
    auto h12 = df2.Histo2D({"","; |MM(#gamma pK^{+}K^{+}#Xi^{-})|^{2} (GeV^{2}/c^{4}); M(#Lambda#pi^{-}) (GeV/c^{2})", 100, 0,0.1,125, 1.25, 1.5},"total_mm2","decayxim_M", "acc_weight");
    // auto h8 = df7.Histo2D({"","ChiSqNdf > 4; #vec{p}(K^{+}_{Prod}) (GeV/c); #cos#theta_{K^{+}_{Prod}}", 100, 0, 10, 50, 0.5, 1},"kp_highp_P3", "kp_highp_CosTheta", "acc_weight");
    // auto h81 = df71.Histo2D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #vec{p}(K^{+}_{Prod}) (GeV/c); cos#theta_{K^{+}_{Prod}}", 100, 0, 10, 50, 0.5, 1},"kp_highp_P3", "kp_highp_CosTheta", "acc_weight");
    //auto h81 = df71.Histo2D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #vec{p}(K^{+}_{#Xi}) (GeV/c); Events", 100, 0, 10},"kp_highp_P3", "acc_weight");

    //auto h9 = df2.Histo2D({"","; Chisq/Ndf; M(#Lambda#pi^{-}) (GeV)",350, 0, 4, 125,1.25,1.5},"chisqndf", "decayxim_M");//, "acc_weight");
	//auto h5 = df4.Histo1D({"","",500,0,10},"kp_highp_P3");
	//auto h6 = df5.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "kp_highp_P3");
	//auto h7 = df5.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "kp_lowp_P3");
    //auto h10 = df2.Histo2D({"","; Confidence Lvl; M(#Lambda#pi^{-}) (GeV)", 5000, 0, 1, 125,1.25,1.5},"confidencelvl", "decayxim_M", "acc_weight");
    //h->DrawClone("colz");
    setStyle();
    new TCanvas();
    // h10->Scale(h010->GetEntries()/h10->GetEntries());
    // h10->SetLineColor(kRed);
    // h10->DrawClone("e1");
    // h010->DrawClone("e1 same");    
    
    h6->DrawClone("e1");

    // Plot and fit the invariant mass of cascade
    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;
    Double_t yield;
    Double_t yield_err;
    
    cout << "beam_E: " << *cnt1 << endl;
    
    //rooFitHist(*h2, &yield, &yield_err, "ChiSqNdf Cut");
    cout << "chisqndf: " << *cnt2 << " Signal: " << yield << endl;
    
    //rooFitHist(*h3, &yield, &yield_err, "MM2 Cut");
    cout << "total_mm2: " << *cnt3  << " Signal: " << yield << endl;
    
    //rooFitHist(*h4, &yield, &yield_err, "Kaon Momentum Cut");
    cout << "kaon_momentum: " << *cnt4  << " Signal: " << yield << endl;
    
    //rooFitHist(*h5, &yield, &yield_err, "Vertex Cut");
    cout << "vertex_z: " << *cnt5  << " Signal: " << yield << endl;
    
    //rooFitHist(*h6, &yield, &yield_err, "Path Length Sig Cut", false);
    cout << "xim_pathlensig: " << *cnt6  << " Signal: " << yield << endl;
    
	//h3->SetLineColor(kRed);
	//h4->SetLineColor(kGreen);
	//h6->SetLineColor(kMagenta);

    TLine* hl = new TLine(1.25,3,1.5,3);
	hl->SetLineColor(2); hl->SetLineWidth(3);
	// TArrow* ar = new TArrow(1.35, 3, 1.35, 1.5, 0.05, "|>");
	// ar->SetLineColor(2); ar->SetFillColor(2); ar->SetLineWidth(3);
	TArrow* ar = new TArrow(1.35, 3, 1.35, 4.5, 0.05, "|>");
	ar->SetLineColor(2); ar->SetFillColor(2); ar->SetLineWidth(3);

	// h2->DrawClone("hist same");
    //	h9->DrawClone("colz same");

	// h3->DrawClone("same");
	// h4->DrawClone("same");
	// h5->DrawClone("same");
	// //hl->Draw("same");ar->Draw("");
	//h->DrawClone("colz");
	
	// h2->SetFillColorAlpha(kMagenta,0.1);
	// h2->DrawClone("hist same");
	// h1->SetLineColor(kRed);
	// h1->DrawClone("hist same");

	//df6.Snapshot("kpkpxim_flatTree", (gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_")+root_file_name+"Cut.root").c_str());
}

void rooFitHist(TH1 &hist, double* yield, double* yield_err, std::string canName = "c", bool sigmabool = true )
{
  Double_t min_mass = hist.GetXaxis()->GetBinLowEdge(hist.FindFirstBinAbove(0,1,1, hist.FindBin(1.3)));
  if(min_mass < 1.26) min_mass = 1.26;

  TCanvas *c = new TCanvas(canName.c_str(), canName.c_str());
  RooWorkspace* w = new RooWorkspace("ws_name");
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", min_mass, 1.45);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, &hist);
  RooPlot* massframe = mass.frame(RooFit::Title(" "));
  //XiMassKinFit_Ebin_accsub->Print();
  w->import(RooArgSet(mass)); 
  
  if (sigmabool)
    {
      w->factory("Chebychev::bkgd(mass,{a0[0.8,0.01,1.2]})");//,a1[-0.1,-0.5,-0.05]
      // w->factory("Voigtian::sigma(mass,mean[1.3872,1.3867,1.3877],sig[0.005. 0.004, 0.007], width[0.02, 0.01, 0.0415])");
      // w->factory("Johnson::xigaus(mass,mu[1.322,1.31,1.33],lambda[0.005,0.004,0.007], gamma[0], delta[1.1,0.1,10])");
      w->factory("Johnson::xigaus(mass,mu[1.322,1.31, 1.33],lambda[0.005, 0.004, 0.006], gamma[0], delta[1.5, 0.1, 5])");
      w->factory("Voigtian::sigma(mass,mean[1.387],sig[0.005], width[0.018, 0.01, 0.024])");
      w->factory("SUM::model(nbkgd[200,1,1e6]*bkgd, nxi[100,1,1e6]*xigaus, nsigma[0,0,1e5]*sigma)");
    }
  else
    {
      w->factory("Chebychev::bkgd(mass,{a0[0.8,0.01,1.2]})");//a1[-0.1,-0.5,-0.05]
      w->factory("Johnson::xigaus(mass,mu[1.322,1.31,1.33],lambda[0.005,0.004,0.006], gamma[0], delta[1.5,0.1,5])");
      // w->factory("Johnson::xigaus(mass,mu[1.322,1.31, 1.33],lambda[0.005], gamma[0], delta[1.5, 1, 2])");
      w->factory("SUM::model(nbkgd[200,1,1e6]*bkgd, nxi[100,1,1e6]*xigaus)");
    }
  
  w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::SumW2Error(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false)); //RooFit::AsymptoticError(true)
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.5, 0.95, 0.92) );
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(2) );
  w->pdf("xigaus")->plotOn(massframe,RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(4010),RooFit::MoveToBack(), RooFit::Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent) );
  if(sigmabool)
    w->pdf("sigma")->plotOn(massframe, RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(4010),RooFit::MoveToBack() , RooFit::Normalization(w->var("nsigma")->getVal(), RooAbsReal::NumEvent));
  
  w->pdf("bkgd")->plotOn(massframe, RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent) );
  
  *yield = w->var("nxi")->getVal();
  *yield_err = w->var("nxi")->getError();
  
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
  gStyle->SetPadBottomMargin(0.19);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.15);
  gStyle->SetPadRightMargin (0.05);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);
  gStyle->SetGridStyle(3);
  gStyle->SetGridWidth(1);

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
  
  gStyle->SetTitleSize(0.105,"T");
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
