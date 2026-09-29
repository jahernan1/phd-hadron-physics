// Argument parsing shared by the gxana_xsec_* executables (header-only).
#ifndef GXANA_XSECTION_CLIARGS_H
#define GXANA_XSECTION_CLIARGS_H

#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gxana {
namespace cli {

inline std::vector<std::string> Split(const std::string& text, char sep)
{
    std::vector<std::string> parts;
    std::string part;
    std::istringstream in(text);
    while (std::getline(in, part, sep))
        parts.push_back(part);
    if (!text.empty() && text.back() == sep)
        parts.push_back("");
    return parts;
}

inline double ParseDouble(const std::string& text)
{
    size_t used = 0;
    double value = 0;
    try {
        value = std::stod(text, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (text.empty() || used != text.size())
        throw std::invalid_argument("not a number: '" + text + "'");
    return value;
}

// "6.4,7.4,11.4" -> {6.4, 7.4, 11.4}
inline std::vector<double> ParseDoubleList(const std::string& text)
{
    std::vector<double> values;
    for (const auto& part : Split(text, ','))
        values.push_back(ParseDouble(part));
    return values;
}

// "mu=1.3217,1.31,1.33" -> {"mu", {1.3217, 1.31, 1.33}}  (initial value, min, max)
inline std::pair<std::string, std::vector<double>> ParseParam(const std::string& text)
{
    const auto eq = text.find('=');
    if (eq == std::string::npos || eq == 0)
        throw std::invalid_argument("expected NAME=INIT,MIN,MAX: '" + text + "'");
    auto values = ParseDoubleList(text.substr(eq + 1));
    if (values.size() != 3)
        throw std::invalid_argument("expected NAME=INIT,MIN,MAX: '" + text + "'");
    return {text.substr(0, eq), values};
}

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
