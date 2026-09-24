// FitFunctions.h
#ifndef FITFUNCTIONS_H
#define FITFUNCTIONS_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include "TROOT.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TKey.h"
#include "ROOT/RDataFrame.hxx"
#include "RooMsgService.h"
#include "RooWorkspace.h"
#include "RooDataSet.h"
#include "RooPlot.h"
#include "RooFit.h"
#include "RooHist.h"
#include "RooFitResult.h"
#include "RooRealVar.h"
#include "RooAbsReal.h"
#include "RooAbsPdf.h"
#include "RooAbsCollection.h"
#include "RooGlobalFunc.h"

void RooFitMC(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err, std::string fitType, std::unordered_map<std::string,std::vector<double>> &params, std::string hist_weight="hybrid_combo", int max_retries = 10);

void RooFitData(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err, std::string fitType, std::unordered_map<std::string,std::vector<double>> &params, int chebyOrder=2, std::string weight_name="hybrid_combo", int max_retries = 10);

bool AttemptFitMC(RooWorkspace* w, RooDataSet* data, std::unordered_map<std::string,std::vector<double>> &params);

bool AttemptFit(RooWorkspace* w, RooDataSet* data, std::unordered_map<std::string,std::vector<double>> &params, double lowerBound, double upperBound);

std::string constructFitString(const std::string& fitType, const std::unordered_map<std::string, std::vector<double>>& params);

std::string constructFitStringData(const std::string& fitType, const std::unordered_map<std::string, std::vector<double>>& params);
void setStyle();

void GetDiffXSecFile
(
 std::vector<TTree*> trees, TH1D* flux, std::vector<std::string> delim,
 std::string fitType, std::unordered_map<std::string, std::vector<double>>& xiParamRange, 
 std::ofstream& outputFile, std::ofstream& xsecFile,
 std::string weight="hybrid_combo", int chebyOrder=2, int n_threads = 4);

void GetTotXSecFile
(
 std::vector<TTree*> trees, TH1D* flux, std::vector<std::string> delim,
 std::string fitType, std::unordered_map<std::string, std::vector<double>>& xiParamRange, 
 std::ofstream& outputFile, std::ofstream& xsecFile,
 std::string weight="hybrid_combo", int chebyOrder=2, int n_threads = 4);

TH1D* GetFluxHist(std::string filename);

#endif // FITFUNCTIONS_H
