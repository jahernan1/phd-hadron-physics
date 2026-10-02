#include "gxana/common/Strings.h"

#include <cctype>
#include <stdexcept>
#include <string>

namespace gxana {

// Orders bin tables by the full emin value. The legacy body (AnalysisNote/xsection/
// PlotFunctions.cpp:82-100) compared only the integer part, so the configured edges
// 7.40/7.86 and 8.19/8.45/8.68 tied and their panels followed the directory listing and
// the C++ library. Equal values fall back to the name, so the order is total.
// A name without "emin" throws std::out_of_range, as before.
bool NumericCompare(const std::string &a, const std::string &b) {
    auto emin = [](const std::string &str) -> double {
        const std::string sub = str.substr(str.find("emin"));
        for (size_t i = 0; i < sub.size(); ++i) {
            if (isdigit(static_cast<unsigned char>(sub[i]))) return std::stod(sub.substr(i));
        }
        return 0; // no number found
    };
    const double numA = emin(a);
    const double numB = emin(b);
    if (numA != numB) return numA < numB;
    return a < b;
}

} // namespace gxana
