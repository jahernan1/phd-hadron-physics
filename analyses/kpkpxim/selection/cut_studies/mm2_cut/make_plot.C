#include "gxana/common/Paths.h"
/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
//instantiate methods used in main
void get_plot(string delim="_allKaonSep");
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string delim, double chisqcut=8);
void style_format();

// main function (change main name when copying)
int make_plot()
{
  //get_plot("_vertexCuts");
  get_plot("_kphighrap");
  return 0;
}

void get_plot(string delim="_allCuts")
{
  //initiate variables
  string thisDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/event_selection/mm2_cut/");
  vector<TH1D*> vect_histo_2017;
  vector<TH1D*> vect_histo_201801;
  vector<TH1D*> vect_histo_201808;

  // Load root tree of choice
  TFile *f = TFile::Open( (thisDir + "data"+delim+"_RF.root").c_str() );
  vect_histo_2017.push_back( (TH1D*)f->Get("Spring_2017/hist_total_mm2")->Clone() );
  vect_histo_2017.push_back( (TH1D*)f->Get("Spring_2017/hist_total_mm2_mc" )->Clone() );
  
  vect_histo_201801.push_back( (TH1D*)f->Get("Spring_2018/hist_total_mm2")->Clone() );
  vect_histo_201801.push_back( (TH1D*)f->Get("Spring_2018/hist_total_mm2_mc" )->Clone() );
  
  vect_histo_201808.push_back( (TH1D*)f->Get("Fall_2018/hist_total_mm2")->Clone() );
  vect_histo_201808.push_back( (TH1D*)f->Get("Fall_2018/hist_total_mm2_mc" )->Clone() );
  
  // Call function to plot 
  make_plot(vect_histo_2017, "mm2_data_mc_2017", "Spring 2017", delim);
  make_plot(vect_histo_201801, "mm2_data_mc_201801", "Spring 2018",delim);
  make_plot(vect_histo_201808, "mm2_data_mc_201808", "Fall 2018",delim);
}

// make plot to be called by main function
/* Arguments Description:
hist: vector of histograms to plot
legend_title: title of legend (name of data set)
*/
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string delim, double chisqcut=8)
{
  // call plot styling function
  style_format();
  string saveloc = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/event_selection/plots/");
  // Plot histogram(s) algorithm 
  TCanvas *c = new TCanvas("leg_title","leg_title");
  THStack *hs = new THStack("leg_title","");
  
  double pxmax = c->GetUxmax();
  double pxmin = c->GetUxmax();
  double pymax = c->GetUymax();
  double pymin = c->GetUymax();

  //vec_hist[0]->RebinX();
  vec_hist[0]->SetLineColor(kBlack);
  vec_hist[0]->SetMarkerStyle(20);
  //vec_hist[0]->SetFillColorAlpha(kGray,0.9);
  
  //vec_hist[1]->RebinX();
  vec_hist[1]->SetLineColor(kBlack);
  vec_hist[1]->SetFillColorAlpha(kAzure-9,0.6);
  double scale_factor = vec_hist[0]->GetMaximum() / vec_hist[1]->GetMaximum();
  //double scale_factor = vec_hist[0]->Integral("width") / vec_hist[1]->Integral("width");
  vec_hist[1]->Scale(scale_factor);
  hs->Add(vec_hist[1],"hist");
  hs->Add(vec_hist[0],"e1");
  
  //hs->SetTitle(stack_title.c_str());
  hs->Draw("nostack");
  hs->SetTitle("; #left|p^{#mu}_{MM_{X}}#right|^{2} #lower[0.15]{(GeV^{2} )}; arb. units");
  hs->GetYaxis()->SetMaxDigits(3);
  // Draw legend
  auto legend = new TLegend(pxmin*0.65,pymin*0.75,pxmax*0.9,pymax*0.91); //top right corner
  legend->SetBorderSize(0);
  legend->SetTextSize(0.065);
  legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
  legend->AddEntry(vec_hist[0], "Data", "lep");
  legend->AddEntry(vec_hist[1], "Recon MC", "f");
  legend->Draw("same");  
  //Draw cut line and arrow
  auto l1 = new TLine(-0.02,0,-0.02,vec_hist[1]->GetMaximum());
  l1->SetLineColor(kBlue);
  l1->SetLineWidth(4);
  l1->Draw();
  auto l2 = new TLine(0.02,0,0.02,vec_hist[1]->GetMaximum()*0.8);
  l2->SetLineColor(kBlue);
  l2->SetLineWidth(4);
  l2->Draw();
  auto lar = new TArrow(-0.02,vec_hist[1]->GetMaximum()*0.6,0.02,vec_hist[1]->GetMaximum()*0.6,0.04,"<|>");
  lar->SetLineColor(kBlue);
  lar->SetLineWidth(4);
  lar->SetFillColor(kBlue);
  lar->Draw();
  //Save plot
  gPad->Update();
  c->SaveAs((save_name+delim+".pdf").c_str());
}

// input specific style formatting of user choice
void style_format()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(700);
  gStyle->SetCanvasDefW(800);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.21);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.05);
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
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
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
  
  gStyle->SetTitleOffset(1.05,"X");
  gStyle->SetTitleOffset(1.0,"Y");
  
  gStyle->SetTextSize(0.08);
  gStyle->SetTextFont(132);
  gStyle->SetOptStat(0);

  TLatex* latex = new TLatex();
  latex->SetNDC();
  latex->SetTextFont(132);
  latex->SetTextSize(0.08);
  latex->SetTextAlign(32);
  gROOT->ForceStyle();
}
