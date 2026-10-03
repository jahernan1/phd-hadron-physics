#include "gxana/common/Paths.h"
#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "TSystem.h"

// gxana: AnalysisNote's GetPreppedFlatTree(root_file_name, vec_cuts, n_threads) is renamed to
// flatTreePrep(root_file_name, n_threads) so the file's macro entry point matches its name
// (ROOT `root flatTreePrep.C(args)` convention). vec_cuts was never overridden by the batch
// driver below, so its default value is inlined in the function body.
void flatTreePrep(std::string root_file_name="flatTree_kpkpxim__M23_2017-01_ana45", int n_threads = 8);

//initiate functions
// gxana: AnalysisNote's driver (originally also named flatTreePrep(), with no arguments) is
// renamed flatTreePrepAll to avoid clashing with the per-file entry point above.
int flatTreePrepAll()
{
    /* -------
       Data
       ------- */
    flatTreePrep();
    flatTreePrep("flatTree_kpkpxim__M23_2017-01_ana56");
    flatTreePrep("flatTree_kpkpxim__B4_M23_2018-01_ana03");
    flatTreePrep("flatTree_kpkpxim__B4_M23_2018-08_ana02");
    flatTreePrep("flatTree_kpkpxim__B4_M23_2018-01_ana03_oneRfBunch");
    flatTreePrep("flatTree_kpkpxim__B4_M23_2018-08_ana02_oneRfBunch");

    /* -------------
       Simulation
       ------------- */
    flatTreePrep("flatTree_kpkpxim__M23_2017-01_ana45_gen_amp_V2_ac_YstarRest");
    flatTreePrep("flatTree_kpkpxim__M23_2017-01_ana56_gen_amp_V2_ac_YstarRest");
    flatTreePrep("flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest");
    flatTreePrep("flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest");
    flatTreePrep("flatTree_kpkpxim__B4_M23_2018-01_ana03_gen_amp_V2_ac_YstarRest_oneRfBunch");
    flatTreePrep("flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_ac_YstarRest_oneRfBunch");
    flatTreePrep("flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_nobkg");

    return 0;
}

void flatTreePrep(std::string root_file_name, int n_threads)
{
    std::vector<std::string> vec_cuts = {"chisqndf < 8", "xim_pathlensig > 2"};
    /*
       Parallelize with n threads
    */
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	/*
       Import select braches to speed things up
    */
    // std::vector<std::string> branches = {};
	/*
       make data frame
	   format : tree name, file name, branches to open
    */
    auto df0 = ROOT::RDataFrame("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "Trees/flatTree/rawTrees/")+root_file_name+".root").c_str());
    //auto df = df0.Filter("decayxim_M<1.334 && decayxim_M>1.31","xiMassCut")
    //.Filter("best_combo==1");
    // Dataset specific cut
    // Perform nominal cuts
    auto df1 = df0
        .Define("kphigh_theta","kphigh_p4.Theta()*180/TMath::Pi()")
        .Define("kplow_theta","kplow_p4.Theta()*180/TMath::Pi()")
        // gxana: rapidity/pseudorapidity names fixed as in gx1 PrepFlatTrees.C (docs/KNOWN_ISSUES.md).
        .Define("kphigh_prapidity","atanh(kphigh_p4.Pz()/kphigh_p4.P())")
        .Define("kplow_prapidity","atanh(kplow_p4.Pz()/kplow_p4.P())")
        .Define("kphigh_rapidity","kphigh_p4.Rapidity()")
        .Define("kplow_rapidity","kplow_p4.Rapidity()")
        .Define("ystar_prapidity","atanh(ystar_p4.Pz()/ystar_p4.P())")
        .Define("ystar_rapidity","ystar_p4.Rapidity()")
        .Define("t_dist_truth","-(beam_p4_truth - kphigh_p4 ).M2()")
        .Define("hybrid_combo","best_combo_rf*acc_weight")
        .Define("acc_combo","uniq_weight*acc_weight")
        .Filter("beam_E > 6.4 && beam_E < 11.4")
        .Filter(vec_cuts[0].c_str(), vec_cuts[0].c_str())//chisqcut
        .Filter("abs(total_mm2) < 0.02 ","mm2Cut")
        .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut");   
    //
    /* Display Table of tree values */
    // df1.Display({"evnt_num", "combo_num", "chisqndf", "kp_highp_P3", "kp_lowp_P3", "decayxim_M"}, 50)->Print();
    //
    auto df2 = df1.Filter("xim_pathlensig > 0.", "XimPathLenSigCut>0.")
        .Filter("lambda_pathlensig > 0.", "LambPathLenSigCut>0.");
    auto df3 = df2.Filter(vec_cuts[1].c_str(), vec_cuts[1].c_str());//xim_pathlen cut
    //
    auto df4 = df3.Filter("kp_highp_P3-kp_lowp_P3 > 1.2", "KaonSepMom>1.2");
    //.Filter("kp_lowp_P3 > 0.4", "KaonSlowMom>0.4");
    //
    auto df5 = df3.Filter("t_dist<2.4","TDistCut");
    auto df6 = df5.Filter("kplow_p4.P()>0.4","KpLowPCut");
    auto df7 = df5.Filter("kplow_p4.CosTheta()>0","KpLowThetaCut");
    //
    auto df8 = df3.Filter("kphigh_p4.Rapidity()>2","KpHighRapidityCut");
    auto df9 = df8.Filter("kplow_p4.Rapidity()>0","KpLowRapidityCut");

    /* Print cut analysis */
    cout << root_file_name << endl;
    df8.Report()->Print();//data
    df8.Filter("decayxim_M<1.334 && decayxim_M>1.308")
        .Stats("decayxim_M")->Print();
    
    cout << "Saving flat trees with cuts applied..." << endl;
    // df.Snapshot("flatTree_kpkpxim",
    //                (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_xiMassCut.root").c_str());

    // gxana: legacy /AnalysisNote/flatTrees/ pre-existed on the shared disk; $GXANA_DATA/flatTrees
    // must be created here.
    gSystem->mkdir(gxana::EnvPath("GXANA_DATA","flatTrees").c_str(), true);

    cout << "\t> " << root_file_name + "_nominal.root" << endl;
    df1.Snapshot("flatTree_kpkpxim",
                 (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal.root").c_str());
    //
    cout << "\t> " << root_file_name + "_nominal_ximVertexCut.root" << endl;
    df3.Snapshot("flatTree_kpkpxim",
                 (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_ximVertexCut.root").c_str());
    //
    // cout << "\t> " << root_file_name + "_nominal_allKaonSep.root" << endl;
    // df4.Snapshot("flatTree_kpkpxim",
    //              (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_allKaonSep.root").c_str());
    //
    cout << "\t> " << root_file_name + "_nominal_tCut.root" << endl;
    df5.Snapshot("flatTree_kpkpxim",
                 (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_tCut.root").c_str());
    //
    // cout << "\t> " << root_file_name + "_nominal_momCut.root" << endl;
    // df6.Snapshot("flatTree_kpkpxim",
    //              (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_momCut.root").c_str());
    //   
    // cout << "\t> " << root_file_name + "_nominal_cosThetaCut.root" << endl;
    // df7.Snapshot("flatTree_kpkpxim",
    //            (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_cosThetaCut.root").c_str());
    //
    cout << "\t> " << root_file_name + "_nominal_kphighrap.root" << endl;
    df8.Snapshot("flatTree_kpkpxim",
                 (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_kphighrap.root").c_str());
    
    cout << "\t> " << root_file_name + "_nominal_rapidityCuts.root" << endl;
    df9.Snapshot("flatTree_kpkpxim",
                 (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_rapidityCuts.root").c_str());
  
    cout << endl;
}
