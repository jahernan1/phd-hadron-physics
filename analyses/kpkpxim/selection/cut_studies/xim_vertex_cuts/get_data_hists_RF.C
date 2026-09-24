#include "gxana/common/Paths.h"
/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string delim, TFile *save_file, string cutSet, Int_t n_threads=16);
void make_data_hists(string cutSet, string delim= "_allKaonSep_RF");
  
//main function
int get_data_hists_RF()
{
    //make_data_hists("lambda_pathlensig>1","_vertexCuts");
    make_data_hists("lambda_pathlensig>0 && kphigh_rap>2","kphighrap");
    
    return 0;
}
//
void make_data_hists(string cutSet, string delim= "")
{
    //set up root file with directories
    cout << "Exporting histograms to " << "data_"+delim+".root" << endl; 
    TFile *f =  TFile::Open( ("data_"+delim+".root").c_str(), "RECREATE");
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
    //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_nominal.root", "", f, cutSet);
    save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ana56_nominal_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal_1111111.root", "_qval", f, cutSet);
    save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest_nominal.root", "_mc", f, cutSet);
    //spring 2018
    f->cd();
    f->cd("Spring_2018");
    //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal.root", "", f, cutSet );
    save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ana03_nominal_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_1111111.root", "_qval", f, cutSet);
    save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest_nominal.root", "_mc", f, cutSet);
    //fall 2018
    f->cd();
    f->cd("Fall_2018");
    //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal.root", "", f, cutSet);
    save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ana02_nominal_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_1111111.root", "_qval", f, cutSet);
    save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest_nominal.root", "_mc", f, cutSet);

    return;  // gxana: ROOT 6.40 rejects `return 0;` in a void function
}

void save_from_flattrees(string root_file_path, string delim, TFile *save_file, string cutSet, Int_t n_threads=16)
{
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
    //Import select braches to speed things up
    // format : tree name, file name, branches to open
    if(delim=="_qval")
        {
            auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str())
                .Define("kphigh_rap","kphigh_p4.Rapidity()")
                .Define("qvalue_acc","qvalue_decayxim_M*hybrid_combo")
                .Define("xim_pathlen_diff","xim_vertexZ-beam_vertexZ")
                .Filter(cutSet.c_str(), "kinematicCuts");
          
            auto df1 = df.Filter("xim_pathlensig>0 && lambda_pathlensig>0");
            //
            //MAKE HISTOGRAMS
            auto hist = df1.Histo1D({"",";#Xi Pathlength Significance; Events",80u,0,30}, "xim_pathlensig","qvalue_acc");
            auto hist0 = df1.Histo1D({"",";#Xi Pathlength (cm); Events",80u,00,60}, "xim_pathlen","qvalue_acc");
            auto hist1 = df1.Histo1D({"",";Z_{#Xi} - Z_{#lower[-0.2]{#it{prod}}} (cm); Events",80u,0,60}, "xim_pathlen_diff","qvalue_acc");
            hist->Write( ("xim_pathlensig"+delim).c_str(),TObject::kOverwrite);
            hist0->Write( ("xim_pathlen"+delim).c_str(),TObject::kOverwrite);
            hist1->Write( ("xim_pathlen_diff"+delim).c_str(),TObject::kOverwrite);
            //
            auto histnv = df.Histo1D({"",";#Xi Pathlength Significance; Events",80u,-5,30}, "xim_pathlensig","qvalue_acc");
            auto hist0nv = df.Histo1D({"",";#Xi Pathlength (cm); Events",80u,-5,60}, "xim_pathlen","qvalue_acc");
            auto hist1nv = df.Histo1D({"",";Z_{#Xi} - Z_{#lower[-0.2]{#it{prod}}} (cm); Events",80u,-5,60}, "xim_pathlen_diff","qvalue_acc");
            histnv->Write( ("xim_pathlensig_novertex"+delim).c_str(),TObject::kOverwrite);
            hist0nv->Write( ("xim_pathlen_novertex"+delim).c_str(),TObject::kOverwrite);
            hist1nv->Write( ("xim_pathlen_diff_novertex"+delim).c_str(),TObject::kOverwrite);
          
        }
    else
        {
            auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str())
                .Define("kphigh_rap","kphigh_p4.Rapidity()")
                .Define("xim_pathlen_diff","xim_vertexZ-beam_vertexZ")
                .Filter(cutSet.c_str(), "kinematicCuts");
            auto df1 = df.Filter("xim_pathlensig>0 && lambda_pathlensig>0");
            //
            //MAKE HISTOGRAMS
            auto hist = df1.Histo1D({"",";#Xi Pathlength Significance; Events",80u,0,30}, "xim_pathlensig","hybrid_combo");
            auto hist0 = df1.Histo1D({"",";#Xi Pathlength (cm); Events",80u,00,60}, "xim_pathlen","hybrid_combo");
            auto hist1 = df1.Histo1D({"",";Z_{#Xi} - Z_{#lower[-0.2]{#it{prod}}} (cm); Events",80u,0,60}, "xim_pathlen_diff","hybrid_combo");
            hist->Write( ("xim_pathlensig"+delim).c_str(),TObject::kOverwrite);
            hist0->Write( ("xim_pathlen"+delim).c_str(),TObject::kOverwrite);
            hist1->Write( ("xim_pathlen_diff"+delim).c_str(),TObject::kOverwrite);
            //
            auto histnv = df.Histo1D({"",";#Xi Pathlength Significance; Events",80u,-5,30}, "xim_pathlensig","hybrid_combo");
            auto hist0nv = df.Histo1D({"",";#Xi Pathlength (cm); Events",80u,-5,60}, "xim_pathlen","hybrid_combo");
            auto hist1nv = df.Histo1D({"",";Z_{#Xi} - Z_{#lower[-0.2]{#it{prod}}} (cm); Events",80u,-5,60}, "xim_pathlen_diff","hybrid_combo");
            histnv->Write( ("xim_pathlensig_novertex"+delim).c_str(),TObject::kOverwrite);
            hist0nv->Write( ("xim_pathlen_novertex"+delim).c_str(),TObject::kOverwrite);
            hist1nv->Write( ("xim_pathlen_diff_novertex"+delim).c_str(),TObject::kOverwrite);
        }
    //
    /////////////////////////////////////////////////////////////////////////////////////
    // auto histw = df1.Histo1D({"",";#Xi Pathlength Significance; Events",80u,0,20}, "xim_pathlensig", weight.c_str());
    // auto hist0w = df1.Histo1D({"",";#Xi Pathlength (cm); Events",80u,0,60}, "xim_pathlen",weight.c_str());
    // auto hist1w = df1.Histo1D({"",";Z_{#Xi} - Z_{#gamma} (cm); Events",80u,0,60}, "xim_pathlen_diff",weight.c_str());
    // histw->Write( ("xim_pathlensig"+delim+"_weighted").c_str(),TObject::kOverwrite);
    // hist0w->Write( ("xim_pathlen"+delim+"_weighted").c_str(),TObject::kOverwrite);
    // hist1w->Write( ("xim_pathlen_diff"+delim+"_weighted").c_str(),TObject::kOverwrite);
    // //
    // auto histnvw = df.Histo1D({"",";#Xi Pathlength Significance; Events",80u,-5,20}, "xim_pathlensig", weight.c_str());
    // auto histnv0w = df.Histo1D({"",";#Xi Pathlength (cm); Events",80u,-5,60}, "xim_pathlen",weight.c_str());
    // auto histnv1w = df.Histo1D({"",";Z_{#Xi} - Z_{#gamma} (cm); Events",200u,-5,60}, "xim_pathlen_diff",weight.c_str());
    // histnvw->Write( ("xim_pathlensig_novertex"+delim+"_weighted").c_str(),TObject::kOverwrite);
    // histnv0w->Write( ("xim_pathlen_novertex"+delim+"_weighted").c_str(),TObject::kOverwrite);
    // histnv1w->Write( ("xim_pathlen_diff_novertex"+delim+"_weighted").c_str(),TObject::kOverwrite);
}
