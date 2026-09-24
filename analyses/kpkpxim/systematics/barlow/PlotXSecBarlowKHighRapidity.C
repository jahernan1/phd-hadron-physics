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
void make_plot(string delim, vector<string> rootFile, vector<string> setStr);
TGraphErrors* calc_signif(TGraphErrors *nominal, TGraphErrors *variation);
void plotMultipleGraphs(const std::vector<std::string>& fileNames);
void plotTotXSecAndBarlow(const std::vector<std::string>& fileNames, std::pair<string,string> cutPair);
void plotDiffXSecAndBarlow(const std::vector<std::string>& fileNames, std::pair<string,string> cutPair);
TGraphErrors* calculateStdDevGraph(const std::vector<TGraphErrors*>& graphs);
TGraphErrors* calc_barlow(TGraphErrors *nominal, TGraphErrors *variation);

//main  
int PlotXSecBarlowKHighRapidity()
{
    SetStyle();
    string nominalDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/");
    std::vector<std::pair<string, string>> enRanges{{"6.40","7.40"},{"7.40","7.86"},{"7.86","8.19"},{"8.19","8.45"},{"8.45","8.68"},{"8.68","9.26"},{"9.26","10.18"},{"10.18","11.40"}};
    
    std::vector<std::string> cutNames = {"kphigh_prap_1.6","kphigh_prap_1.8",
                                         "kphigh_prap_2.1","kphigh_prap_2.2"};

    string cutName = cutNames[0].substr(0, cutNames[0].find_last_of("_"));
    std::pair<string,string> cutPair = {cutName, "y(K^{+}_{#it{#lower[-0.3]{fast}}})  > "};//literal cut, latex formated
    std::vector<std::string> arrGraphs = 
        {nominalDir+"weighted_data/totxsec_weighted_output.txt",
         "weighted_data/weighted_totxsec_vary_"+cutNames[0]+".txt",
         "weighted_data/weighted_totxsec_vary_"+cutNames[1]+".txt",
         "weighted_data/weighted_totxsec_vary_"+cutNames[2]+".txt",
         "weighted_data/weighted_totxsec_vary_"+cutNames[3]+".txt"};
    
    //plotMultipleGraphs(arrGraphs);
    plotTotXSecAndBarlow(arrGraphs, cutPair);
    
    for(const auto& enbins : enRanges){
        std::vector<std::string> files =
            {nominalDir+"weighted_data/weighted_diffxsec_emin_"+enbins.first+"_emax_"+enbins.second+".txt",
             "weighted_data/weighted_diffxsec_vary_"+cutNames[0]+"_emin_"+enbins.first+"_emax_"+enbins.second+".txt",
             "weighted_data/weighted_diffxsec_vary_"+cutNames[1]+"_emin_"+enbins.first+"_emax_"+enbins.second+".txt",
             "weighted_data/weighted_diffxsec_vary_"+cutNames[2]+"_emin_"+enbins.first+"_emax_"+enbins.second+".txt",
             "weighted_data/weighted_diffxsec_vary_"+cutNames[3]+"_emin_"+enbins.first+"_emax_"+enbins.second+".txt"
            };
            
        plotDiffXSecAndBarlow(files, cutPair);
    }
    
    return 0;
}

TGraphErrors* calc_barlow(TGraphErrors *nominal, TGraphErrors *variation)
{
    TGraphErrors *graph = new TGraphErrors();
    double Delta, sigma;
    for(int i = 0; i < nominal->GetN(); i++) {
        Delta = nominal->GetPointY(i) - variation->GetPointY(i);
        sigma = TMath::Sqrt( abs( nominal->GetErrorY(i)*nominal->GetErrorY(i)
                                  - variation->GetErrorY(i)*variation->GetErrorY(i) ) );
        // cout << "Nominal Point:  " <<  nominal->GetPointY(i) << " +/- " << nominal->GetErrorY(i) << "\n"
        //      << "Variation Point:  " <<  variation->GetPointY(i) << " +/- " << variation->GetErrorY(i)
        //      << " @ " << nominal->GetPointX(i) << endl;
        cout << "Results:  " << Delta << "  " << sigma << "  " << Delta / sigma
             << " @ " << nominal->GetPointX(i) << endl;
        if(sigma != 0.0)    graph->SetPointY( i, Delta/sigma );
        else                graph->SetPointY( i, 0.0 );
        graph->SetPointX( i, nominal->GetPointX(i) );
        graph->SetPointError( i, variation->GetErrorX(i), 0 );
    }

    return graph;
}

void plotDiffXSecAndBarlow(const std::vector<std::string>& fileNames, std::pair<string,string> cutPair) {
    if (fileNames.empty()) {
        std::cerr << "No files provided!" << std::endl;
        return;
    }

    // Get name to save plot
    string cutName = cutPair.first; string latexCut = cutPair.second;
    string cutFile = fileNames[1].substr(fileNames[1].find("/")+1);
    cout << cutFile << endl;
    string saveName = cutFile.substr(0,cutFile.find(cutName)+cutName.size()) +
                      cutFile.substr(cutFile.find("_emin"));
    saveName = saveName.substr(0, saveName.find_last_of("."));
    // Get enbins for plot title
    string enMin = saveName.substr(saveName.find("emin")+5);
    enMin = enMin.substr(0,enMin.find_first_of("_"));
    string enMax = saveName.substr(saveName.find("emax")+5);
    enMax = enMax.substr(0,enMax.find_first_of("_"));
    
    vector<string> cutVals;
    // Load TGraphErrors from files
    std::vector<TGraphErrors*> graphs;
    std::vector<TGraphErrors*> barlowGraphs;
    for (const auto& fileName : fileNames) {
        TGraphErrors* graph = new TGraphErrors(fileName.c_str());
        //graph->SetMinimum(0);
        graphs.push_back(graph);
        // Get the cut parameter for legend
        string cutVal = fileName.substr(fileName.find(cutName)+cutName.size()+1);
        cutVal = cutVal.substr(0,cutVal.find("_"));
        cutVals.push_back(cutVal);
    }
    // erase first element, this is the nominal and is nonsense
    cutVals.erase(cutVals.begin());

    // Calculate standard deviation graph
    for (int i = 1; i < graphs.size(); ++i){
        TGraphErrors* barlowGraph = calc_barlow(graphs[0], graphs[i]);
        barlowGraphs.push_back(barlowGraph);
      
        if (!barlowGraph) return;
    }

    // Create canvas and pads
    TCanvas* c1 = new TCanvas("c1", "Graphs and Barlow");
    TPad* topPad = new TPad("topPad", "Top Pad", 0.0, 0.3, 1.0, 1.0);
    TPad* bottomPad = new TPad("bottomPad", "Bottom Pad", 0.0, 0.0, 1.0, 0.3);
    topPad->SetBottomMargin(0.001);
    bottomPad->SetTopMargin(0.001);
    bottomPad->SetBottomMargin(0.4);

    c1->cd();
    topPad->Draw();
    bottomPad->Draw();

    // Draw graphs in the top pad
    topPad->cd();
    TLegend* legend = new TLegend(0.60,0.51, 0.93, 0.9);
    legend->SetFillStyle(0);
    legend->SetBorderSize(0);
    gPad->SetGrid();

    int colorIndex = 1;
    int cutIndex = 0;
    TMultiGraph *mg = new TMultiGraph();
    for (auto* graph : graphs) {
        graph->SetMarkerStyle(20);
        graph->SetMarkerColor(colorIndex);
        graph->SetLineColor(colorIndex);
        graph->GetXaxis()->SetLimits(6,12);
        
        if (colorIndex == 1){
            graph->SetFillColorAlpha(kBlack,0.3);
            graph->SetLineWidth(3);
            mg->Add(graph,"AE3");
            legend->AddEntry(graph, "Nominal", "f");
        } else{
            mg->Add(graph,"P SAME");
            legend->AddEntry(graph, (latexCut + cutVals[cutIndex]).c_str(), "lep");
            cutIndex++;
        }
        
        colorIndex++;
        if(colorIndex==5) colorIndex++;
    }
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    mg->SetTitle( ("#bf{E_{#gamma} (GeV): ("+enMin+", "+enMax+")}").c_str() );
    mg->Draw("ap");
    mg->GetXaxis()->SetLimits(0,2.5);
    legend->Draw();
    // Draw standard deviation graph in the bottom pad
    bottomPad->cd();
    gPad->SetGrid();

    //Draw Barlow plots
    TMultiGraph *mg1 = new TMultiGraph();
    colorIndex = 2;
    for (auto* barlowGraph : barlowGraphs) {
        barlowGraph->SetMarkerStyle(20);
        barlowGraph->SetMarkerColor(colorIndex);
        barlowGraph->SetLineColor(colorIndex);
                
        if (colorIndex == 2)
            mg1->Add(barlowGraph,"AP");
        else
            mg1->Add(barlowGraph,"P SAME");
        
        colorIndex++;
        if(colorIndex==5) colorIndex++;
    }
    mg1->Draw("ap");
    mg1->GetXaxis()->SetLimits(0,2.5);

    double ymin = mg1->GetHistogram()->GetMinimum();
    double ymax = mg1->GetHistogram()->GetMaximum();
    double ysym = abs(ymax) > abs(ymin) ? abs(ymax) : abs(ymin);
    if (ysym<6) ysym = 6;
    
    mg->GetYaxis()->SetTitle("d#sigma/dt (nb/GeV^{2} )");
    mg1->GetXaxis()->SetTitle("-t (GeV^{2 })");
    mg->SetMinimum(0.01);
    mg->GetXaxis()->SetNdivisions(510);

    mg1->GetYaxis()->SetTitle("#sigma_{#it{B}}");
    mg1->GetYaxis()->CenterTitle(true);
    mg1->GetYaxis()->SetRangeUser(-ysym-1,ysym+1);
    mg1->GetYaxis()->SetNdivisions(505);
    mg1->GetXaxis()->SetNdivisions(510);
    mg1->GetXaxis()->SetLabelSize(0.13);
    mg1->GetYaxis()->SetLabelSize(0.13);
    mg1->GetXaxis()->SetTitleSize(0.17);
    mg1->GetYaxis()->SetTitleSize(0.17);
    mg1->GetXaxis()->SetTitleOffset(0.9);
    mg1->GetYaxis()->SetTitleOffset(0.35);

    // Draw box in balow 4sigma range
    TBox *box = new TBox(0,-4,2.5,4);
    box->SetFillColorAlpha(kAzure,0.1);
    box->Draw();
    // Save canvas
    c1->SaveAs( ("plots/barlow_"+saveName+".pdf").c_str());

    // Clean up
    for (auto* graph : graphs) delete graph;
    for (auto* barlowGraph : barlowGraphs) delete barlowGraph;
}

void plotTotXSecAndBarlow(const std::vector<std::string>& fileNames, std::pair<string,string> cutPair) {
    if (fileNames.empty()) {
        std::cerr << "No files provided!" << std::endl;
        return;
    }

    string cutName = cutPair.first; string latexCutName = cutPair.second;
    // get name to save plot
    string cutFile = fileNames[1].substr(fileNames[1].find("/")+1); string saveName ;
    cout << cutFile << endl;
    saveName = cutFile.substr(0,cutFile.find(cutName)+cutName.size()) ;
    
    vector<string> cutVals;
    // Load TGraphErrors from files
    std::vector<TGraphErrors*> graphs;
    std::vector<TGraphErrors*> barlowGraphs;
    for (const auto& fileName : fileNames) {
        TGraphErrors* graph = new TGraphErrors(fileName.c_str());
        //graph->SetMinimum(0);
        graphs.push_back(graph);
        // Get the cut parameter for legend
        string cutVal = fileName.substr(fileName.find_last_of("_")+1);
        cutVal = cutVal.substr(0,cutVal.find_last_of("."));
        cutVals.push_back(cutVal);
    }
    // erase first element, this is the nominal and is nonsense
    cutVals.erase(cutVals.begin());

    // Calculate standard deviation graph
    for (int i = 1; i < graphs.size(); ++i){
        TGraphErrors* barlowGraph = calc_barlow(graphs[0], graphs[i]);
        barlowGraphs.push_back(barlowGraph);
      
        if (!barlowGraph) return;
    }

    // Create canvas and pads
    TCanvas* c1 = new TCanvas("c1", "Graphs and Barlow");
    TPad* topPad = new TPad("topPad", "Top Pad", 0.0, 0.3, 1.0, 1.0);
    TPad* bottomPad = new TPad("bottomPad", "Bottom Pad", 0.0, 0.0, 1.0, 0.3);
    topPad->SetBottomMargin(0.001);
    bottomPad->SetTopMargin(0.001);
    bottomPad->SetBottomMargin(0.4);

    c1->cd();
    topPad->Draw();
    bottomPad->Draw();

    // Draw graphs in the top pad
    topPad->cd();
    TLegend* legend = new TLegend(0.61,0.51, 0.89, 0.9);
    legend->SetFillStyle(0);
    legend->SetBorderSize(0);
    gPad->SetGrid();

    int colorIndex = 1;
    int cutIndex = 0;
    TMultiGraph *mg = new TMultiGraph();
    for (auto* graph : graphs) {
        graph->SetMarkerStyle(20);
        graph->SetMarkerColor(colorIndex);
        graph->SetLineColor(colorIndex);
        graph->GetXaxis()->SetLimits(6,12);
        
        if (colorIndex == 1){
            graph->SetFillColorAlpha(kBlack,0.3);
            graph->SetLineWidth(3);
            mg->Add(graph,"AE3");
            legend->AddEntry(graph, "Nominal", "f");
        } else{
            mg->Add(graph,"P SAME");
            legend->AddEntry(graph, (latexCutName + cutVals[cutIndex]).c_str(), "lep");
            cutIndex++;
        }
        
        colorIndex++;
        if(colorIndex==5) colorIndex++;
    }
    mg->Draw("ap");
    mg->GetXaxis()->SetLimits(6.2,11.6);
    legend->Draw();
    // Draw standard deviation graph in the bottom pad
    bottomPad->cd();
    gPad->SetGrid();

    //Draw Barlow plots
    TMultiGraph *mg1 = new TMultiGraph();
    colorIndex = 2;
    for (auto* barlowGraph : barlowGraphs) {
        barlowGraph->SetMarkerStyle(20);
        barlowGraph->SetMarkerColor(colorIndex);
        barlowGraph->SetLineColor(colorIndex);
        barlowGraph->GetXaxis()->SetLimits(6,12);
        
        if (colorIndex == 2)
            mg1->Add(barlowGraph,"AP");
        else
            mg1->Add(barlowGraph,"P SAME");
        
        colorIndex++;
        if(colorIndex==5) colorIndex++;
    }
    mg1->Draw("ap");
    mg1->GetXaxis()->SetLimits(6.2,11.6);
    double ymin = mg1->GetHistogram()->GetMinimum();
    double ymax = mg1->GetHistogram()->GetMaximum();
    double ysym = abs(ymax) > abs(ymin) ? abs(ymax) : abs(ymin);
    if(ysym<6) ysym = 6;
    
    //graphs[0]->SetTitle("Graph");
    mg->GetYaxis()->SetTitle("#sigma(#gamma p#rightarrow K^{+}K^{+}#Xi^{-}) (nb)");
    mg->GetXaxis()->SetNdivisions(510);
    mg1->GetYaxis()->SetTitle("#sigma_{#it{B}}");
    mg1->GetXaxis()->SetTitle("E_{#gamma} (GeV)");
    mg1->GetYaxis()->CenterTitle(true);
    mg1->GetYaxis()->SetRangeUser(-ysym,ysym);
    mg1->GetYaxis()->SetNdivisions(505);
    mg1->GetXaxis()->SetNdivisions(510);
    mg1->GetXaxis()->SetLabelSize(0.13);
    mg1->GetYaxis()->SetLabelSize(0.13);
    mg1->GetXaxis()->SetTitleSize(0.16);
    mg1->GetYaxis()->SetTitleSize(0.16);
    mg1->GetXaxis()->SetTitleOffset(1.1);
    mg1->GetYaxis()->SetTitleOffset(0.35);

    // Draw box in balow 4sigma range
    TBox *box = new TBox(6.2,-4,11.6,4);
    box->SetFillColorAlpha(kAzure,0.1);
    box->Draw();
    // Save canvas
    c1->SaveAs( ("plots/barlow_"+saveName+".pdf").c_str());

    // Clean up
    for (auto* graph : graphs) delete graph;
    for (auto* barlowGraph : barlowGraphs) delete barlowGraph;
}

void plotMultipleGraphs(const std::vector<std::string>& fileNames) {
    // Check if file names are provided
    if (fileNames.empty()) {
        std::cerr << "No files provided!" << std::endl;
        return;
    }

    // Prepare a canvas
    TCanvas* c = new TCanvas("c", "TGraphErrors", 800, 600);
    c->SetGrid();

    TLegend* legend = new TLegend(0.62,0.51, 0.93, 0.9); // For labeling graphs

    int colorIndex = 1; // Color index for different graphs
    for (const auto& fileName : fileNames) {
        // Create TGraphErrors
        TGraphErrors* graph = new TGraphErrors(fileName.c_str());
        graph->SetMarkerStyle(20);
        graph->SetMarkerColor(colorIndex);
        graph->SetLineColor(colorIndex);
        graph->SetTitle(("Data from " + fileName).c_str());
        graph->GetXaxis()->SetTitle("X-axis");
        graph->GetYaxis()->SetTitle("Y-axis");
        
        // Draw the graph
        if (colorIndex == 1)
            graph->Draw("AP"); // Draw with axis and points for the first graph
        else
            graph->Draw("P SAME"); // Overlay others

        // Add to legend
        legend->AddEntry(graph, fileName.c_str(), "lp");

        colorIndex++; // Change color for next graph
        if(colorIndex==5) colorIndex++;
    }

    // Draw legend
    legend->Draw();

    // Save canvas to file
    c->SaveAs("multiple_graphs.pdf");
}



TGraphErrors* calculateStdDevGraph(const std::vector<TGraphErrors*>& graphs) {
    if (graphs.empty()) {
        std::cerr << "No graphs provided for standard deviation calculation!" << std::endl;
        return nullptr;
    }

    size_t numPoints = graphs[0]->GetN(); // Number of points in each graph
    size_t numGraphs = graphs.size();

    std::vector<double> meanX(numPoints, 0.0); // To store x-coordinates
    std::vector<double> stdDevY(numPoints, 0.0); // To store std deviation of y-values
    std::vector<double> stdDevX(numPoints, 0.0); // Assuming no x-errors for std-dev

    // Process each point
    for (size_t i = 0; i < numPoints; ++i) {
        std::vector<double> yValues;

        // Collect all y-values for the current x-value
        for (size_t j = 0; j < numGraphs; ++j) {
            double x, y;
            graphs[j]->GetPoint(i, x, y);

            // Use the x-value from the first graph (assume aligned x-points)
            if (j == 0) {
                meanX[i] = x;
            }

            yValues.push_back(y);
            cout << "XValue, YValues: " << x << " " << y << endl;
        }

        // Compute standard deviation for y-values
        double sumY = 0.0, sumYSq = 0.0;
        for (double y : yValues) {
            sumY += y;
            sumYSq += y * y;
        }


        double meanY = sumY / yValues.size();
        double varianceY = (sumYSq / yValues.size()) - (meanY * meanY);
        stdDevY[i] = (varianceY > 0) ? std::sqrt(varianceY) : 0.0;
        cout << "Mean: " << meanY << " Variance: " << varianceY << " StdDev: " << stdDevY[i] << endl;
    }

    // Create and populate the new TGraphErrors for standard deviation
    TGraphErrors* stdDevGraph = new TGraphErrors(numPoints);
    for (size_t i = 0; i < numPoints; ++i) {
        stdDevGraph->SetPoint(i, meanX[i], stdDevY[i]);
        stdDevGraph->SetPointError(i, stdDevX[i], 0); // x-errors are zero
    }

    stdDevGraph->SetMarkerStyle(21);
    stdDevGraph->SetMarkerColor(kRed);
    stdDevGraph->SetLineColor(kRed);

    return stdDevGraph;
}

void SetStyle()
{
    gStyle->SetCanvasColor(0);
    gStyle->SetCanvasBorderSize(5);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasDefH(800);
    gStyle->SetCanvasDefW(700);

    gStyle->SetPadColor       (0);
    gStyle->SetPadBorderSize  (10);
    gStyle->SetPadBorderMode  (0);
    gStyle->SetPadBottomMargin(0.16);
    gStyle->SetPadTopMargin   (0.08);
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

    gStyle->SetNdivisions(510);

    gStyle->SetLineWidth(1);
    gStyle->SetHistLineWidth(1);
    //gStyle->SetLegendBorder(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLegendFillColor(0);
    gStyle->SetLegendFont(132);
    gStyle->SetLegendTextSize(0.06);
    gStyle->SetMarkerSize(1.0);
    gStyle->SetMarkerStyle(20);

    gStyle->SetLabelSize(0.055,"X");
    gStyle->SetLabelSize(0.055,"Y");

    gStyle->SetLabelOffset(0.008,"X");
    gStyle->SetLabelOffset(0.008,"Y");

    gStyle->SetLabelFont(132,"X");
    gStyle->SetLabelFont(132,"Y");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetTitleFont(132);
    gStyle->SetTitleFont(132,"X");
    gStyle->SetTitleFont(132,"Y");

    gStyle->SetTitleSize(0.055,"T");
    gStyle->SetTitleSize(0.075,"X");
    gStyle->SetTitleSize(0.075,"Y");

    gStyle->SetTitleOffset(0,"T");
    gStyle->SetTitleOffset(1.,"X");
    gStyle->SetTitleOffset(0.85,"Y");

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
