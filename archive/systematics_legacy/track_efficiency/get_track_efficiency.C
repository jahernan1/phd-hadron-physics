#include "gxana/common/Paths.h"
/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
#include <stdio.h>

//instantiate methods used in main
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, Bool_t make_leg = true);
void merge_plot(vector<TH1D*> vec_hist, string save_name, string leg_title,string axis_title, Bool_t make_leg = true);
void compare_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true);
void compare_plot_log(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true);
void style_format();
void MakeStackedAngleHist(vector<TH1D*> arr_hist, double eff, string leg_pos="tl", string leg_title="GlueX#lower[-0.15]{-}#kern[0.1]{I}");
void MakeStackedHist(vector<TH1D*> arr_hist, string leg_pos="tl", string leg_title="GlueX#lower[-0.15]{-}#kern[0.1]{I}");

// main function (change main name when copying)
int get_track_efficiency()
{
  // call plot styling function
  style_format();
  
  //initiate variables
  vector<TH1D*> hist;
  vector<TH1D*> hist_particle_angle;
  vector<TH1D*> hist_particle_angle_mc;
  
  vector<TH1D*> hist_particle_angle_corr;
  vector<string> particles = {"kp1_kin", "kp2_kin",
      "pim1_kin", "pim2_kin", "proton_kin" };
    
  
  // Load root tree of choice
  TFile *f = TFile::Open( "particle_kinematics.root");
  //Get Angular Histograms
  double tot_eff = 0; double tot_eff_mc = 0;
  for(const auto& part : particles )
      {
          TH2D* hist_tmp = (TH2D*)f->Get( (part+"_phase1").c_str())->Clone();
          TH2D* hist_tmp_mc = (TH2D*)f->Get( (part+"_phase1_mc").c_str())->Clone();
          TH1D* hist_angle = (TH1D*)hist_tmp->ProjectionX()->Clone( (part+"_angle_phase1_mc").c_str());
          TH1D* hist_angle_mc = (TH1D*)hist_tmp_mc->ProjectionX();
          TH1D* hist_mom = (TH1D*)hist_tmp->ProjectionY()->Clone( (part+"_mom_phase1_mc").c_str());
          TH1D* hist_mom_mc = (TH1D*)hist_tmp_mc->ProjectionY();

          hist_particle_angle.push_back( hist_angle);
          hist_particle_angle_mc.push_back( hist_angle_mc);

          double lowBin = hist_angle->FindBin(20);
          double lowBinMC = hist_angle_mc->FindBin(20);
          double Nlow = hist_angle->Integral(0,lowBin);
          double NlowMC = hist_angle_mc->Integral(0,lowBinMC);
          double Nhigh = hist_angle->Integral(lowBin+1, hist_angle->GetNbinsX());
          double NhighMC = hist_angle_mc->Integral(lowBinMC+1, hist_angle_mc->GetNbinsX());
          double eff = (0.03*Nlow + 0.05*Nhigh) / (Nlow + Nhigh);
          double effMC = (0.03*NlowMC + 0.05*NhighMC) / (NlowMC + NhighMC);
          tot_eff += eff;
          tot_eff_mc += effMC;
          cout << part << ": (Data, MC)\n"
               << "\tNlow (" << Nlow << ", " << NlowMC << ")\n"
               << "\tNhigh (" << Nhigh << ", " << NhighMC << ")\n"
               << "\tTrack Eff (" << eff << ", " << effMC << ")"
               << endl;
          
          MakeStackedAngleHist({hist_angle,hist_angle_mc},effMC, "tr");
          //MakeStackedHist({hist_angle,hist_angle_mc}, "tr");
          // MakeStackedHist({hist_mom,hist_mom_mc}, "tr");          
      }
  
  cout << "Total Track Efficiency: (" << tot_eff << ", "
       << tot_eff_mc << ")"<< endl;
        
  return 0;
}

void MakeStackedAngleHist(vector<TH1D*> arr_hist, double eff, string leg_pos="tl", string leg_title="GlueX#lower[-0.15]{-}#kern[0.1]{I}")
{
  string plotdir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/event_selection/rapidity_cuts/");
  string histName = arr_hist[0]->GetName();  
  string saveName = histName + "_data_mc.pdf";
  char entries[4][100];
  vector<Color_t> arr_colors = {kBlue, kMagenta, kRed, kCyan, kYellow, kOrange};
  vector<Style_t> arr_marker_style = {kFullCircle, kFullSquare, kFullTriangleUp, kFullStar, kFullTriangleDown, kFullDiamond};
  TCanvas *c = new TCanvas((histName).c_str(), (histName+"_").c_str());
  THStack *hs = new THStack("hs","");
  
  double pxmax = c->GetUxmax();
  double pxmin = c->GetUxmax();
  double pymax = c->GetUymax();
  double pymin = c->GetUymax();
  double scale_factor;
  //arr_hist[0]->SetFillColorAlpha(kGray,0.9);
  arr_hist[0]->SetMarkerStyle(20);
  arr_hist[0]->SetMarkerSize(0.9);
  arr_hist[0]->SetMarkerColor(kBlack);
  arr_hist[0]->SetLineColor(kBlack);
  
  for(Int_t i = 1; i < arr_hist.size(); i++){
    //arr_hist[i]->RebinX();
    cout << "Bin Width Data: " << arr_hist[0]->GetBinWidth(1) << endl;
    cout << "Bin Width MC: " << arr_hist[i]->GetBinWidth(1) << endl;
        
    // scale the histos to area
    if(arr_hist[i]->Integral("width") < arr_hist[0]->Integral("width"))
      scale_factor = arr_hist[0]->Integral("width") / arr_hist[i]->Integral("width");
    else
      scale_factor = arr_hist[i]->Integral("width") / arr_hist[0]->Integral("width") ;
    
    if(arr_hist[i]->GetMaximum() < arr_hist[0]->GetMaximum())
      arr_hist[i]->Scale(scale_factor );
    else
      arr_hist[i]->Scale(1/scale_factor );

    arr_hist[i]->SetLineColor(kBlack);
    arr_hist[i]->SetFillColorAlpha(kAzure-9,0.6);
    //arr_hist[i]->SetFillStyle(4010);
    sprintf(entries[i], "%.0f arb. units", arr_hist[i]->GetEntries());

    hs->Add(arr_hist[i],"hist");
  }

  //arr_hist[0]->RebinX();
  hs->Add(arr_hist[0],"e1");

  string title = Form("#bf{#color[2]{#varepsilon = %.2f%%}}; %s; arb. units", eff*100, arr_hist[0]->GetXaxis()->GetTitle());
  hs->SetTitle(title.c_str());
  hs->Draw("nostack");
  hs->GetYaxis()->SetMaxDigits(2);
  //hs->GetXaxis()->SetLimits(xmin,xmax);
    
  // Draw legend
  TLegend* legend;
  if(leg_pos=="tl")
    legend = new TLegend(0.16,0.75,0.4,0.94); //top left corner
  else
    legend = new TLegend(0.66,0.71,0.86,0.88); //top right corner
    
  legend->SetBorderSize(0);
  legend->SetTextSize(0.065);
  legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
  
  legend->AddEntry(arr_hist[0], "Data", "lep");
  legend->AddEntry(arr_hist[1], "Recon MC", "f");
  legend->Draw("same");

  auto l = new TLine(20, 0, 20, arr_hist[1]->GetMaximum()*1.05);
  l->SetLineColor(kBlue);
  l->SetLineStyle(kDashed);
  l->SetLineWidth(4);
  l->Draw("same");
  
  gPad->Update();
  c->SaveAs(saveName.c_str());
  //c->Close();
}

void MakeStackedHist(vector<TH1D*> arr_hist, string leg_pos="tl", string leg_title="GlueX#lower[-0.15]{-}#kern[0.1]{I}")
{
  string plotdir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/event_selection/rapidity_cuts/");
  string histName = arr_hist[0]->GetName();  
  string saveName = histName + "_data_mc.pdf";
  char entries[4][100];
  vector<Color_t> arr_colors = {kBlue, kMagenta, kRed, kCyan, kYellow, kOrange};
  vector<Style_t> arr_marker_style = {kFullCircle, kFullSquare, kFullTriangleUp, kFullStar, kFullTriangleDown, kFullDiamond};
  TCanvas *c = new TCanvas((histName).c_str(), (histName+"_").c_str());
  THStack *hs = new THStack("hs","");
  
  double pxmax = c->GetUxmax();
  double pxmin = c->GetUxmax();
  double pymax = c->GetUymax();
  double pymin = c->GetUymax();
  double scale_factor;
  //arr_hist[0]->SetFillColorAlpha(kGray,0.9);
  arr_hist[0]->SetMarkerStyle(20);
  arr_hist[0]->SetMarkerSize(0.9);
  arr_hist[0]->SetMarkerColor(kBlack);
  arr_hist[0]->SetLineColor(kBlack);
  
  for(Int_t i = 1; i < arr_hist.size(); i++){
    //arr_hist[i]->RebinX();
    cout << "Bin Width Data: " << arr_hist[0]->GetBinWidth(1) << endl;
    cout << "Bin Width MC: " << arr_hist[i]->GetBinWidth(1) << endl;
        
    // scale the histos to area
    if(arr_hist[i]->Integral("width") < arr_hist[0]->Integral("width"))
      scale_factor = arr_hist[0]->Integral("width") / arr_hist[i]->Integral("width");
    else
      scale_factor = arr_hist[i]->Integral("width") / arr_hist[0]->Integral("width") ;
    
    if(arr_hist[i]->GetMaximum() < arr_hist[0]->GetMaximum())
      arr_hist[i]->Scale(scale_factor );
    else
      arr_hist[i]->Scale(1/scale_factor );

    arr_hist[i]->SetLineColor(kBlack);
    arr_hist[i]->SetFillColorAlpha(kAzure-9,0.6);
    //arr_hist[i]->SetFillStyle(4010);
    sprintf(entries[i], "%.0f arb. units", arr_hist[i]->GetEntries());

    hs->Add(arr_hist[i],"hist");
  }

  //arr_hist[0]->RebinX();
  hs->Add(arr_hist[0],"e1");

  string title = Form("; %s; arb. units / %.2f", arr_hist[0]->GetXaxis()->GetTitle(),arr_hist[0]->GetBinWidth(1));
  hs->SetTitle(title.c_str());
  hs->Draw("nostack");
  hs->GetYaxis()->SetMaxDigits(2);
  //hs->GetXaxis()->SetLimits(xmin,xmax);
    
  // Draw legend
  TLegend* legend;
  if(leg_pos=="tl")
    legend = new TLegend(0.16,0.75,0.4,0.94); //top left corner
  else
    legend = new TLegend(0.66,0.71,0.86,0.88); //top right corner
    
  legend->SetBorderSize(0);
  legend->SetTextSize(0.065);
  legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
  
  legend->AddEntry(arr_hist[0], "Data", "lep");
  legend->AddEntry(arr_hist[1], "Recon MC", "f");
  legend->Draw("same");

  gPad->Update();
  c->SaveAs(saveName.c_str());
  //c->Close();
}

// input specific style formatting of user choice
void style_format()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(600);
  gStyle->SetCanvasDefW(700);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.15);
  gStyle->SetPadRightMargin (0.04);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);

  gStyle->SetFrameFillStyle ( 0);
  gStyle->SetFrameFillColor ( 0);
  gStyle->SetFrameLineColor ( 1);
  gStyle->SetFrameLineStyle ( 0);
  gStyle->SetFrameLineWidth ( 1);
  gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);

  gStyle->SetNdivisions(505);

  //gStyle->SetLineWidth(1);
  //gStyle->SetHistLineWidth(1);
  //gStyle->SetFrameLineWidth(2);
  gStyle->SetLegendFillColor(0);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.07,"X");
  gStyle->SetLabelSize(0.07,"Y");

  gStyle->SetLabelOffset(0.008,"X");
  gStyle->SetLabelOffset(0.008,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132,"T");
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");
  
  gStyle->SetTitleSize(0.07,"T");
  gStyle->SetTitleSize(0.08,"X");
  gStyle->SetTitleSize(0.08,"Y");
  
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(0.9,"Y");
  gStyle->SetTitleAlign(33);
  gStyle->SetTitleX(.9);
  gStyle->SetTextSize(0.08);
  gStyle->SetTextFont(132);

  gStyle->SetOptStat(0);

  gROOT->ForceStyle();

  TLatex* latex = new TLatex();
  latex->SetNDC();
  latex->SetTextFont(132);
  latex->SetTextSize(0.08);
  latex->SetTextAlign(32);
  gROOT->ForceStyle();
}
