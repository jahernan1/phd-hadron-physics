#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
// Define a data structure to store TGraphErrors associated with each prefix
using GraphMap = std::map<std::string, std::vector<TGraphErrors*>>;
using GraphCollection = std::vector<std::vector<TGraphErrors*>>;

// input specific style formatting of user choice
void style_format()
{
    gxana::StyleParams p = gxana::ComparisonStyle();
    p.padBottomMargin = 0.5;
    p.titleX = 1.0;
    p.titleSizeT = 0.1;
    gxana::ApplyStyle(p);
}

void plotComponent(GraphCollection arrGraphs, double xmax, double ymax, std::string Title, double title_pos, std::string saveName)
{
  //Initiate variables
  char savePath[250];
  double numBins = arrGraphs[0].size();
  std::cout << "Num Bins in plotting: " << numBins << std::endl;
  double small = 1e-5;
  double ymin = 0.;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection", 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.12, 0.95, 0.95);
  C->Draw();
  C->Divide(3, 3, small, small);

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
      arrGraphs[0][j]->SetMarkerStyle(20);
      arrGraphs[0][j]->SetMarkerSize(0.8);
      arrGraphs[0][j]->GetXaxis()->SetMaxDigits(3);
      
      arrGraphs[1][j]->SetMaximum(ymax);
      arrGraphs[1][j]->SetMinimum(ymin);
      arrGraphs[1][j]->SetMarkerColor(kSpring-6);
      arrGraphs[1][j]->SetLineColor(kSpring-6);
      arrGraphs[1][j]->SetMarkerStyle(20);
      arrGraphs[1][j]->SetMarkerSize(0.8);
      arrGraphs[1][j]->GetXaxis()->SetMaxDigits(3);
            
      arrGraphs[2][j]->SetMaximum(ymax);
      arrGraphs[2][j]->SetMinimum(ymin);
      arrGraphs[2][j]->SetMarkerColor(kRed+1);
      arrGraphs[2][j]->SetLineColor(kRed+1);
      arrGraphs[2][j]->SetMarkerStyle(20);
      arrGraphs[2][j]->SetMarkerSize(0.8);
      arrGraphs[2][j]->GetXaxis()->SetMaxDigits(3);
      
      // Plot all data sets
      arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.01,xmax);
      arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.01,xmax);
      arrGraphs[2][j]->GetXaxis()->SetRangeUser(0.01,xmax);
      arrGraphs[0][j]->Draw("ap");
      arrGraphs[1][j]->Draw("p same");
      arrGraphs[2][j]->Draw("p same");     
    }
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
        yax = new TGaxis(0.1, 0.95-.83/3*(i+1), 0.1, 0.95-.83/3*i, ymin, ymax, 505);
        yax->SetLabelSize(0.03);
        yax->SetMaxDigits(3);
        //yax->SetLabelOffset(-0.01);
        yax->Draw("same");
    }
  
  //tC->cd();
  for ( int i = 1; i < 3; i++)
      {
          xax = new TGaxis(0.95-.85/3*(i+1), 0.12, 0.95-.85/3*(i), 0.12, 0.01, xmax);
          xax->SetLabelSize(0.03);
          //xax->SetLabelOffset(-0.01);
          xax->Draw("same");
      }
  
  for ( int i = 0; i < 1; i++)
      {
          xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.83/3*(2), 0.95-.85/3*(i), 0.95-.83/3*(2), 0.01, xmax);
          xax->SetLabelSize(0.03);
          //xax->SetLabelOffset(-0.01);
          xax->Draw("same");
      }
  
  // Draw plot axis labels
  xTitle.SetTextFont(132);
  xTitle.SetTextSize(0.06);
  xTitle.DrawLatex(0.52, 0.015, "-t (GeV^{2} )");
  
  yTitle.SetTextFont(132);
  yTitle.SetTextSize(0.06);
  yTitle.SetTextAngle(90);
  yTitle.DrawLatex(0.038, title_pos, Title.c_str());
 
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

  sprintf(savePath, gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/plots/%s.pdf").c_str(), saveName.c_str());
  tC->SaveAs(savePath);
}

GraphMap ProcessFilesToTFile(const char* directory, const char* outputFileName) {
    // Map to store TDirectories by prefix
    std::map<std::string, TDirectory*> directories;
    GraphMap graphCollection;
    
    // List of prefixes for classification
    std::vector<std::string> prefixes = {"data_yield", "qval_yield", "mc_yield", "thrown_yield", "accept"};

    // Open the output TFile
    TFile* outputFile = new TFile(outputFileName, "RECREATE");
    if (!outputFile->IsOpen()) {
        std::cerr << "Error creating output file: " << outputFileName << std::endl;
        return graphCollection;
    }

    // Create TDirectories for each prefix
    for (const auto& prefix : prefixes) {
        directories[prefix] = outputFile->mkdir(prefix.c_str());
    }

    // Use TSystem to get the list of files in the directory
    void* dir = gSystem->OpenDirectory(directory);
    if (!dir) {
        std::cerr << "Error opening directory: " << directory << std::endl;
        return graphCollection;
    }

    const char* entry;
    while ((entry = gSystem->GetDirEntry(dir))) {
        TString fileName(entry);
        std::string strfileName(entry);
        strfileName = strfileName.substr(0,strfileName.find_last_of("."));
            
        if (!fileName.EndsWith(".txt")) continue;
        if (!fileName.Contains("emin")) continue;
        
        // Determine prefix by matching the filename
        std::string matchedPrefix = "";
        for (const auto& prefix : prefixes) {
            if (fileName.BeginsWith(prefix.c_str())) {
                matchedPrefix = prefix;
                break;
            }
        }

        // Skip files without a matching prefix
        if (matchedPrefix.empty()) continue;

        // Full file path
        TString filePath = TString::Format("%s%s", directory, fileName.Data());

        // Create a TGraphErrors directly from the file
        TGraphErrors* graph = new TGraphErrors(filePath.Data(), "%lg %lg %lg %lg");
        if (!graph || graph->GetN() == 0) {
            std::cerr << "Error creating graph from file: " << filePath << std::endl;
            delete graph;
            continue;
        }
        // Get enbins for plot title
        std::string enMin,enMax;
        if(strfileName.find("emin") != std::string::npos){
            enMin = strfileName.substr(strfileName.find("emin")+5);
            enMin = enMin.substr(0,enMin.find_first_of("_"));
            enMax = strfileName.substr(strfileName.find("emax")+5);
            enMax = enMax.substr(0,enMax.find_first_of("_"));
        }
        
        // Create a TGraphErrors from the file
        graph->SetTitle( ("#bf{E_{#gamma} (GeV): (" + enMin + ", " + enMax + ")}").c_str());
        // Set the name of the graph to the file name (without extension)
        graph->SetName(fileName.ReplaceAll(".txt", "").Data());
        
        // Write the graph to the corresponding TDirectory
        directories[matchedPrefix]->cd();
        graph->Write(fileName.ReplaceAll(".txt", "").Data(),TObject::kOverwrite);

        // Store the graph in the map
        graphCollection[matchedPrefix].push_back(graph);
    }
    
    // Clean up
    gSystem->FreeDirectory(dir);
    outputFile->Close();
    delete outputFile;

    std::cout << "Processing complete. Output written to "
              << outputFileName << std::endl;

    return graphCollection;
}

int PlotXSecComponents()
{
    std::string dir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/components/");
    std::vector<std::string> dataType = {"hybrid_combo","johnson", "mcPdf", "mcPdf_cheby1"};

    for(const auto& type : dataType){
        GraphMap graphMap_sp17 = ProcessFilesToTFile( (dir+"sp17/"+type+"/").c_str(), (dir+"sp17_xsec_components"+type+".root").c_str());
        GraphMap graphMap_sp18 = ProcessFilesToTFile( (dir+"sp18/"+type+"/").c_str(), (dir+"sp18_xsec_components"+type+".root").c_str());
        GraphMap graphMap_fa18 = ProcessFilesToTFile( (dir+"fa18/"+type+"/").c_str(), (dir+"fa18_xsec_components"+type+".root").c_str());
        vector<GraphMap> graphMaps = {graphMap_sp17,graphMap_sp18,graphMap_fa18};

        style_format();
        plotComponent({graphMap_sp17["data_yield"],graphMap_sp18["data_yield"],graphMap_fa18["data_yield"]}, 2.5, 180, "Data Yield", 0.7,"data_yield_runs_"+type);
        plotComponent({graphMap_sp17["mc_yield"],graphMap_sp18["mc_yield"],graphMap_fa18["mc_yield"]}, 2.5, 2200, "MC Yield", 0.72, "mc_yield_runs_"+type);
        plotComponent({graphMap_sp17["qval_yield"],graphMap_sp18["qval_yield"],graphMap_fa18["qval_yield"]}, 2.5, 180, "Q-Factor Yield", 0.6, "qvalue_yield_runs_"+type);
        plotComponent({graphMap_sp17["thrown_yield"],graphMap_sp18["thrown_yield"],graphMap_fa18["thrown_yield"]}, 2.5, 4.5e5, "Thrown Yield", 0.63, "thrown_yield_runs_"+type);
        plotComponent({graphMap_sp17["accept"],graphMap_sp18["accept"],graphMap_fa18["accept"]}, 2.5, 0.022, "Acceptance", 0.7, "accept_runs_"+type);
    }

    //success
    return 0;
}
