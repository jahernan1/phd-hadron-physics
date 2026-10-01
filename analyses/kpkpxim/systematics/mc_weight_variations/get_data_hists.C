#include "gxana/common/AcceptanceCorrect.h"
#include "gxana/common/Paths.h"
#include "gxana/common/PeriodHists.h"
#include "gxana/common/Periods.h"
/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
gxana::FillSpec DataFill(string hist_name);
gxana::FillSpec ThrownFill();
void save_to_file(string delim="");
TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon);
TH1D* GetAcceptanceCorrHist1D(vector<TH1D*> vec_hist, TFile *save_file);
void WriteAcceptanceHist(vector<vector<TH1D*>> vec_hist, TFile* f, const vector<gxana::Period>& periods);

//main function
int get_data_hists()
{
  save_to_file("_chisqndf+1");
  save_to_file("_kp_momsep_+05");
  save_to_file("_lambda_pathlensig+05");
  save_to_file("_total_mm2+005");
  save_to_file("_xim_pathlensig-05");
  
  return 0;
}

void save_to_file(string delim="")
{
  //Print statement 
  cout << "Processing " << ("data"+delim+".root...").c_str() << endl;
  //set up root file with directories (the run periods of $GXANA_OUTPUT/kpkpxim/config/channel.kv)
  TFile *f =  TFile::Open( ("data"+delim+".root").c_str(), "RECREATE");

  //initiate variables
  string root_file_dir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/root_trees/");
  string root_file_dir_thrown = gxana::EnvPath("GXANA_DATA", "Trees/flatTree/rawTrees/");
  // Ystar2400_1600_genr8 is configured for 2018-08 only: its stem is the data stem + _Ystar2400_1600_genr8
  vector<gxana::Period> periods = gxana::MakePeriods(gxana::ChannelInfo::Load("kpkpxim"),
      {root_file_dir+"flatTree_{stem}_vary"+delim+".root",
       root_file_dir+"flatTree_{stem}_Ystar2400_1600_genr8_vary"+delim+".root",
       root_file_dir_thrown+"flatTree_thrown_{stem}_Ystar2400_1600_genr8.root", ""});
    
  //perform actions: per period the data, MC and thrown trees
  gxana::FillPeriodHists(periods, {{gxana::Input::Data, DataFill("")}, {gxana::Input::MC, DataFill("_mc")},
                                   {gxana::Input::Thrown, ThrownFill()}}, f, 16);

  //change tfile access to read
  f->ReOpen("READ");
  // Perform acceptance correction
  vector<vector<TH1D*>> data_hist(periods.size());
  vector<string> vec_delim = {"","_mc","_thrown"};
  string cut="_cut";
  for(int i = 0; i < 3; i++)
    {
      if(i==2)
        cut = ""; 
      cout << vec_delim[i] << "\t" << cut <<endl;
      vector<TH1D*> hists = gxana::GetPeriodHists<TH1D>(f, periods, "xim_costheta_hf"+cut+vec_delim[i]);
      for(size_t k = 0; k < periods.size(); k++)
        data_hist[k].push_back(hists[k]);
    }
  
  f->ReOpen("UPDATE");
  WriteAcceptanceHist(data_hist, f, periods);
  f->Close();
}

void WriteAcceptanceHist(vector<vector<TH1D*>> vec_hist, TFile* f, const vector<gxana::Period>& periods)
{
  f->cd(periods[0].Dir().c_str());
  TH1D* hist_all = (TH1D*)GetAcceptanceCorrHist1D(vec_hist[0], f)->Clone();
  for(size_t k = 1; k < periods.size(); k++)
    {
      f->cd(periods[k].Dir().c_str());
      TH1D* hist_tmp = (TH1D*)GetAcceptanceCorrHist1D(vec_hist[k], f)->Clone(); 
      hist_all->Add( hist_tmp );
    }
   
  f->cd();
  hist_all->Write("xim_costheta_hf_all_acceptcorr",TObject::kOverwrite);  
}

TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon)
{
  TH1D* hist_accept = (TH1D*)gxana::Acceptance(*hist_genr, *hist_recon, "acceptance", gxana::AccErrors::Binomial);
  hist_accept->GetYaxis()->SetTitle("Acceptance, #epsilon");
  
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
  TH1D* hist_data_acccorr = (TH1D*)gxana::AcceptanceCorrect(*vec_hist[0], *hist_accept, vec_hist[0]->GetName(), gxana::AccErrors::Plain);
  hist_data_acccorr->Print();
  hist_data_acccorr->Write("xim_costheta_hf_acceptcorr",TObject::kOverwrite);
  
  return hist_data_acccorr;
}

// save_from_flattrees before the port: data (hist_name "") and MC ("_mc") share one fill.
gxana::FillSpec DataFill(string hist_name)
{
  gxana::FillSpec s;
  s.tree = "flatTree_kpkpxim";
  s.frames = {{"df1", "decayxim_M<1.334 && decayxim_M>1.308"}};
  s.hists = {
    {"xim_costheta_hf"+hist_name, "", " ; cos#Theta^{Y#Xi}_{#bf{H}} ; Events", {"xim_costheta_hf"}, {200,-1,1}, "", ""},
    {"xim_costheta_hf_cut"+hist_name, "", " ;  cos#Theta^{Y#Xi}_{#bf{H}}; Events", {"xim_costheta_hf"}, {200,-1,1}, "", "df1"},
  };
  return s;
}

gxana::FillSpec ThrownFill()
{
  gxana::FillSpec s;
  s.tree = "flatTree_thrown_kpkpxim";
  s.steps = {{"", "main_pid==1"}};
  s.hists = {
    {"xim_costheta_hf_thrown", "", " ;  cos#Theta^{Y#Xi}_{#bf{H}}; Generated Events", {"xim_costheta_hf"}, {200,-1,1}, "", ""},
  };
  return s;
}
