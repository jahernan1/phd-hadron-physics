// Argument parsing of the gxana_xsec_* executables (header-only); the generic parsers
// (Split, ParseDouble, ParseDoubleList, ParseParam, SplitAssign) are in gxana/common/Cli.h.
#ifndef GXANA_XSECTION_CLIARGS_H
#define GXANA_XSECTION_CLIARGS_H

#include "gxana/common/Cli.h"
#include "gxana/xsection/Physics.h"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace gxana {
namespace cli {

// label/chebyOrder/weight capture the --label/--cheby/--weight in effect when
// this JOB was parsed (gxana_xsec_tables: JOBs are order-sensitive to those
// options, spec D20).
struct XSecJob {
    std::string name, data, mc, thrown, flux;
    std::string label;
    int chebyOrder = 2;
    std::string weight; // event-weight branch; gxana_xsec_tables requires --weight before a JOB
};

// "NAME:DATA:MC:THROWN:FLUX" (paths must not contain ':')
inline XSecJob ParseJob(const std::string& text, const std::string& label = "", int chebyOrder = 2,
                        const std::string& weight = "")
{
    const auto parts = Split(text, ':');
    if (parts.size() != 5)
        throw std::invalid_argument("expected NAME:DATA:MC:THROWN:FLUX: '" + text + "'");
    for (const auto& part : parts)
        if (part.empty())
            throw std::invalid_argument("empty field in job '" + text + "'");
    return {parts[0], parts[1], parts[2], parts[3], parts[4], label, chebyOrder, weight};
}

// --cheby only ever takes the literal background Chebychev order 1 or 2.
inline int ParseChebyOrder(const std::string& text)
{
    if (text == "1")
        return 1;
    if (text == "2")
        return 2;
    throw std::invalid_argument("--cheby must be 1 or 2: '" + text + "'");
}

// --fit JohnsonMCShape or JohnsonMCShapeSyst: exactly mu, lambda, gamma, delta, and --cheby 2 for every
// JOB (legacy MakeXSecFiles.C had no other background order).
inline void CheckMCShapeArgs(const std::unordered_map<std::string, std::vector<double>>& params,
                             const std::vector<XSecJob>& jobs)
{
    for (const char* name : {"mu", "lambda", "gamma", "delta"})
        if (!params.count(name))
            throw std::invalid_argument(std::string("JohnsonMCShape needs --param ") + name);
    if (params.size() != 4)
        throw std::invalid_argument("JohnsonMCShape takes only mu, lambda, gamma, delta");
    for (const auto& job : jobs)
        if (job.chebyOrder != 2)
            throw std::invalid_argument("JohnsonMCShape needs --cheby 2 (label " + job.label + ")");
}

// --br VALUE,ERROR
inline void ParseBranchingRatio(const std::string& text, double& value, double& error)
{
    const auto values = ParseDoubleList(text);
    if (values.size() != 2)
        throw std::invalid_argument("--br takes VALUE,ERROR: '" + text + "'");
    value = values[0];
    error = values[1];
}

// --target ZMIN,ZMAX,DENSITY,MOLAR_MASS,ATOMS (cm, cm, g/cm^3, g/mol, atoms per molecule)
inline xsec::Target ParseTarget(const std::string& text)
{
    const auto v = ParseDoubleList(text);
    if (v.size() != 5)
        throw std::invalid_argument("--target takes ZMIN,ZMAX,DENSITY,MOLAR_MASS,ATOMS: '" + text + "'");
    if (!(v[0] < v[1]))
        throw std::invalid_argument("--target: ZMIN must be below ZMAX: '" + text + "'");
    return {v[0], v[1], v[2], v[3], v[4]};
}

// --mass-window NAME=VALUE (GeV) sets one field of `windows`; returns NAME.
inline std::string SetMassWindow(xsec::MassWindows& windows, const std::string& text)
{
    const auto assign = SplitAssign(text);
    const double value = ParseDouble(assign.second);
    const std::string& name = assign.first;
    if (name == "lo") windows.lo = value;
    else if (name == "mc_hi") windows.mcHi = value;
    else if (name == "mc_signal_hi") windows.mcSignalHi = value;
    else if (name == "mc_plot_hi") windows.mcPlotHi = value;
    else if (name == "data_hi") windows.dataHi = value;
    else if (name == "data_edge") windows.dataEdge = value;
    else if (name == "mcpdf_data_lo") windows.mcPdfDataLo = value;
    else
        throw std::invalid_argument("unknown --mass-window " + name + " (lo, mc_hi, mc_signal_hi, mc_plot_hi, "
                                    "data_hi, data_edge, mcpdf_data_lo)");
    return name;
}

// --qvalue-branch BRANCH, or "none" for a channel without Q-factors ("").
inline std::string ParseQValueBranch(const std::string& text)
{
    return text == "none" ? "" : text;
}

// Directory one JOB's tables go to: <out>/<label>/, as legacy
// MakeXSecFitVariations.C wrote data/<variation>/ per label.
inline std::string LabelOutDir(const std::string& outDir, const std::string& label)
{
    std::string dir = outDir;
    if (!dir.empty() && dir.back() != '/')
        dir += '/';
    return dir + label;
}

} // namespace cli
} // namespace gxana

#endif
