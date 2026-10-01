#include "gxana/common/AcceptanceCorrect.h"
#include "gxana/common/Paths.h"
/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file,  Int_t n_threads=16);
TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon);
TH1D* GetAcceptanceCorrHist1D(vector<TH1D*> vec_hist, TFile *save_file, Bool_t weighted=false);

//main function
int get_data_hists()
{
  //set up root file with directories
  TFile *f =  TFile::Open( "data.root", "RECREATE");
  f->cd();
  gDirectory->mkdir("Spring_2017");
  f->cd();
  gDirectory->mkdir("Spring_2018");  
  f->cd();
  gDirectory->mkdir("Fall_2018");

  //initiate variables
  string root_file_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
  string root_file_dir_qval = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/");
  vector<TH1D*> vect_histo_2017;
  vector<TH1D*> vect_histo_201801;
  vector<TH1D*> vect_histo_201808;
  TH1D* hist_all;
  TH1D* hist_all_weighted;
  vector<string> vec_delim = {"_qval","_mc","_thrown","_mc_weighted","_thrown_weighted"};
  
  //perform actions
  //spring 2017
  f->cd();
  f->cd("Spring_2017");
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ver56_nominal_vertexCuts.root", "", f);
  save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ana56_nominal_allKaonSep_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal_allKaonSep_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_nominal_allKaonSep_Weighted.root", "_mc", f);
  save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__M23_2017-01_ana56_gen_amp_nominal_allKaonSep_Weighted.root", "_thrown", f);
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_allKaonSep.root", "", f);
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ana03_nominal_allKaonSep_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_allKaonSep_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_nominal_allKaonSep_Weighted.root", "_mc", f);
  save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__B4_M23_2018-01_ana03_gen_amp_nominal_allKaonSep_Weighted.root", "_thrown", f);
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_allKaonSep.root", "", f);
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ana02_nominal_allKaonSep_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_allKaonSep_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_nominal_allKaonSep_Weighted.root", "_mc", f);
  save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02_gen_amp_nominal_allKaonSep_Weighted.root", "_thrown", f);

  f->ReOpen("READ");
  //get histos from root tree
  for(int i = 0; i < vec_delim.size(); i++)
    {
      vect_histo_2017.push_back( (TH1D*)f->Get( ("Spring_2017/t_dist"+vec_delim[i] ).c_str())->Clone());
      vect_histo_201801.push_back( (TH1D*)f->Get( ("Spring_2018/t_dist"+vec_delim[i] ).c_str())->Clone());
      vect_histo_201808.push_back( (TH1D*)f->Get( ("Fall_2018/t_dist"+vec_delim[i] ).c_str())->Clone());
    }

  //Perform acceptance correction
  f->ReOpen("UPDATE");
  f->cd("Spring_2017");
  hist_all = (TH1D*)GetAcceptanceCorrHist1D(vect_histo_2017, f)->Clone();
  hist_all_weighted = (TH1D*)GetAcceptanceCorrHist1D(vect_histo_2017, f, true)->Clone();
  //
  f->cd("Spring_2018");
  TH1D* hist_tmp = (TH1D*)GetAcceptanceCorrHist1D(vect_histo_201801, f)->Clone();
  TH1D* hist_tmp_weighted = (TH1D*)GetAcceptanceCorrHist1D(vect_histo_201801, f, true)->Clone(); 
  hist_all->Add( hist_tmp );
  hist_all_weighted->Add( hist_tmp_weighted );
  //
  f->cd("Fall_2018");
  TH1D* hist_tmp2 = (TH1D*)GetAcceptanceCorrHist1D(vect_histo_201808, f)->Clone();
  TH1D* hist_tmp2_weighted = (TH1D*)GetAcceptanceCorrHist1D(vect_histo_201808, f, true)->Clone();
  hist_all->Add( hist_tmp2 );
  hist_all_weighted->Add( hist_tmp2_weighted );
   
  f->cd();
  hist_all->Write("t_dist_all_acceptcorr",TObject::kOverwrite);
  hist_all_weighted->Write("t_dist_all_acceptcorr_weighted",TObject::kOverwrite);  
  f->Close();
  
  return 0;
}

TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon)
{
  TH1D* hist_accept = (TH1D*)gxana::Acceptance(*hist_genr, *hist_recon, "acceptance", gxana::AccErrors::Binomial);
  hist_accept->GetYaxis()->SetTitle("Acceptance, #epsilon");

  printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
  return hist_accept;
}

//`vect_hist` [0](data),[1](recon),[2](generated)
TH1D* GetAcceptanceCorrHist1D(vector<TH1D*> vec_hist, TFile *save_file, Bool_t weighted=false)
{
  TH1D* hist_accept;
  //get the acceptance
  if(weighted)
    { hist_accept = (TH1D*)GetAcceptanceHist1D(vec_hist[4],vec_hist[3])->Clone();//(generated, recon)
      hist_accept->Write( "t_dist_acceptance_weighted",TObject::kOverwrite);}
  else
    { hist_accept = (TH1D*)GetAcceptanceHist1D(vec_hist[2],vec_hist[1])->Clone();
      hist_accept->Write( "t_dist_acceptance",TObject::kOverwrite);}
  //hist_accept->Sumw2();

  char newName[150];
  double binwidth = vec_hist[0]->GetBinWidth(1);
  sprintf(newName,"Events/#epsilon / %.2f GeV", binwidth);
  
  vec_hist[0]->GetYaxis()->SetTitle(newName);
  TH1D* hist_data_acccorr = (TH1D*)gxana::AcceptanceCorrect(*vec_hist[0], *hist_accept, vec_hist[0]->GetName(), gxana::AccErrors::Plain);
  if(weighted)
    hist_data_acccorr->Write( "t_dist_acceptcorr_weighted",TObject::kOverwrite);
  else
    hist_data_acccorr->Write( "t_dist_acceptcorr",TObject::kOverwrite);
  
  return hist_data_acccorr;
}

/* ****************************************************************************************************
***************************************************************************************************** 
**************************************************************************************************** */ 
void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file, Int_t n_threads=20)
{
  if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
  //Import select braches to speed things up
    
  // make data frame and braches for histograms from 4 vectors
  // format : tree name, file name, branches to open
  if(hist_name=="_thrown")
    {
      auto df = ROOT::RDataFrame("flatTree_thrown_kpkpxim", (root_file_path).c_str(), {"t_dist", "xim_costheta_hf", "ystar_M"});
      //make histograms and add to tfile
      auto hist_tdist = df.Histo1D({""," ; -t (GeV)^{2}; Events",45u,0,5}, "t_dist");
      auto hist_tdist_weighted = df.Histo1D({""," ; -t (GeV)^{2}; Events",45u,0,5}, "t_dist","mc_weight");
      auto hist_ystar_M = df.Histo1D({""," ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events",100u,1.7,4.2}, "ystar_M");
      auto hist_ystar_M_weighted = df.Histo1D({""," ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events",100u,1.7,4.2}, "ystar_M", "mc_weight");
      auto hist_costheta_weighted = df.Histo1D({""," ; -t (GeV)^{2}; Events",50u,-1,1}, "xim_costheta_hf","mc_weight");
      //
      hist_tdist->Write(("t_dist"+hist_name).c_str(),TObject::kOverwrite);
      hist_tdist_weighted->Write(("t_dist"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
      hist_ystar_M->Write(("ystar_M"+hist_name).c_str(),TObject::kOverwrite);
      hist_ystar_M_weighted->Write(("ystar_M"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
      hist_costheta_weighted->Write(("xim_costheta_hf"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
    }
  else if(hist_name=="_mc")
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"t_dist", "xim_costheta_hf", "ystar_M"});
      //make histograms and add to tfile
      auto hist_tdist = df.Histo1D({""," ; -t (GeV)^{2}; Events",45u,0,5}, "t_dist");
      auto hist_tdist_weighted = df.Histo1D({""," ; -t (GeV)^{2}; Events",45u,0,5}, "t_dist","mc_weight");
      auto hist_ystar_M = df.Histo1D({""," ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events",100u,1.7,4.2}, "ystar_M");
      auto hist_ystar_M_weighted = df.Histo1D({""," ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events",100u,1.7,4.2}, "ystar_M", "mc_weight");
      auto hist_costheta_weighted = df.Histo1D({""," ; -t (GeV)^{2}; Events",50u,-1,1}, "xim_costheta_hf","mc_weight");
      //
      hist_tdist->Write(("t_dist"+hist_name).c_str(),TObject::kOverwrite);
      hist_tdist_weighted->Write(("t_dist"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
      hist_ystar_M->Write(("ystar_M"+hist_name).c_str(),TObject::kOverwrite);
      hist_ystar_M_weighted->Write(("ystar_M"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
      hist_costheta_weighted->Write(("xim_costheta_hf"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
    }
  else
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"t_dist", "xim_costheta_hf", "ystar_M"});
      auto df1 = df.Filter("decayxim_M<1.334 && decayxim_M>1.31");        
      //make histograms and add to tfile
      auto hist_tdist = df1.Histo1D({""," ; -t (GeV)^{2}; Events",45u,0,5}, "t_dist");
      auto hist_tdist_qval = df.Histo1D({""," ; -t (GeV)^{2}; Events",45u,0,5}, "t_dist","qvalue_decayxim_M");
      auto hist_ystar_M = df1.Histo1D({""," ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events",100u,1.7,4.2}, "ystar_M");
      auto hist_ystar_M_qval = df.Histo1D({""," ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events",100u,1.7,4.2}, "ystar_M","qvalue_decayxim_M");
      auto hist_costheta_qval = df.Histo1D({""," ; -t (GeV)^{2}; Events",50u,-1,1}, "xim_costheta_hf","qvalue_decayxim_M");
      //
      hist_tdist->Write("t_dist",TObject::kOverwrite);
      hist_tdist_qval->Write(("t_dist"+hist_name).c_str(),TObject::kOverwrite);
      hist_ystar_M->Write("ystar_M",TObject::kOverwrite);
      hist_ystar_M_qval->Write(("ystar_M"+hist_name).c_str(),TObject::kOverwrite);
      hist_costheta_qval->Write(("xim_costheta_hf"+hist_name).c_str(),TObject::kOverwrite);
    } 
}
