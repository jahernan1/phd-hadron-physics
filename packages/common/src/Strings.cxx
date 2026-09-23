#include "gxana/common/Strings.h"

#include <cctype>
#include <string>

namespace gxana {

// Body copied verbatim from AnalysisNote/xsection/PlotFunctions.cpp:82-100.
bool NumericCompare(const std::string &a, const std::string &b) {
    // Find first digit in each string
    auto findFirstNumber = [](const std::string &str) -> int {
        for (size_t i = 0; i < str.size(); ++i) {
            if (isdigit(str[i])) {
                return std::stoi(str.substr(i));
            }
        }
        return 0; // No number found, default to 0
    };

    std::string suba = a.substr(a.find("emin"));
    std::string subb = b.substr(b.find("emin"));
    
    int numA = findFirstNumber(suba);
    int numB = findFirstNumber(subb);

    return numA < numB;
}

} // namespace gxana
