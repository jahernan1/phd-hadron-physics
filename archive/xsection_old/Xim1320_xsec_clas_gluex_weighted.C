// kpkpxim xsection clas and gluex data

#include "TF1.h"
#include "TH1.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include "RooPlot.h"

void SetStyle();
void make_plot(string delim="_allCuts");
  
int Xim1320_xsec_clas_gluex_weighted()
{
  //make_plot();
  make_plot("_allKaonSep");
  
  return 0;
}

void make_plot(string delim="_allCuts")
{
  SetStyle();
  string dataPath = "/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/data";
  TCanvas *c = new TCanvas("c", "c");

  TGraphErrors *g1 = new TGraphErrors("/d/grid17/hjesse/analysis/Clas_data.csv", "%lg %lg %lg");//, option=" \t,;");
  g1->SetTitle("Clas Data");
  g1->SetMarkerStyle(22);
  g1->SetMarkerSize(1.2);
  g1->SetDrawOption("AP");
  g1->SetMarkerColor(kOrange+7);
  g1->SetLineWidth(2);
  g1->SetFillStyle(0);  

  TGraphErrors *g22 = new TGraphErrors(  (dataPath+"Integrated_xsec_kpkpxim__M23_2017-01_ana56_nominal"+delim+"_weighted.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  //TGraphErrors *g3 = new TGraphErrors("xsec_val_kpkpxim_2017-01_ana-45.txt", "%lg %lg %lg %lg");//, option=" \t,;");
  g22->SetTitle("Weighted");
  g22->SetMarkerStyle(20);
  g22->SetMarkerSize(1.2);
  g22->SetDrawOption("AP");
  g22->SetMarkerColor(kGreen);
  g22->SetLineColor(kGreen);
  g22->SetLineWidth(2);
  g22->SetFillStyle(0);  

  TGraphErrors *g33 = new TGraphErrors(  (dataPath+"Integrated_xsec_kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_weighted.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  //TGraphErrors *g3 = new TGraphErrors("xsec_val_kpkpxim_2018-01_ana-03.txt", "%lg %lg %lg %lg");
  g33->SetTitle("Spring 2018 ANAver03");
  g33->SetMarkerStyle(20);
  g33->SetMarkerSize(1.2);
  g33->SetDrawOption("AP");
  g33->SetMarkerColor(kRed);
  g33->SetLineColor(kRed);
  g33->SetLineWidth(2);
  g33->SetFillStyle(0);
  
  TGraphErrors *g44 = new TGraphErrors( (dataPath+"Integrated_xsec_kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_weighted.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  //TGraphErrors *g44 = new TGraphErrors("xsec_val_kpkpxim_2018-08_ana-02.txt", "%lg %lg %lg %lg");
  g44->SetTitle("Fall 2018 ANAver02");
  g44->SetMarkerStyle(20);
  g44->SetMarkerSize(1.2);
  g44->SetDrawOption("AP");
  g44->SetMarkerColor(kBlue);
  g44->SetLineColor(kBlue);
  g44->SetLineWidth(2);
  g44->SetFillStyle(0);  
  
  TGraphErrors *g5 = new TGraphErrors( (dataPath+"intxsec_avgw"+delim+".txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  //TGraphErrors *g5 = new TGraphErrors("xsec_val_kpkpxim_2018-08_ana-02.txt", "%lg %lg %lg %lg");
  g5->SetTitle("GlueX-I Data");
  //g5->SetMarkerStyle(20);
  //g5->SetMarkerSize(1.2);
  g5->SetDrawOption("e4");
  //g5->SetMarkerColor(kAzure);
  g5->SetLineColor(kAzure);
  g5->SetFillColorAlpha(kAzure,0.4);
  g5->SetLineWidth(2);
  //g5->SetFillStyle(0);  
  
  TMultiGraph *mg = new TMultiGraph();
  mg->SetTitle(" ; E_{#gamma} (GeV);#sigma = #int #frac{#partial#sigma}{#partial t}(#gammap #rightarrow K^{+}K^{+}#Xi^{-}) dt (nb)");
  mg->Add(g1);
  mg->Add(g22);
  mg->Add(g33);
  mg->Add(g44);

  mg->Draw("ap");
  gPad->SetGrid(0,1);
  mg->GetXaxis()->SetLimits(0,12);
  mg->GetYaxis()->SetRangeUser(0,18);
  TF1 *fit = new TF1("fit","pol5",2,11.4);
  fit->SetLineWidth(4);
  fit->SetLineColor(kBlack);
  fit->SetLineStyle(7);
  fit->SetNpx(1000);
  mg->Fit("fit", "LR");
  Double_t  chisqndf = fit->GetChisquare() / fit->GetNDF();
  TLatex latex;
  latex.SetTextFont(132);
  latex.SetTextSize(0.04);
  latex.SetTextAlign(12);
  latex.DrawLatex(9,11,("(#chi^{2}_{#nu} = " + to_string(chisqndf).substr(0,4)+")").c_str());
  //
  g5->Draw("same e3");
  //
  auto legend = new TLegend(0.62,0.59,0.948,0.91);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  legend->SetFillStyle(0);
  //legend->SetHeader(" ","C"); // option "C" allows to center the header
  legend->AddEntry(g1,"CLAS Data","lep");
  legend->AddEntry(g5,"GlueX-I","f");
  legend->AddEntry(g22,"Spring 2017","lep");
  legend->AddEntry(g33,"Spring 2018","lep");
  legend->AddEntry(g44,"Fall 2018","lep");
  legend->AddEntry(fit,"Fit","l");
  //
  legend->Draw();
  //
  c->SaveAs(("/d/grid17/hjesse/AnalysisNote/xsection/plots/Xi_intxsec_clas_gluex_Phase1"+delim+"_WeightedMC.pdf").c_str());
}


void SetStyle()
{
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(5);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(600);
  gStyle->SetCanvasDefW(700);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.14);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.18);
  gStyle->SetPadRightMargin (0.05);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);

  gStyle->SetFrameFillStyle ( 0);
  gStyle->SetFrameFillColor ( 0);
  gStyle->SetFrameLineColor ( 1);
  gStyle->SetFrameLineStyle ( 0);
  gStyle->SetFrameLineWidth ( 2);
  gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);

  gStyle->SetNdivisions(505);

  gStyle->SetLineWidth(1);
  gStyle->SetHistLineWidth(1);
  //gStyle->SetLegendBorder(0);
  gStyle->SetFrameLineWidth(2);
  gStyle->SetLegendFillColor(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.0);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.05,"X");
  gStyle->SetLabelSize(0.05,"Y");

  gStyle->SetLabelOffset(0.009,"X");
  gStyle->SetLabelOffset(0.009,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132);
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");

  gStyle->SetTitleSize(0.065,"X");
  gStyle->SetTitleSize(0.065,"Y");

  gStyle->SetTitleOffset(1.,"X");
  gStyle->SetTitleOffset(1.,"Y");

  gStyle->SetTextSize(0.055);
  gStyle->SetTextFont(132);

  gStyle->SetOptStat(0);

  gROOT->ForceStyle();

  TLatex* latex = new TLatex();
  latex->SetNDC();
  //latex->SetTextFont(42);
  latex->SetTextSize(0.04);
  latex->SetTextAlign(32);
}
