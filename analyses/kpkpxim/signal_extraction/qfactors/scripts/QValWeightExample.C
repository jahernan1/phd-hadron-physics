#include "gxana/common/Paths.h"
void QValWeightExample(string root_file_name="kpkpxim__M23_2017-01_ver56_nominal_1111111", int n_threads = 16) {
	// Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	//setStyle();

	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M",
                                         "qvalue_decayxim_M",
                                         "acc_weight"  
	};
    // make data frame
	// format : tree name, file name, branches to open
	auto df = ROOT::RDataFrame("flatTree_kpkpxim", (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/")+root_file_name+"/postQVal_flatTree_"+root_file_name+".root"), branches);
    // cut out any nan q-value events
    auto df1 = df.Define("qacc_decayxim_M", "acc_weight*qvalue_decayxim_M")
      .Define("qbkg_decayxim_M", "acc_weight*(1-qvalue_decayxim_M)");

    // Print Yield Value
	cout << "Sum of Weighted Q-Values: " << df1.Sum("qacc_decayxim_M").GetValue() << endl;

	// Draw q-value subtracted mass distribution
	auto h = df1.Histo1D({"","; M(#Lambda#pi^{-}) (GeV); Events/ 2 MeV/c^{2}",100,1.27,1.47}, "decayxim_M", "acc_weight");
	auto h1 = df1.Histo1D({"","",100,1.27,1.47},"decayxim_M", "qacc_decayxim_M");
    h1->SetFillColorAlpha(kGray,0.6);
	auto h2 = df1.Histo1D({"","",100,1.27,1.47},"decayxim_M", "qbkg_decayxim_M");
    h2->SetFillColorAlpha(kPink,0.6);
    
    new TCanvas;
    h->DrawClone();
    h1->DrawClone("hist same");
    h2->DrawClone("hist same");
}

