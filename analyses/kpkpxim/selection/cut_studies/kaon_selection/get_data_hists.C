#include "gxana/common/Paths.h"
/* ------------------------------------------------------------------
# [Jesse A. Hernandez]
#     Use the Rdataframe to get histograms into root file
# ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string delim, TFile *save_file, Int_t n_threads=16);
void make_root_tree(string delim="_vertexCuts")  ;

//main function
int get_data_hists()
{
  //make the root trees with different cuts
  // make_root_tree();
  //make_root_tree("_ximVertexCut");
  make_root_tree("_allKaonSep");
  
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
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_nominal"+delim+"_Weighted.root", "_mc", f);
  save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__M23_2017-01_ana56_gen_amp_nominal"+delim+"_Weighted.root", "_thrown", f);
  //spring 2018
  f->cd();
  f->cd("Spring_2018");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_nominal"+delim+"_Weighted.root", "_mc", f);
  save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__B4_M23_2018-01_ana03_gen_amp_nominal"+delim+"_Weighted.root", "_thrown", f);
  //fall 2018
  f->cd();
  f->cd("Fall_2018");
  save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_1111111.root", "_qval", f);
  save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_nominal"+delim+"_Weighted.root", "_mc", f);
  save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02_gen_amp_nominal"+delim+"_Weighted.root", "_thrown", f);
  f->Close();
}

void save_from_flattrees(string root_file_path, string delim, TFile *save_file, Int_t n_threads=20)
{
  //Declare Variables
  string weight;
  //Multithreating
  if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
  if(delim=="_qval")
    weight = "qvalue_decayxim_M";
  else
    weight = "mc_weight";
      
  //Import select braches to speed things up
    
  // make data frame and braches for histograms from 4 vectors
  // format : tree name, file name, branches to open
  if(delim!="_thrown")
    {
      auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"kp1_p4_kin","kp2_p4_kin","kphigh_p4","kplow_p4","kp1_P3_Truth","kp2_P3_Truth", weight})
        //.Filter("decayxim_M>1.308 && decayxim_M<1.334")
        .Define("kp1_Theta","kp1_p4_kin.Theta()*180/TMath::Pi()")
        .Define("kp2_Theta","kp2_p4_kin.Theta()*180/TMath::Pi()")
        .Define("kplow_Theta","kplow_p4.Theta()*180/TMath::Pi()")  
        .Define("kphigh_Theta","kphigh_p4.Theta()*180/TMath::Pi()")
        .Define("kplow_P3","kplow_p4.P()")
        .Define("kphigh_P3","kphigh_p4.P()")
        .Define("kp_high_low_sep","kphigh_P3-kplow_P3");
      
      //df1.Report()->Print();
  
      //raw kinfit kaons
      auto hist_kp1_P_Theta = df.Histo2D({""," ; #Theta(#gammaK^{+}_{1}) (#circ);#left|#vec{#bf{p}}(K^{+}_{1})#right| (GeV)",180u,0,180,100u,0,10}, "kp1_Theta","kp1_P3",weight);
      auto hist_kp2_P_Theta = df.Histo2D({""," ; #Theta(#gammaK^{+}_{2}) (#circ);#left|#vec{#bf{p}}(K^{+}_{2})#right| (GeV)",180u,0,180,100u,0,10}, "kp2_Theta","kp2_P3",weight);
      auto hist_kp12_P_Theta = (TH1D*)hist_kp1_P_Theta->Clone("");
      hist_kp12_P_Theta->SetTitle(" ; #Theta(#gammaK^{+}_{1,2}) (#circ);#left|#vec{#bf{p}}(K^{+}_{1,2})#right| (GeV)");
      hist_kp12_P_Theta->Add(hist_kp2_P_Theta.GetPtr());
      auto hist_kp12_P = df.Histo2D({""," ; #left|#vec{#bf{p}}(K^{+}_{2})#right| (GeV);#left|#vec{#bf{p}}(K^{+}_{1})#right| (GeV)",100u,0,10,100u,0,10}, "kp2_P3","kp1_P3",weight);
      hist_kp12_P_Theta->Write(("kp12_p3_theta"+delim).c_str(),TObject::kOverwrite);
      hist_kp12_P->Write(("kp12_p"+delim).c_str(),TObject::kOverwrite);
      
      //kaons chosen with momentum
      auto hist_kphigh_P = df.Histo2D({""," ; #Theta(#gammaK^{+}_{fast}) (#circ);#left|#vec{#bf{p}}(K^{+}_{fast})#right| (GeV)",180u,0,180,100u,0,10}, "kphigh_Theta","kphigh_P3",weight);
      auto hist_kplow_P = df.Histo2D({""," ; #Theta(#gammaK^{+}_{slow}) (#circ);#left|#vec{#bf{p}}(K^{+}_{slow})#right| (GeV)",180u,0,180,100u,0,10}, "kplow_Theta","kplow_P3",weight);
      auto hist_kphighlow_P = df.Histo2D({""," ; #left|#vec{#bf{p}}(K^{+}_{fast}#right|;#left|#vec{#bf{p}}(K^{+}_{slow})#right| (GeV)",100u,0,10,100u,0,10}, "kplow_P3","kphigh_P3",weight);
      auto hist_kphighlowsep = df.Histo1D({""," ; #left|#vec{#bf{p}}(K^{+}_{fast}#right|-#left|#vec{#bf{p}}(K^{+}_{slow})#right| (GeV); Events",100u,0,10} ,"kp_high_low_sep",weight);

      hist_kphighlowsep->Write(("kpHighLowSep"+delim).c_str(),TObject::kOverwrite);  
      hist_kphigh_P->Write(("kphigh_p3_theta"+delim).c_str(),TObject::kOverwrite);
      hist_kplow_P->Write(("kplow_p3_theta"+delim).c_str(),TObject::kOverwrite);
      hist_kphighlowsep->Write(("kpHighLowSep"+delim).c_str(),TObject::kOverwrite);
      
      //compare kaon selection methods to truth in generated events for mc
      if(delim=="_mc")
        {
          auto hist_kp1_P_Theta_weighted = df.Histo2D({""," ; #Theta(#gammaK^{+}_{1}) (#circ);#left|#vec{#bf{p}}(K^{+}_{1})#right| (GeV)",180u,0,180,100u,0,10}, "kp1_Theta","kp1_P3","mc_weight");
          auto hist_kp2_P_Theta_weighted = df.Histo2D({""," ; #Theta(#gammaK^{+}_{2}) (#circ);#left|#vec{#bf{p}}(K^{+}_{2})#right| (GeV)",180u,0,180,100u,0,10}, "kp2_Theta","kp2_P3","mc_weight");
          auto hist_kp12_P_Theta_weighted = (TH1D*)hist_kp1_P_Theta_weighted->Clone("");
          hist_kp12_P_Theta_weighted->SetTitle(" ; #Theta(#gammaK^{+}_{1,2}) (#circ);#left|#vec{#bf{p}}(K^{+}_{1,2})#right| (GeV)");
          hist_kp12_P_Theta_weighted->Add(hist_kp2_P_Theta_weighted.GetPtr());
          hist_kp12_P_Theta_weighted->Write(("kp12_p3_theta"+delim+"_weighted").c_str(),TObject::kOverwrite);
          auto hist_kp12_P_weighted = df.Histo2D({""," ; #left|#vec{#bf{p}}(K^{+}_{2})#right| (GeV);#left|#vec{#bf{p}}(K^{+}_{1})#right| (GeV)",100u,0,10,100u,0,10}, "kp2_P3","kp1_P3","mc_weight");
          hist_kp12_P_weighted->Write(("kp12_p"+delim+"_weighted").c_str(),TObject::kOverwrite);

          
          auto hist_kphighlowsep_weighted = df.Histo1D({""," ; #left|#vec{#bf{p}}(K^{+}_{fast}#right|-#left|#vec{#bf{p}}(K^{+}_{slow})#right| (GeV); Events",100u,0,10},"kp_high_low_sep","mc_weight");      
          auto hist_kphigh_P_weighted = df.Histo2D({""," ; #Theta(#gammaK^{+}_{fast}) (#circ); #left|#vec{#bf{p}}(K^{+}_{fast})#right| (GeV)",180u,0,180,100u,0,10}, "kphigh_Theta","kphigh_P3","mc_weight");
          auto hist_kplow_P_weighted = df.Histo2D({""," ; #Theta(#gammaK^{+}_{slow}) (#circ); #left|#vec{#bf{p}}(K^{+}_{slow})#right| (GeV)",180u,0,180,100u,0,10}, "kplow_Theta","kplow_P3","mc_weight");
          hist_kphigh_P_weighted->Write(("kphigh_p3_theta"+delim+"_weighted").c_str(),TObject::kOverwrite);
          hist_kplow_P_weighted->Write(("kplow_p3_theta"+delim+"_weighted").c_str(),TObject::kOverwrite);
           
          auto hist_kphightruth_P = df.Histo2D({""," ;True #left|#vec{#bf{p}}(K^{+}_{1})#right| (GeV);#left|#vec{#bf{p}}(K^{+}_{fast})#right| (GeV)",100u,0,10,100u,0,10},"kp1_P3_Truth", "kphigh_P3");
          auto hist_kplowtruth_P = df.Histo2D({""," ;True #left|#vec{#bf{p}}(K^{+}_{2})#right| (GeV);#left|#vec{#bf{p}}(K^{+}_{slow})#right| (GeV)",100u,0,10,100u,0,10},"kp2_P3_Truth", "kplow_P3");

          auto hist_kphightruth_P_weighted= df.Histo2D({""," ;True #left|#vec{#bf{p}}(K^{+}_{1})#right| (GeV);#left|#vec{#bf{p}}(K^{+}_{fast})#right| (GeV)",100u,0,10,100u,0,10},"kp1_P3_Truth", "kphigh_P3","mc_weight");
          auto hist_kplowtruth_P_weighted= df.Histo2D({""," ;True #left|#vec{#bf{p}}(K^{+}_{2})#right| (GeV);#left|#vec{#bf{p}}(K^{+}_{slow})#right| (GeV)",100u,0,10,100u,0,10},"kp2_P3_Truth", "kplow_P3","mc_weight");

          hist_kphighlowsep_weighted->Write(("kpHighLowSep"+delim+"_weighted").c_str(),TObject::kOverwrite);
          hist_kphightruth_P->Write(("kphigh_truth_p3"+delim).c_str(),TObject::kOverwrite);
          hist_kplowtruth_P->Write(("kplow_truth_p3"+delim).c_str(),TObject::kOverwrite);

          hist_kphightruth_P_weighted->Write(("kphigh_truth_p3"+delim+"_weighted").c_str(),TObject::kOverwrite);
          hist_kplowtruth_P_weighted->Write(("kplow_truth_p3"+delim+"_weighted").c_str(),TObject::kOverwrite);
        }
    }
  else
    {
      auto df = ROOT::RDataFrame("flatTree_thrown_kpkpxim", (root_file_path).c_str(), {"kp1_p4","kp2_p4",weight})
        .Define("kp1_Theta","kp1_p4.Theta()*180/TMath::Pi()")
        .Define("kp2_Theta","kp2_p4.Theta()*180/TMath::Pi()")
        .Define("kp1_P3","kp1_p4.P()")
        .Define("kp2_P3","kp2_p4.P()");

      auto hist_kp1_P = df.Histo2D({""," ; True #Theta(#gammaK^{+}_{fast}) (#circ); True #left|#vec{#bf{p}}(K^{+}_{fast})#right| (GeV)",180u,0,180,100u,0,10}, "kp1_Theta","kp1_P3");
      auto hist_kp2_P =  df.Histo2D({""," ; True #Theta(#gammaK^{+}_{slow}) (#circ); True #left|#vec{#bf{p}}(K^{+}_{slow})#right| (GeV)",180u,0,180,100u,0,10}, "kp2_Theta","kp2_P3");
      
      auto hist_kp1_P_weighted = df.Histo2D({""," ; True #Theta(#gammaK^{+}_{fast}) (#circ); True #left|#vec{#bf{p}}(K^{+}_{fast})#right| (GeV)",180u,0,180,100u,0,10}, "kp1_Theta","kp1_P3","mc_weight");
      auto hist_kp2_P_weighted = df.Histo2D({""," ; True #Theta(#gammaK^{+}_{slow}) (#circ); True #left|#vec{#bf{p}}(K^{+}_{slow})#right| (GeV)",180u,0,180,100u,0,10}, "kp2_Theta","kp2_P3","mc_weight");

      hist_kp1_P->Write(("kp1_p3_theta"+delim).c_str(),TObject::kOverwrite);
      hist_kp2_P->Write(("kp2_p3_theta"+delim).c_str(),TObject::kOverwrite);
      hist_kp1_P_weighted->Write(("kp1_p3_theta"+delim+"_weighted").c_str(),TObject::kOverwrite);
      hist_kp2_P_weighted->Write(("kp2_p3_theta"+delim+"_weighted").c_str(),TObject::kOverwrite);
    }
}
