/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file, Int_t n_threads=16);
void save_to_file(string delim="");
TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon);
TH1D* GetAcceptanceCorrHist1D(vector<TH1D*> vec_hist, TFile *save_file);
void WriteAcceptanceHist(vector<vector<TH1D*>> vec_hist, TFile* f);
//main function
int get_data_hists()
{
  // save_to_file();//nominal cut values
  // save_to_file("_vertex");//vertex_sig>0
  // save_to_file("_ximVertexCut");//xim_vertexsig>2  
  // save_to_file("_vertexCuts");//xim and lambda vertex sig
  // save_to_file("_allCuts");
  save_to_file("_allKaonSep");
  
  return 0;
}

void save_to_file(string delim="")
{
  //Print statement 
  cout << "Processing " << ("data"+delim+".root...").c_str() << endl;
  //set up root file with directories
  TFile *f =  TFile::Open( ("data"+delim+".root").c_str(), "RECREATE");
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
  vector<TH1D*> hist_2017;
  vector<TH1D*> hist_201801;
  vector<TH1D*> hist_201808;
    
  //perform actions
  //spring 2017
  f->cd();
  f->cd("Spring_2017");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ana56_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal"+delim+"_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_nominal"+delim+".root", "_mc", f);
  save_from_flattrees(root_file_dir_thrown+"flatTree_thrown_kpkpxim__M23_2017-01_ana56_gen_amp_V2.root", "_thrown", f);
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_nominal"+delim+".root", "_mc", f );
  save_from_flattrees(root_file_dir_thrown+"flatTree_thrown_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2.root", "_thrown", f);
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_nominal"+delim+".root", "_mc", f);
  save_from_flattrees(root_file_dir_thrown+"flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2.root", "_thrown", f);

  //change tfile access to read
  f->ReOpen("READ");
  // Perform acceptance correction
  vector<string> vec_delim = {"_qval","_mc","_thrown"};
  for(int i = 0; i < 3; i++)
    {
      hist_2017.push_back( (TH1D*)f->Get( ("Spring_2017/xim_costheta_hf"+vec_delim[i] ).c_str())->Clone());
      hist_201801.push_back( (TH1D*)f->Get( ("Spring_2018/xim_costheta_hf"+vec_delim[i] ).c_str())->Clone());
      hist_201808.push_back( (TH1D*)f->Get( ("Fall_2018/xim_costheta_hf"+vec_delim[i] ).c_str())->Clone());
    }
  
  f->ReOpen("UPDATE");
  vector<vector<TH1D*>> data_hist{hist_2017,hist_201801,hist_201808};
  WriteAcceptanceHist(data_hist, f);
  f->Close();
}

void WriteAcceptanceHist(vector<vector<TH1D*>> vec_hist, TFile* f)
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
  hist_all->Write("xim_costheta_hf_all_acceptcorr",TObject::kOverwrite);  
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
  hist_accept->Write("xim_costheta_hf_acceptance",TObject::kOverwrite);

  char newName[150];
  double binwidth = vec_hist[0]->GetBinWidth(1);
  sprintf(newName,"Events/#epsilon / %.3f GeV", binwidth);
  
  vec_hist[0]->GetYaxis()->SetTitle(newName);
  TH1D* hist_data_acccorr = (TH1D*)vec_hist[0]->Clone();
  //hist_data_acccorr->Sumw2();
  //hist_data_acccorr->Divide(vec_hist[0],hist_accept,1,1,"B");
  hist_data_acccorr->Divide(hist_accept);
  //hist_data_acccorr->Print();
  hist_data_acccorr->Write("xim_costheta_hf_acceptcorr",TObject::kOverwrite);
  
  return hist_data_acccorr;
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
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str());
      //auto df1 = df.Filter("decayxim_M<1.334 && decayxim_M>1.31");
      auto hist_xim_hf = df.Histo1D({""," ; cos#Theta^{Y#Xi}_{#bf{H}} ; Events",200u,-1,1}, "xim_costheta_hf");
      auto hist_xim_hf_qval = df.Histo1D({""," ;  cos#Theta^{Y#Xi}_{#bf{H}}; Events",200u,-1,1}, "xim_costheta_hf","qvalue_decayxim_M");
      
      hist_xim_hf->Write(("xim_costheta_hf"),TObject::kOverwrite);
      hist_xim_hf_qval->Write(("xim_costheta_hf"+hist_name).c_str(),TObject::kOverwrite);
    }
  else if(hist_name=="_mc")
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str());
      auto hist_xim_hf = df.Histo1D({""," ;  cos#Theta^{Y#Xi}_{#bf{H}}; Reconstructed Events",200u,-1,1}, "xim_costheta_hf");
      hist_xim_hf->Write(("xim_costheta_hf"+hist_name).c_str(),TObject::kOverwrite);
    }
  else if(hist_name=="_thrown")
    {
      auto df1 = ROOT::RDataFrame("flatTree_thrown_kpkpxim", (root_file_path).c_str())
        .Filter("main_pid==1");
      auto hist_xim_hf = df1.Histo1D({""," ;  cos#Theta^{Y#Xi}_{#bf{H}}; Generated Events",200u,-1,1}, "xim_costheta_hf");
      //auto hist_xim_hf_main = df.Histo1D({""," ;  cos#Theta^{Y#Xi}_{#bf{H}}; Generated Events",200u,-1,1}, "xim_costheta_hf","main_pid");
      hist_xim_hf->Write(("xim_costheta_hf"+hist_name).c_str(),TObject::kOverwrite);
      //hist_xim_hf_main->Write(("xim_costheta_hf_mainpid"+hist_name).c_str(),TObject::kOverwrite);
    }
}


