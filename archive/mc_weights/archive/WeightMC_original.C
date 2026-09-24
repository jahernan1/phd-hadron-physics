//#include <RooAbsDataHelper.h>

void setStyle();
void MakeWeightedTree(string root_file_name="kpkpxim__M23_2017-01_ver56_nominal_1111111", string rootMC_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_nominal", string rootThrown_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8",int n_threads = 8);
TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon);
TH1* GetAcceptanceCorrHist1D(TH1* hist_data, TH1* hist_accept);

//main
int WeightMC()
{
  
  vector<string> rootFile = {"kpkpxim_2017-01__M23_ana45_PathlenSigCut_111111"
  };
    
  for(int loc_i=0; loc_i < rootFile.size(); loc_i++)
    {
      // MakeWeightedTree();
      // MakeWeightedTree("kpkpxim__B4_M23_2018-01_ver03_nominal_1111111","kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600_nominal","kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600");
      // MakeWeightedTree("kpkpxim__B4_M23_2018-08_ver02_nominal_1111111","kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600_nominal","kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600");
      MakeWeightedTree("kpkpxim__M23_2017-01_ver56_nominal_ximLambVertex11_1111111","kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_nominal_ximLambVertex11","kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8");
      MakeWeightedTree("kpkpxim__B4_M23_2018-01_ver03_nominal_ximLambVertex11_1111111","kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600_nominal_ximLambVertex11","kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600");
      MakeWeightedTree("kpkpxim__B4_M23_2018-08_ver02_nominal_ximLambVertex11_1111111","kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600_nominal_ximLambVertex11","kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600");
      // MakeWeightedTree("kpkpxim__M23_2017-01_ver56_nominal_all_1111111","kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_nominal_all","kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8");
      // MakeWeightedTree("kpkpxim__B4_M23_2018-01_ver03_nominal_all_1111111","kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600_nominal_all","kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600");
      // MakeWeightedTree("kpkpxim__B4_M23_2018-08_ver02_nominal_all_1111111","kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600_nominal_all","kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600");
      
    }
  
  return 0;
}

void MakeWeightedTree(string root_file_name="kpkpxim__M23_2017-01_ver56_nominal_1111111", string rootMC_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_nominal", string rootThrown_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8",int n_threads = 8) {
    // Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	setStyle();
    
	// Branches you want to get
	std::vector<std::string> branchesQ = {"ystar_M",
                                          "decayxim_M",
                                          "qvalue_decayxim_M",
                                          "acc_weight",
                                          "kplow_p4_ystar_hf",
                                          "kplow_costheta_hf",
                                          "kp_lowp_P3",
                                          "beam_E"
    };

    // format : tree name, file name, branches to open	
    // make data frames
    auto df = ROOT::RDataFrame("flatTree_kpkpxim", ("/d/grid17/hjesse/macros/QFactors/logs/"+root_file_name+"/postQVal_flatTree_"+root_file_name+".root").c_str(), branchesQ);
    auto dfMC = ROOT::RDataFrame("flatTree_kpkpxim", ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_"+rootMC_file_name+".root").c_str());
    auto dfT = ROOT::RDataFrame("flatTree_thrown_kpkpxim", ("/d/grid17/hjesse/Trees/flatTree/flatTree_thrown_"+rootThrown_file_name+".root").c_str());
    
    // select data points and make signal and bkg weights
    auto df1 = df.Define("qbkg_decayxim_M", "acc_weight*(1-qvalue_decayxim_M)");
    // get mc and thrown df
    auto dfT1 = dfT.Define("decayxim_M","decayxim_p4.M()");
            
    // Get data helicity histogram
    auto hist_thetahf = df1.Histo1D({"","Data; kplow_costheta_hf; counts*qacc_weight",50u,-1,1},"kplow_costheta_hf","qacc_decayxim_M");
    TH1D* hist_thetahf_norm = (TH1D*)hist_thetahf.GetPtr()->Clone();  
    auto mchist_thetahf = dfMC1.Histo1D({"","Reconstructed;kplow_costheta_hf;counts*acc_weight",50u,-1,1},"kplow_costheta_hf","acc_weight");
    auto thrownhist_thetahf = dfT1.Histo1D({"","Generated;kplow_costheta_hf;counts",50u,-1,1},"kp2_costheta_hf");
    
    // plot helicity in data and simulation
    TCanvas *c = new TCanvas();
    c->Divide(3,1);
    c->cd(1);
    hist_thetahf->DrawClone("hist");
    c->cd(2);
    mchist_thetahf->SetMinimum(0);
    mchist_thetahf->DrawClone("hist");
    c->cd(3);
    thrownhist_thetahf->SetMinimum(0);
    thrownhist_thetahf->DrawClone("hist");
    
    // Get the acceptance histos
    TH1D *thetahf_acceptance = (TH1D*)GetAcceptanceHist1D(thrownhist_thetahf.GetPtr(), mchist_thetahf.GetPtr())->Clone("");
    TCanvas *acceptCan = new TCanvas("acceptCan","Acceptance");
    thetahf_acceptance->SetTitle("Acceptance");
    thetahf_acceptance->DrawClone("e1");
    // acceptCan->SaveAs( ("/d/grid17/hjesse/pres_plots/xsecwg_apr19/kplow_thetahf_"+rootMC_file_name+".pdf").c_str());
    TH1D *hist_thetahf_acceptcorr = (TH1D*)GetAcceptanceCorrHist1D(hist_thetahf.GetPtr(), thetahf_acceptance)->Clone("");
    TCanvas *acceptCorrCan = new TCanvas("acceptCorrCan","AccCorr");
    hist_thetahf_acceptcorr->SetTitle("Acceptance Corrected");
    hist_thetahf_acceptcorr->DrawClone("e1");
    TH1D *hist_thetahf_acceptcorr_norm = (TH1D*)hist_thetahf_acceptcorr->Clone("");
    
    // data normalization
    double thetahf_integral = hist_thetahf->Integral();
    cout << "Lets Check the integral: " << thetahf_integral << endl;
    //cout << "Is it the same as NumEntries? " << hist_thetahf->GetEntries() << endl;
    double thetahf_acceptcorr_integral = hist_thetahf_acceptcorr->Integral();
    cout << "Lets Check the AccCorr integral: " << thetahf_acceptcorr_integral << endl;
    
    for(int i = 0; i < hist_thetahf->GetNbinsX(); i++ )
      {
        hist_thetahf_norm->SetBinContent(i+1, hist_thetahf->GetBinContent(i+1)/thetahf_integral);
        hist_thetahf_acceptcorr_norm->SetBinContent(i+1, hist_thetahf_acceptcorr->Integral(i+1,i+1)/thetahf_acceptcorr_integral);
      }

    TCanvas *normCan = new TCanvas("normCan","Norm Can");
    normCan->Divide(2,1);
    normCan->cd(1);
    hist_thetahf_norm->DrawClone("hist");
    normCan->cd(2);
    //hist_thetahf_acceptcorr_norm->SetLineColor(kRed);
    hist_thetahf_acceptcorr_norm->DrawClone("hist");
    cout << "thetahf norm integral: " << hist_thetahf_norm->Integral() << endl;
    cout << "thetahf acceptance corr norm integral: " << hist_thetahf_acceptcorr_norm->Integral() << endl;
    
    // Get weights for all mc events
    auto mc_weight = [&hist_thetahf_norm](double angle_hf){return hist_thetahf_norm->GetBinContent( hist_thetahf_norm->FindBin( angle_hf));};
    auto thrown_weight = [&hist_thetahf_acceptcorr_norm](double angle_hf){return hist_thetahf_acceptcorr_norm->GetBinContent( hist_thetahf_acceptcorr_norm->FindBin( angle_hf));};
    // Get mc and add weight
    auto dfMC2 = dfMC1.Define("mc_weight", thrown_weight, {"kplow_costheta_hf"})
      .Define("tot_weight", "mc_weight*acc_weight");
    cout << "Sum of MC Weights: " << dfMC2.Sum("tot_weight").GetValue() << endl;
    auto dfT2 = dfT1.Define("mc_weight", thrown_weight, {"kp2_costheta_hf"});
    cout << "Sum of Thrown Weights: " << dfT2.Sum("mc_weight").GetValue() << endl;
    
    auto hMC = dfMC2.Histo1D({"","Reconstructed ;kplow_costheta_hf; counts*tot_weight",50u,-1,1},"kplow_costheta_hf","tot_weight");
    auto hT = dfT2.Histo1D({"","Generated;kplow_costheta_hf; counts*mc_weight",50u,-1,1},"kp2_costheta_hf","mc_weight");
    auto h1 = df1.Histo1D("kp_lowp_P3","qacc_decayxim_M");
    auto hMC1 = dfMC2.Histo1D("kp_lowp_P3","tot_weight");
    auto hMC2 = dfMC2.Histo1D("kp_lowp_P3","acc_weight");
    
    cout << "Sum of MC Weighted Events: " << hMC->Integral() << endl;
    // Get acceptance corrected histos after weights
    TH1D *thetahf_acceptance_W = (TH1D*)GetAcceptanceHist1D(hT.GetPtr(), hMC.GetPtr())->Clone("");
    TCanvas *acceptCanWeighted = new TCanvas("acceptCanWeighted","Weighted Acceptance");
    //acceptCanWeighted->Divide(2,1);
    //acceptCanWeighted->cd(1);
    thetahf_acceptance_W->SetTitle("Weighted Acceptance");
    thetahf_acceptance_W->DrawClone("e1");
    TH1D *hist_thetahf_acceptcorr_W = (TH1D*)GetAcceptanceCorrHist1D(hist_thetahf.GetPtr(), thetahf_acceptance_W)->Clone("");
    //acceptCanWeighted->cd(2);
    hist_thetahf_acceptcorr_W->SetTitle("Weighted Acceptance Corrected");
    //hist_thetahf_acceptcorr_W->DrawClone("e1");
    
    TCanvas *weightedCan = new TCanvas("weightedCan","Weighted Reconstructed/Generated");
    weightedCan->Divide(2,1);
    weightedCan->cd(1);
    hMC->DrawClone("hist");
    weightedCan->cd(2);
    hT->DrawClone("hist");
    new TCanvas;
    h1->Scale(hMC1->Integral()/h1->Integral());
    h1->SetLineColor(kRed);
    hMC2->Scale(hMC1->Integral()/hMC2->Integral());
    hMC2->SetLineColor(kGreen);
    h1->DrawClone("hist");
    hMC1->DrawClone("hist same");
    hMC2->DrawClone("hist same");
    
    TCanvas *can = new TCanvas();
    hist_thetahf->SetFillColorAlpha(kGray,0.8);
    hist_thetahf->SetTitle("; cos#theta_{H}; Arbitrary Units");
    hist_thetahf->GetYaxis()->SetTitleOffset(0.8);
    hist_thetahf->GetXaxis()->SetRangeUser(-1.2,1.2);
    //hist_thetahf->GetYaxis()->SetRangeUser(0,200);
    hist_thetahf->DrawClone("hist");
    hMC->Scale((hist_thetahf->Integral()/hMC->Integral()));
    hMC->SetMarkerColor(kAzure);
    hMC->SetMarkerStyle(24);
    hMC->DrawClone("e1 same");
    auto *legend = new TLegend(0.65,0.68,0.949,0.92);
    legend->SetBorderSize(2);
    legend->SetTextSize(0.045);
    legend->SetTextFont(132);
    legend->SetFillColor(0);
    //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
    legend->AddEntry(hist_thetahf.GetPtr(),"Data","f");
    legend->AddEntry(hMC.GetPtr(),"Simulation","lep");
    legend->DrawClone();
    can->SaveAs("/d/grid17/hjesse/pres_plots/nuc_sem_331/KPlusLow_ThetaHF_DatavsMC.png");
    
    // Print Yield Value
    cout << "Q-Value Tree: " << root_file_name.c_str() << endl;
    cout << "Sum of Weighted Q-Values: " << df1.Sum("qacc_decayxim_M").GetValue() << endl;
    cout << endl;
    
    cout << "Writing weighted trees, please wait..." << endl;
    dfMC2.Snapshot("kpkpxim_flatTree", "/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_"+rootMC_file_name+"_Weighted.root");
    dfT2.Snapshot("kpkpxim_thrown_flatTree", "/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_thrown_"+rootThrown_file_name+"_Weighted.root");
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

void setStyle()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  //gStyle->SetCanvasDefH(600);
  //gStyle->SetCanvasDefW(700);
  
  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadTopMargin   (0.1);
  gStyle->SetPadLeftMargin  (0.18);
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
  
  gStyle->SetTitleOffset(0.8,"X");
  gStyle->SetTitleOffset(1.2,"Y");

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
