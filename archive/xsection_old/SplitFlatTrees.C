#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

void GetPrepedFlatTree(string root_file_name, string mc_file_name, string thrown_file_name, vector<vector<Double_t>> vv_EnRanges, vector<vector<Double_t>> vv_TRanges, int n_threads = 16 );
void GetTree(string delim="_allCuts", string sim_delim="gen_amp_V2_2D_ac");

//main
int SplitFlatTrees()
{
  //GetTree("_ximVertexCut");  
    GetTree("_tCut");
    GetTree("_momCut");
    // GetTree("_cosThetaCut");
    GetTree("_rapidityCuts");
      
    return 0;
}

void GetTree(string delim="_allCuts", string sim_delim="gen_amp_V2_2D_ac")
{
    // 2d vector of Egamma and -t bins
    Int_t enBins=10; Double_t enMin=6.4, enMax=11.4;
    Double_t enBinWidth=(enMax-enMin)/enBins;
    Int_t tBins=30; Double_t tMax=2.1, tMin=0.1, tTmp=0.53;
    Double_t tBinWidth=(tMax-tMin)/tBins;
    vector<vector<Double_t>> vv_EnCutRange{{6.40,7.40},{7.40,7.86},{7.86,8.19},{8.19,8.45},{8.45,8.68},{8.68,9.26},{9.26,10.18},{10.18,11.4}};
    //{{6.40,7.20},{7.20,7.72},{7.72,7.94},{7.94,8.24},{8.24,8.43},{8.43,8.62},{8.62,8.79},{8.79,9.50},{9.50,10.21},{10.21,11.40}};
    vector<vector<Double_t>> vv_TCutRange{{0.10,0.35},{0.35,0.53},{0.53,0.71},{0.71,0.92},{0.92,1.19},{1.19,1.53},{1.53,2.40}};
    //Exponential Binning testing
    // vv_TCutRange.push_back({tMin,0.35});
    // vv_TCutRange.push_back({0.35,tTmp}); tMin=tTmp;
    //     while(tTmp<tMax)
    //       {
    //         double expoBin=exp(tMin+0.1)-exp(tMin);
    //         if(tMin+expoBin > tMax)
    //           vv_TCutRange.push_back({tMin,tMax});
    //         else
    //           vv_TCutRange.push_back({tMin,tMin+expoBin});
    //         tMin+=expoBin;
    //         tTmp=tMin;
    //       }

    
    // Get the tree for the xsection
    //GetPrepedFlatTree("kpkpxim__M23_2017-01_ana56_nominal"+delim,"kpkpxim__M23_2017-01_ana56_gen_amp_V2_2-1_nominal"+delim+"_Weighted", "kpkpxim__M23_2017-01_ana56_gen_amp_V2_2-1_nominal"+delim+"_Weighted", vv_EnCutRange, vv_TCutRange);
    GetPrepedFlatTree("kpkpxim__M23_2017-01_ana56_nominal"+delim,"kpkpxim__M23_2017-01_ana56_"+sim_delim+"_nominal"+delim, "kpkpxim__M23_2017-01_ana56_"+sim_delim+"", vv_EnCutRange, vv_TCutRange);
    GetPrepedFlatTree("kpkpxim__B4_M23_2018-01_ana03_nominal"+delim,"kpkpxim__B4_M23_2018-01_ana03_"+sim_delim+"_nominal"+delim,"kpkpxim__B4_M23_2018-01_ana03_"+sim_delim+"", vv_EnCutRange, vv_TCutRange);
    GetPrepedFlatTree("kpkpxim__B4_M23_2018-08_ana02_nominal"+delim,"kpkpxim__B4_M23_2018-08_ana02_"+sim_delim+"_nominal"+delim,"kpkpxim__B4_M23_2018-08_ana02_"+sim_delim+"", vv_EnCutRange, vv_TCutRange);

}

void GetPrepedFlatTree(string root_file_name, string mc_file_name, string thrown_file_name, vector<vector<Double_t>> vv_EnRanges, vector<vector<Double_t>> vv_TRanges, int n_threads = 16 ) {
  // Parallelize with n threads
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);

    string treeDir = "/d/grid17/hjesse/AnalysisNote/flatTrees/";
    string outputDir = "/d/grid17/hjesse/AnalysisNote/xsection/prepped_ttrees/gen_amp_V2_2D_ac/";
    string qTreePath = "/d/grid17/hjesse/AnalysisNote/QFactors/logs/"+root_file_name+"_1111111/"+"postQVal_flatTree_"+root_file_name+"_1111111.root";
    string logDir = "/d/grid17/hjesse/AnalysisNote/xsection/logs/";
    TObjArray hList(0);
    TObjArray hListWeighted(0);
    
    // Branches you want to use get
    std::vector<std::string> branches = {"beam_E","beam_E_Truth",
                                         "decayxim_M","xim_costheta_hf",
                                         "t_dist","t_dist_truth",
                                         "chisqndf","confidencelvl",
                                         "hybrid_combo",
                                         "best_combo","acc_combo",
                                         "beam_vertexZ","xim_lifetime_restframe",
                                         "kphigh_p4","kplow_p4",
                                         "decayxim_p4","ystar_p4",};
    
    std::vector<std::string> branchesMC = branches;
    //branchesMC.push_back("mc_weight");
    branches.push_back("qvalue_decayxim_M");
    
	// make data frame
	// format : tree name, file name, branches to open
    //auto df = ROOT::RDataFrame("flatTree_kpkpxim", (treeDir+"flatTree_"+root_file_name+".root").c_str(), branches);
    //Get trees for Ebins
    FILE *enStatsF = fopen((logDir+root_file_name+"_EnBins_Yields.txt").c_str(),"w+");
    FILE *enStatsF_MC = fopen((logDir+"recon_"+root_file_name+"_EnBins_Yields.txt").c_str(),"w+");
    FILE *enStatsF_Thrown = fopen((logDir+"thrown_"+root_file_name+"_EnBins_Yields.txt").c_str(),"w+");
    cout << "Prepping flattrees " << root_file_name << endl;
    cout << "Saving Prepped Trees to: " << outputDir << endl;
    for(Int_t loc_E=0; loc_E < vv_EnRanges.size(); loc_E++)
      {
        //
        string str_enMin = to_string(vv_EnRanges[loc_E][0]);
        string str_enMax = to_string(vv_EnRanges[loc_E][1]);
        str_enMin = str_enMin.substr(0,str_enMin.find_last_of('.')+3);
        str_enMax = str_enMax.substr(0,str_enMax.find_last_of('.')+3);
        //Make the sub trees for energy bins
        auto df = ROOT::RDataFrame("flatTree_kpkpxim", qTreePath.c_str(), branches)
          .Filter( ("beam_E>="+str_enMin+"&&beam_E<"+str_enMax).c_str());
        auto dfmc = ROOT::RDataFrame("flatTree_kpkpxim", (treeDir+"flatTree_"+mc_file_name+".root").c_str(), branchesMC)
          .Filter( ("beam_E>="+str_enMin+"&&beam_E<"+str_enMax).c_str());
        auto dfT = ROOT::RDataFrame("flatTree_thrown_kpkpxim", (treeDir+"flatTree_thrown_"+thrown_file_name+".root").c_str(),{"beam_E", "t_dist"})
          .Filter( ("beam_E>="+str_enMin+"&&beam_E<"+str_enMax).c_str());;
        //Get Value of events after cut
        auto data_events = df.Filter("decayxim_M<1.334 && decayxim_M>1.308 && fabs(hybrid_combo)>0").Count().GetValue() - df.Filter("acc_weight<0").Sum("acc_weight").GetValue();
        auto mc_events = dfmc.Filter("fabs(hybrid_combo)>0").Count().GetValue() - df.Filter("acc_weight<0").Sum("acc_weight").GetValue();
        
        cout << "Entries in (" << str_enMin << "," << str_enMax << "): "
             << data_events  << "\t"
             << mc_events << "\t"
                    << dfT.Count().GetValue() << endl;
                    
        //Save values in text file
        fprintf(enStatsF, "%f\t %f\t %f\t \n", vv_EnRanges[loc_E][0], vv_EnRanges[loc_E][1], data_events);
        fprintf(enStatsF_MC, "%f\t %f\t %f\n", vv_EnRanges[loc_E][0], vv_EnRanges[loc_E][1], mc_events);   
        fprintf(enStatsF_Thrown, "%f\t %f\t %llu\n", vv_EnRanges[loc_E][0], vv_EnRanges[loc_E][1], dfT.Count().GetValue());
        //Save sub trees for energy bin
        df.Snapshot("flatTree_kpkpxim", (outputDir+root_file_name+"_Emin-"+str_enMin+"_Emax-"+str_enMax+".root").c_str(), branches);
        dfmc.Snapshot("flatTree_kpkpxim", (outputDir+"recon_"+root_file_name+"_Emin-"+str_enMin+"_Emax-"+str_enMax+".root").c_str(), branchesMC);
        dfT.Snapshot("flatTree_thrown_kpkpxim", (outputDir+"thrown_"+root_file_name+"_Emin-"+str_enMin+"_Emax-"+str_enMax+".root").c_str(),{"beam_E","t_dist"});
        //Get trees for Ebin and trange
        FILE *entStatsF = fopen((logDir+root_file_name+"_Emin-"+str_enMin+"_Emax-"+str_enMax+"_TBins_Yields.txt").c_str(),"w+");
        FILE *entStatsF_MC = fopen((logDir+"recon_"+root_file_name+"_Emin-"+str_enMin+"_Emax-"+str_enMax+"_TBins_Yields.txt").c_str(),"w+");
        FILE *entStatsF_Thrown = fopen((logDir+"thrown_"+root_file_name+"_Emin-"+str_enMin+"_Emax-"+str_enMax+"_TBins_Yields.txt").c_str(),"w+");
        //
        for(Int_t loc_T=0; loc_T < vv_TRanges.size(); loc_T++)
          {
            string str_tMin = to_string(vv_TRanges[loc_T][0]);
            string str_tMax = to_string(vv_TRanges[loc_T][1]);
            str_tMin = str_tMin.substr(0,str_tMin.find_last_of('.')+3);
            str_tMax = str_tMax.substr(0,str_tMax.find_last_of('.')+3);
            //Make the sub trees for t-bins
            auto df1 = df.Filter( ("t_dist>="+str_tMin+"&&t_dist<"+str_tMax).c_str());
            auto dfmc1 = dfmc.Filter( ("t_dist>="+str_tMin+"&&t_dist<"+str_tMax).c_str());
            auto dft1 = dfT.Filter( ("t_dist>="+str_tMin+"&&t_dist<"+str_tMax).c_str());

            //Get Value of events after cut
            auto data_events_t = df1.Filter("decayxim_M<1.334 && decayxim_M>1.308 && fabs( hybrid_combo)>0").Count().GetValue() - df1.Filter("acc_weight<0").Sum("acc_weight").GetValue();
            auto mc_events_t = dfmc1.Filter("fabs( hybrid_combo)>0").Count().GetValue() - dfmc1.Filter("acc_weight<0").Sum("acc_weight").GetValue();

            cout << ">>>>>> Entries in (" << str_tMin << "," << str_tMax << "): "
                 << data_events_t  << "\t"
                 << mc_events_t << "\t"
                 << dft1.Count().GetValue() << endl;
            
            //Save yields to text file
            fprintf(entStatsF, "%f\t %f\t %f\t \n", vv_TRanges[loc_T][0], vv_TRanges[loc_T][1], data_events_t);
            fprintf(entStatsF_MC, "%f\t %f\t %f\n", vv_TRanges[loc_T][0], vv_TRanges[loc_T][1], mc_events_t);
            fprintf(entStatsF_Thrown, "%f\t %f\t %llu\n", vv_TRanges[loc_T][0], vv_TRanges[loc_T][1], dft1.Count().GetValue());         
            // //Save sub trees for energy bin
            df1.Snapshot("flatTree_kpkpxim", (outputDir+root_file_name+"_Emin-"+str_enMin+"_Emax-"+str_enMax+"_Tmin-"+str_tMin+"_Tmax-"+str_tMax+".root").c_str(), branches);
            dfmc1.Snapshot("flatTree_kpkpxim", (outputDir+"recon_"+root_file_name+"_Emin-"+str_enMin+"_Emax-"+str_enMax+"_Tmin-"+str_tMin+"_Tmax-"+str_tMax+".root").c_str(), branchesMC);
            dft1.Snapshot("flatTree_thrown_kpkpxim", (outputDir+"thrown_"+root_file_name+"_Emin-"+str_enMin+"_Emax-"+str_enMax+"_Tmin-"+str_tMin+"_Tmax-"+str_tMax+".root").c_str(), {"beam_E","t_dist"});
          }
        fclose(entStatsF); fclose(entStatsF_MC); fclose(entStatsF_Thrown);
      }
    fclose(enStatsF); fclose(enStatsF_MC); fclose(enStatsF_Thrown);
}
