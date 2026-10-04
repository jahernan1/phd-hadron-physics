#include "gxana/common/Paths.h"
#include "gxana/common/AcceptanceCorrect.h"
#include "gxana/common/PeriodHists.h"
#include "gxana/common/Periods.h"
/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
gxana::FillSpec QvalFill();
gxana::FillSpec McFill();
gxana::FillSpec ThrownFill();
void save_to_file(string delim="");

TH2D* GetAcceptanceCorrHist2D(vector<TH2D*> vec_hist, TFile *save_file);
void WriteAcceptanceHist2D(vector<vector<TH2D*>> vec_hist, TFile* f, const vector<gxana::Period>& periods);

//main function
int PrepSampling()
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
  string file = "data_ac"+delim+"_hist2d_YstarRest.root";
  cout << "Processing " << (file).c_str() << endl;
  //set up root file with directories (the run periods of $GXANA_OUTPUT/kpkpxim/config/channel.kv)
  TFile *f =  TFile::Open( (file).c_str(), "RECREATE");

  //initiate variables
  string root_file_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
  string root_file_dir_thrown = gxana::EnvPath("GXANA_DATA", "flatTrees/");
  string root_file_dir_qval = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/");
  vector<gxana::Period> periods = gxana::MakePeriods(gxana::ChannelInfo::Load("kpkpxim"),
      {root_file_dir_qval+"{stem}_nominal"+delim+"_1111111/postQVal_flatTree_{stem}_nominal"+delim+"_1111111.root",
       root_file_dir+"flatTree_{mc_stem}_nominal"+delim+".root",
       root_file_dir_thrown+"flatTree_thrown_{mc_stem}.root", "gen_amp_V2_ac_YstarRest"});
    
  //perform actions: per period the Q-factor data, MC and thrown trees
  gxana::FillPeriodHists(periods, {{gxana::Input::Data, QvalFill()}, {gxana::Input::MC, McFill()},
                                   {gxana::Input::Thrown, ThrownFill()}}, f, 16);

  //change tfile access to read
  f->ReOpen("READ");
  // Perform acceptance correction
  vector<vector<TH2D*>> data_hist(periods.size());
  vector<string> vec_delim = {"_qval","_mc","_thrown"};
  for(int i = 0; i < 3; i++)
    {
      vector<TH2D*> hists = gxana::GetPeriodHists<TH2D>(f, periods, "ResMassVsCosTheta"+vec_delim[i]);
      for(size_t k = 0; k < periods.size(); k++)
        data_hist[k].push_back(hists[k]);
    }
  
  f->ReOpen("UPDATE");
  WriteAcceptanceHist2D(data_hist, f, periods);//Write acceptance-corrected distribution
  //add MVsEgamma
  gxana::MergeHists(gxana::GetPeriodHists<TH2D>(f, periods, "ResMassVsEgamma_qval"))->Write(("ResMassVsEgamma_qval_Phase1"),TObject::kOverwrite);
  gxana::MergeHists(gxana::GetPeriodHists<TH2D>(f, periods, "ResMassVsCosTheta_qval"))->Write(("ResMassVsCosTheta_qval_Phase1"),TObject::kOverwrite);
  gxana::MergeHists(gxana::GetPeriodHists<TH2D>(f, periods, "ResMassVsCosTheta_mc"))->Write(("ResMassVsCosTheta_mc_Phase1"),TObject::kOverwrite);
  gxana::MergeHists(gxana::GetPeriodHists<TH2D>(f, periods, "ResMassVsCosTheta_thrown"))->Write(("ResMassVsCosTheta_thrown_Phase1"),TObject::kOverwrite);

  f->Close();
}

// save_from_flattrees before the port: one FillSpec per input; the keys are written in this order.
gxana::FillSpec QvalFill()
{
  gxana::FillSpec s;
  s.tree = "flatTree_kpkpxim";
  s.steps = {{"", "qvalue_decayxim_M > 1e-2"}, {"qvalue_acc","qvalue_decayxim_M*best_combo"}};
  s.hists = {
    {"ResMassVsCosTheta_qval", "CosThetaVsMass", "; ystar_M; costheta_hf", {"ystar_M","xim_costheta_gen_amp"}, {50,1.8,4.2,50,-1,1}, "qvalue_acc", ""},
    {"xim_costheta_gen_amp", "", " ; cos#Theta^{Y#Xi}_{#bf{H}} ; Events", {"xim_costheta_gen_amp"}, {200,-1,1}, "", ""},
    {"xim_costheta_gen_amp_qval", "", " ;  cos#Theta^{Y#Xi}_{#bf{H}}; Events", {"xim_costheta_gen_amp"}, {200,-1,1}, "qvalue_acc", ""},
    {"ResMassVsEgamma_qval", "MVsE", "; beam_E; ystar_M", {"beam_E","ystar_M"}, {120,6.,12.,160,1.7,4.5}, "qvalue_acc", ""},
    {"ResMVstdist_qval", "MVst", "; t_dist; ystar_M", {"t_dist","ystar_M"}, {100,0,10 ,160,1.7,4.5}, "qvalue_acc", ""},
    {"ResCosThetaVsEgamma_qval", "CosThetaVsE", "; beam_E; costheta_hf", {"beam_E","xim_costheta_gen_amp"}, {120,6.,12.,100,-1,1}, "qvalue_acc", ""},
    {"ResCosThetaVst_qval", "CosThetaVst", "; beam_E; costheta_hf", {"t_dist","xim_costheta_gen_amp"}, {100,1,10,100,-1,1}, "qvalue_acc", ""},
  };
  return s;
}

gxana::FillSpec McFill()
{
  gxana::FillSpec s;
  s.tree = "flatTree_kpkpxim";
  s.hists = {
    {"ResMassVsCosTheta_mc", "CosThetaVsMass", "; ystar_M; costheta_hf", {"ystar_M","xim_costheta_gen_amp"}, {50,1.8,4.2,50,-1,1}, "best_combo", ""},
    {"xim_costheta_gen_amp_mc", "", " ;  cos#Theta^{Y#Xi}_{#bf{H}}; Reconstructed Events", {"xim_costheta_gen_amp"}, {200,-1,1}, "", ""},
    {"ResMassVsEgamma_mc", "MVsE", "; beam_E; ystar_M", {"beam_E","ystar_M"}, {120,6.,12.,160,1.7,4.5}, "best_combo", ""},
    {"ResMVstdist_mc", "MVst", "; t_dist; ystar_M", {"t_dist","ystar_M"}, {100,0,10 ,160,1.7,4.5}, "best_combo", ""},
    {"ResCosThetaVsEgamma_mc", "CosThetaVsE", "; beam_E; costheta_hf", {"beam_E","xim_costheta_gen_amp"}, {120,6.,12.,100,-1,1}, "best_combo", ""},
    {"ResCosThetaVst_mc", "CosThetaVst", "; t_dist; costheta_hf", {"t_dist","xim_costheta_gen_amp"}, {100,1,10,100,-1,1}, "best_combo", ""},
  };
  return s;
}

gxana::FillSpec ThrownFill()
{
  gxana::FillSpec s;
  s.tree = "flatTree_thrown_kpkpxim";
  s.steps = {{"ystar_M","ystar_p4.M()"}};
  s.hists = {
    {"ResMassVsCosTheta_thrown", "CosThetaVsMass", "; ystar_M; costheta_hf", {"ystar_M","xim_costheta_gen_amp"}, {50,1.8,4.2,50,-1,1}, "", ""},
    {"xim_costheta_gen_amp_thrown", "", " ;  cos#Theta^{Y#Xi}_{#bf{H}}; Generated Events", {"xim_costheta_gen_amp"}, {200,-1,1}, "", ""},
    {"ResMassVsEgamma_thrown", "MVsE", "; beam_E; ystar_M", {"beam_E","ystar_M"}, {120,6.,12.,160,1.7,4.5}, "", ""},
    {"ResMVstdist_thrown", "MVst", "; t_dist; ystar_M", {"t_dist","ystar_M"}, {100,0,10 ,160,1.7,4.5}, "", ""},
    {"ResCosThetaVsEgamma_thrown", "CosThetaVsE", "; beam_E; costheta_hf", {"beam_E","xim_costheta_gen_amp"}, {120,6.,12.,100,-1,1}, "", ""},
    {"ResCosThetaVst_thrown", "CosThetaVst", "; t_dist; costheta_hf", {"t_dist","xim_costheta_gen_amp"}, {100,1,10,100,-1,1}, "", ""},
  };
  return s;
}

void WriteAcceptanceHist2D(vector<vector<TH2D*>> vec_hist, TFile* f, const vector<gxana::Period>& periods)
{
  vector<TH2D*> corr;
  for(size_t i=0; i<periods.size(); ++i){
    f->cd(periods[i].Dir().c_str());
    corr.push_back(GetAcceptanceCorrHist2D(vec_hist[i], f));
  }
  vector<const TH1*> corr_c(corr.begin(), corr.end());
  TH1* hist_all = gxana::MergeCorrected(corr_c, {}, nullptr, false);
  f->cd();
  hist_all->Write("ResMassVsCosTheta_Phase1_ac",TObject::kOverwrite);
}

//`vect_hist` [0](data),[1](recon),[2](generated)
TH2D* GetAcceptanceCorrHist2D(vector<TH2D*> vec_hist, TFile *save_file)
{
  TH1* acc = nullptr;
  TH2D* hist_data_acccorr = (TH2D*)gxana::AcceptanceCorrect(*vec_hist[0], *vec_hist[1], *vec_hist[2],
                                vec_hist[0]->GetName(), gxana::AccErrors::PlainNoSumw2, &acc);
  acc->SetName("acceptance");
  acc->Write("costhetahf_ystar_acceptance",TObject::kOverwrite);
  int lost = gxana::LostBins(*vec_hist[0], *acc);
  if(lost>0)
    printf("WARNING %s: %d populated data bins have zero acceptance (gen_amp cannot sample them)\n", gDirectory->GetName(), lost);
  hist_data_acccorr->Write("costhetahf_ystar_acceptcorr",TObject::kOverwrite);
  delete acc;
  return hist_data_acccorr;
}
