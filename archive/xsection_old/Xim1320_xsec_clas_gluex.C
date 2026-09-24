// kpkpxim xsection clas and gluex data

#include "TF1.h"
#include "TH1.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include "RooPlot.h"

void SetStyle();

void Xim1320_xsec_clas_gluex()
{
  SetStyle();
  string dataPath = "/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/data/";
  TCanvas *c = new TCanvas("c", "c");

  TGraphErrors *g1 = new TGraphErrors("/d/grid17/hjesse/analysis/Clas_data.csv", "%lg %lg %lg");//, option=" \t,;");
  g1->SetTitle("Clas Data");
  g1->SetMarkerStyle(22);
  g1->SetMarkerSize(1.2);
  g1->SetDrawOption("AP");
  g1->SetMarkerColor(kOrange+7);
  g1->SetLineWidth(2);
  g1->SetFillStyle(0);  

  TGraphErrors *g2 = new TGraphErrors(  (dataPath+"totxsec_kpkpxim__B4_M23_2018-01_ana03_nominal_allKaonSep.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  //TGraphErrors *g3 = new TGraphErrors("xsec_val_kpkpxim_2017-01_ana-45.txt", "%lg %lg %lg %lg");//, option=" \t,;");
  g2->SetTitle("Spring 2017 ANAver56");
  g2->SetMarkerStyle(20);
  g2->SetMarkerSize(1.2);
  g2->SetDrawOption("AP");
  g2->SetMarkerColor(kGreen);
  g2->SetLineColor(kGreen);
  g2->SetLineWidth(2);
  g2->SetFillStyle(0);  

  TGraphErrors *g3 = new TGraphErrors(  (dataPath+"totxsec_kpkpxim__B4_M23_2018-08_ana02_nominal_allKaonSep.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  //TGraphErrors *g3 = new TGraphErrors("xsec_val_kpkpxim_2018-01_ana-03.txt", "%lg %lg %lg %lg");
  g3->SetTitle("Spring 2018 ANAver03");
  g3->SetMarkerStyle(20);
  g3->SetMarkerSize(1.2);
  g3->SetDrawOption("AP");
  g3->SetMarkerColor(kRed);
  g3->SetLineColor(kRed);
  g3->SetLineWidth(2);
  g3->SetFillStyle(0);
  
  TGraphErrors *g4 = new TGraphErrors( (dataPath+"totxsec_kpkpxim__M23_2017-01_ana56_nominal_allKaonSep.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  //TGraphErrors *g4 = new TGraphErrors("xsec_val_kpkpxim_2018-08_ana-02.txt", "%lg %lg %lg %lg");
  g4->SetTitle("Fall 2018 ANAver02");
  g4->SetMarkerStyle(20);
  g4->SetMarkerSize(1.2);
  g4->SetDrawOption("AP");
  g4->SetMarkerColor(kBlue);
  g4->SetLineColor(kBlue);
  g4->SetLineWidth(2);
  g4->SetFillStyle(0);  

  TMultiGraph *mg = new TMultiGraph();
  mg->SetTitle(" ; E_{#gamma} (GeV); #sigma(#gammap #rightarrow K^{+}K^{+}#Xi^{-}) (nb)");//#int #frac{d#sigma}{dt}(#gammap #rightarrow K^{+}K^{+}#Xi^{-}) (nb)");
  mg->Add(g1);
  mg->Add(g2);
  mg->Add(g3);
  mg->Add(g4);
    
  mg->Draw("ap");
  gPad->SetGrid(0,1);
  mg->GetXaxis()->SetLimits(0,12);
  mg->GetYaxis()->SetRangeUser(0,17);
  TF1 *fit = new TF1("fit","expo(0)",4.3,12);
  fit->SetParameters(3.29572e+00,-2.15093e-01);
  fit->SetLineWidth(4);
  fit->SetLineColor(kBlack);
  fit->SetLineStyle(7);
  fit->SetNpx(1000);
  mg->Fit("fit", "LR");
  Double_t  chisqndf = fit->GetChisquare() / fit->GetNDF();
  //
  //g5->Draw("same e3");
  //
  TLatex latex;
  latex.SetTextFont(132);
  latex.SetTextSize(0.04);
  latex.SetTextAlign(12);
  latex.DrawLatex(9.6,10.5,("(#chi^{2}_{#nu} = " + to_string(chisqndf).substr(0,4)+")").c_str());
  //
  auto legend = new TLegend(0.62,0.59,0.948,0.91);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  legend->SetFillStyle(0);
  //legend->SetHeader(" ","C"); // option "C" allows to center the header
  legend->AddEntry(g1,"CLAS Data","lep");
  //legend->AddEntry(g5,"GlueX-I","f");
  legend->AddEntry(g2,"Spring 2017","lep");
  legend->AddEntry(g3,"Spring 2018","lep");
  legend->AddEntry(g4,"Fall 2018","lep");
  legend->AddEntry(fit,"Expo","l");

  gStyle->SetOptFit(0);  
  legend->Draw();

  c->SaveAs("/d/grid17/hjesse/AnalysisNote/xsection/plots/Xi1320Fit_clas_gluex_Phase1.pdf");
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
