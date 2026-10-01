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

    // gxana: the comparison macros read the graphs in key order (one panel and
    // one fit_variations_stats.txt block per energy bin), so write them in
    // ascending emin, not in directory-listing order (filesystem dependent).
    std::vector<std::string> files;
    const char* fileName;
    while ((fileName = gSystem->GetDirEntry(dir))) {
        std::string fullPath = directory + "/" + fileName;
        // Check if the entry is a file and ends with ".txt"
        if (gSystem->AccessPathName(fullPath.c_str()) == false && fullPath.substr(fullPath.size() - 4) == ".txt" && fullPath.find("diffxsec")!=std::string::npos)
            files.push_back(fileName);
    }
    gSystem->FreeDirectory(dir);
    auto binEdge = [](const std::string& f, const std::string& key) {
        size_t pos = f.find(key);
        return pos == std::string::npos ? std::string() : f.substr(pos + key.size(), f.find('_', pos + key.size()) - pos - key.size());
    };
    std::sort(files.begin(), files.end(), [&](const std::string& a, const std::string& b) {
        std::string ea = binEdge(a, "emin_"), eb = binEdge(b, "emin_");
        if (!ea.empty() && !eb.empty() && ea != eb) return std::stod(ea) < std::stod(eb);
        return a < b;
    });

    for (const auto& file : files) {
        std::string fullPath = directory + "/" + file;
        std::string name = file.substr(0, file.find_last_of("."));
        TGraphErrors* graph = new TGraphErrors(fullPath.c_str());
        std::string enMin = binEdge(name, "emin_"), enMax = binEdge(name + "_", "emax_");
        // gxana: the panel title of the dissertation figures
        if (!enMin.empty() && !enMax.empty())
            graph->SetTitle(("#bf{E_{#gamma} (GeV): (" + enMin + ", " + enMax + ")}").c_str());
        graph->Write(name.c_str(),TObject::kOverwrite);
        delete graph;
    }

    outputFile->Close();
    std::cout << "ROOT file " << outputFileName << " created successfully!" << std::endl;
}

// Main macro. No arguments: the legacy label set, each to
// weighted_data/<label>/weighted_diffxsec.root. With labelDir and outputFile
// (gxana run systematics fit-study call): one label directory to outputFile.
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
