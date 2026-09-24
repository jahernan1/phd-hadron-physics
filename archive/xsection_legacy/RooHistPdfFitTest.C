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

void fitWithHistogramPDF(const char* simFile, const char* simTree, const char* dataFile, const char* dataTree) {
    // Define variable
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.40);
    RooRealVar weight("hybrid_combo", "weight", -1, 1);
    // Build Chebychev polynomial pdf
    RooRealVar a0("a0", "a0", 0.5, 0., 1.);
    RooRealVar a1("a1", "a1", 0.2, 0., 1.);
    RooChebychev bkg("bkg", "Background", mass, RooArgSet(a0, a1));
    // Associated nsig/nbkg as expected number of events with sig/bkg _in_the_range_ "signalRange"
    RooRealVar nsig("nsig","number of signal events in signalRange",500,0.,10000) ;
    RooRealVar nbkg("nbkg","number of background events in signalRange",500,0,10000) ; 
    // Load simulated data
    TFile* fSim = TFile::Open(simFile);
    TTree* tSim = (TTree*)fSim->Get(simTree);
    TH1D* hSim = new TH1D("hSim", "Simulated Data Histogram", 130, 1.27, 1.4);
    tSim->Draw("decayxim_M>>hSim","hybrid_combo");
    // Convert histogram to RooFit object
    RooDataHist* simDataHist = new RooDataHist("simDataHist", "Simulated Data Histogram", RooArgSet(mass), hSim);
    RooHistPdf histPdf("histPdf", "Histogram PDF", RooArgSet(mass), *simDataHist,2);

    // Use AddPdf to extend the model. Giving as many coefficients as pdfs switches
   // on extension.
   RooAddPdf  model("model","g1+g2", RooArgList(bkg,histPdf), RooArgList(nbkg,nsig)) ;
    
    // Load data to fit
    TFile fData(dataFile);
    TTree* tData = (TTree*)fData.Get(dataTree);
    RooDataSet* dataSet = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*tData), WeightVar(weight));
        
    // Perform the fit
    RooFitResult* fitResult = model.fitTo(*dataSet, SumW2Error(false), Hesse(false), PrintLevel(-1), Save(true));
    fitResult->Print();

    // Plot results
    TCanvas* c1 = new TCanvas("c1", "Fit Results", 800, 600);
    RooPlot* frame = mass.frame();
    dataSet->plotOn(frame);
    histPdf.plotOn(frame, LineStyle(kDashed));
    model.paramOn(frame);
    model.plotOn(frame);
    model.plotOn(frame,Components("histPdf"),LineColor(kRed));
    model.plotOn(frame,Components("bkg"),LineColor(kMagenta));
    frame->Draw();
    //frame->SetMinimum(0.01);
    //gPad->SetLogy();
    c1->SaveAs("fit_result.pdf");
}

int RooHistPdfFitTest()
{
    string rootFile = "/d/grid17/hjesse/AnalysisNote/flatTrees/";
    string dataTreeName = "flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap.root";
    string mcTreeName = "flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root";
    fitWithHistogramPDF( (rootFile+mcTreeName).c_str(), "flatTree_kpkpxim", (rootFile+dataTreeName).c_str(), "flatTree_kpkpxim");    
    
    return 0;
}
