
// kpkpxim xsection 
#include "TF1.h"
#include "TH1.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include "RooPlot.h"

void SetStyle();
TGraphErrors* MakeBinnedDiffXSec(string xsecFile, vector<Double_t> arr_bins);
//TMultiGraph* MakeBinnedDiffXSec(string xsecFile)
vector<vector<string>> SplitString(string s, string delim);
void plotDiffXSec(vector<vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, string saveName);
void plotWeightedXSec(vector<TGraphErrors*> arrGraphs, double xmax, double ymax, string saveName);
TGraphErrors* calc_weightedavg(TGraphErrors *graph1, TGraphErrors *graph2, TGraphErrors *graph3);
TGraphErrors* calc_totalxsec(vector<TGraphErrors*> arrGraph, vector<vector<double>> arrEn, string saveName);

int MakeXSec(string dataFiles)//these are full paths
{
  TGraphErrors* tmpg;
  TObjArray hList(0);
  vector<vector<TGraphErrors*>> arrGraphs{{},{},{}};
  vector<TGraphErrors*> arrWeightedGraph;
  vector<vector<string>> arrAllRootFilePath = SplitString(dataFiles, ",");
  vector<vector<double>> arrEnBins{{6.40,7.40},{7.40,7.86},{7.86,8.19},{8.19,8.45},{8.45,8.68},{8.68,9.26},{9.26,10.18},{10.18,11.40}};
  //strings needed to save unique root tree
  string rootFileName = arrAllRootFilePath[0][0];
  string comboTypeName = rootFileName.substr(0,rootFileName.find("diffxsec")-1);
  comboTypeName = comboTypeName.substr(comboTypeName.find_last_of("/")+1);
  rootFileName = rootFileName.substr(0,rootFileName.find_last_of("."));//remove extension
  rootFileName = rootFileName.substr(rootFileName.find_last_of("/")+1);//only keep file name
  rootFileName = rootFileName.substr(0,rootFileName.find("Emin"));//strip the binning info
  rootFileName = rootFileName.substr(0,rootFileName.find("__")) +
                 rootFileName.substr(rootFileName.find("_nominal")) + 
                 comboTypeName;
  
  //TFile to store xsections
  cout << "Output Weighted Diff Xsec Name:" << "\n" << rootFileName+".root" << endl;
  TFile *tfout = TFile::Open( (rootFileName+".root").c_str(),"RECREATE");
  
  for (Int_t j=0;j<arrAllRootFilePath.size();j++)
    {
      //Set up TFile
      string dataSetName="";
      switch (j) {
      case 0:
        dataSetName = "Spring_2017/";
        break;
      case 1:
        dataSetName = "Spring_2018/";
        break;
      case 2:
        dataSetName = "Fall_2018/";
        break;
      }
      tfout->cd();
      gDirectory->mkdir(dataSetName.c_str());
      
      for(Int_t i=0;i<arrAllRootFilePath[j].size();i++)
        {
          if(arrAllRootFilePath[j].empty())
            cout << j << " elements are empty (0-2017-01, 1-2018-01, 2-2018-08)..." << endl;
          else
            {
              string enBinStr = to_string(arrEnBins[i][0]);
              enBinStr = enBinStr.substr(0,enBinStr.find_last_of(".")+3);
              cout << arrAllRootFilePath[j][i] << endl;
              tmpg = (TGraphErrors*)MakeBinnedDiffXSec(arrAllRootFilePath[j][i], arrEnBins[i]); 
              tfout->cd(dataSetName.c_str());
              tmpg->Write(("diffxsec_bin_"+enBinStr).c_str(),TObject::kOverwrite);
              arrGraphs[j].push_back(tmpg);
            }
        }
    }
  //
  plotDiffXSec(arrGraphs, 2.5, 20, "");
  
  //
  tfout->cd();
  for(Int_t jj=0; jj<arrAllRootFilePath[0].size();jj++)
    {
      tmpg = (TGraphErrors*)calc_weightedavg(arrGraphs[0][jj],arrGraphs[1][jj],arrGraphs[2][jj]);
      string name="diffxsec_weighted_bin"+to_string(jj);
      //tmpg->SetName( (name).c_str() );
      tfout->WriteTObject(tmpg, name.c_str());
      arrWeightedGraph.push_back(tmpg);
    }
  tfout->Close();
    
  plotWeightedXSec(arrWeightedGraph, 2.5, 20, "");

  calc_totalxsec(arrWeightedGraph,arrEnBins,"intxsec_weighted");  
  calc_totalxsec(arrGraphs[0],arrEnBins,"intxsec_2017.pdf");
  calc_totalxsec(arrGraphs[1],arrEnBins,"intxsec_201801.pdf");
  calc_totalxsec(arrGraphs[2],arrEnBins,"intxsec_201808.pdf");  
  
  return 0;
}

TGraphErrors* MakeBinnedDiffXSec(string xsecFile, vector<Double_t> arr_bins)
{
  SetStyle();
  //string dataPath = "/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/data/";
  TCanvas *c = new TCanvas("c", "c");
  c->SetLogy();
  
  TGraphErrors *g1 = new TGraphErrors( (xsecFile).c_str(), "%lg %lg %lg %lg");
  
  string str_enMin = to_string(arr_bins[0]);
  string str_enMax = to_string(arr_bins[1]);
  str_enMin = str_enMin.substr(0,str_enMin.find_last_not_of('0')+1);
  str_enMax = str_enMax.substr(0,str_enMax.find_last_not_of('0')+1);
  string title = "#bf{E_{#gamma} (GeV): ("+str_enMin+", "+str_enMax+")}; -t (GeV)^{2};d#sigma/dt (nb/GeV^{2} )"; 
  g1->SetTitle(title.c_str());
  g1->SetMarkerStyle(22);
  g1->SetMarkerSize(1.2);
  g1->SetDrawOption("AP");
  g1->SetMarkerColor(kOrange+7);
  g1->SetLineWidth(2);
  g1->SetFillStyle(0);
  gStyle->SetTitleSize(0.10,"T");
  gStyle->SetTitleFont(132,"T");
  gROOT->ForceStyle();
  
  g1->Draw("ap");
  // c->SaveAs("test.pdf");
  return g1;
}

vector<vector<string>> SplitString(string s, string delim){
  //
  size_t pos = 0;
  string token;
  vector<vector<string>> result{{},{},{}};

  cout << "" << endl;
  cout << "Root files to process..." << endl;
  while( (pos = s.find(delim)) != string::npos)
    {
      token = s.substr(0, pos);
      if(token.find("2017-01") < token.length())
        result[0].push_back(token);
      else if(token.find("2018-01") < token.length())
        result[1].push_back(token);
      else if(token.find("2018-08") < token.length())
        result[2].push_back(token);
    
      cout << token << endl;
      s.erase(0, pos + delim.length());
    }

  return result;
}

void plotDiffXSec(vector<vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, string saveName)
{
  //Initiate variables
  char savePath[200];
  double numBins = arrGraphs[0].size();
  cout << "Num Bins in plotting: " << numBins << endl;
  double small = 1e-5;
  double ymin = 0.15;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection", 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);
  
  C->Draw();
  C->Divide(3, 3, small, small);

  //rpad->Draw();
  //rpad->Divide(1,4,small,small);
  
  //setStyle();
  for (int j = 0; j < numBins; j++)
    {
      C->cd(j+1);
      
      // Set Pad style for plots
      gPad->SetLogy();
      gPad->SetGrid();
      gPad->SetFillStyle(0);
      gPad->SetFrameFillStyle(0);
      gPad->SetTopMargin(small);
      gPad->SetBottomMargin(small);
      gPad->SetRightMargin(small);
      gPad->SetLeftMargin(small);    
      gStyle->SetOptStat(0);

      TLatex* latex = new TLatex();
      latex->SetNDC();
      //latex->SetTextFont(42);
      latex->SetTextSize(0.04);
      latex->SetTextAlign(32);

      // Set histograms and draw on Pad
      arrGraphs[0][j]->SetMaximum(ymax);
      arrGraphs[0][j]->SetMinimum(ymin);
      arrGraphs[0][j]->SetMarkerColor(kAzure);
      arrGraphs[0][j]->SetLineColor(kAzure);
      arrGraphs[0][j]->SetMarkerStyle(20);
      arrGraphs[0][j]->SetMarkerSize(0.8);

      arrGraphs[1][j]->SetMaximum(ymax);
      arrGraphs[1][j]->SetMinimum(ymin);
      arrGraphs[1][j]->SetMarkerColor(kSpring-6);
      arrGraphs[1][j]->SetLineColor(kSpring-6);
      arrGraphs[1][j]->SetMarkerStyle(20);
      arrGraphs[1][j]->SetMarkerSize(0.8);
      
      arrGraphs[2][j]->SetMaximum(ymax);
      arrGraphs[2][j]->SetMinimum(ymin);
      arrGraphs[2][j]->SetMarkerColor(kRed);
      arrGraphs[2][j]->SetLineColor(kRed);
      arrGraphs[2][j]->SetMarkerStyle(20);
      arrGraphs[2][j]->SetMarkerSize(0.8);

      // Plot all data sets
      arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      arrGraphs[2][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      arrGraphs[0][j]->Draw("ap");
      arrGraphs[1][j]->Draw("p same");
      arrGraphs[2][j]->Draw("p same");     
    }
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
      yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax, 510, "G");
      yax->SetLabelSize(0.04);
      //yax->SetLabelOffset(-0.01);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 1; i < 3; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.05, xmax, 205, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  for ( int i = 0; i < 1; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.05, xmax, 205, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  // Draw plot axis labels
  xTitle.SetTextFont(132);
  xTitle.SetTextSize(0.06);
  xTitle.DrawLatex(0.5, 0.01, "-t (GeV^{2} )");
  
  yTitle.SetTextFont(132);
  yTitle.SetTextSize(0.06);
  yTitle.SetTextAngle(90);
  yTitle.DrawLatex(0.05, 0.55, "d#sigma/dt (nb/GeV^{2} )");
  
  gStyle->SetPadBottomMargin(0.2);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.04);
  gStyle->SetLabelOffset(0.03,"Y");
 
  // Add Legend
  auto legend = new TLegend(0.7,0.15,0.95,0.3);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  legend->AddEntry(arrGraphs[0][1],"Spring 2017","lep");
  legend->AddEntry(arrGraphs[1][1],"Spring 2018","lep");
  legend->AddEntry(arrGraphs[2][1],"Fall 2018","lep");
  //legend->AddEntry(diff_xsec_weighted[1],"GlueX-I","lep");
  //legend->AddEntry(diff_xsec_weighted[1],"Spring 2018 GlueX-I","lep");
  legend->Draw();

  sprintf(savePath, "/d/grid17/hjesse/AnalysisNote/xsection/plots/%s.pdf", saveName.c_str());
  tC->SaveAs("canvas.pdf");
}

void plotWeightedXSec(vector<TGraphErrors*> arrGraphs, double xmax, double ymax, string saveName)
{
  //Initiate variables
  char savePath[200];
  double numBins = arrGraphs.size();
  cout << "Num Bins in plotting: " << numBins << endl;
  double small = 1e-5;
  double ymin = 0.15;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection_weighted", 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);
  
  C->Draw();
  C->Divide(3, 3, small, small);

  //rpad->Draw();
  //rpad->Divide(1,4,small,small);
  
  //setStyle();
  for (int j = 0; j < numBins; j++)
    {
      C->cd(j+1);
      
      // Set Pad style for plots
      gPad->SetLogy();
      //gPad->SetGrid();
      gPad->SetFillStyle(0);
      gPad->SetFrameFillStyle(0);
      gPad->SetTopMargin(small);
      gPad->SetBottomMargin(small);
      gPad->SetRightMargin(small);
      gPad->SetLeftMargin(small);    
      gStyle->SetOptStat(0);

      TLatex* latex = new TLatex();
      latex->SetNDC();
      //latex->SetTextFont(42);
      latex->SetTextSize(0.04);
      latex->SetTextAlign(32);

      // Set histograms and draw on Pad
      arrGraphs[j]->SetMaximum(ymax);
      arrGraphs[j]->SetMinimum(ymin);
      arrGraphs[j]->SetMarkerColor(kAzure);
      arrGraphs[j]->SetLineColor(kAzure);
      arrGraphs[j]->SetMarkerStyle(20);
      arrGraphs[j]->SetMarkerSize(0.8);

      // Plot all data sets
      arrGraphs[j]->GetXaxis()->SetRangeUser(0.05,xmax);
      arrGraphs[j]->DrawClone("ap");
    }
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
      yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax, 510, "G");
      yax->SetLabelSize(0.04);
      //yax->SetLabelOffset(-0.01);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 1; i < 3; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.05, xmax, 205, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  for ( int i = 0; i < 1; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.05, xmax, 205, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  // Draw plot axis labels
  xTitle.SetTextFont(132);
  xTitle.SetTextSize(0.06);
  xTitle.DrawLatex(0.5, 0.01, "-t (GeV^{2} )");
  
  yTitle.SetTextFont(132);
  yTitle.SetTextSize(0.06);
  yTitle.SetTextAngle(90);
  yTitle.DrawLatex(0.05, 0.55, "d#sigma/dt (nb/GeV^{2} )");
  
  gStyle->SetPadBottomMargin(0.2);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.04);
  gStyle->SetLabelOffset(0.03,"Y");
 
  // Add Legend
  auto legend = new TLegend(0.7,0.15,0.95,0.3);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  legend->AddEntry(arrGraphs[0],"GlueX-I","lep");
  legend->Draw();

  sprintf(savePath, "/d/grid17/hjesse/AnalysisNote/xsection/plots/%s.pdf", saveName.c_str());
  tC->SaveAs("canvas_weighted.pdf");
  tC->SaveAs("canvas_weighted.root");
  tC->SaveAs("canvas_weighted.C");
}

TGraphErrors* calc_weightedavg(TGraphErrors *graph1, TGraphErrors *graph2, TGraphErrors *graph3)
{
    TGraphErrors *graph = (TGraphErrors*)graph1->Clone();
    
    double xsec, xsec_error;
    for(int i = 0; i < graph1->GetN(); i++) {
      xsec = ( graph1->GetPointY(i)*pow( 1/graph1->GetErrorY(i), 2 ) +  graph2->GetPointY(i)*pow( 1/graph2->GetErrorY(i), 2 ) +  graph3->GetPointY(i)*pow( 1/graph3->GetErrorY(i), 2 ) ) / ( pow( 1/graph1->GetErrorY(i), 2 ) + pow( 1/graph2->GetErrorY(i), 2 ) + pow( 1/graph3->GetErrorY(i), 2 ) );
      xsec_error = 1 / TMath::Sqrt( pow( 1/graph1->GetErrorY(i), 2 ) + pow( 1/graph2->GetErrorY(i), 2 ) + pow( 1/graph3->GetErrorY(i), 2 ) );
      cout << xsec << " errx:    " << graph1->GetErrorX(i) <<  "  erry:   " << xsec_error << "\t"<< xsec / xsec_error << endl;
      graph->SetPointY( i, xsec );
      graph->SetPointX( i, graph1->GetPointX(i) );
      graph->SetPointError( i, graph1->GetErrorX(i), xsec_error );
      // cout << i << "   " << Delta / xsec_error << endl;
      // cout << "    "<< xsec_error << endl;
    }
    return graph;
}

TGraphErrors* calc_totalxsec(vector<TGraphErrors*> arrGraph, vector<vector<double>> arrEn, string saveName)
{
    TGraphErrors *graph = new TGraphErrors();
    double xsec, xsec_err;
    for(int i = 0; i < arrGraph.size(); i++) {
      xsec = xsec_err = 0;
      double enBinCentral = (arrEn[i][0]+arrEn[i][1])/2;
      double enBinError = (arrEn[i][1]-arrEn[i][0])/2;
      for(int j=0; j < arrGraph[i]->GetN() ; j++){
        xsec +=  arrGraph[i]->GetPointY(j)*2*arrGraph[i]->GetErrorX(j);
        xsec_err += pow(arrGraph[i]->GetErrorY(j), 2)*pow(2*arrGraph[i]->GetErrorX(j),2);
      }
      xsec_err = sqrt(xsec_err);
      cout << xsec << " errx:    " << enBinError <<  "  erry:   " << xsec_err << endl;
      graph->SetPointY( i, xsec );
      graph->SetPointX( i,  enBinCentral);
      graph->SetPointError( i, enBinError, xsec_err );
      // cout << i << "   " << Delta / xsec_err << endl;
      // cout << "    "<< xsec_err << endl;
    }

    TCanvas *c1 = new TCanvas();
    graph->Draw("ap");
    c1->SaveAs(saveName.c_str());
    
    return graph;
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
  gStyle->SetTitleOffset(0.85,"Y");

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
