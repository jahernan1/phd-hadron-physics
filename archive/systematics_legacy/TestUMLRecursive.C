using namespace RooFit;

void RooFitHistMC(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> &params, string hist_weight, int max_retries = 5, int attempt = 1)
{
    TH1::AddDirectory(kFALSE);
    string saveDir = "/d/grid17/hjesse/AnalysisNote/systematics/fits/";

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

    RooFitResult* fitResult = w->pdf("model")->fitTo(*data, SumW2Error(false), Hesse(false), PrintLevel(-1), Range("signal"), Save(true));

    fitResult->Print();
    // Store results 
    *yield = w->var("nxi")->getVal();
    *yield_err = w->var("nxi")->getError();
    params[0] = w->var("lambda")->getVal();
    params[1] = w->var("delta")->getVal();
    params[2] = w->var("gamma")->getVal();

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
    // if (!delim.empty())
    //     fitCan->SaveAs((saveDir + delim + ".pdf").c_str());
    // fitCan->Close();

    // Check if fitResult is null before using it
    if (fitResult == nullptr || fitResult->status() != 0) {
        cout << "Fit attempt " << attempt << " failed. Retrying with updated parameters." << endl;

        // Update parameters based on the current fit for the next attempt
        if (fitResult != nullptr) { // Check to ensure fitResult is valid
            fitResult->Print();
            params[0] = w->var("lambda")->getVal();
            params[1] = w->var("gamma")->getVal();
            params[2] = w->var("delta")->getVal();
        }

        delete w;  // Make sure to delete the workspace before retrying
        if (fitResult != nullptr) delete fitResult;
        
        RooFitHistMC(treeData, histTitle, delim, yield, yield_err, params, hist_weight, max_retries, attempt + 1);
        return;
    }
}

void RooFitHist(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> &params, string hist_weight, int max_retries = 5, int attempt = 1)
{
    if (attempt > max_retries) {
        cout << "Max retries reached. Fit did not converge." << endl;
        return;
    }

    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    string saveDir = "/d/grid17/hjesse/AnalysisNote/systematics/fits/";
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
    mass.setRange("fitrange", min_mass, max_mass);
  
    //Set up workspace
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    w->import(RooArgSet(mass));

    //Build model and Fit data
    w->factory("Chebychev::bkgd(decayxim_M,{a0[0.81,0.01,2.],a1[-0.21,-2.,-0.01]})");//,a1[-0.1,-2,-1e-2]
    w->factory(Form("Johnson::xisignal(decayxim_M, mu[1.3217,1.32,1.33], lambda[%f], gamma[%f], delta[%f])", params[0], params[1], params[2]));
    //w->factory("Johnson::xisignal(decayxim_M,mu[1.3127,1.321,1.323],lambda[0.005,0.003,0.008], gamma[0], delta[0.8,0.3,2])");
    w->factory("SUM::model( nxi[2000,1,1e6]*xisignal, nbkgd[2000,1,1e6]*bkgd)"); //nbkgd[200,1,1e6]*bkgd,
    RooFitResult* fitResult = w->pdf("model")->fitTo(*data,
                               Extended(true), //EvalErrorWall(false),
                               SumW2Error(false),RecoverFromUndefinedRegions(10),
                               //RooFit::AsymptoticError(true),
                               Hesse(false), Range("fitrange"),
                                                     PrintLevel(-1), Save(true));

    //cout << "Fit Status Code: " << fitResult->status() << endl;
    fitResult->Print();
    *yield = w->var("nxi")->getVal();
    *yield_err = w->var("nxi")->getError();
    params[0] = w->var("lambda")->getVal();
    params[1] = w->var("delta")->getVal();
    params[2] = w->var("gamma")->getVal();
        
    //Plot model and data 
    RooPlot* massframe = mass.frame( Title( histTitle.c_str() ) );
    TCanvas* fitCan = new TCanvas("fitCan"," c", 800, 700);
        
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
    // if(!delim.empty())
    //     fitCan->Print( (saveDir+delim+".pdf").c_str());
    // fitCan->Close();
    w->writeToFile("model.root");

    // Check if fitResult is null before using it
    if (fitResult == nullptr || fitResult->status() != 0) {
        cout << "Fit attempt " << attempt << " failed. Retrying with updated parameters." << endl;

        // Update parameters based on the current fit for the next attempt
        if (fitResult != nullptr) { // Check to ensure fitResult is valid
            fitResult->Print();
            params[0] = w->var("lambda")->getVal();
            params[1] = w->var("gamma")->getVal();
            params[2] = w->var("delta")->getVal();
        }

        delete w;  // Make sure to delete the workspace before retrying
        if (fitResult != nullptr) delete fitResult;
        
        RooFitHistMC(treeData, histTitle, delim, yield, yield_err, params, hist_weight, max_retries, attempt + 1);
        return;
    }
}

int TestUMLRecursive()
{
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    string saveDir = "/d/grid17/hjesse/AnalysisNote/systematics/root_trees/";
    string root_file_name = "flatTree_kpkpxim__M23_2017-01_ana56";
    string delim = "chisqndf_6";
    string variation = delim.substr(0,delim.find_last_of("_"));

    // Retrieve the new TTree
    TFile *file = TFile::Open((saveDir+root_file_name+"_"+variation+"_variations.root").c_str(), "READ");  // Use "UPDATE" mode to modify existing file
    TTree* newTree = (TTree*)file->Get(("vary_"+delim).c_str());
    TTree* newTreeMC = (TTree*)file->Get(("vary_"+delim+"_mc").c_str());
    
    // Get Value of events after cut
    double yield, yield_err,yieldMC, yieldMC_err;
    double nominal_yield, nominal_yield_err, nominal_yieldMC, nominal_yieldMC_err;
    
    // get yield of nominal tree
    vector<double> xiParamsNew{0.0045,-0.01,1};

    // Perfrom fits, make sure MC fit is first; to use reosultion params in data
    RooFitHistMC(newTreeMC, " ","recon_"+root_file_name+"_"+delim, &yieldMC, &yieldMC_err, xiParamsNew, "hybrid_combo");
    RooFitHist(newTree, " ","data_"+root_file_name+"_"+delim, &yield, &yield_err, xiParamsNew, "hybrid_combo");
    
    return 1;
}
