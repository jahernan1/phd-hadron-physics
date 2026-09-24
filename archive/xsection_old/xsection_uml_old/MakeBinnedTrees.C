#include <ROOT/RDataFrame.hxx>
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <vector>
#include <string>

void divideNominalIntoBins(const std::string& filePath,
                           const std::string& outputFilePath,
                           const std::vector<std::pair<double, double>>& enRange,
                           const std::vector<std::pair<double, double>>& tRange);
void divideThrownIntoBins(const std::string& filePath,
                          const std::string& outputFilePath,
                          const std::vector<std::pair<double, double>>& enRange,
                          const std::vector<std::pair<double, double>>& tRange);

int MakeBinnedTrees()
{
    // Define the bins used for the xsection
    std::vector<std::pair<double, double>> enRange{{6.40,7.40},{7.40,7.86},{7.86,8.19},{8.19,8.45},{8.45,8.68},{8.68,9.26},{9.26,10.18},{10.18,11.4}};
    std::vector<std::pair<double, double>> tRange{{0.10,0.35},{0.35,0.53},{0.53,0.71},{0.71,0.92},{0.92,1.19},{1.19,1.53},{1.53,2.40}};
    // directory paths
    string treeDir = "/d/grid17/hjesse/AnalysisNote/flatTrees/";

    // Save binned nominal trees
    divideNominalIntoBins
        (treeDir+"flatTree_kpkpxim__M23_2017-01_ana56_nominal_rapidityCuts.root",
         treeDir+"binned_flatTree_kpkpxim__M23_2017-01_ana56_nominal_rapidityCuts.root",
         enRange, tRange);
    
    divideNominalIntoBins
        (treeDir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_rapidityCuts.root",
         treeDir+"binned_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_rapidityCuts.root",
         enRange, tRange);
    
    divideNominalIntoBins
        (treeDir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_rapidityCuts.root",
         treeDir+"binned_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_rapidityCuts.root",
         enRange, tRange);
        
    // Save binned thrown trees once bc they dont change for variations
    divideThrownIntoBins
        (treeDir+"flatTree_thrown_kpkpxim__M23_2017-01_ana56_gen_amp_V2_2D_ac.root",
         treeDir+"binned_thrown_flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_2D_ac.root",
         enRange, tRange);
    
    divideThrownIntoBins
        (treeDir+"flatTree_thrown_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_2D_ac.root",
         treeDir+"binned_thrown_flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_2D_ac.root",
         enRange, tRange);
    
    divideThrownIntoBins
        (treeDir+"flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_2D_ac.root",
         treeDir+"binned_thrown_flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_2D_ac.root",
         enRange, tRange);
       
    return 0;
}

void divideThrownIntoBins(const std::string& filePath,
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
    
    // Load each TTree using RDataFrame
    ROOT::RDataFrame df("flatTree_thrown_kpkpxim", inputFile);
    // Apply selections based on enRange and save each subset
    std::cout << "Processing TFile: " << filename << std::endl;
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
        df_enFiltered.Snapshot("/emin_" + strLowE + "_emax_" + strHighE , outputFilePath, {"t_dist", "beam_E"}, opts);
                
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
            df_tFiltered.Snapshot("/emin_" + strLowE + "_emax_" + strHighE + "_tmin_"+ strLowT + "_tmax_" + strHighT, outputFilePath, {"t_dist", "beam_E"}, opts);
        }
    }
    
    // Clean up
    inputFile->Close();    
    std::cout << "Binned thrown data saved as: " << outputFilename << std::endl;
}

void divideNominalIntoBins(const std::string& filePath,
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
    
    // Load each TTree using RDataFrame
    ROOT::RDataFrame df("flatTree_kpkpxim", inputFile);
    // Apply selections based on enRange and save each subset
    std::cout << "Processing TFile: " << filename << std::endl;
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
        df_enFiltered.Snapshot("/emin_" + strLowE + "_emax_" + strHighE , outputFilePath,"", opts);
                
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
            df_tFiltered.Snapshot("/emin_" + strLowE + "_emax_" + strHighE + "_tmin_"+ strLowT + "_tmax_" + strHighT, outputFilePath, "", opts);
        }
    }
    
    // Clean up
    inputFile->Close();    
    std::cout << "Binned nominal data saved as: " << outputFilename << std::endl;
}
