#include "gxana/common/Paths.h"
/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string delim, TFile *save_file, string cutSet, Int_t n_threads=16);
void make_data_hists(string cutSet, string delim= "_allCuts");
  
//main function
int get_data_hists()
{
  make_data_hists("lambda_pathlensig>0","_vertexCuts");
  //make_data_hists("lambda_pathlensig>0 && kp_highp_P3-kp_lowp_P3>1.2");
  
  return 0;
}
//
void make_data_hists(string cutSet, string delim= "_allCuts")
{
  //set up root file with directories
  cout << "Exporting histograms to " << "data"+delim+".root" << endl; 
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
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_nominal.root", "", f, cutSet);
  save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ana56_nominal_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal_1111111.root", "_qval", f, cutSet);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_Ystar2400_1600_genr8_nominal_Weighted.root", "_mc", f, cutSet);
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal.root", "", f, cutSet );
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ana03_nominal_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_1111111.root", "_qval", f, cutSet);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600_genr8_nominal_Weighted.root", "_mc", f, cutSet);
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal.root", "", f, cutSet);
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ana02_nominal_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_1111111.root", "_qval", f, cutSet);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600_genr8_nominal_Weighted.root", "_mc", f, cutSet);

  return;  // gxana: ROOT 6.40 rejects `return 0;` in a void function
}

void save_from_flattrees(string root_file_path, string delim, TFile *save_file, string cutSet, Int_t n_threads=16)
{
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
    //Import select braches to speed things up
    string weight="";
    if(delim=="_mc") weight="mc_weight";
    else weight="qvalue_decayxim_M";
                     
    // make data frame
    // format : tree name, file name, branches to open
    auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"xim_pathlen","xim_pathlensig","lambda_pathlensig","beam_vertexZ","xim_vertexZ","kp_highp_P3","kp_lowp_P3",weight.c_str()})
      .Define("xim_pathlen_diff","xim_vertexZ-beam_vertexZ")
      .Filter(cutSet.c_str(), "kinematicCuts")
      .Filter("decayxim_M<1.334 && decayxim_M>1.308","xiMassCut");
    //
    auto df1 = df.Filter("xim_pathlensig>0 && lambda_pathlensig>0");
    //
    //MAKE HISTOGRAMS
    auto hist = df1.Histo1D({"",";#Xi Pathlength Significance; Events",100u,0,20}, "xim_pathlensig");
    auto hist0 = df1.Histo1D({"",";#Xi Pathlength (cm); Events",100u,00,60}, "xim_pathlen");
    auto hist1 = df1.Histo1D({"",";Z_{#Xi} - Z_{#gamma} (cm); Events",100u,0,60}, "xim_pathlen_diff");
    hist->Write( ("xim_pathlensig"+delim).c_str(),TObject::kOverwrite);
    hist0->Write( ("xim_pathlen"+delim).c_str(),TObject::kOverwrite);
    hist1->Write( ("xim_pathlen_diff"+delim).c_str(),TObject::kOverwrite);
    //
    auto histnv = df.Histo1D({"",";#Xi Pathlength Significance; Events",100u,-10,20}, "xim_pathlensig");
    auto hist0nv = df.Histo1D({"",";#Xi Pathlength (cm); Events",100u,-10,60}, "xim_pathlen");
    auto hist1nv = df.Histo1D({"",";Z_{#Xi} - Z_{#gamma} (cm); Events",100u,-10,60}, "xim_pathlen_diff");
    histnv->Write( ("xim_pathlensig_novertex"+delim).c_str(),TObject::kOverwrite);
    hist0nv->Write( ("xim_pathlen_novertex"+delim).c_str(),TObject::kOverwrite);
    hist1nv->Write( ("xim_pathlen_diff_novertex"+delim).c_str(),TObject::kOverwrite);
    /////////////////////////////////////////////////////////////////////////////////////
    auto histw = df1.Histo1D({"",";#Xi Pathlength Significance; Events",100u,0,20}, "xim_pathlensig", weight.c_str());
    auto hist0w = df1.Histo1D({"",";#Xi Pathlength (cm); Events",100u,0,60}, "xim_pathlen",weight.c_str());
    auto hist1w = df1.Histo1D({"",";Z_{#Xi} - Z_{#gamma} (cm); Events",100u,0,60}, "xim_pathlen_diff",weight.c_str());
    histw->Write( ("xim_pathlensig"+delim+"_weighted").c_str(),TObject::kOverwrite);
    hist0w->Write( ("xim_pathlen"+delim+"_weighted").c_str(),TObject::kOverwrite);
    hist1w->Write( ("xim_pathlen_diff"+delim+"_weighted").c_str(),TObject::kOverwrite);
    //
    auto histnvw = df.Histo1D({"",";#Xi Pathlength Significance; Events",100u,-10,20}, "xim_pathlensig", weight.c_str());
    auto histnv0w = df.Histo1D({"",";#Xi Pathlength (cm); Events",100u,-10,60}, "xim_pathlen",weight.c_str());
    auto histnv1w = df.Histo1D({"",";Z_{#Xi} - Z_{#gamma} (cm); Events",200u,-10,60}, "xim_pathlen_diff",weight.c_str());
    histnvw->Write( ("xim_pathlensig_novertex"+delim+"_weighted").c_str(),TObject::kOverwrite);
    histnv0w->Write( ("xim_pathlen_novertex"+delim+"_weighted").c_str(),TObject::kOverwrite);
    histnv1w->Write( ("xim_pathlen_diff_novertex"+delim+"_weighted").c_str(),TObject::kOverwrite);
}
