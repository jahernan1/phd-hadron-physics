#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
// Total cross section of the three GlueX-I run periods and their weighted average against the CLAS g12
// points, with an exponential fit to CLAS + the three periods (dissertation chapter 6,
// totxsec_clas_gluex_Phase1.pdf). Reads the direct total cross section,
// <xsecDir>/data/<label>/totxsec_flatTree_<stem>.txt and <xsecDir>/weighted_data/<label>/totxsec_weighted_output.txt.
// The dissertation figure is label hybrid_combo of the systematics variant pool (the JohnsonMCShape study
// tables; see docs/KNOWN_ISSUES.md). Writes totxsec_clas_gluex_Phase1.pdf and
// totxsec_clas_gluex_Phase1.root (graphs clas, weighted, sp17, sp18, fa18 and the fit) to plotDir.
// `gxana run xsection --steps figures` runs it (xsection.figures). Returns 1 if an input is missing.

#include "TF1.h"
#include "TH1.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include "RooPlot.h"

void SetStyle();

int PlotTotXsecWithClas(string xsecDir = "", string label = "hybrid_combo", string plotDir = "")
{
  if (xsecDir.empty()) xsecDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/variants");
  if (plotDir.empty()) plotDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/figures");
  const string dataPath = xsecDir + "/data/" + label + "/";
  const string wdataPath = xsecDir + "/weighted_data/" + label + "/";
  const vector<string> inputs = {
      gxana::EnvPath("GXANA_ROOT", "analyses/kpkpxim/xsection/external_data/Clas_data.csv"),
      wdataPath + "totxsec_weighted_output.txt",
      dataPath + "totxsec_flatTree_kpkpxim__M23_2017-01_ana56.txt",
      dataPath + "totxsec_flatTree_kpkpxim__B4_M23_2018-01_ana03.txt",
      dataPath + "totxsec_flatTree_kpkpxim__B4_M23_2018-08_ana02.txt"};
  for (const auto& in : inputs)
    if (gSystem->AccessPathName(in.c_str())) {
      cerr << "PlotTotXsecWithClas: missing " << in << endl;
      return 1;
    }
  gSystem->mkdir(plotDir.c_str(), true);
  SetStyle();
  TCanvas *c = new TCanvas("c", "c");

  TGraphErrors *g1 = new TGraphErrors(inputs[0].c_str(), "%lg %lg %lg");//, option=" \t,;");
  g1->SetTitle("CLAS Data");
  g1->SetMarkerStyle(22);
  g1->SetMarkerSize(1.2);
  g1->SetDrawOption("AP");
  g1->SetMarkerColor(kOrange+7);
  g1->SetLineWidth(2);
  g1->SetFillStyle(0);  

  TGraphErrors *g2 = new TGraphErrors(  inputs[1].c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  g2->SetTitle("Weighted");
  g2->SetMarkerStyle(24);
  g2->SetMarkerSize(1.2);
  g2->SetDrawOption("AP");
  g2->SetMarkerColor(kBlack);
  g2->SetLineColor(kBlack);
  g2->SetLineWidth(2);
  g2->SetFillColorAlpha(kBlack,0.3);
  //g2->SetFillStyle(0);  

  TGraphErrors *g3 = new TGraphErrors(  inputs[2].c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  g3->SetTitle("Spring 2017");
  g3->SetMarkerStyle(20);
  g3->SetMarkerSize(1.2);
  g3->SetDrawOption("AP");
  g3->SetMarkerColor(kRed);
  g3->SetLineColor(kRed);
  g3->SetLineWidth(2);
  g3->SetFillStyle(0);

  TGraphErrors *g4 = new TGraphErrors( inputs[3].c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
  g4->SetTitle("Spring 2018");
  g4->SetMarkerStyle(20);
  g4->SetMarkerSize(1.2);
  g4->SetDrawOption("AP");
  g4->SetMarkerColor(kBlue);
  g4->SetLineColor(kBlue);
  g4->SetLineWidth(2);
  g4->SetFillStyle(0);  
  
  TGraphErrors *g5 = new TGraphErrors( inputs[4].c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
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

  c->SaveAs((plotDir + "/totxsec_clas_gluex_Phase1.pdf").c_str());
  TFile out((plotDir + "/totxsec_clas_gluex_Phase1.root").c_str(), "RECREATE");
  g1->Write("clas");
  g2->Write("weighted");
  g3->Write("sp17");
  g4->Write("sp18");
  g5->Write("fa18");
  fit->Write("fit");
  out.Close();
  return 0;
}


void SetStyle()
{
    gxana::StyleParams p = gxana::ThesisStyle();
    p.padLeftMargin = 0.16;
    p.titleAlign.set = false;
    p.titleX.set = false;
    p.titleSizeT.set = false;
    p.titleOffsetY = 1.0;
    gxana::ApplyStyle(p);
}
