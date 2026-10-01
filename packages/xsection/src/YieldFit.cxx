#include "gxana/xsection/YieldFit.h"
#include "gxana/xsection/XSec.h"
#include "gxana/common/Style.h"

#include <RVersion.h>
#include <RooAbsPdf.h>
#include <RooAddPdf.h>
#include <RooArgList.h>
#include <RooChebychev.h>
#include <RooDataHist.h>
#include <RooDataSet.h>
#include <RooHistPdf.h>
#include <RooArgSet.h>
#include <RooFitResult.h>
#include <RooGlobalFunc.h>
#include <RooHist.h>
#include <RooPlot.h>
#include <RooRealVar.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TMath.h>
#include <TPad.h>
#include <TStyle.h>
#include <TSystem.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>

using namespace RooFit;

// ROOT >= 6.32 evaluates likelihoods with a new backend by default; keep the
// legacy evaluator the thesis fits used (ROOT 6.24).
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 32, 0)
#define GXANA_LEGACY_EVAL , RooFit::EvalBackend::Legacy()
#else
#define GXANA_LEGACY_EVAL
#endif

// RooFit's default minimizer changed from TMinuit ("Minuit") to Minuit2 when
// ROOT 6.30 made Minuit2 the RooFit default; pin "Minuit","migrad" explicitly
// there so fit yields reproduce the thesis (ROOT 6.24) on newer ROOT. Below
// 6.30 the default is already TMinuit/migrad, so the macro is a no-op there
// -- and must be, because RooAbsPdf::fitTo on ROOT 6.24/6.26 only has fixed
// overloads up to 8 RooCmdArgs; adding a 9th (on top of the other 8 passed
// below) would fail to compile there.
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 30, 0)
#define GXANA_LEGACY_MINIMIZER , RooFit::Minimizer("Minuit", "migrad")
#else
#define GXANA_LEGACY_MINIMIZER
#endif

namespace gxana {
namespace xsec {

namespace {

std::string gFitPlotDir;

// Directory for this variation's fit PDFs, created on demand; "" = no plots.
// Legacy: FitFunctions.cpp always wrote fit PDFs under a hard-coded site fits/
// directory; the port only writes them when a plot dir is set via
// SetFitPlotDir (default: gFitPlotDir empty, so nothing is saved).
std::string FitSaveDir(const std::vector<std::string>& delim)
{
    if (gFitPlotDir.empty() || delim.empty())
        return "";
    std::string dir = gFitPlotDir + "/" + delim[0] + "/";
    gSystem->mkdir(dir.c_str(), true);
    return dir;
}

} // namespace

const char* const kJohnsonMCShape = "JohnsonMCShape";
const char* const kJohnsonMCShapeSyst = "JohnsonMCShapeSyst";
const char* const kMCPdf = "MCPdf";

bool IsMCPdfFit(const std::string& fitType) { return fitType == kMCPdf; }

bool IsMCShapeFit(const std::string& fitType)
{
    return fitType == kJohnsonMCShape || fitType == kJohnsonMCShapeSyst;
}

void SetFitPlotDir(const std::string& dir) { gFitPlotDir = dir; }

const std::string& GetFitPlotDir() { return gFitPlotDir; }

// RooFit factory arguments are positional. The legacy code iterated the
// unordered_map and relied on libstdc++ returning the four Johnson (or three
// Voigtian, two Gaussian) names in reverse insertion order, which matches the
// constructor order below; libc++ does not, so the order is explicit here.
std::vector<std::pair<std::string, std::vector<double>>> OrderedFitParams(const std::string& fitType,
                                                                          const FitParams& params)
{
    static const std::map<std::string, std::vector<std::string>> kOrder = {
        {"Johnson", {"mu", "lambda", "gamma", "delta"}},
        {kJohnsonMCShape, {"mu", "lambda", "gamma", "delta"}},
        {kJohnsonMCShapeSyst, {"mu", "lambda", "gamma", "delta"}},
        {"Gaussian", {"mean", "sigma"}},
        {"Voigtian", {"mean", "width", "sigma"}},
    };
    std::vector<std::pair<std::string, std::vector<double>>> ordered;
    const auto order = kOrder.find(fitType);
    if (order != kOrder.end() && order->second.size() == params.size()) {
        for (const auto& name : order->second) {
            const auto param = params.find(name);
            if (param == params.end())
                break;
            ordered.emplace_back(*param);
        }
        if (ordered.size() == params.size())
            return ordered;
        ordered.clear();
    }
    ordered.assign(params.begin(), params.end());
    return ordered;
}

std::string constructFitString(const std::string& fitType, const FitParams& params, const Observable& obs) {
    std::ostringstream oss;
    oss << fitType << "::xisignal(" << obs.branch;
    for (const auto& param : OrderedFitParams(fitType, params)) {
        oss << ", " << param.first << "[" << param.second[0] << ", "
            << param.second[1] << ", " << param.second[2] << "]";
    }
    oss << ")";
    return oss.str();
}

std::string constructFitStringData(const std::string& fitType, const FitParams& params, const Observable& obs) {
    std::ostringstream oss;
    oss << fitType << "::xisignal(" << obs.branch;
    for (const auto& param : OrderedFitParams(fitType, params)) {
        oss << ", " << param.first << "[";
        if(fitType=="Gaussian")
            oss << param.second[0] << ", "
                << param.second[1] << ", " << param.second[2] << "]";
        else if(fitType=="Johnson"){
                if( param.first=="gamma")
                    oss << param.second[0] << "]";
                else if(param.first=="delta" || param.first=="lambda" )
                    oss << (param.second[0] - param.second[2])/2 << ", "
                        << param.second[0] << ", "
                        << param.second[2] << "]";
                else
                    oss << param.second[0] << ", "
                        << param.second[1] << ", "
                        << param.second[2] << "]";
            }
        else if(fitType=="Voigtian"){
            if(param.first=="width")
                    oss << param.second[0] << "]";
            else if(param.first=="sigma")
                oss << param.second[0] << ", "
                    << param.second[0] << ", "
                    << param.second[2] << "]";
            else
                oss << param.second[0] << ", "
                    << param.second[1] << ", "
                    << param.second[2] << "]";
        }
        else
            oss << param.second[0] << ", "
                << param.second[1] << ", "
                << param.second[2] << "]";
    }
    oss << ")";
    return oss.str();
}

bool AttemptFit(RooWorkspace* w, RooDataSet* data, FitParams &params, double lowerBound, double upperBound,
                const Observable& obs) {

    w->var(obs.branch.c_str())->setRange("signal", lowerBound, upperBound);

    RooFitResult* fitResult =
        w->pdf("model")->fitTo(*data,
                               Extended(true), EvalErrorWall(true),
                               SumW2Error(false),RecoverFromUndefinedRegions(10),
                               //RooFit::AsymptoticError(true),
                               Hesse(false),  Range("signal"),
                               PrintLevel(-1), Save(true) GXANA_LEGACY_EVAL GXANA_LEGACY_MINIMIZER);

    // gxana: a fitTo that returns no result (RooFit gives up before minimising)
    // must take the "Fit failed" path instead of dereferencing nullptr.
    if (fitResult == nullptr || fitResult->status() != 0) {
        std::cerr << "Fit failed with status: " << (fitResult ? fitResult->status() : -1) << std::endl;
        delete fitResult;
        return false;
    }
    fitResult->Print();

    // Update parameters if fit is successful
    auto paramList = fitResult->floatParsFinal();
    RooRealVar *p;
    for (int i = 0; i < paramList.getSize(); i++)
        {
            p = (RooRealVar *)paramList.at(i);
            const char* par = p->getTitle();
            std::cout << p->getTitle() << std::endl;
            std::cout << p->getVal() << " +/- " << p->getError() << std::endl;
            if(params.find(par) != params.end())
                params[par][0] = p->getVal();
        }

    delete fitResult;
    return true;
}

bool AttemptFitMC(RooWorkspace* w, RooDataSet* data, FitParams &params) {
    RooFitResult* fitResult = w->pdf("model")->fitTo(*data, SumW2Error(false), Hesse(false), PrintLevel(-1), Range("signal"), Save(true) GXANA_LEGACY_EVAL GXANA_LEGACY_MINIMIZER);

    if (fitResult == nullptr || fitResult->status() != 0) {
        std:: cerr << "Fit failed with status: " << (fitResult ? fitResult->status() : -1) << std::endl;
        delete fitResult;
        return false;
    }

    fitResult->Print();
    // Update parameters if fit is successful
    auto paramList = fitResult->floatParsFinal();
    RooRealVar *p;
    for (int i = 0; i < paramList.getSize(); i++)
        {
            p = (RooRealVar *)paramList.at(i);
            const char* par = p->getTitle();
            std::cout << p->getTitle() << std::endl;
            std::cout << p->getVal() << " +/- " << p->getError() << std::endl;
            if(params.find(par) != params.end())
                params[par][0] = p->getVal();
        }

    delete fitResult;
    return true;
}

void RooFitMC(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err,std::string fitType, FitParams &params, const Observable& obs, const MassWindows& win, std::string hist_weight, int max_retries)
{
    TH1::AddDirectory(kFALSE);
    const std::string saveDir = FitSaveDir(delim);
    // Set up workspace and data
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooRealVar mass(obs.branch.c_str(), obs.title.c_str(), win.lo, win.mcHi);
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    mass.setRange("signal", win.lo, win.mcSignalHi);

    w->import(RooArgSet(mass));

    // Take all signal events in MC
    *yield = data->sumEntries();
    *yield_err = TMath::Sqrt(*yield);

    // Build model with initial parameters from params std::vector
    std::string signalStr = constructFitString(fitType, params, obs);
    std::cout << "Signal String: " << signalStr << std::endl;
    w->factory(signalStr.c_str());
    w->factory("SUM::model(nxi[1000,1,1e6]*xisignal)");

    // Attempt fit up to max_retries times
    int attempt = 1;
    while (attempt <= max_retries) {
        std::cout << "Attempt " << attempt << " to fit mc data." << std::endl;
        if (AttemptFitMC(w, data, params)) {
            break;  // Successful fit
        }
        attempt++;
    }

    // Check if fit was ultimately unsuccessful
    if (attempt > max_retries) {
        std::cout << "Max retries reached. Fit did not converge." << std::endl;
        delete w;
        return;
    }

    // Plot model and data
    RooPlot* massframe = mass.frame(Title(histTitle.c_str()));
    TCanvas* fitCan = new TCanvas("fitCanMC", "mc", 800, 700);
    TPad* topPad = new TPad("topPad", "Top Pad", 0.0, 0.3, 1.0, 1.0);
    TPad* bottomPad = new TPad("bottomPad", "Bottom Pad", 0.0, 0.0, 1.0, 0.3);
    topPad->SetBottomMargin(0.02);
    topPad->SetGrid();
    bottomPad->SetTopMargin(0.015);
    bottomPad->SetBottomMargin(0.4);
    bottomPad->SetGrid(1,0);

    fitCan->cd();
    topPad->Draw();
    bottomPad->Draw();

    // Draw graphs in the top pad
    topPad->cd();

    data->plotOn(massframe, Name("data"), Binning(40, win.lo, win.mcPlotHi));
    w->pdf("model")->paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.92), ShowConstants(true));
    w->pdf("model")->plotOn(massframe, LineWidth(3), Range("signal"), Name("model"));
    w->pdf("model")->plotOn(massframe,Components("xisignal"), DrawOption("F"), FillColor(kBlue - 9), FillStyle(3001), MoveToBack(), Range("signal"));

    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505, kFALSE);
    massframe->SetMinimum(0.1);
    massframe->Draw();

    bottomPad->cd();
    bottomPad->SetGrid(0,1);
    RooHist* resPlot = massframe->residHist("data","model",true);
    resPlot->SetTitle(Form("; %s;#sigma_{#scale[0.7]{pull}}",massframe->GetXaxis()->GetTitle()));
    resPlot->GetYaxis()->CenterTitle(true);
    resPlot->GetYaxis()->SetNdivisions(505);
    resPlot->GetXaxis()->SetLabelSize(0.13);
    resPlot->GetYaxis()->SetLabelSize(0.13);
    resPlot->GetXaxis()->SetTitleSize(0.18);
    resPlot->GetYaxis()->SetTitleSize(0.18);
    resPlot->GetXaxis()->SetTitleOffset(0.9);
    resPlot->GetYaxis()->SetTitleOffset(0.25);

    resPlot->DrawClone("ap");

    if (!saveDir.empty())
        fitCan->Print( (saveDir+"recon_"+delim[1]+"_"+delim[2]+".pdf").c_str());
    fitCan->Close();

    delete w;
}

void RooFitData(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err, std::string fitType, FitParams &params, const Observable& obs, const MassWindows& win, int chebyOrder, std::string hist_weight, int max_retries){

    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    const std::string saveDir = FitSaveDir(delim);
    //Import dataset to plot
    double max_mass=win.dataHi; double small = 1e-4;
    RooRealVar mass(obs.branch.c_str(), obs.title.c_str(), win.lo, win.dataHi);
    //Weighted fit
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    // Legacy: data->createHistogram("decayxim_M",200,1.25,1.45), which ROOT 6.24
    // resolved to createHistogram(varNameList, xbins, ybins, zbins): 200 bins over
    // decayxim_M's own range [1.27, 1.45]. Same histogram, on every ROOT version:
    TH1* dataHist = (TH1*)data->createHistogram(data->GetName(), mass, Binning(200))->Clone(delim[2].c_str());
    double min_mass = win.lo;

    // Fit window: shrink the edges until they land on a populated bin, so the
    // RooFit lineshape is drawn (and evaluated) only where there is data -- a
    // Johnson pdf drawn over an empty or zero-content edge bin blows up.
    // gxana: the upper scan is bounded by the lower edge; on a histogram with
    // no populated bin the legacy loop never terminated.
    while(max_mass > min_mass && dataHist->GetBinContent(dataHist->FindBin(max_mass)) < small)
        max_mass = max_mass - dataHist->GetBinWidth(1)/2;
    while(dataHist->GetBinContent(dataHist->FindBin(min_mass)) < small && min_mass<win.dataEdge)
        min_mass = min_mass + dataHist->GetBinWidth(1)/2;
    if (!(max_mass > min_mass)) {
        std::cerr << "[WARNING] " << delim[2] << ": no populated mass bin, skipping the data fit (yield 0)." << std::endl;
        *yield = 0; *yield_err = 0;
        delete data; delete dataHist;
        return;
    }

    mass.setRange("fitrange", min_mass, max_mass);
    const double rangeExpandStep = 0.005;

    //Set up workspace
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    w->import(RooArgSet(mass));

    //Build model and Fit data
    std::string signalStr = constructFitStringData(fitType, params, obs);
    w->factory(signalStr.c_str());
    if(chebyOrder==1)
        w->factory(Form("Chebychev::bkgd(%s,{a0[0.,0.,1.2]})", obs.branch.c_str()));//,a1[-0.1,-2,-1e-2]
    else
        w->factory(Form("Chebychev::bkgd(%s,{a0[0.,0.,0.9],a1[-0.0,-0.5,0.2]})", obs.branch.c_str()));
    w->factory("SUM::model( nxi[200,1,1e6]*xisignal, nbkgd[200,1,1e6]*bkgd)"); //nbkgd[200,1,1e6]*bkgd,

    // Attempt fit up to max_retries times
    int attempt = 1;
    while (attempt <= max_retries) {
        std::cout << "Attempt " << attempt << " to fit data." << std::endl;
        if (AttemptFit(w, data, params, min_mass, max_mass, obs)) {
            break;  // Successful fit
        }
        // Expand the fit range slightly on each retry
        min_mass = std::max(min_mass, min_mass + rangeExpandStep);  // Ensuring it doesn't go below 1.27
        max_mass = std::min(max_mass, max_mass - rangeExpandStep);  // Ensuring it doesn't go above 1.40
        std::cout << "Modify bin edges: (" << min_mass << "," << max_mass << ")" << std::endl;
        attempt++;
    }

    // Check if fit was ultimately unsuccessful
    if (attempt > max_retries) {
        std::cerr << "[Warning] Max retries reached. Fit did not converge." << std::endl;
    }

    *yield = w->var("nxi")->getVal();
    *yield_err = w->var("nxi")->getError();

    //Plot model and data
    RooPlot* massframe = mass.frame( Title( histTitle.c_str() ) );
    TCanvas* fitCan = new TCanvas("fitCan"," c", 700, 700);
    TPad* topPad = new TPad("topPad", "Top Pad", 0.0, 0.3, 1.0, 1.0);
    TPad* bottomPad = new TPad("bottomPad", "Bottom Pad", 0.0, 0.0, 1.0, 0.3);
    topPad->SetBottomMargin(0.02);
    topPad->SetGrid();
    bottomPad->SetTopMargin(0.015);
    bottomPad->SetBottomMargin(0.4);
    bottomPad->SetGrid(1,0);

    fitCan->cd();
    topPad->Draw();
    bottomPad->Draw();

    // Draw graphs in the top pad
    topPad->cd();

    data->plotOn(massframe, Binning(30, win.lo, win.dataHi), Name("data"));
    w->pdf("model")->paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.92), ShowConstants(true));
    w->pdf("model")->plotOn(massframe, LineWidth(4), Name("model"), Range("fitrange"));
    w->pdf("model")->plotOn(massframe, Components("xisignal"), DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Range("fitrange"));//
    w->pdf("model")->plotOn(massframe,  Components("bkgd"), LineStyle(kDotted), Range("fitrange"));

    //set up specific frame styling
    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505);

    massframe->DrawClone();

    bottomPad->cd();
    bottomPad->SetGrid(0,1);
    RooHist* resPlot = massframe->residHist("data","model",true);
    resPlot->SetTitle(Form("; %s;#sigma_{#scale[0.7]{pull}}",massframe->GetXaxis()->GetTitle()));
    double ymax = TMath::MaxElement(resPlot->GetN(),resPlot->GetY());
    double ymin = TMath::MinElement(resPlot->GetN(),resPlot->GetY());

    // std::abs: the legacy abs() resolved to the double overload (libstdc++).
    double ysym = std::abs(ymax) >  std::abs(ymin) ?
        std::abs(ymax) : std::abs(ymin);

    resPlot->GetYaxis()->CenterTitle(true);
    resPlot->GetYaxis()->SetNdivisions(505);
    resPlot->GetXaxis()->SetLabelSize(0.13);
    resPlot->GetYaxis()->SetLabelSize(0.13);
    resPlot->GetXaxis()->SetTitleSize(0.18);
    resPlot->GetYaxis()->SetTitleSize(0.18);
    resPlot->GetXaxis()->SetTitleOffset(0.9);
    resPlot->GetYaxis()->SetTitleOffset(0.25);
    resPlot->GetYaxis()->SetRangeUser(-ysym-2,ysym+2);
    resPlot->DrawClone("ap");

    if(!saveDir.empty())
        fitCan->Print( (saveDir+"data_"+delim[1]+"_"+delim[2]+".pdf").c_str());
    fitCan->Close();
}

// JohnsonMCShape: port of legacy AnalysisNote/xsection/MakeXSecFiles.C
// (RooFitHistMC, RooFitHist), the fits behind the thesis hybrid_combo tables.

namespace {

double Start(const FitParams& params, const char* name) { return params.at(name).at(0); }
double Lower(const FitParams& params, const char* name) { return params.at(name).at(1); }
double Upper(const FitParams& params, const char* name) { return params.at(name).at(2); }

// Literals that differ between the two ports of the MC-shape-seeded fit.
//
// JohnsonMCShape is MakeXSecFiles.C (the thesis tables). JohnsonMCShapeSyst is
// the fit of the legacy AnalysisNote GetXSecFilesUML.C (the Barlow cut-variation
// systematics, per-variation tables). Both seed the signal shape with a Johnson
// fit to the reconstructed MC and then fit data with that shape plus a
// Chebychev background; they differ as follows (legacy RooFitHistMC/RooFitHist):
//
//   quantity                      JohnsonMCShape            JohnsonMCShapeSyst
//   MC yield                      data->sumEntries()        fitted nxi (getVal)
//   MC yield error                sqrt(yield)               sqrt(yield)
//   data yield error              nxi->getError()           sqrt(nxi)
//   data-fit mu range             from params (1.32,1.33)   fixed literal (1.31,1.33)
//   data-fit lambda range         [MC lambda, 0.008]        [MC lambda, 0.01]
//   Chebychev background          a0[0.81,1e-3,1.25],       a0[0.,0.,1.2],
//                                 a1[-0.1,-3.,-1e-3]        a1[0.,-0.5,0.2]
//   window-scan start (data)      1.32                      1.30
//   entries gate (XSec.cxx)       window entries > 25       window entries > 0
//   MC-fit parameter ranges       from params               from params; the
//     (mu/lambda/gamma/delta)     (config)                  legacy literals
//                                                           [1.32,1.33],[0.002,0.007],
//                                                           [-0.5,0.5],[0.2,1.5]
//                                                           are carried by the config
//
// Shared by both: gamma and delta are fixed to the MC fit in the data fit, the
// signal-shape lambda is floored at the MC value, createHistogram("decayxim_M")
// default 100 bins, every bin restarts from the configured start values (the
// legacy call builds a fresh parameter vector per bin), and 10 retries.
// gxana additions shared by both: LegacyFindBin for the bin lookups, and the
// bounded window scan with an early return (yield 0) on an empty histogram.
struct MCShapeLiterals {
    bool mcYieldFromFit;        // MC yield = nxi (true) or sum of weights (false)
    bool dataErrSqrt;           // data yield error = sqrt(yield) (true) or nxi error (false)
    bool dataMuFromParams;      // mu range in the data fit from params (true) or muLo/muHi
    double muLo, muHi;
    const char* lambdaHi;       // literal text of the lambda upper bound
    const char* chebychev;      // printf format of the background factory string; %s = observable branch
    double scanStart;
};

const MCShapeLiterals& Literals(const std::string& fitType)
{
    static const MCShapeLiterals nominal{false, false, true, 0., 0., "0.008",
                                         "Chebychev::bkgd(%s,{a0[0.81,1e-3,1.25],a1[-0.1,-3.,-1e-3]})", 1.32};
    static const MCShapeLiterals syst{true, true, false, 1.31, 1.33, "0.01",
                                      "Chebychev::bkgd(%s,{a0[0.,0.,1.2],a1[0.,-0.5,0.2]})", 1.30};
    if (fitType == kJohnsonMCShapeSyst)
        return syst;
    if (fitType == kJohnsonMCShape)
        return nominal;
    throw std::invalid_argument("not an MC-shape fit type: " + fitType);
}

} // namespace

std::string constructFitStringMCShape(const FitParams& params, const Observable& obs, const std::string& fitType)
{
    Literals(fitType);  // validates the fit type; both types take their MC ranges from params
    // Legacy literal: mu[%f,1.32,1.33], lambda[%f,0.002,0.007], gamma[%f, -1,1], delta[%f,0.2,5.]
    // (JohnsonMCShapeSyst: gamma[%f,-0.5,0.5], delta[%f,0.2,1.5])
    return Form("Johnson::xisignal(%s, mu[%f,%.17g,%.17g], lambda[%f,%.17g,%.17g], "
                "gamma[%f,%.17g,%.17g], delta[%f,%.17g,%.17g])", obs.branch.c_str(),
                Start(params, "mu"), Lower(params, "mu"), Upper(params, "mu"),
                Start(params, "lambda"), Lower(params, "lambda"), Upper(params, "lambda"),
                Start(params, "gamma"), Lower(params, "gamma"), Upper(params, "gamma"),
                Start(params, "delta"), Lower(params, "delta"), Upper(params, "delta"));
}

std::string constructFitStringDataMCShape(const FitParams& params, const Observable& obs, const std::string& fitType)
{
    const MCShapeLiterals& lit = Literals(fitType);
    // Legacy literal: mu[%f,1.32,1.33], lambda[%f,%f,0.008], gamma[%f], delta[%f]
    // (JohnsonMCShapeSyst: mu[%f,1.31,1.33], lambda[%f,%f,0.01], gamma[%f], delta[%f])
    const double muLo = lit.dataMuFromParams ? Lower(params, "mu") : lit.muLo;
    const double muHi = lit.dataMuFromParams ? Upper(params, "mu") : lit.muHi;
    return Form("Johnson::xisignal(%s, mu[%f,%.17g,%.17g], lambda[%f,%f,%s], gamma[%f], delta[%f])",
                obs.branch.c_str(), Start(params, "mu"), muLo, muHi,
                Start(params, "lambda"), Start(params, "lambda"), lit.lambdaHi,
                Start(params, "gamma"), Start(params, "delta"));
}

void RooFitMCShapeSeed(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err, FitParams &params, const Observable& obs, const MassWindows& win, std::string hist_weight, int max_retries, const std::string& fitType)
{
    const MCShapeLiterals& lit = Literals(fitType);
    TH1::AddDirectory(kFALSE);
    const std::string saveDir = FitSaveDir(delim);
    // Set up workspace and data
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooRealVar mass(obs.branch.c_str(), obs.title.c_str(), win.lo, win.mcHi);
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    mass.setRange("signal", win.lo, win.mcSignalHi);

    w->import(RooArgSet(mass));

    // Build model with initial parameters from params
    w->factory(constructFitStringMCShape(params, obs, fitType).c_str());
    w->factory("SUM::model(nxi[1000,1,1e6]*xisignal)");

    // Attempt fit up to max_retries times
    int attempt = 1;
    while (attempt <= max_retries) {
        std::cout << "Attempt " << attempt << " to fit mc data." << std::endl;
        if (AttemptFitMC(w, data, params)) {
            break;  // Successful fit
        }
        attempt++;
    }

    // Check if fit was ultimately unsuccessful
    if (attempt > max_retries) {
        std::cout << "Max retries reached. Fit did not converge." << std::endl;
        // Legacy returned here with the yield unset (uninitialized).
        *yield = *yield_err = std::nan("");
        delete w;
        return;
    }

    // Store results if fit was successful
    *yield = lit.mcYieldFromFit ? w->var("nxi")->getVal() : data->sumEntries();
    *yield_err = std::sqrt(*yield);

    // Plot model and data
    RooPlot* massframe = mass.frame(Title(histTitle.c_str()));
    TCanvas* fitCan = new TCanvas("fitCanMC", "mc", 800, 700);
    fitCan->SetLogy();

    data->plotOn(massframe, Name("datapnts"), Binning(60, win.lo, win.mcPlotHi));
    w->pdf("model")->paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.92), Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("lambda"))));
    w->pdf("model")->plotOn(massframe, LineWidth(3), Name("model"), Range("signal"));
    w->pdf("xisignal")->plotOn(massframe, DrawOption("F"), FillColor(kBlue - 9), FillStyle(3001), MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Name("xisignal"), Range("signal"));

    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505, kFALSE);
    massframe->SetMinimum(0.1);
    massframe->Draw();

    fitCan->SetGrid();
    if (!saveDir.empty())
        fitCan->Print( (saveDir+"recon_"+delim[1]+"_"+delim[2]+".pdf").c_str());
    fitCan->Close();

    delete w;
}

void RooFitDataMCShape(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err, FitParams &params, const Observable& obs, const MassWindows& win, std::string hist_weight, int max_retries, const std::string& fitType)
{
    const MCShapeLiterals& lit = Literals(fitType);
    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    const std::string saveDir = FitSaveDir(delim);
    //Import dataset to plot
    double max_mass=win.dataHi; double small = 1e-4;
    RooRealVar mass(obs.branch.c_str(), obs.title.c_str(), win.lo, win.dataHi);
    //Weighted fit
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    // Legacy: data->createHistogram("decayxim_M"), i.e. decayxim_M's default
    // binning (100 bins over [1.27, 1.45]); written out explicitly here.
    TH1* dataHist = (TH1*)data->createHistogram(data->GetName(), mass, Binning(100))->Clone(delim[2].c_str());
    // gxana: LegacyFindBin (ROOT 6.24 formula) for the bin lookups; the scan
    // below steps by a third of a bin and so lands on bin edges.
    const TAxis* axis = dataHist->GetXaxis();
    // Fit window: the edges move inwards until they sit on a populated bin, so
    // the RooFit lineshape is drawn (and evaluated) only where there is data --
    // a Johnson pdf drawn over an empty or zero-content edge bin blows up.
    // gxana: FindFirstBinAbove returns -1 when nothing is populated at or above
    // the scan start (1.32; 1.30 for JohnsonMCShapeSyst), and the upper scan is bounded by the lower edge; on such a
    // histogram the legacy code read a bogus bin edge and looped forever.
    const int firstBin = dataHist->FindFirstBinAbove(small, 1, 1, LegacyFindBin(axis, lit.scanStart));
    double min_mass = firstBin > 0 ? axis->GetBinLowEdge(firstBin) : max_mass;

    while(max_mass > min_mass && dataHist->GetBinContent(LegacyFindBin(axis, max_mass)) < small)
        max_mass = max_mass - dataHist->GetBinWidth(1)/3;
    while(dataHist->GetBinContent(LegacyFindBin(axis, min_mass)) < small && min_mass<win.dataEdge)
        min_mass = min_mass + dataHist->GetBinWidth(1)/3;
    if (!(max_mass > min_mass)) {
        std::cerr << "[WARNING] " << delim[2] << ": no populated mass bin, skipping the data fit (yield 0)." << std::endl;
        *yield = 0; *yield_err = 0;
        delete data; delete dataHist;
        return;
    }

    mass.setRange("fitrange", min_mass, max_mass);
    const double rangeExpandStep = 0.005;

    //Set up workspace
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    w->import(RooArgSet(mass));

    //Build model and Fit data (legacy order: background first)
    w->factory(Form(lit.chebychev, obs.branch.c_str()));
    w->factory(constructFitStringDataMCShape(params, obs, fitType).c_str());
    w->factory("SUM::model( nxi[2000,1,1e6]*xisignal, nbkgd[2000,1,1e6]*bkgd)");

    // Attempt fit up to max_retries times. AttemptFit adds EvalErrorWall(true),
    // RooFit's default, to the legacy fitTo arguments.
    int attempt = 1;
    while (attempt <= max_retries) {
        std::cout << "Attempt " << attempt << " to fit data." << std::endl;
        if (AttemptFit(w, data, params, min_mass, max_mass, obs)) {
            break;  // Successful fit
        }
        // Narrow the fit range on each retry
        min_mass = std::max(min_mass, min_mass + rangeExpandStep);
        max_mass = std::min(max_mass, max_mass - rangeExpandStep);
        attempt++;
    }

    // Check if fit was ultimately unsuccessful
    if (attempt > max_retries) {
        std::cerr << "[Warning] Max retries reached. Fit did not converge." << std::endl;
    }

    *yield = w->var("nxi")->getVal();
    *yield_err = lit.dataErrSqrt ? std::sqrt(*yield) : w->var("nxi")->getError();

    //Plot model and data
    RooPlot* massframe = mass.frame( Title( histTitle.c_str() ) );
    TCanvas* fitCan = new TCanvas("fitCan"," c", 800, 700);

    data->plotOn(massframe, Name("datapnts"), Binning(50, win.lo, win.dataHi));
    w->pdf("model")->paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.92));
    w->pdf("model")->plotOn(massframe, LineWidth(4), Name("model"), Range("fitrange"));
    w->pdf("xisignal")->plotOn(massframe, DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Name("xisignal"), Range("fitrange"));
    w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("fitrange"));

    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505);
    massframe->DrawClone();

    fitCan->SetGrid();
    if(!saveDir.empty())
        fitCan->Print( (saveDir+"data_"+delim[1]+"_"+delim[2]+".pdf").c_str());
    fitCan->Close();
}

void RooFitMCPdf(TTree* mcTree, TTree* dataTree, std::string histTitle, std::vector<std::string> delim,
                 double* yieldMC, double* yieldMC_err, double* yield, double* yield_err,
                 const Observable& obs, const MassWindows& win, std::string hist_weight, int chebyOrder)
{
    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    const std::string saveDir = FitSaveDir(delim);

    // getHistogramPdf: weighted MC mass -> 60-bin histogram -> RooHistPdf (order 0).
    RooRealVar mcMass(obs.branch.c_str(), obs.title.c_str(), win.lo, win.dataHi);
    RooRealVar mcWeight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* mcData = new RooDataSet("mcData", "Dataset of mass", RooArgSet(mcMass, mcWeight), Import(*mcTree), WeightVar(mcWeight));
    TH1* hSim = (TH1*)mcData->createHistogram(mcData->GetName(), mcMass, Binning(60))->Clone();
    RooDataHist* simDataHist = new RooDataHist("simDataHist", "Simulated Data Histogram", RooArgSet(mcMass), hSim);
    RooHistPdf* histPdf = new RooHistPdf("histPdf", "Histogram PDF", RooArgSet(mcMass), RooArgSet(mcMass), *simDataHist, 0);
    *yieldMC = mcData->sumEntries();
    *yieldMC_err = std::sqrt(std::max(0.0, *yieldMC));

    // gxana: an empty tree, a missing weight branch or a non-positive weight sum
    // has no usable shape (zero-normalisation RooHistPdf); the legacy fit is not
    // attempted and the bin gets yield 0.
    RooRealVar mass(obs.branch.c_str(), obs.title.c_str(), win.mcPdfDataLo, win.dataHi);
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*dataTree), WeightVar(weight));
    if (mcData->numEntries() == 0 || data->numEntries() == 0 || !(*yieldMC > 0) || !(hSim->Integral() > 0)) {
        std::cerr << "[WARNING] " << (delim.size() > 2 ? delim[2] : histTitle)
                  << ": empty MC or data tree, or non-positive MC weight sum; skipping the MCPdf fit (yield 0)." << std::endl;
        *yieldMC = 0; *yieldMC_err = 0; *yield = 0; *yield_err = 0;
        delete data; delete mcData; delete simDataHist; delete histPdf; delete hSim;
        return;
    }

    RooPlot* mcFrame = mcMass.frame(Title(histTitle.c_str()));
    TCanvas* mcCan = new TCanvas("fitCanMC", "mc", 800, 700);
    mcData->plotOn(mcFrame, Name("datapnts"), Binning(60, win.lo, win.dataHi));
    histPdf->plotOn(mcFrame);
    mcFrame->GetYaxis()->SetMaxDigits(2);
    mcFrame->GetYaxis()->SetNdivisions(505, kFALSE);
    mcFrame->SetMinimum(0.1);
    mcFrame->Draw();
    mcCan->SetGrid();
    if (!saveDir.empty())
        mcCan->Print((saveDir + "recon_" + delim[1] + "_" + delim[2] + ".pdf").c_str());
    mcCan->Close();

    // RooFitHist: data over 1.275..1.45, histPdf + Chebychev, extended weighted fit.
    RooArgSet chebySet;
    RooRealVar a0("a0", "a0", 0.9, 0.01, 2.);
    RooRealVar a1("a1", "a1", -0.1, -2., -0.01);
    chebySet.add(a0);
    if (chebyOrder == 2) chebySet.add(a1);
    RooChebychev bkg("bkg", "Background", mass, chebySet);

    RooRealVar nsig("nsig", "number of signal events", 500, 1., 10000);
    RooRealVar nbkg("nbkg", "number of background events", 50, 0, 10000);
    RooAddPdf model("model", "g1+g2", RooArgList(bkg, *histPdf), RooArgList(nbkg, nsig));

    RooFitResult* fitResult = model.fitTo(*data, Extended(true), SumW2Error(false), RecoverFromUndefinedRegions(10),
                                          Hesse(false), PrintLevel(-1), Save(true) GXANA_LEGACY_EVAL GXANA_LEGACY_MINIMIZER);
    fitResult->Print();
    *yield = nsig.getVal();
    *yield_err = std::sqrt(std::max(0.0, *yield));

    RooPlot* massframe = mass.frame(Title(histTitle.c_str()));
    TCanvas* fitCan = new TCanvas("fitCan", " c", 800, 700);
    data->plotOn(massframe, Name("datapnts"), Binning(50, win.lo, win.dataHi));
    model.paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.9));
    model.plotOn(massframe, LineWidth(4));
    model.plotOn(massframe, Components("histPdf"), DrawOption("F"), FillColor(kBlue-9), FillStyle(3001), MoveToBack(), Name("xisignal"));
    model.plotOn(massframe, Components("bkg"), LineStyle(kDotted));
    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505);
    massframe->Draw();
    fitCan->SetGrid();
    if (!saveDir.empty())
        fitCan->Print((saveDir + "data_" + delim[1] + "_" + delim[2] + ".pdf").c_str());
    fitCan->Close();

    delete fitResult;
    delete data;
    delete mcData;
    delete histPdf;
    delete simDataHist;
    delete hSim;
}

void SetFitStyle()
{
  gxana::ApplyStyle(gxana::FitStyle());
}

} // namespace xsec
} // namespace gxana
