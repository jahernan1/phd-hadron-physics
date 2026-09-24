void PlotfromFlatTreeMC(int n_threads = 4, string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111") {
	// Parallelize with n threads
        if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M",
					     "xim_pathlen",
					     "xim_pathlensig",
					     "xim_lifetime",
					     "xim_lifetime_restframe",
					     "lambda_lifetime",
					     "lambda_lifetime_restframe",
					     "acc_weight",
					     "kp1_P3",
					     "kp_highp_P3",
					     "kp2_P3",
					     "kp_lowp_P3",
					     "beam_E"
					     
	};

	// make data frame
	// format : tree name, file name, branches to open
	// auto df = ROOT::RDataFrame("kpkpxim_flatTree", ("/d/grid17/hjesse/macros/QFactors/logs/"+root_file_name+"/postQVal_flatTree_"+root_file_name+".root").c_str(), branches);
	auto df = ROOT::RDataFrame("kpkpxim_flatTree", "/d/grid17/hjesse/Trees/flatTree/flatTree_kpkpxim_2017-01_ana-45_Ystar2400_genr8.root", branches);
	// keep events only in-time region
	auto accCut = [](double f, double e) {return f == 1. && e >= 6.4;};
	auto df2 = df.Filter(accCut, {"acc_weight","beam_E"});
	auto df3 = df2.Filter("kp_highp_P3 >= 3 && kp_lowp_P3 < 3");
	auto df4 = df2.Filter("xim_pathlensig >= 3");
	auto df5 = df2.Filter("decayxim_M >= 1.308 && decayxim_M <= 1.334");
	
	// Draw q-value subtracted mass distribution
	auto h = df2.Histo2D({"","",250,1.25,1.5,500,0,500},"decayxim_M", "xim_pathlensig");
	auto h1 = df2.Histo2D({"","",250,1.25,1.5,400,0,40},"decayxim_M", "xim_lifetime");
	auto h2 = df4.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "xim_lifetime_restframe");
	auto h22 = df2.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "kp_highp_P3");
	auto h3 = df3.Histo1D({"","",500,0,10},"kp_highp_P3");
	auto h4 = df3.Histo1D({"","",500,0,10},"kp_lowp_P3");
	auto h5 = df4.Histo1D({"","",500,0,10},"kp_highp_P3");
	auto h6 = df5.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "kp_highp_P3");
	auto h7 = df5.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "kp_lowp_P3");

	//h->GetXaxis()->SetRangeUser(1.26,1.40);
	h->SetTitle("; M(#Lambda#pi^{-}) (GeV); Pathlength Significance");
	h1->SetTitle("; M(#Lambda#pi^{-}) (GeV); #Xi^{-} Lifetime (ns)");
	h2->SetTitle("; M(#Lambda#pi^{-}) (GeV); #Xi^{-} Lifetime Restframe (ns)");
	h22->SetTitle("; M(#Lambda#pi^{-}) (GeV); #vec{#bf{p}}(K^{+}_{highp}) (GeV/c)");
	h7->SetTitle("; M(#Lambda#pi^{-}) (GeV); #vec{#bf{p}}(K^{+}_{lowp}) (GeV/c)");
	h6->SetTitle("; M(#Lambda#pi^{-}) (GeV); #vec{#bf{p}}(K^{+}_{highp}) (GeV/c)");
	
        TLine* hl = new TLine(1.25,3,1.5,3);
	hl->SetLineColor(2); hl->SetLineWidth(3);
	// TArrow* ar = new TArrow(1.35, 3, 1.35, 1.5, 0.05, "|>");
	// ar->SetLineColor(2); ar->SetFillColor(2); ar->SetLineWidth(3);
	TArrow* ar = new TArrow(1.35, 3, 1.35, 4.5, 0.05, "|>");
	ar->SetLineColor(2); ar->SetFillColor(2); ar->SetLineWidth(3);

	h2->DrawClone("colz");
	//hl->Draw("same");ar->Draw("");
	
	// h2->SetFillColorAlpha(kMagenta,0.1);
	// h2->DrawClone("hist same");
	// h1->SetLineColor(kRed);
	// h1->DrawClone("hist same");

	//df2.Snapshot("kpkpxim_flatTree", ("/d/grid17/hjesse/Trees/flatTree/flatTree_postQVal"+root_file_name+".root").c_str());
}
