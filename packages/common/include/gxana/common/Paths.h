#ifndef GXANA_COMMON_PATHS_H
#define GXANA_COMMON_PATHS_H

#include <string>

namespace gxana {

// $var joined with rel (e.g. EnvPath("GXANA_DATA", "flatTrees/x.root")).
// Throws std::runtime_error if var is unset or empty.
std::string EnvPath(const std::string& var, const std::string& rel = "");

} // namespace gxana

#endif
