#include "gxana/barlow/VariationTrees.h"

#include <ROOT/RDataFrame.hxx>
#include <RVersion.h>
#include <RooAbsPdf.h>
#include <RooAbsReal.h>
#include <RooDataSet.h>
#include <RooFitResult.h>
#include <RooMsgService.h>
#include <RooPlot.h>
#include <RooRealVar.h>
#include <RooWorkspace.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTree.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

// Same pin as YieldFit.cxx: ROOT >= 6.30 defaults RooFit to Minuit2; the legacy
// check ran on 6.24 (TMinuit/migrad).
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 30, 0)
#define GXANA_LEGACY_MINIMIZER , RooFit::Minimizer("Minuit", "migrad")
#else
#define GXANA_LEGACY_MINIMIZER
#endif

namespace gxana {
namespace barlow {

std::pair<std::string, std::string> SplitAssign(const std::string& text)
{
    const auto eq = text.find('=');
    if (eq == std::string::npos || eq == 0)
        throw std::invalid_argument("expected NAME=VALUE: '" + text + "'");
    return {text.substr(0, eq), text.substr(eq + 1)};
}

namespace {

ROOT::RDF::RNode Prepare(ROOT::RDF::RNode node, const std::vector<std::pair<std::string, std::string>>& defines,
                         const std::vector<std::string>& filters)
{
    for (const auto& define : defines)
        node = node.Define(define.first, define.second);
    for (const auto& filter : filters)
        node = node.Filter(filter);
    return node;
}

} // namespace

void WriteVariationTrees(const VariationTreesSpec& spec)
{
    for (const auto& path : {spec.input, spec.inputMC})
        if (gSystem->AccessPathName(path.c_str()))
            throw std::runtime_error("cannot open " + path);
    if (spec.threads > 0)
        ROOT::EnableImplicitMT(spec.threads);
    const std::string dir = gSystem->GetDirName(spec.out.c_str()).Data();
    if (!dir.empty())
        gSystem->mkdir(dir.c_str(), true);

    ROOT::RDataFrame dfData(spec.tree, spec.input);
    ROOT::RDataFrame dfMC(spec.tree, spec.inputMC);
    auto data = Prepare(dfData, spec.defines, spec.filtersData);
    auto mc = Prepare(dfMC, spec.defines, spec.filtersMC);

    ROOT::RDF::RSnapshotOptions opts;
    opts.fOverwriteIfExists = true;
    bool first = true;
    for (const auto& variation : spec.variations) {
        opts.fMode = first ? "RECREATE" : "UPDATE";
        first = false;
        data.Filter(variation.cut, "cutset").Snapshot(variation.tree, spec.out, spec.branches, opts);
        opts.fMode = "UPDATE";
        mc.Filter(variation.cut, "cutset").Snapshot(variation.tree + "_mc", spec.out, spec.branches, opts);
    }
}

namespace {

using namespace RooFit;
using std::cout;
using std::endl;
using std::string;
using std::vector;

bool AttemptFitMC(RooWorkspace* w, RooDataSet* data, vector<double> &params)
{
    RooFitResult* fitResult = w->pdf("model")->fitTo(*data, SumW2Error(false), Hesse(true), PrintLevel(-1), Range("signal"), Save(true) GXANA_LEGACY_MINIMIZER);

    if (fitResult == nullptr || fitResult->status() != 0) {
        cout << "Fit failed with status: " << (fitResult ? fitResult->status() : -1) << endl;
        delete fitResult;
        return false;
    }

    fitResult->Print();
    // Update parameters if fit is successful
    params[0] = w->var("lambda")->getVal();
    params[1] = w->var("gamma")->getVal();
    params[2] = w->var("delta")->getVal();
    delete fitResult;
    return true;
}

void RooFitHistMC(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> &params, string hist_weight, const std::string& fitDir, int max_retries = 5)
{
    TH1::AddDirectory(kFALSE);

    // Set up workspace and data
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.40);
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    mass.setRange("signal", 1.27, 1.38);

    w->import(RooArgSet(mass));

    // Build model with initial parameters from params vector
    w->factory(Form("Johnson::xisignal(decayxim_M, mu[1.3217,1.32,1.33], lambda[%f,0.002,0.007], gamma[%f, -1,1], delta[%f,0.2,5.])", params[0], params[1], params[2]));
    w->factory("EXPR::bkgd('(decayxim_M)*(((decayxim_M)/m0)**2-1.0)**p*exp(b*(((decayxim_M)/m0)**2-1.0))',decayxim_M, m0[1.2602,1.255,1.275], b[-22,-40.,-5.], p[2])");
    w->factory("SUM::model(nxi[10000,1,1e6]*xisignal, nbkgd[500,1,1e6]*bkgd)");

    // Attempt fit up to max_retries times
    int attempt = 1;
    while (attempt <= max_retries) {
        cout << "Attempt " << attempt << " to fit mc data." << endl;
        if (AttemptFitMC(w, data, params)) {
            break;  // Successful fit
        }
        attempt++;
    }

    // Check if fit was ultimately unsuccessful
    if (attempt > max_retries) {
        cout << "Max retries reached. Fit did not converge." << endl;
        delete w;
        return;
    }

    // Store results if fit was successful
    *yield = w->var("nxi")->getVal();
    *yield_err = w->var("nxi")->getError();

    // Plot model and data
    RooPlot* massframe = mass.frame(Title(histTitle.c_str()));
    TCanvas* fitCan = new TCanvas("fitCanMC", "mc", 800, 700);
    fitCan->SetLogy();

    data->plotOn(massframe, Name("datapnts"), Binning(60, 1.27, 1.42));
    w->pdf("model")->paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.92), Parameters(RooArgSet(*w->var("nxi"), *w->var("nbkgd"), *w->var("mu"), *w->var("lambda"))));
    w->pdf("model")->plotOn(massframe, LineWidth(3), Name("model"), Range("signal"));
    w->pdf("xisignal")->plotOn(massframe, DrawOption("F"), FillColor(kBlue - 9), FillStyle(3001), MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Name("xisignal"), Range("signal"));
    w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("signal"));

    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505, kFALSE);
    massframe->SetMinimum(0.1);
    massframe->Draw();

    fitCan->SetGrid();
    if (!delim.empty())
        fitCan->SaveAs((fitDir + "/" + delim + ".pdf").c_str());
    fitCan->Close();

    delete w;
}

bool AttemptFit(RooWorkspace* w, RooDataSet* data, vector<double> &params) {

    RooFitResult* fitResult =
        w->pdf("model")->fitTo(*data,
                               Extended(true), //EvalErrorWall(true),
                               SumW2Error(false),RecoverFromUndefinedRegions(10),
                               //RooFit::AsymptoticError(true),
                               Hesse(false),  Range("fitrange"),
                               PrintLevel(-1), Save(true) GXANA_LEGACY_MINIMIZER);

    if (fitResult == nullptr || fitResult->status() != 0) {
        cout << "Fit failed with status: " << (fitResult ? fitResult->status() : -1) << endl;
        delete fitResult;
        return false;
    }
    fitResult->Print();

    // Update parameters if fit is successful
    params[0] = w->var("lambda")->getVal();
    params[1] = w->var("gamma")->getVal();
    params[2] = w->var("delta")->getVal();
    delete fitResult;
    return true;
}

    
void RooFitHist(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> &params, string hist_weight, const std::string& fitDir, int max_retries = 5){

    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    //Import dataset to plot
    double max_mass=1.45;
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.26, max_mass);
    //Weighted fit
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    TH1* dataHist = (TH1*)data->createHistogram("decayxim_M")->Clone(delim.c_str());
    Double_t min_mass = dataHist->GetXaxis()->GetBinLowEdge(dataHist->FindFirstBinAbove(1,1,1, dataHist->FindBin(1.32)));
    
    while(dataHist->GetBinContent(dataHist->FindBin(max_mass)) < 0)
        max_mass = max_mass - dataHist->GetBinWidth(1);
    mass.setRange("fitrange", 1.27, max_mass);
  
    //Set up workspace
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    w->import(RooArgSet(mass));

    //Build model and Fit data
    w->factory("Chebychev::bkgd(decayxim_M,{a0[0.81,0.01,2.],a1[-0.21,-2.,-0.01]})");//,a1[-0.1,-2,-1e-2]
    w->factory(Form("Johnson::xisignal(decayxim_M, mu[1.3217,1.32,1.33], lambda[%f,%f,0.008], gamma[%f], delta[%f])", params[0], params[0], params[1], params[2]));
    //w->factory("Johnson::xisignal(decayxim_M,mu[1.3127,1.321,1.323],lambda[0.005,0.003,0.008], gamma[0], delta[0.8,0.3,2])");
    w->factory("SUM::model( nxi[2000,1,1e6]*xisignal, nbkgd[2000,1,1e6]*bkgd)"); //nbkgd[200,1,1e6]*bkgd,
    
    // Attempt fit up to max_retries times
    int attempt = 1;
    while (attempt <= max_retries) {
        cout << "Attempt " << attempt << " to fit data." << endl;
        if (AttemptFit(w, data, params)) {
            break;  // Successful fit
        }
        attempt++;
    }

    // Check if fit was ultimately unsuccessful
    if (attempt > max_retries) {
        cout << "Max retries reached. Fit did not converge." << endl;
        //delete w;
        //return;
    }

    //cout << "Fit Status Code: " << fitResult->status() << endl;
    *yield = w->var("nxi")->getVal();
    *yield_err = w->var("nxi")->getError();
    
    //Plot model and data 
    RooPlot* massframe = mass.frame( Title( histTitle.c_str() ) );
    TCanvas* fitCan = new TCanvas("fitCan"," c", 800, 700);
    //fitCan->SetLogy();
        
    data->plotOn(massframe, Name("datapnts"), Binning(50, 1.26, max_mass));
    w->pdf("model")->paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.92));
    w->pdf("model")->plotOn(massframe, LineWidth(4), Name("model"), Range("fitrange"));
    w->pdf("xisignal")->plotOn(massframe, DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Name("xisignal"), Range("fitrange"));//
    w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("fitrange"));

    //set up specific frame styling
    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505);
    //massframe->SetMinimum(0.1);
  
    massframe->DrawClone();
  
    fitCan->SetGrid();
    //fitCan->Print("fit.pdf");
    if(!delim.empty())
        fitCan->Print( (fitDir+"/"+delim+".pdf").c_str());
    fitCan->Close();
    //w->writeToFile("model.root");
}

} // namespace


namespace {

TTree* GetTree(TFile& file, const std::string& name)
{
    auto* tree = file.Get<TTree>(name.c_str());
    if (!tree)
        throw std::runtime_error("no tree " + name + " in " + file.GetName());
    return tree;
}

std::unique_ptr<TFile> OpenRead(const std::string& path)
{
    std::unique_ptr<TFile> file(gSystem->AccessPathName(path.c_str()) ? nullptr : TFile::Open(path.c_str(), "READ"));
    if (!file || file->IsZombie())
        throw std::runtime_error("cannot open " + path);
    return file;
}

// Legacy appendToTextFile, with the file name a parameter.
void AppendYields(const std::string& path, const std::string& name, const std::string& id,
                  const std::vector<double>& values)
{
    std::ofstream out(path, std::ios::app);
    if (!out)
        throw std::runtime_error("cannot write " + path);
    out << name << "\t" << id;
    for (double v : values)
        out << "\t" << v;
    out << std::endl;
    std::cout << "Yield data appended to " << path << std::endl;
}

} // namespace

void CheckVariationYields(const CheckSpec& spec)
{
    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
    gSystem->mkdir(spec.fitDir.c_str(), true);
    auto file = OpenRead(spec.out);
    auto nominalFile = OpenRead(spec.nominal);
    auto nominalFileMC = OpenRead(spec.nominalMC);
    TTree* oldTree = GetTree(*nominalFile, spec.tree);
    TTree* oldTreeMC = GetTree(*nominalFileMC, spec.tree);
    for (const auto& variation : spec.variations) {
        const std::string id = variation.tree.rfind("vary_", 0) == 0 ? variation.tree.substr(5) : variation.tree;
        TTree* newTree = GetTree(*file, variation.tree);
        TTree* newTreeMC = GetTree(*file, variation.tree + "_mc");
        // Legacy left these uninitialised when a fit gave up; 0 here.
        double yield = 0, yield_err = 0, yieldMC = 0, yieldMC_err = 0;
        double nominal_yield = 0, nominal_yield_err = 0, nominal_yieldMC = 0, nominal_yieldMC_err = 0;
        std::vector<double> xiParamsOld(3), xiParamsNew(3); // fresh zeros per variation, as legacy

        // MC first: its resolution parameters seed the data fit.
        RooFitHistMC(oldTreeMC, " ", "", &nominal_yieldMC, &nominal_yieldMC_err, xiParamsOld, spec.weight, spec.fitDir);
        RooFitHist(oldTree, " ", "", &nominal_yield, &nominal_yield_err, xiParamsOld, spec.weight, spec.fitDir);
        RooFitHistMC(newTreeMC, " ", "recon_" + spec.name + "_" + id, &yieldMC, &yieldMC_err, xiParamsNew,
                     spec.weight, spec.fitDir);
        RooFitHist(newTree, " ", "data_" + spec.name + "_" + id, &yield, &yield_err, xiParamsNew, spec.weight,
                   spec.fitDir);

        const double pct_diff = std::fabs(nominal_yield - yield) / nominal_yield * 100;
        const double pct_diff_mc = std::fabs(nominal_yieldMC - yieldMC) / nominal_yieldMC * 100;
        std::cout << "Root File Processed: " << spec.name << "\n"
                  << "Nominal Yield:  " << nominal_yield << "\t" << nominal_yieldMC << "\n"
                  << "Variation Yield:  " << yield << "\t" << yieldMC << "\n"
                  << "Pct. Diff.(<10%):  " << pct_diff << "%\t" << pct_diff_mc << "%\n";
        AppendYields(spec.yields, spec.name, id,
                     {nominal_yield, yield, pct_diff, nominal_yieldMC, yieldMC, pct_diff_mc});
        std::cout << std::endl;
    }
}

} // namespace barlow
} // namespace gxana
