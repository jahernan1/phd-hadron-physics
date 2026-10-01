#include "gxana/common/Paths.h"
/* ------------------------------------------------------------------
   # [Jesse A. Hernandez]
   #     Use the Rdataframe to get histograms into root file
   # ------------------------------------------------------------------*/

//include needed libraries and fucntions
void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file,  Int_t n_threads=16);
TH2D* GetAcceptanceHist2D(TH2D* hist_genr, TH2D* hist_recon);
TH2D* GetAcceptanceCorrHist2D(vector<TH2D*> vec_hist, TFile *save_file, Bool_t weighted=false);

//main function
int get_hists()
{
    //set up root file with directories
    TFile *f =  TFile::Open( "particle_kinematics.root", "RECREATE");
    f->cd();
    gDirectory->mkdir("Spring_2017");
    f->cd();
    gDirectory->mkdir("Spring_2018");  
    f->cd();
    gDirectory->mkdir("Fall_2018");

    //initiate variables
    string root_file_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
    string root_file_dir_qval = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/");
    vector<vector<TH2D*>> vect_histo_2017(5);
    vector<vector<TH2D*>> vect_histo_201801(5);
    vector<vector<TH2D*>> vect_histo_201808(5);
    vector<TH2D*> hist_all_kin(5);
    vector<string> vec_delim = {"_qval","_mc","_thrown"};
    vector<string> particles = {"kp1_kin", "kp2_kin",
                                "pim1_kin", "pim2_kin", "proton_kin" };
    
    //perform actions
    //spring 2017
    f->cd();
    f->cd("Spring_2017");
    save_from_flattrees(root_file_dir_qval+"kpkpxim__M23_2017-01_ana56_nominal_kphighrap_1111111/postQVal_flatTree_kpkpxim__M23_2017-01_ana56_nominal_kphighrap_1111111.root", "_qval", f);
    save_from_flattrees(root_file_dir+"flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root", "_mc", f);
    save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest.root", "_thrown", f);
    //spring 2018
    f->cd();
    f->cd("Spring_2018");
    save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-01_ana03_nominal_kphighrap_1111111.root", "_qval", f);
    save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root", "_mc", f);
    save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest.root", "_thrown", f);
    //fall 2018
    f->cd();
    f->cd("Fall_2018");
    save_from_flattrees(root_file_dir_qval+"kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap_1111111.root", "_qval", f);
    save_from_flattrees(root_file_dir+"flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root", "_mc", f);
    save_from_flattrees(root_file_dir+"flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest.root", "_thrown", f);

    
    //get histos from root tree
    for(int j = 0; j < particles.size();j++)
        {
            f->ReOpen("READ");
            for(int i = 0; i < vec_delim.size(); i++)
                {
                    vect_histo_2017[j].push_back( (TH2D*)f->Get( ("Spring_2017/"+particles[j]+vec_delim[i] ).c_str() )->Clone( (particles[j]+vec_delim[i]).c_str()));
                    vect_histo_201801[j].push_back( (TH2D*)f->Get( ("Spring_2018/"+particles[j]+vec_delim[i] ).c_str() )->Clone()) ;
                    vect_histo_201808[j].push_back( (TH2D*)f->Get( ("Fall_2018/"+particles[j]+vec_delim[i] ).c_str())->Clone() );
                }
            f->ReOpen("UPDATE");
            //Perform acceptance correction
            f->cd("Spring_2017");
            hist_all_kin[j] = (TH2D*)GetAcceptanceCorrHist2D(vect_histo_2017[j], f)->Clone( (particles[j]+"_phase1_acccorr").c_str());
            //
            f->cd("Spring_2018");
            TH2D* hist_tmp = (TH2D*)GetAcceptanceCorrHist2D(vect_histo_201801[j], f)->Clone();
            hist_all_kin[j]->Add( hist_tmp );
            //
            f->cd("Fall_2018");
            TH2D* hist_tmp2 = (TH2D*)GetAcceptanceCorrHist2D(vect_histo_201808[j], f)->Clone();
            hist_all_kin[j]->Add( hist_tmp2 );
            //
            f->cd();
            hist_all_kin[j]->Write(hist_all_kin[j]->GetName(),TObject::kOverwrite);
            
            //merge all the kinematics plots without correction
            TH2D* merged_kin = (TH2D*)vect_histo_2017[j][0]->Clone( (particles[j]+"_phase1").c_str());
            merged_kin->Add( (TH2D*)vect_histo_201801[j][0]->Clone() );
            merged_kin->Add( (TH2D*)vect_histo_201808[j][0]->Clone() );
            merged_kin->Write(merged_kin->GetName(),TObject::kOverwrite);
            TH2D* merged_kin_mc = (TH2D*)vect_histo_2017[j][1]->Clone((particles[j]+"_phase1_mc").c_str());
            merged_kin_mc->Add( (TH2D*)vect_histo_201801[j][1]->Clone() );
            merged_kin_mc->Add( (TH2D*)vect_histo_201808[j][1]->Clone() );
            merged_kin_mc->Write(merged_kin_mc->GetName(),TObject::kOverwrite);
        }
          
    f->Close();
  
    return 0;
}

TH2D* GetAcceptanceHist2D(TH2D* hist_genr, TH2D* hist_recon)
{
    TH2D* hist_accept = (TH2D*)hist_recon->Clone("acceptance");
    //hist_accept->Divide(hist_recon,hist_genr,1,1,"B");
    hist_accept->Divide(hist_genr);
  
    printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
    return hist_accept;
}

//`vect_hist` [0](data),[1](recon),[2](generated)
TH2D* GetAcceptanceCorrHist2D(vector<TH2D*> vec_hist, TFile *save_file, Bool_t weighted=false)
{
    //get the acceptance
    string name = vec_hist[0]->GetName();
    TH2D* hist_accept = (TH2D*)GetAcceptanceHist2D(vec_hist[2],vec_hist[1])->Clone();
    hist_accept->Write( (name+"_acceptance").c_str(),TObject::kOverwrite);
    
    TH2D* hist_data_acccorr = (TH2D*)vec_hist[0]->Clone();
    //hist_data_acccorr->Divide(vec_hist[0],hist_accept,1,1,"B");
    hist_data_acccorr->Divide(hist_accept);
    hist_data_acccorr->Write( (name+"_acceptcorr").c_str(),TObject::kOverwrite);
    
    return hist_data_acccorr;
}

/* ****************************************************************************************************
***************************************************************************************************** 
**************************************************************************************************** */ 
void save_from_flattrees(string root_file_path, string hist_name, TFile *save_file, Int_t n_threads=20)
{
  if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
  //Import select braches to speed things up
    
  // make data frame and braches for histograms from 4 vectors
  // format : tree name, file name, branches to open
  if(hist_name=="_thrown")
    {
        auto df = ROOT::RDataFrame("flatTree_thrown_kpkpxim", (root_file_path).c_str())
            .Define("kp1_p","kp1_p4.P()")
            .Define("kp1_theta","kp1_p4.Theta()*180/TMath::Pi()")
            .Define("kp2_p","kp2_p4.P()")
            .Define("kp2_theta","kp2_p4.Theta()*180/TMath::Pi()")
            .Define("pim1_p","pim1_p4.P()")
            .Define("pim1_theta","pim1_p4.Theta()*180/TMath::Pi()")
            .Define("pim2_p","pim2_p4.P()")
            .Define("pim2_theta","pim2_p4.Theta()*180/TMath::Pi()")
            .Define("proton_p","proton_p4.P()")
            .Define("proton_theta","proton_p4.Theta()*180/TMath::Pi()");
        //make histograms and add to tfile
        auto hist_kp1 = df.Histo2D({""," ; #vartheta(K^{+}_{#it{#lower[-0.3]{fast}}}) (#circ ); #left|#vec{#bf{p}}(K^{+}_{#it{#lower[-0.3]{fast}}} )#right| (GeV)",30u,0,20,30u,0,10}, "kp1_theta","kp1_p");
        auto hist_kp2 = df.Histo2D({""," ; #vartheta(K^{+}_{#it{#lower[-0.4]{slow}}}) (#circ ); #left|#vec{#bf{p}}(K^{+}_{#it{#lower[-0.4]{slow}}} )#right| (GeV);",30u,0,150,30u,0,2.3}, "kp2_theta","kp2_p");
        
        auto hist_pim1 = df.Histo2D({""," ; #vartheta(#pi^{-}_{#Xi^{-}}) (#circ ); #left|#vec{#bf{p}}(#pi^{-}_{#Xi})#right| (GeV)",30u,0,80,30u,0,1.8}, "pim1_theta","pim1_p"); 
        auto hist_pim2 = df.Histo2D({""," ; #vartheta(#pi^{-}_{#Lambda}) (#circ ); #left|#vec{#bf{p}}(#pi^{-}_{#Lambda})#right| (GeV)",30u,0,80,30u,0,1.5}, "pim2_theta","pim2_p");
        auto hist_proton = df.Histo2D({""," ; #vartheta(p) (#circ ); #left|#vec{#bf{p}}(p)#right| (GeV)",30u,0,50,30u,0,7}, "proton_theta","proton_p");
        //
        hist_kp1->Write(("kp1_kin"+hist_name).c_str(),TObject::kOverwrite);
        hist_kp2->Write(("kp2_kin"+hist_name).c_str(),TObject::kOverwrite);
        hist_pim1->Write(("pim1_kin"+hist_name).c_str(),TObject::kOverwrite);
        hist_pim2->Write(("pim2_kin"+hist_name).c_str(),TObject::kOverwrite);
        hist_proton->Write(("proton_kin"+hist_name).c_str(),TObject::kOverwrite);
    }
  else if(hist_name=="_mc")
      {
          auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str())
              .Define("kp1_p","kphigh_p4.P()")
              .Define("kp1_theta","kphigh_p4.Theta()*180/TMath::Pi()")
              .Define("kp2_p","kplow_p4.P()")
              .Define("kp2_theta","kplow_p4.Theta()*180/TMath::Pi()")
              .Define("pim1_p","pim1_p4_kin.P()")
              .Define("pim1_theta","pim1_p4_kin.Theta()*180/TMath::Pi()")
              .Define("pim2_p","pim2_p4_kin.P()")
              .Define("pim2_theta","pim2_p4_kin.Theta()*180/TMath::Pi()")
              .Define("proton_p","p_p4_kin.P()")
              .Define("proton_theta","p_p4_kin.Theta()*180/TMath::Pi()");
          
          //make histograms and add to tfile
          auto hist_kp1 = df.Histo2D({""," ; #vartheta(K^{+}_{#it{#lower[-0.3]{fast}}}) (#circ ); #left|#vec{#bf{p}}(K^{+}_{#it{#lower[-0.3]{fast}}} )#right| (GeV)",30u,0,20,30u,0,10}, "kp1_theta","kp1_p","hybrid_combo");
          auto hist_kp2 = df.Histo2D({""," ; #vartheta(K^{+}_{#it{#lower[-0.4]{slow}}}) (#circ ); #left|#vec{#bf{p}}(K^{+}_{#it{#lower[-0.4]{slow}}} )#right| (GeV);",30u,0,150,30u,0,2.3}, "kp2_theta","kp2_p","hybrid_combo");
          
          auto hist_pim1 = df.Histo2D({""," ; #vartheta(#pi^{-}_{#Xi^{-}}) (#circ ); #left|#vec{#bf{p}}(#pi^{-}_{#Xi})#right| (GeV)",30u,0,80,30u,0,1.8}, "pim1_theta","pim1_p","hybrid_combo"); 
          auto hist_pim2 = df.Histo2D({""," ; #vartheta(#pi^{-}_{#Lambda}) (#circ ); #left|#vec{#bf{p}}(#pi^{-}_{#Lambda})#right| (GeV)",30u,0,80,30u,0,1.5}, "pim2_theta","pim2_p","hybrid_combo");
          auto hist_proton = df.Histo2D({""," ; #vartheta(p) (#circ ); #left|#vec{#bf{p}}(p)#right| (GeV)",30u,0,50,30u,0,7}, "proton_theta","proton_p","hybrid_combo");
          //
          hist_kp1->Write(("kp1_kin"+hist_name).c_str(),TObject::kOverwrite);
          hist_kp2->Write(("kp2_kin"+hist_name).c_str(),TObject::kOverwrite);
          hist_pim1->Write(("pim1_kin"+hist_name).c_str(),TObject::kOverwrite);
          hist_pim2->Write(("pim2_kin"+hist_name).c_str(),TObject::kOverwrite);
          hist_proton->Write(("proton_kin"+hist_name).c_str(),TObject::kOverwrite);
      }
  else
      {
          auto df = ROOT::RDataFrame("flatTree_kpkpxim", (root_file_path).c_str(), {"t_dist", "xim_costheta_gen_amp", "ystar_M","best_combo","qvalue_decayxim_M"})
              .Define("kp1_p","kphigh_p4.P()")
              .Define("kp1_theta","kphigh_p4.Theta()*180/TMath::Pi()")
              .Define("kp2_p","kplow_p4.P()")
              .Define("kp2_theta","kplow_p4.Theta()*180/TMath::Pi()")
              .Define("pim1_p","pim1_p4_kin.P()")
              .Define("pim1_theta","pim1_p4_kin.Theta()*180/TMath::Pi()")
              .Define("pim2_p","pim2_p4_kin.P()")
              .Define("pim2_theta","pim2_p4_kin.Theta()*180/TMath::Pi()")
              .Define("proton_p","p_p4_kin.P()")
              .Define("proton_theta","p_p4_kin.Theta()*180/TMath::Pi()")
              .Define("qvalue_hybrid","qvalue_decayxim_M*hybrid_combo");

          //make histograms and add to tfile
          auto hist_kp1 = df.Histo2D({""," ; #vartheta(K^{+}_{#it{#lower[-0.3]{fast}}}) (#circ ); #left|#vec{#bf{p}}(K^{+}_{#it{#lower[-0.3]{fast}}} )#right| (GeV)",30u,0,20,30u,0,10}, "kp1_theta","kp1_p","qvalue_hybrid");
          auto hist_kp2 = df.Histo2D({""," ; #vartheta(K^{+}_{#it{#lower[-0.4]{slow}}}) (#circ ); #left|#vec{#bf{p}}(K^{+}_{#it{#lower[-0.4]{slow}}} )#right| (GeV);",30u,0,150,30u,0,2.3}, "kp2_theta","kp2_p","qvalue_hybrid");
          
          auto hist_pim1 = df.Histo2D({""," ; #vartheta(#pi^{-}_{#Xi^{-}}) (#circ ); #left|#vec{#bf{p}}(#pi^{-}_{#Xi})#right| (GeV)",30u,0,80,30u,0,1.8}, "pim1_theta","pim1_p","qvalue_hybrid"); 
          auto hist_pim2 = df.Histo2D({""," ; #vartheta(#pi^{-}_{#Lambda}) (#circ ); #left|#vec{#bf{p}}(#pi^{-}_{#Lambda})#right| (GeV)",30u,0,80,30u,0,1.5}, "pim2_theta","pim2_p","qvalue_hybrid");
          auto hist_proton = df.Histo2D({""," ; #vartheta(p) (#circ ); #left|#vec{#bf{p}}(p)#right| (GeV)",30u,0,50,30u,0,7}, "proton_theta","proton_p","qvalue_hybrid");
          //
          hist_kp1->Write(("kp1_kin"+hist_name).c_str(),TObject::kOverwrite);
          hist_kp2->Write(("kp2_kin"+hist_name).c_str(),TObject::kOverwrite);
          hist_pim1->Write(("pim1_kin"+hist_name).c_str(),TObject::kOverwrite);
          hist_pim2->Write(("pim2_kin"+hist_name).c_str(),TObject::kOverwrite);
          hist_proton->Write(("proton_kin"+hist_name).c_str(),TObject::kOverwrite);
      } 
}
