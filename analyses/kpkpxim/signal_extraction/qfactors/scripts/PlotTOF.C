#include "gxana/common/Paths.h"
void PlotTOF(int n_threads = 4, string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111") {
	// Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M",
					     //"qvalue_decayxim_M",
					     "acc_weight",
					     "xim_lifetime_restframe",
					     "xim_lifetime",
					     "lambda_lifetime_restframe"

	};

	// make data frame
	// format : tree name, file name, branches to open
	//auto df = ROOT::RDataFrame("kpkpxim_flatTree", (gxana::EnvPath("GXANA_OUTPUT", "legacy_macros/QFactors/logs/")+root_file_name+"/postQVal_flatTree_"+root_file_name+".root").c_str(), branches);
	auto df = ROOT::RDataFrame("kpkpxim_flatTree", gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_kpkpxim_2017-01_ana-45_Ystar2400_genr8.root").c_str(), branches);
	// make new data frame with cuts
	// auto nanCut = [](float f) {return !isnan(f);};
	// auto df2 = df.Filter(nanCut, {"qvalue_decayxim_M"})
	//              .Define("qacc_decayxim_M", "acc_weight*qvalue_decayxim_M");
	
	// Xim Mass cut
	auto xiMassCut = [](double f) {return f>1.31 && f<1.334;};
	auto df21 = df.Filter(xiMassCut, {"decayxim_M"});
	              
	// Define bkg q-values
	//auto df3 = df2.Define("qbkg_decayxim_M","(1-qvalue_decayxim_M)");
	// Draw q-value subtracted mass distribution
	auto h = df21.Histo1D({"","",100,0,5},"xim_lifetime");
	auto h1 = df21.Histo1D({"","",100,0,5},"xim_lifetime","acc_weight");
	//h1->Scale(h->GetMaximum()/h1->GetMaximum());
	h1->SetLineColor(kMagenta);
	auto h2 = df.Histo1D({"","",100,1.25,1.45},"decayxim_M");
	auto h3 = df21.Histo1D({"","",100,1.25,1.45},"decayxim_M");
	//auto h1 = df3.Histo1D({"","",100,0,5},"xim_lifetime_restframe", "qacc_decayxim_M");// Filter("qacc_decayxim_M > 0")
	//auto h2 = df3.Histo1D({"","",100,0,5},"xim_lifetime_restframe", "qbkg_decayxim_M");
	
	//h->GetXaxis()->SetRangeUser(1.26,1.40);
	//h->SetTitle("; M(#Lambda#pi^{-}) (GeV); Events");
	h->DrawClone("hist");
	//h2->SetFillColorAlpha(kMagenta,0.1);
	h1->DrawClone("hist same");
	//h1->SetLineColor(kRed);
	//h1->DrawClone("hist same");

	//df2.Snapshot("kpkpxim_flatTree", (gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_postQVal")+root_file_name+".root").c_str());
}
