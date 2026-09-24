#include "gxana/common/Paths.h"
/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file, Bool_t bool_mc=false, Int_t n_threads=16);

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
  string root_file_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
  string root_file_dir_qval = gxana::EnvPath("GXANA_OUTPUT", "legacy_macros/QFactors/logs/");

  //perform actions
  //spring 2017
  f->cd();
  f->cd("Spring_2017");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ver56_nominal_vertexCuts.root", "", f);
  //save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ver56_nominal_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ver56_nominal_1111111.root", "hist_mm2_qval", f, false);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_nominal_vertexCuts_Weighted.root", "_mc", f, true);
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ver03_nominal_vertexCuts.root", "", f);
  //save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ver03_nominal_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ver03_nominal_1111111.root", "hist_mm2_qval", f, false);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600_nominal_vertexCuts_Weighted.root", "_mc", f, true);
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ver02_nominal_vertexCuts.root", "", f);
  //save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ver02_nominal_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ver02_nominal_1111111.root", "hist_mm2_qval", f, false);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600_nominal_vertexCuts_Weighted.root", "_mc", f, true);
  
  return 0;
}

void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file, Bool_t bool_mc=false, Int_t n_threads=20)
{
  if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
  //Import select braches to speed things up
    
  // make data frame and braches for histograms from 4 vectors
  // format : tree name, file name, branches to open
  auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"kp1_p4_kin","kp2_p4_kin","kphigh_p4","kplow_p4","kphigh_fcal_p4","kplow_bcal_p4","kp1_P3_Truth","kp2_P3_Truth"})
    .Filter("decayxim_M<1.334 && decayxim_M>1.31")
    //.Define("kp1_P3","kp1_p4_kin.P()")
    .Define("kp1_Theta","kp1_p4_kin.Theta()*180/TMath::Pi()")
    //.Define("kp2_P3","kp2_p4_kin.P()")
    .Define("kp2_Theta","kp2_p4_kin.Theta()*180/TMath::Pi()")
    .Define("kphigh_P3","kphigh_p4.P()")
    .Define("kphigh_Theta","kphigh_p4.Theta()*180/TMath::Pi()")
    .Define("kplow_P3","kplow_p4.P()")
    .Define("kplow_Theta","kplow_p4.Theta()*180/TMath::Pi()")
    .Define("kphighfcal_P3","kphigh_fcal_p4.P()")
    .Define("kphighfcal_Theta","kphigh_fcal_p4.Theta()*180/TMath::Pi()")
    .Define("kplowbcal_P3","kplow_bcal_p4.P()")
    .Define("kplowbcal_Theta","kplow_bcal_p4.Theta()*180/TMath::Pi()")
    .Define("kphigh_ellipse_P3","kphigh_ellipse_p4.P()")
    .Define("kphigh_ellipse_Theta","kphigh_ellipse_p4.Theta()*180/TMath::Pi()")
    .Define("kplow_ellipse_P3","kplow_ellipse_p4.P()")
    .Define("kplow_ellipse_Theta","kplow_ellipse_p4.Theta()*180/TMath::Pi()");
  
  auto df1 = df.Filter("kphighfcal_P3 > 0")
    .Filter("kplowbcal_P3 > 0");
  
  auto df2 = df.Filter("kphigh_ellipse_P3 > 0")
    .Filter("kplow_ellipse_P3 > 0");
  
  df1.Report()->Print();
  
  //raw kinfit kaons
  auto hist_kp1_P_Theta = df.Histo2D({""," ; #Theta(#gammaK^{+}_{1}) (^{#circ});#vec{#bf{p}}(K^{+}_{1}) (GeV)",180u,0,180,100u,0,10}, "kp1_Theta","kp1_P3");
  auto hist_kp2_P_Theta = df.Histo2D({""," ; #Theta(#gammaK^{+}_{2}) (^{#circ});#vec{#bf{p}}(K^{+}_{2}) (GeV)",180u,0,180,100u,0,10}, "kp2_Theta","kp2_P3");
  auto hist_kp12_P_Theta = (TH1D*)hist_kp1_P_Theta->Clone("");
  hist_kp12_P_Theta->SetTitle(" ; #Theta(#gammaK^{+}_{1,2}) (^{#circ});#vec{#bf{p}}(K^{+}_{1,2}) (GeV)");
  hist_kp12_P_Theta->Add(hist_kp2_P_Theta.GetPtr());
  auto hist_kp12_P = df.Histo2D({""," ; #vec{#bf{p}}(K^{+}_{2}) (GeV);#vec{#bf{p}}(K^{+}_{1}) (GeV)",100u,0,10,100u,0,10}, "kp2_P3","kp1_P3");
  hist_kp12_P_Theta->Write(("kp12_p3_theta"+hist_name).c_str(),TObject::kOverwrite);
  hist_kp12_P->Write(("kp12_p"+hist_name).c_str(),TObject::kOverwrite);
  
  //kaons chosen in elliptical region
  auto hist_kphighellipse_P_Theta = df2.Histo2D({""," ; #Theta(#gammaK^{+}_{#gammap}) (^{#circ});#vec{#bf{p}}(K^{+}_{#gammap}) (GeV)",180u,0,180,100u,0,10}, "kphigh_ellipse_Theta","kphigh_ellipse_P3");
  auto hist_kplowellipse_P_Theta = df2.Histo2D({""," ; #Theta(#gammaK^{+}_{Y^{*}}) (^{#circ});#vec{#bf{p}}(K^{+}_{Y^{*}}) (GeV)",180u,0,180,100u,0,10}, "kplow_ellipse_Theta","kplow_ellipse_P3");
  auto hist_kphighlowellipse_P = df2.Histo2D({""," ; #vec{#bf{p}}(K^{+}_{Y^{*}} (GeV);#vec{#bf{p}}(K^{+}_{#gammap}) (GeV)",100u,0,10,100u,0,10}, "kplow_ellipse_P3","kphigh_ellipse_P3");
  
  hist_kphighellipse_P_Theta->Write(("kpHighEllipse_p3_theta"+hist_name).c_str(),TObject::kOverwrite);
  hist_kplowellipse_P_Theta->Write(("kpLowEllipse_p3_theta"+hist_name).c_str(),TObject::kOverwrite);
  hist_kphighlowellipse_P->Write(("kpHighLowEllipse_p3"+hist_name).c_str(),TObject::kOverwrite);
  
  //kaons chosen with momentum
  auto hist_kphigh_P = df.Histo2D({""," ; #Theta(#gammaK^{+}_{fast}) (deg);#vec{#bf{p}}(K^{+}_{fast}) (GeV)",180u,0,180,100u,0,10}, "kphigh_Theta","kphigh_P3");
  auto hist_kplow_P = df.Histo2D({""," ; #Theta(#gammaK^{+}_{slow}) (deg});#vec{#bf{p}}(K^{+}_{slow}) (GeV)",180u,0,180,100u,0,10}, "kplow_Theta","kplow_P3");
  auto hist_kphighlow_P = df.Histo2D({""," ; #vec{#bf{p}}(K^{+}_{fast};#vec{#bf{p}}(K^{+}_{slow}) (GeV)",100u,0,10,100u,0,10}, "kplow_P3","kphigh_P3");
  
  hist_kphigh_P->Write(("kphigh_p3_theta"+hist_name).c_str(),TObject::kOverwrite);
  hist_kplow_P->Write(("kplow_p3_theta"+hist_name).c_str(),TObject::kOverwrite);
  hist_kphighlow_P->Write(("kpHighLow_p3"+hist_name).c_str(),TObject::kOverwrite);
  
  //kaons chosen with fcal/bcal hits
  auto hist_kphighfcal_P = df1.Histo2D({""," ; #Theta(#gammaK^{+}_{fcal}) (^{#circ});#vec{#bf{p}}(K^{+}_{fcal}) (GeV)",180u,0,180,100u,0,10}, "kphighfcal_Theta","kphighfcal_P3");
  auto hist_kplowbcal_P = df1.Histo2D({""," ; #Theta(#gammaK^{+}_{bcal}) (^{#circ});#vec{#bf{p}}(K^{+}_{bcal}) (GeV)",180u,0,180,100u,0,10}, "kplowbcal_Theta","kplowbcal_P3");
  auto hist_kphighlowbcalfcal_P = df1.Histo2D({""," ; #vec{#bf{p}}(K^{+}_{Y^{*}} (GeV);#vec{#bf{p}}(K^{+}_{#gammap}) (GeV)",100u,0,10,100u,0,10}, "kplowbcal_P3","kphighfcal_P3");
  
  hist_kphighfcal_P->Write(("kphighfcal_p3_theta"+hist_name).c_str(),TObject::kOverwrite);
  hist_kplowbcal_P->Write(("kplowbcal_p3_theta"+hist_name).c_str(),TObject::kOverwrite);
  hist_kphighlowbcalfcal_P->Write(("kpHighLowBcalFcal_p3"+hist_name).c_str(),TObject::kOverwrite);
  
  //compare kaon selection methods to truth in generated events for mc
  if(bool_mc)
    {
      auto hist_kp1_P_Theta_weighted = df.Histo2D({""," ; #Theta(#gammaK^{+}_{1}) (^{#circ});#vec{#bf{p}}(K^{+}_{1}) (GeV)",180u,0,180,100u,0,10}, "kp1_Theta","kp1_P3","mc_weight");
      auto hist_kp2_P_Theta_weighted = df.Histo2D({""," ; #Theta(#gammaK^{+}_{2}) (^{#circ});#vec{#bf{p}}(K^{+}_{2}) (GeV)",180u,0,180,100u,0,10}, "kp2_Theta","kp2_P3","mc_weight");
      auto hist_kp12_P_Theta_weighted = (TH1D*)hist_kp1_P_Theta_weighted->Clone("");
      hist_kp12_P_Theta_weighted->SetTitle(" ; #Theta(#gammaK^{+}_{1,2}) (^{#circ});#vec{#bf{p}}(K^{+}_{1,2}) (GeV)");
      hist_kp12_P_Theta_weighted->Add(hist_kp2_P_Theta_weighted.GetPtr());
      hist_kp12_P_Theta->Write(("kp12_p3_theta"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
      auto hist_kp12_P_weighted = df.Histo2D({""," ; #vec{#bf{p}}(K^{+}_{2}) (GeV);#vec{#bf{p}}(K^{+}_{1}) (GeV)",100u,0,10,100u,0,10}, "kp2_P3","kp1_P3","mc_weight");
      hist_kp12_P_weighted->Write(("kp12_p"+hist_name+"_weighted").c_str(),TObject::kOverwrite);

      auto hist_kphigh_P_weighted = df.Histo2D({""," ; #Theta(#gammaK^{+}_{fast}) (deg); #vec{#bf{p}}(K^{+}_{fast}) (GeV)",180u,0,180,100u,0,10}, "kphigh_Theta","kphigh_P3","mc_weight");
      auto hist_kplow_P_weighted = df.Histo2D({""," ; #Theta(#gammaK^{+}_{slow}) (deg); #vec{#bf{p}}(K^{+}_{slow}) (GeV)",180u,0,180,100u,0,10}, "kplow_Theta","kplow_P3","mc_weight");
      hist_kphigh_P_weighted->Write(("kphigh_p3_theta"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
      hist_kplow_P_weighted->Write(("kplow_p3_theta"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
           
      auto hist_kphightruth_P = df.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{1}) (GeV);#vec{#bf{p}}(K^{+}_{fast}) (GeV)",100u,0,10,100u,0,10},"kp1_P3_Truth", "kphigh_P3");
      auto hist_kplowtruth_P = df.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{2}) (GeV);#vec{#bf{p}}(K^{+}_{slow}) (GeV)",100u,0,10,100u,0,10},"kp2_P3_Truth", "kplow_P3");
      auto hist_kphighellipsetruth_P = df2.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{1}) (GeV);#vec{#bf{p}}(K^{+}_{#gammap}) (GeV)",100u,0,10,100u,0,10},"kp1_P3_Truth", "kphigh_ellipse_P3");
      auto hist_kplowellipsetruth_P = df2.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{2}) (GeV);#vec{#bf{p}}(K^{+}_{Y^{*}}) (GeV)",100u,0,10,100u,0,10},"kp2_P3_Truth", "kplow_ellipse_P3");
      auto hist_kphighfcaltruth_P = df1.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{1}) (GeV);#vec{#bf{p}}(K^{+}_{fcal}) (GeV)",100u,0,10,100u,0,10},"kp1_P3_Truth", "kphighfcal_P3");
      auto hist_kplowbcaltruth_P = df1.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{2}) (GeV);#vec{#bf{p}}(K^{+}_{bcal}) (GeV)",100u,0,10,100u,0,10},"kp2_P3_Truth", "kplowbcal_P3");

      auto hist_kphightruth_P_weighted= df.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{1}) (GeV);#vec{#bf{p}}(K^{+}_{fast}) (GeV)",100u,0,10,100u,0,10},"kp1_P3_Truth", "kphigh_P3","mc_weight");
      auto hist_kplowtruth_P_weighted= df.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{2}) (GeV);#vec{#bf{p}}(K^{+}_{slow}) (GeV)",100u,0,10,100u,0,10},"kp2_P3_Truth", "kplow_P3","mc_weight");
      auto hist_kphighellipsetruth_P_weighted= df2.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{1}) (GeV);#vec{#bf{p}}(K^{+}_{#gammap}) (GeV)",100u,0,10,100u,0,10},"kp1_P3_Truth", "kphigh_ellipse_P3","mc_weight");
      auto hist_kplowellipsetruth_P_weighted= df2.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{2}) (GeV);#vec{#bf{p}}(K^{+}_{Y^{*}}) (GeV)",100u,0,10,100u,0,10},"kp2_P3_Truth", "kplow_ellipse_P3","mc_weight");
      auto hist_kphighfcaltruth_P_weighted= df1.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{1}) (GeV);#vec{#bf{p}}(K^{+}_{fcal}) (GeV)",100u,0,10,100u,0,10},"kp1_P3_Truth", "kphighfcal_P3","mc_weight");
      auto hist_kplowbcaltruth_P_weighted= df1.Histo2D({""," ;True #vec{#bf{p}}(K^{+}_{2}) (GeV);#vec{#bf{p}}(K^{+}_{bcal}) (GeV)",100u,0,10,100u,0,10},"kp2_P3_Truth", "kplowbcal_P3","mc_weight");

      hist_kphightruth_P->Write(("kphigh_truth_p3"+hist_name).c_str(),TObject::kOverwrite);
      hist_kplowtruth_P->Write(("kplow_truth_p3"+hist_name).c_str(),TObject::kOverwrite);
      hist_kphighellipsetruth_P->Write(("kphighellipse_truth_p3"+hist_name).c_str(),TObject::kOverwrite);
      hist_kplowellipsetruth_P->Write(("kplowellipse_truth_p3"+hist_name).c_str(),TObject::kOverwrite);
      hist_kphighfcaltruth_P->Write(("kphighfcal_truth_p3"+hist_name).c_str(),TObject::kOverwrite);
      hist_kplowbcaltruth_P->Write(("kplowbcal_truth_p3"+hist_name).c_str(),TObject::kOverwrite);

      hist_kphightruth_P_weighted->Write(("kphigh_truth_p3"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
      hist_kplowtruth_P_weighted->Write(("kplow_truth_p3"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
      hist_kphighellipsetruth_P_weighted->Write(("kphighellipse_truth_p3"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
      hist_kplowellipsetruth_P_weighted->Write(("kplowellipse_truth_p3"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
      hist_kphighfcaltruth_P_weighted->Write(("kphighfcal_truth_p3"+hist_name+"_weighted").c_str(),TObject::kOverwrite);
      hist_kplowbcaltruth_P_weighted->Write(("kplowbcal_truth_p3"+hist_name+"_weighted").c_str(),TObject::kOverwrite);

    }

}
