#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string delim, TFile *save_file, Int_t n_threads=16);
void make_root_tree(string delim="_vertexCuts")  ;
void make_histos();
void style_format();

//main function
int get_data_hists()
{
  //make the root trees with different cuts
  // make_root_tree();
  make_root_tree("_momCut");
  //make_root_tree("_allKaonSep");

  style_format();
  make_histos();
  
  return 0;
}

void make_root_tree(string delim="_vertexCuts")  
{
  //set up root file with directories
  TFile *f =  TFile::Open( ("data"+delim+".root").c_str(), "RECREATE");
  f->cd();
  gDirectory->mkdir("Spring_2017");
  f->cd();
  gDirectory->mkdir("Spring_2018");  
  f->cd();
  gDirectory->mkdir("Fall_2018");

  //initiate variables
  string root_file_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
  string root_file_dir_qval = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/");

  //perform actions
  //spring 2017
  f->cd();
  f->cd("Spring_2017");
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_nominal"+delim+".root", "", f);
  save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ana56_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal"+delim+"_1111111.root", "", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_2D_ac_nominal"+delim+".root", "_mc", f);
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111.root", "", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_2D_ac_nominal"+delim+".root", "_mc", f);
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_1111111.root", "", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_2D_ac_nominal"+delim+".root", "_mc", f);
  f->Close();
}

void save_from_flattrees(string root_file_path, string delim, TFile *save_file, Int_t n_threads=20)
{
  //Declare Variables
  string weight;
  //Multithreating
  if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
  //Import select braches to speed things up
  std::vector<std::string> branches = {"beam_rfbunches", "uniq_weight", "uniqall_weight","acc_weight", "best_combo_rf", "best_combo" , "decayxim_M", "chisqndf"};
  
  // make data frame and braches for histograms from 4 vectors
  auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), branches) 
    .Define("uniq_acc_weight","acc_weight")
    .Define("uniqall_acc_weight","uniqall_weight*acc_weight")
    .Define("rf_weight","best_combo_rf*acc_weight");
  
  // format : tree name, file name, branches to open
  auto hist_bunches = df.Histo1D( {""," ; #Delta t_{rf} (ns); Photon Combos" ,200u,-20,20},"beam_rfbunches");
  auto hist_bunches_as = df.Histo1D( {""," ; #Delta t_{rf} (ns); Photon Combos" ,200u,-20,20},"beam_rfbunches","uniq_acc_weight");
  auto hist_bunches_rf = df.Histo1D( {""," ; #Delta t_{rf} (ns); Photon Combos" ,200u,-20,20},"beam_rfbunches","rf_weight");
  auto hist_bunches_bc = df.Histo1D( {""," ; #Delta t_{rf} (ns); Photon Combos" ,200u,-20,20},"beam_rfbunches","best_combo");
  
  auto xim_mass_as = df.Histo1D( {""," ; M(#pi^{-}p) (GeV) ; Events" ,100u,1.25,1.45},"decayxim_M", "uniq_acc_weight");
  auto xim_mass_rf = df.Histo1D( {""," ; M(#pi^{-}p) (GeV); Events" ,100u,1.25,1.45},"decayxim_M", "rf_weight");
  auto xim_mass_bc = df.Histo1D( {""," ; M(#pi^{-}p) (GeV); Events" ,100u,1.25,1.45},"decayxim_M", "best_combo");

  auto chisqndf_as = df.Histo1D( {""," ; #chi^{2}_{#nu}; Events" ,100u,0,10},"chisqndf", "uniq_acc_weight");
  auto chisqndf_rf = df.Histo1D( {""," ; #chi^{2}_{#nu}; Events" ,100u,0,10},"chisqndf", "rf_weight");
  auto chisqndf_bc = df.Histo1D( {""," ; #chi^{2}_{#nu}; Events" ,100u,0,10},"chisqndf", "best_combo");
  
  hist_bunches->Write(("rfbunch"+delim).c_str(),TObject::kOverwrite);
  hist_bunches_as->Write(("rfbunch_as"+delim).c_str(),TObject::kOverwrite);
  hist_bunches_rf->Write(("rfbunch_rf"+delim).c_str(),TObject::kOverwrite);
  hist_bunches_bc->Write(("rfbunch_bc"+delim).c_str(),TObject::kOverwrite);
  xim_mass_as->Write(("decayxim_M_as"+delim).c_str(),TObject::kOverwrite);
  xim_mass_rf->Write(("decayxim_M_rf"+delim).c_str(),TObject::kOverwrite);
  xim_mass_bc->Write(("decayxim_M_bc"+delim).c_str(),TObject::kOverwrite);
  chisqndf_as->Write(("chisqndf_as"+delim).c_str(),TObject::kOverwrite);
  chisqndf_rf->Write(("chisqndf_rf"+delim).c_str(),TObject::kOverwrite);
  chisqndf_bc->Write(("chisqndf_bc"+delim).c_str(),TObject::kOverwrite);
}

void make_histos()
{
  //Make vector of plots
  vector<TH1*> vec_hist, vec_hist_mc;
  vector<string> dataSet{"Spring 2017","Spring 2018","Fall 2018" };
  TCanvas *c = new TCanvas("c","c",900,300);
  TCanvas *c_mc = new TCanvas("c_mc","c_mc",900,300);
  c->Divide(3,1); c_mc->Divide(3,1);
  //Get data tree
  TFile *f =  TFile::Open( "data_allKaonSep.root", "READ");
  vec_hist.push_back((TH1D*)f->Get( "Spring_2017/rfbunch" )->Clone() );
  vec_hist.push_back((TH1D*)f->Get( "Spring_2018/rfbunch" )->Clone() );
  vec_hist.push_back((TH1D*)f->Get( "Fall_2018/rfbunch" )->Clone() );
  vec_hist_mc.push_back((TH1D*)f->Get( "Spring_2017/rfbunch_mc" )->Clone() );
  vec_hist_mc.push_back((TH1D*)f->Get( "Spring_2018/rfbunch_mc" )->Clone() );
  vec_hist_mc.push_back((TH1D*)f->Get( "Fall_2018/rfbunch_mc" )->Clone() );
  
  //Setup
  gStyle->SetOptStat(0);
  
  //Plot histos in Canvas
  for(int i=0; i<vec_hist.size(); i++)
    {
      if(i==0)
        {
          vec_hist[i]->GetXaxis()->SetRangeUser(-6,6);
          vec_hist_mc[i]->GetXaxis()->SetRangeUser(-6,6);
        }
      
      c->cd(i+1);
      vec_hist[i]->GetYaxis()->SetMaxDigits(3);
      vec_hist[i]->SetTitle(dataSet[i].c_str());
      vec_hist[i]->SetLineColor(kBlack);
      vec_hist[i]->Draw();
      TH1D* in_hist = (TH1D*)vec_hist[i]->Clone("intime");
      in_hist->SetFillColorAlpha(kSpring-1,0.6);
      in_hist->GetXaxis()->SetRangeUser(-2.004,2.004);
      in_hist->Draw("same");
      TH1D* outL_hist = (TH1D*)vec_hist[i]->Clone("outtime");
      outL_hist->SetFillColorAlpha(kRed,0.6);
      outL_hist->GetXaxis()->SetRangeUser(vec_hist[i]->GetBinLowEdge(1),-2.004);
      outL_hist->Draw("same");
      TH1D* outR_hist = (TH1D*)vec_hist[i]->Clone("outtime");
      outR_hist->SetFillColorAlpha(kRed,0.6);
      outR_hist->GetXaxis()->SetRangeUser(2.004,vec_hist[i]->GetBinLowEdge(vec_hist[i]->GetNbinsX()));
      outR_hist->Draw("same");

      //Plot mc rfbunches
      c_mc->cd(i+1);
      vec_hist_mc[i]->GetYaxis()->SetMaxDigits(3);
      vec_hist_mc[i]->SetTitle(dataSet[i].c_str());
      vec_hist_mc[i]->Draw();
      TH1D* in_hist_mc = (TH1D*)vec_hist_mc[i]->Clone("intime");
      in_hist_mc->SetFillColorAlpha(kSpring-1,0.6);
      in_hist_mc->GetXaxis()->SetRangeUser(-2.004,2.004);
      in_hist_mc->Draw("same");
      TH1D* outL_hist_mc = (TH1D*)vec_hist_mc[i]->Clone("outtime");
      outL_hist_mc->SetFillColorAlpha(kRed,0.6);
      outL_hist_mc->GetXaxis()->SetRangeUser(vec_hist_mc[i]->GetBinLowEdge(1),-2.004);
      outL_hist_mc->Draw("same");
      TH1D* outR_hist_mc = (TH1D*)vec_hist_mc[i]->Clone("outtime");
      outR_hist_mc->SetFillColorAlpha(kRed,0.6);
      outR_hist_mc->GetXaxis()->SetRangeUser(2.004,vec_hist_mc[i]->GetBinLowEdge(vec_hist_mc[i]->GetNbinsX()));
      outR_hist_mc->Draw("same");
    }

  c->SaveAs("rfbunches_phase1.pdf");
  c_mc->SaveAs("rfbunches_mc_phase1.pdf");
}

void style_format()
{
    gxana::StyleParams p = gxana::DistributionStyle();
    p.padBottomMargin = 0.15;
    p.padTopMargin = 0.07;
    p.padLeftMargin = 0.15;
    p.padRightMargin = 0.02;
    p.ndivisionsX = 505;
    p.titleAlign = 33;
    p.titleX = 0.95;
    p.titleOffsetY = 1.05;
    gxana::ApplyStyle(p);
}
