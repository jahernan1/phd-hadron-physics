#include "gxana/common/Paths.h"
// kpkpxim xsection clas and gluex data

#include "TF1.h"
#include "TH1.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include "RooPlot.h"

void SetStyle();

void PlotTotXsecWithClas(string delim="_allCuts")
{
  SetStyle();
  string dataPath = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/data/hybrid_combo/");
  string wdataPath = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/weighted_data/hybrid_combo/");
  TCanvas *c = new TCanvas("c", "c");

  TGraphErrors *g1 = new TGraphErrors(gxana::EnvPath("GXANA_ROOT", "analyses/kpkpxim/xsection/external_data/Clas_data.csv").c_str(), "%lg %lg %lg");//, option=" \t,;");
  g1->SetTitle("CLAS Data");
  g1->SetMarkerStyle(22);
  g1->SetMarkerSize(1.2);
  g1->SetDrawOption("AP");
  g1->SetMarkerColor(kOrange+7);
  g1->SetLineWidth(2);
  g1->SetFillStyle(0);  

  TGraphErrors *g2 = new TGraphErrors(  (wdataPath+"totxsec_weighted_output.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  g2->SetTitle("Weighted");
  g2->SetMarkerStyle(24);
  g2->SetMarkerSize(1.2);
  g2->SetDrawOption("AP");
  g2->SetMarkerColor(kBlack);
  g2->SetLineColor(kBlack);
  g2->SetLineWidth(2);
  g2->SetFillColorAlpha(kBlack,0.3);
  //g2->SetFillStyle(0);  

  TGraphErrors *g3 = new TGraphErrors(  (dataPath+"totxsec_flatTree_kpkpxim__M23_2017-01_ana56.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  g3->SetTitle("Spring 2017");
  g3->SetMarkerStyle(20);
  g3->SetMarkerSize(1.2);
  g3->SetDrawOption("AP");
  g3->SetMarkerColor(kRed);
  g3->SetLineColor(kRed);
  g3->SetLineWidth(2);
  g3->SetFillStyle(0);

  TGraphErrors *g4 = new TGraphErrors( (dataPath+"totxsec_flatTree_kpkpxim__B4_M23_2018-01_ana03.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  g4->SetTitle("Spring 2018");
  g4->SetMarkerStyle(20);
  g4->SetMarkerSize(1.2);
  g4->SetDrawOption("AP");
  g4->SetMarkerColor(kBlue);
  g4->SetLineColor(kBlue);
  g4->SetLineWidth(2);
  g4->SetFillStyle(0);  
  
  TGraphErrors *g5 = new TGraphErrors( (dataPath+"totxsec_flatTree_kpkpxim__B4_M23_2018-08_ana02.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  g5->SetTitle("Spring 2018");
  g5->SetMarkerStyle(20);
  g5->SetMarkerSize(1.2);
  g5->SetDrawOption("AP");
  g5->SetMarkerColor(kSpring+9);
  g5->SetLineColor(kSpring+9);
  g5->SetLineWidth(2);
  g5->SetFillStyle(0);  
  
  // TGraphErrors *g5 = new TGraphErrors( (dataPath+"intxsec_avgw"+delim+".txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  // g5->SetTitle("GlueX-I Data");
  // g5->SetMarkerStyle(20);
  // g5->SetMarkerSize(1.2);
  // g5->SetDrawOption("e4");
  // g5->SetMarkerColor(kSpring);
  // g5->SetLineColor(kSpring);
  // //g5->SetFillColorAlpha(kAzure,0.4);
  // g5->SetLineWidth(2);
  // //g5->SetFillStyle(0);  
  
  TMultiGraph *mg = new TMultiGraph();
  mg->SetTitle(" ; E_{#gamma} (GeV);#sigma(E_{#gamma})#lower[0.1]{#scale[1.7]{|}}^{t_{max}}_{t_{min}} (nb)");
  //#gammap #rightarrow K^{+}K^{+}#Xi^{-}
  mg->Add(g1);
  //mg->Add(g2);
  mg->Add(g3);
  mg->Add(g4);
  mg->Add(g5);
  
  mg->Draw("ap");
  gPad->SetGrid(0,1);
  mg->GetXaxis()->SetLimits(2,12);
  mg->GetYaxis()->SetRangeUser(0,18);
  TF1 *fit = new TF1("fit","expo",4.3,11.4);
  fit->SetParameters(3.3,-0.2);
  fit->SetLineWidth(4);
  fit->SetLineColor(kBlack);
  fit->SetLineStyle(7);
  fit->SetNpx(1000);
  mg->Fit("fit", "R");
  Double_t  chisqndf = fit->GetChisquare() / fit->GetNDF();
  TLatex latex;
  latex.SetTextFont(132);
  latex.SetTextSize(0.04);
  latex.SetTextAlign(12);
  latex.DrawLatex(10,9.7,("(#chi^{2}_{#nu} = " + to_string(chisqndf).substr(0,4)+")").c_str());
  //
  g2->Draw("e3 same");
  
  auto legend = new TLegend(0.62,0.55,0.948,0.91);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  legend->SetFillStyle(0);
  //legend->SetHeader(" ","C"); // option "C" allows to center the header
  legend->AddEntry(g1,"CLAS Data","lep");
  legend->AddEntry(g2,"GlueX-I","f");
  //legend->AddEntry(g5,"GlueX-I","f");
  legend->AddEntry(g3,"Spring 2017","lep");
  legend->AddEntry(g4,"Spring 2018","lep");
  legend->AddEntry(g5,"Fall 2018","lep");
  legend->AddEntry(fit,"Expo","l");
  

  gStyle->SetOptFit(0);
  legend->Draw();

  c->SaveAs("totxsec_clas_gluex_Phase1.pdf");
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
  gStyle->SetPadTopMargin   (0.05);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.04);
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
