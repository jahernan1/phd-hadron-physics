#include "gxana/xsection/XSec.h"

#include <ROOT/RDataFrame.hxx>
#include <TFile.h>
#include <TKey.h>
#include <TSystem.h>

#include <cmath>
#include <iostream>
#include <math.h>
#include <memory>
#include <stdexcept>
#include <stdio.h>

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

// GetDiffXSecFile and GetTotXSecFile: verbatim from AnalysisNote/xsection/FitFunctions.cpp.
void GetDiffXSecFile
(
 std::vector<TTree*> trees, TH1D* flux, std::vector<std::string> delim,
 std::string fitType, std::unordered_map<std::string, std::vector<double>>& xiParamRange, 
 std::ofstream& outputFile, std::ofstream& xsecFile,
 std::string weight, int chebyOrder, int n_threads)
{
    //Initialize variables
    Double_t yield, yield_err, yieldMC, yieldMC_err, yieldT, yieldT_err, yieldF, yieldF_err, yieldQVal;
    Double_t diffxsec, diffxsec_err, intxsec, intxsec_err, accept, accept_err, deltaT;
    // Target values
    Double_t Na = 6.022e23; //[atoms/mol]
    Double_t tar_Len = 79.1-50.4; //[cm]
    Double_t tar_Den = 70.08e-3; // 2018 (+/- 0.0035) [g/cm^3]
    Double_t hy_MM = 2.01588; //[g/mol]
    Double_t tar_val = 2 * Na * tar_Len * tar_Den * pow(10,-24) / hy_MM;
    printf("The target value is: %f\n", tar_val); 
    
    Double_t hy_den = tar_val; Double_t BR_Lamb = 0.641; Double_t BR_Lamb_Err = 0.005;
    
    std::string treeName = delim[2];

    //Get energy boundries for flux yields
    std::cout << treeName << std::endl;
    std::string emin = treeName.substr(treeName.find("emin")+5);
    emin = emin.substr(0,emin.find("_"));
    std::string emax = treeName.substr(treeName.find("emax")+5);
    emax = emax.substr(0,emax.find("_"));
    std::string tmin = treeName.substr(treeName.find("tmin")+5);
    tmin = tmin.substr(0,tmin.find("_"));
    std::string tmax = treeName.substr(treeName.find("tmax")+5);
    //tmax = tmax.substr(0,tmax.find("_"));//end of std::string not needed
    
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
    auto df = ROOT::RDataFrame(*trees[0])
        .Define("qvalue_acc", (weight+"*qvalue_decayxim_M").c_str());
    yieldQVal = df.Sum("qvalue_acc").GetValue();
    
    if(trees[0]->GetEntries() > 0 && trees[1]->GetEntries() > 0
       && trees[0]->GetEntries("(hybrid_combo)*(decayxim_M>1.3&&decayxim_M<1.35)") > 10){
        
        RooFitMC(trees[1], histTitle, delim, &yieldMC, &yieldMC_err, fitType, xiParamRange,  weight);
        RooFitData(trees[0], histTitle, delim, &yield, &yield_err, fitType, xiParamRange, chebyOrder, weight);
    
        //Get thrown yields
        yieldT = trees[2]->GetEntries(); 
        yieldT_err = sqrt(yieldT);
        std::cout << "Yields: " << yield << " +/- " << yield_err << "(QVal: " << yieldQVal << ")"  << std::endl;
        std::cout << "Yields MC: " << yieldMC << " +/- " << yieldMC_err << std::endl;
        std::cout << "Yields Thrown: " << yieldT << " +/- " << yieldT_err << std::endl;
    
        if(yield_err > yield)
            std::cerr << "[WARNING] Yield_err > Yield" << std::endl;
        //Get acceptance
        accept = yieldMC / yieldT;
        accept_err = accept * sqrt( pow( yieldMC_err / yieldMC, 2  ) + pow ( yieldT_err / yieldT, 2) );

        //GetDiffXSec
        diffxsec = yield / ( hy_den * yieldF * BR_Lamb * accept * deltaT );
        diffxsec_err = diffxsec * sqrt( pow( yield_err / yield, 2) + pow( yieldF_err / yieldF, 2) + pow (accept_err / accept, 2) + pow (BR_Lamb_Err / BR_Lamb, 2) );
        //Write out data to files
    }
    else{
            std::cerr << "[WARNING] Tree entries are too small to fit data... saving Cross section to 0."
                      << std::endl;
            yield = 0; yield_err = 0;
            diffxsec = 0; diffxsec_err = 0;
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
 std::string weight, int chebyOrder, int n_threads)
{
    //Initialize variables
    Double_t yield, yield_err, yield_qval, yieldMC, yieldMC_err, yieldT, yieldT_err, yieldF, yieldF_err;
    Double_t totxsec, totxsec_err, accept, accept_err, deltaT;

    // Target values
    Double_t Na = 6.022e23; //[atoms/mol]
    Double_t tar_Len = 79.1-50.4; //[cm]
    Double_t tar_Den = 70.08e-3; // 2018 (+/- 0.0035) [g/cm^3]
    Double_t hy_MM = 2.01588; //[g/mol]
    Double_t tar_val = 2 * Na * tar_Len * tar_Den * pow(10,-24) / hy_MM;
    printf("The target value is: %f\n", tar_val); 
    
    Double_t hy_den = tar_val; Double_t BR_Lamb = 0.641; Double_t BR_Lamb_Err = 0.005;
    
    std::string treeName = delim[2];
    
    //Get energy boundries for flux yields
    std::string emin = treeName.substr(treeName.find("emin")+5);
    emin = emin.substr(0,emin.find("_"));
    std::string emax = treeName.substr(treeName.find("emax")+5);
    //emax = emax.substr(0,emax.find("_"));//end of std::string not needed
    
    std::cout << "Emin,Emax: (" << stod(emin) <<","<< stod(emax) <<")" << std::endl;
    Double_t deltaE = stod(emax) - stod(emin);
    
    // Get flux value in the energy range
    // gxana: ROOT 6.24 TAxis::FindFixBin formula; 6.40 FindBin differs at exact bin edges and would change the flux (thesis reproduction)
    std::cout << "EnBin: (" << LegacyFindBin(flux->GetXaxis(),stod(emin)) <<","<< LegacyFindBin(flux->GetXaxis(),stod(emax))-1 << ")" << std::endl;
    yieldF = flux->IntegralAndError(LegacyFindBin(flux->GetXaxis(),stod(emin)), LegacyFindBin(flux->GetXaxis(),stod(emax))-1, yieldF_err);//last bin is exclusiv so n-1

    //Fit Data and MC for yields
    std::string histTitle = "#bf{E_{#gamma}: ("+emin+", "+emax+")}";
    RooFitMC(trees[1], histTitle, delim, &yieldMC, &yieldMC_err, fitType, xiParamRange, weight);//mc is a weighted likelihood fit
    RooFitData(trees[0], histTitle, delim, &yield, &yield_err, fitType, xiParamRange, chebyOrder, weight);
    
    //Get thrown yields
    yieldT = trees[2]->GetEntries(); 
    yieldT_err = sqrt(yieldT);
    std::cout << "Yields: " << yield << " +/- " << yield_err << std::endl;
    std::cout << "Yields MC: " << yieldMC << " +/- " << yieldMC_err << std::endl;
    std::cout << "Yields Thrown: " << yieldT << " +/- " << yieldT_err << std::endl;
    
    //Get acceptance
    accept = yieldMC / yieldT;
    accept_err = accept * sqrt( pow( yieldMC_err / yieldMC, 2  ) + pow ( yieldT_err / yieldT, 2) );
    
    //GetTotXSec
    totxsec = yield / ( hy_den * yieldF * BR_Lamb * accept );
    totxsec_err = totxsec * sqrt( pow( yield_err / yield, 2) + pow( yieldF_err / yieldF, 2) + pow( accept_err / accept, 2  ) + pow (BR_Lamb_Err / BR_Lamb, 2) );

    // Get the Qvalue yield for data (tree[0])
    auto df = ROOT::RDataFrame(*trees[0])
        .Define("qvalue_acc", (weight+"*qvalue_decayxim_M").c_str());
    yield_qval = df.Sum("qvalue_acc").GetValue();
    
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

TTree* TreeOrThrow(TFile& file, const std::string& name)
{
    auto* tree = dynamic_cast<TTree*>(file.Get(name.c_str()));
    if (!tree)
        throw std::runtime_error("WriteXSecTables: no tree " + name + " in " + file.GetName());
    return tree;
}

} // namespace

void WriteXSecTables(const std::string& dataFile, const std::string& mcFile, const std::string& thrownFile,
                     TH1D* flux, const std::string& name, const std::string& label,
                     const std::string& fitType, FitParams& params, const std::string& logDir,
                     const std::string& weight, int chebyOrder)
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
    std::vector<std::string> delim{label, name, ""};

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
    TIter nextTree(data->GetListOfKeys());
    TKey* treeKey;
    std::ofstream diff_outf; std::ofstream diffxsec_outf;
    while ((treeKey = (TKey*)nextTree())) {
        if (std::string(treeKey->GetClassName()) != "TTree")
            continue;
        std::string treeName = treeKey->GetName();
        delim[2] = treeName;
        TTree* tree = TreeOrThrow(*data, treeName);
        TTree* treeMC = TreeOrThrow(*mc, treeName);
        TTree* treeThrown = TreeOrThrow(*thrown, treeName);
        std::cout << " Processing TTree: " << treeName << std::endl;

        if (treeName.find("tmin") == std::string::npos) { // full energy bin
            GetTotXSecFile({tree, treeMC, treeThrown}, flux, delim, fitType, params,
                           tot_outf, totxsec_outf, weight, chebyOrder);

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
                            diff_outf, diffxsec_outf, weight, chebyOrder);
        }
    }
}

} // namespace xsec
} // namespace gxana
