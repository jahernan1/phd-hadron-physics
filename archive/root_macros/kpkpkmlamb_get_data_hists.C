/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, TFile *save_file, string cutSet, string delim="", Int_t n_threads=20);
void make_data_hists(string cutSet, string delim= "_allCuts");

//main function
int get_data_hists()
{
  make_data_hists("chisqndf<100 && xim_pathlensig>2 && lambda_pathlensig>1",
                  "_vertexCuts");
  make_data_hists("chisqndf<100 && xim_pathlensig>2 && lambda_pathlensig>0 && kp_highp_P3>2.3 && kp_lowp_P3<2.3");
  return 0;
}
/*
cutSet takes string formated as "branchName [>,<,=] cutValue && ... "
*/
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
  string root_file_dir = "/d/grid13/hjesse/Trees/flatTree/rawTrees/";
  
  //perform actions
  //spring 2017
  f->cd();
  f->cd("Spring_2017");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56.root", f, cutSet, "");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_Ystar2400_1600_genr8.root", f, cutSet, "_mc");
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03.root", f, cutSet, "");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600_genr8.root", f, cutSet, "_mc");
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02.root", f, cutSet, "");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600_genr8.root", f, cutSet, "_mc");
  f->Close();
}

void save_from_flattrees(string root_file_path, TFile *save_file, string cutSet, string delim="", Int_t n_threads=16)
{
  if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
  //Import select braches to speed things up
    
  // make data frame
  // format : tree name, file name, branches to open
  auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"chisqndf"})
    .Filter("beam_E > 6.4 && beam_E < 11.4")
    .Filter("best_combo==1","bestCombo")
    .Filter("abs(total_mm2) < 0.02 ","mm2Cut")
    .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")//nominal cuts above
    .Filter(cutSet.c_str(), " kinematicCuts")//extra cuts 
    .Filter("decayxim_M<1.334 && decayxim_M>1.308","xiMassCut");

  auto hist = df.Histo1D({"",";#chi^{2}_{#nu}; Events",100u,0,16}, "chisqndf");
  hist->Write(("hist_chisqndf"+delim).c_str(),TObject::kOverwrite);  
}
