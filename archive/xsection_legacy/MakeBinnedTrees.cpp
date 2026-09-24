// MakeBinnedTrees.cpp (Source File)
#include "MakeBinnedTrees.h"

void divideThrownIntoBins(const std::string& filePath,
                         const std::string& outputFilePath,
                         const std::vector<std::pair<double, double>>& enRange,
                         const std::vector<std::pair<double, double>>& tRange)
{
    std::string filename = filePath.substr(filePath.find_last_of("/")+1);
    std::string outputFilename = outputFilePath.substr(outputFilePath.find_last_of("/")+1);
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
        std::string strLowE = std::to_string(lowE);
        strLowE = strLowE.substr(0,strLowE.find(".")+3);
        std::string strHighE = std::to_string(highE);
        strHighE = strHighE.substr(0,strHighE.find(".")+3);
        std::string enFilter = "beam_E >= " + strLowE + " && beam_E < " + strHighE;

        auto df_enFiltered = df.Filter(enFilter);
        df_enFiltered.Snapshot("/emin_" + strLowE + "_emax_" + strHighE , outputFilePath, {"t_dist", "beam_E"}, opts);

        for (size_t j = 0; j < tRange.size(); ++j) {
            double lowT = tRange[j].first;
            double highT = tRange[j].second;
            std::string strLowT = std::to_string(lowT);
            strLowT = strLowT.substr(0,strLowT.find(".")+3);
            std::string strHighT = std::to_string(highT);
            strHighT = strHighT.substr(0,strHighT.find(".")+3);
            std::string tFilter = "t_dist >= " + strLowT + " && t_dist < " + strHighT;

            auto df_tFiltered = df_enFiltered.Filter(tFilter);
            df_tFiltered.Snapshot("/emin_" + strLowE + "_emax_" + strHighE + "_tmin_"+ strLowT + "_tmax_" + strHighT, outputFilePath, {"t_dist", "beam_E"}, opts);
        }
    }
    inputFile->Close();    
    std::cout << "Binned thrown data saved as: " << outputFilename << std::endl;
}

void divideNominalIntoBins(const std::string& filePath,
                           const std::string& outputFilePath,
                           const std::vector<std::pair<double, double>>& enRange,
                           const std::vector<std::pair<double, double>>& tRange,
                           bool data)
{
    std::string filename = filePath.substr(filePath.find_last_of("/")+1);
    std::string outputFilename = outputFilePath.substr(outputFilePath.find_last_of("/")+1);
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

    std::vector<std::string> branches =
        {"beam_E","chisqndf","total_mm2","xim_pathlensig","lambda_pathlensig",
         "kphigh_p4","kplow_p4", "beam_vertexZ",
         "hybrid_combo", "kphigh_prapidity","kplow_prapidity","total_mm2",
         "beam_E_Truth","beam_p4_truth","decayxim_M",
         "xim_costheta_gen_amp","xim_costheta_hf",
         "t_dist","t_dist_truth",
         "chisqndf","confidencelvl",
         "best_combo","best_combo_rf", "acc_weight",
         "xim_lifetime_restframe", "lambda_lifetime_restframe",
         "decayxim_p4","ystar_p4"}; 
    if(data)
        branches.push_back("qvalue_decayxim_M");

    ROOT::RDataFrame df("flatTree_kpkpxim", inputFile);
    std::cout << "Processing TFile: " << filename << std::endl;
    for (size_t i = 0; i < enRange.size(); i++) {
        double lowE = enRange[i].first;
        double highE = enRange[i].second;
        std::string strLowE = std::to_string(lowE);
        strLowE = strLowE.substr(0,strLowE.find(".")+3);
        std::string strHighE = std::to_string(highE);
        strHighE = strHighE.substr(0,strHighE.find(".")+3);
        std::string enFilter = "beam_E >= " + strLowE + " && beam_E < " + strHighE;

        auto df_enFiltered = df.Filter(enFilter)
            .Filter("t_dist<2.4");
        df_enFiltered.Snapshot("/emin_" + strLowE + "_emax_" + strHighE , outputFilePath, branches, opts);

        for (size_t j = 0; j < tRange.size(); ++j) {
            double lowT = tRange[j].first;
            double highT = tRange[j].second;
            std::string strLowT = std::to_string(lowT);
            strLowT = strLowT.substr(0,strLowT.find(".")+3);
            std::string strHighT = std::to_string(highT);
            strHighT = strHighT.substr(0,strHighT.find(".")+3);
            std::string tFilter = "t_dist >= " + strLowT + " && t_dist < " + strHighT;

            auto df_tFiltered = df_enFiltered.Filter(tFilter);
            df_tFiltered.Snapshot("/emin_" + strLowE + "_emax_" + strHighE + "_tmin_"+ strLowT + "_tmax_" + strHighT, outputFilePath, branches, opts);
        }
    }
    inputFile->Close();    
    std::cout << "Binned nominal data saved as: " << outputFilename << std::endl;
}
