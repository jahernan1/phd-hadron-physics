#include "gxana/xsection/YieldFit.h"
#include "gxana/xsection/XSec.h"

#include <RVersion.h>
#include <RooAbsPdf.h>
#include <RooArgSet.h>
#include <RooFitResult.h>
#include <RooGlobalFunc.h>
#include <RooHist.h>
#include <RooPlot.h>
#include <RooRealVar.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TLatex.h>
#include <TMath.h>
#include <TPad.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>

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

std::string constructFitString(const std::string& fitType, const FitParams& params) {
    std::ostringstream oss;
    oss << fitType << "::xisignal(decayxim_M";
    for (const auto& param : OrderedFitParams(fitType, params)) {
        oss << ", " << param.first << "[" << param.second[0] << ", "
            << param.second[1] << ", " << param.second[2] << "]";
    }
    oss << ")";
    return oss.str();
}

std::string constructFitStringData(const std::string& fitType, const FitParams& params) {
    std::ostringstream oss;
    oss << fitType << "::xisignal(decayxim_M";
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

bool AttemptFit(RooWorkspace* w, RooDataSet* data, FitParams &params, double lowerBound, double upperBound) {

    w->var("decayxim_M")->setRange("signal", lowerBound, upperBound);

    RooFitResult* fitResult =
        w->pdf("model")->fitTo(*data,
                               Extended(true), EvalErrorWall(true),
                               SumW2Error(false),RecoverFromUndefinedRegions(10),
                               //RooFit::AsymptoticError(true),
                               Hesse(false),  Range("signal"),
                               PrintLevel(-1), Save(true) GXANA_LEGACY_EVAL GXANA_LEGACY_MINIMIZER);

    fitResult->Print();
    if (fitResult == nullptr || fitResult->status() != 0) {
        std::cerr << "Fit failed with status: " << (fitResult ? fitResult->status() : -1) << std::endl;
        delete fitResult;
        return false;
    }

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

void RooFitMC(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err,std::string fitType, FitParams &params, std::string hist_weight, int max_retries)
{
    TH1::AddDirectory(kFALSE);
    const std::string saveDir = FitSaveDir(delim);
    // Set up workspace and data
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.40);
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    mass.setRange("signal", 1.27, 1.38);

    w->import(RooArgSet(mass));

    // Take all signal events in MC
    *yield = data->sumEntries();
    *yield_err = TMath::Sqrt(*yield);

    // Build model with initial parameters from params std::vector
    std::string signalStr = constructFitString(fitType, params);
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

    data->plotOn(massframe, Name("data"), Binning(40, 1.27, 1.42));
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

void RooFitData(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err, std::string fitType, FitParams &params, int chebyOrder, std::string hist_weight, int max_retries){

    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    const std::string saveDir = FitSaveDir(delim);
    //Import dataset to plot
    double max_mass=1.45; double small = 1e-4;
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.45);
    //Weighted fit
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    // Legacy: data->createHistogram("decayxim_M",200,1.25,1.45), which ROOT 6.24
    // resolved to createHistogram(varNameList, xbins, ybins, zbins): 200 bins over
    // decayxim_M's own range [1.27, 1.45]. Same histogram, on every ROOT version:
    TH1* dataHist = (TH1*)data->createHistogram(data->GetName(), mass, Binning(200))->Clone(delim[2].c_str());
    double min_mass = 1.27;

    while(dataHist->GetBinContent(dataHist->FindBin(max_mass)) < small)
        max_mass = max_mass - dataHist->GetBinWidth(1)/2;
    while(dataHist->GetBinContent(dataHist->FindBin(min_mass)) < small && min_mass<1.28)
        min_mass = min_mass + dataHist->GetBinWidth(1)/2;


    mass.setRange("fitrange", min_mass, max_mass);
    const double rangeExpandStep = 0.005;

    //Set up workspace
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    w->import(RooArgSet(mass));

    //Build model and Fit data
    std::string signalStr = constructFitStringData(fitType, params);
    w->factory(signalStr.c_str());
    if(chebyOrder==1)
        w->factory("Chebychev::bkgd(decayxim_M,{a0[0.,0.,1.2]})");//,a1[-0.1,-2,-1e-2]
    else
        w->factory("Chebychev::bkgd(decayxim_M,{a0[0.,0.,0.9],a1[-0.0,-0.5,0.2]})");
    w->factory("SUM::model( nxi[200,1,1e6]*xisignal, nbkgd[200,1,1e6]*bkgd)"); //nbkgd[200,1,1e6]*bkgd,

    // Attempt fit up to max_retries times
    int attempt = 1;
    while (attempt <= max_retries) {
        std::cout << "Attempt " << attempt << " to fit data." << std::endl;
        if (AttemptFit(w, data, params, min_mass, max_mass)) {
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

    data->plotOn(massframe, Binning(30, 1.27, 1.45), Name("data"));
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

} // namespace

std::string constructFitStringMCShape(const FitParams& params)
{
    // Legacy literal: mu[%f,1.32,1.33], lambda[%f,0.002,0.007], gamma[%f, -1,1], delta[%f,0.2,5.]
    return Form("Johnson::xisignal(decayxim_M, mu[%f,%.17g,%.17g], lambda[%f,%.17g,%.17g], "
                "gamma[%f,%.17g,%.17g], delta[%f,%.17g,%.17g])",
                Start(params, "mu"), Lower(params, "mu"), Upper(params, "mu"),
                Start(params, "lambda"), Lower(params, "lambda"), Upper(params, "lambda"),
                Start(params, "gamma"), Lower(params, "gamma"), Upper(params, "gamma"),
                Start(params, "delta"), Lower(params, "delta"), Upper(params, "delta"));
}

std::string constructFitStringDataMCShape(const FitParams& params)
{
    // Legacy literal: mu[%f,1.32,1.33], lambda[%f,%f,0.008], gamma[%f], delta[%f]
    return Form("Johnson::xisignal(decayxim_M, mu[%f,%.17g,%.17g], lambda[%f,%f,0.008], gamma[%f], delta[%f])",
                Start(params, "mu"), Lower(params, "mu"), Upper(params, "mu"),
                Start(params, "lambda"), Start(params, "lambda"),
                Start(params, "gamma"), Start(params, "delta"));
}

void RooFitMCShapeSeed(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err, FitParams &params, std::string hist_weight, int max_retries)
{
    TH1::AddDirectory(kFALSE);
    const std::string saveDir = FitSaveDir(delim);
    // Set up workspace and data
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.40);
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    mass.setRange("signal", 1.27, 1.38);

    w->import(RooArgSet(mass));

    // Build model with initial parameters from params
    w->factory(constructFitStringMCShape(params).c_str());
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
    *yield = data->sumEntries();
    *yield_err = std::sqrt(*yield);

    // Plot model and data
    RooPlot* massframe = mass.frame(Title(histTitle.c_str()));
    TCanvas* fitCan = new TCanvas("fitCanMC", "mc", 800, 700);
    fitCan->SetLogy();

    data->plotOn(massframe, Name("datapnts"), Binning(60, 1.27, 1.42));
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

void RooFitDataMCShape(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err, FitParams &params, std::string hist_weight, int max_retries)
{
    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    const std::string saveDir = FitSaveDir(delim);
    //Import dataset to plot
    double max_mass=1.45; double small = 1e-4;
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.45);
    //Weighted fit
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    // Legacy: data->createHistogram("decayxim_M"), i.e. decayxim_M's default
    // binning (100 bins over [1.27, 1.45]); written out explicitly here.
    TH1* dataHist = (TH1*)data->createHistogram(data->GetName(), mass, Binning(100))->Clone(delim[2].c_str());
    // gxana: LegacyFindBin (ROOT 6.24 formula) for the bin lookups; the scan
    // below steps by a third of a bin and so lands on bin edges.
    const TAxis* axis = dataHist->GetXaxis();
    double min_mass = axis->GetBinLowEdge(dataHist->FindFirstBinAbove(small, 1, 1, LegacyFindBin(axis, 1.32)));

    while(dataHist->GetBinContent(LegacyFindBin(axis, max_mass)) < small)
        max_mass = max_mass - dataHist->GetBinWidth(1)/3;
    while(dataHist->GetBinContent(LegacyFindBin(axis, min_mass)) < small && min_mass<1.28)
        min_mass = min_mass + dataHist->GetBinWidth(1)/3;

    mass.setRange("fitrange", min_mass, max_mass);
    const double rangeExpandStep = 0.005;

    //Set up workspace
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    w->import(RooArgSet(mass));

    //Build model and Fit data (legacy order: background first)
    w->factory("Chebychev::bkgd(decayxim_M,{a0[0.81,1e-3,1.25],a1[-0.1,-3.,-1e-3]})");
    w->factory(constructFitStringDataMCShape(params).c_str());
    w->factory("SUM::model( nxi[2000,1,1e6]*xisignal, nbkgd[2000,1,1e6]*bkgd)");

    // Attempt fit up to max_retries times. AttemptFit adds EvalErrorWall(true),
    // RooFit's default, to the legacy fitTo arguments.
    int attempt = 1;
    while (attempt <= max_retries) {
        std::cout << "Attempt " << attempt << " to fit data." << std::endl;
        if (AttemptFit(w, data, params, min_mass, max_mass)) {
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
    *yield_err =  w->var("nxi")->getError();

    //Plot model and data
    RooPlot* massframe = mass.frame( Title( histTitle.c_str() ) );
    TCanvas* fitCan = new TCanvas("fitCan"," c", 800, 700);

    data->plotOn(massframe, Name("datapnts"), Binning(50, 1.27, 1.45));
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

void SetFitStyle()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(600);
  gStyle->SetCanvasDefW(700);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadTopMargin   (0.06);
  gStyle->SetPadLeftMargin  (0.12);
  gStyle->SetPadRightMargin (0.05);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);

  gStyle->SetFrameFillStyle ( 0);
  gStyle->SetFrameFillColor ( 0);
  gStyle->SetFrameLineColor ( 1);
  gStyle->SetFrameLineStyle ( 0);
  gStyle->SetFrameLineWidth ( 1);
  gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);

  gStyle->SetNdivisions(505);

  gStyle->SetLineWidth(2);
  gStyle->SetHistLineWidth(2);
  gStyle->SetFrameLineWidth(2);
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.055,"X");
  gStyle->SetLabelSize(0.055,"Y");

  gStyle->SetLabelOffset(0.010,"X");
  gStyle->SetLabelOffset(0.010,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132);
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");

  gStyle->SetTitleSize(0.08,"X");
  gStyle->SetTitleSize(0.08,"Y");

  gStyle->SetTitleOffset(0.9,"X");
  gStyle->SetTitleOffset(0.65,"Y");

  gStyle->SetTextSize(0.08);
  gStyle->SetTextFont(132);

  gStyle->SetOptStat(0);

  gROOT->ForceStyle();

  TLatex* latex = new TLatex();
  latex->SetNDC();
  latex->SetTextFont(132);
  latex->SetTextSize(0.08);
  latex->SetTextAlign(32);
  gROOT->ForceStyle();
}

} // namespace xsec
} // namespace gxana
