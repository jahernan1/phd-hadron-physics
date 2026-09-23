#ifndef GXANA_COMMON_STRINGS_H
#define GXANA_COMMON_STRINGS_H

#include <string>

namespace gxana {

// Orders file names by the integer part of the first number after "emin"
// (e.g. diffxsec_emin_7.40 < diffxsec_emin_10.18). Legacy behavior kept:
// names with equal integer parts compare equal, and a name without "emin"
// throws std::out_of_range.
bool NumericCompare(const std::string& a, const std::string& b);

} // namespace gxana

#endif
