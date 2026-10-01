// Argument parsing of the gxana_xsec_* executables (header-only); the generic parsers
// (Split, ParseDouble, ParseDoubleList, ParseParam, SplitAssign) are in gxana/common/Cli.h.
#ifndef GXANA_XSECTION_CLIARGS_H
#define GXANA_XSECTION_CLIARGS_H

#include "gxana/common/Cli.h"

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
    std::string weight = "hybrid_combo";
};

// "NAME:DATA:MC:THROWN:FLUX" (paths must not contain ':')
inline XSecJob ParseJob(const std::string& text, const std::string& label = "", int chebyOrder = 2,
                        const std::string& weight = "hybrid_combo")
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
