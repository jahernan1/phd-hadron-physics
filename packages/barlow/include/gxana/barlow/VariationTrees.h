#ifndef GXANA_BARLOW_VARIATIONTREES_H
#define GXANA_BARLOW_VARIATIONTREES_H

#include <string>
#include <utility>
#include <vector>

namespace gxana {
namespace barlow {

// One cut variation: output tree name (vary_<id>; MC gets _mc) and its full cut.
struct Variation {
    std::string tree;
    std::string cut;
};

// Port of the snapshot half of GetVariationTreesUML.C GetPrepedFlatTree: data and
// MC dataframes get the defines, then their filters, then each variation's cut, and
// the --branch columns are written to `out` (first tree RECREATE, the rest UPDATE,
// so a rerun replaces the file; legacy used UPDATE only and appended).
struct VariationTreesSpec {
    std::string tree, input, inputMC, out;
    std::vector<std::pair<std::string, std::string>> defines;
    std::vector<std::string> filtersData, filtersMC, branches;
    std::vector<Variation> variations;
    int threads = 0; // > 0: ROOT::EnableImplicitMT(threads), as legacy n_threads
};

void WriteVariationTrees(const VariationTreesSpec& spec);

// Port of the GetVariationTreesUML.C yield side check (lines 162-197, 228-411): per
// variation, fit the nominal MC then data trees and the variation MC then data trees
// (Johnson signal; Argus-like background for MC, 2nd-order Chebychev for data, the
// legacy ranges), print the yields and pct_diff = |nom - var| / nom * 100, and append
// NAME, id, nom, var, pct, nomMC, varMC, pctMC (tab separated) to `yields`. Fit PDFs
// go to fitDir/recon_NAME_<id>.pdf and fitDir/data_NAME_<id>.pdf. Nothing is enforced
// ("<10%" is printed as it was).
struct CheckSpec {
    std::string tree, out, nominal, nominalMC, name, weight, yields, fitDir;
    std::vector<Variation> variations;
};

void CheckVariationYields(const CheckSpec& spec);

// "name=expr" -> {name, expr}, split at the first '=' (cuts may contain "<=").
std::pair<std::string, std::string> SplitAssign(const std::string& text);

} // namespace barlow
} // namespace gxana

#endif
