#include "gxana/xsection/XSec.h"

#include "gxana/common/BinNames.h"

#include <ROOT/RDataFrame.hxx>
#include <TFile.h>
#include <TKey.h>
#include <TDirectory.h>
#include <TSystem.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <math.h>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <tuple>

namespace gxana {
namespace xsec {

int LegacyFindBin(const TAxis* axis, double x)
{
    // Mirrors ROOT 6.24 TAxis::FindFixBin: `!(x < xmax)` (not `x >= xmax`) so
    // NaN falls into the overflow bin instead of reaching the fixed-bin
    // formula's static_cast<int>(NaN), which is undefined behavior. Variable
    // bin axes need FindFixBin's TMath::BinarySearch over the bin edges
    // rather than the fixed formula below; none of gxana's callers pass one
    // (the flux histogram axis is always fixed-width), so this throws
    // instead of silently returning a wrong bin.
    const int n = axis->GetNbins();
    const double xmin = axis->GetXmin();
    const double xmax = axis->GetXmax();
    if (x < xmin)
        return 0;
    if (!(x < xmax))
        return n + 1;
    if (axis->GetXbins()->fN > 0)
        throw std::invalid_argument("LegacyFindBin: variable-bin axes are not supported");
    return 1 + static_cast<int>(n * (x - xmin) / (xmax - xmin));
}

double TargetDensity(const Target& target)
{
    const Double_t Na = 6.022e23; //[atoms/mol]
    const Double_t length = target.zMax - target.zMin; //[cm]
    return target.atoms * Na * length * target.density * pow(10,-24) / target.molarMass;
}

namespace {

// Sum of weight*qvalueBranch over the data tree; nan for a channel without Q-factors.
double QValueYield(TTree& tree, const std::string& weight, const std::string& qvalueBranch)
{
    if (qvalueBranch.empty())
        return std::nan("");
    auto df = ROOT::RDataFrame(tree)
        .Define("qvalue_acc", (weight+"*"+qvalueBranch).c_str());
    return df.Sum("qvalue_acc").GetValue();
}

// Acceptance mc/thrown and its error (MC error and sqrt(thrown) in quadrature).
std::pair<double, double> ScalarAcceptance(double mc, double mcErr, double thrown)
{
    const double accept = mc / thrown;
    return {accept, accept * sqrt( pow( mcErr / mc, 2  ) + pow ( sqrt(thrown) / thrown, 2) )};
}

} // namespace

// GetDiffXSecFile and GetTotXSecFile: verbatim from AnalysisNote/xsection/FitFunctions.cpp.
void GetDiffXSecFile
(
 std::vector<TTree*> trees, TH1D* flux, std::vector<std::string> delim,
 std::string fitType, std::unordered_map<std::string, std::vector<double>>& xiParamRange, 
 std::ofstream& outputFile, std::ofstream& xsecFile,
 const XSecPhysics& physics, std::string weight, int chebyOrder, int n_threads)
{
    //Initialize variables
    Double_t yield, yield_err, yieldMC, yieldMC_err, yieldT, yieldT_err, yieldF, yieldF_err, yieldQVal;
    Double_t diffxsec, diffxsec_err, intxsec, intxsec_err, accept, accept_err, deltaT;
    // Target values
    Double_t tar_val = TargetDensity(physics.target);
    printf("The target value is: %f\n", tar_val); 
    
    Double_t hy_den = tar_val; Double_t BR = physics.br; Double_t BR_Err = physics.brErr;
    
    std::string treeName = delim[2];

    //Get energy boundries for flux yields
    std::cout << treeName << std::endl;
    const gxana::BinNameParts bin = gxana::ParseBinName(treeName);
    std::string emin = bin.emin;
    std::string emax = bin.emax;
    std::string tmin = bin.tmin;
    std::string tmax = bin.tmax;
    
    //En bins for flux
    // gxana: ROOT 6.24 TAxis::FindFixBin formula; 6.40 FindBin differs at exact bin edges and would change the flux (thesis reproduction)
    std::cout << "EnBin: (" << LegacyFindBin(flux->GetXaxis(),stod(emin)) <<","<< LegacyFindBin(flux->GetXaxis(),stod(emax))-1 << ")" << std::endl;
    yieldF = flux->IntegralAndError(LegacyFindBin(flux->GetXaxis(),stod(emin)), LegacyFindBin(flux->GetXaxis(),stod(emax))-1, yieldF_err);

    //tbins
    std::cout << "TMin, TMax: (" << tmin << "," << tmax <<")"<< std::endl;
    deltaT = stod(tmax) - stod(tmin);
    
    //Fit Data and MC for yields
    std::string histTitle = "#bf{-t: ("+tmin+", "+tmax+")}";

    // Get the Qvalue yield for data (tree[0])
    yieldQVal = QValueYield(*trees[0], weight, physics.qvalueBranch);
    
    // gxana: JohnsonMCShape (legacy MakeXSecFiles.C) needs > 25 entries, and
    // restarts every bin from the configured parameters. JohnsonMCShapeSyst
    // (legacy GetXSecFilesUML.C) has no minimum (> 0 in the window only).
    const bool mcShape = IsMCShapeFit(fitType);
    // MCPdf (legacy MakeXSecFitMC.C) only needs both trees non-empty (-1: no window count).
    const bool mcPdf = IsMCPdfFit(fitType);
    const int minEntries = mcPdf ? -1 : (fitType == kJohnsonMCShapeSyst ? 0 : (mcShape ? 25 : 10));
    if(trees[0]->GetEntries() > 0 && trees[1]->GetEntries() > 0
       && trees[0]->GetEntries(physics.gate.c_str()) > minEntries){

        if (mcPdf) {
            RooFitMCPdf(trees[1], trees[0], histTitle, delim, &yieldMC, &yieldMC_err, &yield, &yield_err, weight, chebyOrder);
        } else if (mcShape) {
            FitParams binParams = xiParamRange;
            RooFitMCShapeSeed(trees[1], histTitle, delim, &yieldMC, &yieldMC_err, binParams, weight, 10, fitType);
            RooFitDataMCShape(trees[0], histTitle, delim, &yield, &yield_err, binParams, weight, 10, fitType);
        } else {
            RooFitMC(trees[1], histTitle, delim, &yieldMC, &yieldMC_err, fitType, xiParamRange,  weight);
            RooFitData(trees[0], histTitle, delim, &yield, &yield_err, fitType, xiParamRange, chebyOrder, weight);
        }

        //Get thrown yields
        yieldT = trees[2]->GetEntries(); 
        yieldT_err = sqrt(yieldT);
        std::cout << "Yields: " << yield << " +/- " << yield_err << "(QVal: " << yieldQVal << ")"  << std::endl;
        std::cout << "Yields MC: " << yieldMC << " +/- " << yieldMC_err << std::endl;
        std::cout << "Yields Thrown: " << yieldT << " +/- " << yieldT_err << std::endl;
    
        if(yield_err > yield)
            std::cerr << "[WARNING] Yield_err > Yield" << std::endl;
        //Get acceptance
        std::tie(accept, accept_err) = ScalarAcceptance(yieldMC, yieldMC_err, yieldT);

        //GetDiffXSec
        if (mcPdf && !(yieldMC > 0)) {
            // gxana: the MCPdf guard returns zeros (no usable MC or data); no acceptance to divide by
            std::cerr << "[WARNING] MCPdf fit gave no MC yield... saving Cross section to 0." << std::endl;
            yield = 0; yield_err = 0;
            diffxsec = 0; diffxsec_err = 0;
            accept = accept_err = 0;
        } else {
        diffxsec = yield / ( hy_den * yieldF * BR * accept * deltaT );
        diffxsec_err = diffxsec * sqrt( pow( yield_err / yield, 2) + pow( yieldF_err / yieldF, 2) + pow (accept_err / accept, 2) + pow (BR_Err / BR, 2) );
        }
        //Write out data to files
    }
    else{
            std::cerr << "[WARNING] Tree entries are too small to fit data... saving Cross section to 0."
                      << std::endl;
            yield = 0; yield_err = 0;
            diffxsec = 0; diffxsec_err = 0;
            // gxana: legacy printed these uninitialized; NaN marks "not computed".
            yieldMC = yieldMC_err = yieldT = yieldT_err = accept = accept_err = std::nan("");
    }
    
    // Save outputs to files
    outputFile << (stod(tmin)+stod(tmax))/2 << "  " << deltaT/2 << "  "
               << yield << "  " << yield_err << "  "
               << yieldQVal << "  " << sqrt(yieldQVal)  << "  "
               << yieldMC << "  " << yieldMC_err << "  "
               << yieldT << "  " << yieldT_err << "  "
               << accept << "  " << accept_err << "  "
               << yieldF << "  " << yieldF_err
               << std::endl;

    xsecFile << (stod(tmin)+stod(tmax))/2 << "  " << diffxsec*pow(10,9) << "  "
             << deltaT/2 << "  " << diffxsec_err*pow(10,9)
             << std::endl;
}

void GetTotXSecFile
(
 std::vector<TTree*> trees, TH1D* flux, std::vector<std::string> delim,
 std::string fitType, std::unordered_map<std::string, std::vector<double>>& xiParamRange, 
 std::ofstream& outputFile, std::ofstream& xsecFile,
 const XSecPhysics& physics, std::string weight, int chebyOrder, int n_threads)
{
    //Initialize variables
    Double_t yield, yield_err, yield_qval, yieldMC, yieldMC_err, yieldT, yieldT_err, yieldF, yieldF_err;
    Double_t totxsec, totxsec_err, accept, accept_err, deltaT;

    // Target values
    Double_t tar_val = TargetDensity(physics.target);
    printf("The target value is: %f\n", tar_val); 
    
    Double_t hy_den = tar_val; Double_t BR = physics.br; Double_t BR_Err = physics.brErr;
    
    std::string treeName = delim[2];
    
    //Get energy boundries for flux yields
    const gxana::BinNameParts bin = gxana::ParseBinName(treeName);
    std::string emin = bin.emin;
    std::string emax = bin.emax;
    
    std::cout << "Emin,Emax: (" << stod(emin) <<","<< stod(emax) <<")" << std::endl;
    Double_t deltaE = stod(emax) - stod(emin);
    
    // Get flux value in the energy range
    // gxana: ROOT 6.24 TAxis::FindFixBin formula; 6.40 FindBin differs at exact bin edges and would change the flux (thesis reproduction)
    std::cout << "EnBin: (" << LegacyFindBin(flux->GetXaxis(),stod(emin)) <<","<< LegacyFindBin(flux->GetXaxis(),stod(emax))-1 << ")" << std::endl;
    yieldF = flux->IntegralAndError(LegacyFindBin(flux->GetXaxis(),stod(emin)), LegacyFindBin(flux->GetXaxis(),stod(emax))-1, yieldF_err);//last bin is exclusiv so n-1

    //Fit Data and MC for yields
    std::string histTitle = "#bf{E_{#gamma}: ("+emin+", "+emax+")}";
    // gxana: the legacy total-cross-section path had no entry gate (the thesis
    // energy bins are all well populated); same gate as GetDiffXSecFile so an
    // empty bin of a low-statistics channel writes 0 instead of fitting nothing.
    const bool mcShape = IsMCShapeFit(fitType);
    // MCPdf (legacy MakeXSecFitMC.C) only needs both trees non-empty (-1: no window count).
    const bool mcPdf = IsMCPdfFit(fitType);
    const int minEntries = mcPdf ? -1 : (fitType == kJohnsonMCShapeSyst ? 0 : (mcShape ? 25 : 10));
    if(trees[0]->GetEntries() > 0 && trees[1]->GetEntries() > 0
       && trees[0]->GetEntries(physics.gate.c_str()) > minEntries){
        if (mcPdf) {
            RooFitMCPdf(trees[1], trees[0], histTitle, delim, &yieldMC, &yieldMC_err, &yield, &yield_err, weight, chebyOrder);
        } else if (mcShape) {
            FitParams binParams = xiParamRange;  // every bin restarts from the configured parameters
            RooFitMCShapeSeed(trees[1], histTitle, delim, &yieldMC, &yieldMC_err, binParams, weight, 10, fitType);
            RooFitDataMCShape(trees[0], histTitle, delim, &yield, &yield_err, binParams, weight, 10, fitType);
        } else {
            RooFitMC(trees[1], histTitle, delim, &yieldMC, &yieldMC_err, fitType, xiParamRange, weight);//mc is a weighted likelihood fit
            RooFitData(trees[0], histTitle, delim, &yield, &yield_err, fitType, xiParamRange, chebyOrder, weight);
        }

        //Get thrown yields
        yieldT = trees[2]->GetEntries(); 
        yieldT_err = sqrt(yieldT);
        std::cout << "Yields: " << yield << " +/- " << yield_err << std::endl;
        std::cout << "Yields MC: " << yieldMC << " +/- " << yieldMC_err << std::endl;
        std::cout << "Yields Thrown: " << yieldT << " +/- " << yieldT_err << std::endl;

        //Get acceptance
        std::tie(accept, accept_err) = ScalarAcceptance(yieldMC, yieldMC_err, yieldT);

        //GetTotXSec
        if (mcPdf && !(yieldMC > 0)) {
            // gxana: the MCPdf guard returns zeros (no usable MC or data); no acceptance to divide by
            std::cerr << "[WARNING] MCPdf fit gave no MC yield... saving total cross section to 0." << std::endl;
            yield = 0; yield_err = 0;
            totxsec = 0; totxsec_err = 0;
            accept = accept_err = 0;
        } else {
        totxsec = yield / ( hy_den * yieldF * BR * accept );
        totxsec_err = totxsec * sqrt( pow( yield_err / yield, 2) + pow( yieldF_err / yieldF, 2) + pow( accept_err / accept, 2  ) + pow (BR_Err / BR, 2) );
        }
    }
    else{
        std::cerr << "[WARNING] Tree entries are too small to fit data... saving total cross section to 0."
                  << std::endl;
        yield = 0; yield_err = 0;
        totxsec = 0; totxsec_err = 0;
        yieldMC = yieldMC_err = yieldT = yieldT_err = accept = accept_err = std::nan("");
    }

    // Get the Qvalue yield for data (tree[0])
    yield_qval = QValueYield(*trees[0], weight, physics.qvalueBranch);
    
    //Write out data to files
    outputFile <<  (stod(emax)+stod(emin))/2 << "  " <<  deltaE/2 << "  "
               <<  yield << "  " <<  yield_err << "  "
               <<  yield_qval << "  " <<  sqrt(yield_qval) << "  "
               <<  yieldMC << "  " <<  yieldMC_err << "  "
               <<  yieldT << "  " <<  yieldT_err << "  "
               <<  accept << "  " <<  accept_err << "  "
               <<  yieldF << "  " <<  yieldF_err
               <<  std::endl;
    
    xsecFile << (stod(emin)+stod(emax))/2 << "  " <<  totxsec*pow(10,9) << "  " <<  deltaE/2 << "  "
             <<  totxsec_err*pow(10,9)
             <<  std::endl;
}

namespace {

std::unique_ptr<TFile> OpenOrThrow(const std::string& path)
{
    std::unique_ptr<TFile> file(TFile::Open(path.c_str(), "READ"));
    if (!file || file->IsZombie())
        throw std::runtime_error("WriteXSecTables: cannot open " + path);
    return file;
}

TTree* TreeOrThrow(TDirectory& dir, const std::string& name)
{
    auto* tree = dynamic_cast<TTree*>(dir.Get(name.c_str()));
    if (!tree)
        throw std::runtime_error("WriteXSecTables: no tree " + name + " in " + dir.GetName());
    return tree;
}

bool EndsWith(const std::string& text, const std::string& suffix)
{
    return text.size() >= suffix.size() && text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// The tables of one set of binned trees: dataDir holds the energy-bin trees
// followed by their -t-bin trees; mcDir and thrownDir the trees of the same names.
// outName is the table-name stem (totxsec_<outName>.txt ...); delim[0] is the
// fit-plot label, delim[1] the plot name.
void WriteTablesForTrees(TDirectory& dataDir, TDirectory& mcDir, TDirectory& thrownDir, TH1D* flux,
                         const std::string& outName, const std::string& label, const std::string& fitType,
                         FitParams& params, const std::string& dir, const XSecPhysics& physics,
                         const std::string& weight, int chebyOrder)
{
    std::vector<std::string> delim{label, outName, ""};
    const std::string& name = outName;

    // Set up files for total xsection
    std::ofstream tot_outf( (dir+"totout_"+name+".txt").c_str() );
    tot_outf << "enBinCenter\t" << "EnErr\t"
             << "data_yield\t" << "yield_err\t"
             << "qval_yield\t" << "qval_yield_err\t"
             << "mc_yield\t" << "mc_err\t"
             << "thrown_yield\t" << "thrown_err\t"
             << "accept\t" << "accept_err\t"
             << "flux\t" << "flux_err\t" << std::endl;
    std::ofstream totxsec_outf( (dir+"totxsec_"+name+".txt").c_str() );
    totxsec_outf << "enBinCenter\t" << "sigma\t" << "enBinWidth\t"
                 << "Yerr\t" << std::endl;

    // Iterate over all keys in the root file
    TIter nextTree(dataDir.GetListOfKeys());
    TKey* treeKey;
    std::ofstream diff_outf; std::ofstream diffxsec_outf;
    while ((treeKey = (TKey*)nextTree())) {
        if (std::string(treeKey->GetClassName()) != "TTree")
            continue;
        std::string treeName = treeKey->GetName();
        delim[2] = treeName;
        TTree* tree = TreeOrThrow(dataDir, treeName);
        TTree* treeMC = TreeOrThrow(mcDir, treeName);
        TTree* treeThrown = TreeOrThrow(thrownDir, treeName);
        std::cout << " Processing TTree: " << treeName << std::endl;

        if (treeName.find("tmin") == std::string::npos) { // full energy bin
            GetTotXSecFile({tree, treeMC, treeThrown}, flux, delim, fitType, params,
                           tot_outf, totxsec_outf, physics, weight, chebyOrder);

            // Set diffxsec output files with headers
            if (diffxsec_outf.is_open()) {
                diffxsec_outf.close(); diff_outf.close();
            }
            diff_outf.open( (dir+"diffout_"+name+"_"+treeName+".txt").c_str());
            diffxsec_outf.open( (dir+"diffxsec_"+name+"_"+treeName+".txt").c_str());
            diff_outf << "tcenter\t" << "terr\t"
                      << "data_yield\t" << "yield_err\t"
                      << "qval_yield\t" << "qval_yield_err\t"
                      << "mc_yield\t" << "mc_err\t"
                      << "thrown_yield\t" << "thrown_err\t"
                      << "accept\t" << "accep_err\t"
                      << "flux\t" << "flux_err"
                      << std::endl;

            diffxsec_outf << "tBinCenter\t" << "dsigmadt\t"
                          << "tBinWidth\t" << "Yerr" << std::endl;
        } else { // -t bin
            GetDiffXSecFile({tree, treeMC, treeThrown}, flux, delim, fitType, params,
                            diff_outf, diffxsec_outf, physics, weight, chebyOrder);
        }
    }
}

} // namespace

void WriteXSecTables(const std::string& dataFile, const std::string& mcFile, const std::string& thrownFile,
                     TH1D* flux, const std::string& name, const std::string& label,
                     const std::string& fitType, FitParams& params, const std::string& logDir,
                     const XSecPhysics& physics, const std::string& weight, int chebyOrder)
{
    std::string dir = logDir;
    if (dir.empty() || dir.back() != '/')
        dir += '/';
    std::cout << "Processing cross section output for:\n" << dataFile << std::endl;
    auto data = OpenOrThrow(dataFile);
    auto mc = OpenOrThrow(mcFile);
    auto thrown = OpenOrThrow(thrownFile);
    gSystem->mkdir(dir.c_str(), true);
    std::cout << "Storing data files to:\n" << dir << std::endl;

    bool directoryMode = false;
    {
        TIter nextKey(data->GetListOfKeys());
        TKey* key;
        while ((key = (TKey*)nextKey()))
            if (std::string(key->GetClassName()) == "TDirectoryFile")
                directoryMode = true;
    }
    if (!directoryMode) {
        WriteTablesForTrees(*data, *mc, *thrown, flux, name, label, fitType, params, dir, physics, weight, chebyOrder);
        return;
    }

    // Variation file (legacy GetXSecFilesUML.C, written by divideVariationTreesIntoBins):
    // one directory vary_<cut>_<value> per variation holding the data trees and a
    // sibling vary_<cut>_<value>_mc directory holding the reconstructed-MC trees
    // (taken from mcFile, which may be the same file); the thrown trees are
    // the top-level trees of thrownFile. Tables are named <stem>_<name>_<directory>.
    std::vector<std::string> dirNames;
    TIter nextDir(data->GetListOfKeys());
    TKey* dirKey;
    while ((dirKey = (TKey*)nextDir())) {
        const std::string dirName = dirKey->GetName();
        if (std::string(dirKey->GetClassName()) != "TDirectoryFile" || EndsWith(dirName, "_mc"))
            continue;
        if (std::find(dirNames.begin(), dirNames.end(), dirName) == dirNames.end())
            dirNames.push_back(dirName);
    }
    for (const auto& dirName : dirNames) {
        std::cout << "Processing Trees in TDirectory: " << dirName << std::endl;
        auto* dataDir = dynamic_cast<TDirectory*>(data->Get(dirName.c_str()));
        auto* mcDir = dynamic_cast<TDirectory*>(mc->Get((dirName + "_mc").c_str()));
        if (!dataDir)
            throw std::runtime_error("WriteXSecTables: no directory " + dirName + " in " + dataFile);
        if (!mcDir)
            throw std::runtime_error("WriteXSecTables: no directory " + dirName + "_mc in " + mcFile);
        WriteTablesForTrees(*dataDir, *mcDir, *thrown, flux, name + "_" + dirName, label, fitType, params,
                            dir, physics, weight, chebyOrder);
    }
}

} // namespace xsec
} // namespace gxana
