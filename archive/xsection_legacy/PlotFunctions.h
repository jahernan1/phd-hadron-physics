#ifndef PLOTDIFFXSEC_H
#define PLOTDIFFXSEC_H

#include <TGraphErrors.h>
#include <TFile.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TAxis.h>
#include <TSystem.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <regex>
#include <string>
#include <fnmatch.h> // For wildcard matching
#include <TLatex.h>
#include <TGaxis.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TLegend.h>

// Sets the ROOT style
void SetStyle();

// Creates TGraphErrors objects from text files in a directory
std::vector<TGraphErrors*> CreateTGraphErrorsFromTxt(std::string dir , const std::string &pattern);

// Custom comparison for numeric sorting
bool NumericCompare(const std::string &a, const std::string &b);
    
// Plots multiple differential cross-sections together
void plotDiffXSec(std::vector<std::vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, std::string saveName);

void plotOneWeightedXSec(std::vector<TGraphErrors*> arrGraphs, double xmax, double ymax, std::string saveName);
    
// Plots weighted differential cross-sections
void plotWeightedXSec(std::vector<TGraphErrors*> arrGraphs, double xmax, double ymax, std::string saveName);

void plotFinalWeightedXSec(std::vector<std::vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, std::string saveName);

#endif // PLOTDIFFXSEC_H
