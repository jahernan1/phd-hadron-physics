#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
//instantiate methods used in main
void get_plot(string delim="_allKaonSep");
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string delim, string axis_title);
void style_format();

// main function (change main name when copying)
int make_plot()
{
  style_format();
  //get_plot("_vertexCuts");
  get_plot("_momCut");
  return 0;
}

void get_plot(string delim="_allCuts")
{
  //initiate variables
  string thisDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/event_selection/accidentals/");
  vector<vector<TH1D*>> vecHistXimMass{{},{}};
  vector<vector<TH1D*>> vecHistChiSqNdf{{},{}};
  
  // Load root tree of choice
  TFile *f = TFile::Open( (thisDir + "data"+delim+".root").c_str() );
  // Load Plots
  vecHistXimMass[0].push_back((TH1D*)f->Get("Fall_2018/decayxim_M_bc")->Clone());
  vecHistXimMass[0].push_back((TH1D*)f->Get("Fall_2018/decayxim_M_rf")->Clone());
  vecHistXimMass[0].push_back((TH1D*)f->Get("Fall_2018/decayxim_M_as")->Clone());
   
  vecHistXimMass[1].push_back((TH1D*)f->Get("Fall_2018/decayxim_M_bc_mc")->Clone());
  vecHistXimMass[1].push_back((TH1D*)f->Get("Fall_2018/decayxim_M_rf_mc")->Clone());
  vecHistXimMass[1].push_back((TH1D*)f->Get("Fall_2018/decayxim_M_as_mc")->Clone());
  //
  vecHistChiSqNdf[0].push_back((TH1D*)f->Get("Fall_2018/chisqndf_bc")->Clone());
  vecHistChiSqNdf[0].push_back((TH1D*)f->Get("Fall_2018/chisqndf_rf")->Clone());
  vecHistChiSqNdf[0].push_back((TH1D*)f->Get("Fall_2018/chisqndf_as")->Clone());
   
  vecHistChiSqNdf[1].push_back((TH1D*)f->Get("Fall_2018/chisqndf_bc_mc")->Clone());
  vecHistChiSqNdf[1].push_back((TH1D*)f->Get("Fall_2018/chisqndf_rf_mc")->Clone());
  vecHistChiSqNdf[1].push_back((TH1D*)f->Get("Fall_2018/chisqndf_as_mc")->Clone());
  
  // Call function to plot 
  make_plot(vecHistXimMass[0], "decayxim_M_combo_methods", "Fall 2018", delim, "; M(#Lambda#pi^{-}) (GeV); Events");
  make_plot(vecHistXimMass[1], "decayxim_M_mc_combo_methods", "Fall 2018 (MC)", delim, "; M(#Lambda#pi^{-}) (GeV); Events");
  //
  make_plot(vecHistChiSqNdf[0], "chisqndf_combo_methods", "Fall 2018", delim, "; #chi^{2}_{#nu}; Events");
  make_plot(vecHistChiSqNdf[1], "chisqndf_mc_combo_methods", "Fall 2018 (MC)", delim, "; #chi^{2}_{#nu}; Events");
}

// make plot to be called by main function
/* Arguments Description:
hist: vector of histograms to plot
legend_title: title of legend (name of data set)
*/
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string delim, string axis_title)
{
  // call plot styling function
  //style_format();
  string saveloc = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/event_selection/plots/");
  // Plot histogram(s) algorithm 
  TCanvas *c = new TCanvas("leg_title","leg_title");
  THStack *hs = new THStack("leg_title","");

  vec_hist[0]->RebinX();
  vec_hist[0]->SetLineColor(kSpring-6);
  vec_hist[0]->SetMarkerColor(kSpring-6);
  vec_hist[0]->SetMarkerStyle(20);
  vec_hist[0]->SetMarkerSize(0.7);
  //vec_hist[0]->SetFillColorAlpha(kGray,0.9);
  hs->Add(vec_hist[0],"e1");
  
  vec_hist[1]->RebinX();
  vec_hist[1]->SetLineColor(kRed+1);
  vec_hist[1]->SetMarkerColor(kRed+1);
  vec_hist[1]->SetMarkerStyle(20);
  vec_hist[1]->SetMarkerSize(0.7);
  //vec_hist[1]->SetFillColorAlpha(kAzure-9,0.6);
  hs->Add(vec_hist[1],"e1");

  vec_hist[2]->RebinX();
  vec_hist[2]->SetLineColor(kAzure);
  vec_hist[2]->SetMarkerColor(kAzure);
  vec_hist[2]->SetMarkerStyle(20);
  vec_hist[2]->SetMarkerSize(0.7);
  //vec_hist[2]->SetFillColorAlpha(kAzure-9,0.6);
  hs->Add(vec_hist[2],"e1");
  
  //hs->SetTitle(stack_title.c_str());
  hs->Draw("nostack");
  hs->SetTitle(axis_title.c_str());
  hs->GetYaxis()->SetMaxDigits(3);
  // Draw legend
  auto legend = new TLegend(0.65,0.70,0.9,0.9); //top right corner
  legend->SetBorderSize(0);
  legend->SetTextSize(0.065);
  legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
  legend->AddEntry(vec_hist[0], "Best #chi^{2}", "lep");
  legend->AddEntry(vec_hist[1], "Hydrid", "lep");
  legend->AddEntry(vec_hist[2], "RF Sub.", "lep");
  legend->Draw("same");
  
  //Save plot
  gPad->Update();
  c->SaveAs(("plots/"+save_name+delim+".pdf").c_str());
}

// input specific style formatting of user choice
void style_format()
{
    gxana::StyleParams p = gxana::CutStudyStyle();
    p.markerSize = 1.0;
    p.markerStyle = 24;
    gxana::ApplyStyle(p);
}
