#include "gxana/xsection/Binning.h"

#include "gxana/common/BinNames.h"

#include <ROOT/RDataFrame.hxx>
#include <TFile.h>
#include <TKey.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace gxana {
namespace xsec {

namespace {

// Legacy MakeBinnedTrees.cpp branch list. It names total_mm2 and chisqndf
// twice; keep the first occurrence so the Snapshot column list does not
// depend on how a given ROOT version treats duplicates.
std::vector<std::string> NominalBranches(bool data)
{
    const std::vector<std::string> legacy =
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
    std::vector<std::string> branches;
    for (const auto& name : legacy)
        if (std::find(branches.begin(), branches.end(), name) == branches.end())
            branches.push_back(name);
    if (data)
        branches.push_back("qvalue_decayxim_M");
    return branches;
}

std::string EnergyFilter(double lowE, double highE)
{
    return "beam_E >= " + BinEdgeLabel(lowE) + " && beam_E < " + BinEdgeLabel(highE);
}

std::string TFilter(double lowT, double highT)
{
    return "t_dist >= " + BinEdgeLabel(lowT) + " && t_dist < " + BinEdgeLabel(highT);
}

std::string BaseName(const std::string& path)
{
    return path.substr(path.find_last_of("/") + 1);
}

// Open the input and recreate (reset) the output, as the legacy code did.
std::unique_ptr<TFile> OpenInput(const std::string& filePath, const std::string& outputFilePath)
{
    std::unique_ptr<TFile> inputFile(TFile::Open(filePath.c_str(), "READ"));
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return nullptr;
    }
    std::unique_ptr<TFile> outputFile(TFile::Open(outputFilePath.c_str(), "RECREATE"));
    if (!outputFile || outputFile->IsZombie()) {
        std::cerr << "Error creating file: " << outputFilePath << std::endl;
        return nullptr;
    }
    outputFile->Close();
    return inputFile;
}

// Save all bins in the same output file.
ROOT::RDF::RSnapshotOptions UpdateOptions()
{
    ROOT::RDF::RSnapshotOptions opts;
    opts.fMode = "UPDATE";
    opts.fOverwriteIfExists = true;
    return opts;
}

} // namespace

// The bin-name format lives in gxana/common/BinNames.h; these names stay for existing callers.
std::string BinEdgeLabel(double edge) { return gxana::BinEdgeLabel(edge); }

std::string EnergyBinName(double lowE, double highE) { return gxana::EnergyBinName(lowE, highE); }

std::string BinName(double lowE, double highE, double lowT, double highT)
{
    return gxana::BinName(lowE, highE, lowT, highT);
}

BinRanges EdgesToBins(const std::vector<double>& edges)
{
    if (edges.size() < 2)
        throw std::invalid_argument("EdgesToBins: need at least two edges");
    BinRanges bins;
    for (size_t i = 0; i + 1 < edges.size(); ++i) {
        if (!(edges[i] < edges[i + 1]))
            throw std::invalid_argument("EdgesToBins: edges must increase");
        bins.emplace_back(edges[i], edges[i + 1]);
    }
    return bins;
}

bool divideThrownIntoBins(const std::string& filePath, const std::string& outputFilePath,
                          const BinRanges& enRange, const BinRanges& tRange, const std::string& treeName)
{
    auto inputFile = OpenInput(filePath, outputFilePath);
    if (!inputFile)
        return false;
    const auto opts = UpdateOptions();
    const std::vector<std::string> branches{"t_dist", "beam_E"};

    ROOT::RDataFrame df(treeName, inputFile.get());
    std::cout << "Processing TFile: " << BaseName(filePath) << std::endl;
    for (const auto& en : enRange) {
        auto df_enFiltered = df.Filter(EnergyFilter(en.first, en.second));
        df_enFiltered.Snapshot("/" + EnergyBinName(en.first, en.second), outputFilePath, branches, opts);
        for (const auto& t : tRange) {
            auto df_tFiltered = df_enFiltered.Filter(TFilter(t.first, t.second));
            df_tFiltered.Snapshot("/" + BinName(en.first, en.second, t.first, t.second), outputFilePath,
                                  branches, opts);
        }
    }
    inputFile->Close();
    std::cout << "Binned thrown data saved as: " << BaseName(outputFilePath) << std::endl;
    return true;
}

bool divideNominalIntoBins(const std::string& filePath, const std::string& outputFilePath,
                           const BinRanges& enRange, const BinRanges& tRange, bool data,
                           const std::string& treeName)
{
    auto inputFile = OpenInput(filePath, outputFilePath);
    if (!inputFile)
        return false;
    const auto opts = UpdateOptions();
    const auto branches = NominalBranches(data);

    ROOT::RDataFrame df(treeName, inputFile.get());
    std::cout << "Processing TFile: " << BaseName(filePath) << std::endl;
    for (const auto& en : enRange) {
        // gxana: thesis total sigma used energy bins without the t cut (author decision 2026-09-24)
        auto df_enFiltered = df.Filter(EnergyFilter(en.first, en.second));
        df_enFiltered.Snapshot("/" + EnergyBinName(en.first, en.second), outputFilePath, branches, opts);
        for (const auto& t : tRange) {
            auto df_tFiltered = df_enFiltered.Filter(TFilter(t.first, t.second));
            df_tFiltered.Snapshot("/" + BinName(en.first, en.second, t.first, t.second), outputFilePath,
                                  branches, opts);
        }
    }
    inputFile->Close();
    std::cout << "Binned nominal data saved as: " << BaseName(outputFilePath) << std::endl;
    return true;
}

bool divideVariationTreesIntoBins(const std::string& filePath, const std::string& outputFilePath,
                                  const BinRanges& enRange, const BinRanges& tRange)
{
    auto inputFile = OpenInput(filePath, outputFilePath);
    if (!inputFile)
        return false;
    const auto opts = UpdateOptions();

    // Loop over all keys in the file and find TTrees
    std::cout << "Processing TFile: " << BaseName(filePath) << std::endl;
    TIter next(inputFile->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {
        if (key->GetClassName() != std::string("TTree"))
            continue;
        std::string treeName = key->GetName();
        std::cout << "  ++ Processing TTree: " << treeName << std::endl;

        ROOT::RDataFrame df(treeName, inputFile.get());
        for (const auto& en : enRange) {
            auto df_enFiltered = df.Filter(EnergyFilter(en.first, en.second));
            df_enFiltered.Snapshot(treeName + "/" + EnergyBinName(en.first, en.second), outputFilePath, "", opts);
            for (const auto& t : tRange) {
                auto df_tFiltered = df_enFiltered.Filter(TFilter(t.first, t.second));
                df_tFiltered.Snapshot(treeName + "/" + BinName(en.first, en.second, t.first, t.second),
                                      outputFilePath, "", opts);
            }
        }
    }
    inputFile->Close();
    std::cout << "Binned data Saved as: " << BaseName(outputFilePath) << std::endl;
    return true;
}

} // namespace xsec
} // namespace gxana
