// For every TTree in a file (recursing into directories), print:
//   "TREE <path> <entries> <sum beam_E> <sum t_dist>"
//   "BRANCHES <path> <deduped, sorted, comma-separated branch names>"
//   "SUMS <path> <sum decayxim_M|NA> <sum hybrid_combo|NA> <sum qvalue_decayxim_M|NA>"
// (golden binning comparison).
#include <ROOT/RDataFrame.hxx>
#include <TBranch.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TKey.h>
#include <TSystem.h>
#include <TTree.h>

#include <cstdio>
#include <memory>
#include <set>
#include <string>

namespace {

// TTree::Branch() does not guard against a repeated name (verified: calling it
// twice with the same name on ROOT 6.40 yields two TBranch objects both named
// "total_mm2"); newer RDataFrame::Snapshot added an explicit check that
// rejects a column list with a repeated name outright ("column X was passed
// to Snapshot twice"). NominalBranches() in Binning.cxx documents that the
// legacy branch list names total_mm2/chisqndf twice, so a legacy output tree
// (written by an older Snapshot lacking that check) can genuinely carry two
// TBranch objects with the same name; a std::set dedups that away, matching
// the single-instance list the port's deduped Snapshot column list produces.
std::set<std::string> BranchNames(TTree* tree)
{
    std::set<std::string> names;
    TIter next(tree->GetListOfBranches());
    while (auto* branch = static_cast<TBranch*>(next()))
        names.insert(branch->GetName());
    return names;
}

std::string Join(const std::set<std::string>& names)
{
    std::string joined;
    for (const auto& name : names) {
        if (!joined.empty())
            joined += ",";
        joined += name;
    }
    return joined;
}

} // namespace

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

            const auto branches = BranchNames(tree);
            std::printf("BRANCHES %s%s %s\n", prefix.c_str(), name.c_str(), Join(branches).c_str());

            std::printf("SUMS %s%s", prefix.c_str(), name.c_str());
            for (const char* col : {"decayxim_M", "hybrid_combo", "qvalue_decayxim_M"}) {
                if (branches.count(col)) {
                    auto sum = df.Sum(col);
                    std::printf(" %.17g", *sum);
                } else {
                    std::printf(" NA");
                }
            }
            std::printf("\n");
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
