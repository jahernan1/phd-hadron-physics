void RooFitHist(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> params, string weight_name);
void RooFitHistMC(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> &params, string weight_name);
void setStyle();
void GetDiffXSecFile(string filename, FILE *outputFile, FILE *diffxsecFile, string diresctory, string weight_name, int n_threads = 4);
void GetTotXSecFile(string filename, FILE *outputFile, FILE *xsecFile,string diresctory, string weight_name, int n_threads = 4);
TH1D* GetFluxHist(string filename);
using namespace RooFit;

int UMLFits(string filename, Bool_t binFlag, string directory, string weight="hybrid_combo")
{
    setStyle();
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    // GetYieldFile("_vertexCuts");
    // GetYieldFile();
    string logDir = "/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/data/"+directory+weight+"/";
    string subdir = directory.substr(directory.find_first_of("/"));
    cout << "Storing data files to:\n" << logDir << endl;
    string name = filename.substr(0,filename.find_last_of("_"));
    name = name.substr(0,name.find_last_of("_"));
    string nameTot = filename.substr(0,filename.find("Emin")-1);
    FILE *diff_outf, *tot_outf, *diffxsec_outf, *totxsec_outf;
    //
    if(binFlag)
      {//check if file had been opened to write header
        if (FILE *file = fopen((logDir+"diffout_"+name+".txt").c_str(), "r"))
          {
            fclose(file); cout << "file exists" << endl;
            //Open Files to Fill
            diff_outf = fopen((logDir+"diffout_"+name+".txt").c_str(),"a+");
            diffxsec_outf = fopen((logDir+"diffxsec_"+name+".txt").c_str(),"a+");
          }

          {
            //Open Files to Fill
            diff_outf = fopen((logDir+"diffout_"+name+".txt").c_str(),"a+");
            diffxsec_outf = fopen((logDir+"diffxsec_"+name+".txt").c_str(),"a+");
            
            //Set Up Headers
            cout << "Set up header for output Files..." << endl;
            fprintf(diff_outf, "#tcenter  terr  deltaT  data_yield  yield_err  mc_yield  mc_err  thrown_yield  thrown_err accept accep_err  flux  flux_err \n");
            fprintf(diffxsec_outf, "#tBinCenter  dsigma/dt  tBinWidth Yerr\n");//format as X,Y,EX,EY for TGraphErrors
          }
        //Process diff xsec files
        cout << "Processing: " << filename << endl;
        GetDiffXSecFile(filename, diff_outf, diffxsec_outf, directory, weight);
        fclose(diff_outf); fclose(diffxsec_outf);
      }
    else
      {//total xsec files
        if (FILE *file = fopen((logDir+"totout_"+nameTot+".txt").c_str(), "r"))
          {
            fclose(file); cout << "file exists" << endl;
            //Open Files to Fill
            tot_outf = fopen((logDir+"totout_"+nameTot+".txt").c_str(),"a+");
            totxsec_outf = fopen((logDir+"totxsec_"+nameTot+".txt").c_str(),"a+");
          }

        else
          {
            //Open Files to Fill
            tot_outf = fopen((logDir+"totout_"+nameTot+".txt").c_str(),"a+");
            totxsec_outf = fopen((logDir+"totxsec_"+nameTot+".txt").c_str(),"a+");
            
            //Set Up Headers
            cout << "Set up header for output Files..." << endl;

            fprintf(tot_outf, "#nBinCenter EnErr  deltaE  data_yield  yield_err  mc_yield  mc_err  thrown_yield  thrown_err accept accept_err  flux  flux_err \n");
            fprintf(totxsec_outf, "#enBinCenter  sigma  enBinWidth Yerr\n");//format as X,Y,EX,EY for TGraphErrors
          }
        //Process tot xsec files
        cout << "Processing: " << filename << endl;
        GetTotXSecFile(filename, tot_outf, totxsec_outf, directory, weight);
        fclose(tot_outf); fclose(totxsec_outf);
      }    
    
    return 0;
}

void GetDiffXSecFile(string filename, FILE *outputFile, FILE *diffxsecFile, string directory, string weight, int n_threads = 4)
{
	//Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
    //Initialize variables
    string tree_dir = "/d/grid17/hjesse/AnalysisNote/xsection/prepped_ttrees/"+directory;
    //string logDir = "/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/yield_data/"+weight;
    string fluxDir = "/d/grid17/hjesse/AnalysisNote/fluxFiles/";
    //Branches you want to use get
    std::vector<std::string> branches = {"decayxim_M"};
    auto dfT = ROOT::RDataFrame("flatTree_thrown_kpkpxim", tree_dir+"thrown_"+filename);
    auto thrownBeam = dfT.Histo1D("beam_E");

    //Get trees for Ebins
    TFile* dataFile = TFile::Open((tree_dir+filename).c_str(), "READ");
    TFile* reconFile = TFile::Open((tree_dir+"recon_"+filename).c_str(), "READ");
    TFile* thrownFile = TFile::Open((tree_dir+"thrown_"+filename).c_str(), "READ");
    TFile* fluxFile; TH1D *tag_flux;
    TTree* dataTree = (TTree*)dataFile->Get("flatTree_kpkpxim");
    TTree* reconTree = (TTree*)reconFile->Get("flatTree_kpkpxim");
    TTree* thrownTree = (TTree*)thrownFile->Get("flatTree_thrown_kpkpxim");
    Double_t yield, yield_err, yieldMC, yieldMC_err, yieldT, yieldT_err, yieldF, yieldF_err;
    Double_t diffxsec, diffxsec_err, intxsec, intxsec_err, accept, accept_err, deltaT;
    // Target values
    Double_t Na = 6.022e23; //[atoms/mol]
    Double_t tar_Len = 79.1-50.4; //[cm]
    Double_t tar_Den = 70.08e-3; // 2018 (+/- 0.0035) [g/cm^3]
    Double_t hy_MM = 2.01588; //[g/mol]
    Double_t tar_val = 2 * Na * tar_Len * tar_Den * pow(10,-24) / hy_MM;
    //Double_t tar_val = 1.271;// 2017 (+/- 0.006 b^-1)
    //Double_t tar_val = 1.22;
    printf("The target value is: %f\n", tar_val); 
    
    Double_t hy_den = tar_val; Double_t BR_Lamb = 0.641; Double_t BR_Lamb_Err = 0.005;
    vector<double> xiParams(3);
    
    //Get the flux histo
    tag_flux = (TH1D*)GetFluxHist(filename)->Clone("tagged_flux");

    //Get energy boundries for flux yields
    string emin = filename.substr(filename.find("Emin")+5);
    emin = emin.substr(0, emin.find(".")+3);
    string emax = filename.substr(filename.find("Emax")+5);
    emax = emax.substr(0, emax.find(".")+3);

    //En bins for flux
    //cout << "EnBin: (" << tag_flux->FindBin(stod(emin)) <<","<< tag_flux->FindBin(stod(emax)) << ")" << endl;
    yieldF = tag_flux->IntegralAndError(tag_flux->FindBin(stod(emin)), tag_flux->FindBin(stod(emax))-1, yieldF_err);

    //tbins
    cout << "TMin, TMax: (" << filename.substr(filename.find("Tmin")+5,4) << "," <<filename.substr(filename.find("Tmax")+5,4) <<")"<< endl;
    string tmin = filename.substr(filename.find("Tmin")+5,4);
    string tmax = filename.substr(filename.find("Tmax")+5,4) ;
    deltaT = stod(tmax) - stod(tmin);
    
    //Fit Data and MC for yields
    string histTitle = "#bf{-t: ("+tmin+", "+tmax+")} ;M(#Lambda#pi^{-}) (GeV); Events";
    RooFitHistMC(reconTree, histTitle, directory+"recon_"+filename.substr(0,filename.find_last_of(".")), &yieldMC, &yieldMC_err, xiParams, weight);//mc is a weighted likelihood fit
    RooFitHist(dataTree, histTitle, directory+filename.substr(0,filename.find_last_of(".")), &yield, &yield_err, xiParams, weight);
    
    //Get thrown yields
    yieldT = thrownBeam->IntegralAndError(1,thrownBeam->GetNbinsX(), yieldT_err); 
    cout << "Yields: " << yield << " +/- " << yield_err << endl;
    cout << "Yields MC: " << yieldMC << " +/- " << yieldMC_err << endl;
    cout << "Yields Thrown: " << yieldT << " +/- " << yieldT_err << endl; 

    if(yield_err > yield)
      cout << "[WARNING] Yield_err > Yield";
    //Get acceptance
    accept = yieldMC / yieldT;
    accept_err = accept * sqrt( pow( yieldMC_err / yieldMC, 2  ) + pow ( yieldT_err / yieldT, 2) );

    //GetDiffXSec
    diffxsec = yield / ( hy_den * yieldF * BR_Lamb * accept * deltaT );
    diffxsec_err = diffxsec * sqrt( pow( yield_err / yield, 2) + pow( yieldF_err / yieldF, 2) + pow (accept_err / accept, 2) + pow (BR_Lamb_Err / BR_Lamb, 2) );
    //Write out data to files
    fprintf(outputFile, "%f  %f  %f  %f  %f  %f  %f  %f  %f  %f  %f  %f  %f\n", (stod(tmin)+stod(tmax))/2, deltaT/2, deltaT, yield, yield_err, yieldMC, yieldMC_err, yieldT, yieldT_err, accept, accept_err, yieldF, yieldF_err);

    fprintf(diffxsecFile, "%f  %f  %f  %f\n", (stod(tmin)+stod(tmax))/2, diffxsec*pow(10,9), deltaT/2, diffxsec_err*pow(10,9));
}

//
void GetTotXSecFile(string filename, FILE *outputFile, FILE *xsecFile, string directory, string weight, int n_threads = 4)
{
	//Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
    //Initialize variables
    string tree_dir = "/d/grid17/hjesse/AnalysisNote/xsection/prepped_ttrees/"+directory;
    //string logDir = "/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/yield_data/"+weight;
    string fluxDir = "/d/grid17/hjesse/AnalysisNote/fluxFiles/";
    //Branches you want to use get
    std::vector<std::string> branches = {"decayxim_M"};
    auto dfT = ROOT::RDataFrame("flatTree_thrown_kpkpxim", tree_dir+"thrown_"+filename);
    auto thrownBeam = dfT.Histo1D("beam_E");

    //Get trees for Ebins
    TFile* dataFile = TFile::Open((tree_dir+filename).c_str(), "READ");
    TFile* reconFile = TFile::Open((tree_dir+"recon_"+filename).c_str(), "READ");
    TFile* thrownFile = TFile::Open((tree_dir+"thrown_"+filename).c_str(), "READ");
    TFile* fluxFile; TH1D *tag_flux;
    TTree* dataTree = (TTree*)dataFile->Get("flatTree_kpkpxim");
    TTree* reconTree = (TTree*)reconFile->Get("flatTree_kpkpxim");
    TTree* thrownTree = (TTree*)thrownFile->Get("flatTree_thrown_kpkpxim");
    Double_t yield, yield_err, yieldMC, yieldMC_err, yieldT, yieldT_err, yieldF, yieldF_err;
    Double_t totxsec, totxsec_err, accept, accept_err, deltaT;
    Double_t hy_den = 1.22; Double_t BR_Lamb = 0.641; Double_t BR_Lamb_Err = 0.005;
    vector<double> xiParams(3);
    
    //Get the flux histo
    tag_flux = (TH1D*)GetFluxHist(filename)->Clone("tagged_flux");

    //Get energy boundries for flux yields
    string emin = filename.substr(filename.find("Emin")+5);
    emin = emin.substr(0, emin.find(".")+3);
    string emax = filename.substr(filename.find("Emax")+5);
    emax = emax.substr(0, emax.find(".")+3);

    cout << "Emin,Emax: (" << stof(emin) <<","<< stod(emax) <<")" << endl;
    Double_t deltaE = stod(emax) - stod(emin);
    
    //En bins for flux
    cout << "EnBin: (" << tag_flux->FindBin(stod(emin)) <<","<< tag_flux->FindBin(stod(emax)) << ")" << endl;
    yieldF = tag_flux->IntegralAndError(tag_flux->FindBin(stod(emin)), tag_flux->FindBin(stod(emax))-1, yieldF_err);

    //Fit Data and MC for yields
    string histTitle = "#bf{E_{#gamma}: ("+emin+", "+emax+")} ;M(#Lambda#pi^{-}) (GeV); Events";
    RooFitHistMC(reconTree, histTitle, directory+"recon_"+filename.substr(0,filename.find_last_of(".")), &yieldMC, &yieldMC_err, xiParams, weight);//mc is a weighted likelihood fit
    RooFitHist(dataTree, histTitle, directory+filename.substr(0,filename.find_last_of(".")), &yield, &yield_err, xiParams, weight);

    //Get thrown yields
    yieldT = thrownBeam->IntegralAndError(1,thrownBeam->GetNbinsX(), yieldT_err); 
    cout << "Yields: " << yield << " +/- " << yield_err << endl;
    cout << "Yields MC: " << yieldMC << " +/- " << yieldMC_err << endl;
    cout << "Yields Thrown: " << yieldT << " +/- " << yieldT_err << endl;
    
    //Get acceptance
    accept = yieldMC / yieldT;
    accept_err = accept * sqrt( pow( yieldMC_err / yieldMC, 2  ) + pow ( yieldT_err / yieldT, 2) );

    //GetDiffXSec
    totxsec = yield / ( hy_den * yieldF * BR_Lamb * accept );
    totxsec_err = totxsec * sqrt( pow( yield_err / yield, 2) + pow( yieldF_err / yieldF, 2) + pow( accept_err / accept, 2  ) + pow (BR_Lamb_Err / BR_Lamb, 2) );
    
    //Write out data to files
    fprintf(outputFile, "%f  %f  %f  %f  %f  %f  %f  %f  %f  %f  %f  %f  %f\n", (stod(emax)+stod(emin))/2,deltaE/2, deltaE, yield, yield_err, yieldMC, yieldMC_err, yieldT, yieldT_err, accept, accept_err, yieldF, yieldF_err);
  
    fprintf(xsecFile, "%f  %f  %f  %f\n", (stod(emin)+stod(emax))/2, totxsec*pow(10,9), deltaE/2, totxsec_err*pow(10,9));
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
    else
      {cout << "Cannot find correct flux file..." << cout;}    

    return tag_flux;
}

void RooFitHist(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> params, string hist_weight)
{
  TH1::AddDirectory(kFALSE);
  gStyle->SetTitleAlign(33);
  gStyle->SetTitleX(.95);
  //Import dataset to plot
  RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.26, 1.45);
  //Weighted fit
  RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
  RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
  //RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass), Import(*treeData));
  TH1* dataHist = (TH1*)data->createHistogram("decayxim_M")->Clone(delim.c_str());
  Double_t min_mass = dataHist->GetXaxis()->GetBinLowEdge(dataHist->FindFirstBinAbove(0,1,1, dataHist->FindBin(1.32)));
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
  w->factory("Chebychev::bkgd(decayxim_M,{a0[0.8,0.01,2.],a1[-0.2,-2.,-0.01]})");//,a1[-0.1,-2,-1e-2]
  w->factory(("Johnson::xisignal(decayxim_M,mu[1.322,1.321,1.323],lambda["+to_string(params[0])+","+to_string(params[0])+",0.008], gamma["+to_string(params[2])+"], delta["+to_string(params[1])+"])").c_str());
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
  //fitCan->Print();
  fitCan->Print( ("/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/fits/"+delim+"_"+hist_weight+".pdf").c_str());
  fitCan->Close();
  delete fitCan, data, w;
}

void RooFitHistMC(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> &params, string hist_weight)
{
  TH1::AddDirectory(kFALSE);
  //Set up workspace
  RooWorkspace* w = new RooWorkspace(histTitle.c_str());
  RooRealVar mass("decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.27, 1.42);
  
  RooRealVar weight(hist_weight.c_str(), "weight", -10, 10);
  //With weight 
  RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass, weight), Import(*treeData), WeightVar(weight));
  //RooDataSet* data = new RooDataSet("data", "Dataset of mass", RooArgSet(mass), Import(*treeData));
  // TH1* dataHist = (TH1*)data->createHistogram("decayxim_M")->Clone(delim.c_str());
  // Double_t min_mass = dataHist->GetXaxis()->GetBinLowEdge(dataHist->FindFirstBinAbove(0,1,1, dataHist->FindBin(1.32)));
  mass.setRange("signal", 1.27, 1.37);
  RooPlot* massframe = mass.frame(Title(histTitle.c_str()));
  TCanvas* fitCan = new TCanvas("fitCanMC"," mc", 800, 700);
  //fitCan->SetLogy();
  w->import(RooArgSet(mass));
    
  //Build model and Fit data
  //w->factory("EXPR::bkgd('(decayxim_M)*(((decayxim_M)/m0)**2-1.0)**p*exp(b*(((decayxim_M)/m0)**2-1.0))',decayxim_M, m0[1.2602,1.255,1.275], b[-22,-40.,-5.], p[2])");
  //w->factory("Chebychev::bkgd(decayxim_M,{a0[0.8,0.1,1.5],a1[-0.2,-1,-0.1]})");//,a1[-0.1,-2,-1e-2]
  //w->factory("Gaussian::xisignal(decayxim_M,mean1[1.32,1.31,1.33], sigma[0.005,0.003,0.007])");
  w->factory("Johnson::xisignal(decayxim_M,mu[1.3217,1.32,1.33],lambda[0.0045,0.002,0.007], gamma[0, -1,1], delta[1.2,0.2,5.])");
  w->factory("SUM::model( nxi[1000,1,1e6]*xisignal)");// nbkgd[200,1,1e6]*bkgd,
  w->pdf("model")->fitTo(*data,SumW2Error(false),Hesse(false),PrintLevel(-1), Range("signal"));
  
  //Plot model and data 
  data->plotOn(massframe, Name("datapnts"));
  w->pdf("model")->paramOn(massframe, Format("NE",AutoPrecision(1)), Layout(0.55, 0.95, 0.92), Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("lambda"))));
  w->pdf("model")->plotOn(massframe, LineWidth(3), Name("model"), Range("signal"));
  w->pdf("xisignal")->plotOn(massframe,DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Name("xisignal"), Range("signal"));//
  //w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("signal"));

  *yield = w->var("nxi")->getVal();
  *yield_err = w->var("nxi")->getError();
  params[0] = w->var("lambda")->getVal();
  params[1] = w->var("delta")->getVal();
  params[2] = w->var("gamma")->getVal();
  
  //set up specific frame styling
  massframe->GetYaxis()->SetMaxDigits(2);
  massframe->GetYaxis()->SetNdivisions(505, kFALSE);
  massframe->SetMinimum(0.1);
  massframe->Draw();

  fitCan->SetGrid();
  fitCan->SaveAs( ("/d/grid17/hjesse/AnalysisNote/xsection/ml_fits/fits/"+delim+"_"+hist_weight+".pdf").c_str());
  fitCan->Close();
  delete fitCan, data, w;
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
