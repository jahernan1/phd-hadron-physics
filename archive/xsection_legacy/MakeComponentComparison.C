/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
#include <TFile.h>
#include <TGraphErrors.h>
#include <TKey.h>
#include <iostream>
#include <vector>

//instantiate methods used in main
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string delim, double chisqcut=8);
void style_format();
std::vector<TGraphErrors*> GetAllTGraphErrors(const char* fileName, const char* directoryName = "");
std::vector<TGraphErrors*> GetAllTGraphErrorsFromDirectory(TDirectory* dir);
void PlotWeightedXSec(vector<vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, string saveName, string yAxisStr);
void GetComponentComparison(string component_str, double xmax, double ymax, string dataSet="Spring_2017");
  
// main function 
int MakeComponentComparison()
{
  vector<string> subdir = {"Spring_2017","Spring_2018","Fall_2018"};

  for(auto &str : subdir){
    GetComponentComparison("accept", 2.5, 0.025, str);
    GetComponentComparison("data_yield", 10, 150, str);
    GetComponentComparison("mc_yield", 10, 4500, str);
    GetComponentComparison("thrown_yield", 10, 80000, str);
  }
  
  return 0;
}

void GetComponentComparison(string component_str, double xmax=0, double ymax=0, string dataSet="Spring_2017")
{
  //initiate variables
  string thisDir = "/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/";
  
  // Load root tree of choice
  vector<TGraphErrors*> graphs_ac = GetAllTGraphErrors( (component_str+"_subset_kpkpxim_nominal_momCut_acc_combo.root").c_str(), dataSet.c_str());
  vector<TGraphErrors*> graphs_bc = GetAllTGraphErrors( (component_str+"_subset_kpkpxim_nominal_momCut_best_combo.root").c_str() , dataSet.c_str());
  vector<TGraphErrors*> graphs_hc = GetAllTGraphErrors( (component_str+"_subset_kpkpxim_nominal_momCut_hybrid_combo.root").c_str(), dataSet.c_str() );

  graphs_ac[0]->Draw("ap");
  
  // Call function to plot
  PlotWeightedXSec({graphs_ac,graphs_bc,graphs_hc}, xmax, ymax, (component_str+"_combos_"+dataSet+".pdf"), component_str);
}

// make plot to be called by main function
void PlotWeightedXSec(vector<vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, string saveName, string yAxisStr)
{
  //Initiate variables
  char savePath[200];
  double numBins = arrGraphs[0].size();
  cout << "Num Bins in plotting: " << numBins << endl;
  double small = 1e-5;
  double ymin = 0.001;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", saveName.substr(0,saveName.find_last_of(".")).c_str(), 1000, 700);
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
      arrGraphs[0][j]->SetMarkerColor(kAzure);
      arrGraphs[0][j]->SetLineColor(kAzure);
      arrGraphs[0][j]->SetMarkerStyle(24);
      arrGraphs[0][j]->SetMarkerSize(0.8);
      arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      arrGraphs[0][j]->GetXaxis()->SetMaxDigits(3);
      //
      arrGraphs[1][j]->SetMaximum(ymax);
      arrGraphs[1][j]->SetMinimum(ymin);
      arrGraphs[1][j]->SetMarkerColor(kSpring-6);
      arrGraphs[1][j]->SetLineColor(kSpring-6);
      arrGraphs[1][j]->SetMarkerStyle(24);
      arrGraphs[1][j]->SetMarkerSize(0.8);
      arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      //
      arrGraphs[2][j]->SetMaximum(ymax);
      arrGraphs[2][j]->SetMinimum(ymin);
      arrGraphs[2][j]->SetMarkerColor(kRed+1);
      arrGraphs[2][j]->SetLineColor(kRed+1);
      arrGraphs[2][j]->SetMarkerStyle(24);
      arrGraphs[2][j]->SetMarkerSize(0.8);
      arrGraphs[2][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      // Plot all data sets
      arrGraphs[0][j]->DrawClone("ap");
      arrGraphs[1][j]->DrawClone("p same");
      arrGraphs[2][j]->DrawClone("p same");
      //
    }
    
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
      yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax );
      yax->SetLabelSize(0.04);
      yax->SetMaxDigits(3);
      //yax->SetLabelOffset(-0.01);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 1; i < 3; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.01, xmax);
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  for ( int i = 0; i < 1; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.01, xmax);
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
  yTitle.DrawLatex(0.05, 0.55, yAxisStr.c_str());
  
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
  legend->AddEntry(arrGraphs[0][0],"RF Sub.","lep");
  legend->AddEntry(arrGraphs[1][1],"Best #chi^{2}","lep");
  legend->AddEntry(arrGraphs[2][0],"Hybrid #chi^{2}","lep");
  legend->Draw();

  sprintf(savePath, "/d/grid17/hjesse/AnalysisNote/xsection/plots/%s.pdf", saveName.c_str());
  tC->SaveAs(saveName.c_str());
}

std::vector<TGraphErrors*> GetAllTGraphErrorsFromDirectory(TDirectory* dir) {
    // Create a vector to store the TGraphErrors objects
    std::vector<TGraphErrors*> graphs;

    // Check if the directory is valid
    if (!dir) {
        std::cerr << "Error: Invalid directory" << std::endl;
        return graphs; // Return empty vector if directory is invalid
    }

    // Loop over all keys in the directory
    TIter nextKey(dir->GetListOfKeys());
    TKey* key;
    
    while ((key = (TKey*)nextKey())) {
        // Get the class name of the object
        const char* className = key->GetClassName();

        // Check if the object is a subdirectory
        if (strcmp(className, "TDirectoryFile") == 0) {
            // If the object is a subdirectory, recurse into it
            TDirectory* subdir = (TDirectory*)key->ReadObj();
            std::vector<TGraphErrors*> subdirGraphs = GetAllTGraphErrorsFromDirectory(subdir);
            // Append the graphs from the subdirectory to the main vector
            graphs.insert(graphs.end(), subdirGraphs.begin(), subdirGraphs.end());
        }
        // Check if the object is of type TGraphErrors
        else if (strcmp(className, "TGraphErrors") == 0) {
            // Retrieve the object as a TGraphErrors
            TGraphErrors* graph = (TGraphErrors*)key->ReadObj();

            // Add the TGraphErrors to the vector
            graphs.push_back(graph);
        }
    }

    // Return the vector containing all TGraphErrors objects
    return graphs;
}

std::vector<TGraphErrors*> GetAllTGraphErrors(const char* fileName, const char* directoryName = "") {
    // Open the ROOT file
    TFile* file = TFile::Open(fileName);
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return {}; // Return empty vector if file can't be opened
    }

    // Get the directory (if no directoryName is provided, use the root directory of the file)
    TDirectory* dir = directoryName[0] ? file->GetDirectory(directoryName) : file;
    if (!dir) {
        std::cerr << "Error: Cannot open directory " << directoryName << std::endl;
        file->Close();
        delete file;
        return {};
    }

    // Get all TGraphErrors from the directory
    std::vector<TGraphErrors*> graphs = GetAllTGraphErrorsFromDirectory(dir);

    // Close the file
    file->Close();
    delete file;

    // Return the vector containing all TGraphErrors objects
    return graphs;
}

// input specific style formatting of user choice
void style_format()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(600);
  gStyle->SetCanvasDefW(700);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.05);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);

  gStyle->SetFrameFillStyle ( 0);
  gStyle->SetFrameFillColor ( 0);
  gStyle->SetFrameLineColor ( 1);
  gStyle->SetFrameLineStyle ( 0);
  gStyle->SetFrameLineWidth ( 1);
  gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);

  gStyle->SetNdivisions(505);

  //gStyle->SetLineWidth(1);
  //gStyle->SetHistLineWidth(1);
  //gStyle->SetFrameLineWidth(2);
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.07,"X");
  gStyle->SetLabelSize(0.07,"Y");

  gStyle->SetLabelOffset(0.008,"X");
  gStyle->SetLabelOffset(0.008,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132,"T");
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");
  
  gStyle->SetTitleSize(0.07,"T");
  gStyle->SetTitleSize(0.08,"X");
  gStyle->SetTitleSize(0.08,"Y");
  
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(1.0,"Y");
  
  gStyle->SetTextSize(0.08);
  gStyle->SetTextFont(132);
  gStyle->SetOptStat(0);

  TLatex* latex = new TLatex();
  latex->SetNDC();
  latex->SetTextFont(132);
  latex->SetTextSize(0.08);
  latex->SetTextAlign(32);
  gROOT->ForceStyle();
}
