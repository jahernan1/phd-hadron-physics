#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

void GetPrepedFlatTree(string root_file_name="kpkpxim__M23_2017-01_ver56_nominal", string mc_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_genr8_nominal_Weighted", string thrown_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_genr8_Weighted", int n_threads = 8 );
void GetHistos(string delim="_allCuts");
TH2D* GetAcceptanceCorrHist2D(vector<TH2D*> vec_hist, TFile *save_file);
TH2D* GetAcceptanceHist2D(TH2D* hist_genr, TH2D* hist_recon);

//main
int exampleHist2D()
{
  // GetHistos("_vertex");
  // GetHistos("_ximVertexCut");
  // GetHistos("_vertexCuts");
  // GetHistos();
  GetHistos("_allKaonSep");
    
  return 0;
}

void GetHistos(string delim="_allCuts")
{ 
  // Get the tree for the xsection
  GetPrepedFlatTree("kpkpxim__M23_2017-01_ana56_nominal"+delim,"kpkpxim__M23_2017-01_ana56_Ystar2400_1600_genr8_nominal"+delim+"_Weighted", "kpkpxim__M23_2017-01_ana56_Ystar2400_1600_genr8_nominal"+delim+"_Weighted");
  GetPrepedFlatTree("kpkpxim__B4_M23_2018-01_ana03_nominal"+delim,"kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600_genr8_nominal"+delim+"_Weighted","kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600_genr8_nominal"+delim+"_Weighted");
  GetPrepedFlatTree("kpkpxim__B4_M23_2018-08_ana02_nominal"+delim,"kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600_genr8_nominal"+delim+"_Weighted","kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600_genr8_nominal"+delim+"_Weighted");
}

void GetPrepedFlatTree(string root_file_name="kpkpxim__M23_2017-01_ana56_nominal", string mc_file_name="kpkpxim__M23_2017-01_ana56_Ystar2400_1600_genr8_nominal", string thrown_file_name="kpkpxim__M23_2017-01_ana56_Ystar2400_1600_genr8", int n_threads = 16 ) {
    cout << "Prepping cross section component histograms for " << root_file_name << endl;
    // Parallelize with n threads
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
    string outputDir = "/d/grid17/hjesse/AnalysisNote/xsection/prepped_trees/";
    TObjArray hList(0);
    
    // Branches you want to use get
    std::vector<std::string> branches = {"beam_E",
                                         "ystar_M",
                                         "t_dist",
                                         "xim_costheta_hf",
                                         
    };
    std::vector<std::string> branchesMC = branches; branchesMC.push_back("mc_weight");
    branches.push_back("qvalue_decayxim_M");
    
	// make data frame
	// format : tree name, file name, branches to open
    auto df = ROOT::RDataFrame("flatTree_kpkpxim", ("/d/grid17/hjesse/AnalysisNote/QFactors/logs/"+root_file_name+"_1111111/postQVal_flatTree_"+root_file_name+"_1111111.root").c_str(), branches);
    auto dfMC = ROOT::RDataFrame("flatTree_kpkpxim", ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_"+mc_file_name+".root").c_str(), branchesMC);
    auto dfT = ROOT::RDataFrame("flatTree_thrown_kpkpxim", ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_thrown_"+thrown_file_name+".root").c_str(), branchesMC);

     // Make histogram
    auto h = df.Histo2D({"MVsE","; beam_E; ystar_M",120,6.,12.,140,1.7,4.5},"beam_E","ystar_M","qvalue_decayxim_M");
    auto h1 = df.Histo2D({"MVst","; t_dist; ystar_M",100,0,10 ,140,1.7,4.5},"t_dist","ystar_M","qvalue_decayxim_M");
    auto h2 = df.Histo2D({"CosThetaVsE","; beam_E; costheta_hf",120,6.,12.,100,-1,1},"beam_E","xim_costheta_hf","qvalue_decayxim_M");
    auto h2MC = dfMC.Histo2D({"CosThetaVsE","; beam_E; costheta_hf",120,6.,12.,100,-1,1},"beam_E","xim_costheta_hf");
    auto h2T = dfT.Histo2D({"CosThetaVsE","; beam_E; costheta_hf",120,6.,12.,100,-1,1},"beam_E","xim_costheta_hf");
    
    // Save histograms to trees for xsection
    TFile *outputFile = new TFile("exampleHist2D.root", "RECREATE");
    auto h2_accept = (TH2D*)GetAcceptanceCorrHist2D({h2.GetPtr(), h2MC.GetPtr(), h2T.GetPtr()},outputFile)->Clone();
    h2_accept->DrawClone("colz");
    
    h->Write();
    h1->Write();
    h2_accept->Write("CosTheta_Egamma");
    outputFile->Close();
}

TH2D* GetAcceptanceHist2D(TH2D* hist_genr, TH2D* hist_recon)
{
  TH2D* hist_accept = (TH2D*)hist_recon->Clone("acceptance");
  //hist_accept->GetYaxis()->SetTitle("Acceptance, #epsilon");
  hist_accept->Divide(hist_accept,hist_genr,1,1,"B");
  
  printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
  return hist_accept;
}

//`vect_hist` [0](data),[1](recon),[2](generated)
TH2D* GetAcceptanceCorrHist2D(vector<TH2D*> vec_hist, TFile *save_file)
{
  //get the acceptance
  TH2D* hist_accept = (TH2D*)GetAcceptanceHist2D(vec_hist[2],vec_hist[1])->Clone();
  //hist_accept->Sumw2();
  hist_accept->Write("costheta_hf_egamma_acceptance",TObject::kOverwrite);

  // char newName[150];
  // double binwidth = vec_hist[0]->GetBinWidth(1);
  // sprintf(newName,"Events/#epsilon / %.3f GeV", binwidth);
  
  //vec_hist[0]->GetYaxis()->SetTitle(newName);
  TH2D* hist_data_acccorr = (TH2D*)vec_hist[0]->Clone();
  //hist_data_acccorr->Sumw2();
  //hist_data_acccorr->Divide(vec_hist[0],hist_accept,1,1,"B");
  hist_data_acccorr->Divide(hist_accept);
  //hist_data_acccorr->Print();
  hist_data_acccorr->Write("costheta_hf_egamma",TObject::kOverwrite);
  
  return hist_data_acccorr;
}
