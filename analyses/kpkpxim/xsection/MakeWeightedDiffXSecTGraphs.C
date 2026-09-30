#include "gxana/common/Paths.h"

void CreateRootFileFromTextFiles(const std::string& directory, const std::string& outputFileName) {
    void* dir = gSystem->OpenDirectory(directory.c_str());
    if (!dir) {
        std::cerr << "Error: Cannot open directory " << directory << std::endl;
        return;
    }

    // outputFileName is relative to directory unless it is an absolute path.
    std::string outputPath = (!outputFileName.empty() && outputFileName[0] == '/') ? outputFileName : directory+outputFileName;
    TFile* outputFile = TFile::Open(outputPath.c_str(), "RECREATE");
    if (!outputFile || outputFile->IsZombie()) {
        std::cerr << "Error: Cannot create ROOT file " << outputFileName << std::endl;
        return;
    }

    const char* fileName;
    while ((fileName = gSystem->GetDirEntry(dir))) {
        std::string fullPath = directory + "/" + fileName;
        std::string name = fullPath.substr(fullPath.find_last_of("/")+1);
        name = name.substr(0,name.find_last_of("."));
        
        // Check if the entry is a file and ends with ".txt"
        if (gSystem->AccessPathName(fullPath.c_str()) == false && fullPath.substr(fullPath.size() - 4) == ".txt" && fullPath.find("diffxsec")!=std::string::npos) {
            TGraphErrors* graph = new TGraphErrors(fullPath.c_str());
            if (graph) {
                graph->Write(name.c_str(),TObject::kOverwrite);
                //outputFile->WriteObject(graph,name.c_str());
                delete graph;
            }
        }
    }

    gSystem->FreeDirectory(dir);
    outputFile->Close();
    std::cout << "ROOT file " << outputFileName << " created successfully!" << std::endl;
}

// Main macro. No arguments: the legacy label set, each to
// weighted_data/<label>/weighted_diffxsec.root. With labelDir and outputFile
// (gxana run xsection --steps fitfigs): one label directory to outputFile.
int MakeWeightedDiffXSecTGraphs(const char* labelDir = "", const char* outputFile = "")
{
    if (std::string(labelDir) != "") {
        CreateRootFileFromTextFiles(labelDir, outputFile);
        return 0;
    }

    string filePath = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/weighted_data/");
    vector<string> accDir = {"acc_weight/","best_combo/","hybrid_combo/"};

    for(int i=0; i<accDir.size(); i++)
        CreateRootFileFromTextFiles(filePath+accDir[i], "weighted_diffxsec.root");

    CreateRootFileFromTextFiles(filePath+"qvalues/", "weighted_diffxsec.root");
    CreateRootFileFromTextFiles(filePath+"oneRfBunch/", "weighted_diffxsec.root");
        
    return 0;
}
