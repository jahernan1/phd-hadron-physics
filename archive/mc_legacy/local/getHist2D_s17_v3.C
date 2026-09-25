/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file, Int_t n_threads=16);
void save_to_file(string delim="");
TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon);
TH1D* GetAcceptanceCorrHist1D(vector<TH1D*> vec_hist, TFile *save_file);
void WriteAcceptanceHist1D(vector<vector<TH1D*>> vec_hist, TFile* f);

TH2D* GetAcceptanceHist2D(TH2D* hist_genr, TH2D* hist_recon);
TH2D* GetAcceptanceCorrHist2D(vector<TH2D*> vec_hist, TFile *save_file);
void WriteAcceptanceHist2D(vector<vector<TH2D*>> vec_hist, TFile* f);

//main function
int getHist2D_s17_v3()
{
  // save_to_file();//nominal cut values
  // save_to_file("_vertex");//vertex_sig>0
  save_to_file("_ximVertexCut");//xim_vertexsig>2  
  // save_to_file("_vertexCuts");//xim and lambda vertex sig
  // save_to_file("_allCuts");
  // save_to_file("_allKaonSep");
  
  return 0;
}

void save_to_file(string delim="")
{
  //Print statement 
  string file = "data"+delim+"_hist2d_s17_v3.root";
  cout << "Processing " << (file).c_str() << endl;
  //set up root file with directories
  TFile *f =  TFile::Open( (file).c_str(), "RECREATE");
  f->cd();
  gDirectory->mkdir("Spring_2017");
  f->cd();
  gDirectory->mkdir("Spring_2018");  
  f->cd();
  gDirectory->mkdir("Fall_2018");

  //initiate variables
  string root_file_dir = "/d/grid17/hjesse/AnalysisNote/flatTrees/";
  string root_file_dir_thrown = "/d/grid17/hjesse/Trees/flatTree/rawTrees/";
  string root_file_dir_qval = "/d/grid17/hjesse/AnalysisNote/QFactors/logs/";
  //set up histograms for acceptance corrected data histo
  vector<TH2D*> hist_2017;
  vector<TH2D*> hist_201801;
  vector<TH2D*> hist_201808;
    
  //perform actions
  //spring 2017
  f->cd();
  f->cd("Spring_2017");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ana45_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ana45_nominal"+delim+"_1111111.root", "_qval", f);  
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana45_gen_amp_V2_YstarRest_noac_nominal"+delim+".root", "_mc", f);
  save_from_flattrees(root_file_dir_thrown+"flatTree_thrown_kpkpxim__M23_2017-01_ana45_gen_amp_V2_YstarRest_noac.root", "_thrown", f);
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_noac_YstarRest_nominal"+delim+".root", "_mc", f );
  save_from_flattrees(root_file_dir_thrown+"flatTree_thrown_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_noac_YstarRest.root", "_thrown", f);
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_noac_YstarRest_nominal"+delim+".root", "_mc", f);
  save_from_flattrees(root_file_dir_thrown+"flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_noac_YstarRest.root", "_thrown", f);
  
  //change tfile access to read
  f->ReOpen("READ");
  // Perform acceptance correction
  vector<string> vec_delim = {"_qval","_mc","_thrown"};
  for(int i = 0; i < 3; i++)
    {
      hist_2017.push_back( (TH2D*)f->Get( ("Spring_2017/ResMassVsCosTheta"+vec_delim[i] ).c_str())->Clone());
      hist_201801.push_back( (TH2D*)f->Get( ("Spring_2018/ResMassVsCosTheta"+vec_delim[i] ).c_str())->Clone());
      hist_201808.push_back( (TH2D*)f->Get( ("Fall_2018/ResMassVsCosTheta"+vec_delim[i] ).c_str())->Clone());
    }

  f->ReOpen("UPDATE");
  vector<vector<TH2D*>> data_hist{hist_2017,hist_201801,hist_201808};
  WriteAcceptanceHist2D(data_hist, f);
  //add MVsEgamma
  TH2D* hist_M_Egamma_all = (TH2D*)f->Get("Spring_2017/ResMassVsEgamma_qval")->Clone();
  hist_M_Egamma_all->Add((TH2D*)f->Get("Spring_2018/ResMassVsEgamma_qval")->Clone());
  hist_M_Egamma_all->Add((TH2D*)f->Get("Fall_2018/ResMassVsEgamma_qval")->Clone());
  hist_M_Egamma_all->Write(("ResMassVsEgamma_qval_Phase1"),TObject::kOverwrite);

  TH2D* hist_CosTheta_Mass_all = (TH2D*)f->Get("Spring_2017/ResMassVsCosTheta_qval")->Clone();
  hist_CosTheta_Mass_all->Add((TH2D*)f->Get("Spring_2018/ResMassVsCosTheta_qval")->Clone());
  hist_CosTheta_Mass_all->Add((TH2D*)f->Get("Fall_2018/ResMassVsCosTheta_qval")->Clone());
  hist_CosTheta_Mass_all->Write(("ResMassVsCosTheta_qval_Phase1"),TObject::kOverwrite);

  f->Close();
}

void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file, Int_t n_threads=20)
{
  if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
  //Import select braches to speed things up
  //ROOT::RDataFrame df(0);
  // make data frame and braches for histograms from 4 vectors
  // format : tree name, file name, branches to open
  if(hist_name=="_qval")
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str())
          .Filter("qvalue_decayxim_M > 1e-2")
          .Define("qvalue_acc","qvalue_decayxim_M*best_combo");
      
      //.Filter("kp_lowp_P3>0.4");
      //auto df1 = df.Filter("decayxim_M<1.334 && decayxim_M>1.31");
      auto hist_xim_hf = df.Histo1D({""," ; cos#Theta^{Y#Xi}_{#bf{H}} ; Events",200u,-1,1}, "xim_costheta_gen_amp");
      auto hist_xim_hf_qval = df.Histo1D({""," ;  cos#Theta^{Y#Xi}_{#bf{H}}; Events",200u,-1,1}, "xim_costheta_gen_amp","qvalue_acc");
      auto h = df.Histo2D({"MVsE","; beam_E; ystar_M",120,6.,12.,160,1.7,4.5},"beam_E","ystar_M","qvalue_acc");
      auto h1 = df.Histo2D({"MVst","; t_dist; ystar_M",100,0,10 ,160,1.7,4.5},"t_dist","ystar_M","qvalue_acc");
      auto h2 = df.Histo2D({"CosThetaVsE","; beam_E; costheta_hf",120,6.,12.,100,-1,1},"beam_E","xim_costheta_gen_amp","qvalue_acc");
      auto h3 = df.Histo2D({"CosThetaVst","; beam_E; costheta_hf",100,1,10,100,-1,1},"t_dist","xim_costheta_gen_amp","qvalue_acc");
      auto h4 = df.Histo2D({"CosThetaVsMass","; ystar_M; costheta_hf",50,1.8,4.2,50,-1,1},"ystar_M","xim_costheta_gen_amp","qvalue_acc");
      h4->Write(("ResMassVsCosTheta"+hist_name).c_str(),TObject::kOverwrite);
      
      hist_xim_hf->Write(("xim_costheta_gen_amp"),TObject::kOverwrite);
      hist_xim_hf_qval->Write(("xim_costheta_gen_amp"+hist_name).c_str(),TObject::kOverwrite);
      h->Write(("ResMassVsEgamma"+hist_name).c_str(),TObject::kOverwrite);
      h1->Write(("ResMVstdist"+hist_name).c_str(),TObject::kOverwrite);
      h2->Write(("ResCosThetaVsEgamma"+hist_name).c_str(),TObject::kOverwrite);
      h3->Write(("ResCosThetaVst"+hist_name).c_str(),TObject::kOverwrite);
    }
  else if(hist_name=="_mc")
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str());
        //.Filter("kp_lowp_P3>0.4");
      
      auto hist_xim_hf = df.Histo1D({""," ;  cos#Theta^{Y#Xi}_{#bf{H}}; Reconstructed Events",200u,-1,1}, "xim_costheta_gen_amp");
      auto h = df.Histo2D({"MVsE","; beam_E; ystar_M",120,6.,12.,160,1.7,4.5},"beam_E","ystar_M","best_combo");
      auto h1 = df.Histo2D({"MVst","; t_dist; ystar_M",100,0,10 ,160,1.7,4.5},"t_dist","ystar_M","best_combo");
      auto h2 = df.Histo2D({"CosThetaVsE","; beam_E; costheta_hf",120,6.,12.,100,-1,1},"beam_E","xim_costheta_gen_amp","best_combo");
      auto h3 = df.Histo2D({"CosThetaVst","; t_dist; costheta_hf",100,1,10,100,-1,1},"t_dist","xim_costheta_gen_amp","best_combo");
      auto h4 = df.Histo2D({"CosThetaVsMass","; ystar_M; costheta_hf",50,1.8,4.2,50,-1,1},"ystar_M","xim_costheta_gen_amp","best_combo");
      h4->Write(("ResMassVsCosTheta"+hist_name).c_str(),TObject::kOverwrite);
      
      hist_xim_hf->Write(("xim_costheta_gen_amp"+hist_name).c_str(),TObject::kOverwrite);
      h->Write(("ResMassVsEgamma"+hist_name).c_str(),TObject::kOverwrite);
      h1->Write(("ResMVstdist"+hist_name).c_str(),TObject::kOverwrite);
      h2->Write(("ResCosThetaVsEgamma"+hist_name).c_str(),TObject::kOverwrite);
      h3->Write(("ResCosThetaVst"+hist_name).c_str(),TObject::kOverwrite);
    }
  else if(hist_name=="_thrown")
    {
      auto df1 = ROOT::RDataFrame("flatTree_thrown_kpkpxim", (root_file_path).c_str())
        .Define("ystar_M","ystar_p4.M()");
      auto hist_xim_hf = df1.Histo1D({""," ;  cos#Theta^{Y#Xi}_{#bf{H}}; Generated Events",200u,-1,1}, "xim_costheta_gen_amp");
      
      auto h = df1.Histo2D({"MVsE","; beam_E; ystar_M",120,6.,12.,160,1.7,4.5},"beam_E","ystar_M");
      auto h1 = df1.Histo2D({"MVst","; t_dist; ystar_M",100,0,10 ,160,1.7,4.5},"t_dist","ystar_M");
      auto h2 = df1.Histo2D({"CosThetaVsE","; beam_E; costheta_hf",120,6.,12.,100,-1,1},"beam_E","xim_costheta_gen_amp");
    auto h3 = df1.Histo2D({"CosThetaVst","; t_dist; costheta_hf",100,1,10,100,-1,1},"t_dist","xim_costheta_gen_amp");
    auto h4 = df1.Histo2D({"CosThetaVsMass","; ystar_M; costheta_hf",50,1.8,4.2,50,-1,1},"ystar_M","xim_costheta_gen_amp");
      h4->Write(("ResMassVsCosTheta"+hist_name).c_str(),TObject::kOverwrite);
      
      //auto hist_xim_hf_main = df.Histo1D({""," ;  cos#Theta^{Y#Xi}_{#bf{H}}; Generated Events",200u,-1,1}, "xim_costheta_gen_amp","main_pid");
      hist_xim_hf->Write(("xim_costheta_gen_amp"+hist_name).c_str(),TObject::kOverwrite);
      h->Write(("ResMassVsEgamma"+hist_name).c_str(),TObject::kOverwrite);
      h1->Write(("ResMVstdist"+hist_name).c_str(),TObject::kOverwrite);
      h2->Write(("ResCosThetaVsEgamma"+hist_name).c_str(),TObject::kOverwrite);
      h3->Write(("ResCosThetaVst"+hist_name).c_str(),TObject::kOverwrite);
      //hist_xim_hf_main->Write(("xim_costheta_mainpid"+hist_name).c_str(),TObject::kOverwrite);
    }
}

void WriteAcceptanceHist1D(vector<vector<TH1D*>> vec_hist, TFile* f)
{
  f->cd("Spring_2017");
  TH1D* hist_all = (TH1D*)GetAcceptanceCorrHist1D(vec_hist[0], f)->Clone();
  f->cd("Spring_2018");
  TH1D* hist_tmp = (TH1D*)GetAcceptanceCorrHist1D(vec_hist[1], f)->Clone(); 
  hist_all->Add( hist_tmp );
  f->cd("Fall_2018");
  TH1D* hist_tmp2 = (TH1D*)GetAcceptanceCorrHist1D(vec_hist[2], f)->Clone();
  hist_all->Add( hist_tmp2 );
   
  f->cd();
  hist_all->Write("xim_costheta_all_acceptcorr",TObject::kOverwrite);  
}

TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon)
{
  TH1D* hist_accept = (TH1D*)hist_recon->Clone("acceptance");
  hist_accept->GetYaxis()->SetTitle("Acceptance, #epsilon");
  hist_accept->Divide(hist_accept,hist_genr,1,1,"B");
  
  printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
  return hist_accept;
}

//`vect_hist` [0](data),[1](recon),[2](generated)
TH1D* GetAcceptanceCorrHist1D(vector<TH1D*> vec_hist, TFile *save_file)
{
  //get the acceptance
  TH1D* hist_accept = (TH1D*)GetAcceptanceHist1D(vec_hist[2],vec_hist[1])->Clone();
  //hist_accept->Sumw2();
  hist_accept->Write("xim_costheta_acceptance",TObject::kOverwrite);

  char newName[150];
  double binwidth = vec_hist[0]->GetBinWidth(1);
  sprintf(newName,"Events/#epsilon / %.3f GeV", binwidth);
  
  vec_hist[0]->GetYaxis()->SetTitle(newName);
  TH1D* hist_data_acccorr = (TH1D*)vec_hist[0]->Clone();
  //hist_data_acccorr->Sumw2();
  //hist_data_acccorr->Divide(vec_hist[0],hist_accept,1,1,"B");
  hist_data_acccorr->Divide(hist_accept);
  //hist_data_acccorr->Print();
  hist_data_acccorr->Write("xim_costheta_acceptcorr",TObject::kOverwrite);
  
  return hist_data_acccorr;
}

void WriteAcceptanceHist2D(vector<vector<TH2D*>> vec_hist, TFile* f)
{
  f->cd("Spring_2017");
  TH2D* hist_all = (TH2D*)GetAcceptanceCorrHist2D(vec_hist[0], f)->Clone();
  f->cd("Spring_2018");
  TH2D* hist_tmp = (TH2D*)GetAcceptanceCorrHist2D(vec_hist[1], f)->Clone(); 
  hist_all->Add( hist_tmp );
  f->cd("Fall_2018");
  TH2D* hist_tmp2 = (TH2D*)GetAcceptanceCorrHist2D(vec_hist[2], f)->Clone();
  hist_all->Add( hist_tmp2 );
   
  f->cd();
  hist_all->Write("ResMassVsCosTheta_Phase1_ac",TObject::kOverwrite);  
}

TH2D* GetAcceptanceHist2D(TH2D* hist_genr, TH2D* hist_recon)
{
  TH2D* hist_accept = (TH2D*)hist_recon->Clone("acceptance");
  hist_accept->Sumw2(false);
  hist_accept->Divide(hist_genr);

  printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
  return hist_accept;
}

//`vect_hist` [0](data),[1](recon),[2](generated)
TH2D* GetAcceptanceCorrHist2D(vector<TH2D*> vec_hist, TFile *save_file)
{
  //get the acceptance
  TH2D* hist_accept = (TH2D*)GetAcceptanceHist2D(vec_hist[2],vec_hist[1])->Clone();
  hist_accept->Write("costhetahf_ystar_acceptance",TObject::kOverwrite);
  
  TH2D* hist_data_acccorr = (TH2D*)vec_hist[0]->Clone();
  hist_data_acccorr->Sumw2(false);
  
  //hist_data_acccorr->Divide(vec_hist[0],hist_accept,1,1,"B");
  hist_data_acccorr->Divide(hist_accept);
  //hist_data_acccorr->Print();
  hist_data_acccorr->Write("costhetahf_ystar_acceptcorr",TObject::kOverwrite);
  
  return hist_data_acccorr;
}
