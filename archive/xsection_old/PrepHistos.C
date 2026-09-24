#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

void GetPrepedFlatTree(string root_file_name="kpkpxim__M23_2017-01_ver56_nominal", string mc_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_genr8_nominal_Weighted", string thrown_file_name="kpkpxim__M23_2017-01_ver56_Ystar2400_1600_genr8_Weighted", string flux_file_name="flux_30274_31057_r4", int n_threads = 8 );
void GetHistos(string delim="_allCuts");
  
//main
int PrepHistos()
{
  // GetHistos("_vertex");
  // GetHistos("_ximVertexCut");
  // GetHistos("_vertexCuts");
  // GetHistos();
  GetHistos("_allKaonSep");
    
  return 0;
}

void GetHistos(string delim="_allCuts")
{ 
  // Get the tree for the xsection
  GetPrepedFlatTree("kpkpxim__M23_2017-01_ana56_nominal"+delim,"kpkpxim__M23_2017-01_ana56_gen_amp_V2_nominal"+delim+"_Weighted", "kpkpxim__M23_2017-01_ana56_gen_amp_V2_nominal"+delim+"_Weighted", "flux_30274_31057_r4");
  GetPrepedFlatTree("kpkpxim__B4_M23_2018-01_ana03_nominal"+delim,"kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_nominal"+delim+"_Weighted","kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_nominal"+delim+"_Weighted", "flux_40856_42559");
  GetPrepedFlatTree("kpkpxim__B4_M23_2018-08_ana02_nominal"+delim,"kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_nominal"+delim+"_Weighted","kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_nominal"+delim+"_Weighted", "flux_50685_51768");
}

void GetPrepedFlatTree(string root_file_name="kpkpxim__M23_2017-01_ana56_nominal", string mc_file_name="kpkpxim__M23_2017-01_ana56_gen_amp_nominal", string thrown_file_name="kpkpxim__M23_2017-01_ana56_gen_amp", string flux_file_name="flux_30274_31057_r4", int n_threads = 16 ) {
    cout << "Prepping cross section component histograms for " << root_file_name << endl;
    // Parallelize with n threads
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
    string outputDir = "/d/grid17/hjesse/AnalysisNote/xsection/prepped_trees/";
    TObjArray hList(0);
    TObjArray hListWeighted(0);

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
    auto df = ROOT::RDataFrame("flatTree_kpkpxim", ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_"+root_file_name+".root").c_str(), branches);
    auto dfmc = ROOT::RDataFrame("flatTree_kpkpxim", ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_"+mc_file_name+".root").c_str(), branchesMC);
    auto dfT = ROOT::RDataFrame("flatTree_thrown_kpkpxim", ("/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_thrown_"+thrown_file_name+".root").c_str());
      //.Filter("main_pid==1");
     
    // Make histograms for the cross section code
    auto h = df.Histo3D({"Xi_Egamma_t","counts; beam_E; decayxim_M; -tval",100,6.4,11.4,125,1.25,1.5,100,0,10},"beam_E","decayxim_M","t_dist");
    hList.Add(h.GetPtr());
    hListWeighted.Add(h.GetPtr());
    auto hmc = dfmc.Histo3D({"Xi_Egamma_t_MC","counts; beam_E; decayxim_M; -tval",100,6.4,11.4,125,1.25,1.5,100,0,10},"beam_E","decayxim_M","t_dist");
    auto hmcW = dfmc.Histo3D({"Xi_Egamma_t_MC","counts; beam_E; decayxim_M; -tval",100,6.4,11.4,125,1.25,1.5,100,0,10},"beam_E","decayxim_M","t_dist", "mc_weight");
    cout << "MC Integral: " << hmc.GetPtr()->Integral() << "/ Weighted: " << hmcW.GetPtr()->Integral() << endl; 
    //hmcW->Scale(hmc.GetPtr()->Integral()/hmcW.GetPtr()->Integral());
    hList.Add(hmc.GetPtr());
    hListWeighted.Add(hmcW.GetPtr());
    auto hT = dfT.Histo2D({"thrown_Egamma_t","counts; beam_E; -tval",100,6.4,11.4,100,0,10},"beam_E","t_dist");
    auto hTW = dfT.Histo2D({"thrown_Egamma_t","counts*mc_weight; beam_E; -tval",100,6.4,11.4,100,0,10},"beam_E","t_dist", "mc_weight");
    cout << "Thrown Integral: " << hT.GetPtr()->Integral() << "/ Weighted: " << hTW.GetPtr()->Integral() << endl; 
    cout << "Ratio: " << hmc.GetPtr()->Integral()/hT.GetPtr()->Integral() << " Weighted " << hmcW.GetPtr()->Integral()/hTW.GetPtr()->Integral() << "\n" << endl;
    //hTW->Scale(hT.GetPtr()->Integral()/hTW.GetPtr()->Integral());
    hList.Add(hT.GetPtr());
    hListWeighted.Add(hTW.GetPtr());
    
    // Get the fluxfile 
    TFile *fluxf = TFile::Open(("/d/grid17/hjesse/analysis/kpkpxim/flux/"+flux_file_name+".root").c_str());
    TH1* hist_flux = (TH1*)fluxf->Get("tagged_flux")->Clone();
    hList.Add(hist_flux);
    hListWeighted.Add(hist_flux);

    // Save histograms to trees for xsection
    TFile *outputFile = new TFile((outputDir+"xsec_"+root_file_name+".root").c_str(), "RECREATE");
    hList.Write();
    TFile *outputFileWeighted = new TFile((outputDir+"xsec_"+root_file_name+"_weighted.root").c_str(), "RECREATE");
    hListWeighted.Write();
}
