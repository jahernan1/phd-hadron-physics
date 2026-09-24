#include "gxana/common/Paths.h"
#include <TFile.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <ROOT/RDataFrame.hxx>

void CompareFromTree
     (
      std::string treename = "flatTree_kpkpxim__B4_M23_2018-08_ana02",
      std::string branchName = "decayxim_M", 
      std::vector<double> histBinning = {50,1.27,1.45}
      ){
    // File and branch names
    std::string treedir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
    std::string file1 = treename+"_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root";
    std::string file2 = treename+"_gen_amp_V2_nobkg_nominal_kphighrap.root";
    std::string treeName = "flatTree_kpkpxim";

    // Create RDataFrames
    ROOT::RDataFrame df1(treeName, treedir+file1);
    ROOT::RDataFrame df2(treeName, treedir+file2);

    // Define histogram settings
    int nbins = (int)histBinning[0];
    double xmin = histBinning[1], xmax = histBinning[2]; // Adjust as needed

    // Create histograms
    auto h1 = df1.Histo1D({"h1", "Histogram from file1;X axis;Y axis", nbins, xmin, xmax}, branchName, "hybrid_combo");
    auto h2 = df2.Histo1D({"h2", "Histogram from file2;X axis;Y axis", nbins, xmin, xmax}, branchName, "hybrid_combo");

    // Scale h2 to match integral of h1
    double scaleFactor = h1->Integral() / h2->Integral();
    h2->Scale(scaleFactor);

    // Create canvas and draw histograms
    TCanvas* c = new TCanvas("c", "Comparison", 800, 600);
    gPad->SetLogy();
    h1->SetLineColor(kRed);
    h2->SetLineColor(kBlue);
    h1->DrawClone("e1");
    h2->DrawClone("SAME e1");

    // Save or show the canvas
    // c->SaveAs("comparison.png");
}
