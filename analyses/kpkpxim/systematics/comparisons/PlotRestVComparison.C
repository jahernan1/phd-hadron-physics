#include "gxana/common/Paths.h"
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
std::vector<TGraphErrors*> GetAllTGraphErrors(const char* fileName);
void PlotWeightedXSec(vector<vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, string saveName , string leg_entry="GlueX#lower[-0.15]{-}#kern[0.2]{I}");

struct GraphStats {
    double avg;       // Average of all y-values
    double std_dev;   // Standard deviation
    double pct_diff;
    double signif;
};
GraphStats GetAvgStdDev(const TGraphErrors& graph1, const TGraphErrors& graph2);


// main function 
int PlotRestVComparison()
{
    //initiate variables
    string fileDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/weighted_data/");
  
    // Load root tree of choice
    vector<TGraphErrors*> graphs_rest3 = GetAllTGraphErrors("WeightedDiffXSecTGraphs_s17_rest3.root" );
    vector<TGraphErrors*> graphs_hybrid = GetAllTGraphErrors("WeightedDiffXSecTGraphs_oneEBin.root"  );

    vector<TGraphErrors*> graphs_s17_rest3 = GetAllTGraphErrors("DiffXSecTGraphs_kpkpxim__M23_2017-01_ana45_s17_rest3.root" );
    vector<TGraphErrors*> graphs_s17_hybrid = GetAllTGraphErrors("DiffXSecTGraphs_kpkpxim__M23_2017-01_ana56_oneEBin.root"  );

    // Call function to plot
    style_format();
    PlotWeightedXSec({graphs_hybrid, graphs_rest3}, 2.5, 9, "weighted_diffxsec_s17_rest3", "GlueX#lower[-0.15]{-}#kern[0.2]{I}");
    PlotWeightedXSec({graphs_s17_hybrid, graphs_s17_rest3}, 2.5, 9, "diffxsec_s17_s17_rest3", "Spring 2017");
    // make_plot(vect_histo_2017, "mm2_data_mc_2017", "Spring 2017", delim);
    // make_plot(vect_histo_201801, "mm2_data_mc_201801", "Spring 2018",delim);
    // make_plot(vect_histo_201808, "mm2_data_mc_201808", "Fall 2018",delim);
    return 0;
}

GraphStats GetAvgStdDev(const TGraphErrors& graph1, const TGraphErrors& graph2) {
    int nPoints1 = graph1.GetN();
    int nPoints2 = graph2.GetN();

    if (nPoints1 != nPoints2) {
        throw std::invalid_argument("Graphs must have the same number of points.");
    }

    int nPoints = nPoints1; // Either graph1 or graph2 size, since they're equal
    double totalPercentDifference = 0;
    std::vector<double> yValues;
    double Delta, sigma, tot_sig;
    
    // Gather y-values from both graphs
    for (int i = 0; i < nPoints; ++i) {
        double y1, y2;
        y1 = graph1.GetY()[i];
        y2 = graph2.GetY()[i];

        // Calculate the percent difference for this pair of points
        if (y1 != 0) {
            double percentDifference = ((y2 - y1) / y1) * 100.0;
            totalPercentDifference += percentDifference;
        } else {
            throw std::runtime_error("Encountered a zero value in graph1; cannot compute percent difference.");
        }
        
        // Add each graph's y-value to the list
        yValues.push_back(y1);
        yValues.push_back(y2);

        // Get significance of difference
        Delta = abs(graph1.GetPointY(i) - graph2.GetPointY(i));
        sigma = TMath::Sqrt( abs( graph1.GetErrorY(i)*graph1.GetErrorY(i) + graph2.GetErrorY(i)*graph2.GetErrorY(i) ) );
        tot_sig += Delta/sigma;
    }

    // Compute the mean of all y-values
    double sum = 0;
    for (double y : yValues) {
        sum += y;
    }
    double mean = sum / yValues.size();

    // Compute the standard deviation
    double variance = 0;
    for (double y : yValues) {
        variance += (y - mean) * (y - mean);
    }
    variance /= (yValues.size() - 1); // Sample standard deviation
    double stdDev = std::sqrt(variance);

    return {mean, stdDev, totalPercentDifference/nPoints, tot_sig/nPoints};
}

// make plot to be called by main function
void PlotWeightedXSec(vector<vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, string saveName, string leg_entry="GlueX-I")
{
    //Initiate variables
    char savePath[200];
    double numBins = arrGraphs[0].size();
    cout << "Num Bins in plotting: " << numBins << endl;
    double small = 1e-5;
    double ymin = 0.15;
    TLatex xTitle, yTitle;
    // Plot the differential cross sections together
    TCanvas *tC = new TCanvas("canvas", "diff_xsection_cuts",700,600);

    //loop
    for (int j = 0; j < numBins; j++)
        {
            // Set Pad style for plots
            //gPad->SetLogy();
            gPad->SetGrid();
        
            // Set histograms and draw on Pad
            arrGraphs[0][j]->SetMaximum(ymax);
            arrGraphs[0][j]->SetMinimum(ymin);
            arrGraphs[0][j]->SetMarkerColor(kBlack);
            arrGraphs[0][j]->SetLineColor(kBlack);
            arrGraphs[0][j]->SetMarkerStyle(20);
            //arrGraphs[0][j]->SetMarkerSize(0.8);
            //arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.05,xmax);

            arrGraphs[1][j]->SetMaximum(ymax);
            arrGraphs[1][j]->SetMinimum(ymin);
            arrGraphs[1][j]->SetMarkerColor(kRed+1);
            arrGraphs[1][j]->SetLineColor(kRed+1);
            arrGraphs[1][j]->SetMarkerStyle(24);
            //arrGraphs[1][j]->SetMarkerSize(0.8);
            //arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      
            // Plot all data sets
            arrGraphs[0][j]->Draw("ap");
            arrGraphs[1][j]->Draw("p same");
      
            //Get stat difference between cut analysis
            GraphStats stats = GetAvgStdDev(*arrGraphs[0][j], *arrGraphs[1][j]);
            std::cout << "Average: " << stats.avg << "\nStandard Deviation: " << stats.std_dev << "\nPercent Difference: " << stats.pct_diff << "\nSiginificance: " << stats.signif << "\n" <<  std::endl;
      
            // Create TLatex to display the result on canvas
            TLatex* latex = new TLatex();
            latex->SetNDC();
            latex->SetTextColor(kRed+1);      
            //latex->SetTextFont(42);
            latex->SetTextSize(0.055);
            latex->SetTextAlign(32);
            latex->DrawLatex(0.95, 0.65, Form("#bf{Avg. Pct. Diff.: %.2f%%}", stats.pct_diff));
        }

    arrGraphs[0][0]->GetYaxis()->SetTitle("d#sigma/dt (nb/GeV^{2} )");
    arrGraphs[0][0]->GetXaxis()->SetTitle( "-t (GeV^{2} )");
    tC->Modified();
    
    gStyle->SetPadBottomMargin(0.18);
    gStyle->SetPadTopMargin   (0.08);
    gStyle->SetPadLeftMargin  (0.17);
    gStyle->SetPadRightMargin (0.04);
    gStyle->SetLabelOffset(0.03,"Y");
  
    // Add Legend
    auto legend = new TLegend(0.6,0.7,0.9,0.9);
    legend->SetBorderSize(0);
    legend->SetTextSize(0.055);
    legend->SetTextFont(132);
    legend->SetFillColor(0);
    legend->SetFillStyle(0);
  
    legend->SetHeader(leg_entry.c_str(),"C"); // option "C" allows to center the header
    legend->AddEntry(arrGraphs[0][0],"Sp17 REST4","lep");
    legend->AddEntry(arrGraphs[1][0],"Sp17 REST3","lep");
    legend->Draw();

    sprintf(savePath, gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/plots/%s.pdf").c_str(), saveName.c_str());
    tC->SaveAs(savePath);
}

// Get All histograms
std::vector<TGraphErrors*> GetAllTGraphErrors(const char* fileName) {
    // Create a vector to store the TGraphErrors objects
    std::vector<TGraphErrors*> graphs;

    // Open the ROOT file
    TFile* file = TFile::Open(fileName);
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return graphs; // Return empty vector if file can't be opened
    }

    // Loop over all keys in the file
    TIter nextKey(file->GetListOfKeys());
    TKey* key;
    
    while ((key = (TKey*)nextKey())) {
        // Get the class name of the object
        const char* className = key->GetClassName();

        // Check if the object is of type TGraphErrors
        if (strcmp(className, "TGraphErrors") == 0) {
            // Retrieve the object as a TGraphErrors
            TGraphErrors* graph = (TGraphErrors*)key->ReadObj();

            // Add the TGraphErrors to the vector
            graphs.push_back(graph);
        }
    }

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
  
    gStyle->SetTitleSize(0.06,"T");
    gStyle->SetTitleSize(0.08,"X");
    gStyle->SetTitleSize(0.08,"Y");
  
    gStyle->SetTitleOffset(1.0,"X");
    gStyle->SetTitleOffset(1.0,"Y");

    gStyle->SetTextSize(0.08);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(0.99);
    gStyle->SetTextFont(132);
    gStyle->SetOptStat(0);

    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextFont(132);
    latex->SetTextSize(0.08);
    latex->SetTextAlign(32);
    gROOT->ForceStyle();
}
