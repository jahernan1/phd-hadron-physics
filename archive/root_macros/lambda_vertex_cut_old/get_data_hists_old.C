/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file, Bool_t delim=true, Int_t n_threads=16);

//main function
int get_data_hists()
{
  //set up root file with directories
  TFile *f =  TFile::Open( "data.root", "RECREATE");
  f->cd();
  gDirectory->mkdir("Spring_2017");
  f->cd();
  gDirectory->mkdir("Spring_2018");  
  f->cd();
  gDirectory->mkdir("Fall_2018");

  //initiate variables
  string root_file_dir = "/d/grid17/hjesse/AnalysisNote/flatTrees/";
  string root_file_dir_qval = "/d/grid17/hjesse/AnalysisNote/QFactors/logs/";

  //perform actions
  //spring 2017
  f->cd();
  f->cd("Spring_2017");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ver56_nominal_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ver56_nominal_1111111.root", "hist_lambda_pathlensig_qval", f, false);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_nominal_Weighted.root", "hist_lambda_pathlensig_mc", f);
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ver03_nominal_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ver03_nominal_1111111.root", "hist_lambda_pathlensig_qval", f, false);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600_nominal_Weighted.root", "hist_lambda_pathlensig_mc", f);
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ver02_nominal_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ver02_nominal_1111111.root", "hist_lambda_pathlensig_qval", f, false);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600_nominal_Weighted.root", "hist_lambda_pathlensig_mc", f);
  
  return 0;
}

void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file, Bool_t delim=true, Int_t n_threads=20)
{
  if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
  //Import select braches to speed things up
    
  // make data frame
  // format : tree name, file name, branches to open
  if(delim)
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"lambda_pathlensig"})
        .Filter("beam_E > 6.4 && beam_E < 11.4")
        .Filter("best_combo==1","bestCombo")
        .Filter("xim_vertex_weight==1")
        .Filter("lambda_vertex_weight==1")
        .Filter("chisqndf<8","chiSqnNfCut")
        .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")
        .Filter("xim_pathlensig>2","ximPathlenCut")
        .Filter("decayxim_M<1.334 && decayxim_M>1.31","ximMassCut");
      auto hist = df.Histo1D({"",";#Lambda Pathlength Significance; Events",100u,0,20}, "lambda_pathlensig");
      hist->Write(hist_name.c_str(),TObject::kOverwrite);
    }
  else
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"lambda_pathlensig"});
      auto hist = df.Histo1D({"",";#Lambda Pathlength Significance; Events",100u,0,20}, "lambda_pathlensig","qvalue_decayxim_M");
      hist->Write(hist_name.c_str(),TObject::kOverwrite);
    }
  
  //hist->SaveAs(("c_format_plots/"+hist_name+".C").c_str());(this command take long for large data)
  
}
