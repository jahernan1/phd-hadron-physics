#include "gxana/common/Paths.h"
#include <filesystem>
void GetParamTree(string logDir);

namespace fs = std::filesystem;

int MakeParamTrees()
{
    GetParamTree("kpkpxim__M23_2017-01_ver45_nominal_1111111");
    
    return 0;
}

void GetParamTree(string logDir)
{
    string pathToFiles = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/");
    pathToFiles += logDir;
    
    for (const auto & entry : fs::directory_iterator(pathToFiles))
        std::cout << entry.path() << std::endl;
}
