#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
//instantiate methods used in main
void MakeStackedHist(vector<TH1D*> arr_hist, string stack_title, string identifier="", string leg_pos="tl", string leg_title="GlueX#lower[-0.15]{-}#kern[0.1]{I}");
void style_format();

// main function 
int PlotTDistComparison()
{
  //style function
  style_format();

  // Open the TFile
  TFile *file = TFile::Open("data_ximVertexCut.root", "READ");
  TFile *file1 = TFile::Open("data_kphighrap.root", "READ");
  if (!file || file->IsZombie()) {
      std::cerr << "Error opening file: " << file << std::endl;
      return -1;
  }// Load root tree of choice

  TH1D* hist_tdist = (TH1D*)file->Get("TDist_qval");
  TH1D* hist_tdist1 = (TH1D*)file1->Get("TDist_qval");
  TH1D* hist_tdistmc = (TH1D*)file1->Get("TDist_mc");
  MakeStackedHist({hist_tdist, hist_tdistmc, hist_tdist1}," ; -t_{#lower[-0.2]{#gammaK^{+}_{#it{#lower[-0.3]{fast}}}}} (GeV^{2} ); arb. units","_CutComparison", "tr");

  return 0;
}

void MakeStackedHist(vector<TH1D*> arr_hist, string stack_title, string identifier="", string leg_pos="tl", string leg_title="GlueX-I")
{
  string plotdir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/event_selection/rapidity_cuts/");
  string histName = arr_hist[0]->GetName();  
  string saveName = plotdir + histName + identifier +".pdf";
  char entries[4][100];
  vector<Color_t> arr_colors = {kBlue, kMagenta, kRed, kCyan, kYellow, kOrange};
  vector<Style_t> arr_marker_style = {kFullCircle, kFullSquare, kFullTriangleUp, kFullStar, kFullTriangleDown, kFullDiamond};
  TCanvas *c = new TCanvas((histName+"_"+identifier).c_str(), (histName+"_"+identifier).c_str());
  THStack *hs = new THStack("hs","");
  gPad->SetLogy();
  
  double pxmax = c->GetUxmax();
  double pxmin = c->GetUxmax();
  double pymax = c->GetUymax();
  double pymin = c->GetUymax();
  double scale_factor;
  //arr_hist[0]->SetFillColorAlpha(kGray,0.9);
  arr_hist[0]->SetMarkerStyle(20);
  arr_hist[0]->SetMarkerSize(0.8);
  arr_hist[0]->SetMarkerColor(kBlack);
  arr_hist[0]->SetLineColor(kBlack);
  
  for(Int_t i = 1; i < arr_hist.size() ; i++){
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

  }

  arr_hist[1]->SetLineColor(kBlack);
  arr_hist[1]->SetFillColorAlpha(kAzure-9,0.6);
    
  arr_hist[2]->SetMarkerStyle(20);
  arr_hist[2]->SetMarkerSize(0.8);
  arr_hist[2]->SetMarkerColor(kRed+1);
  arr_hist[2]->SetLineColor(kRed+1);
  
  //arr_hist[0]->RebinX();
  hs->Add(arr_hist[1],"hist");
  hs->Add(arr_hist[0],"e1");
  hs->Add(arr_hist[2],"e1");

  hs->SetTitle(stack_title.c_str());
  hs->Draw("nostack");
  hs->GetYaxis()->SetMaxDigits(3);
  //hs->GetXaxis()->SetLimits(0,3.5);
  hs->SetMinimum(1e-2);
  hs->SetMaximum(1e4);
  // Draw legend
  TLegend* legend;
  if(leg_pos=="tl")
    legend = new TLegend(0.16,0.66,0.4,0.94); //top left corner
  else
    legend = new TLegend(0.64,0.67,0.93,0.95); //top right corner
    
  legend->SetBorderSize(0);
  legend->SetTextSize(0.055);
  legend->SetHeader(leg_title.c_str(),"C"); // option "C" alhighs to center the header
  
  legend->AddEntry(arr_hist[0], "Data", "lep");
  legend->AddEntry(arr_hist[2], "y(K^{+}_{#it{#lower[-0.3]{fast}}}) > 2", "lep");
  legend->AddEntry(arr_hist[1], "Monte Carlo", "f");
  legend->Draw("same");

  auto l = new TLine(3, 0, 3, 1e2);
  l->SetLineColor(kRed+1);
  l->SetLineStyle(kDashed);
  l->SetLineWidth(3);
  //l->Draw("same");
  
  auto l1 = new TLine(2.4, 0, 2.4, arr_hist[1]->GetMaximum()*1.1);
  l1->SetLineColor(kAzure);
  //l1->SetLineStyle(kDashed);
  l1->SetLineWidth(4);
  l1->Draw("same");
  
  gPad->Update();
  c->SaveAs(saveName.c_str());
  //c->Close();
}
//
// input specific style formatting of user choice
void style_format()
{
    gxana::ApplyStyle(gxana::DistributionStyle());
}
