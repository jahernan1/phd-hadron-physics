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
TH2D* GetAcceptanceCorrHist2D(vector<TH2D*> vec_hist, TFile *save_file);

//main function
int get_data_hists_RF()
{
  //set up root file with directories (the run periods of $GXANA_OUTPUT/kpkpxim/config/channel.kv)
  TFile *f =  TFile::Open( "data_ac_hist2d_kphighrap_2d.root", "RECREATE");

  //initiate variables
  string root_file_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
  string root_file_dir_qval = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/");
  vector<gxana::Period> periods = gxana::MakePeriods(gxana::ChannelInfo::Load("kpkpxim"),
      {root_file_dir_qval+"{stem}_nominal_kphighrap_1111111/postQVal_flatTree_{stem}_nominal_kphighrap_1111111.root",
       root_file_dir+"flatTree_{mc_stem}_nominal_kphighrap.root",
       root_file_dir+"flatTree_thrown_{mc_stem}.root", "gen_amp_V2_ac_YstarRest"});
  const size_t nPeriods = periods.size();
  vector<vector<TH1D*>> vect_histo(nPeriods), vect_costheta(nPeriods), vect_ystar(nPeriods);
  vector<vector<TH2D*>> vect2d_costheta_ystarM(nPeriods);

  TH1D* hist_all;  TH1D* hist_all_costheta; TH1D* hist_all_ystar;
  TH2D* hist_all_costheta_ystar;
  vector<string> vec_delim = {"_qval","_mc","_thrown"};

  //perform actions: per period the Q-factor data, MC and thrown trees
  gxana::FillPeriodHists(periods, {{gxana::Input::Data, QvalFill()}, {gxana::Input::MC, McFill()},
                                   {gxana::Input::Thrown, ThrownFill()}}, f, 16);

  f->ReOpen("READ");
  //get histos from root tree
  for(int i = 0; i < vec_delim.size(); i++)
    {
      vector<TH1D*> tdist = gxana::GetPeriodHists<TH1D>(f, periods, "t_dist"+vec_delim[i], "tdist"+vec_delim[i]);
      vector<TH1D*> costheta = gxana::GetPeriodHists<TH1D>(f, periods, "costheta_gen_amp"+vec_delim[i], "costheta"+vec_delim[i]);
      vector<TH1D*> ystar = gxana::GetPeriodHists<TH1D>(f, periods, "ystar_M"+vec_delim[i], "ystarM"+vec_delim[i]);
      // 2d Distributions
      vector<TH2D*> costheta_ystarM = gxana::GetPeriodHists<TH2D>(f, periods, "costheta_gen_amp_ystarM"+vec_delim[i], "costheta_ystarM"+vec_delim[i]);
      // as before the port: the second period's 2-D clones are named costheta_ystar_M<kind>
      // (written as the object name of Spring_2018/costheta_ystarM_acceptcorr)
      if (nPeriods > 1)
        costheta_ystarM[1]->SetName(("costheta_ystar_M"+vec_delim[i]).c_str());
      for(size_t k = 0; k < nPeriods; k++)
        {
          vect_histo[k].push_back(tdist[k]);
          vect_costheta[k].push_back(costheta[k]);
          vect_ystar[k].push_back(ystar[k]);
          vect2d_costheta_ystarM[k].push_back(costheta_ystarM[k]);
        }
    }

  //Perform acceptance correction
  f->ReOpen("UPDATE");
  f->cd(periods[0].Dir().c_str());
  hist_all = (TH1D*)GetAcceptanceCorrHist1D(vect_histo[0], f)->Clone("tdist_acccorr");
  hist_all_costheta = (TH1D*)GetAcceptanceCorrHist1D(vect_costheta[0], f)->Clone("costheta_acccorr");
  hist_all_ystar = (TH1D*)GetAcceptanceCorrHist1D(vect_ystar[0], f)->Clone("ystarM_acccorr");
  hist_all_costheta_ystar = (TH2D*)GetAcceptanceCorrHist2D(vect2d_costheta_ystarM[0], f)->Clone("costheta_ystarM_acccorr");
  //
  for(size_t k = 1; k < nPeriods; k++)
    {
      f->cd(periods[k].Dir().c_str());
      TH1D* hist_tmp = (TH1D*)GetAcceptanceCorrHist1D(vect_histo[k], f);
      hist_all->Add( hist_tmp );
      hist_all_costheta->Add((TH1D*)GetAcceptanceCorrHist1D(vect_costheta[k], f));
      hist_all_ystar->Add((TH1D*)GetAcceptanceCorrHist1D(vect_ystar[k], f));
      hist_all_costheta_ystar->Add((TH2D*)GetAcceptanceCorrHist2D(vect2d_costheta_ystarM[k], f));
    }
  //
  f->cd();
  hist_all->Write("tdist_all_acceptcorr",TObject::kOverwrite);
  hist_all_costheta->Write("costheta_all_acceptcorr",TObject::kOverwrite);
  hist_all_ystar->Write("ystarM_all_acceptcorr",TObject::kOverwrite);
  hist_all_costheta_ystar->Write("costheta_ystarM_all_acceptcorr",TObject::kOverwrite);

  // Get combined distributions (index i of every period: vec_delim[i])
  auto periods2d = [&](int i) { vector<TH2D*> v; for (auto& h : vect2d_costheta_ystarM) v.push_back(h[i]); return v; };
  auto periods1d = [&](int i) { vector<TH1D*> v; for (auto& h : vect_histo) v.push_back(h[i]); return v; };
  gxana::MergeHists(periods2d(0))->Write("costheta_ystar_all_qval",TObject::kOverwrite);
  // as before the port: the _mc sum (index 1) is written as ..._thrown and the _thrown sum as ..._mc
  gxana::MergeHists(periods2d(1))->Write("costheta_ystar_all_thrown",TObject::kOverwrite);
  gxana::MergeHists(periods2d(2))->Write("costheta_ystar_all_mc",TObject::kOverwrite);

  // TDist
  gxana::MergeHists(periods1d(0))->Write("tdist_all_qval",TObject::kOverwrite);
  gxana::MergeHists(periods1d(1))->Write("tdist_all_mc",TObject::kOverwrite);
  gxana::MergeHists(periods1d(2))->Write("tdist_all_thrown",TObject::kOverwrite);
 
  //Get the 2D distributions from the original files
  string path2d = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/MC/");
  TFile *f2d =  TFile::Open( (path2d+"data_ac_ximVertexCut_hist2d_YstarRest.root").c_str(), "READ");
  TH2D* hist2d_ac = (TH2D*)f2d->Get( "ResMassVsCosTheta_Phase1_ac")->Clone();
  string name2d = hist2d_ac->GetName();
  //
  TH2D* hist2d_qval = (TH2D*)f2d->Get( "ResMassVsCosTheta_qval_Phase1")->Clone();
  TH2D* hist2d_mc = (TH2D*)f2d->Get( "ResMassVsCosTheta_mc_Phase1")->Clone();
  TH2D* hist2d_thrown = (TH2D*)f2d->Get( "ResMassVsCosTheta_thrown_Phase1")->Clone();
  f->cd();
  hist2d_ac->Write(  (name2d+"_qval_ac").c_str(), TObject::kOverwrite);
  hist2d_qval->Write( (name2d+"_qval").c_str()), TObject::kOverwrite;
  hist2d_mc->Write( (name2d+"_mc").c_str()), TObject::kOverwrite;
  hist2d_thrown->Write( (name2d+"_thrown").c_str()), TObject::kOverwrite;
  
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

//`vect_hist` [0](data),[1](recon),[2](generated)
TH2D* GetAcceptanceCorrHist2D(vector<TH2D*> vec_hist, TFile *save_file)
{
  TH2D* hist_accept = (TH2D*)gxana::Acceptance(*vec_hist[2], *vec_hist[1], "acceptance", gxana::AccErrors::PlainNoSumw2);
  printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
  hist_accept->Write("costheta_ystarM_acceptance",TObject::kOverwrite);
  TH2D* hist_data_acccorr = (TH2D*)gxana::AcceptanceCorrect(*vec_hist[0], *hist_accept, vec_hist[0]->GetName(), gxana::AccErrors::Plain);
  hist_data_acccorr->Write("costheta_ystarM_acceptcorr",TObject::kOverwrite);
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
    {"t_dist_thrown", "", " ; -t (GeV)^{2} ); Events", {"t_dist"}, {50,0,5}, "", ""},
    {"ystar_M_thrown", "", " ; M(#Xi^{-}K^{+}_{#it{#lower[-0.3]{S}}}) (GeV); Events", {"ystar_M"}, {50,1.8,4.2}, "", ""},
    {"costheta_gen_amp_thrown", "", " ; cos#theta_{H}; Events", {"xim_costheta_gen_amp"}, {50,-1,1}, "", ""},
    {"costheta_gen_amp_ystarM_thrown", "", " ; cos#theta_{H}; M(#Xi^{-}K^{+}_{#it{#lower[-0.3]{S}}}) (GeV)", {"xim_costheta_gen_amp", "ystar_M"}, {50,-1,1, 50, 1.8, 4.2}, "", ""},
  };
  return s;
}

gxana::FillSpec McFill()
{
  gxana::FillSpec s;
  s.tree = "flatTree_kpkpxim";
  s.hists = {
    {"t_dist_mc", "", " ; -t (GeV)^{2}; Events", {"t_dist_truth"}, {50,0,5}, "hybrid_combo", ""},
    {"ystar_M_mc", "", " ; M(#Xi^{-}K^{+}_{#it{#lower[-0.3]{S}}}) (GeV); Events", {"ystar_M"}, {50,1.8,4.2}, "hybrid_combo", ""},
    {"costheta_gen_amp_mc", "", " ; cos#theta_{H}; Events", {"xim_costheta_gen_amp"}, {50,-1,1}, "hybrid_combo", ""},
    {"costheta_gen_amp_ystarM_mc", "", " ; cos#theta_{H}; M(#Xi^{-}K^{+}_{#it{#lower[-0.3]{S}}}) (GeV)", {"xim_costheta_gen_amp", "ystar_M"}, {50,-1,1, 50, 1.8, 4.2}, "hybrid_combo", ""},
  };
  return s;
}

gxana::FillSpec QvalFill()
{
  gxana::FillSpec s;
  s.tree = "flatTree_kpkpxim";
  s.steps = {{"", "qvalue_decayxim_M > 2e-2"}, {"qvalue_hybrid","qvalue_decayxim_M*hybrid_combo"}};
  s.frames = {{"df1", "decayxim_M<1.334 && decayxim_M>1.31"}};
  s.hists = {
    {"t_dist", "", " ; -t (GeV)^{2}; Events", {"t_dist"}, {50,0,5}, "best_combo", "df1"},
    {"t_dist_qval", "", " ; -t (GeV)^{2}; Events", {"t_dist"}, {50,0,5}, "qvalue_hybrid", ""},
    {"ystar_M", "", " ; M(#Xi^{-}K^{+}_{#it{#lower[-0.3]{S}}}) (GeV); Events", {"ystar_M"}, {50,1.8,4.2}, "hybrid_combo", "df1"},
    {"ystar_M_qval", "", " ; M(#Xi^{-}K^{+}_{#it{#lower[-0.3]{S}}}) (GeV); Events", {"ystar_M"}, {50,1.8,4.2}, "qvalue_hybrid", ""},
    {"costheta_gen_amp_qval", "", " ; cos#theta_{H}; Events", {"xim_costheta_gen_amp"}, {50,-1,1}, "qvalue_hybrid", ""},
    {"costheta_gen_amp_ystarM_qval", "", " ; cos#theta_{H}; M(#Xi^{-}K^{+}_{#it{#lower[-0.3]{S}}}) (GeV)", {"xim_costheta_gen_amp", "ystar_M"}, {50,-1,1, 50, 1.8, 4.2}, "qvalue_hybrid", ""},
  };
  return s;
}
