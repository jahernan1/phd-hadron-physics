#include "gxana/common/AcceptanceCorrect.h"
#include "gxana/common/Paths.h"
/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file, Int_t n_threads=16);
void save_to_file(string delim="");

TH3F* GetAcceptanceHist3d(TH3F* hist_genr, TH3F* hist_recon);
TH3F* GetAcceptanceCorrHist3d(vector<TH3F*> vec_hist, TFile *save_file);
void WriteAcceptanceHist3d(vector<TH3F*> vec_hist, TFile* f);

//main function
int getHist3D_F18()
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
  cout << "Processing " << ("data"+delim+"_F18_hist3d.root...").c_str() << endl;
  //set up root file with directories
  TFile *f =  TFile::Open( ("data"+delim+"_F18_hist3d.root").c_str(), "RECREATE");
  f->cd();
  gDirectory->mkdir("Spring_2017");
  f->cd();
  gDirectory->mkdir("Spring_2018");  
  f->cd();
  gDirectory->mkdir("Fall_2018");

  //initiate variables
  string root_file_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
  string root_file_dir_thrown = gxana::EnvPath("GXANA_DATA", "flatTrees/");
  string root_file_dir_qval = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/");
  //set up histograms for acceptance corrected data histo
  vector<TH3F*> hist_2017;
  vector<TH3F*> hist_201801;
  vector<TH3F*> hist_201808;
    
  //perform actions
  //spring 2017
  // f->cd();
  // f->cd("Spring_2017");
  // save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ana56_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal"+delim+"_1111111.root", "_qval", f);
  // save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_nominal"+delim+".root", "_mc", f);
  // save_from_flattrees(root_file_dir_thrown+"flatTree_thrown_kpkpxim__M23_2017-01_ana56_gen_amp_V2.root", "_thrown", f);
  // //spring 2018
  // f->cd();
  // f->cd("Spring_2018");
  // save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111.root", "_qval", f);
  // save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_nominal"+delim+".root", "_mc", f );
  // save_from_flattrees(root_file_dir_thrown+"flatTree_thrown_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2.root", "_thrown", f);
  // //fall 2018
  f->cd();
  f->cd("Fall_2018");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ana02_nominalBC"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominalBC"+delim+"_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_3D_mask011_nominalBC"+delim+".root", "_mc", f);
  save_from_flattrees(root_file_dir_thrown+"flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_3D_mask011.root", "_thrown", f);

  //change tfile access to read
  f->ReOpen("READ");
  // Perform acceptance correction
  vector<string> vec_delim = {"_qval","_mc","_thrown"};
  for(int i = 0; i < vec_delim.size(); i++)
    {
      hist_201808.push_back( (TH3F*)f->Get( ("Fall_2018/ResMassVsCosThetaVsT"+vec_delim[i] ).c_str())->Clone());
    }
  
  f->ReOpen("UPDATE");
  //vector<vector<TH3F*>> data_hist{hist_201808};
  WriteAcceptanceHist3d(hist_201808, f);
  //add MVsEgamma
  TH3F* hist_M_Egamma_all = (TH3F*)hist_201808[0]->Clone();
  //hist_M_Egamma_all->Add((TH3F*)hist_201801[0]->Clone());
  //hist_M_Egamma_all->Add((TH3F*)hist_201808[0]->Clone());
  hist_M_Egamma_all->Write(("ResMassVsCosThetaVsT_Phase1_noac"),TObject::kOverwrite);

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
        .Define("qvalue_acc","qvalue_decayxim_M");
      
      //.Filter("kp_lowp_P3>0.4");
      //auto df1 = df.Filter("decayxim_M<1.334 && decayxim_M>1.31");
      auto h4 = df.Histo3D({"ResMassVsCosThetaVsBeamE","; ystar_M; costheta_hf; beam_E",100,1.8,4.2,100,-1,1,100,6,12},"ystar_M","xim_costheta_hf","beam_E","qvalue_acc");
      h4->Write(("ResMassVsCosThetaVsBeamE"+hist_name).c_str(),TObject::kOverwrite);
      auto h5 = df.Histo3D({"ResMassVsCosThetaVsT","; ystar_M; costheta_hf; t_dist",100,1.8,4.2,100,-1,1,100,0,12},"ystar_M","xim_costheta_hf","t_dist","qvalue_acc");
      
      h5->Write(("ResMassVsCosThetaVsT"+hist_name).c_str(),TObject::kOverwrite);
    }
  else if(hist_name=="_mc")
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str())
        .Define("abs_tdist_truth","abs(t_dist_truth)");
        //.Filter("kp_lowp_P3>0.4");
      
      auto h4 = df.Histo3D({"ResMassVsCosThetaVsBeamE","; ystar_M; costheta_hf; beam_E",100,1.8,4.2,100,-1,1,100,6,12},"ystar_M","xim_costheta_hf","beam_E");
      h4->Write(("ResMassVsCosThetaVsBeamE"+hist_name).c_str(),TObject::kOverwrite);
      auto h5 = df.Histo3D({"ResMassVsCosThetaVsT","; ystar_M; costheta_hf; t_dist",100,1.8,4.2,100,-1,1,100,0,12},"ystar_M","xim_costheta_hf","abs_tdist_truth");
      h5->Write(("ResMassVsCosThetaVsT"+hist_name).c_str(),TObject::kOverwrite); 
    }
  else if(hist_name=="_thrown")
    {
      auto df1 = ROOT::RDataFrame("flatTree_thrown_kpkpxim", (root_file_path).c_str())
        .Define("ystar_M","ystar_p4.M()");
      
      auto h4 = df1.Histo3D({"ResMassVsCosThetaVsBeamE","; ystar_M; costheta_hf; beam_E",100,1.8,4.2,100,-1,1,100,6,12},"ystar_M","xim_costheta_hf","beam_E");
      h4->Write(("ResMassVsCosThetaVsBeamE"+hist_name).c_str(),TObject::kOverwrite);
      auto h5 = df1.Histo3D({"ResMassVsCosThetaVsT","; ystar_M; costheta_hf; t_dist",100,1.8,4.2,100,-1,1,100,0,12},"ystar_M","xim_costheta_hf","t_dist");
      h5->Write(("ResMassVsCosThetaVsT"+hist_name).c_str(),TObject::kOverwrite);
    }
}

void WriteAcceptanceHist3d(vector<TH3F*> vec_hist, TFile* f)
{
  // f->cd("Spring_2017");
  // TH3F* hist_all = (TH3F*)GetAcceptanceCorrHist3d(vec_hist[0], f)->Clone();
  // f->cd("Spring_2018");
  // TH3F* hist_tmp = (TH3F*)GetAcceptanceCorrHist3d(vec_hist[1], f)->Clone(); 
  // hist_all->Add( hist_tmp );
  f->cd("Fall_2018");
  // TH3F* hist_tmp2 = (TH3F*)GetAcceptanceCorrHist3d(vec_hist[2], f)->Clone();
  // hist_all->Add( hist_tmp2 );
  TH3F* hist_all = (TH3F*)GetAcceptanceCorrHist3d(vec_hist, f)->Clone();
 
  f->cd();
  hist_all->Write("ResMassVsCosThetaVsT_Phase1",TObject::kOverwrite);  
}

TH3F* GetAcceptanceHist3d(TH3F* hist_genr, TH3F* hist_recon)
{
  TH3F* hist_accept = (TH3F*)gxana::Acceptance(*hist_genr, *hist_recon, "acceptance", gxana::AccErrors::Binomial);
  return hist_accept;
}

//`vect_hist` [0](data),[1](recon),[2](generated)
TH3F* GetAcceptanceCorrHist3d(vector<TH3F*> vec_hist, TFile *save_file)
{
  //get the acceptance
  TH3F* hist_accept = (TH3F*)GetAcceptanceHist3d(vec_hist[2],vec_hist[1])->Clone("acceptance");//vec_hist[1]->Clone("acceptance");
  //hist_accept->Divide(vec_hist[2]);
  hist_accept->Sumw2();
  hist_accept->Write("acceptance",TObject::kOverwrite);

  string name = vec_hist[0]->GetName();
  TH3F* hist_data_acccorr = (TH3F*)gxana::AcceptanceCorrect(*vec_hist[0], *hist_accept, (name+"_acceptcorr").c_str(), gxana::AccErrors::Plain);
  hist_data_acccorr->Write((name+"_acceptcorr").c_str(),TObject::kOverwrite);
  
  return hist_data_acccorr;
}

//`vect_hist` [0](data),[1](recon),[2](generated)
TH3F* GetAcceptanceCorrHist3dByBin(vector<TH3F*> vec_hist)
{
  //get the acceptance
  TH3F* hist_accept = (TH3F*)vec_hist[1]->Clone("acceptance");
  hist_accept->Divide(vec_hist[2]);
  //hist_accept->Sumw2();
  hist_accept->Write("acceptance",TObject::kOverwrite);

  string name = vec_hist[0]->GetName();
  TH3F* hist_data_acccorr = (TH3F*)vec_hist[0]->Clone((name+"_acceptcorr").c_str());

  hist_data_acccorr->Sumw2();
  hist_data_acccorr->Divide(hist_accept);
  
  //hist_data_acccorr->Divide(hist_accept);
  //hist_data_acccorr->Print();
  hist_data_acccorr->Write((name+"_acceptcorr_bin").c_str(),TObject::kOverwrite);
  
  return hist_data_acccorr;
}
