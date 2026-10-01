#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
//instantiate methods used in main
void get_plot_chisqndf(string delim="_kphighrap");
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string delim, double chisqcut=8);
void load_histos(TFile *f, string dir, vector<TH1D*> vect_histo);
void style_format();

// main function (change main name when copying)
int make_plot_chisqndf()
{
  //get_plot_chisqndf("_vertexCuts");
  get_plot_chisqndf();
  return 0;
}

void get_plot_chisqndf(string delim="_kphighrap")
{
  //initiate variables
  string thisDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/event_selection/chisqndf_cut/");
  vector<TH1D*> vect_histo_2017;
  vector<TH1D*> vect_histo_201801;
  vector<TH1D*> vect_histo_201808;
  
  // Load root tree of choice
  TFile *f = TFile::Open( (thisDir + "data"+delim+"_RF.root").c_str() );
  vect_histo_2017.push_back( (TH1D*)f->Get("Spring_2017/hist_chisqndf")->Clone() );
  vect_histo_2017.push_back( (TH1D*)f->Get("Spring_2017/hist_chisqndf_mc" )->Clone() );
  //
  vect_histo_201801.push_back( (TH1D*)f->Get("Spring_2018/hist_chisqndf")->Clone() );
  vect_histo_201801.push_back( (TH1D*)f->Get("Spring_2018/hist_chisqndf_mc" )->Clone() );
  //
  vect_histo_201808.push_back( (TH1D*)f->Get("Fall_2018/hist_chisqndf")->Clone() );
  vect_histo_201808.push_back( (TH1D*)f->Get("Fall_2018/hist_chisqndf_mc" )->Clone() );
  // Call function to plot 
  make_plot(vect_histo_2017, "chisqndf_data_mc_2017", "Spring 2017", delim);
  make_plot(vect_histo_201801, "chisqndf_data_mc_201801", "Spring 2018", delim);
  make_plot(vect_histo_201808, "chisqndf_data_mc_201808", "Fall 2018", delim);
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
  
  // Plot histogram(s) algorithm 
  TCanvas *c = new TCanvas("leg_title","leg_title");
  THStack *hs = new THStack("leg_title","");
  
  double pxmax = c->GetUxmax();
  double pxmin = c->GetUxmax();
  double pymax = c->GetUymax();
  double pymin = c->GetUymax();
  
  //vec_hist[0]->SetFillColorAlpha(kGray,0.9);
  vec_hist[0]->SetMarkerStyle(20);
  vec_hist[0]->SetLineColor(kBlack);
  //vec_hist[0]->RebinX();
  

  vec_hist[1]->SetFillColorAlpha(kAzure-9,0.6);
  vec_hist[1]->SetLineColor(kBlack);
  //vec_hist[1]->RebinX();
  double scale_factor = vec_hist[0]->GetMaximum() / vec_hist[1]->GetMaximum();
  //double scale_factor = vec_hist[0]->Integral("width") / vec_hist[1]->Integral("width");
  vec_hist[1]->Scale(scale_factor);
  hs->Add(vec_hist[1],"hist");
  hs->Add(vec_hist[0],"e1");
  
  //hs->SetTitle(stack_title.c_str());
  hs->Draw("nostack");
  hs->SetTitle("; #chi^{2}_{#nu}; arb. units");
    
  // Draw legend
  auto legend = new TLegend(pxmin*0.65,pymin*0.73,pxmax*0.9,pymax*0.91); //top right corner
  legend->SetBorderSize(0);
  legend->SetTextSize(0.065);
  legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
  legend->AddEntry(vec_hist[0], "Data", "lep");
  //legend->AddEntry(vec_hist[0], "Data (Q-Value)", "f");
  legend->AddEntry(vec_hist[1], "Recon MC", "f");
  legend->Draw("same");
  
  //Draw cut line and arrow
  auto l = new TLine(chisqcut, 0, chisqcut, vec_hist[1]->GetMaximum()*1.05);
  l->SetLineColor(kBlue);
  l->SetLineWidth(4);
  l->Draw("same");
  auto lar = new TArrow(chisqcut ,vec_hist[1]->GetMaximum()*0.7, 4, vec_hist[1]->GetMaximum()*0.7, 0.05, "|>");
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
    gxana::StyleParams p = gxana::CutStudyStyle();
    p.canvasDefH = 700;
    p.canvasDefW = 800;
    gxana::ApplyStyle(p);
}
