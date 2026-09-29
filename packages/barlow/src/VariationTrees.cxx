#include "gxana/barlow/VariationTrees.h"

#include <ROOT/RDataFrame.hxx>
#include <TROOT.h>
#include <TSystem.h>

#include <stdexcept>

namespace gxana {
namespace barlow {

std::pair<std::string, std::string> SplitAssign(const std::string& text)
{
    const auto eq = text.find('=');
    if (eq == std::string::npos || eq == 0)
        throw std::invalid_argument("expected NAME=VALUE: '" + text + "'");
    return {text.substr(0, eq), text.substr(eq + 1)};
}

namespace {

ROOT::RDF::RNode Prepare(ROOT::RDF::RNode node, const std::vector<std::pair<std::string, std::string>>& defines,
                         const std::vector<std::string>& filters)
{
    for (const auto& define : defines)
        node = node.Define(define.first, define.second);
    for (const auto& filter : filters)
        node = node.Filter(filter);
    return node;
}

} // namespace

void WriteVariationTrees(const VariationTreesSpec& spec)
{
    for (const auto& path : {spec.input, spec.inputMC})
        if (gSystem->AccessPathName(path.c_str()))
            throw std::runtime_error("cannot open " + path);
    if (spec.threads > 0)
        ROOT::EnableImplicitMT(spec.threads);
    const std::string dir = gSystem->GetDirName(spec.out.c_str()).Data();
    if (!dir.empty())
        gSystem->mkdir(dir.c_str(), true);

    ROOT::RDataFrame dfData(spec.tree, spec.input);
    ROOT::RDataFrame dfMC(spec.tree, spec.inputMC);
    auto data = Prepare(dfData, spec.defines, spec.filtersData);
    auto mc = Prepare(dfMC, spec.defines, spec.filtersMC);

    ROOT::RDF::RSnapshotOptions opts;
    opts.fOverwriteIfExists = true;
    bool first = true;
    for (const auto& variation : spec.variations) {
        opts.fMode = first ? "RECREATE" : "UPDATE";
        first = false;
        data.Filter(variation.cut, "cutset").Snapshot(variation.tree, spec.out, spec.branches, opts);
        opts.fMode = "UPDATE";
        mc.Filter(variation.cut, "cutset").Snapshot(variation.tree + "_mc", spec.out, spec.branches, opts);
    }
}

} // namespace barlow
} // namespace gxana
