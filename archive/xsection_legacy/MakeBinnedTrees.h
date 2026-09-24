// MakeBinnedTrees.h (Header File)
#ifndef MAKE_BINNED_TREES_H
#define MAKE_BINNED_TREES_H

#include <ROOT/RDataFrame.hxx>
#include <TFile.h>
#include <TTree.h>
#include <TObject.h>
#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <utility>

// Function declarations
void divideNominalIntoBins(const std::string& filePath,
                           const std::string& outputFilePath,
                           const std::vector<std::pair<double, double>>& enRange,
                           const std::vector<std::pair<double, double>>& tRange,
                           bool data = true);
void divideThrownIntoBins(const std::string& filePath,
                          const std::string& outputFilePath,
                          const std::vector<std::pair<double, double>>& enRange,
                          const std::vector<std::pair<double, double>>& tRange);

#endif // MAKE_BINNED_TREES_H
