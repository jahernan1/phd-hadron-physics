void PlotfromFlatTree(int n_threads = 4, string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111") {
	// Parallelize with n threads
        if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M",
                                         "xim_pathlen",
                                         "xim_pathlensig",
                                         "xim_vertexZ",
                                         "xim_lifetime_restframe",
                                         "lambda_pathlen",
                                         "lambda_pathlensig",
                                         "lambda_vertexZ",
                                         "lambda_lifetime_restframe",
                                         "acc_weight",
                                         "chisqndf",
                                         "confidencelvl",
                                         "kp1_P3",
                                         "kp_highp_P3",
                                         "kp2_P3",
                                         "kp_lowp_P3",
                                         "beam_E",
                                         "pim1_p4_kin"
                                         "kplow_p4"
                                      
					     
	};

	// make data frame
	// format : tree name, file name, branches to open
	// auto df = ROOT::RDataFrame("kpkpxim_flatTree", ("/d/grid17/hjesse/macros/QFactors/logs/"+root_file_name+"/postQVal_flatTree_"+root_file_name+".root").c_str(), branches);
	auto df = ROOT::RDataFrame("flatTree_kpkpxim", "/d/grid17/hjesse/AnalysisNote/QFactors/logs/kpkpxim__B4_M23_2018-08_ana02_nominal_momCut_1111111/postQVal_flatTree_kpkpxim__B4_M23_2018-08_ana02_nominal_momCut_1111111.root", branches)
      .Define("hybrid_qvalue","hybrid_combo*qvalue_decayxim_M")
      .Define("kplow_costheta","kplow_p4.CosTheta()");
    auto df1 = ROOT::RDataFrame("flatTree_kpkpxim", "/d/grid17/hjesse/AnalysisNote/flatTrees/flatTree_kpkpxim__B4_M23_2018-08_ana02_gen_amp_V2_2D_ac_nominal_momCut.root", branches)
      .Define("kplow_costheta","kplow_p4.CosTheta()");
	// keep events only in-time region
	//auto accCut = [](double f, double e) {return f == 1. && e >= 6.4;};
    // auto df = df1.Define("kstar_M", "(pim1_p4_kin+kplow_p4).M()");
   
 	// Draw q-value subtracted mass distribution
    auto h = df.Histo1D("kp_lowp_P3", "hybrid_qvalue");
    auto h1 = df1.Histo1D("kp_lowp_P3", "hybrid_combo");
    h1->SetMarkerStyle(20);
    h1->SetMarkerColor(kAzure);
    
    h1->Scale(h->Integral("width")/h1->Integral("width"));
	h->DrawClone("hist");
    h1->DrawClone("same e1");

    auto h2 = df.Histo1D("kplow_costheta", "hybrid_qvalue");
    auto h3 = df1.Histo1D("kplow_costheta", "hybrid_combo");
    h3->SetMarkerStyle(20);
    h3->SetMarkerColor(kAzure);
    
    h3->Scale(h2->Integral("width")/h3->Integral("width"));
    new TCanvas;
    h2->DrawClone("hist");
    h3->DrawClone("same e1");
	
	//df2.Snapshot("kpkpxim_flatTree", ("/d/grid17/hjesse/Trees/flatTree/flatTree_postQVal"+root_file_name+".root").c_str());
}
