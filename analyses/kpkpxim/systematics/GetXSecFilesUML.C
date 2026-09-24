#include "gxana/common/Paths.h"

void RooFitHistMC(TTree* treeData, string histTitle, vector<string> delim, double *yield, double *yield_err, vector<double> &params, string hist_weight="hybrid_combo", int max_retries = 10);
void RooFitHist(TTree* treeData, string histTitle, vector<string> delim, double *yield, double *yield_err, vector<double> &params, string weight_name="hybrid_combo", int max_retries = 10);
bool AttemptFitMC(RooWorkspace* w, RooDataSet* data, vector<double> &params);
bool AttemptFit(RooWorkspace* w, RooDataSet* data, vector<double> &params, double lowerBound, double upperBound);
//bool AttemptFit(RooWorkspace* w, RooDataSet* data, vector<double> &params);
void setStyle();
void MakeXSecFiles(string filename);
void GetDiffXSecFile(vector<TTree*> trees, TH1D* flux, vector<string> delim, ofstream& outputFile, ofstream& xsecFile, string weight="hybrid_combo", int n_threads = 4);
void GetTotXSecFile(vector<TTree*> trees, TH1D* flux, std::vector<string> delim, ofstream& outputFile, ofstream& xsecFile, string weight="hybrid_combo", int n_threads = 4);
TH1D* GetFluxHist(string filename);
using namespace RooFit;

// flatTree_kpkpxim__B4_M23_2018-01_ana03_chisqndf_variations.root
int GetXSecFilesUML(){
    setStyle();
    RooMsgService::instance().setGlobalKillBelow(ERROR);

    std::vector<std::string> cuts = {"chisqndf",
                                     "total_mm2_abs",
                                     "xim_pathlensig",
                                     "lambda_pathlensig",
                                     "kphigh_prap",
                                     //"kplow_prap"
    };

    for(int i = 0; i < cuts.size(); ++i){
        MakeXSecFiles("flatTree_kpkpxim__M23_2017-01_ana56_"+cuts[i]+"_gen_amp_V2_ac_YstarRest_variations");
        MakeXSecFiles("flatTree_kpkpxim__B4_M23_2018-01_ana03_"+cuts[i]+"_gen_amp_V2_ac_YstarRest_variations");
        MakeXSecFiles("flatTree_kpkpxim__B4_M23_2018-08_ana02_"+cuts[i]+"_gen_amp_V2_ac_YstarRest_variations");
    }
    
    return 0;
}

void MakeXSecFiles(string filename)
{
    //Set up variables
    string logDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/xsection_data/");
    string rootFileDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/variation_trees/");
    string thrownFileDir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
    string fluxDir = gxana::EnvPath("GXANA_DATA", "flux/");
    cout << "Storing data files to:\n" << logDir << endl;
    string name = filename.substr(0,filename.find("ana")+5);
    std::vector<string> delim(3); delim[0] = name;
    
    // Get the tfiles for the cross section
    cout << "Processing cross section output for: " << filename << endl;
    TFile *dataFile = TFile::Open( (rootFileDir+"binned_"+filename+".root").c_str(), "READ");
    TFile *thrownFile = TFile::Open( (thrownFileDir+"binned_thrown_"+name+"_gen_amp_V2_ac_YstarRest.root").c_str(), "READ");
    TH1D* tag_flux = (TH1D*)GetFluxHist(filename)->Clone("tagged_flux");
    //tag_flux->Draw();
    // Iterate over all keys in the root file
    TIter nextKey(dataFile->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)nextKey())) {//loop over all variations in file
        // Check if the key is a TDirectory
        if (std::string(key->GetClassName()) == "TDirectoryFile"){
            std::string dirName = key->GetName();
            delim[1] = dirName;
            // Skip MC dir because i will call its trees in this same loop
            if(dirName.find("_mc")!=std::string::npos)
                continue;

            // Set up files for total xsection
            std::ofstream tot_outf( (logDir+"totout_"+name+"_"+dirName+".txt").c_str() );
            tot_outf << "enBinCenter\t" << "EnErr\t" << "deltaE\t" << "data_yield\t"
                     << "yield_err\t" << "mc_yield\t" << "mc_err\t" << "thrown_yield\t"
                     << "thrown_err\t" << "accept\t" << "accept_err\t" << "flux\t"
                     << "flux_err\t" << endl;
            std::ofstream totxsec_outf( (logDir+"totxsec_"+name+"_"+dirName+".txt").c_str() );
            totxsec_outf << "enBinCenter\t" << "sigma\t" << "enBinWidth\t"
                         << "Yerr\t" << endl;
                        
            std::cout << "Processing Trees in TDirectory: " << dirName << std::endl;
            // Get the TDirectory object
            TDirectory* dir = (TDirectory*)dataFile->Get(dirName.c_str());
            TDirectory* dirMC = (TDirectory*)dataFile->Get( (dirName+"_mc").c_str());
            if (!dir) {
                std::cerr << "Failed to get directory: " << dirName << std::endl;
                return;
            }

            // Now iterate over keys in the directory to find TTrees
            TIter nextTree(dir->GetListOfKeys());
            TKey* treeKey;
            std::ofstream diff_outf; std::ofstream diffxsec_outf;
                
            while ((treeKey = (TKey*)nextTree())) {//loop over all bins for variation
                // Check if the key is a TTree
                if (std::string(treeKey->GetClassName()) == "TTree"){
                    std::string treeName = treeKey->GetName();
                    delim[2] = treeName;
                    // Retrieve the TTree object
                    TTree* tree = (TTree*)dir->Get(treeName.c_str());
                    TTree* treeMC = (TTree*)dirMC->Get(treeName.c_str());
                    TTree* treeThrown = (TTree*)thrownFile->Get(treeName.c_str());
                     
                    if(treeName.find("tmin")==std::string::npos){//full energy bin
                        std::cout << "  TDirectory: " << dirName << "\n"
                                  << " Processing TTree: " << treeName << std::endl;

                        // Get total xsec
                        GetTotXSecFile({tree,treeMC,treeThrown}, tag_flux,
                                       delim, tot_outf, totxsec_outf);

                        // Set diffxsec output files with headers
                        if(diffxsec_outf.is_open() ){
                            diffxsec_outf.close(); diff_outf.close();}
                            
                        diff_outf.open( (logDir+"diffout_"+name+"_"+dirName+"_"+treeName+".txt").c_str());
                        diffxsec_outf.open( (logDir+"diffxsec_"+name+"_"+dirName+"_"+treeName+".txt").c_str());
                        diff_outf << "tcenter\t" << "terr\t" <<   "deltaT\t"
                                  << "data_yield\t" << "yield_err\t" << "mc_yield\t"
                                  << "mc_err\t" << "thrown_yield\t"
                                  << "thrown_err\t" << "accept accep_err\t"
                                  << "flux\t" << "flux_err"
                                  << std::endl;
                            
                        diffxsec_outf << "tBinCenter\t" << "dsigma/dt\t"
                                      << "tBinWidth\t" << "Yerr" << std::endl;
                    }
                    else if(treeName.find("tmin")!=std::string::npos){//is tbin
                        std::cout << "  TDirectory: " << dirName << "\n"
                                  << " Processing TTree: " << treeName << std::endl;
                        // Omit tbins from name to open file
                        treeName = treeName.substr(0,treeName.find("_tmin"));
                        // Get Diff Xsec
                        GetDiffXSecFile({tree,treeMC,treeThrown}, tag_flux, delim, diff_outf, diffxsec_outf);
                    }
                    else{
                        std::cerr << "Failed to get proper tree:" << treeName
                                  << std::endl;
                        return;
                    }
                }
            }
            // Close tot files
            tot_outf.close(); totxsec_outf.close();
            diff_outf.close(); diffxsec_outf.close();
        }
    }
    
    return;  // gxana: legacy `return 1;` in a void function fails to compile on ROOT 6.40's stricter cling
}

void GetDiffXSecFile(vector<TTree*> trees, TH1D* flux, vector<string> delim, ofstream& outputFile, ofstream& xsecFile, string weight="hybrid_combo", int n_threads = 4)
{
    //Initialize variables
    string tree_dir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/root_files/");
    
    //Get trees for Ebins
    Double_t yield, yield_err, yieldMC, yieldMC_err, yieldT, yieldT_err, yieldF, yieldF_err;
    Double_t diffxsec, diffxsec_err, intxsec, intxsec_err, accept, accept_err, deltaT;
    // Target values
    Double_t Na = 6.022e23; //[atoms/mol]
    Double_t tar_Len = 79.1-50.4; //[cm]
    Double_t tar_Den = 70.08e-3; // 2018 (+/- 0.0035) [g/cm^3]
    Double_t hy_MM = 2.01588; //[g/mol]
    Double_t tar_val = 2 * Na * tar_Len * tar_Den * pow(10,-24) / hy_MM;
    printf("The target value is: %f\n", tar_val); 
    
    Double_t hy_den = tar_val; Double_t BR_Lamb = 0.641; Double_t BR_Lamb_Err = 0.005;
    vector<double> xiParams = {1.3217,0.004,-0.01,1.2};
    string treeName = delim[2];

    //Get energy boundries for flux yields
    cout << treeName << endl;
    string emin = treeName.substr(treeName.find("emin")+5);
    emin = emin.substr(0,emin.find("_"));
    string emax = treeName.substr(treeName.find("emax")+5);
    emax = emax.substr(0,emax.find("_"));
    string tmin = treeName.substr(treeName.find("tmin")+5);
    tmin = tmin.substr(0,tmin.find("_"));
    string tmax = treeName.substr(treeName.find("tmax")+5);
    //tmax = tmax.substr(0,tmax.find("_"));//end of string not needed
    
    //En bins for flux
    cout << "EnBin: (" << flux->FindBin(stod(emin)) <<","<< flux->FindBin(stod(emax))-1 << ")" << endl;
    yieldF = flux->IntegralAndError(flux->FindBin(stod(emin)), flux->FindBin(stod(emax))-1, yieldF_err);

    //tbins
    cout << "TMin, TMax: (" << tmin << "," << tmax <<")"<< endl;
    deltaT = stod(tmax) - stod(tmin);
    
    //Fit Data and MC for yields
    string histTitle = "#bf{-t: ("+tmin+", "+tmax+")}";
    
    if(trees[0]->GetEntries() > 0 && trees[1]->GetEntries() > 0){
        RooFitHistMC(trees[1], histTitle, delim, &yieldMC, &yieldMC_err, xiParams);
        RooFitHist(trees[0], histTitle, delim, &yield, &yield_err, xiParams);
    
        //Get thrown yields
        yieldT = trees[2]->GetEntries(); 
        yieldT_err = sqrt(yieldT);
        cout << "Yields: " << yield << " +/- " << yield_err << endl;
        cout << "Yields MC: " << yieldMC << " +/- " << yieldMC_err << endl;
        cout << "Yields Thrown: " << yieldT << " +/- " << yieldT_err << endl;
    
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
    outputFile << (stod(tmin)+stod(tmax))/2 << "  " << deltaT/2 << "  " << deltaT << "  " << yield << "  "
                   << yield_err << "  " << yieldMC << "  " << yieldMC_err << "  " << yieldT << "  " << yieldT_err
                   << "  " << accept << "  " << accept_err << "  " << yieldF << "  " << yieldF_err
                   << std::endl;

    xsecFile << (stod(tmin)+stod(tmax))/2 << "  " << diffxsec*pow(10,9) << "  " << deltaT/2
                 << "  " << diffxsec_err*pow(10,9) << std::endl;
}

void GetTotXSecFile(vector<TTree*> trees, TH1D* flux, vector<string> delim, ofstream& outputFile, ofstream& xsecFile, string weight="hybrid_combo", int n_threads = 4)
{
    //Initialize variables
    string tree_dir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/root_files/");
    Double_t yield, yield_err, yieldMC, yieldMC_err, yieldT, yieldT_err, yieldF, yieldF_err;
    Double_t totxsec, totxsec_err, accept, accept_err, deltaT;

    // Target values
    Double_t Na = 6.022e23; //[atoms/mol]
    Double_t tar_Len = 79.1-50.4; //[cm]
    Double_t tar_Den = 70.08e-3; // 2018 (+/- 0.0035) [g/cm^3]
    Double_t hy_MM = 2.01588; //[g/mol]
    Double_t tar_val = 2 * Na * tar_Len * tar_Den * pow(10,-24) / hy_MM;
    printf("The target value is: %f\n", tar_val); 
    
    Double_t hy_den = tar_val; Double_t BR_Lamb = 0.641; Double_t BR_Lamb_Err = 0.005;
    
    vector<double> xiParams = {1.3217,0.004,-0.01,1.2};;
    string treeName = delim[2];
    
    //Get energy boundries for flux yields
    string emin = treeName.substr(treeName.find("emin")+5);
    emin = emin.substr(0,emin.find("_"));
    string emax = treeName.substr(treeName.find("emax")+5);
    //emax = emax.substr(0,emax.find("_"));//end of string not needed
    
    cout << "Emin,Emax: (" << stod(emin) <<","<< stod(emax) <<")" << endl;
    Double_t deltaE = stod(emax) - stod(emin);
    
    // Get flux value in the energy range
    cout << "EnBin: (" << flux->FindBin(stod(emin)) <<","<< flux->FindBin(stod(emax))-1 << ")" << endl;
    yieldF = flux->IntegralAndError(flux->FindBin(stod(emin)), flux->FindBin(stod(emax))-1, yieldF_err);//last bin is exclusiv so n-1

    //Fit Data and MC for yields
    string histTitle = "#bf{E_{#gamma}: ("+emin+", "+emax+")}";
    RooFitHistMC(trees[1], histTitle, delim, &yieldMC, &yieldMC_err, xiParams, weight);//mc is a weighted likelihood fit
    RooFitHist(trees[0], histTitle, delim, &yield, &yield_err, xiParams, weight);
    
    //Get thrown yields
    yieldT = trees[2]->GetEntries(); 
    yieldT_err = sqrt(yieldT);
    cout << "Yields: " << yield << " +/- " << yield_err << endl;
    cout << "Yields MC: " << yieldMC << " +/- " << yieldMC_err << endl;
    cout << "Yields Thrown: " << yieldT << " +/- " << yieldT_err << endl;
    
//Get acceptance
    accept = yieldMC / yieldT;
    accept_err = accept * sqrt( pow( yieldMC_err / yieldMC, 2  ) + pow ( yieldT_err / yieldT, 2) );
    
    //GetTotXSec
    totxsec = yield / ( hy_den * yieldF * BR_Lamb * accept );
    totxsec_err = totxsec * sqrt( pow( yield_err / yield, 2) + pow( yieldF_err / yieldF, 2) + pow( accept_err / accept, 2  ) + pow (BR_Lamb_Err / BR_Lamb, 2) );

    //Write out data to files
    outputFile <<  (stod(emax)+stod(emin))/2 << "  " <<  deltaE/2 << "  " <<  deltaE << "  "
               <<  yield << "  " <<  yield_err << "  " <<  yieldMC << "  " <<  yieldMC_err << "  "
               <<  yieldT << "  " <<  yieldT_err << "  " <<  accept << "  " <<  accept_err << "  "
               <<  yieldF << "  " <<  yieldF_err
               <<  endl;
    
    xsecFile << (stod(emin)+stod(emax))/2 << "  " <<  totxsec*pow(10,9) << "  " <<  deltaE/2 << "  "
             <<  totxsec_err*pow(10,9)
             <<  endl;
}

//
TH1D* GetFluxHist(string filename)
{
    string fluxDir = gxana::EnvPath("GXANA_DATA", "flux/");
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
    else
      {cout << "Cannot find correct flux file..." << endl;}  // gxana: legacy `<< cout` (self-insertion) does not compile on ROOT 6.40's stricter cling

    return tag_flux;
}

bool AttemptFitMC(RooWorkspace* w, RooDataSet* data, vector<double> &params) {
    RooFitResult* fitResult = w->pdf("model")->fitTo(*data, SumW2Error(false), Hesse(false), PrintLevel(-1), Range("signal"), Save(true));

    if (fitResult == nullptr || fitResult->status() != 0) {
        cerr << "Fit failed with status: " << (fitResult ? fitResult->status() : -1) << endl;
        delete fitResult;
        return false;
    }

    fitResult->Print();
    // Update parameters if fit is successful
    params[0] = w->var("mu")->getVal();
    params[1] = w->var("lambda")->getVal();
    params[2] = w->var("gamma")->getVal();
    params[3] = w->var("delta")->getVal();

    delete fitResult;
    return true;
}

void RooFitHistMC(TTree* treeData, string histTitle, vector<string> delim, double *yield, double *yield_err, vector<double> &params, string hist_weight, int max_retries = 5)
{
    TH1::AddDirectory(kFALSE);
    string saveDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/fits/");

    // Set up workspace and data
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.40);
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    mass.setRange("signal", 1.27, 1.38);

    w->import(RooArgSet(mass));

    // Build model with initial parameters from params vector
    w->factory(Form("Johnson::xisignal(decayxim_M, mu[%f,1.32,1.33], lambda[%f,0.002,0.007], gamma[%f, -0.5,0.5], delta[%f,0.2,1.5])", params[0], params[1], params[2], params[3]));
    //w->factory("EXPR::bkgd('(decayxim_M)*(((decayxim_M)/m0)**2-1.0)**p*exp(b*(((decayxim_M)/m0)**2-1.0))',decayxim_M, m0[1.2602,1.255,1.275], b[-22,-40.,-5.], p[2])");
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

    // Store results if fit was successful
    *yield = w->var("nxi")->getVal();
    *yield_err = sqrt(*yield);//w->var("nxi")->getError();

    // Plot model and data
    RooPlot* massframe = mass.frame(Title(histTitle.c_str()));
    TCanvas* fitCan = new TCanvas("fitCanMC", "mc", 800, 700);
    fitCan->SetLogy();

    data->plotOn(massframe, Name("datapnts"), Binning(60, 1.27, 1.42));
    w->pdf("model")->paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.92), Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("lambda"))));
    w->pdf("model")->plotOn(massframe, LineWidth(3), Name("model"), Range("signal"));
    w->pdf("xisignal")->plotOn(massframe, DrawOption("F"), FillColor(kBlue - 9), FillStyle(3001), MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Name("xisignal"), Range("signal"));
    //w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("signal"));

    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505, kFALSE);
    massframe->SetMinimum(0.1);
    massframe->Draw();

    fitCan->SetGrid();
    if (!delim.empty())
        fitCan->Print( (saveDir+"recon_"+delim[0]+"_"+delim[1]+"_"+delim[2]+".pdf").c_str());
    fitCan->Close();

    delete w;
}

bool AttemptFit(RooWorkspace* w, RooDataSet* data, vector<double> &params, double lowerBound, double upperBound) {

    w->var("decayxim_M")->setRange("signal", lowerBound, upperBound);
    
    RooFitResult* fitResult =
        w->pdf("model")->fitTo(*data,
                               Extended(true), //EvalErrorWall(true),
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
    params[0] = w->var("mu")->getVal();
    params[1] = w->var("lambda")->getVal();
    params[2] = w->var("gamma")->getVal();
    params[3] = w->var("delta")->getVal();

    delete fitResult;
    return true;
}

    
void RooFitHist(TTree* treeData, string histTitle, vector<string> delim, double *yield, double *yield_err, vector<double> &params, string hist_weight, int max_retries = 5){

    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    string saveDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/fits/");
    //Import dataset to plot
    double max_mass=1.45; double small = 1e-4;
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.45);
    //Weighted fit
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    TH1* dataHist = (TH1*)data->createHistogram("decayxim_M")->Clone(delim[0].c_str());
    double min_mass = dataHist->GetXaxis()->GetBinLowEdge(dataHist->FindFirstBinAbove(small,1,1, dataHist->FindBin(1.30)));
    
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
    w->factory("Chebychev::bkgd(decayxim_M,{a0[0.,0.,1.2],a1[0.,-0.5,0.2]})");//,a1[-0.1,-2,-1e-2]
    w->factory(Form("Johnson::xisignal(decayxim_M, mu[%f,1.31,1.33], lambda[%f,%f,0.01], gamma[%f], delta[%f])",params[0], params[1], params[1], params[2], params[3]));
    //w->factory("Johnson::xisignal(decayxim_M,mu[1.3127,1.321,1.323],lambda[0.005,0.003,0.008], gamma[0], delta[0.8,0.3,2])");
    w->factory("SUM::model( nxi[2000,1,1e6]*xisignal, nbkgd[2000,1,1e6]*bkgd)"); //nbkgd[200,1,1e6]*bkgd,
    
    // Attempt fit up to max_retries times
    int attempt = 1;
    while (attempt <= max_retries) {
        cout << "Attempt " << attempt << " to fit data." << endl;
        if (AttemptFit(w, data, params, min_mass, max_mass)) {
            break;  // Successful fit
        }
        // Expand the fit range slightly on each retry
        min_mass = std::max(min_mass, min_mass + rangeExpandStep);  // Ensuring it doesn't go below 1.27
        max_mass = std::min(max_mass, max_mass - rangeExpandStep);  // Ensuring it doesn't go above 1.40
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
    *yield_err = sqrt(*yield);//w->var("nxi")->getError();
    
    //Plot model and data 
    RooPlot* massframe = mass.frame( Title( histTitle.c_str() ) );
    TCanvas* fitCan = new TCanvas("fitCan"," c", 800, 700);
    //fitCan->SetLogy();
        
    data->plotOn(massframe, Name("datapnts"), Binning(50, 1.27, 1.45));
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
        fitCan->Print( (saveDir+"data_"+delim[0]+"_"+delim[1]+"_"+delim[2]+".pdf").c_str());
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


// void GetXSecFile(string file)
// {
//     //Read the data yields
//     ifstream infile;
//     infile.open(file.c_str());
//     Int_t tBins = 8;
//     vector<double> tmin, tmax, yield, yield 
    
//     for(Int_t loc_i = 0; loc_i<tBins;loc_i++) // reads file to end of *file*, not line
//       { 
//         infile >> exam1[num]; // read first column number
//         infile >> exam2[num]; // read second column number
//         infile >> exam3[num]; // read third column number
//       }
         
  
// }
