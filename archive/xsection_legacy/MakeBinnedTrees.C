// main.cpp
#include "MakeBinnedTrees.h"

// Compile to an executable 
// g++ -o MakeBinnedTrees MakeBinnedTrees.cpp MakeBinnedTrees.C `root-config --cflags --libs`
//
 
int main()
{
    // Define the bins used for the xsection
    std::vector<std::pair<double, double>> enRange{{6.40,7.40},{7.40,7.86},{7.86,8.19},{8.19,8.45},{8.45,8.68},{8.68,9.26},{9.26,10.18},{10.18,11.4}};
    std::vector<std::pair<double, double>> tRange{{0.10,0.35},{0.35,0.53},{0.53,0.71},{0.71,0.92},{0.92,1.19},{1.19,1.53},{1.53,2.40}};
    // directory paths
    std::string treeDir = "/d/grid17/hjesse/AnalysisNote/flatTrees/";
    std::string qTreeDir = "/d/grid17/hjesse/AnalysisNote/QFactors/logs/";
    std::vector<std::string> treeNames = {"kpkpxim__M23_2017-01_ana45",
                                          "kpkpxim__M23_2017-01_ana56",
                                          "kpkpxim__B4_M23_2018-01_ana03",
                                          "kpkpxim__B4_M23_2018-08_ana02"
    };
    
    for(int i = 0 ; i < treeNames.size(); ++i){
        //Save binned nominal trees
        divideNominalIntoBins(qTreeDir+treeNames[i]+"_nominal_kphighrap_1111111/"+"postQVal_flatTree_"+treeNames[i]+"_nominal_kphighrap_1111111.root",
                              treeDir+"binned_flatTree_"+treeNames[i]+"_nominal_kphighrap.root",
                              enRange, tRange);
        divideNominalIntoBins(treeDir+"flatTree_"+treeNames[i]+"_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root",
                              treeDir+"binned_flatTree_"+treeNames[i]+"_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root",
                              enRange, tRange, false);
        
        divideThrownIntoBins(treeDir+"flatTree_thrown_"+treeNames[i]+"_gen_amp_V2_ac_YstarRest.root",
                             treeDir+"binned_thrown_flatTree_"+treeNames[i]+"_gen_amp_V2_ac_YstarRest.root",
                             enRange, tRange);//thrown
        
        //Make diffxsec for one bin
        //Save binned nominal trees
        divideNominalIntoBins(qTreeDir+treeNames[i]+"_nominal_kphighrap_1111111/"+"postQVal_flatTree_"+treeNames[i]+"_nominal_kphighrap_1111111.root",
                              treeDir+"enBin_flatTree_"+treeNames[i]+"_nominal_kphighrap.root",
                              {{6.4,11.4}}, tRange);
        divideNominalIntoBins(treeDir+"flatTree_"+treeNames[i]+"_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root",
                              treeDir+"enBin_flatTree_"+treeNames[i]+"_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root",
                              {{6.4,11.4}}, tRange, false);
        
        divideThrownIntoBins(treeDir+"flatTree_thrown_"+treeNames[i]+"_gen_amp_V2_ac_YstarRest.root",
                             treeDir+"enBin_thrown_flatTree_"+treeNames[i]+"_gen_amp_V2_ac_YstarRest.root",
                             {{6.4,11.4}}, tRange);//thrown

        
        //run files for one beam bunch in 2018 data
        if(i>1)
            {
                divideNominalIntoBins(qTreeDir+treeNames[i]+"_oneRfBunch_nominal_kphighrap_1111111/"+"postQVal_flatTree_"+treeNames[i]+"_oneRfBunch_nominal_kphighrap_1111111.root",
                                       treeDir+"enBin_flatTree_"+treeNames[i]+"_oneRfBunch_nominal_kphighrap.root",
                                      {{6.4,11.4}}, tRange);

                divideNominalIntoBins(treeDir+"flatTree_"+treeNames[i]+"_gen_amp_V2_ac_YstarRest_oneRfBunch_nominal_kphighrap.root",
                                      treeDir+"enBin_flatTree_"+treeNames[i]+"_gen_amp_V2_ac_YstarRest_oneRfBunch_nominal_kphighrap.root",
                                      {{6.4,11.4}}, tRange, false);
            }
    }
    
    return 0;
}
