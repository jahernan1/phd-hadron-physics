#include "gxana/common/Paths.h"
//#include <RooAbsDataHelper.h>

void setStyle();
void rooFitHist(TH1* hist,char* histTitle, char* ws_name, double* yield, double* yield_err);
void GetDataMCPlots(string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111", string mc_root_file_name="flatTree_kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_allCuts_Weighted", string thrown_root_file_name="flatTree_thrown_kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8_Weighted", string delim="", int n_threads = 8);
void MakeStackedHist(vector<TH1D*> arr_hist, string stack_title, string identifier=" ", string leg_pos="tr", string leg_title="Data" );
void MakeKinematicsDataMC_RF(string delim="rapidityCuts");
    
//main
int GetKinematicsDataMC_RF(){
    
    //MakeKinematicsDataMC_RF();
    //MakeKinematicsDataMC_RF("ximVertexCut");
    MakeKinematicsDataMC_RF("kphighrap");
    
    return 0;
}

void MakeKinematicsDataMC_RF(string cutSet="rapidityCuts")
{
  vector<string> rootFile =
    {"kpkpxim__M23_2017-01_ana56_nominal_"+cutSet+"_1111111",
     "kpkpxim__B4_M23_2018-01_ana03_nominal_"+cutSet+"_1111111",
     "kpkpxim__B4_M23_2018-08_ana02_nominal_"+cutSet+"_1111111"
    };
  vector<string> rootFileMC =
    {"flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest_nominal_"+cutSet,
     "flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest_nominal_"+cutSet,
     "flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest_nominal_"+cutSet
    };
  vector<string> rootFileThrown =
    {"flatTree_thrown_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest",
     "flatTree_thrown_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest",
     "flatTree_thrown_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest"
    };
  vector<string> delim = {"2017-01_ver56_"+cutSet,"2018-01_ver03_"+cutSet,"2018-08_ver02_"+cutSet};
  
  //run analysis
  for(int loc_i=0; loc_i < rootFile.size(); loc_i++)
    {
      GetDataMCPlots(rootFile[loc_i], rootFileMC[loc_i], rootFileThrown[loc_i], delim[loc_i]);
      //GerapidityCutsAnalysis(mm2Bins, rootFile[loc_i], 0.04 , "total_mm22", "|MM(#gammapK^{+}K^{+}#Xi^{-})|^{2} (GeV^{2}/c^{4})" );
      //GerapidityCutsAnalysis(xifsBins, rootFile[loc_i], 2 , "xim_pathlensig", "#Xi Flight Significance" );
    }

}

void GetDataMCPlots(string root_file_name, string mc_root_file_name, string thrown_root_file_name, string delim, int n_threads = 8) {
	// Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	setStyle();
    
	// Branches you want to use get
	std::vector<std::string> branches =
        {"chisqndf","total_mm2","t_dist","beam_E","beam_vertexZ",
         "kp_highp_P3", "kphigh_theta","kphigh_rapidity","kphigh_prapidity",
         "ystar_M","ystar_P3","ystar_Theta","ystar_rapidity","ystar_prapidity",
         "decayxim_M","xim_lifetime_restframe","xim_pathlensig","xim_vertexZ",
         "kp_lowp_P3","kplow_theta","kplow_rapidity","kplow_prapidity",
         "lambda_vertexZ","lambda_lifetime_restframe",
         "pim1_P3","pim2_P3","proton_P3", "xim_costheta_gen_amp", "xim_costheta_hf", "hybrid_combo"
        };
    std::vector<std::string> branchesMC = branches; branches.push_back("qvalue_decayxim_M");
    std::vector<std::string> branchesT =
        {"kp_highp_P3","kp_lowp_P3","pim1_P3","pim2_P3","proton_P3","xim_costheta_gen_amp","xim_costheta_hf"};
   
    vector<string> arr_HistTitle =
      {" ; #chi^{2}_{#nu}; arb. unit",
       " ; #left|MM_{X}(#gamma p K^{+}K^{+}#Xi^{-})#right|^{2} (GeV^{2} ); arb. unit",
       " ; -t_{#lower[-0.2]{#gammaK^{+}_{#lower[-0.3]{#it{fast}}}}} (GeV^{2} ); arb. unit",
       " ; E_{#gamma} (GeV); arb. unit",
       " ; Z_{#lower[-0.2]{#it{prod}}} (cm); arb. unit",
       " ; #left|#vec{#bf{p}}(K^{+}_{#lower[-0.3]{#it{fast}}} )#right| (GeV); arb. unit",
       " ; #vartheta(K^{+}_{#lower[-0.3]{#it{fast}}}) (#circ ); arb. unit",
       " ; #eta(K^{+}_{#lower[-0.3]{#it{fast}}}) ; arb. unit",
       " ; y(K^{+}_{#lower[-0.3]{#it{fast}}}) ; arb. unit",
       " ; M(#Xi^{-}K^{+}_{#it{slow}}) (GeV); arb. unit",
       " ; #left|#vec{#bf{p}}(Y#lower[-0.5]{#scale[0.6]{*}})#right| (GeV); arb. unit",
       " ; cos#vartheta^{#gammaY#lower[-0.5]{#scale[0.6]{*}}} ; arb. unit",
       " ; #eta(Y#lower[-0.5]{#scale[0.6]{*}}) ; arb. unit",
       " ; y(Y#lower[-0.5]{#scale[0.6]{*}}) ; arb. unit",
       " ; M(#Lambda#pi^{-}) (GeV); arb. unit",
       " ; #tau_{#Xi^{-}} (ns); arb. unit",
       " ; #Xi^{-} Pathlength Significance; arb. unit",
       " ; Z_{#Xi^{-}} (cm); arb. unit",
       " ; #left|#vec{#bf{p}}(K^{+}_{#lower[-0.4]{#it{slow}}} )#right| (GeV); arb. unit",
       " ; #vartheta(K^{+}_{#lower[-0.4]{#it{slow}}}) (#circ ); arb. unit",
       " ; #eta(K^{+}_{#lower[-0.4]{#it{slow}}}) ; arb. unit",
       " ; y(K^{+}_{#lower[-0.4]{#it{slow}}}) ; arb. unit",
       " ; Z_{#Lambda} (cm); arb. unit",
       " ; #tau_{#Lambda} (ns); arb. unit",
       " ; #left|#vec{#bf{p}}(#pi^{-}_{#Xi})#right| (GeV); arb. unit",
       " ; #left|#vec{#bf{p}}(#pi^{-}_{#Lambda})#right| (GeV); arb. unit",
       " ; #left|#vec{#bf{p}}(p)#right| (GeV); arb. unit",
       " ; cos#vartheta_{#it{h^{#lower[0.2]{*}}}}^{#Xi^{-}}; arb. unit",
       " ; cos#vartheta_{#it{h}}^{#Xi^{-}}; arb. unit"
      };
    
    vector<string> arr_HistTitleThrown =
      {" ; #left|#vec{#bf{p}}(K^{+}_{#lower[-0.3]{#it{fast}}})#right| (GeV); arb. unit",                           
       " ; #left|#vec{#bf{p}}(K^{+}_{#lower[-0.4]{#it{slow}}})#right| (GeV); arb. unit",
       " ; #left|#vec{#bf{p}}(#pi^{-}_{#Xi})#right| (GeV); arb. unit",
       " ; #left|#vec{#bf{p}}(#pi^{-}_{#Lambda})#right| (GeV); arb. unit",
       " ; #left|#vec{#bf{p}}(p)#right| (GeV); arb. unit",
       " ; cos#vartheta_{#it{h^{*}}}^{#Xi^{-}}; arb. unit",
       " ; cos#vartheta_{#it{h}}^{#Xi^{-}}; arb. unit"
      };

    //Make vectors of hists for data
    std::vector<ROOT::RDF::RResultPtr<TH1D>> dataHist;
    std::vector<ROOT::RDF::RResultPtr<TH1D>> dataHistMC;
    std::vector<ROOT::RDF::RResultPtr<TH1D>> hist_ThrownMC;
    //std::vector<ROOT::RDataFrame> dfMC;
	// make data frame
	// format : tree name, file name, branches to open
	auto df = ROOT::RDataFrame("flatTree_kpkpxim", (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/")+root_file_name+"/postQVal_flatTree_"+root_file_name+".root"), branches);
    auto dfmc = ROOT::RDataFrame("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "flatTrees/")+mc_root_file_name+".root"), branchesMC);
    auto dft = ROOT::RDataFrame("flatTree_thrown_kpkpxim", (gxana::EnvPath("GXANA_DATA", "Trees/flatTree/rawTrees/")+thrown_root_file_name+".root"));
  
	// cut out any nan q-value events
    auto df1 = df.Filter("beam_E > 6.4 && beam_E < 11.4")
      .Define("qvalue_acc","qvalue_decayxim_M*hybrid_combo");
            
	auto dfmc1 = dfmc.Filter("beam_E > 6.4 && beam_E < 11.4");
    auto dft1 = dft//.Define("beam_E", "beam_p4.E()")
      .Filter("beam_E > 6.4 && beam_E < 11.4")
      .Filter("main_pid==1")
      .Define("kp_highp_P3", "kp1_p4.P()")
      .Define("kp_lowp_P3", "kp2_p4.P()")
      .Define("pim1_P3", "pim1_p4.P()")
      .Define("pim2_P3", "pim2_p4.P()")
      .Define("proton_P3", "proton_p4.P()");
    // Print Yield Value
	cout << "Sum of Weighted Q-Values: " << df1.Sum("qvalue_acc").GetValue() << endl;

    // Draw q-value subtracted distributions
    for(int loc_i=0; loc_i < branchesMC.size()-1;loc_i++)
      {
        auto hist = df1.Histo1D(branches[loc_i], "qvalue_acc");
        auto histmc = dfmc1.Histo1D(ROOT::RDF::TH1DModel(*hist), branches[loc_i],"hybrid_combo");
        dataHist.push_back(hist);
        dataHistMC.push_back(histmc);
        
        std::vector<TH1D*> vec{dataHist[loc_i].GetPtr(), dataHistMC[loc_i].GetPtr()};

        if(branchesMC[loc_i]=="xim_costheta_gen_amp" || branchesMC[loc_i]=="ystar_Theta" || branchesMC[loc_i]=="beam_vertexZ")
          MakeStackedHist(vec,arr_HistTitle[loc_i], delim,"tl");
        else
          MakeStackedHist(vec,arr_HistTitle[loc_i], delim);
      };
    
    for(int loc_j=0; loc_j < branchesT.size(); loc_j++)
      {
        auto hist = df1.Histo1D(branchesT[loc_j], "qvalue_acc");
        auto histt = dft1.Histo1D(ROOT::RDF::TH1DModel(*hist), branchesT[loc_j]);
        auto histmc1 = dfmc1.Histo1D(ROOT::RDF::TH1DModel(*hist), branchesT[loc_j],"hybrid_combo");

        hist_ThrownMC.push_back( histt);
        hist_ThrownMC.push_back( histmc1);
            
        std::vector<TH1D*> vec{hist_ThrownMC[2*loc_j].GetPtr(), hist_ThrownMC[2*loc_j+1].GetPtr()};
        if(branchesT[loc_j]=="xim_costheta_gen_amp")
          MakeStackedHist(vec,arr_HistTitleThrown[loc_j], delim+"_MC_Truth", "tl","Gen MC");
        else
          MakeStackedHist(vec,arr_HistTitleThrown[loc_j], delim+"_MC_Truth", "tr","Gen MC");
      }

    // TESTING
    // auto hist = df1.Histo1D(branches[1], "qvalue_decayxim_M");
    // auto histmc = dfmc1.Histo1D(ROOT::RDF::TH1DModel(*hist), branchesMC[1]);
    // dataHist.push_back(hist);
    // dataHistMC.push_back(histmc);
    
    // std::vector<TH1D*> vec{dataHist[0].GetPtr(), dataHistMC[0].GetPtr()};
    // MakeStackedHist(vec,arr_HistTitle[0], delim, "tl");

	//df2.Snapshot("kpkpxim_flatTree", gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_postQVal")+root_file_name+".root");
}

void MakeStackedHist(vector<TH1D*> arr_hist, string stack_title, string identifier="", string leg_pos="tr", string leg_title="Data")
{
  string plotdir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/data_mc_kinematics/");
  string histName = arr_hist[0]->GetName();  
  string saveName = plotdir + histName + "_" + identifier + "_ac.pdf";
  char entries[4][100];
  TCanvas *c = new TCanvas((histName+"_"+identifier).c_str(), (histName+"_"+identifier).c_str());
  THStack *hs = new THStack("hs","");
  double scale_factor;

  if(leg_title=="Gen MC"){
      arr_hist[0]->SetFillColorAlpha(kGray,0.8);
      arr_hist[0]->SetLineColor(kBlack);
      arr_hist[0]->RebinX();
      hs->Add(arr_hist[0],"hist");
  }
  else
      arr_hist[0]->RebinX();
  
  for(Int_t i = 1; i < arr_hist.size(); i++){
    arr_hist[i]->RebinX();
    cout << "Bin Width Data: " << arr_hist[0]->GetBinWidth(1) << endl;
    cout << "Bin Width MC: " << arr_hist[i]->GetBinWidth(1) << endl;
        
    // scale the histos to area
    if(arr_hist[i]->Integral("width") < arr_hist[0]->Integral("width"))
      scale_factor = arr_hist[0]->Integral("width") / arr_hist[i]->Integral("width");
    else
      scale_factor = arr_hist[i]->Integral("width") / arr_hist[0]->Integral("width");
    
    if(arr_hist[i]->GetMaximum() < arr_hist[0]->GetMaximum())
      arr_hist[i]->Scale(scale_factor );
    else{
        scale_factor = 1/scale_factor ;
        arr_hist[i]->Scale( scale_factor );
    }

    arr_hist[i]->SetLineColor(kBlack);
    arr_hist[i]->SetFillColorAlpha(kAzure-9,0.6);
    //arr_hist[i]->SetFillStyle(4010);
    sprintf(entries[i], "%.0f Events", arr_hist[i]->GetEntries());

    hs->Add(arr_hist[i],"hist");
  }

  if(leg_title=="Data"){
      arr_hist[0]->SetLineColor(kBlack);
      arr_hist[0]->SetMarkerColor(kBlack);
      arr_hist[0]->SetMarkerStyle(20);
      arr_hist[0]->SetMarkerSize(0.9);
      hs->Add(arr_hist[0],"e1");
  }
  
  double ymax = arr_hist[0]->GetMaximum() > arr_hist[1]->GetMaximum()
      ? arr_hist[0]->GetMaximum() : arr_hist[1]->GetMaximum();   
  hs->SetTitle(stack_title.c_str());
  hs->Draw("nostack");
  hs->GetYaxis()->SetMaxDigits(3);
  hs->SetMaximum(ymax*1.1);
  //hs->GetXaxis()->SetLimits(xmin,xmax);
    
  // Draw legend
  TLegend* legend;
  if(leg_pos=="tl")
    legend = new TLegend(0.2,0.75,0.4,0.91); //top left corner
  else
    legend = new TLegend(0.68,0.76,0.88,0.92); //top right corner
    
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  legend->SetTextSize(0.055);
  //legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header

  if(leg_title=="Data")
      legend->AddEntry(arr_hist[0], leg_title.c_str(), "lep");
  else
      legend->AddEntry(arr_hist[0], leg_title.c_str(), "f");
  legend->AddEntry(arr_hist[1], "Recon MC", "f");
  legend->Draw("same");
  
  gPad->Update();
  c->SaveAs(saveName.c_str());
  //c->Close();
}

void setStyle()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(600);
  gStyle->SetCanvasDefW(700);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.05);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);

  gStyle->SetFrameFillStyle ( 0);
  gStyle->SetFrameFillColor ( 0);
  gStyle->SetFrameLineColor ( 1);
  gStyle->SetFrameLineStyle ( 0);
  gStyle->SetFrameLineWidth ( 1);
  gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);

  //gStyle->SetNdivisions(510);

  //gStyle->SetLineWidth(1);
  //gStyle->SetHistLineWidth(1);
  //gStyle->SetFrameLineWidth(2);
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.06,"X");
  gStyle->SetLabelSize(0.06,"Y");

  gStyle->SetLabelOffset(0.008,"X");
  gStyle->SetLabelOffset(0.008,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132,"T");
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");
  
  gStyle->SetTitleSize(0.07,"T");
  gStyle->SetTitleSize(0.08,"X");
  gStyle->SetTitleSize(0.08,"Y");
  
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(1.0,"Y");

  gStyle->SetTextSize(0.08);
  gStyle->SetTextFont(132);

  gStyle->SetOptStat(0);

  gROOT->ForceStyle();

  TLatex* latex = new TLatex();
  latex->SetNDC();
  latex->SetTextFont(132);
  latex->SetTextSize(0.08);
  latex->SetTextAlign(32);
  latex->SetIndiceSize(0.2);
  gROOT->ForceStyle();
}
