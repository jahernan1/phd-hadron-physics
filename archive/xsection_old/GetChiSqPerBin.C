#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

void MakeChiSqPerBin(string root_file_name="kpkpxim__M23_2017-01_ver56_nominal", string mc_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_nominal", int n_threads = 8 );
  
//main
int GetChiSqPerBin()
{
  
  MakeChiSqPerBin("kpkpxim__M23_2017-01_ver56_nominal_vertexCuts","kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_nominal_vertexCuts_Weighted");
  MakeChiSqPerBin("kpkpxim__B4_M23_2018-01_ver03_nominal_vertexCuts","kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600_nominal_vertexCuts_Weighted");
  MakeChiSqPerBin("kpkpxim__B4_M23_2018-08_ver02_nominal_vertexCuts","kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600_nominal_vertexCuts_Weighted");

  return 0;
}

void MakeChiSqPerBin(string root_file_name="kpkpxim__M23_2017-01_ver56_nominal", string mc_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_nominal", int n_threads = 8 ) {
	// Parallelize with n threads
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);

    //ROOT::RDataFrame df1;
    //ROOT::RDataFrame dfmc1;
    string bin_str;
    Int_t enBins = 10;
    Double_t enBinWidth = 0.5; 
    string outputDir = "/d/grid17/hjesse/AnalysisNote/xsection/";
    TObjArray hList(0);
    TCanvas *can = new TCanvas("can","can");
    can->DivideSquare(enBins);

    // Branches you want to use get
    std::vector<std::string> branches = {"beam_E",
                                         "decayxim_M",
                                         "t_dist",
                                         "chisqndf",
                                         "confidencelvl"};
    std::vector<std::string> branchesMC = branches;
    branchesMC.push_back("mc_weight");

	// make data frame
	// format : tree name, file name, branches to open
    auto df = ROOT::RDataFrame("flatTree_kpkpxim", ("/d/grid17/hjesse/AnalysisNote/QFactors/logs/"+ root_file_name+"_1111111/postQVal_flatTree_"+root_file_name+"_1111111.root").c_str(), branches);
    auto dfmc = ROOT::RDataFrame("flatTree_kpkpxim", ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_"+mc_file_name+".root").c_str(), branchesMC);
         
    // Make histograms for the cross section code
    for(Int_t loc_i = 0; loc_i < enBins; ++loc_i)
      {
        cout << "Processing Bin " << loc_i << endl;
        bin_str = "beam_E > " + to_string( 6.4+(loc_i*enBinWidth)) + " && beam_E <= " + to_string(6.4+((loc_i+1)*enBinWidth));
        // only keep data for specific bin
        auto df1 = df.Filter( bin_str.c_str());
        auto dfmc1 = dfmc.Filter( bin_str.c_str());
        
        // make plots
        auto h = df1.Histo1D({("chisqndf_bin"+to_string(loc_i)).c_str(), ( bin_str + " ; #chi^{2}/d.o.f.; counts ").c_str(), 20, 0, 6}, "chisqndf", "qvalue_decayxim_M");
        hList.Add(h.GetPtr());
        auto hmc = dfmc1.Histo1D({("chisqndf_mcbin"+to_string(loc_i)).c_str(), ( bin_str + " ; #chi^{2}/d.o.f.; counts ").c_str(), 20, 0, 6}, "chisqndf");
        hList.Add(hmc.GetPtr());
        // plot on canvas
        can->cd(loc_i+1);
        h->SetFillColorAlpha(kGray,0.8);
        h->DrawClone("hist");
        hmc->SetFillColorAlpha(kMagenta,0.8);
        hmc->Scale(h->GetMaximum()/hmc->GetMaximum());
        hmc->DrawClone("hist same");
      }

    can->SaveAs((outputDir+"chisqndfbins_"+root_file_name+".pdf").c_str());
    // Save histograms to trees for xsection
    //TFile *outputFile = new TFile((outputDir+"chisqndfbins_"+root_file_name+".root").c_str(), "RECREATE");
    //hList.Write();
    
    // cout << "Saving Flattree with cuts applied...\n" << cout;
    //df1.Snapshot("kpkpxim_flatTree", ("/d/grid17/hjesse/AnalysisNote/flatTrees/"+root_file_name+"_allCuts.root").c_str());
    //df2.Snapshot("kpkpxim_flatTree", ("/d/grid17/hjesse/AnalysisNote/flatTrees/"+root_file_name+"_allCuts_PathlenSig.root").c_str());
}
