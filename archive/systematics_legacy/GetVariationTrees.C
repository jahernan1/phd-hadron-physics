#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

//initiate functions
void GetPrepedFlatTree(string root_file_name, vector<double> vecCuts, string delim, int n_threads = 16 ) ;

//main
int GetVariationTrees()
{
  /*vv_Cuts[loc_i] 
    0: chisqndf
    1: total_mm2
    2: xim_pathlensig
    3: lambda_pathlensig
    4: kp_highp_P3-kp_lowp_P3(kaon momentum separation)
   */
  vector<vector<double>> vv_Cuts =
    { {9, 0.02, 2, 0, 1}, {8, 0.025, 2, 0, 1}, {8, 0.02, 1.5, 0, 1},
      {8, 0.02, 2, 0.5, 1}, {8, 0.02, 2, 0, 1.5}
    };
  vector<string> vec_Delim =
    {"vary_chisqndf+1", "vary_total_mm2+005", "vary_xim_pathlensig-05",
     "vary_lambda_pathlensig+05", "vary_kp_momsep_+05"};
  //Data
  for(Int_t loc_i=0; loc_i<vv_Cuts.size();loc_i++ )
    {
      // GetPrepedFlatTree();
      // GetPrepedFlatTree("flatTree_kpkpxim__M23_2017-01_ana45_ystar2400_genr8");
      GetPrepedFlatTree("flatTree_kpkpxim__M23_2017-01_ana56", vv_Cuts[loc_i], vec_Delim[loc_i]);
      GetPrepedFlatTree("flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2", vv_Cuts[loc_i], vec_Delim[loc_i]);
      GetPrepedFlatTree("flatTree_kpkpxim__B4_M23_2018-01_ana03", vv_Cuts[loc_i], vec_Delim[loc_i]);
      GetPrepedFlatTree("flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2", vv_Cuts[loc_i], vec_Delim[loc_i]);
      GetPrepedFlatTree("flatTree_kpkpxim__B4_M23_2018-08_ana02", vv_Cuts[loc_i], vec_Delim[loc_i]);
      GetPrepedFlatTree("flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2", vv_Cuts[loc_i], vec_Delim[loc_i]);
    }
  return 0;
}

void GetPrepedFlatTree(string root_file_name, vector<double> vecCuts, string delim, int n_threads = 8 ) 
{
    /* 
       Initalize variables 
    */
    string treeDir = "/d/grid17/hjesse/Trees/flatTree/rawTrees/";
    string saveDir = "/d/grid17/hjesse/AnalysisNote/systematics/root_trees/";
    /* 
       Parallelize with n threads 
    */
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	/* 
       Import select braches to speed things up 
    */
    // std::vector<std::string> branches = {};
	/* 
       make data frame
	   format : tree name, file name, branches to open
    */
    auto df0 = ROOT::RDataFrame("flatTree_kpkpxim", (treeDir+root_file_name+".root").c_str());
    //auto df = df0.Filter("decayxim_M<1.334 && decayxim_M>1.31","xiMassCut")
    //.Filter("best_combo==1");
    // Dataset specific cut
    // Perform nominal cuts
    auto df1 = df0.Filter("beam_E > 6.4 && beam_E < 11.4")
      .Filter("best_combo==1","bestCombo")
      .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")
      .Filter(("chisqndf<"+to_string(vecCuts[0])).c_str(), ("chisqndf<"+to_string(vecCuts[0])).c_str())
      .Filter(("abs(total_mm2)<"+to_string(vecCuts[1])).c_str(), ("abs(total_mm2)<"+to_string(vecCuts[1])).c_str())
      .Filter(("xim_pathlensig>"+to_string(vecCuts[2])).c_str(), ("xim_pathlensig>"+to_string(vecCuts[2])).c_str())
      .Filter(("lambda_pathlensig>"+to_string(vecCuts[3])).c_str(), ("lambda_pathlensig>"+to_string(vecCuts[3])).c_str())
      .Filter(("kp_highp_P3-kp_lowp_P3>"+to_string(vecCuts[4])).c_str(), ("kp_momsep>"+to_string(vecCuts[4])).c_str())
      .Define("kphigh_theta","kphigh_p4.Theta()*180/TMath::Pi()")
      .Define("kplow_theta","kplow_p4.Theta()*180/TMath::Pi()");
    
    /* Display Table of tree values */
    // df1.Display({"evnt_num", "combo_num", "chisqndf", "kp_highp_P3", "kp_lowp_P3", "decayxim_M"}, 50)->Print();
    
    /* Print cut analysis */
    cout << root_file_name << endl;
    cout << "Variation: " << delim << endl;
    df1.Filter("decayxim_M<1.334 && decayxim_M>1.308")
      .Stats("decayxim_M")->Print();

    //ofstream txtOut;
    //df1.Report()->Print();//data
        
    cout << "Saving flat trees with cuts applied...\n" << endl;
    // df.Snapshot("flatTree_kpkpxim",
    //             ("/d/grid17/hjesse/AnalysisNote/flatTrees/"+root_file_name+"_xiMassCut.root").c_str());
    df1.Snapshot("flatTree_kpkpxim",
                 (saveDir+root_file_name+"_"+delim+".root").c_str());
}
