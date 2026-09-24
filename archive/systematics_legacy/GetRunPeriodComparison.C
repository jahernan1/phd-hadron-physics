// kpkpxim xsection clas and gluex data

#include "TF1.h"
#include "TH1.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include "RooPlot.h"

void SetStyle();
void make_plot(string delim, vector<string> rootFile, vector<string> setStr);
TGraphErrors* calc_signif(TGraphErrors *nominal, TGraphErrors *variation);

//main  
int GetRunPeriodComparison()
{
  SetStyle();
  vector<string> setStr = {"Spring-2017","Spring-2018", "Fall-2018", "GlueX-I"};
  vector<vector<double>> arrEnBins{{6.40,7.40},{7.40,7.86},{7.86,8.19},{8.19,8.45},{8.45,8.68},{8.68,9.26},{9.26,10.18},{10.18,11.40}};
  //
  for(int i=0; i<arrEnBins.size(); i++)
    { 
      //Get barlow for variations for 3 run periods
      string xmin = to_string(arrEnBins[i][0]); string xmax = to_string(arrEnBins[i][1]);
      string xminCut = xmin.substr(0,xmin.find_first_of(".")+3);
      string xmaxCut = xmax.substr(0,xmax.find_first_of(".")+3);
      make_plot("_nominal_allKaonSep_Emin-"+xminCut+"_Emax-"+xmaxCut, {"_kpkpxim__M23_2017-01_ana56","_kpkpxim__B4_M23_2018-01_ana03"}, {setStr[0],setStr[1]});
      make_plot("_nominal_allKaonSep_Emin-"+xminCut+"_Emax-"+xmaxCut, {"_kpkpxim__M23_2017-01_ana56","_kpkpxim__B4_M23_2018-08_ana02"}, {setStr[0],setStr[2]});
      make_plot("_nominal_allKaonSep_Emin-"+xminCut+"_Emax-"+xmaxCut, {"_kpkpxim__B4_M23_2018-01_ana03","_kpkpxim__B4_M23_2018-08_ana02"}, {setStr[1],setStr[2]});
    }
  
  return 0;
}

void make_plot(string delim, vector<string> rootFile, vector<string> setStr)
{
  
  string dataNomPath = "/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/data/gen_amp_V2/";
  string saveDir = "/d/grid17/hjesse/AnalysisNote/systematics/results/";
  //Make Canvas 
  TCanvas *c = new TCanvas("c", "c");
  auto *p2 = new TPad("p2","p3",0.,0.,1.,0.3); p2->Draw();
  p2->SetTopMargin(0.001);
  p2->SetBottomMargin(0.4);
  //p2->SetLogx ();
  p2->SetGrid(0,1);

  auto *p1 = new TPad("p1","p1",0.,0.3,1.,1.);  p1->Draw();
  p1->SetBottomMargin(0.001);
  p1->SetLogy();
  p1->cd();
  p1->SetGrid(0,1);
   
  TGraphErrors *g1 = new TGraphErrors((dataNomPath+"diffxsec"+rootFile[0]+delim+".txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  g1->SetTitle("");
  g1->SetMarkerStyle(21);
  g1->SetMarkerSize(1.2);
  g1->SetDrawOption("AP");
  g1->SetMarkerColor(kAzure);
  g1->SetLineColor(kAzure);
  g1->SetLineWidth(3);
  g1->SetFillStyle(0);  
  //g1->Draw("ap");
  
  TGraphErrors *g2 = new TGraphErrors(  (dataNomPath+"diffxsec"+rootFile[1]+delim+".txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
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
  legend->SetTextSize(0.075);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  legend->SetFillStyle(0);
  //legend->SetHeader(legTitle[0].c_str(),"C"); // option "C" allows to center the header
  //legend->AddEntry((TObject*)0,legTitle[1].c_str(),"");
  legend->AddEntry(g1,setStr[0].c_str(),"lep");
  legend->AddEntry(g2,setStr[1].c_str(),"lep");
  //
  legend->Draw();
  //ratio plot
  p2->cd();
  TGraphErrors *pull = (TGraphErrors*)calc_signif(g1,g2);
  //pull->SetTitle(" ; -t (GeV^{2} ); #left|#sigma_{pull}#right|");
  pull->SetTitle(" ; -t (GeV^{2} ); %Diff");
  pull->GetYaxis()->CenterTitle(true);
  pull->GetXaxis()->SetRangeUser(0,2.);
  pull->SetMinimum(0);
  // if(pull->GetMaximum()<2 )
  //   pull->GetYaxis()->SetRangeUser(-0.1,2.5);
  // else
  //   pull->GetYaxis()->SetRangeUser(-0.1,pull->GetMaximum()+0.5);
  pull->GetYaxis()->SetNdivisions(505);
  pull->GetXaxis()->SetLabelSize(0.15);
  pull->GetYaxis()->SetLabelSize(0.15);
  pull->GetXaxis()->SetTitleSize(0.18);
  pull->GetYaxis()->SetTitleSize(0.18);
  pull->GetXaxis()->SetTitleOffset(0.9);
  pull->GetYaxis()->SetTitleOffset(0.3);
  
  pull->Draw("ap");
  
  c->SaveAs((saveDir+"pctdiff"+delim+"_"+setStr[0]+"_"+setStr[1]+".pdf").c_str());
}

TGraphErrors* calc_signif(TGraphErrors *nominal, TGraphErrors *variation)
{
    TGraphErrors *graph = new TGraphErrors();
    double Delta, sigma;
    for(int i = 0; i < nominal->GetN(); i++) {
      Delta = abs(nominal->GetPointY(i) - variation->GetPointY(i));
      sigma = nominal->GetPointY(i) + variation->GetPointY(i);
      //sigma = nominal->GetPointY(i);
      //sigma = TMath::Sqrt( abs( nominal->GetErrorY(i)*nominal->GetErrorY(i) + variation->GetErrorY(i)*variation->GetErrorY(i) ) );
      cout << Delta << "    " << sigma << "     " << Delta / sigma << endl;
      if(sigma != 0.0)    graph->SetPointY( i, Delta/sigma*100 );
      else                graph->SetPointY( i, 0.0 );
      graph->SetPointX( i, nominal->GetPointX(i) );
      graph->SetPointError( i, variation->GetErrorX(i), 0 );
      graph->SetMaximum(3);
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
  gStyle->SetPadLeftMargin  (0.15);
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

  gStyle->SetTitleOffset(1.,"X");
  gStyle->SetTitleOffset(0.9,"Y");

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
