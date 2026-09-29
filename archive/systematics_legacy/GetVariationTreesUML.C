#include "gxana/common/Paths.h"
#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
using namespace RooFit;

//initiate functions
void GetPrepedFlatTree(string root_file_name, string cut_set, string delim, string mc_version="gen_amp_V2_ac_YstarRest", int n_threads = 4 ); 
void RooFitHist(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> &params, string hist_weight, int max_retries = 5);
void RooFitHistMC(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> &params, string hist_weight, int max_retries = 5);
bool AttemptFit(RooWorkspace* w, RooDataSet* data, vector<double> &params);
bool AttemptFitMC(RooWorkspace* w, RooDataSet* data, vector<double> &params);
std::string replaceValueInDelimitedString(const std::string& input, const std::string& oldValue, const std::string& newValue);
void appendToTextFile(vector<string> dataSet_cut, vector<double> values);

//main
int GetVariationTreesUML()
{
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    // blank out files because will open in append mode later 
    std::ofstream outFile("output_yields.txt");
    outFile.close();
    
    // Define some data to pick from
    std::string nominal_cutSet="chisqndf<8&total_mm2_abs<0.02&&xim_pathlensig>2&&lambda_pathlensig>0&&kphigh_prap>2&&t_dist<2.4";
    std::vector<std::string> cuts = {"chisqndf<6","chisqndf<7",
                                     "chisqndf<9", "chisqndf<10",
                                     "total_mm2_abs<0.01","total_mm2_abs<0.015",
                                     "total_mm2_abs<0.025","total_mm2_abs<0.03",
                                     "xim_pathlensig>1","xim_pathlensig>1.5",
                                     "xim_pathlensig>2.5","xim_pathlensig>3",
                                     "lambda_pathlensig>0.5", "lambda_pathlensig>1",
                                     "kphigh_prap>1.6","kphigh_prap>1.8",
                                     "kphigh_prap>2.1","kphigh_prap>2.2"
    };
    std::vector<std::string> cutSets;
    // Define a vector of maps
    std::vector<std::map<std::string, std::string>> variations;

    // Populate the vector with variations
    for (int i = 0; i < cuts.size(); ++i) {
        // Generate new cut set string to use
        if(cuts[i].find("chisqndf")!=std::string::npos)
            cutSets.push_back( replaceValueInDelimitedString(nominal_cutSet,"chisqndf<8",cuts[i]) );
        else if(cuts[i].find("total_mm2")!=std::string::npos)
            cutSets.push_back( replaceValueInDelimitedString(nominal_cutSet,"total_mm2_abs<0.02",cuts[i]) );
        else if(cuts[i].find("xim_pathlensig")!=std::string::npos)
            cutSets.push_back( replaceValueInDelimitedString(nominal_cutSet,"xim_pathlensig>2",cuts[i]) );
        else if(cuts[i].find("lambda_pathlensig")!=std::string::npos)
            cutSets.push_back( replaceValueInDelimitedString(nominal_cutSet,"lambda_pathlensig>0",cuts[i]) );
        else if(cuts[i].find("kphigh_prap")!=std::string::npos)
            cutSets.push_back( replaceValueInDelimitedString(nominal_cutSet,"kphigh_prap>2",cuts[i]) );
        // else if(cuts[i].find("kplow_prap")!=std::string::npos)
        //    cutSets.push_back( replaceValueInDelimitedString(nominal_cutSet,"kplow_prap>0",cuts[i]) );
        else{
            cout << "[Error] Could not find cut in nominal values.";
            return -1;
        }
        // Define and fill maps to put into data structure
        std::map<std::string, std::string> variation;
        variation["CutSet"] = cutSets[i];
        if(cuts[i].find(">")!=std::string::npos)
            variation["Variation"] = cuts[i].substr(0,cuts[i].find(">")) +"_"+ cuts[i].substr(cuts[i].find(">")+1) ;
        else if(cuts[i].find("<")!=std::string::npos)
            variation["Variation"] = cuts[i].substr(0,cuts[i].find("<")) +"_"+ cuts[i].substr(cuts[i].find("<")+1) ;
        else{
            cout << "[Error] Variational cut not well defined.";
            return -1;
        }
        // Add the variation map to the vector
        variations.push_back(variation);
    }
    
    // Iterate over the vector and prep root files with variations
    for (const auto& variation : variations) {
        std::cout << "Variation Info: " << std::endl;
        vector<string> vars;
        
        for (const auto& var : variation) {
            std::cout << " ++ " << var.first << ": \"" << var.second << "\"" << std::endl;    
            vars.push_back(var.second);
        }
        // this saves the newest frees for data and mc to one file with all variations
        GetPrepedFlatTree("flatTree_kpkpxim__M23_2017-01_ana56", vars[0], vars[1]);
        GetPrepedFlatTree("flatTree_kpkpxim__B4_M23_2018-01_ana03", vars[0], vars[1]);
        GetPrepedFlatTree("flatTree_kpkpxim__B4_M23_2018-08_ana02", vars[0], vars[1]);
    }

    return 0;
}

void GetPrepedFlatTree(string root_file_name, string cut_set, string delim, string mc_version="gen_amp_V2_ac_YstarRest", int n_threads = 8 ) 
{
    /* 
       Initalize variables 
    */
    string treeDir = gxana::EnvPath("GXANA_DATA", "Trees/flatTree/rawTrees/");
    string saveDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/variation_trees/");
    string variation = delim.substr(0,delim.find_last_of("_"));    
    /* 
       Parallelize with n threads 
    */
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	/* 
       Export select braches to speed things up 
    */
    std::vector<std::string> branches =
        {"beam_E","chisqndf","total_mm2","xim_pathlensig","lambda_pathlensig",
         "kphigh_p4","kplow_p4", "beam_vertexZ",
         "hybrid_combo", "kphigh_prap","kplow_prap","total_mm2_abs",
         "beam_E_Truth","beam_p4_truth","decayxim_M","xim_costheta_hf",
         "t_dist","t_dist_truth",
         "chisqndf","confidencelvl",
         "best_combo","best_combo_rf", "acc_weight",
         "xim_lifetime_restframe", "lambda_lifetime_restframe",
         "decayxim_p4","ystar_p4"};
	/* 
       make data frame
	   format : tree name, file name, branches to open
    */
    auto df = ROOT::RDataFrame("flatTree_kpkpxim", (treeDir+root_file_name+".root").c_str())
        .Define("hybrid_combo","best_combo_rf*acc_weight")
        .Define("t_dist_truth","-(beam_p4_truth - kphigh_p4 ).M2()")
        .Define("kphigh_prap","kphigh_p4.Rapidity()")
        .Define("kplow_prap","kplow_p4.Rapidity()")
        .Define("total_mm2_abs","fabs(total_mm2)")
        .Filter("beam_E > 6.4 && beam_E < 11.4")
        .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")
        .Filter(cut_set.c_str(),"cutset");

    auto dfmc = ROOT::RDataFrame("flatTree_kpkpxim", (treeDir+root_file_name+"_"+mc_version+".root").c_str())
        .Define("hybrid_combo","best_combo_rf*acc_weight")
        .Define("t_dist_truth","-(beam_p4_truth - kphigh_p4 ).M2()")
        .Define("kphigh_prap","kphigh_p4.Rapidity()")
        .Define("kplow_prap","kplow_p4.Rapidity()")
        .Define("total_mm2_abs","fabs(total_mm2)")
        .Filter("beam_E_Truth > 6.4 && beam_E_Truth < 11.4")
        .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")
        .Filter(cut_set.c_str(),"cutset");

    /*
      Use Snapshot to write the TTree
    */    
    ROOT::RDF::RSnapshotOptions opts;
    opts.fMode = "UPDATE";
	opts.fOverwriteIfExists=true;
    cout << "Saving flat trees with cuts applied..." << endl;
    df.Snapshot(("vary_"+delim).c_str(), (saveDir+root_file_name+"_"+variation+"_"+mc_version+"_variations.root").c_str(),branches,opts);
    dfmc.Snapshot(("vary_"+delim+"_mc").c_str(), (saveDir+root_file_name+"_"+variation+"_"+mc_version+"_variations.root").c_str(),branches,opts);
    //cout << "Saved TTree: flatTree_kpkpxim_" << delim << endl;

    // Retrieve the new TTree
    TFile *file = TFile::Open((saveDir+root_file_name+"_"+variation+"_"+mc_version+"_variations.root").c_str(), "READ");  // Use "UPDATE" mode to modify existing file
    TTree* newTree = (TTree*)file->Get(("vary_"+delim).c_str());
    TTree* newTreeMC = (TTree*)file->Get(("vary_"+delim+"_mc").c_str());
    
    // Get Value of events after cut
    double yield, yield_err,yieldMC, yieldMC_err;
    double nominal_yield, nominal_yield_err, nominal_yieldMC, nominal_yieldMC_err;
    
    // get yield of nominal tree
    TFile *nominal_file = TFile::Open((gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_kphighrap.root").c_str(), "READ");
    TTree* oldTree = (TTree*)nominal_file->Get("flatTree_kpkpxim");
    TFile *nominal_file_mc = TFile::Open((gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_"+mc_version+"_nominal_kphighrap.root").c_str(), "READ");
    TTree* oldTreeMC = (TTree*)nominal_file_mc->Get("flatTree_kpkpxim");
    vector<double> xiParamsOld(3);
    vector<double> xiParamsNew(3);

    // Perfrom fits, make sure MC fit is first; to use reosultion params in data
    RooFitHistMC(oldTreeMC, " ","", &nominal_yieldMC, &nominal_yieldMC_err, xiParamsOld, "hybrid_combo");
    RooFitHist(oldTree, " ","", &nominal_yield, &nominal_yield_err, xiParamsOld, "hybrid_combo");
    //
    RooFitHistMC(newTreeMC, " ","recon_"+root_file_name+"_"+delim, &yieldMC, &yieldMC_err, xiParamsNew, "hybrid_combo");
    RooFitHist(newTree, " ","data_"+root_file_name+"_"+delim, &yield, &yield_err, xiParamsNew, "hybrid_combo");
    
    double pct_diff = fabs(nominal_yield - yield) / nominal_yield * 100;
    double pct_diff_mc = fabs(nominal_yieldMC - yieldMC) / nominal_yieldMC * 100;
        
    cout << "Root File Processed: " << root_file_name << "\n"
         << "Nominal Yield:  " << nominal_yield << "\t" << nominal_yieldMC << "\n"
         << "Variation Yield:  " << yield << "\t" << yieldMC << "\n"
         << "Pct. Diff.(<10%):  " << pct_diff << "%\t" << pct_diff_mc << "%\n";

    // Write infromation on yields in txt file 
    appendToTextFile({root_file_name, delim}, {nominal_yield,yield,pct_diff,nominal_yieldMC,yieldMC,pct_diff_mc});
    cout << endl;
    // Cleanup
    file->Close();
    nominal_file->Close();
    nominal_file_mc->Close();
    
    /* Display Table of tree values */
    // df1.Display({"evnt_num", "combo_num", "chisqndf", "kp_highp_P3", "kp_lowp_P3", "decayxim_M"}, 50)->Print();
    
    /* Print cut analysis */
}

std::string replaceValueInDelimitedString(const std::string& input, const std::string& oldValue, const std::string& newValue) {
    // Split the input string by "&&"
    std::vector<std::string> parts;
    std::stringstream ss(input);
    std::string item;
    while (std::getline(ss, item, '&')) {
        if (ss.peek() == '&') ss.ignore(); // Ignore the second '&' in each delimiter
        parts.push_back(item);
    }

    // Replace the target value
    for (auto& part : parts) {
        if (part == oldValue) {
            part = newValue;
            break;  // Replace only the first occurrence
        }
    }

    // Rejoin the parts with "&&"
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        result += parts[i];
        if (i < parts.size() - 1) {
            result += "&&";
        }
    }

    return result;
}

bool AttemptFitMC(RooWorkspace* w, RooDataSet* data, vector<double> &params)
{
    RooFitResult* fitResult = w->pdf("model")->fitTo(*data, SumW2Error(false), Hesse(true), PrintLevel(-1), Range("signal"), Save(true));

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

void RooFitHistMC(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> &params, string hist_weight, int max_retries = 5)
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
        fitCan->SaveAs((saveDir + delim + ".pdf").c_str());
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
                               PrintLevel(-1), Save(true));

    fitResult->Print();
    if (fitResult == nullptr || fitResult->status() != 0) {
        cout << "Fit failed with status: " << (fitResult ? fitResult->status() : -1) << endl;
        delete fitResult;
        return false;
    }

    // Update parameters if fit is successful
    params[0] = w->var("lambda")->getVal();
    params[1] = w->var("gamma")->getVal();
    params[2] = w->var("delta")->getVal();
    delete fitResult;
    return true;
}

    
void RooFitHist(TTree* treeData, string histTitle, string delim, double *yield, double *yield_err, vector<double> &params, string hist_weight, int max_retries = 5){

    TH1::AddDirectory(kFALSE);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
    string saveDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/systematics/fits/");
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
        fitCan->Print( (saveDir+delim+".pdf").c_str());
    fitCan->Close();
    //w->writeToFile("model.root");
}

void appendToTextFile(vector<string> dataSet_cut, vector<double> values) {
    // Define the filename
    const char* filename = "output_yields.txt";

    // Open the file in append mode
    std::ofstream outFile(filename, std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    // Append data to the file
    outFile << dataSet_cut[0] << "\t"
            << dataSet_cut[1] ;
    for(int i=0;i<values.size();++i)
       outFile << "\t" << values[i] ;
   outFile << std::endl;
    
    // Close the file
    outFile.close();

    std::cout << "Yield data appended to " << filename << std::endl;
}
