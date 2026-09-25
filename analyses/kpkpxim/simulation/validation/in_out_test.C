#include "gxana/common/Paths.h"

//functions
int make_plots(string tree_file_name, string input_file, int n_threads=8);
void compare_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true);

//main
int in_out_test()
{
  make_plots("kpkpxim__B4_M23_2018-08_ana02","data_ac_ximVertexCut_hist3d.root");
  
  return 0;
}

//get histos to plot
int make_plots(string tree_file_name, string input_file, int n_threads=8)
{
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
    //Import select braches to speed things up
    string dir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/");
    
    // make data frame and braches for histograms from 4 vectors
    // format : tree name, file name, branches to open
    auto df = ROOT::RDataFrame("flatTree_kpkpxim", (dir + "flatTrees/flatTree_" + tree_file_name + "_nominalRF_ximVertexCut.root").c_str(), {"t_dist", "xim_costheta_hf", "ystar_M", "acc_weight"})
      .Filter("decayxim_M<1.334 && decayxim_M>1.31");
    auto dfmc = ROOT::RDataFrame("flatTree_kpkpxim", (dir + "flatTrees/flatTree_" + tree_file_name + "_gen_amp_V2_3D_mask011_nominalRF_ximVertexCut.root").c_str(), {"t_dist_truth", "xim_costheta_hf", "ystar_M", "acc_weight"})
      .Define("abs_tdist_truth","abs(t_dist_truth)");
    auto dft = ROOT::RDataFrame("flatTree_thrown_kpkpxim", (dir + "flatTrees/flatTree_thrown_" + tree_file_name + "_gen_amp_V2_3D_mask011.root").c_str(), {"t_dist", "xim_costheta_hf", "ystar_p4"})
      .Define("ystar_M","ystar_p4.M()");
    //make histograms and add to tfile
    auto tdist = df.Histo1D({""," ; -t (GeV)^{2}; Events",100u,0,5}, "t_dist","acc_weight");
    auto ystar_M = df.Histo1D({""," ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events",100u,1.7,4.2}, "ystar_M","acc_weight");
    auto costheta = df.Histo1D({""," ; cos#theta_{H}; Events",100u,-1,1}, "xim_costheta_hf","acc_weight");
    //
    auto mc_tdist = dfmc.Histo1D({""," ; -t (GeV)^{2}; Events",100u,0,5}, "abs_tdist_truth","acc_weight");
    auto mc_ystar_M = dfmc.Histo1D({""," ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events",100u,1.7,4.2}, "ystar_M","acc_weight");
    auto mc_costheta = dfmc.Histo1D({""," ; cos#theta_{H}; Events",100u,-1,1}, "xim_costheta_hf","acc_weight");
    //
    auto true_tdist = dft.Histo1D({""," ; -t (GeV)^{2}; Events",100u,0,5}, "t_dist");
    auto true_ystar_M = dft.Histo1D({""," ; M(#Xi^{-}K^{+}_{slow}) (GeV); Events",100u,1.7,4.2}, "ystar_M");
    auto true_costheta = dft.Histo1D({""," ; cos#theta_{H}; Events",100u,-1,1}, "xim_costheta_hf");
    //
    // hist_tdist->Write(("t_dist"+hist_name).c_str(),TObject::kOverwrite);
    // hist_ystar_M->Write(("ystar_M"+hist_name).c_str(),TObject::kOverwrite);
    // hist_costheta->Write(("xim_costheta_hf"+hist_name).c_str(),TObject::kOverwrite);
    //
    TFile *inf = TFile::Open( ( dir + "MC/" + input_file ).c_str() );
    TH3D* hist_3d_ac = (TH3D*)inf->Get("ResMassVsCosThetaVsT_Phase1")->Clone();
    TH3D* hist_3d = (TH3D*)inf->Get("ResMassVsCosThetaVsT_Phase1_noac")->Clone();
    //
    TH1D* tdist_ac = (TH1D*)hist_3d_ac->ProjectionZ()->Clone("tdist_ac");
    TH1D* costheta_ac = (TH1D*)hist_3d_ac->ProjectionY()->Clone("costheta_ac");
    TH1D* ystar_ac = (TH1D*)hist_3d_ac->ProjectionX()->Clone("ystar_ac");
    //tmp->Draw();
    gStyle->SetOptStat(0);
    compare_plot({ true_ystar_M.GetPtr(), ystar_ac},"in_thrown_ystar","","",true);
    compare_plot({ true_costheta.GetPtr(), costheta_ac},"in_thrown_costheta","","",false);

    compare_plot({ mc_ystar_M.GetPtr(), ystar_M.GetPtr()},"data_mc_ystar","","",true);
    compare_plot({ mc_costheta.GetPtr(), costheta.GetPtr()},"data_mc_costheta","","",false);
    compare_plot({ mc_tdist.GetPtr(), tdist.GetPtr()},"data_mc_tdist","","");
    
    return 0;
}

void compare_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true)
{
  // Plot histogram(s) algorithm 
  TH1D* merged_hist = (TH1D*)vec_hist[0]->Clone();
  TH1D* merged_hist_mc = (TH1D*)vec_hist[1]->Clone();
  
  //style_format();
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16); 
  TCanvas *c = new TCanvas(leg_title.c_str(),leg_title.c_str(),800,700);
  
  c->SetGrid();
  
  merged_hist->SetFillColorAlpha(kGray,0.9);
  merged_hist->SetLineColor(kBlack);
  merged_hist->GetYaxis()->SetMaxDigits(3);
  
  merged_hist_mc->SetFillColorAlpha(kAzure-9,0.6);
  merged_hist_mc->SetLineColor(kBlack);
  merged_hist_mc->Scale(merged_hist->Integral("width")/merged_hist_mc->Integral("width"));
  //merged_hist_mc->SetMaximum(merged_hist->GetMaximum()*1.2);

  merged_hist->GetYaxis()->SetRangeUser(0,merged_hist->GetMaximum()*1.2);
  merged_hist->SetTitle(axis_title.c_str());
  merged_hist->SetTitleOffset(1.,"Y");
  if(merged_hist->GetMaximum() < merged_hist_mc->GetMaximum())
    merged_hist->SetMaximum(merged_hist_mc->GetMaximum()*1.2);
  merged_hist->Draw("hist");  merged_hist_mc->Draw( "hist same");
  
  // Draw legend
  TLegend *legend;
  if(make_leg)
    legend = new TLegend(0.68,0.73,0.92,0.91); //top right corner
  else
    legend = new TLegend(0.18,0.73,0.42,0.91); //top left corner

  legend->SetBorderSize(0);
  legend->SetTextSize(0.065);
  legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
  legend->AddEntry(merged_hist, "Thrown", "f");
  //legend->AddEntry(vec_hist[0], "Data (Q-Value)", "f");
  legend->AddEntry(merged_hist_mc, "Input", "f");
  legend->Draw("same");
  //Fit the merged hist

  //Save plot
  gPad->Update();
  c->SaveAs((save_name+".pdf").c_str());
}
