#ifndef GXANA_COMMON_STRINGS_H
#define GXANA_COMMON_STRINGS_H

#include <string>

namespace gxana {

// Orders file names by the full numeric value after "emin" (e.g.
// diffxsec_emin_7.40 < diffxsec_emin_7.86 < diffxsec_emin_10.18), then by name,
// so the order is total. A name without "emin" throws std::out_of_range.
bool NumericCompare(const std::string& a, const std::string& b);

} // namespace gxana

#endif
