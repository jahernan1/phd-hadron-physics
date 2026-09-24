#include "gxana/common/Paths.h"
void RooFitHist(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> params, string hist_weight);
using namespace RooFit;

void weighted_unbinned_fit(string cutSet="kphigh_p4.Rapidity()>2 && kplow_p4.Rapidity()>0")
{    
    auto df = ROOT::RDataFrame("flatTree_kpkpxim","test_tree.root")
        .Filter(cutSet.c_str());

    // Use Snapshot to write the TTree
    ROOT::RDF::RSnapshotOptions opts;
    opts.fMode = "UPDATE";
	opts.fOverwriteIfExists=true;
    df.Snapshot("newTree", "test_tree.root","",opts);

    // Step 5: Retrieve the new TTree from the in-memory directory
    TFile *file = TFile::Open("test_tree.root", "READ");  // Use "UPDATE" mode to modify existing file
    TTree* newTree = (TTree*)file->Get("newTree");
    
    double yield, yield_err;
    RooFitHist(newTree, "","", &yield, &yield_err, {0.004,1.1,0}, "hybrid_combo");

    // Cleanup
    file->Close();
}

void RooFitHist(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> params, string hist_weight)
{
    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    //Import dataset to plot
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.26, 1.46);
    //Weighted fit
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    //RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass), Import(*treeData));
    TH1* dataHist = (TH1*)data->createHistogram("decayxim_M")->Clone(delim.c_str());
    Double_t min_mass = dataHist->GetXaxis()->GetBinLowEdge(dataHist->FindFirstBinAbove(1,1,1, dataHist->FindBin(1.32)));
    double max_mass=1.45;
    while(dataHist->GetBinContent(dataHist->FindBin(max_mass)) < 0)
        max_mass = max_mass - dataHist->GetBinWidth(1);
    mass.setRange("fitrange", min_mass, max_mass);
  
    //Set up workspace
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooPlot* massframe = mass.frame( Title( histTitle.c_str() ) );
    TCanvas* fitCan = new TCanvas("fitCan"," c", 800, 700);
    //fitCan->SetLogy();
    w->import(RooArgSet(mass));

    //Build model and Fit data
    w->factory("Chebychev::bkgd(decayxim_M,{a0[0.8,0.1,2.],a1[-0.2,-2.,-0.1]})");//,a1[-0.1,-2,-1e-2]
    //w->factory(("Johnson::xisignal(decayxim_M,mu[1.322,1.321,1.323],lambda["+to_string(params[0])+","+to_string(params[0])+",0.008], gamma["+to_string(params[2])+"], delta["+to_string(params[1])+"])").c_str());
    w->factory("Johnson::xisignal(decayxim_M,mu[1.322,1.321,1.323],lambda[0.005,0.003,0.008], gamma[0], delta[0.8,0.5,2])");
    // w->factory("Johnson::xisignal(decayxim_M,mu[1.322,1.321,1.323],lambda[0.006,0.004,0.008], gamma[0], delta[1.5,1.,3.])");
    w->factory("SUM::model( nxi[120,0,1e6]*xisignal, nbkgd[200,0,1e6]*bkgd)");// nbkgd[200,1,1e6]*bkgd,
    w->pdf("model")->fitTo(*data,
                           Extended(true), //EvalErrorWall(false),
                           SumW2Error(false),RecoverFromUndefinedRegions(10), 
                           //RooFit::AsymptoticError(true), 
                           Hesse(false), Range("fitrange"),
                           PrintLevel(-1),PrintEvalErrors(-1),Verbose(false));
  
    //Plot model and data 
    data->plotOn(massframe, Name("datapnts"), Binning(60, 1.26, 1.48));
    w->pdf("model")->paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.92), Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("lambda"), *w->var("nbkgd"))));
    w->pdf("model")->plotOn(massframe, LineWidth(4), Name("model"), Range("fitrange"));
    w->pdf("xisignal")->plotOn(massframe, DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Name("xisignal"), Range("fitrange"));//
    w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("fitrange"));
    //set up specific frame styling
    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505);
    //massframe->SetMinimum(0.1);
  
    *yield = w->var("nxi")->getVal();
    *yield_err = w->var("nxi")->getError();
    massframe->DrawClone();
  
    fitCan->SetGrid();
    fitCan->Print("fit.pdf");
    //fitCan->Print( (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/ml_fits/fits/")+delim+"_"+hist_weight+".pdf").c_str());
    fitCan->Close();
    delete fitCan, data, w;
}
