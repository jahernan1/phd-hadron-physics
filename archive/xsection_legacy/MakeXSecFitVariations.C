#include "FitFunctions.h"
using namespace RooFit;

void getXSecFiles(std::string filename, std::string fitType, std::unordered_map<std::string,std::vector<double>> &params, std::string accType="hybrid_combo", std::string variation="", int chebyOrder=2, std::string nameDelim="binned_" )
{
    //Set up variables
    std::string logDir;
    if(variation.empty())
        logDir = "/d/grid17/hjesse/AnalysisNote/xsection/data/"+accType+"/";
    else
      logDir = "/d/grid17/hjesse/AnalysisNote/xsection/data/"+variation+"/";  
    //make sure directory exists
    gSystem->mkdir(logDir.c_str());
    std::string rootFileDir = "/d/grid17/hjesse/AnalysisNote/flatTrees/";
    std::string fluxDir = "/d/grid17/hjesse/AnalysisNote/fluxFiles/";
    std::cout << "Storing data files to:\n" << logDir << std::endl;
    std::string name = filename.substr(0,filename.find("ana")+5);//store just treename
    std::string full_name = filename.substr(0,filename.find("_nominal"));//store just treename
    std::string end = filename.substr(filename.find("ana")+6);//store just end deliminator
    std::vector<std::string> delim(3); delim[1] = name;
    if(variation.empty())
        delim[0] = accType;
    else
        delim[0] = variation;
    
    // Get the tfiles for the cross section
   std::cout << "Processing cross section output for:\n" << filename << std::endl;
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
             << "flux\t" << "flux_err\t" << std::endl;
    std::ofstream totxsec_outf( (logDir+"totxsec_"+name+".txt").c_str() );
    totxsec_outf << "enBinCenter\t" << "sigma\t" << "enBinWidth\t"
                 << "Yerr\t" << std::endl;
    
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
                               delim, fitType, params,
                               tot_outf, totxsec_outf, accType, chebyOrder);

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
                                delim, fitType, params,
                                diff_outf, diffxsec_outf, accType, chebyOrder);
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

// flatTree_kpkpxim__B4_M23_2018-01_ana03_chisqndf_variations.root
int main(){
    setStyle();
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    
    /* 
       This function produces the binned data and mc into a single root file 
       and the binned thrown into a seperate root file
    */
    //Johnson Fit + Chevy2
    std::unordered_map<std::string,std::vector<double>> xiJohnsonParams;
    /* 
       order matters check fit string, unorder_map will read down up
       first element last imput
    */
    xiJohnsonParams["delta"] = {1.,0.2,1.5};
    xiJohnsonParams["gamma"] = {0.,-0.5,0.5};
    xiJohnsonParams["lambda"] = {0.004,0.003,0.01};
    xiJohnsonParams["mu"] = {1.3217,1.31,1.33};
    //
    getXSecFiles(std::string("flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap"),"Johnson",xiJohnsonParams, "hybrid_combo","johnson");
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap"),"Johnson",xiJohnsonParams, "hybrid_combo","johnson");
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap"), "Johnson",xiJohnsonParams, "hybrid_combo","johnson");
    //Cheby1
     getXSecFiles(std::string("flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap"),"Johnson",xiJohnsonParams, "hybrid_combo","johnson_cheby1", 1);
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap"),"Johnson",xiJohnsonParams, "hybrid_combo","johnson_cheby1", 1);
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap"), "Johnson",xiJohnsonParams, "hybrid_combo","johnson_cheby1", 1);

    // // Gaus Fits 
    // std::unordered_map<std::string,std::vector<double>> xiGausParams;
    // xiGausParams["sigma"] = {0.005,0.003,0.01};    
    // xiGausParams["mean"] = {1.3217,1.32,1.33};
    // // Cheby2
    // getXSecFiles(std::string("flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap"),"Gaussian",xiGausParams, "hybrid_combo","gaus");
    // getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap"),"Gaussian",xiGausParams, "hybrid_combo","gaus");
    // getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap"), "Gaussian",xiGausParams, "hybrid_combo","gaus");
    // // Cheby1
    // getXSecFiles(std::string("flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap"),"Gaussian",xiGausParams, "hybrid_combo","gaus_cheby1", 1);
    // getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap"),"Gaussian",xiGausParams, "hybrid_combo","gaus_cheby1", 1);
    // getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap"), "Gaussian",xiGausParams, "hybrid_combo","gaus_cheby1", 1);

    //Voitian Fit + Chevy2
    std::unordered_map<std::string,std::vector<double>> xiVoigtParams;
    xiVoigtParams["sigma"] = {0.002,0.001,0.018};
    xiVoigtParams["width"] = {0.004,0.001,0.008};
    xiVoigtParams["mean"] = {1.3217,1.32,1.33};
    //
    getXSecFiles(std::string("flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap"),"Voigtian",xiVoigtParams, "hybrid_combo","voigt");
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap"),"Voigtian",xiVoigtParams, "hybrid_combo","voigt");
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap"), "Voigtian",xiVoigtParams, "hybrid_combo","voigt");
    // Cheby1
    getXSecFiles(std::string("flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap"),"Voigtian",xiVoigtParams, "hybrid_combo","voigt_cheby1", 1);
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap"),"Voigtian",xiVoigtParams, "hybrid_combo","voigt_cheby1", 1);
    getXSecFiles(std::string("flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap"), "Voigtian",xiVoigtParams, "hybrid_combo","voigt_cheby1", 1);

    return 0;
}
