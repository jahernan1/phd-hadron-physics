#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <RooRealVar.h>
#include <RooDataHist.h>
#include <RooHistPdf.h>
#include <RooPlot.h>
#include <RooFitResult.h>
#include <RooAddPdf.h>
#include <TCanvas.h>
using namespace RooFit;

// Function to retrieve the histogram PDF from the Monte Carlo file
RooHistPdf* getHistogramPDF(const char* simFile, const char* simTree, RooRealVar& mass) {
    TFile* fSim = TFile::Open(simFile);
    if (!fSim || fSim->IsZombie()) {
        std::cerr << "Error: Cannot open simulation file!" << std::endl;
        return nullptr;
    }
    TTree* tSim = (TTree*)fSim->Get(simTree);
    if (!tSim) {
        std::cerr << "Error: Cannot find simulation tree!" << std::endl;
        return nullptr;
    }
    
    TH1D* hSim = new TH1D("hSim", "Simulated Data Histogram", 130, 1.27, 1.4);
    tSim->Draw("decayxim_M>>hSim", "hybrid_combo");
    RooDataHist* simDataHist = new RooDataHist("simDataHist", "Simulated Data Histogram", RooArgSet(mass), hSim);
    
    RooHistPdf* histPdf = new RooHistPdf("histPdf", "Histogram PDF", RooArgSet(mass), *simDataHist, 2);
    return histPdf;
}

// Function to fit the data using the retrieved histogram PDF
void fitWithHistogramPDF(const char* dataFile, const char* dataTree, RooHistPdf* histPdf) {
    // Define variable
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.40);
    RooRealVar weight("hybrid_combo", "weight", -1, 1);
    
    // Background model
    RooRealVar a0("a0", "a0", 0.5, 0., 1.);
    RooRealVar a1("a1", "a1", 0.2, 0., 1.);
    RooChebychev bkg("bkg", "Background", mass, RooArgSet(a0, a1));
    
    // Associated number of signal/background events
    RooRealVar nsig("nsig", "number of signal events", 500, 0., 10000);
    RooRealVar nbkg("nbkg", "number of background events", 500, 0, 10000);
    
    // Build total model
    RooAddPdf model("model", "bkg+histPdf", RooArgList(bkg, *histPdf), RooArgList(nbkg, nsig));
    
    // Load data
    TFile fData(dataFile);
    TTree* tData = (TTree*)fData.Get(dataTree);
    RooDataSet* dataSet = new RooDataSet("data", "Dataset", RooArgSet(mass, weight), Import(*tData), WeightVar(weight));
    
    // Fit model to data
    RooFitResult* fitResult = model.fitTo(*dataSet, SumW2Error(false), Hesse(false), PrintLevel(-1), Save(true));
    fitResult->Print();
    
    // Plot results
    TCanvas* c1 = new TCanvas("c1", "Fit Results", 800, 600);
    RooPlot* frame = mass.frame();
    dataSet->plotOn(frame);
    histPdf->plotOn(frame, LineStyle(kDashed));
    model.plotOn(frame);
    model.plotOn(frame, Components("histPdf"), LineColor(kRed));
    model.plotOn(frame, Components("bkg"), LineColor(kMagenta));
    frame->Draw();
    c1->SaveAs("fit_result.pdf");
}

int RooHistPdfFitTest_1() {
    string rootFile = "/d/grid17/hjesse/AnalysisNote/flatTrees/";
    string dataTreeName = "flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap.root";
    string mcTreeName = "flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root";
    
    // Define mass variable for histogram extraction
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.40);
    RooHistPdf* histPdf = getHistogramPDF((rootFile + mcTreeName).c_str(), "flatTree_kpkpxim", mass);
    
    if (histPdf) {
        fitWithHistogramPDF((rootFile + dataTreeName).c_str(), "flatTree_kpkpxim", histPdf);
    }
    
    return 0;
}
