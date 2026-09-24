#include "gxana/common/Paths.h"
#include <ROOT/RDataFrame.hxx>
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <vector>
#include <string>

void divideDataIntoBins(const std::string& filename,
                        const std::string& outputFilename,
                        const std::vector<std::pair<double, double>>& enRange,
                        const std::vector<std::pair<double, double>>& tRange);

int SplitVariationTrees()
{
    // Define the bins used for the xsection
    std::vector<std::pair<double, double>> enRange{{6.40,7.40},{7.40,7.86},{7.86,8.19},{8.19,8.45},{8.45,8.68},{8.68,9.26},{9.26,10.18},{10.18,11.4}};
    std::vector<std::pair<double, double>> tRange{{0.10,0.35},{0.35,0.53},{0.53,0.71},{0.71,0.92},{0.92,1.19},{1.19,1.53},{1.53,2.40}};
    // variation names to loop over
    std::vector<std::string> cuts = {"chisqndf",
                                     "total_mm2_abs",
                                     "xim_pathlensig",
                                     "lambda_pathlensig",
                                     "kphigh_prap",
                                     //"kplow_prap"
    };
    string saveDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/variation_trees/");
    string treeDir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
    
    // Loop over variation files to get binned trees
    for(int i = 0; i<cuts.size(); ++i)
        {
            divideDataIntoBins
            (saveDir+"flatTree_kpkpxim__M23_2017-01_ana56_"+cuts[i]+"_gen_amp_V2_ac_YstarRest_variations.root",
             saveDir+"binned_flatTree_kpkpxim__M23_2017-01_ana56_"+cuts[i]+"_gen_amp_V2_ac_YstarRest_variations.root",
             enRange, tRange);

            divideDataIntoBins
            (saveDir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_"+cuts[i]+"_gen_amp_V2_ac_YstarRest_variations.root",
             saveDir+"binned_flatTree_kpkpxim__B4_M23_2018-01_ana03_"+cuts[i]+"_gen_amp_V2_ac_YstarRest_variations.root",
             enRange, tRange);
            
            divideDataIntoBins
            (saveDir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_"+cuts[i]+"_gen_amp_V2_ac_YstarRest_variations.root",
             saveDir+"binned_flatTree_kpkpxim__B4_M23_2018-08_ana02_"+cuts[i]+"_gen_amp_V2_ac_YstarRest_variations.root",
             enRange, tRange);            
        }

    return 0;
}

void divideDataIntoBins(const std::string& filePath,
                               const std::string& outputFilePath,
                               const std::vector<std::pair<double, double>>& enRange,
                               const std::vector<std::pair<double, double>>& tRange)
{
    string filename = filePath.substr(filePath.find_last_of("/")+1);
    string outputFilename = outputFilePath.substr(outputFilePath.find_last_of("/")+1);
    // Open the input file
    TFile *inputFile = TFile::Open(filePath.c_str(), "READ");
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }
    // Open the output file to reset values
    TFile *outputFile = new TFile(outputFilePath.c_str(), "RECREATE");
    outputFile->Close();
    
    // Set up the snapshot parameters to save all bins in same tree
    ROOT::RDF::RSnapshotOptions opts;
    opts.fMode = "UPDATE";
	opts.fOverwriteIfExists=true;    
    
    // Loop over all keys in the file and find TTrees
    std::cout << "Processing TFile: " << filename << std::endl;
    TIter next(inputFile->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)next())) {
        if (key->GetClassName() == std::string("TTree")) { // Check if the key is a TTree
            std::string treeName = key->GetName();
            std::cout << "  ++ Processing TTree: " << treeName << std::endl;
            
            // Load each TTree using RDataFrame
            ROOT::RDataFrame df(treeName, inputFile);

            // Apply selections based on enRange and save each subset
            for (size_t i = 0; i < enRange.size(); i++) {
                double lowE = enRange[i].first;
                double highE = enRange[i].second;
                string strLowE = to_string(lowE);
                strLowE = strLowE.substr(0,strLowE.find(".")+3);
                string strHighE = to_string(highE);
                strHighE = strHighE.substr(0,strHighE.find(".")+3);
                std::string enFilter = "beam_E >= " + strLowE + " && beam_E < " + strHighE;

                auto df_enFiltered = df.Filter(enFilter);
                //std::cout << "Saving TTree: " << "enBin_" + to_string(i) << std::endl;
                df_enFiltered.Snapshot(treeName + "/emin_" + strLowE + "_emax_" + strHighE , outputFilePath, "", opts);
                
                // Apply selections based on tRange and save each subset
                for (size_t j = 0; j < tRange.size(); ++j) {
                    double lowT = tRange[j].first;
                    double highT = tRange[j].second;
                    string strLowT = to_string(lowT);
                    strLowT = strLowT.substr(0,strLowT.find(".")+3);
                    string strHighT = to_string(highT);
                    strHighT = strHighT.substr(0,strHighT.find(".")+3);
                    std::string tFilter = "t_dist >= " + strLowT + " && t_dist < " + strHighT;
                    
                    auto df_tFiltered = df_enFiltered.Filter(tFilter);
                    //std::cout << "Saving TTree: " << "enBin_" + to_string(i) + "_tBin_" + to_string(j) << std::endl;
                    df_tFiltered.Snapshot(treeName + "/emin_" + strLowE + "_emax_" + strHighE + "_tmin_"+ strLowT + "_tmax_" + strHighT, outputFilePath, "", opts);
                }
            }
        }
    }
    
    // Clean up
    inputFile->Close();    
    std::cout << "Binned data Saved as: " << outputFilename << std::endl;
}
