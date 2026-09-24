#include "FitFunctions.h"
using namespace RooFit;

std::string constructFitString(const std::string& fitType, const std::unordered_map<std::string, std::vector<double>>& params) {
    std::ostringstream oss;
    oss << fitType << "::xisignal(decayxim_M";
    for (const auto& param : params) {
        oss << ", " << param.first << "[" << param.second[0] << ", "
            << param.second[1] << ", " << param.second[2] << "]";
    }
    oss << ")";
    return oss.str();
}

std::string constructFitStringData(const std::string& fitType, const std::unordered_map<std::string, std::vector<double>>& params) {
    std::ostringstream oss;
    oss << fitType << "::xisignal(decayxim_M";
    for (const auto& param : params) {
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
    std::cout << "EnBin: (" << flux->FindBin(stod(emin)) <<","<< flux->FindBin(stod(emax))-1 << ")" << std::endl;
    yieldF = flux->IntegralAndError(flux->FindBin(stod(emin)), flux->FindBin(stod(emax))-1, yieldF_err);

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
    std::cout << "EnBin: (" << flux->FindBin(stod(emin)) <<","<< flux->FindBin(stod(emax))-1 << ")" << std::endl;
    yieldF = flux->IntegralAndError(flux->FindBin(stod(emin)), flux->FindBin(stod(emax))-1, yieldF_err);//last bin is exclusiv so n-1

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

//
TH1D* GetFluxHist(std::string filename)
{
    std::string fluxDir = "/d/grid17/hjesse/AnalysisNote/fluxFiles/";
    TH1D* tag_flux;
    //
    if(filename.find("2017-01") < filename.length() )
      {
        TFile *fluxFile = TFile::Open((fluxDir+"flux_30274_31057_r4.root").c_str(), "READ"); 
        tag_flux = (TH1D*)fluxFile->Get("tagged_flux");
      }
    else if(filename.find("2018-01") < filename.length())
      {
        TFile *fluxFile = TFile::Open((fluxDir+"flux_40856_42559.root").c_str(), "READ"); 
        tag_flux = (TH1D*)fluxFile->Get("tagged_flux");
      }
    else if(filename.find("2018-08") < filename.length())
      {
        TFile *fluxFile = TFile::Open((fluxDir+"flux_50685_51768.root").c_str(), "READ"); 
        tag_flux = (TH1D*)fluxFile->Get("tagged_flux");
      }
    else{
        std::cerr << "[error] Cannot find correct flux file..." << std::endl;
        exit(-1);
    }    

    return tag_flux;
}

bool AttemptFit(RooWorkspace* w, RooDataSet* data, std::unordered_map<std::string,std::vector<double>> &params, double lowerBound, double upperBound) {

    w->var("decayxim_M")->setRange("signal", lowerBound, upperBound);
    
    RooFitResult* fitResult =
        w->pdf("model")->fitTo(*data,
                               Extended(true), EvalErrorWall(true),
                               SumW2Error(false),RecoverFromUndefinedRegions(10),
                               //RooFit::AsymptoticError(true),
                               Hesse(false),  Range("signal"),
                               PrintLevel(-1), Save(true));

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

bool AttemptFitMC(RooWorkspace* w, RooDataSet* data, std::unordered_map<std::string,std::vector<double>> &params) {
    RooFitResult* fitResult = w->pdf("model")->fitTo(*data, SumW2Error(false), Hesse(false), PrintLevel(-1), Range("signal"), Save(true));

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

void RooFitMC(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err,std::string fitType, std::unordered_map<std::string,std::vector<double>> &params, std::string hist_weight, int max_retries)
{
    TH1::AddDirectory(kFALSE);
    std::string saveDir = "/d/grid17/hjesse/AnalysisNote/xsection/fits/"+delim[0]+"/";
    gSystem->mkdir(saveDir.c_str());
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
    //w->factory("Gaussian::xisignal(decayxim_M, mean[1.3217,1.32,1.33], sigma[0.005,0.002,0.01])");
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
    //topPad->SetLogy();
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
    //w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("signal"));

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

    if (!delim.empty())
        fitCan->Print( (saveDir+"recon_"+delim[1]+"_"+delim[2]+".pdf").c_str());
    fitCan->Close();

    delete w;
}
    
void RooFitData(TTree* treeData, std::string histTitle, std::vector<std::string> delim, double *yield, double *yield_err, std::string fitType, std::unordered_map<std::string, std::vector<double>> &params, int chebyOrder, std::string hist_weight, int max_retries){

    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    std::string saveDir = "/d/grid17/hjesse/AnalysisNote/xsection/fits/"+delim[0]+"/";
    gSystem->mkdir(saveDir.c_str());
    //Import dataset to plot
    double max_mass=1.45; double small = 1e-4;
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.45);
    //Weighted fit
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    TH1* dataHist = (TH1*)data->createHistogram("decayxim_M",200,1.25,1.45)->Clone(delim[2].c_str());
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
        //delete w;
        //return;
    }

    //cout << "Fit Status Code: " << fitResult->status() << std::endl;
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
  
    //fitCan->SetLogy();
        
    data->plotOn(massframe, Binning(30, 1.27, 1.45), Name("data"));
    w->pdf("model")->paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.92), ShowConstants(true));
    w->pdf("model")->plotOn(massframe, LineWidth(4), Name("model"), Range("fitrange"));
    w->pdf("model")->plotOn(massframe, Components("xisignal"), DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Range("fitrange"));//
    w->pdf("model")->plotOn(massframe,  Components("bkgd"), LineStyle(kDotted), Range("fitrange"));

    //set up specific frame styling
    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505);
    //massframe->SetMinimum(0.1);
  
    massframe->DrawClone();

    bottomPad->cd();
    bottomPad->SetGrid(0,1);
    RooHist* resPlot = massframe->residHist("data","model",true);
    resPlot->SetTitle(Form("; %s;#sigma_{#scale[0.7]{pull}}",massframe->GetXaxis()->GetTitle()));
    double ymax = TMath::MaxElement(resPlot->GetN(),resPlot->GetY());
    double ymin = TMath::MinElement(resPlot->GetN(),resPlot->GetY());
    
    double ysym = abs(ymax) >  abs(ymin) ?
        abs(ymax) : abs(ymin);
    //if(ysym<5) ysym=5;
        
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
    
    if(!delim.empty())
        fitCan->Print( (saveDir+"data_"+delim[1]+"_"+delim[2]+".pdf").c_str());
    fitCan->Close();
    //w->writeToFile("model.root");
}

void setStyle()
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
