#include "gxana/common/Paths.h"

#include <cstdlib>
#include <stdexcept>

namespace gxana {

std::string EnvPath(const std::string& var, const std::string& rel)
{
    const char* value = std::getenv(var.c_str());
    if (value == nullptr || *value == '\0')
        throw std::runtime_error(var + " is not set; run `source env/setup.sh`");
    std::string path(value);
    if (rel.empty())
        return path;
    if (path.back() != '/')
        path += '/';
    return path + rel;
}

} // namespace gxana
