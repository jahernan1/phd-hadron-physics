#include "PlotFunctions.h"
using namespace std;

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
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);

    gStyle->SetTitleSize(0.1,"T");
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

// Custom comparison for numeric sorting
bool NumericCompare(const std::string &a, const std::string &b) {
    // Find first digit in each string
    auto findFirstNumber = [](const std::string &str) -> int {
        for (size_t i = 0; i < str.size(); ++i) {
            if (isdigit(str[i])) {
                return std::stoi(str.substr(i));
            }
        }
        return 0; // No number found, default to 0
    };

    std::string suba = a.substr(a.find("emin"));
    std::string subb = b.substr(b.find("emin"));
    
    int numA = findFirstNumber(suba);
    int numB = findFirstNumber(subb);

    return numA < numB;
}

// Function to create TGraphErrors from .txt files in a directory
std::vector<TGraphErrors*> CreateTGraphErrorsFromTxt(string dir , const std::string &pattern) {
    // Get list of all .txt files in the directory
    void *dirp = gSystem->OpenDirectory(dir.c_str());
    if (!dirp) {
        std::cerr << "Error: Cannot open directory " << dir << std::endl;
        return {};
    }

    const char *entry;
    std::vector<std::string> txtFiles;
    while ((entry = gSystem->GetDirEntry(dirp))) {
        std::string filename = entry;
        // Check for ".txt" extension and optionally filter by pattern
        if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".txt" &&
            fnmatch(pattern.c_str(), filename.c_str(), 0) == 0)
            {
                txtFiles.push_back(std::string(dir) + "/" + filename);
                std::cout << "Entry: " << txtFiles.back() << std::endl;
            }
    }

    gSystem->FreeDirectory(dirp);
    if (txtFiles.empty()) {
        std::cerr << "No .txt files found in directory " << dir 
                  << " with pattern: " << pattern << std::endl;
        return {};
    }

    // sort the vector of file names to endure proper plotting
    std::sort(txtFiles.begin(), txtFiles.end(), NumericCompare);
    
    // Create a ROOT file to save TGraphErrors
    std::string type = dir.substr(dir.find("/")+1);
    type = type.substr(0,type.find("/"));
    
    TFile *rootFile;
    if(dir.find("weighted_data") != std::string::npos)
        rootFile = new TFile(("WeightedDiffXSecTGraphs_"+type+".root").c_str(), "RECREATE");
    else
        {
            string delim = txtFiles[0].substr(txtFiles[0].find_last_of("/"));
            delim = delim.substr(delim.find("kpkpxim"));
            delim = delim.substr(0, delim.find("_emin"));
            rootFile = new TFile(("DiffXSecTGraphs_"+delim+"_"+type+".root").c_str(), "RECREATE");
        }        

    if (!rootFile->IsOpen()) {
        std::cerr << "Error: Cannot create output file WeightedXSecTGraphs.root" << std::endl;
        return {};
    }

    // Vector to store the TGraphErrors
    std::vector<TGraphErrors*> graphs;
    std::string enMin, enMax;
    
    // Process each .txt file
    for (const auto& file : txtFiles) {
        // Extract base name without path and extension
        std::string fileName = file.substr(file.find_last_of("/") + 1);
        fileName = fileName.substr(0, fileName.find_last_of("."));
        // Get enbins for plot title
        if(fileName.find("emin") != std::string::npos){
            enMin = fileName.substr(fileName.find("emin")+5);
            enMin = enMin.substr(0,enMin.find_first_of("_"));
            enMax = fileName.substr(fileName.find("emax")+5);
            enMax = enMax.substr(0,enMax.find_first_of("_"));
        }
        // Create a TGraphErrors from the file
        TGraphErrors *graph = new TGraphErrors(file.c_str());
        graph->SetTitle( ("#bf{E_{#gamma} (GeV): (" + enMin + ", " + enMax + ")}").c_str());
        if (!graph || graph->GetN() == 0) {
            std::cerr << "Warning: Failed to create graph from file: " << file << std::endl;
            delete graph;
            continue;
        }

        // Set graph name and write it to the ROOT file
        graph->SetName(("Graph_" + fileName).c_str());
        graph->Write();
        graphs.push_back(graph);

        std::cout << "Processed file: " << fileName << std::endl;
    }

    rootFile->Close();
    delete rootFile;

    std::cout << "All graphs saved to WeightedXSecTGraphs.root" << std::endl;

    return graphs;
}

void plotDiffXSec(std::vector<std::vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, std::string saveName)
{
  //Initiate variables
  char savePath[250];
  double numBins = arrGraphs[0].size();
  std::cout << "Num Bins in plotting: " << numBins << std::endl;
  double small = 1e-5;
  double ymin = 0.15;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection", 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);
  
  C->Draw();
  C->Divide(3, 3, small, small);

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
      arrGraphs[2][j]->SetMarkerColor(kRed+1);
      arrGraphs[2][j]->SetLineColor(kRed+1);
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
  //legend->AddEntry(diff_xsec_weighted[1],"GlueX#lower[-0.15]{-}#kern[0.2]{I}","lep");
  //legend->AddEntry(diff_xsec_weighted[1],"Spring 2018 GlueX-I","lep");
  legend->Draw();

  sprintf(savePath, "/d/grid17/hjesse/AnalysisNote/xsection/plots/%s.pdf", saveName.c_str());
  tC->SaveAs(savePath);
}

void plotWeightedXSec(std::vector<TGraphErrors*> arrGraphs, double xmax, double ymax, std::string saveName)
{
  //Initiate variables
  char savePath[250];
  double numBins = arrGraphs.size();
  std::cout << "Num Bins in plotting: " << numBins << std::endl;
  double small = 1e-5;
  double ymin = 0.15;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection_weighted", 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);
  
  C->Draw();
  C->Divide(3,3, small, small);
  
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
  legend->AddEntry(arrGraphs[0],"GlueX#lower[-0.15]{-}#kern[0.2]{I}","lep");
  legend->Draw();

  sprintf(savePath, "/d/grid17/hjesse/AnalysisNote/xsection/plots/%s.pdf", saveName.c_str());
  tC->SaveAs(savePath);
}

void plotOneWeightedXSec(std::vector<TGraphErrors*> arrGraphs, double xmax, double ymax, std::string saveName)
{
  //Initiate variables
  char savePath[250];
  double numBins = arrGraphs.size();
  std::cout << "Num Bins in plotting: " << numBins << std::endl;
  double small = 1e-5;
  double ymin = 0.15;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection_weighted");
   
  for (int j = 0; j < numBins; j++)
    {
        // Set Pad style for plots
      gPad->SetLogy();
      gPad->SetGrid();
      // gPad->SetFillStyle(0);
      // gPad->SetFrameFillStyle(0);
      // gPad->SetTopMargin(small);
      // gPad->SetBottomMargin(small);
      // gPad->SetRightMargin(small);
      // gPad->SetLeftMargin(small);    
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
  legend->AddEntry(arrGraphs[0],"GlueX#lower[-0.15]{-}#kern[0.2]{I}","lep");
  legend->Draw();

  sprintf(savePath, "/d/grid17/hjesse/AnalysisNote/xsection/plots/%s.pdf", saveName.c_str());
  tC->SaveAs(savePath);
}

void plotFinalWeightedXSec(std::vector<std::vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, std::string saveName)
{
    //Initiate variables
    char savePath[300];
    double numBins = arrGraphs[0].size();
    std::cout << "Num Bins in plotting: " << numBins << std::endl;
    double small = 1e-5;
    double ymin = 0.15;
    TGaxis  *yax, *xax;
    TLatex xTitle, yTitle;
    // Plot the differential cross sections together
    TCanvas *tC = new TCanvas("canvas", "diff_xsection", 1000, 700);
    TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);
  
    C->Draw();
    C->Divide(3, 3, small, small);

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
            arrGraphs[0][j]->SetMaximum(ymax);
            arrGraphs[0][j]->SetMinimum(ymin);
            arrGraphs[0][j]->SetMarkerColor(kAzure);
            arrGraphs[0][j]->SetLineColor(kAzure);
            arrGraphs[0][j]->SetMarkerStyle(20);
            arrGraphs[0][j]->SetMarkerSize(0.8);

            arrGraphs[1][j]->SetMaximum(ymax);
            arrGraphs[1][j]->SetMinimum(ymin);
            arrGraphs[1][j]->SetFillColorAlpha(kAzure,0.5);
            
            // Plot all data sets
            arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.05,xmax);
            arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.05,xmax);
            //
            arrGraphs[0][j]->Draw("ap");
            arrGraphs[1][j]->Draw("2|| same");
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
    legend->AddEntry(arrGraphs[0][1],"GlueX#lower[-0.15]{-}#kern[0.2]{I}","lep");
    legend->AddEntry(arrGraphs[1][1],"#splitline{Systematic}{Uncertainty}","f");

    //legend->AddEntry(diff_xsec_weighted[1],"GlueX#lower[-0.15]{-}#kern[0.2]{I}","lep");
    //legend->AddEntry(diff_xsec_weighted[1],"Spring 2018 GlueX-I","lep");
    legend->Draw();

    sprintf(savePath, "/d/grid17/hjesse/AnalysisNote/xsection/plots/%s.pdf", saveName.c_str());
    tC->SaveAs(savePath);
}



// void PlotTotXsecWithClas(std::vector<TGraphErrors*> arrGraphs, double xmax, double ymax, std::string saveName)
// {
//   string dataPath = "/d/grid17/hjesse/AnalysisNote/xsection/data/";
//   string wdataPath = "/d/grid17/hjesse/AnalysisNote/xsection/weighted_data/";
//   TCanvas *c = new TCanvas("c", "c");

//   TGraphErrors *g1 = new TGraphErrors("/d/grid17/hjesse/Clas_data.csv", "%lg %lg %lg");//, option=" \t,;");
//   g1->SetTitle("CLAS Data");
//   g1->SetMarkerStyle(22);
//   g1->SetMarkerSize(1.2);
//   g1->SetDrawOption("AP");
//   g1->SetMarkerColor(kOrange+7);
//   g1->SetLineWidth(2);
//   g1->SetFillStyle(0);  

//   TGraphErrors *g2 = new TGraphErrors(  (wdataPath+"totxsec_weighted_output.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
//   g2->SetTitle("Weighted");
//   g2->SetMarkerStyle(24);
//   g2->SetMarkerSize(1.2);
//   g2->SetDrawOption("AP");
//   g2->SetMarkerColor(kBlack);
//   g2->SetLineColor(kBlack);
//   g2->SetLineWidth(2);
//   g2->SetFillColorAlpha(kBlack,0.3);
//   //g2->SetFillStyle(0);  

//   TGraphErrors *g3 = new TGraphErrors(  (dataPath+"totxsec_flatTree_kpkpxim__M23_2017-01_ana56.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
//   g3->SetTitle("Spring 2017");
//   g3->SetMarkerStyle(20);
//   g3->SetMarkerSize(1.2);
//   g3->SetDrawOption("AP");
//   g3->SetMarkerColor(kRed);
//   g3->SetLineColor(kRed);
//   g3->SetLineWidth(2);
//   g3->SetFillStyle(0);

//   TGraphErrors *g4 = new TGraphErrors( (dataPath+"totxsec_flatTree_kpkpxim__B4_M23_2018-01_ana03.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
//   g4->SetTitle("Spring 2018");
//   g4->SetMarkerStyle(20);
//   g4->SetMarkerSize(1.2);
//   g4->SetDrawOption("AP");
//   g4->SetMarkerColor(kBlue);
//   g4->SetLineColor(kBlue);
//   g4->SetLineWidth(2);
//   g4->SetFillStyle(0);  
  
//   TGraphErrors *g5 = new TGraphErrors( (dataPath+"totxsec_flatTree_kpkpxim__B4_M23_2018-08_ana02.txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
//   g5->SetTitle("Spring 2018");
//   g5->SetMarkerStyle(20);
//   g5->SetMarkerSize(1.2);
//   g5->SetDrawOption("AP");
//   g5->SetMarkerColor(kSpring+9);
//   g5->SetLineColor(kSpring+9);
//   g5->SetLineWidth(2);
//   g5->SetFillStyle(0);  
  
//   // TGraphErrors *g5 = new TGraphErrors( (dataPath+"intxsec_avgw"+delim+".txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
//   // g5->SetTitle("GlueX-I Data");
//   // g5->SetMarkerStyle(20);
//   // g5->SetMarkerSize(1.2);
//   // g5->SetDrawOption("e4");
//   // g5->SetMarkerColor(kSpring);
//   // g5->SetLineColor(kSpring);
//   // //g5->SetFillColorAlpha(kAzure,0.4);
//   // g5->SetLineWidth(2);
//   // //g5->SetFillStyle(0);  
  
//   TMultiGraph *mg = new TMultiGraph();
//   mg->SetTitle(" ; E_{#gamma} (GeV);#sigma(#gammap #rightarrow K^{+}K^{+}#Xi^{-}) (nb)");
//   mg->Add(g1);
//   //mg->Add(g2);
//   mg->Add(g3);
//   mg->Add(g4);
//   mg->Add(g5);
  
//   mg->Draw("ap");
//   gPad->SetGrid(0,1);
//   mg->GetXaxis()->SetLimits(2,12);
//   mg->GetYaxis()->SetRangeUser(0,18);
//   TF1 *fit = new TF1("fit","expo",4.3,11.4);
//   fit->SetParameters(3.3,-0.2);
//   fit->SetLineWidth(4);
//   fit->SetLineColor(kBlack);
//   fit->SetLineStyle(7);
//   fit->SetNpx(1000);
//   mg->Fit("fit", "R");
//   Double_t  chisqndf = fit->GetChisquare() / fit->GetNDF();
//   TLatex latex;
//   latex.SetTextFont(132);
//   latex.SetTextSize(0.04);
//   latex.SetTextAlign(12);
//   latex.DrawLatex(10,9.7,("(#chi^{2}_{#nu} = " + to_string(chisqndf).substr(0,4)+")").c_str());
//   //
//   g2->Draw("e3 same");
  
//   auto legend = new TLegend(0.62,0.55,0.948,0.91);
//   legend->SetBorderSize(0);
//   legend->SetTextSize(0.05);
//   legend->SetTextFont(132);
//   legend->SetFillColor(0);
//   legend->SetFillStyle(0);
//   //legend->SetHeader(" ","C"); // option "C" allows to center the header
//   legend->AddEntry(g1,"CLAS Data","lep");
//   legend->AddEntry(g2,"GlueX#lower[-0.15]{-}#kern[0.2]{I}","f");
//   //legend->AddEntry(g5,"GlueX#lower[-0.15]{-}#kern[0.2]{I}","f");
//   legend->AddEntry(g3,"Spring 2017","lep");
//   legend->AddEntry(g4,"Spring 2018","lep");
//   legend->AddEntry(g5,"Fall 2018","lep");
//   legend->AddEntry(fit,"Expo","l");
  

//   gStyle->SetOptFit(0);
//   legend->Draw();

//   c->SaveAs("totxsec_clas_gluex_Phase1.pdf");
// }
