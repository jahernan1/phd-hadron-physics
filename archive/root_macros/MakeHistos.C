//#include <RooAbsDataHelper.h>

void setStyle();
void rooFitHist(TH1* hist, string histTitle);
void GetHistos(string root_file_name="kpkpxim__M23_2017-01_ver56_allCuts", string rootMC_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_allCuts_Weighted", string rootThrown_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_Weighted",int n_threads = 8); 
TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon);
TH1* GetAcceptanceCorrHist1D(TH1* hist_data, TH1* hist_accept);

int MakeHistos()
{
  //GetHistos("kpkpxim__M23_2017-01_ana56_allCuts_pathlensig_111111", "kpkpxim__M23_2017-01_ver56_allCuts_PathlenSig");
  GetHistos();
  
  return 0;
}

void GetHistos(string root_file_name="kpkpxim__M23_2017-01_ver56_allCuts", string rootMC_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_allCuts", string rootThrown_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_Weighted",int n_threads = 8) 
{
    // Parallelize with n threads
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	setStyle();
    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);

	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M",
                                         "xim_lifetime_restframe",
                                         "xim_pathlensig",
                                         "lambda_pathlensig",
                                         //"qvalue_decayxim_M",
                                         "acc_weight",
                                         "t_dist",
                                         "beam_E",
                                         "ystar_P3",
                                         "ystar_M",
                                         "chisqndf",
                                         "kp_highp_P3",
                                         "kp_lowp_P3",
                                         "pim1_p3",
                                         "pim1_costheta_hf",
                                         "pim2_P3",
                                         "proton_P3"
    };
    std::vector<std::string> branchesMC = {"decayxim_M",
                                           "xim_lifetime_restframe",
                                           "xim_pathlensig",
                                           "lambda_pathlensig",
                                           "acc_weight",
                                           "tot_weight",
                                           "t_dist",
                                           "beam_E",
                                           "ystar_P3",
                                           "ystar_M",
                                           "chisqndf",
                                           "kp_highp_P3",
                                           "kp_lowp_P3",
                                           "pim1_p3",
                                           "pim1_costheta_hf",
                                           "pim2_P3",
                                           "proton_P3",
	};
	// make data frame
	// format : tree name, file name, branches to open
	auto df = ROOT::RDataFrame("kpkpxim_flatTree", ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_"+root_file_name+".root"), branches);
    auto dfMC = ROOT::RDataFrame("kpkpxim_flatTree", ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_"+rootMC_file_name+".root"), branchesMC);
    auto dfT = ROOT::RDataFrame("kpkpxim_thrown_flatTree", ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_thrown_"+rootThrown_file_name+".root"));
	
    // cut out any nan q-value events
    auto df1 = df.Filter("beam_E > 6.4 && beam_E < 11.4")
      .Filter("xim_pathlensig > 1", "Xim_PathlenSigCut");
    auto df2 = df1.Filter("lambda_pathlensig > 1","Lambda_PathlenSigCut");

    cout << root_file_name.c_str() << endl;
    df2.Report()->Print();
  
    auto dfMC1 = dfMC.Filter("beam_E > 6.4 && beam_E < 11.4")
      .Filter("xim_pathlensig > 1","Xim_PathlenSigCut");
    auto dfMC2 = dfMC1.Filter("lambda_pathlensig > 1","Lambda_PathlenSigCut");

    cout << rootMC_file_name.c_str() << endl;
    dfMC2.Report()->Print();

    auto dfT1 = dfT.Define("beam_E", "beam_p4.E()")
      .Filter("beam_E > 6.4 && beam_E < 11.4")
      .Define("t_dist","-(beam_p4-kp1_p4).M2()");
    
    // Draw q-value subtracted mass distribution
    auto hist_xiMass = df1.Histo1D({"","; decayxim_M; counts*accc_weight", 64u, 1.25, 1.5},"decayxim_M", "acc_weight");
    auto hist_xiMass_lamsig = df2.Histo1D({"","; decayxim_M; counts*accc_weight", 64u, 1.25, 1.5},"decayxim_M", "acc_weight");
    
    new TCanvas;
    hist_xiMass->DrawClone("hist");
    hist_xiMass_lamsig->SetLineColor(kRed);
    hist_xiMass_lamsig->DrawClone("hist same");
    
    new TCanvas;
    rooFitHist(hist_xiMass.GetPtr()," ");
    new TCanvas;
    rooFitHist(hist_xiMass_lamsig.GetPtr()," ");
        
    auto h1 = df1.Histo1D({"","; -t (GeV^{2}/c^{4}); Arbitrary Units", 40, 0, 6}, "t_dist", "acc_weight");
    auto hMC1 = dfMC1.Histo1D({"","; -t (GeV^{2}/c^{4}); Events / 0.1 GeV",  40, 0, 6}, "t_dist", "tot_weight");
    auto hT1 = dfT1.Histo1D({"","; -t (GeV^{2}/c^{4}); Events / 0.1 GeV",  40, 0, 6}, "t_dist","mc_weight");

    auto h2 = df1.Histo1D({"","; #tau_{#Xi^{-}} (ns); Arbitrary Units", 40, 0, 1},"xim_lifetime_restframe", "acc_weight");
    auto hMC2 = dfMC1.Histo1D({"","; #tau_{#Xi^{-}} (ns); Arbitrary Units", 40, 0, 1},"xim_lifetime_restframe", "tot_weight");
    auto hT2 = dfT1.Histo1D({"","; #tau_{#Xi^{-}} (ns); Arbitrary Units", 40, 0, 1},"xim_lifetime_restframe","mc_weight");

    auto h3 = df1.Histo1D({"","; cos#theta_{H}; Arbitrary Units", 100, -1, 1},"pim1_costheta_hf", "acc_weight");
    auto hMC3 = dfMC1.Histo1D({"","; cos#theta_{H}; Arbitrary Units", 100, -1, 1},"pim1_costheta_hf", "tot_weight");
    auto  hT3 = dfT1.Histo1D({"","; cos#theta_{H}; Arbitrary Units", 100, -1, 1},"pim1_costheta_hf", "mc_weight");

    // get acceptance corrected t_dist
    // TH1D *tdist_acceptance = (TH1D*)GetAcceptanceHist1D(hT1.GetPtr(), hMC1.GetPtr())->Clone("");
    // new TCanvas;
    // tdist_acceptance->DrawClone("e1");
    // TH1D *tdist_acceptance_corr = (TH1D*)GetAcceptanceCorrHist1D(h1.GetPtr(), tdist_acceptance)->Clone("");
    // tdist_acceptance_corr->GetYaxis()->SetMaxDigits(4);
    // tdist_acceptance_corr->SetMarkerStyle(24);
    // tdist_acceptance_corr->SetMarkerSize(1);
    // tdist_acceptance_corr->SetMarkerColor(kAzure);
        
    // TCanvas *c = new TCanvas("c","c");	
    // c->SetGrid();
        
    // TF1 *f1 = new TF1("f1", "expo", tdist_acceptance_corr->GetBinCenter(tdist_acceptance_corr->GetMaximumBin()), 2.3);
    // tdist_acceptance_corr->Fit("f1", "WLR");
    // tdist_acceptance_corr->DrawClone("e1");
    
    // // get acceptance corrected ximrft_hf
    // TH1D *ximrft_acceptance = (TH1D*)GetAcceptanceHist1D(hT2.GetPtr(), hMC2.GetPtr())->Clone("");
    // new TCanvas;
    // ximrft_acceptance->DrawClone("e1");
    // TH1D *ximrft_acceptance_corr = (TH1D*)GetAcceptanceCorrHist1D(h2.GetPtr(), ximrft_acceptance)->Clone("");
    // ximrft_acceptance_corr->GetYaxis()->SetMaxDigits(4);
    // ximrft_acceptance_corr->SetMarkerStyle(24);
    // ximrft_acceptance_corr->SetMarkerSize(1);
    // ximrft_acceptance_corr->SetMarkerColor(kAzure);
        
    // TCanvas *cc = new TCanvas("cc","cc");	
    // cc->SetGrid();
        
    // TF1 *f2 = new TF1("f2", "expo", ximrft_acceptance_corr->GetBinCenter(ximrft_acceptance_corr->GetMaximumBin()), 0.8);
    // ximrft_acceptance_corr->Fit("f2", "WLIR");
    // //cc->SetLogy();
    // ximrft_acceptance_corr->DrawClone("e1");
    // cout << ximrft_acceptance_corr->GetMean() << " +/- "<< ximrft_acceptance_corr->GetMeanError() <<endl;
    // // get acceptance corrected pim1_costheta_hf
    // TH1D *pim1hf_acceptance = (TH1D*)GetAcceptanceHist1D(hT3.GetPtr(), hMC3.GetPtr())->Clone("");
    // new TCanvas;
    // pim1hf_acceptance->DrawClone("e1");
    // TH1D *pim1hf_acceptance_corr = (TH1D*)GetAcceptanceCorrHist1D(h3.GetPtr(), pim1hf_acceptance)->Clone("");
    // pim1hf_acceptance_corr->GetYaxis()->SetMaxDigits(3);
    // pim1hf_acceptance_corr->SetMarkerStyle(24);
    // pim1hf_acceptance_corr->SetMarkerSize(1);
    // pim1hf_acceptance_corr->SetMarkerColor(kAzure);
        
    // TCanvas *cc1 = new TCanvas("cc1","cc1");	
    // cc1->SetGrid();
        
    // // TF1 *f2 = new TF1("f2", "expo", pim1hf_acceptance_corr->GetBinCenter(pim1hf_acceptance_corr->GetMaximumBin()), 0.6);
    // // pim1hf_acceptance_corr->Fit("f2", "WLR");
    // // cc->SetLogy();
    // pim1hf_acceptance_corr->DrawClone("e1");

    // new TCanvas;
    // h2->DrawClone("e1");
	//df2.Snapshot("kpkpxim_flatTree", "/d/grid17/hjesse/Trees/flatTree/flatTree_postQVal"+root_file_name+".root");
}

TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon)
{
  TH1D* hist_accept = (TH1D*)hist_recon->Clone("acceptance");
  hist_accept->GetYaxis()->SetTitle("#epsilon");
  hist_accept->Divide(hist_genr);
    
  printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
  return hist_accept;
}

TH1* GetAcceptanceCorrHist1D(TH1* hist_data, TH1* hist_accept)
{
  char newName[100];
  double binwidth = hist_data->GetBinWidth(1);
  sprintf(newName,"Events/#epsilon / %.2f GeV", binwidth);
  
  hist_data->GetYaxis()->SetTitle(newName);
  TH1* hist_data_acccorr = (TH1*)hist_data->Clone();
  hist_data_acccorr->Divide(hist_accept);
  
  return hist_data_acccorr;
}

void rooFitHist(TH1* hist, string histTitle)
{
  //setStyle();
  Double_t min_mass = hist->GetXaxis()->GetBinCenter(hist->FindFirstBinAbove(0.1,1,1, hist->FindBin(1.3))+1);
  if(min_mass < 1.26) min_mass = 1.26;

  RooWorkspace* w = new RooWorkspace(histTitle.c_str());
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", min_mass, 1.45);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  RooPlot* massframe = mass.frame(RooFit::Title(histTitle.c_str()));
  w->import(RooArgSet(mass));

  
  w->factory("Chebychev::bkgd(mass,{a0[0.8,0.01,1.2]})");//,a1[-0.1,-0.5,-0.05]
  w->factory("Johnson::xigaus(mass,mu[1.322,1.31, 1.33],lambda[0.005], gamma[0], delta[1.5, 1, 2])");
  //w->factory("Voigtian::sigma(mass,mean[1.387],sig[0.005], width[0.018, 0.01, 0.024])");

  //Create model and fit to data
  w->factory("SUM::model( nbkgd[200,1,1e6]*bkgd, nxi[20,1,1e5]*xigaus)");//nbkgd[200,1,1e6]*bkgd,nsigma[200,1,1e6]*sigma
  
   w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::SumW2Error(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.6, 0.85, 0.95));
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(2));
  w->pdf("xigaus")->plotOn(massframe,RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent));
  //  w->pdf("sigma")->plotOn(massframe, RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nsigma")->getVal(), RooAbsReal::NumEvent));
  w->pdf("bkgd")->plotOn(massframe,RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent));
  //RooFit::DrawOption("F"), RooFit::FillColor(kAzure-1),RooFit::FillStyle(4010),RooFit::MoveToBack()

  massframe->Draw();
}

void setStyle()
{
  // gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  // gStyle->SetCanvasDefH(600);
  // gStyle->SetCanvasDefW(700);
  // gStyle->SetMaxDigits(3);
  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.14);
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
  gStyle->SetMarkerSize(1);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.06,"X");
  gStyle->SetLabelSize(0.06,"Y");

  gStyle->SetLabelOffset(0.008,"X");
  gStyle->SetLabelOffset(0.008,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132,"T");
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");
  
  gStyle->SetTitleSize(0.08,"T");
  gStyle->SetTitleSize(0.08,"X");
  gStyle->SetTitleSize(0.08,"Y");
  
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(0.6,"Y");

  gStyle->SetTextSize(0.08);
  gStyle->SetTextFont(132);

  gStyle->SetOptStat(1);
  gStyle->SetOptFit(1);

  gROOT->ForceStyle();

  TLatex* latex = new TLatex();
  latex->SetNDC();
  latex->SetTextFont(132);
  latex->SetTextSize(0.08);
  latex->SetTextAlign(32);
  gROOT->ForceStyle();
}
