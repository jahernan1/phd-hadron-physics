#include "gxana/common/Paths.h"
/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void make_data_hist(string cutSet, string delim= "_kphighrap");
void save_from_flattrees(string root_file_path, TFile *save_file, string cutSet, string delim="", Int_t n_threads=16);

//main function
int get_data_hists_RF()
{
  //make_data_hist("xim_pathlensig>2 && lambda_pathlensig>1", "_vertexCuts");
  make_data_hist("xim_pathlensig>2 && lambda_pathlensig>0 && kphigh_rap>2");
  return 0;
}

void make_data_hist(string cutSet, string delim= "_kphighrap")
{
  cout << "Exporting histograms to " << "data"+delim+"_RF.root" << endl; 
  //set up root file with directories
  TFile *f =  TFile::Open( ("data"+delim+"_RF.root").c_str(), "RECREATE");
  f->cd();
  gDirectory->mkdir("Spring_2017");
  f->cd();
  gDirectory->mkdir("Spring_2018");  
  f->cd();
  gDirectory->mkdir("Fall_2018");

  //initiate variables
  string root_file_dir = gxana::EnvPath("GXANA_DATA", "Trees/flatTree/rawTrees/");
  
  //perform actions
  //spring 2017
  f->cd();
  f->cd("Spring_2017");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56.root", f, cutSet);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2.root", f, cutSet, "_mc");
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03.root", f, cutSet);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2.root", f, cutSet, "_mc");
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02.root", f, cutSet);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2.root", f, cutSet, "_mc");
}

void save_from_flattrees(string root_file_path, TFile *save_file, string cutSet, string delim="", Int_t n_threads=16)
{
  if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
  //Import select braches to speed things up
    
  // make data frame
  // format : tree name, file name, branches to open
  auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"total_mm2"})
      .Define("kphigh_rap","kphigh_p4.Rapidity()")
      .Define("hybrid_combo","best_combo_rf*acc_weight")
      .Filter("beam_E > 6.4 && beam_E < 11.4")
      .Filter("chisqndf<14","chiSqnNfCut")
      .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")
      .Filter(cutSet.c_str(), "kinematicCuts")
      .Filter("decayxim_M<1.334 && decayxim_M>1.308","xiMassCut");
      
  auto hist = df.Histo1D({"","; #left|MM(#gammapK^{+}K^{+}#Xi^{-})#right|^{2} #left(GeV^{2}#right); Events",100,-0.04,0.04}, "total_mm2","hybrid_combo");
  hist->Write(("hist_total_mm2"+delim).c_str(),TObject::kOverwrite);
}
