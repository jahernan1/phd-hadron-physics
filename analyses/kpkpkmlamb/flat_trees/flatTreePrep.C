#include "gxana/common/Paths.h"
#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

//initiate functions
void GetPrepedFlatTree(string root_file_name="flatTree_kpkpkmlamb__B4_M18_2017-01_ana55", string cut_str="chisqndf < 3 && lambda_pathlensig > 2", int n_threads = 16 );

//main
int flatTreePrep()
{
    string cutStr="chisqndf < 3 && lambda_pathlensig > 0 && kp1_km_p4.M()>1.1 && kp2_km_p4.M()>1.1 && lambda_p4.M()>1.107 && lambda_p4.M()<1.125";//&& lambda_pathlensig > 0
  //Data
  GetPrepedFlatTree("flatTree_kpkpkmlamb__B4_M18_2017-01_ana55", cutStr);
  GetPrepedFlatTree("flatTree_kpkpkmlamb__B4_M18_2018-01_ana22", cutStr);
  GetPrepedFlatTree("flatTree_kpkpkmlamb__B4_M18_2018-08_ana19", cutStr);
  
  return 0;
}

// gxana: default arguments only on the declaration above (cling rejects repeating them)
void GetPrepedFlatTree(string root_file_name, string cut_str, int n_threads)
{
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
    // gxana: raw flat trees from $GXANA_DATA (legacy: the FSU grid13 rawTrees dir)
    auto df0 = ROOT::RDataFrame("flatTree_kpkpkmlamb", (gxana::EnvPath("GXANA_DATA", "Trees/flatTree/rawTrees/")+root_file_name+".root").c_str());
    //auto df = df0.Filter("decayxim_M<1.334 && decayxim_M>1.31","xiMassCut")
    //.Filter("best_combo==1");
    // Dataset specific cut
    // Perform nominal cuts
    auto df1 = df0.Filter("beam_E > 6.4 && beam_E < 11.4")
      .Filter("best_combo==1","bestCombo")
      .Filter("abs(total_mm2) < 0.02 ","mm2Cut")
      .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")
      .Filter(cut_str.c_str(), "allCuts");
          
    /* Display Table of tree values */
    // df1.Display({"evnt_num", "combo_num", "chisqndf", "kp_highp_P3", "kp_lowp_P3", "decayxim_M"}, 50)->Print();
    
    /* Print cut analysis */
    cout << root_file_name << endl;
    df1.Report()->Print();//data
        
    cout << "Saving flat trees with cuts applied...\n" << endl;
    // df.Snapshot("flatTree_kpkpkmlamb",
    //             (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_xiMassCut.root").c_str());
    // gxana: create $GXANA_DATA/kpkpkmlamb/ (the legacy dir existed on disk)
    gSystem->mkdir(gxana::EnvPath("GXANA_DATA", "kpkpkmlamb").c_str(), kTRUE);
    df1.Snapshot("flatTree_kpkpkmlamb",
                 (gxana::EnvPath("GXANA_DATA", "kpkpkmlamb/")+root_file_name+"_nominal_allCuts.root").c_str());
 }
