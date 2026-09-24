
RooHistPdf* getHistogramPdf(TTree* treeData,RooRealVar& mass, string histTitle, vector<string> delim, double *yield, double *yield_err, string hist_weight);
void RooFitHist(TTree* treeData, RooHistPdf* histPdf, string histTitle, vector<string> delim, double *yield, double *yield_err, string hist_weight, int chebyOrder);
bool AttemptFitMC(RooWorkspace* w, RooDataSet* data, vector<double> &params);
bool AttemptFit(RooWorkspace* w, RooDataSet* data, vector<double> &params, double lowerBound, double upperBound);
//bool AttemptFit(RooWorkspace* w, RooDataSet* data, vector<double> &params);
void setStyle();
void GetDiffXSecFile(vector<TTree*> trees, TH1D* flux, vector<string> delim, ofstream& outputFile, ofstream& xsecFile, string weight="hybrid_combo", int chebyOrder=2, int n_threads = 4);
void GetTotXSecFile(vector<TTree*> trees, TH1D* flux, std::vector<string> delim, ofstream& outputFile, ofstream& xsecFile, string weight="hybrid_combo", int chebyOrder=2, int n_threads = 4);
TH1D* GetFluxHist(string filename);
void getXSecFiles(string filename, string accType="hybrid_combo", string variation="", int chebyOrder=2, string nameDelim="binned_");
using namespace RooFit;

// flatTree_kpkpxim__B4_M23_2018-01_ana03_chisqndf_variations.root
int MakeXSecFitMC(){
    setStyle();
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    
    /* 
       This function produces the binned data and mc into a single root file 
       and the binned thrown into a seperate root file
    */
    
    getXSecFiles(std::string("flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap"), "hybrid_combo", "mcPdf");
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap"),"hybrid_combo", "mcPdf");
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap"), "hybrid_combo", "mcPdf");

    getXSecFiles(std::string("flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap"), "hybrid_combo", "mcPdf_cheby1", 1);
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap"),"hybrid_combo", "mcPdf_cheby1", 1);
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap"), "hybrid_combo", "mcPdf_cheby1", 1);

    
    return 0;
}

void getXSecFiles(string filename, string accType="hybrid_combo", string variation="", int chebyOrder=2, string nameDelim="binned_")
{
    //Set up variables
    string logDir;
    if(variation.empty())
        logDir = "/d/grid17/hjesse/AnalysisNote/xsection/data/"+accType+"/";
    else
      logDir = "/d/grid17/hjesse/AnalysisNote/xsection/data/"+variation+"/";  
    //make sure directory exists
    gSystem->mkdir(logDir.c_str());
    string rootFileDir = "/d/grid17/hjesse/AnalysisNote/flatTrees/";
    string fluxDir = "/d/grid17/hjesse/AnalysisNote/fluxFiles/";
    cout << "Storing data files to:\n" << logDir << endl;
    string name = filename.substr(0,filename.find("ana")+5);//store just treename
    string full_name = filename.substr(0,filename.find("_nominal"));//store just treename
    string end = filename.substr(filename.find("ana")+6);//store just end deliminator
    std::vector<string> delim(3); delim[1] = name;
    if(variation.empty())
        delim[0] = accType;
    else
        delim[0] = variation;
    
    // Get the tfiles for the cross section
    cout << "Processing cross section output for:\n" << filename << endl;
    TFile *dataFile = TFile::Open( (rootFileDir+nameDelim+filename+".root").c_str(), "READ");
    TFile *mcFile = TFile::Open( (rootFileDir+nameDelim+name+"_gen_amp_V2_ac_YstarRest_"+end+".root").c_str(), "READ");
    TFile *thrownFile = TFile::Open( (rootFileDir+nameDelim+"thrown_"+name+"_gen_amp_V2_ac_YstarRest.root").c_str(), "READ");
    TH1D* tag_flux = (TH1D*)GetFluxHist(filename)->Clone("tagged_flux");
    //tag_flux->Draw();

    // Set up files for total xsection
    std::ofstream tot_outf( (logDir+"totout_"+name+".txt").c_str() );
    tot_outf << "enBinCenter\t" << "EnErr\t" 
             << "data_yield\t" << "yield_err\t"
             << "qval_yield\t" << "qval_yield_err\t"
             << "mc_yield\t" << "mc_err\t"
             << "thrown_yield\t" << "thrown_err\t"
             << "accept\t" << "accept_err\t"
             << "flux\t" << "flux_err\t" << endl;
    std::ofstream totxsec_outf( (logDir+"totxsec_"+name+".txt").c_str() );
    totxsec_outf << "enBinCenter\t" << "sigma\t" << "enBinWidth\t"
                 << "Yerr\t" << endl;
    
    // Iterate over all keys in the root file    
    TIter nextTree(dataFile->GetListOfKeys());
    TKey* treeKey;
    std::ofstream diff_outf; std::ofstream diffxsec_outf;
                
    while ((treeKey = (TKey*)nextTree())) {//loop over all bins for variation
        // Check if the key is a TTree
        if (std::string(treeKey->GetClassName()) == "TTree"){
            std::string treeName = treeKey->GetName();
            delim[2] = treeName;
            // Retrieve the TTree object
            TTree* tree = (TTree*)dataFile->Get(treeName.c_str());
            TTree* treeMC = (TTree*)mcFile->Get(treeName.c_str());
            TTree* treeThrown = (TTree*)thrownFile->Get(treeName.c_str());
                     
            if(treeName.find("tmin")==std::string::npos){//full energy bin
                std::cout << " Processing TTree: " << treeName << std::endl;

                // Get total xsec
                GetTotXSecFile({tree,treeMC,treeThrown}, tag_flux,
                               delim, tot_outf, totxsec_outf, accType, chebyOrder);

                // Set diffxsec output files with headers
                if(diffxsec_outf.is_open() ){
                    diffxsec_outf.close(); diff_outf.close();}
                            
                diff_outf.open( (logDir+"diffout_"+name+"_"+treeName+".txt").c_str());
                diffxsec_outf.open( (logDir+"diffxsec_"+name+"_"+treeName+".txt").c_str());
                diff_outf << "tcenter\t" << "terr\t" 
                          << "data_yield\t" << "yield_err\t"
                          << "qval_yield\t" << "qval_yield_err\t"
                          << "mc_yield\t" << "mc_err\t"
                          << "thrown_yield\t" << "thrown_err\t"
                          << "accept\t" << "accep_err\t"
                          << "flux\t" << "flux_err"
                          << std::endl;
                            
                diffxsec_outf << "tBinCenter\t" << "dsigmadt\t"
                              << "tBinWidth\t" << "Yerr" << std::endl;
            }
            else if(treeName.find("tmin")!=std::string::npos){//is tbin
                std::cout << " Processing TTree: " << treeName << std::endl;
                // Omit tbins from name to open file
                treeName = treeName.substr(0,treeName.find("_tmin"));
                // Get Diff Xsec
                GetDiffXSecFile({tree,treeMC,treeThrown}, tag_flux,
                                delim, diff_outf, diffxsec_outf,
                                accType, chebyOrder);
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
    
    return 0;
}

void GetDiffXSecFile(vector<TTree*> trees, TH1D* flux, vector<string> delim, ofstream& outputFile, ofstream& xsecFile, string weight="hybrid_combo", int chebyOrder=2, int n_threads = 4)
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
    vector<double> xiParams = {1.3217,0.004};
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
    
    // Get the Qvalue yield for data (tree[0])
    auto df = ROOT::RDataFrame(*trees[0])
        .Define("qvalue_acc", (weight+"*qvalue_decayxim_M").c_str());
    yieldQVal = df.Sum("qvalue_acc").GetValue();
    
    if(trees[0]->GetEntries() > 0 && trees[1]->GetEntries() > 0){
        RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.45);
        RooHistPdf* mcPdf = getHistogramPdf(trees[1], mass, histTitle, delim, &yieldMC, &yieldMC_err,weight);
        RooFitHist(trees[0], mcPdf, histTitle, delim, &yield, &yield_err, weight, chebyOrder);
        
        //Get thrown yields
        yieldT = trees[2]->GetEntries(); 
        yieldT_err = sqrt(yieldT);
        cout << "Yields: " << yield << " +/- " << yield_err << "(QVal: " << yieldQVal << ")"  << endl;
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

void GetTotXSecFile(vector<TTree*> trees, TH1D* flux, vector<string> delim, ofstream& outputFile, ofstream& xsecFile, string weight="hybrid_combo", int chebyOrder=2
                    ,int n_threads = 4)
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
    
    vector<double> xiParams = {1.3217,0.004};;
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
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.45);
    RooHistPdf* mcPdf = getHistogramPdf(trees[1], mass, histTitle, delim, &yieldMC, &yieldMC_err,weight);//mc is a weighted likelihood fit
    RooFitHist(trees[0], mcPdf, histTitle, delim, &yield, &yield_err, weight,chebyOrder);
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
TH1D* GetFluxHist(string filename)
{
    string fluxDir = "/d/grid17/hjesse/AnalysisNote/fluxFiles/";
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

RooHistPdf* getHistogramPdf(TTree* treeData, RooRealVar& mass, string histTitle, vector<string> delim, double *yield, double *yield_err, string hist_weight)
{
    TH1::AddDirectory(kFALSE);
    string saveDir = "/d/grid17/hjesse/AnalysisNote/xsection/fits/"+delim[0]+"/";
    gSystem->mkdir(saveDir.c_str());
    // Set up  data
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));

    TH1* hSim = (TH1*)data->createHistogram("decayxim_M",60)->Clone();
        
    RooDataHist* simDataHist = new RooDataHist("simDataHist", "Simulated Data Histogram", RooArgSet(mass), hSim);
    RooHistPdf* histPdf = new RooHistPdf("histPdf", "Histogram PDF", RooArgSet(mass),RooArgSet(mass), *simDataHist, 0);

    *yield = data->sumEntries(); 
    *yield_err = sqrt(*yield);

    // Plot model and data
    RooPlot* massframe = mass.frame(Title(histTitle.c_str()));
    TCanvas* fitCan = new TCanvas("fitCanMC", "mc", 800, 700);
    //fitCan->SetLogy();

    data->plotOn(massframe, Name("datapnts"), Binning(60, 1.27, 1.45));
    histPdf->plotOn(massframe);
    
    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505, kFALSE);
    massframe->SetMinimum(0.1);
    massframe->Draw();

    fitCan->SetGrid();
    if (!delim.empty())
        fitCan->Print( (saveDir+"recon_"+delim[1]+"_"+delim[2]+".pdf").c_str());
    fitCan->Close();

    return histPdf;
}
    
void RooFitHist(TTree* treeData, RooHistPdf* histPdf, string histTitle, vector<string> delim, double *yield, double *yield_err, string hist_weight, int chebyOrder){

    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    string saveDir = "/d/grid17/hjesse/AnalysisNote/xsection/fits/"+delim[0]+"/";
    gSystem->mkdir(saveDir.c_str());
    //Import dataset to plot
    double max_mass=1.45; double small = 1e-4;
    RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.275, 1.45);
    //Weighted fit
    RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
    RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
    TH1* dataHist = (TH1*)data->createHistogram("decayxim_M")->Clone(delim[2].c_str());
    
    //Build model and Fit data
    RooArgSet chebySet;
    RooRealVar a0("a0", "a0", 0.9, 0.01, 2.);
    RooRealVar a1("a1", "a1", -0.1, -2., -0.01);
    //choose was order
    chebySet.add(a0);//Always at least first order
    if(chebyOrder==2)  chebySet.add(a1);
    
    RooChebychev bkg("bkg", "Background", mass, chebySet);
    
    // Associated number of signal/background events
    RooRealVar nsig("nsig", "number of signal events", 500, 1., 10000);
    RooRealVar nbkg("nbkg", "number of background events", 50, 0, 10000);
    
    // Build total model
    RooAddPdf model("model", "g1+g2", RooArgList(bkg, *histPdf), RooArgList(nbkg, nsig));
    
    RooFitResult* fitResult = model.fitTo(*data,
                                          Extended(true), //EvalErrorWall(true),
                                          SumW2Error(false),
                                          RecoverFromUndefinedRegions(10),
                                          //RooFit::AsymptoticError(true),
                                          Hesse(false),
                                          PrintLevel(-1), Save(true));
    
    fitResult->Print();
    
    //cout << "Fit Status Code: " << fitResult->status() << endl;
    *yield = nsig.getVal() ;
    *yield_err = sqrt(*yield);
    
    //Plot model and data 
    RooPlot* massframe = mass.frame( Title( histTitle.c_str() ) );
    TCanvas* fitCan = new TCanvas("fitCan"," c", 800, 700);
    //fitCan->SetLogy();
        
    data->plotOn(massframe, Name("datapnts"), Binning(50, 1.27, 1.45));
    //histPdf->plotOn(massframe);
    model.paramOn(massframe, Format("NE", AutoPrecision(1)), Layout(0.55, 0.95, 0.9));
    model.plotOn(massframe, LineWidth(4));
    model.plotOn(massframe, Components("histPdf"), DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Name("xisignal"));//    
    model.plotOn(massframe, Components("bkg"), LineStyle(kDotted));

    //set up specific frame styling
    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->GetYaxis()->SetNdivisions(505);
    //massframe->SetMinimum(0.1);
  
    massframe->Draw();
  
    fitCan->SetGrid();
    //fitCan->Print("fit.pdf");
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
