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
void make_plot(vector<string> delim, string rootFile, vector<string> legTitle, string prelim="Diff_xsec");
TGraphErrors* calc_barlow(TGraphErrors *nominal, TGraphErrors *variation);
//main  
int GetBarlowResults()
{
  SetStyle();
  vector<string> varStr = {"_chisqndf+1", "_kp_momsep_+05", "_lambda_pathlensig+05",
                           "_total_mm2+005", "_xim_pathlensig-05"};
  vector<string> niceStr = {"#chi^{2}_{#nu}+1", "|#vec{p}(K^{+}_{fast}-K^{+}_{slow})|+0.5GeV", "FS(#Lambda)+0.5",
                            "|MM^{2}|+0.005GeV^{2}","FS(#Xi^{-})-0.5" };
  vector<string> setStr = {"Spring 2017","Spring 2018", "Fall 2018", "GlueX-I"};
    //
  for(int i=0; i<10; i++)
    {
      for(int j=0; j<varStr.size(); j++)
        {
          //Get barlow for variations for 3 run periods
          make_plot({"_nominal_allKaonSep_weighted_enBin"+to_string(i),"_vary"+varStr[j]+"_weighted_enBin"+to_string(i)}, "_kpkpxim__M23_2017-01_ana56", {setStr[0],niceStr[j]});
          make_plot({"_nominal_allKaonSep_weighted_enBin"+to_string(i),"_vary"+varStr[j]+"_weighted_enBin"+to_string(i)}, "_kpkpxim__B4_M23_2018-01_ana03", {setStr[1],niceStr[j]});
          make_plot({"_nominal_allKaonSep_weighted_enBin"+to_string(i),"_vary"+varStr[j]+"_weighted_enBin"+to_string(i)}, "_kpkpxim__B4_M23_2018-08_ana02", {setStr[2],niceStr[j]});
          //Get barlow for variations of weighted average
          make_plot({"_allKaonSep_enBin"+to_string(i),varStr[j]+"_enBin"+to_string(i)},
                    "", {setStr[3],niceStr[j]}, "diff_xsec_avgw");
        }
    }
  
  return 0;
}

void make_plot(vector<string> delim, string rootFile, vector<string> legTitle, string prelim="Diff_xsec")
{
  
  string dataNomPath = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/data_files/");
  string dataVarPath = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/data_files/");
  string saveDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/results/");
  //Make Canvas 
  TCanvas *c = new TCanvas("c", "c");
  auto *p2 = new TPad("p2","p3",0.,0.,1.,0.6); p2->Draw();
  p2->SetTopMargin(0.001);
  p2->SetBottomMargin(0.4);
  //p2->SetLogx ();
  p2->SetGrid(0,1);

  auto *p1 = new TPad("p1","p1",0.,0.3,1.,1.);  p1->Draw();
  p1->SetBottomMargin(0.001);
  p1->SetLogy();
  p1->cd();
  p1->SetGrid(0,1);
   
  TGraphErrors *g1 = new TGraphErrors((dataNomPath+prelim+rootFile+delim[0]+".txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  g1->SetTitle("Clas Data");
  g1->SetMarkerStyle(21);
  g1->SetMarkerSize(1.2);
  g1->SetDrawOption("AP");
  g1->SetMarkerColor(kAzure);
  g1->SetLineColor(kAzure);
  g1->SetLineWidth(3);
  g1->SetFillStyle(0);  
  //g1->Draw("ap");
  
  TGraphErrors *g2 = new TGraphErrors(  (dataVarPath+prelim+rootFile+delim[1]+".txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  g2->SetTitle("Weighted");
  g2->SetMarkerStyle(38);
  g2->SetMarkerSize(1.2);
  g2->SetDrawOption("AP");
  g2->SetMarkerColor(kOrange+7);
  g2->SetLineColor(kOrange+7);
  g2->SetLineWidth(2);
  g2->SetFillStyle(0);
  //g2->Draw("ap same");
  
  TMultiGraph *mg = new TMultiGraph();
  mg->SetTitle(" ; -t (GeV^{2} );#frac{d#sigma(#gammap #rightarrow K^{+}K^{+}#Xi^{-})}{dt} (nb/GeV^{2} )");//=(p^{#mu}_{#gamma}-p^{#mu}_{K^{+}_{fast}})^{2}
  mg->Add(g1);
  mg->Add(g2);

  mg->Draw("ap");
  mg->GetXaxis()->SetLimits(0,2.);
  //mg->GetYaxis()->SetRangeUser(0.3,11);
  //
  auto legend = new TLegend(0.28,0.03,0.5,0.40);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.06);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  legend->SetFillStyle(0);
  legend->SetHeader(legTitle[0].c_str(),"R"); // option "C" allows to center the header
  legend->AddEntry((TObject*)0,legTitle[1].c_str(),"");
  legend->AddEntry(g1,"Nominal","lep");
  legend->AddEntry(g2,"Variation","lep");
  //
  legend->Draw();
  //ratio plot
  p2->cd();
  TGraphErrors *barlow = (TGraphErrors*)calc_barlow(g1,g2);
  barlow->SetTitle(" ; -t (GeV^{2} ); |#sigma_{barlow}|");
  barlow->GetYaxis()->CenterTitle(true);
  barlow->GetXaxis()->SetRangeUser(0,2.);
  if(barlow->GetMaximum()<4 )
    barlow->GetYaxis()->SetRangeUser(-0.1,4.5);
  else
    barlow->GetYaxis()->SetRangeUser(-0.1,barlow->GetMaximum()+0.5);
  barlow->GetYaxis()->SetNdivisions(505);
  barlow->GetXaxis()->SetLabelSize(0.15);
  barlow->GetYaxis()->SetLabelSize(0.15);
  barlow->GetXaxis()->SetTitleSize(0.18);
  barlow->GetYaxis()->SetTitleSize(0.18);
  barlow->GetXaxis()->SetTitleOffset(0.9);
  barlow->GetYaxis()->SetTitleOffset(0.4);  
  barlow->Draw("ap");
  
  c->SaveAs((saveDir+"barlow"+rootFile+delim[1]+".pdf").c_str());
}

TGraphErrors* calc_barlow(TGraphErrors *nominal, TGraphErrors *variation)
{
    TGraphErrors *graph = new TGraphErrors();
    double Delta, sigma;
    for(int i = 0; i < nominal->GetN(); i++) {
      Delta = abs(nominal->GetPointY(i) - variation->GetPointY(i));
      sigma = TMath::Sqrt( abs( nominal->GetErrorY(i)*nominal->GetErrorY(i) - variation->GetErrorY(i)*variation->GetErrorY(i) ) );
      cout << Delta << "    " << sigma << "     " << Delta / sigma << endl;
      if(sigma != 0.0)    graph->SetPointY( i, Delta/sigma );
      else                graph->SetPointY( i, 0.0 );
      graph->SetPointX( i, nominal->GetPointX(i) );
      graph->SetPointError( i, variation->GetErrorX(i), 0 );
      // cout << i << "   " << Delta / sigma << endl;
      // cout << "    "<< sigma << endl;
    }
    return graph;
}

void SetStyle()
{
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(5);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(800);
  gStyle->SetCanvasDefW(600);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.16);
  gStyle->SetPadTopMargin   (0.05);
  gStyle->SetPadLeftMargin  (0.2);
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

  gStyle->SetLabelSize(0.06,"X");
  gStyle->SetLabelSize(0.06,"Y");

  gStyle->SetLabelOffset(0.008,"X");
  gStyle->SetLabelOffset(0.008,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132);
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");

  gStyle->SetTitleSize(0.07,"X");
  gStyle->SetTitleSize(0.07,"Y");

  gStyle->SetTitleOffset(1.1,"X");
  gStyle->SetTitleOffset(1.,"Y");

  gStyle->SetTextSize(0.06);
  gStyle->SetTextFont(132);

  gStyle->SetOptStat(0);

  gROOT->ForceStyle();

  TLatex* latex = new TLatex();
  latex->SetNDC();
  //latex->SetTextFont(42);
  latex->SetTextSize(0.04);
  latex->SetTextAlign(32);
}
