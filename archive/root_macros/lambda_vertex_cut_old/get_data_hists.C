/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string delim, TFile *save_file, string chiSqCut="chisqndf<8", Int_t n_threads=20);

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
  //save_from_flattrees(root_file_dir+"flatTree_kpkplambda__M23_2017-01_ver56_nominal.root", "", f);
  save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ver56_nominal_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ver56_nominal_1111111.root", "_qval", f, "chisqndf<8", true);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_nominal_Weighted.root", "_mc", f);
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ver03_nominal.root", "", f );
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ver03_nominal_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ver03_nominal_1111111.root", "_qval", f, "chisqndf<8", true);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600_nominal_Weighted.root", "_mc", f);
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  //save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ver02_nominal.root", "", f);
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ver02_nominal_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ver02_nominal_1111111.root", "_qval", f, "chisqndf<8", true);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600_nominal_Weighted.root", "_mc", f);
  
  return 0;
}

void save_from_flattrees(string root_file_path, string delim, TFile *save_file, string chiSqCut="chisqndf<8", Int_t n_threads=20)
{
  if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
  //Import select braches to speed things up
    
  // make data frame
  // format : tree name, file name, branches to open
  if(delim=="_mc")
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"lambda_pathlen","lambda_pathlensig","xim_vertexZ","lambda_vertexZ","mc_weight"})
        .Define("lambda_pathlen_diff","lambda_vertexZ-xim_vertexZ")
        .Filter("xim_pathlensig > 2", "ximPathSigCut")
        .Filter("decayxim_M<1.334 && decayxim_M>1.31","lambdaassCut");
      
      auto hist = df.Histo1D({"",";#Lambda Pathlength Significance; Events",100u,0,20}, "lambda_pathlensig");
      auto hist0 = df.Histo1D({"",";#Lambda Pathlength (cm); Events",100u,-20,60}, "lambda_pathlen");
      auto hist1 = df.Histo1D({"",";Z_{#Lambda} - Z_{#Xi} (cm); Events",200u,-12,60}, "lambda_pathlen_diff");
      hist->Write( ("lambda_pathlensig"+delim).c_str(),TObject::kOverwrite);
      hist0->Write( ("lambda_pathlen"+delim).c_str(),TObject::kOverwrite);
      hist1->Write( ("lambda_pathlen_diff"+delim).c_str(),TObject::kOverwrite);
     
      auto histw = df.Histo1D({"",";#Lambda Pathlength Significance; Events",100u,0,20}, "lambda_pathlensig", "mc_weight");
      auto hist0w = df.Histo1D({"",";#Lambda Pathlength (cm); Events",100u,-20,60}, "lambda_pathlen","mc_weight");
      auto hist1w = df.Histo1D({"",";Z_{#Lambda} - Z_{#Xi} (cm); Events",200u,-12,60}, "lambda_pathlen_diff","mc_weight");
      histw->Write( ("lambda_pathlensig"+delim+"_weighted").c_str(),TObject::kOverwrite);
      hist0w->Write( ("lambda_pathlen"+delim+"_weighted").c_str(),TObject::kOverwrite);
      hist1w->Write( ("lambda_pathlen_diff"+delim+"_weighted").c_str(),TObject::kOverwrite);
    }
  else
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"lambda_pathlen","lambda_pathlensig","xim_vertexZ","lambda_vertexZ","qvalue_decayxim_M"})
        .Define("lambda_pathlen_diff","lambda_vertexZ-xim_vertexZ")
        .Filter("xim_pathlensig > 2", "ximPathSigCut");

      auto df1 = df.Filter("decayxim_M<1.334 && decayxim_M>1.31","lambdaassCut");
      
      auto hist = df1.Histo1D({"",";#Lambda Pathlength Significance; Events",100u,0,20}, "lambda_pathlensig");
      auto hist0 = df1.Histo1D({"",";#Lambda Pathlength (cm); Events",100u,-20,60}, "lambda_pathlen");
      auto hist1 = df1.Histo1D({"",";Z_{#Lambda} - Z_{#Xi} (cm); Events",200u,-12,60}, "lambda_pathlen_diff");
      hist->Write( "lambda_pathlensig",TObject::kOverwrite);
      hist0->Write( "lambda_pathlen",TObject::kOverwrite);
      hist1->Write( "lambda_pathlen_diff",TObject::kOverwrite);
     
      auto histw = df.Histo1D({"",";#Lambda Pathlength Significance; Events",100u,0,20}, "lambda_pathlensig", "qvalue_decayxim_M");
      auto hist0w = df.Histo1D({"",";#Lambda Pathlength (cm); Events",100u,-20,60}, "lambda_pathlen","qvalue_decayxim_M");
      auto hist1w = df.Histo1D({"",";Z_{#Lambda} - Z_{#Xi} (cm); Events",200u,-12,60}, "lambda_pathlen_diff","qvalue_decayxim_M");
      histw->Write( ("lambda_pathlensig"+delim).c_str(),TObject::kOverwrite);
      hist0w->Write( ("lambda_pathlen"+delim).c_str(),TObject::kOverwrite);
      hist1w->Write( ("lambda_pathlen_diff"+delim).c_str(),TObject::kOverwrite);
    }
  
  //hist->SaveAs(("c_format_plots/"+hist_name+".C").c_str());(this command take long for large data)
  
}
