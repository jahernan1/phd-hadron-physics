// kpkpxim xsection clas and gluex data
#include "TF1.h"
#include "TH1.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include "RooPlot.h"

void SetStyle();
TGraphErrors* GetTGraph(string xsecFile, vector<Double_t> arr_bins);
vector<vector<string>> SplitString(string s, string delim);
void PlotComponent(vector<vector<TGraphErrors*>> arrGraphs, double xmax, double ymin, double ymax, string saveName, string yTitleStr);
void GetComponentFile( string inFilePath, ofstream outFile, int col=1);//n is the column to get

int MakeXSecComponents(string dataFiles)//these are full paths
{
  string compDir ="/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/components/gen_amp_V2_2D_ac/";
  vector<vector<double>> arrEnBins{{6.40,7.40},{7.40,7.86},{7.86,8.19},{8.19,8.45},{8.45,8.68},{8.68,9.26},{9.26,10.18},{10.18,11.4}};//{{6.40,7.20},{7.20,7.72},{7.72,7.94},{7.94,8.24},{8.24,8.43},{8.43,8.62},{8.62,8.79},{8.79,9.50},{9.50,10.21},{10.21,11.40}};
  TGraphErrors* tmpg;
  vector<vector<TGraphErrors*>> vvGraphs{{},{},{}};
  //Make component subarrays
  vector<vector<TGraphErrors*>> vvDataYields{{},{},{}};
  vector<vector<TGraphErrors*>> vvMCYields{{},{},{}};
  vector<vector<TGraphErrors*>> vvThrownYields{{},{},{}};
  vector<vector<TGraphErrors*>> vvAcceptance{{},{},{}};
  vector<vector<TGraphErrors*>> vvFlux{{},{},{}};
  //Split string of files
  vector<vector<string>> arrAllRootFilePath = SplitString(dataFiles, ",");

  //strings needed to save unique root tree
  string rootFileName = arrAllRootFilePath[0][0];
  string comboTypeName = rootFileName.substr(0,rootFileName.find_last_of("/"));
  comboTypeName = comboTypeName.substr(comboTypeName.find_last_of("/")+1);
  rootFileName = rootFileName.substr(0,rootFileName.find_last_of("."));//remove extension
  rootFileName = rootFileName.substr(rootFileName.find_last_of("/")+1);//only keep file name
  rootFileName = rootFileName.substr(0,rootFileName.find("Emin"));//strip the binning info
  rootFileName = rootFileName.substr(0,rootFileName.find("__")) +
                 rootFileName.substr(rootFileName.find("_nominal")) + 
                 comboTypeName;
  
  //Get name of plot
  string base_str = arrAllRootFilePath[0][0].substr(arrAllRootFilePath[0][0].find_last_of("/")+1);
  string component_str = base_str.substr(0, base_str.find("_subset"));
  cout << "File component: " << component_str << endl;
  
  //TFile to store xsections
  cout << "Output Weighted Diff Xsec Name:" << "\n" << rootFileName+".root" << endl;
  TFile *tfout = TFile::Open( (rootFileName+".root").c_str(),"RECREATE");
  
  for (Int_t j=0;j<arrAllRootFilePath.size();j++)//j iterated data period 0-2
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
      tfout->cd(dataSetName.c_str());
        
      for(Int_t i=0;i<arrAllRootFilePath[j].size();i++)
        {
          if(arrAllRootFilePath[j].empty())
            cout << j << " elements are empty (0-2017-01, 1-2018-01, 2-2018-08)..." << endl;
          else
            {
              string enBinStr = to_string(arrEnBins[i][0]);
              enBinStr = enBinStr.substr(0,enBinStr.find_last_of(".")+3);
              cout << arrAllRootFilePath[j][i] << endl;
              tmpg = (TGraphErrors*)GetTGraph(arrAllRootFilePath[j][i], arrEnBins[i]); 
              tmpg->Write((component_str+"_bin_"+enBinStr).c_str(),TObject::kOverwrite);
              vvGraphs[j].push_back(tmpg);
            }
        }
    }

   //Fill the correct TGraph vector
  if(component_str=="data_yield")
    PlotComponent(vvGraphs, 2.4, 0.1, 150, component_str, "Data Yield");
  else if(component_str=="mc_yield")
    PlotComponent(vvGraphs, 2.4, 0.3, 4.3e3, component_str, "Simulated Yield");
  else if(component_str=="thrown_yield")
    PlotComponent(vvGraphs, 2.4, 0.0, 9e5, component_str, "Generated Yield");
  else if(component_str=="accept")
    PlotComponent(vvGraphs, 2.4, 0.00, 0.024, component_str, "Acceptance");
  else if(component_str=="flux")
    PlotComponent(vvGraphs, 2.4, 0.1, 4e13, component_str, "Photon Flux");
  else
    cout << "error... no component plots filled..." << endl;           
  
  return 0;
}

TGraphErrors* GetTGraph(string xsecFile, vector<Double_t> arr_bins)
{
  SetStyle();
  string dataPath = "/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/data/gen_amp_V2_2D_ac/";
  new TCanvas;
  //gPad->SetLogy();
  
  TGraphErrors *g1 = new TGraphErrors( (xsecFile).c_str(), "%lg %lg %lg %lg");
  g1->GetYaxis()->SetDecimals(1);
  TGaxis::SetMaxDigits(3);

  string str_enMin = to_string(arr_bins[0]);
  string str_enMax = to_string(arr_bins[1]);
  str_enMin = str_enMin.substr(0,str_enMin.find_last_not_of('0')+1);
  str_enMax = str_enMax.substr(0,str_enMax.find_last_not_of('0')+1);
  string title = "#bf{E_{#gamma} (GeV): ("+str_enMin+", "+str_enMax+")}; -t (GeV)^{2}; "; 
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
  return g1;
}

vector<vector<string>> SplitString(string s, string delim){
  //
  size_t pos = 0;
  string token;
  vector<vector<string>> result{{},{},{}};
  
  cout << "" << endl;
  cout << "Root files to process..." << endl;
  while( (pos = s.find(delim)) != string::npos){
    token = s.substr(0, pos);
    if(token.find("2017-01") < token.length())
      result[0].push_back(token);
    else if(token.find("2018-01") < token.length())
      result[1].push_back(token);
    else if(token.find("2018-08") < token.length())
      result[2].push_back(token);
    
    //cout << token << endl;
    s.erase(0, pos + delim.length());
  }

  return result;
}


void PlotComponent(vector<vector<TGraphErrors*>> arrGraphs, double xmax, double ymin, double ymax, string saveName, string yTitleStr)
{
  //Initiate variables
  char savePath[200];
  double numBins = arrGraphs[0].size();
  cout << "Num Bins in plotting: " << numBins << endl;
  double small = 1e-5;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "canvas", 1000, 700);
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
      //gPad->SetLogy();
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
      arrGraphs[0][j]->SetMaximum(ymax);
      arrGraphs[0][j]->SetMinimum(ymin);
      arrGraphs[0][j]->GetYaxis()->SetNdivisions(505);
      arrGraphs[0][j]->GetXaxis()->SetNdivisions(505);
      arrGraphs[0][j]->SetMarkerColor(kBlack);
      arrGraphs[0][j]->SetLineColor(kAzure);
      arrGraphs[0][j]->SetMarkerStyle(20);
      arrGraphs[0][j]->SetMarkerSize(0.5);

      arrGraphs[1][j]->SetMaximum(ymax);
      arrGraphs[1][j]->SetMinimum(ymin);
      arrGraphs[1][j]->SetMarkerColor(kBlack);
      arrGraphs[1][j]->SetLineColor(kSpring-6);
      arrGraphs[1][j]->SetMarkerStyle(20);
      arrGraphs[1][j]->SetMarkerSize(0.5);
      
      arrGraphs[2][j]->SetMaximum(ymax);
      arrGraphs[2][j]->SetMinimum(ymin);
      arrGraphs[2][j]->SetMarkerColor(kBlack);
      arrGraphs[2][j]->SetLineColor(kRed);
      arrGraphs[2][j]->SetMarkerStyle(20);
      arrGraphs[2][j]->SetMarkerSize(0.5);

      // Plot all data sets
      arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.0,xmax);
      arrGraphs[0][j]->Draw("ap");
      arrGraphs[1][j]->Draw("p same");
      arrGraphs[2][j]->Draw("p same");     
    }

  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
      yax = new TGaxis(0.1, 0.95-0.85/3*(i+1), 0.1, 0.95-0.85/3*i, ymin, ymax, 505, "");
      yax->SetLabelSize(0.035);
      //yax->SetLabelOffset(-0.01);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 1; i < 3; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.0, xmax, 505, "");
      xax->SetLabelSize(0.035);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  for ( int i = 0; i < 1; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.0, xmax, 205, "");
      xax->SetLabelSize(0.035);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  //Draw plot axis labels
  xTitle.SetTextFont(132);
  xTitle.SetTextSize(0.06);
  // xTitle.SetTextAlign(12);
  xTitle.DrawLatex(0.52, 0.01, "-t (GeV^{2} )");
  
  yTitle.SetTextFont(132);
  yTitle.SetTextSize(0.06);
  yTitle.SetTextAngle(90);
  yTitle.DrawLatex(0.045, 0.58, yTitleStr.c_str());
  
  gStyle->SetPadBottomMargin(0.5);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.12);
  gStyle->SetPadRightMargin (0.02);
  gStyle->SetLabelOffset(0.03,"Y");
 
  // Add Legend
  auto legend = new TLegend(0.7,0.16,0.95,0.3);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  legend->AddEntry(arrGraphs[0][1],"Spring 2017","lep");
  legend->AddEntry(arrGraphs[1][1],"Spring 2018","lep");
  legend->AddEntry(arrGraphs[2][1],"Fall 2018","lep");
  legend->Draw();
  
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_201808set.pdf");
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_allSets+weighted.pdf");
  sprintf(savePath, "/d/grid17/hjesse/AnalysisNote/xsection/plots/%s.pdf", saveName.c_str());
  tC->SaveAs( (saveName+".pdf").c_str());
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_2017_201808.pdf");
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_weighted.pdf");
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
  gStyle->SetPadLeftMargin  (0.18);
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
  gStyle->SetTitleOffset(0.95,"Y");

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
