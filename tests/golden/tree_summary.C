// Print "TREE <path> <entries> <sum beam_E> <sum t_dist>" for every TTree in a
// file, recursing into directories (golden binning comparison).
#include <ROOT/RDataFrame.hxx>
#include <TDirectory.h>
#include <TFile.h>
#include <TKey.h>
#include <TSystem.h>
#include <TTree.h>

#include <cstdio>
#include <memory>
#include <set>
#include <string>

void tree_summary_walk(TDirectory* dir, const std::string& prefix)
{
    std::set<std::string> seen;
    TIter next(dir->GetListOfKeys());
    while (auto* key = static_cast<TKey*>(next())) {
        const std::string name = key->GetName();
        if (!seen.insert(name).second)
            continue; // keys list the highest cycle first
        TObject* obj = key->ReadObj();
        if (auto* sub = dynamic_cast<TDirectory*>(obj)) {
            tree_summary_walk(sub, prefix + name + "/");
        } else if (auto* tree = dynamic_cast<TTree*>(obj)) {
            ROOT::RDataFrame df(*tree);
            auto sumE = df.Sum("beam_E");
            auto sumT = df.Sum("t_dist");
            std::printf("TREE %s%s %lld %.17g %.17g\n", prefix.c_str(), name.c_str(),
                        (long long)tree->GetEntries(), *sumE, *sumT);
        }
    }
}

void tree_summary(const char* path)
{
    std::unique_ptr<TFile> file(TFile::Open(path, "READ"));
    if (!file || file->IsZombie()) {
        std::printf("ERROR cannot open %s\n", path);
        gSystem->Exit(1);
    }
    tree_summary_walk(file.get(), "");
}
