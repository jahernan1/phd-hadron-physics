#include "gxana/common/GraphIO.h"
#include "gxana/common/Strings.h"

#include <TFile.h>
#include <TGraphErrors.h>
#include <TKey.h>
#include <TSystem.h>

#include <algorithm>
#include <cstring>
#include <fnmatch.h>
#include <iostream>
#include <memory>

namespace gxana {

std::vector<TGraphErrors*> GetAllTGraphErrors(const char* fileName) {
    // Create a vector to store the TGraphErrors objects
    std::vector<TGraphErrors*> graphs;

    // Open the ROOT file
    TFile* file = TFile::Open(fileName);
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return graphs; // Return empty vector if file can't be opened
    }

    // Loop over all keys in the file
    TIter nextKey(file->GetListOfKeys());
    TKey* key;

    while ((key = (TKey*)nextKey())) {
        // Get the class name of the object
        const char* className = key->GetClassName();

        // Check if the object is of type TGraphErrors
        if (strcmp(className, "TGraphErrors") == 0) {
            // Retrieve the object as a TGraphErrors
            TGraphErrors* graph = (TGraphErrors*)key->ReadObj();

            // Add the TGraphErrors to the vector
            graphs.push_back(graph);
        }
    }

    // Close the file
    file->Close();
    delete file;

    // Return the vector containing all TGraphErrors objects
    return graphs;
}

std::vector<TGraphErrors*> CreateTGraphErrorsFromTxt(const std::string& dir, const std::string& pattern,
                                                     const std::string& outFile)
{
    // Get list of all .txt files in the directory
    void *dirp = gSystem->OpenDirectory(dir.c_str());
    if (!dirp) {
        std::cerr << "Error: Cannot open directory " << dir << std::endl;
        return {};
    }

    const char *entry;
    std::vector<std::string> txtFiles;
    while ((entry = gSystem->GetDirEntry(dirp))) {
        std::string filename = entry;
        // Check for ".txt" extension and optionally filter by pattern
        if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".txt" &&
            fnmatch(pattern.c_str(), filename.c_str(), 0) == 0)
            {
                txtFiles.push_back(dir + "/" + filename);
                std::cout << "Entry: " << txtFiles.back() << std::endl;
            }
    }

    gSystem->FreeDirectory(dirp);
    if (txtFiles.empty()) {
        std::cerr << "No .txt files found in directory " << dir
                  << " with pattern: " << pattern << std::endl;
        return {};
    }

    // sort the vector of file names to endure proper plotting
    std::sort(txtFiles.begin(), txtFiles.end(), NumericCompare);

    std::unique_ptr<TFile> rootFile;
    if (!outFile.empty()) {
        rootFile.reset(TFile::Open(outFile.c_str(), "RECREATE"));
        if (!rootFile || !rootFile->IsOpen()) {
            std::cerr << "Error: Cannot create output file " << outFile << std::endl;
            return {};
        }
    }

    // Vector to store the TGraphErrors
    std::vector<TGraphErrors*> graphs;
    std::string enMin, enMax;

    // Process each .txt file
    for (const auto& file : txtFiles) {
        // Extract base name without path and extension
        std::string fileName = file.substr(file.find_last_of("/") + 1);
        fileName = fileName.substr(0, fileName.find_last_of("."));
        // Get enbins for plot title
        if(fileName.find("emin") != std::string::npos){
            enMin = fileName.substr(fileName.find("emin")+5);
            enMin = enMin.substr(0,enMin.find_first_of("_"));
            enMax = fileName.substr(fileName.find("emax")+5);
            enMax = enMax.substr(0,enMax.find_first_of("_"));
        }
        // Create a TGraphErrors from the file
        TGraphErrors *graph = new TGraphErrors(file.c_str());
        graph->SetTitle( ("#bf{E_{#gamma} (GeV): (" + enMin + ", " + enMax + ")}").c_str());
        if (!graph || graph->GetN() == 0) {
            std::cerr << "Warning: Failed to create graph from file: " << file << std::endl;
            delete graph;
            continue;
        }

        // Set graph name and write it to the ROOT file
        graph->SetName(("Graph_" + fileName).c_str());
        if (rootFile)
            rootFile->WriteObject(graph, graph->GetName());
        graphs.push_back(graph);

        std::cout << "Processed file: " << fileName << std::endl;
    }

    return graphs;
}

} // namespace gxana
