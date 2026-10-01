#include "gxana/common/AcceptanceCorrect.h"
#include "gxana/common/Paths.h"
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
TH1D* GetAcceptanceHist1D(TH1D* hist_genr, TH1D* hist_recon);
TH1D* GetAcceptanceCorrHist1D(vector<TH1D*> vec_hist, TFile *save_file, Bool_t weighted=false);

//main function
int get_data_hists_RF()
{
  //set up root file with directories (the run periods of $GXANA_OUTPUT/kpkpxim/config/channel.kv)
  TFile *f =  TFile::Open( "data_RF.root", "RECREATE");

  //initiate variables
  string root_file_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
  string root_file_dir_qval = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/");
  // the MC sample gen_amp_V2_3D_ac is not in config/samples.yaml: its stem is the data stem + _gen_amp_V2_3D_ac
  vector<gxana::Period> periods = gxana::MakePeriods(gxana::ChannelInfo::Load("kpkpxim"),
      {root_file_dir_qval+"{stem}_nominal_ximVertexCut_1111111/postQVal_flatTree_{stem}_nominal_ximVertexCut_1111111.root",
       root_file_dir+"flatTree_{stem}_gen_amp_V2_3D_ac_nominal_ximVertexCut.root",
       root_file_dir+"flatTree_thrown_{stem}_gen_amp_V2_3D_ac.root", ""});
  const size_t nPeriods = periods.size();
  vector<vector<TH1D*>> vect_histo(nPeriods), vect_costheta(nPeriods), vect_ystar(nPeriods);

  TH1D* hist_all;  TH1D* hist_all_costheta;TH1D* hist_all_ystar;
  vector<string> vec_delim = {"_qval","_mc","_thrown"};
  
  //perform actions: per period the Q-factor data, MC and thrown trees
  gxana::FillPeriodHists(periods, {{gxana::Input::Data, QvalFill()}, {gxana::Input::MC, McFill()},
                                   {gxana::Input::Thrown, ThrownFill()}}, f, 16);

  f->ReOpen("READ");
  //get histos from root tree
  for(int i = 0; i < vec_delim.size(); i++)
    {
      vector<TH1D*> tdist = gxana::GetPeriodHists<TH1D>(f, periods, "t_dist"+vec_delim[i], "tdist"+vec_delim[i]);
      vector<TH1D*> costheta = gxana::GetPeriodHists<TH1D>(f, periods, "xim_costheta_hf"+vec_delim[i], "costheta"+vec_delim[i]);
      vector<TH1D*> ystar = gxana::GetPeriodHists<TH1D>(f, periods, "ystar_M"+vec_delim[i], "ystarM"+vec_delim[i]);
      for(size_t k = 0; k < nPeriods; k++)
        {
          vect_histo[k].push_back(tdist[k]);
          vect_costheta[k].push_back(costheta[k]);
          vect_ystar[k].push_back(ystar[k]);
        }
    }

  //Perform acceptance correction
  f->ReOpen("UPDATE");
  f->cd(periods[0].Dir().c_str());
  hist_all = (TH1D*)GetAcceptanceCorrHist1D(vect_histo[0], f)->Clone("tdist_acccorr");
  hist_all_costheta = (TH1D*)GetAcceptanceCorrHist1D(vect_costheta[0], f)->Clone("costheta_acccorr");
  hist_all_ystar = (TH1D*)GetAcceptanceCorrHist1D(vect_ystar[0], f)->Clone("ystarM_acccorr");
  //
  for(size_t k = 1; k < nPeriods; k++)
    {
      f->cd(periods[k].Dir().c_str());
      TH1D* hist_tmp = (TH1D*)GetAcceptanceCorrHist1D(vect_histo[k], f);
      hist_all->Add( hist_tmp );
      hist_all_costheta->Add((TH1D*)GetAcceptanceCorrHist1D(vect_costheta[k], f));
      hist_all_ystar->Add((TH1D*)GetAcceptanceCorrHist1D(vect_ystar[k], f));
    }
  
  f->cd();
  hist_all->Write("tdist_all_acceptcorr",TObject::kOverwrite);
  hist_all_costheta->Write("costheta_all_acceptcorr",TObject::kOverwrite);
  hist_all_ystar->Write("ystarM_all_acceptcorr",TObject::kOverwrite);
  // thrown (index 2) summed over the periods
  auto thrown = [&](vector<vector<TH1D*>>& v) { vector<TH1D*> out; for (auto& h : v) out.push_back(h[2]); return out; };
  gxana::MergeHists(thrown(vect_costheta))->Write("costheta_all_thrown",TObject::kOverwrite);
  gxana::MergeHists(thrown(vect_ystar))->Write("ystarM_all_thrown",TObject::kOverwrite);
  gxana::MergeHists(thrown(vect_histo))->Write("tdist_all_thrown",TObject::kOverwrite);

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
  sprintf(newName,"Events/#epsilon / %.2f GeV", binwidth);
  
  vec_hist[0]->GetYaxis()->SetTitle(newName);
  TH1D* hist_data_acccorr = (TH1D*)gxana::AcceptanceCorrect(*vec_hist[0], *hist_accept, vec_hist[0]->GetName(), gxana::AccErrors::Plain);
  if(weighted)
    hist_data_acccorr->Write( "t_dist_acceptcorr_weighted",TObject::kOverwrite);
  else
    hist_data_acccorr->Write( (name+"_acceptcorr").c_str(),TObject::kOverwrite);
  
  return hist_data_acccorr;
}

/* ****************************************************************************************************
***************************************************************************************************** 
**************************************************************************************************** */ 
// save_from_flattrees before the port: one FillSpec per input; the keys are written in this order.
gxana::FillSpec ThrownFill()
{
  gxana::FillSpec s;
  s.tree = "flatTree_thrown_kpkpxim";
  s.steps = {{"ystar_M","ystar_p4.M()"}};
  s.hists = {
    {"t_dist_thrown", "", " ; -t (GeV)^{2}; Events", {"t_dist"}, {100,0,5}, "", ""},
    {"ystar_M_thrown", "", " ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events", {"ystar_M"}, {100,1.7,4.2}, "", ""},
    {"xim_costheta_hf_thrown", "", " ; cos#theta_{H}; Events", {"xim_costheta_hf"}, {100,-1,1}, "", ""},
  };
  return s;
}

gxana::FillSpec McFill()
{
  gxana::FillSpec s;
  s.tree = "flatTree_kpkpxim";
  s.hists = {
    {"t_dist_mc", "", " ; -t (GeV)^{2}; Events", {"t_dist"}, {100,0,5}, "acc_weight", ""},
    {"ystar_M_mc", "", " ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events", {"ystar_M"}, {100,1.7,4.2}, "acc_weight", ""},
    {"xim_costheta_hf_mc", "", " ; cos#theta_{H}; Events", {"xim_costheta_hf"}, {100,-1,1}, "acc_weight", ""},
  };
  return s;
}

gxana::FillSpec QvalFill()
{
  gxana::FillSpec s;
  s.tree = "flatTree_kpkpxim";
  s.steps = {{"qvalue_acc","qvalue_decayxim_M*acc_weight"}};
  s.frames = {{"df1", "decayxim_M<1.334 && decayxim_M>1.31"}};
  s.hists = {
    {"t_dist", "", " ; -t (GeV)^{2}; Events", {"t_dist"}, {100,0,5}, "acc_weight", "df1"},
    {"t_dist_qval", "", " ; -t (GeV)^{2}; Events", {"t_dist"}, {100,0,5}, "qvalue_acc", ""},
    {"ystar_M", "", " ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events", {"ystar_M"}, {100,1.7,4.2}, "acc_weight", "df1"},
    {"ystar_M_qval", "", " ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events", {"ystar_M"}, {100,1.7,4.2}, "qvalue_acc", ""},
    {"xim_costheta_hf_qval", "", " ; cos#theta_{H}; Events", {"xim_costheta_hf"}, {100,-1,1}, "qvalue_acc", ""},
  };
  return s;
}
