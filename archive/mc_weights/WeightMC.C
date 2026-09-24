//#include <RooAbsDataHelper.h>

void setStyle();
void MakeWeightedTree(TF1* fit, TFile* f, Bool_t save=true, string rootMC_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_genr8_nominal", string rootThrown_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_genr8", string delimin="" , Int_t n_threads = 16);
TF1* GetFitFunction(TH1D* hist);
void GetMCWeights(Bool_t save=true, string delim="");

//main
int WeightMC(Bool_t save=true)
{
  // GetMCWeights(save);//nominal cut values
  // GetMCWeights(save, "_vertex");//vertex_sig>0
  // GetMCWeights(save, "_ximVertexCut");//xim_vertexsig>2  
  // GetMCWeights(save, "_vertexCuts");//xim and lambda vertex sig
  // GetMCWeights(save, "_allCuts");
  GetMCWeights(save, "_allKaonSep");
  
  return 0;
}

void GetMCWeights(Bool_t save=true, string delim="")
{
  gStyle->SetOptStat(0);
  //import histo and fit to get weights
  cout << "Getting weights using " << "data"+delim+".root...\n" << endl;
  //set up root file with directories
  TFile *f =  TFile::Open( ("data"+delim+".root").c_str(), "READ");
  TH1D* hist_thetahf = (TH1D*)f->Get("xim_costheta_hf_all_acceptcorr");
  Double_t hist_width = hist_thetahf->GetBinWidth(1);
  cout << "Histogram Counts " << hist_thetahf->Integral() << endl;
  cout << "Histogram Bin Width " << hist_width << endl;
  //Get fit function and add to root file
  TF1* fit = GetFitFunction(hist_thetahf);
  
  f->ReOpen("UPDATE");
  hist_thetahf->Write("xim_costheta_hf_acceptance_fit",TObject::kOverwrite);

  f->cd("Spring_2017/");
  //MakeWeightedTree(fit, f, save, ("kpkpxim__M23_2017-01_ana56_gen_amp_V2_2-1_nominal"+delim).c_str(),"kpkpxim__M23_2017-01_ana56_gen_amp_V2_2-1", delim);
  MakeWeightedTree(fit, f, save, ("kpkpxim__M23_2017-01_ana56_gen_amp_V2_nominal"+delim).c_str(),"kpkpxim__M23_2017-01_ana56_gen_amp_V2", delim);
  f->cd("Spring_2018/");
  MakeWeightedTree(fit, f, save, ("kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_nominal"+delim).c_str(),"kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2", delim);
  f->cd("Fall_2018/");
  MakeWeightedTree(fit, f, save,  ("kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_nominal"+delim).c_str(),"kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2", delim);
  f->Close();

  return 0;
}

TF1* GetFitFunction(TH1D* hist)
{
  //Fit histogram to extract weights from fit
  gStyle->SetOptStat(0);
  //new TCanvas;
  TF1* fit = new TF1("fit","expo(0)+pol2(2)",-1,1);
  fit->SetParameters(8.,1.2,1.2,9.);
  TF1* expo1 = new TF1("expo1","expo",-1,1);
  TF1* expo2 = new TF1("expo2","pol1",-1,1);
  //format functions
  fit->SetLineColor(kBlue);
  expo1->SetLineColor(kBlue);
  expo1->SetLineStyle(2);
  expo2->SetLineColor(kBlue);
  expo2->SetLineStyle(2);
  //fit the histogram 
  hist->Fit("fit","LR");
  // fit->SetNormalized(true);
  //get the parameters for expo1 and expo2
  expo1->SetParameters(fit->GetParameter(0),fit->GetParameter(1));
  expo2->SetParameters(fit->GetParameter(2),fit->GetParameter(3));
  //fit the histogram
  expo1->Draw("same");
  expo2->Draw("same");
  
  return fit;
}

void MakeWeightedTree(TF1* fit, TFile* f, Bool_t save=true, string rootMC_file_name="kpkpxim__M23_2017-01_ver56_gen_amp_nominal", string rootThrown_file_name="kpkpxim__M23_2017-01_ver56_gen_amp", string delim="" , Int_t n_threads = 20) {
    // Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	setStyle();

	// Branches you want to get
	std::vector<std::string> branches = {    };

    // format : tree name, file name, branches to open	
    // make data frames
    auto dfMC = ROOT::RDataFrame("flatTree_kpkpxim", ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_"+rootMC_file_name+".root").c_str());
    auto dfT = ROOT::RDataFrame("flatTree_thrown_kpkpxim", ("/d/grid17/hjesse/Trees/flatTree/rawTrees/flatTree_thrown_"+rootThrown_file_name+".root").c_str())
      .Define("ystar_M","ystar_p4.M()")
      .Define("decayxim_M","decayxim_p4.M()");
      //.Filter("main_pid==1");

    //make histogram 
    //fit normalization
    double fit_integral = fit->Integral(-1,1);
    double fit_norm = fit->GetMaximum()/fit_integral;//makes all weights are from 0 to 1
    //cout << "Fit Function Integral " << fit_integral << endl;
    //cout << fit_norm << endl;
    //
    //Get weights for all mc events
    auto mc_weight = [&fit,&fit_integral,&fit_norm](double angle_hf){return fit->Eval(angle_hf)/fit_integral/fit_norm;};
    
    //get weight and add branch to flattree
    auto dfMC1 = dfMC.Define("mc_weight", mc_weight, {"xim_costheta_hf"});  
    auto dfT1 = dfT.Define("mc_weight", mc_weight, {"xim_costheta_hf"});

    //dfT1.Display({"main_pid","mc_weight", "xim_costheta_hf", "decayxim_M", "ystar_M","t_dist", "beam_E"},50)->Print();

    //add weighted histograms to root tree
    //particle hists
    // auto hist = dfMC1.Histo1D({"",";kp_lowp_p3;",100,0,6},"kp_lowp_P3");
    // auto histw = dfMC1.Histo1D({"",";kp_lowp_p3;",100,0,6},"kp_lowp_P3","mc_weight");
    // auto hist = dfMC1.Histo1D({""," ; cos#Theta^{Y^{*}#Xi}_#bf{{#it{H}}} ; Reconstructed Events",200u,-1,1},"xim_costheta_hf");
    auto histmcw = dfMC1.Histo1D({""," ; cos#Theta^{Y^{*}#Xi}_{#bf{#it{H}}} ; Reconstructed Events",200u,-1,1},"xim_costheta_hf","mc_weight");
    histmcw->Write("xim_costheta_hf_mc_weighted",TObject::kOverwrite);
    auto histthrownw = dfT1.Histo1D({""," ; cos#Theta^{Y^{*}#Xi}_{#bf{#it{H}}} ; Generated Events",200u,-1,1},"xim_costheta_hf","mc_weight");
    histthrownw->Write("xim_costheta_hf_thrown_weighted",TObject::kOverwrite);

    if(save)
      {
        cout << "Writing weighted trees, please wait..." << endl;
        dfMC1.Snapshot("flatTree_kpkpxim", "/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_"+rootMC_file_name+"_Weighted.root");
        dfT1.Snapshot("flatTree_thrown_kpkpxim", "/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_thrown_"+rootThrown_file_name+"_nominal"+delim+"_Weighted.root");
      }
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
