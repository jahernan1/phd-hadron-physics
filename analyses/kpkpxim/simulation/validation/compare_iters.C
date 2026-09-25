#include "gxana/common/Paths.h"

//functions
int make_plots(string input_file, string iter_file);
void compare_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true);

//main
int compare_iters()
{
  make_plots("data_ac_ximVertexCut_hist2d.root", "data_ac1_ximVertexCut_hist2d.root");
  
  return 0;
}

//get histos to plot
int make_plots(string input_file, string iter_file)
{
    string dir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/");
    //
    TFile *inf = TFile::Open( ( dir + "MC/" + input_file ).c_str() );
    TFile *inf1 = TFile::Open( ( dir + "MC/" + iter_file ).c_str() );
    TH3D* hist_3d_ac = (TH3D*)inf->Get("ResMassVsCosThetaVsT_Phase1")->Clone();
    TH3D* hist_3d = (TH3D*)inf->Get("ResMassVsCosThetaVsT_Phase1_noac")->Clone();
    TH3D* hist_3d_acceptance = (TH3D*)inf->Get("Fall_2018/acceptance")->Clone();
    //
    TH3D* hist_3d_ac_1 = (TH3D*)inf1->Get("ResMassVsCosThetaVsT_Phase1")->Clone();
    TH3D* hist_3d_1 = (TH3D*)inf1->Get("ResMassVsCosThetaVsT_Phase1_noac")->Clone();
    TH3D* hist_3d_acceptance_1 = (TH3D*)inf1->Get("Fall_2018/acceptance")->Clone();
    //
    TH1D* tdist_ac = (TH1D*)hist_3d_ac->ProjectionZ()->Clone("tdist_ac");
    TH1D* costheta_ac = (TH1D*)hist_3d_ac->ProjectionY()->Clone("costheta_ac");
    TH1D* ystar_ac = (TH1D*)hist_3d_ac->ProjectionX()->Clone("ystar_ac");
    TH1D* cos_theta_acceptance = (TH1D*)hist_3d_acceptance->ProjectionY()->Clone("cos_theta_acceptance");
    TH1D* ystar_acceptance = (TH1D*)hist_3d_acceptance->ProjectionX()->Clone("ystar_acceptance");
    //
    TH1D* tdist_ac_1 = (TH1D*)hist_3d_ac_1->ProjectionZ()->Clone("tdist_ac_1");
    TH1D* costheta_ac_1 = (TH1D*)hist_3d_ac_1->ProjectionY()->Clone("costheta_ac_1");
    TH1D* ystar_ac_1 = (TH1D*)hist_3d_ac_1->ProjectionX()->Clone("ystar_ac_1");
    TH1D* cos_theta_acceptance_1 = (TH1D*)hist_3d_acceptance_1->ProjectionY()->Clone("acceptance_1");
    TH1D* ystar_acceptance_1 = (TH1D*)hist_3d_acceptance_1->ProjectionX()->Clone("ystar_acceptance_1");
    //TH1D* acceptance_1 = (TH1D*)hist_3d_acceptance_1->ProjectionX()->Clone("acceptance_1");
    
    gStyle->SetOptStat(0);
    compare_plot({ ystar_ac, ystar_ac_1},"in_iter_ystar","","",true);
    compare_plot({ costheta_ac, costheta_ac_1},"in_iter_costheta","","",false);
    compare_plot({ cos_theta_acceptance, cos_theta_acceptance_1},"in_iter_costheta_accept","","",false);
    compare_plot({ ystar_acceptance, ystar_acceptance_1},"in_iter_ystar_accept","","",false);

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
  //legend->Draw("same");
  //Fit the merged hist

  //Save plot
  gPad->Update();
  c->SaveAs((save_name+".pdf").c_str());
}
