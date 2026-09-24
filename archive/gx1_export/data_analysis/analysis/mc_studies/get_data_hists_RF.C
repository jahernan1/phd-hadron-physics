/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file,  Int_t n_threads=16);
TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon);
TH1D* GetAcceptanceCorrHist1D(vector<TH1D*> vec_hist, TFile *save_file, Bool_t weighted=false);

//main function
int get_data_hists_RF()
{
  //set up root file with directories
  TFile *f =  TFile::Open( "data_ac_hist2d_kphighrap.root", "RECREATE");
  f->cd();
  gDirectory->mkdir("Spring_2017");
  f->cd();
  gDirectory->mkdir("Spring_2018");  
  f->cd();
  gDirectory->mkdir("Fall_2018");

  //initiate variables
  string root_file_dir = "/d/grid17/hjesse/AnalysisNote/flatTrees/";
  string root_file_dir_qval = "/d/grid17/hjesse/AnalysisNote/QFactors/logs/";
  vector<TH1D*> vect_histo_2017;
  vector<TH1D*> vect_histo_201801;
  vector<TH1D*> vect_histo_201808;
  vector<TH1D*> vect_costheta_2017;
  vector<TH1D*> vect_costheta_201801;
  vector<TH1D*> vect_costheta_201808;
  vector<TH1D*> vect_ystar_2017;
  vector<TH1D*> vect_ystar_201801;
  vector<TH1D*> vect_ystar_201808;

  TH1D* hist_all;  TH1D* hist_all_costheta;TH1D* hist_all_ystar;
  vector<string> vec_delim = {"_qval","_mc","_thrown"};
  
  //perform actions
  //spring 2017
  f->cd();
  f->cd("Spring_2017");
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ver56_nominal_vertexCuts.root", "", f);
  save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ana56_nominal_kphighrap_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root", "_mc", f);
  save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest.root", "_thrown", f);
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap.root", "", f);
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root", "_mc", f);
  save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest.root", "_thrown", f);
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap.root", "", f);
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root", "_mc", f);
  save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest.root", "_thrown", f);

  f->ReOpen("READ");
  //get histos from root tree
  for(int i = 0; i < vec_delim.size(); i++)
    {
      vect_histo_2017.push_back( (TH1D*)f->Get( ("Spring_2017/t_dist"+vec_delim[i] ).c_str())->Clone( ("tdist"+vec_delim[i]).c_str()));
      vect_histo_201801.push_back( (TH1D*)f->Get( ("Spring_2018/t_dist"+vec_delim[i] ).c_str())->Clone( ("tdist"+vec_delim[i]).c_str()));
      vect_histo_201808.push_back( (TH1D*)f->Get( ("Fall_2018/t_dist"+vec_delim[i] ).c_str())->Clone( ("tdist"+vec_delim[i]).c_str()));
      vect_costheta_2017.push_back( (TH1D*)f->Get( ("Spring_2017/xim_costheta_gen_amp"+vec_delim[i] ).c_str())->Clone(("costheta"+vec_delim[i]).c_str()));
      vect_costheta_201801.push_back( (TH1D*)f->Get( ("Spring_2018/xim_costheta_gen_amp"+vec_delim[i] ).c_str())->Clone(("costheta"+vec_delim[i]).c_str()));
      vect_costheta_201808.push_back( (TH1D*)f->Get( ("Fall_2018/xim_costheta_gen_amp"+vec_delim[i] ).c_str())->Clone(("costheta"+vec_delim[i]).c_str()));
      vect_ystar_2017.push_back( (TH1D*)f->Get( ("Spring_2017/ystar_M"+vec_delim[i] ).c_str())->Clone(("ystarM"+vec_delim[i]).c_str()));
      vect_ystar_201801.push_back( (TH1D*)f->Get( ("Spring_2018/ystar_M"+vec_delim[i] ).c_str())->Clone(("ystarM"+vec_delim[i]).c_str()));
      vect_ystar_201808.push_back( (TH1D*)f->Get( ("Fall_2018/ystar_M"+vec_delim[i] ).c_str())->Clone(("ystarM"+vec_delim[i]).c_str()));
      
    }
  vect_histo_2017[0]->Draw();
  
  //Perform acceptance correction
  f->ReOpen("UPDATE");
  f->cd("Spring_2017");
  hist_all = (TH1D*)GetAcceptanceCorrHist1D(vect_histo_2017, f)->Clone("tdist_acccorr");
  hist_all_costheta = (TH1D*)GetAcceptanceCorrHist1D(vect_costheta_2017, f)->Clone("costheta_acccorr");
  hist_all_ystar = (TH1D*)GetAcceptanceCorrHist1D(vect_ystar_2017, f)->Clone("ystarM_acccorr");
  //
  f->cd("Spring_2018");
  TH1D* hist_tmp = (TH1D*)GetAcceptanceCorrHist1D(vect_histo_201801, f);
  hist_all->Add( hist_tmp );
  hist_all_costheta->Add((TH1D*)GetAcceptanceCorrHist1D(vect_costheta_201801, f));
  hist_all_ystar->Add((TH1D*)GetAcceptanceCorrHist1D(vect_ystar_201801, f));
  //
  f->cd("Fall_2018");
  TH1D* hist_tmp2 = (TH1D*)GetAcceptanceCorrHist1D(vect_histo_201808, f);
  hist_all->Add( hist_tmp2 );
  hist_all_costheta->Add((TH1D*)GetAcceptanceCorrHist1D(vect_costheta_201808, f));
  hist_all_ystar->Add((TH1D*)GetAcceptanceCorrHist1D(vect_ystar_201808, f));
  
  f->cd();
  hist_all->Write("tdist_all_acceptcorr",TObject::kOverwrite);
  hist_all_costheta->Write("costheta_all_acceptcorr",TObject::kOverwrite);
  hist_all_ystar->Write("ystarM_all_acceptcorr",TObject::kOverwrite);
  //
  TH1D* costheta_thrown_merged = (TH1D*)vect_costheta_2017[2]->Clone();
  costheta_thrown_merged->Add( (TH1D*)vect_costheta_201801[2]->Clone() );
  costheta_thrown_merged->Add( (TH1D*)vect_costheta_201808[2]->Clone() );
  costheta_thrown_merged->Write("costheta_all_thrown",TObject::kOverwrite);
  //
  TH1D* ystar_thrown_merged = (TH1D*)vect_ystar_2017[2]->Clone();
  ystar_thrown_merged->Add( (TH1D*)vect_ystar_201801[2]->Clone() );
  ystar_thrown_merged->Add( (TH1D*)vect_ystar_201808[2]->Clone() );
  ystar_thrown_merged->Write("ystarM_all_thrown",TObject::kOverwrite);
  //
  TH1D* tdist_thrown_merged = (TH1D*)vect_histo_2017[2]->Clone();
  tdist_thrown_merged->Add( (TH1D*)vect_histo_201801[2]->Clone() );
  tdist_thrown_merged->Add( (TH1D*)vect_histo_201808[2]->Clone() );
  tdist_thrown_merged->Write("tdist_all_thrown",TObject::kOverwrite);

  f->Close();
  
  return 0;
}

TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon)
{
  TH1D* hist_accept = (TH1D*)hist_recon->Clone("acceptance");
  hist_accept->Reset();
  hist_accept->GetYaxis()->SetTitle("Acceptance, #varepsilon");
  hist_accept->Divide(hist_recon,hist_genr,1,1);
  
  printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
  return hist_accept;
}

//`vect_hist` [0](data),[1](recon),[2](generated)
TH1D* GetAcceptanceCorrHist1D(vector<TH1D*> vec_hist, TFile *save_file, Bool_t weighted=false)
{
  TH1D* hist_accept;
  //get the acceptance
  string name = vec_hist[0]->GetName();
  if(weighted)
    { hist_accept = (TH1D*)GetAcceptanceHist1D(vec_hist[4],vec_hist[3])->Clone();//(generated, recon)
      hist_accept->Write( "t_dist_acceptance_weighted",TObject::kOverwrite);}
  else
    { hist_accept = (TH1D*)GetAcceptanceHist1D(vec_hist[2],vec_hist[1])->Clone();
      hist_accept->Write( (name+"_acceptance").c_str(),TObject::kOverwrite);}
  //hist_accept->Sumw2();

  char newName[150];
  double binwidth = vec_hist[0]->GetBinWidth(1);
  sprintf(newName,"Events/#varepsilon / %.2f GeV", binwidth);
  
  vec_hist[0]->GetYaxis()->SetTitle(newName);
  TH1D* hist_data_acccorr = (TH1D*)vec_hist[0]->Clone();
  hist_data_acccorr->Reset();
  //hist_data_acccorr->Sumw2();
  //hist_data_acccorr->Divide(vec_hist[0],hist_accept,1,1,"B");
  hist_data_acccorr->Divide(vec_hist[0],hist_accept);
  if(weighted)
    hist_data_acccorr->Write( "t_dist_acceptcorr_weighted",TObject::kOverwrite);
  else
    hist_data_acccorr->Write( (name+"_acceptcorr").c_str(),TObject::kOverwrite);
  
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
      auto df = ROOT::RDataFrame("flatTree_thrown_kpkpxim", (root_file_path).c_str(), {"t_dist", "xim_costheta_gen_amp", "ystar_p4"})
        .Define("ystar_M","ystar_p4.M()");
      //make histograms and add to tfile
      auto hist_tdist = df.Histo1D({""," ; -t (GeV)^{2} ); Events",50u,0,5}, "t_dist");
      auto hist_ystar_M = df.Histo1D({""," ; M(#Xi^{-}K^{+}_{#it{#lower[-0.3]{S}}}) (GeV); Events",50u,1.8,4.2}, "ystar_M");
      auto hist_costheta = df.Histo1D({""," ; cos#theta_{H}; Events",50u,-1,1}, "xim_costheta_gen_amp");
      //
      hist_tdist->Write(("t_dist"+hist_name).c_str(),TObject::kOverwrite);
      hist_ystar_M->Write(("ystar_M"+hist_name).c_str(),TObject::kOverwrite);
      hist_costheta->Write(("xim_costheta_gen_amp"+hist_name).c_str(),TObject::kOverwrite);
    }
  else if(hist_name=="_mc")
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"t_dist_truth", "xim_costheta_gen_amp", "ystar_M", "best_combo"});
      //make histograms and add to tfile
      auto hist_tdist = df.Histo1D({""," ; -t (GeV)^{2}; Events",50u,0,5}, "t_dist_truth", "best_combo");
      auto hist_ystar_M = df.Histo1D({""," ; M(#Xi^{-}K^{+}_{#it{#lower[-0.3]{S}}}) (GeV); Events",50u,1.8,4.2}, "ystar_M","best_combo");
      auto hist_costheta = df.Histo1D({""," ; cos#theta_{H}; Events",50u,-1,1}, "xim_costheta_gen_amp","best_combo");
      //
      hist_tdist->Write(("t_dist"+hist_name).c_str(),TObject::kOverwrite);
      hist_ystar_M->Write(("ystar_M"+hist_name).c_str(),TObject::kOverwrite);
      hist_costheta->Write(("xim_costheta_gen_amp"+hist_name).c_str(),TObject::kOverwrite);
    }
  else
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"t_dist", "xim_costheta_gen_amp", "ystar_M","best_combo","qvalue_decayxim_M"})
        .Define("qvalue_hybrid","qvalue_decayxim_M*best_combo");
      auto df1 = df.Filter("decayxim_M<1.334 && decayxim_M>1.31");        
      //make histograms and add to tfile
      auto hist_tdist = df1.Histo1D({""," ; -t (GeV)^{2}; Events",50u,0,5}, "t_dist","best_combo");
      auto hist_tdist_qval = df.Histo1D({""," ; -t (GeV)^{2}; Events",50u,0,5}, "t_dist","qvalue_hybrid");
      auto hist_ystar_M = df1.Histo1D({""," ; M(#Xi^{-}K^{+}_{#it{#lower[-0.3]{S}}}) (GeV); Events",50u,1.8,4.2}, "ystar_M","best_combo");
      auto hist_ystar_M_qval = df.Histo1D({""," ; M(#Xi^{-}K^{+}_{#it{#lower[-0.3]{S}}}) (GeV); Events",50u,1.8,4.2}, "ystar_M","qvalue_hybrid");
      auto hist_costheta_qval = df.Histo1D({""," ; cos#theta_{H}; Events",50u,-1,1}, "xim_costheta_gen_amp","qvalue_hybrid");
      //
      hist_tdist->Write("t_dist",TObject::kOverwrite);
      hist_tdist_qval->Write(("t_dist"+hist_name).c_str(),TObject::kOverwrite);
      hist_ystar_M->Write("ystar_M",TObject::kOverwrite);
      hist_ystar_M_qval->Write(("ystar_M"+hist_name).c_str(),TObject::kOverwrite);
      hist_costheta_qval->Write(("xim_costheta_gen_amp"+hist_name).c_str(),TObject::kOverwrite);
    } 
}
