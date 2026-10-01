// Command-line argument parsing shared by the gxana executables (header-only).
#ifndef GXANA_COMMON_CLI_H
#define GXANA_COMMON_CLI_H

#include <sstream>
#include <stdexcept>
#include <string>
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

// "name=expr" -> {name, expr}, split at the first '=' (cuts may contain "<=").
inline std::pair<std::string, std::string> SplitAssign(const std::string& text)
{
    const auto eq = text.find('=');
    if (eq == std::string::npos || eq == 0)
        throw std::invalid_argument("expected NAME=VALUE: '" + text + "'");
    return {text.substr(0, eq), text.substr(eq + 1)};
}

} // namespace cli
} // namespace gxana

#endif
