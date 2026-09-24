#include "gxana/common/Paths.h"
/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
//instantiate methods used in main
void MakeStackedHist(vector<TH1D*> arr_hist, string stack_title, string identifier="", string leg_pos="tl", string leg_title="GlueX#lower[-0.15]{-}#kern[0.1]{I}");
void style_format();

// main function 
int PlotKPlusHighRapidity()
{
  //style function
  style_format();

  // Open the TFile
  TFile *file = TFile::Open("data_ximVertexCut.root", "READ");
  if (!file || file->IsZombie()) {
      std::cerr << "Error opening file: " << file << std::endl;
      return -1;
  }// Load root tree of choice

  TH1D* hist_high = (TH1D*)file->Get("KPlusHighRapidity_qval");
  new TCanvas;
  //hist_high->Draw("e1");
  TH1D* hist_highmc = (TH1D*)file->Get("KPlusHighRapidity_mc");
  MakeStackedHist({hist_high,hist_highmc},";y(K^{+}_{#it{#lower[-0.3]{fast}}});arb. units"); 
  
  return 0;
}

void MakeStackedHist(vector<TH1D*> arr_hist, string stack_title, string identifier="", string leg_pos="tl", string leg_title="GlueX#lower[-0.1]{-}I")
{
  string plotdir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/event_selection/rapidity_cuts/");
  string histName = arr_hist[0]->GetName();  
  string saveName = plotdir + histName + "_data_mc.pdf";
  char entries[4][100];
  vector<Color_t> arr_colors = {kBlue, kMagenta, kRed, kCyan, kYellow, kOrange};
  vector<Style_t> arr_marker_style = {kFullCircle, kFullSquare, kFullTriangleUp, kFullStar, kFullTriangleDown, kFullDiamond};
  TCanvas *c = new TCanvas((histName+"_"+identifier).c_str(), (histName+"_"+identifier).c_str());
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
    sprintf(entries[i], "%.0f Events", arr_hist[i]->GetEntries());

    hs->Add(arr_hist[i],"hist");
  }

  //arr_hist[0]->RebinX();
  hs->Add(arr_hist[0],"e1");

  hs->SetTitle(stack_title.c_str());
  hs->Draw("nostack");
  hs->GetYaxis()->SetMaxDigits(3);
  //hs->GetXaxis()->SetLimits(xmin,xmax);
    
  // Draw legend
  TLegend* legend;
  if(leg_pos=="tl")
    legend = new TLegend(0.16,0.75,0.4,0.94); //top left corner
  else
    legend = new TLegend(0.7,0.75,0.9,0.91); //top right corner
    
  legend->SetBorderSize(0);
  legend->SetTextSize(0.06);
  legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
  
  legend->AddEntry(arr_hist[0], "Data", "lep");
  legend->AddEntry(arr_hist[1], "Recon MC", "f");
  legend->Draw("same");

  auto l = new TLine(2, 0, 2, arr_hist[1]->GetMaximum()*1.05);
  l->SetLineColor(kBlue);
  //l->SetLineStyle(kDashed);
  l->SetLineWidth(3);
  l->Draw("same");
  auto lar = new TArrow(2,arr_hist[1]->GetMaximum()*1.02, 2.8, arr_hist[1]->GetMaximum()*1.02, 0.03, "|>");
  lar->SetLineColor(kBlue);
  lar->SetLineWidth(3);
  lar->SetFillColor(kBlue);
  lar->Draw();

  gPad->Update();
  c->SaveAs(saveName.c_str());
  //c->Close();
}
//
// input specific style formatting of user choice
void style_format()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  //gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(650);
  gStyle->SetCanvasDefW(800);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.17);
  gStyle->SetPadTopMargin   (0.04);
  gStyle->SetPadLeftMargin  (0.13);
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
  //gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);

  gStyle->SetNdivisions(510);
  
  //gStyle->SetLineWidth(1);
  //gStyle->SetHistLineWidth(1);
  //gStyle->SetFrameLineWidth(2);
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.05,"X");
  gStyle->SetLabelSize(0.05,"Y");

  gStyle->SetLabelOffset(0.008,"X");
  gStyle->SetLabelOffset(0.008,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132,"T");
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");
  gStyle->SetTitleFont(132,"Z");
  
  gStyle->SetTitleSize(0.07,"T");
  gStyle->SetTitleSize(0.07,"X");
  gStyle->SetTitleSize(0.07,"Y");
  
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(0.9,"Y");

  gStyle->SetTextSize(0.07);
  gStyle->SetTextFont(132);

  gStyle->SetOptStat(0);

  gROOT->ForceStyle();

  TLatex* latex = new TLatex();
  latex->SetNDC();
  latex->SetTextFont(132);
  latex->SetTextSize(0.07);
  latex->SetTextAlign(32);
  gROOT->ForceStyle();
}
