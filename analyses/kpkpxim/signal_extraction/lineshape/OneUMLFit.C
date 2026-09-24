#include "gxana/common/Paths.h"
void setStyle();
using namespace RooFit;


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

// Helper function to determine the last significant digit step size
double getStepSize(double value) {
    std::string str = std::to_string(value);
    size_t pos = str.find_last_not_of("0");
    if (pos != std::string::npos && pos > 0 && str[pos] == '.') {
        --pos;
    }
    double step = std::pow(10, std::floor(std::log10(std::stod(str.substr(0, pos+1)))) - 1);
    return step;
}

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

bool AttemptFitMC(RooWorkspace* w, RooDataSet* data, std::unordered_map<std::string,std::vector<double>> &params) {
    RooFitResult* fitResult = w->pdf("model")->fitTo(*data, SumW2Error(false), Hesse(false), PrintLevel(-1), Range("signal"), Save(true));

    if (fitResult == nullptr || fitResult->status() != 0) {
        cerr << "Fit failed with status: " << (fitResult ? fitResult->status() : -1) << endl;
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
            cout << p->getTitle() << endl;
            cout << p->getVal() << " +/- " << p->getError() << endl;
            if(params.find(par) != params.end())
                params[par][0] = p->getVal();
        }
    
    delete fitResult;
    return true;
}

void RooFitHistMC(TTree* treeData, std::string histTitle, std::string delim, std::unordered_map<std::string,std::vector<double>> &params, std::string hist_weight, int max_retries = 5)
{
    TH1::AddDirectory(kFALSE);
    //std::string saveDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/fits/")+delim[0]+"/";
    //gSystem->mkdir(saveDir.c_str());
    // Set up workspace and data
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.40);
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    mass.setRange("signal", 1.27, 1.38);

    w->import(RooArgSet(mass));

    // Take all signal events in MC
    std::cout << "MC Events: " << data->sumEntries() << std::endl; 

    // Build model with initial parameters from params std::vector
    std::string signalStr = constructFitString("Johnson", params);
    cout << signalStr << endl;
    w->factory(signalStr.c_str());
    w->factory("SUM::model(nxi[1000,1,1e6]*xisignal)");

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

    // Plot model and data
    RooPlot* massframe = mass.frame(Title(histTitle.c_str()));
    TCanvas* fitCan = new TCanvas("fitCanMC", "mc", 800, 700);
    fitCan->SetLogy();

    data->plotOn(massframe, Name("datapnts"), Binning(60, 1.27, 1.42));
    w->pdf("model")->paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.92));
    w->pdf("model")->plotOn(massframe, LineWidth(3), Range("signal"));
    w->pdf("model")->plotOn(massframe,Components("xisignal"), DrawOption("F"), FillColor(kBlue - 9), FillStyle(3001), MoveToBack(), Range("signal"));
    //w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("signal"));

    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505, kFALSE);
    massframe->SetMinimum(0.1);
    massframe->Draw();

    fitCan->SetGrid();
    //if (!delim.empty())
        //fitCan->Print( (saveDir+"recon_"+delim[1]+"_"+delim[2]+".pdf").c_str());
    //fitCan->Close();

    delete w;
}

bool AttemptFit(RooWorkspace* w, RooDataSet* data, std::unordered_map<std::string, std::vector<double>> &params, double lowerBound, double upperBound) {

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
        cerr << "Fit failed with status: " << (fitResult ? fitResult->status() : -1) << endl;
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

    
void RooFitHist(TTree* treeData, std::string histTitle,std::string delim, std::unordered_map<std::string, std::vector<double>> &params, std::string hist_weight="hybrid_combo", int max_retries=5){

    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    std::string saveDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/fits/")+hist_weight+"/";
    gSystem->mkdir(saveDir.c_str());
    //Import dataset to plot
    double max_mass=1.45; double small = 1e-4;
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.45);
    //Weighted fit
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    TH1* dataHist = (TH1*)data->createHistogram("decayxim_M")->Clone(delim.c_str());
    double min_mass = dataHist->GetXaxis()->GetBinLowEdge(dataHist->FindFirstBinAbove(small,1,1, dataHist->FindBin(1.3)));
    
    while(dataHist->GetBinContent(dataHist->FindBin(max_mass)) < small)
        max_mass = max_mass - dataHist->GetBinWidth(1)/3;
    while(dataHist->GetBinContent(dataHist->FindBin(min_mass)) < small && min_mass<1.28)
        min_mass = min_mass + dataHist->GetBinWidth(1)/3;


    mass.setRange("fitrange", min_mass, max_mass);
    const double rangeExpandStep = 0.005;
    
    //Set up workspace
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    w->import(RooArgSet(mass));

    //Build model and Fit data
    std::string signalStr = constructFitString("Johnson", params);
    //cout << signalStr << endl;
    w->factory(signalStr.c_str());
    w->factory("Chebychev::bkgd(decayxim_M,{a0[0.81,1e-3,1.25],a1[-0.1,-3.,-1e-3]})");//,a1[-0.1,-2,-1e-2]
    
    w->factory("SUM::model( nxi[2000,1,1e6]*xisignal, nbkgd[2000,1,1e6]*bkgd)"); //nbkgd[200,1,1e6]*bkgd,
    
    // Attempt fit up to max_retries times
    int attempt = 1;
    while (attempt <= max_retries) {
        cout << "Attempt " << attempt << " to fit data." << endl;
        if (AttemptFit(w, data, params, min_mass, max_mass)) {
            break;  // Successful fit
        }
        // Expand the fit range slightly on each retry
        // min_mass = std::max(min_mass, min_mass + rangeExpandStep);  // Ensuring it doesn't go below 1.27
        // max_mass = std::min(max_mass, max_mass - rangeExpandStep);  // Ensuring it doesn't go above 1.40
        attempt++;
    }

    // Check if fit was ultimately unsuccessful
    if (attempt > max_retries) {
        cout << "Max retries reached. Fit did not converge." << endl;
        //delete w;
        //return;
    }

    //cout << "Fit Status Code: " << fitResult->status() << endl;
    //*yield = w->var("nxi1")->getVal() + w->var("nxi2")->getVal();
    //*yield_err = sqrt(*yield);
    
    //Plot model and data 
    RooPlot* massframe = mass.frame( Title( histTitle.c_str() ) );
    TCanvas* fitCan = new TCanvas("fitCan"," c", 800, 700);
    //fitCan->SetLogy();
        
    data->plotOn(massframe, Name("datapnts"), Binning(50, 1.27, 1.45));
    w->pdf("model")->paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.92));
    w->pdf("model")->plotOn(massframe, LineWidth(4), Name("model"), Range("fitrange"));
    w->pdf("model")->plotOn(massframe, Components("xisignal"), DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(),Name("xisignal"), Range("fitrange"));//
    w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("fitrange"));

    //set up specific frame styling
    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505);
    //massframe->SetMinimum(0.1);
  
    massframe->DrawClone();
  
    fitCan->SetGrid();
    //fitCan->Print("fit.pdf");
    //if(!delim.empty())
        //fitCan->Print( (saveDir+"data_"+delim[1]+"_"+delim[2]+".pdf").c_str());
    //fitCan->Close();
    //w->writeToFile("model.root");
}

void GetXim1320_IM(string filename, string treeName) {
    setStyle();
    // Initialize variables
    string tree_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
    std::unordered_map<std::string,std::vector<double>> xiJohnsonParams;
    xiJohnsonParams["delta"] = {1.2,0.2,5.0};
    xiJohnsonParams["gamma"] = {-0.01,-1.0,1.0};
    xiJohnsonParams["lambda"] = {0.004,0.002,0.008};
    xiJohnsonParams["mu"] = {1.3217,1.32,1.33};
    
	//make data frame
    TFile* dataFile = TFile::Open((tree_dir+"binned_"+filename+"_nominal_kphighrap.root").c_str(), "READ");
    TFile* dataFileMC = TFile::Open((tree_dir+"binned_"+filename+"_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root").c_str(), "READ");
    TTree* dataTree =  (TTree*)dataFile->Get(treeName.c_str());
    TTree* dataTreeMC =  (TTree*)dataFileMC->Get(treeName.c_str());
 
    //Fit Data and MC for yields
    string tmin = treeName.substr(treeName.find("tmin")+5,4);
    string tmax = treeName.substr(treeName.find("tmax")+5,4);
    string histTitle = "#bf{-t: ("+tmin+", "+tmax+")} ;M(#Lambda#pi^{-}) (GeV); Events";

    RooFitHistMC(dataTreeMC, histTitle, "recon_"+filename.substr(0,filename.find_last_of(".")), xiJohnsonParams, "hybrid_combo");
    //cout << "MC params extracted (lambda, delta): (" << xiParams[0] << ", " << xiParams[1] << ")" << endl;
    RooFitHist(dataTree, histTitle, filename.substr(0,filename.find_last_of(".")), xiJohnsonParams);
}

int OneUMLFit()
{
    GetXim1320_IM("flatTree_kpkpxim__M23_2017-01_ana56", "emin_9.26_emax_10.18_tmin_1.19_tmax_1.53");
    //GetXim1320_IM("kpkpxim__B4_M23_2018-01_ana03_nominal_rapidityCuts_Emin-7.40_Emax-7.86.root");
    
    return 0;
}
