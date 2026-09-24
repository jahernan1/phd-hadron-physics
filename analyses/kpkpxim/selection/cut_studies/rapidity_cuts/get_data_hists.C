#include "gxana/common/Paths.h"
/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string delim, TFile *save_file, Int_t n_threads=16);
void make_root_tree(string delim="_vertexCuts")  ;
void combineAllHistograms(TFile *file);
    
//main function
int get_data_hists()
{
    //make the root trees with different cuts
    make_root_tree("_ximVertexCut");
    make_root_tree("_kphighrap");
 
    return 0;
}

void make_root_tree(string delim="_vertexCuts")  
{
    //set up root file with directories
    TFile *f =  TFile::Open( ("data"+delim+".root").c_str(), "RECREATE");
    f->cd();
    gDirectory->mkdir("Spring_2017");
    f->cd();
    gDirectory->mkdir("Spring_2018");  
    f->cd();
    gDirectory->mkdir("Fall_2018");

    //initiate variables
    string root_file_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
    string root_file_dir_qval = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/");

    //perform actions
    //spring 2017
    f->cd();
    f->cd("Spring_2017");
    save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ana56_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal"+delim+"_1111111.root", "_qval", f);
    save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest_nominal"+delim+".root", "_mc", f);
    save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest.root", "_thrown", f);
    //spring 2018
    f->cd();
    f->cd("Spring_2018");
    save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111.root", "_qval", f);
    save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest_nominal"+delim+".root", "_mc", f);
    save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest.root", "_thrown", f);
    //fall 2018
    f->cd();
    f->cd("Fall_2018");
    save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_1111111.root", "_qval", f);
    save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest_nominal"+delim+".root", "_mc", f);
    save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest.root", "_thrown", f);

    // Call the function to combine all histograms
    combineAllHistograms(f);
    
    // Close the files  
    f->Close();
}

void save_from_flattrees(string root_file_path, string delim, TFile *save_file, Int_t n_threads=16)
{
    //Declare Variables
    string weight=" ";
    //Multithreating
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);

    // make data frame and braches for histograms from 4 vectors
    // format : tree name, file name, branches to open
    if(delim=="_qval")
        {
            auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"kphigh_rapidity","kplow_rapidity","kp_lowp_P3","kp_highp_P3","t_dist","t_dist_truth","hybrid_combo","qvalue_decayxim_M"})
                .Define("kp_momsep","fabs(kp_highp_P3 - kp_lowp_P3)")
                .Define("qvalue_hybrid","qvalue_decayxim_M*hybrid_combo");
        
            //df1.Report()->Print();
        
            //raw kinfit kaons
            auto hist_kphighrap = df.Histo1D({"KPlusHighRapidity"," ; y(K^{+}_{#lower[-0.3]{F}}); Events",80,0,5} , "kphigh_rapidity","hybrid_combo");
            auto hist_kplowrap = df.Histo1D({"KPlusLowRapidity"," ; y(K^{+}_{#lower[-0.3]{S}}); Events",200u,-1.5,5} , "kplow_rapidity","hybrid_combo");
            auto hist_kphighrap_qval = df.Histo1D({"KPlusHighRapidity_qval"," ; y(K^{+}_{#lower[-0.3]{F}}); Events",80,0,5} , "kphigh_rapidity","qvalue_hybrid");
            auto hist_kplowrap_qval = df.Histo1D({"KPlusLowRapidity_qval"," ; y(K^{+}_{#lower[-0.3]{S}}); Events",200u,-1.5,5} , "kplow_rapidity","qvalue_hybrid");
            auto hist_kphigh_P_qval = df.Histo1D({"KPlusHighP_qval"," ; #left|#vec{#bf{p}}(K^{+}_{#lower[-0.3]{F}})#right| (GeV); Events",80u,0,10}, "kp_highp_P3","qvalue_hybrid");
            auto hist_kplow_P_qval = df.Histo1D({"KPlusLowP_qval"," ; #left|#vec{#bf{p}}(K^{+}_{#lower[-0.3]{S}})#right| (GeV); Events",80,0,4}, "kp_lowp_P3","qvalue_hybrid");
            auto hist_kpmomsep_qval = df.Histo1D({"KPlusMomSep_qval"," ; y(K^{+}_{#lower[-0.3]{S}}); Events",80,0,10} , "kp_momsep","qvalue_hybrid");
            auto hist_tdist_qval = df.Histo1D({"TDist_qval"," ; -t ({GeV}^{2} ); Events",80,0,5}, "t_dist","qvalue_hybrid");
                        
            hist_kphighrap->Write("KPlusHighRapidity",TObject::kOverwrite);
            hist_kphighrap_qval->Write(("KPlusHighRapidity"+delim).c_str(),TObject::kOverwrite);  
            hist_kplowrap->Write("KPlusLowRapidity",TObject::kOverwrite);
            hist_kplowrap_qval->Write(("KPlusLowRapidity"+delim).c_str(),TObject::kOverwrite);  
            hist_kphigh_P_qval->Write(("KPlusHighP"+delim).c_str(),TObject::kOverwrite);
            hist_kplow_P_qval->Write(("KPlusLowP"+delim).c_str(),TObject::kOverwrite);
            hist_kpmomsep_qval->Write(("KPlusMomSep"+delim).c_str(),TObject::kOverwrite);
            hist_tdist_qval->Write(("TDist"+delim).c_str(),TObject::kOverwrite);  
        }
    else if(delim=="_mc")
        {
            auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"kphigh_rapidity","kplow_rapidity","kp_lowp_P3","kp_highp_P3","t_dist","t_dist_truth","hybrid_combo"})
                .Define("kp_momsep","fabs(kp_highp_P3 - kp_lowp_P3)");
        
            //df1.Report()->Print();
        
            //raw kinfit kaons
            auto hist_kphighrap = df.Histo1D({"KPlusHighRapidity_mc"," ; y(K^{+}_{#lower[-0.3]{F}}); Events",80,0,5} , "kphigh_rapidity","hybrid_combo");
            auto hist_kplowrap = df.Histo1D({"KPlusLowRapidity_mc"," ; y(K^{+}_{#lower[-0.3]{S}}); Events",200u,-1.5,5} , "kplow_rapidity","hybrid_combo");
            auto hist_kphigh_P = df.Histo1D({"KPlusHighP_mc"," ; #left|#vec{#bf{p}}(K^{+}_{#lower[-0.3]{F}})#right| (GeV); Events",80u,0,10}, "kp_highp_P3","hybrid_combo");
            auto hist_kplow_P = df.Histo1D({"KPlusLowP_mc"," ; #left|#vec{#bf{p}}(K^{+}_{#lower[-0.3]{S}})#right| (GeV); Events",80,0,4}, "kp_lowp_P3","hybrid_combo");
            auto hist_kpmomsep = df.Histo1D({"KPlusMomSep_mc"," ; y(K^{+}_{#lower[-0.3]{S}}); Events",80u,0,10} , "kp_momsep","hybrid_combo");
            auto hist_tdist = df.Histo1D({"TDist_mc"," ; -t ({GeV}^{2} ); Events",80,0,5}, "t_dist_truth","hybrid_combo");
            
            hist_kphighrap->Write(("KPlusHighRapidity"+delim).c_str(),TObject::kOverwrite);  
            hist_kplowrap->Write(("KPlusLowRapidity"+delim).c_str(),TObject::kOverwrite);  
            hist_kphigh_P->Write(("KPlusHighP"+delim).c_str(),TObject::kOverwrite);
            hist_kplow_P->Write(("KPlusLowP"+delim).c_str(),TObject::kOverwrite);
            hist_kpmomsep->Write(("KPlusMomSep"+delim).c_str(),TObject::kOverwrite);
            hist_tdist->Write(("TDist"+delim).c_str(),TObject::kOverwrite);  
        }
}

void combineAllHistograms(TFile *file) {
    //
    file->ReOpen("UPDATE");
    // Access the three TDirectories
    TDirectory* dir1 = (TDirectory*)file->Get("Spring_2017");
    TDirectory* dir2 = (TDirectory*)file->Get("Spring_2018");
    TDirectory* dir3 = (TDirectory*)file->Get("Fall_2018");

    if (!dir1 || !dir2 || !dir3) {
        std::cerr << "Error: One or more TDirectories not found" << std::endl;
        file->Close();
        return;
    }

    // Get the list of keys from one directory (assuming all directories have the same histogram names)
    TList* keys = dir1->GetListOfKeys();

    // Loop over all the keys in the first directory
    TIter next(keys);
    TKey* key;
    while ((key = (TKey*)next())) {
        // Check if the key is for a histogram
        TObject* obj = key->ReadObj();
        if (obj->InheritsFrom(TH1::Class())) {
            const char* histName = obj->GetName();
            cout << "HistName to retrieve: " << histName << endl;
            // Retrieve the histograms with the same name from each directory
            TH1* hist1 = (TH1*)dir1->Get(histName);
            TH1* hist2 = (TH1*)dir2->Get(histName);
            TH1* hist3 = (TH1*)dir3->Get(histName);

            if (!hist1 || !hist2 || !hist3) {
                std::cerr << "Error: One or more histograms not found for " << histName << std::endl;
                continue;
            }

            // Clone the first histogram and reset it for combination
            TH1* combinedHist = (TH1*)hist1->Clone();
            //combinedHist->SetDirectory(0);  // Detach from TDirectory to avoid memory issues
            combinedHist->Reset();          // Clear the contents of the cloned histogram

            // Add the histograms together
            combinedHist->Add(hist1);
            combinedHist->Add(hist2);
            combinedHist->Add(hist3);

            // Write the combined histogram to the output file
            file->cd();
            combinedHist->Write();

            // Clean up
            delete combinedHist;
        }
    }

    std::cout << "All histograms combined and saved to TFile." << std::endl;
}
