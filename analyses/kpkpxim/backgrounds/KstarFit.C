#include "gxana/common/Paths.h"
// Fit the kstar that is seen in data gamma p -> K+ Y*->K+ (K*Lambda)
#include<RooPlot.h>

void rooFitHist(TH1D* hist, string histTitle);

void KstarFit(int n_threads = 8, string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111") {
	// Parallelize with n threads
        if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M",
                                         "ystar_M",
                                         "acc_weight",
                                         "chisqndf",
                                         "pim1_p4_kin"
                                         "kplow_p4",
                                         "kp_lowp_P3"
                                         "kphigh_p4",
                                         "kphigh_p4",
                                         "p_p4_meas",
                                         "pim2_p4_meas"
	};
	std::vector<std::string> branchesQ = {"decayxim_M",
                                          "ystar_M",
                                          "qvalue_decayxim_M"
                                          "acc_weight",
                                          "chisqndf",
                                          "pim1_p4_kin"
                                          "kplow_p4",
                                          "kp_lowp_P3"
                                          "kphigh_p4",
                                          "kphigh_p4",
                                          "p_p4_meas",
                                          "pim2_p4_meas"
	};

	// make data frame
	// format : tree name, file name, branches to open
	// auto df = ROOT::RDataFrame("kpkpxim_flatTree", (gxana::EnvPath("GXANA_OUTPUT", "legacy_macros/QFactors/logs/")+root_file_name+"/postQVal_flatTree_"+root_file_name+".root").c_str(), branches);
	auto df = ROOT::RDataFrame("kpkpxim_flatTree", gxana::EnvPath("GXANA_DATA", "flatTrees/flatTree_kpkpxim__M23_2017-01_ver56_allCuts.root").c_str(), branches);//Trees/flatTree/flatTree_kpkpxim_2017-01_ana-45.root
    auto df0 = ROOT::RDataFrame("kpkpxim_flatTree", gxana::EnvPath("GXANA_OUTPUT", "legacy_macros/QFactors/logs/kpkpxim_2017-01__M23_ana56_allCut_111111/postQVal_flatTree_kpkpxim_2017-01__M23_ana56_allCut_111111.root").c_str(), branchesQ);//Trees/flatTree/flatTree_kpkpxim_2017-01_ana-45.root
    // Define kstar variables
    auto df1 = df.Define("kstar_p4", "(pim1_p4_kin+kplow_p4)")
      .Define("kstar_M", "kstar_p4.M()")
      .Define("kstar_P","kstar_p4.Vect().Mag()")
      .Define("lambda_p4_meas", "pim2_p4_meas+p_p4_meas")
      .Define("kstar_lambda_M", "(kstar_p4+lambda_p4_meas).M()");
    
    auto df2 = df1.Filter("kstar_M < 1 && kstar_M > 0.8");
    auto df3 = df1.Filter("kstar_M > 1 || kstar_M < 0.8");

    auto nanCut = [](float f) {return !isnan(f);};
    auto df01 = df0.Filter(nanCut, {"qvalue_decayxim_M"})
      .Filter("beam_E > 6.4 && beam_E < 11.4")
      .Define("qacc_decayxim_M", "acc_weight*qvalue_decayxim_M")
      .Define("qbkg_decayxim_M", "acc_weight*(1-qvalue_decayxim_M)")
      .Define("kstar_p4", "(pim1_p4_kin+kplow_p4)")
      .Define("kstar_M", "kstar_p4.M()");

    // Draw accsub subtracted mass distribution
    auto h = df1.Histo1D("kstar_M", "acc_weight");
    auto h0 = df01.Histo1D("kstar_M", "qacc_decayxim_M");
    
    auto h1 = df1.Histo1D("kstar_P","acc_weight");
    auto h2 = df1.Histo2D({""," ; M(K^{+}#Xi^{-}) (GeV/c^{2}); M(K^{*}#Lambda) (GeV/c^{2})", 90, 1.8, 3.6, 90, 1.8, 3.6}, "ystar_M", "kstar_lambda_M", "acc_weight");

    auto h3 = df2.Histo1D("ystar_M", "acc_weight");
    auto h4 = df3.Histo1D("ystar_M", "acc_weight");

    auto h5 = df2.Histo1D("kp_lowp_P3", "acc_weight");
    auto h6 = df3.Histo1D("kp_lowp_P3", "acc_weight");

    auto h7 = df2.Histo1D("decayxim_M", "acc_weight");
    auto h8 = df3.Histo1D("decayxim_M", "acc_weight");

    new TCanvas;
    h7->SetTitle("kstar signal cut");
	h7->DrawClone("hist");
    new TCanvas;
    h8->DrawClone("hist");
    new TCanvas;
    h2->DrawClone("colz");
    //ystar with kstar cut
    new TCanvas;
    h3->RebinX();
    h3->SetTitle("kstar signal cut");
    h3->DrawClone("hist");
    new TCanvas;
    h4->RebinX();
    h4->DrawClone("hist");
    // kslow p3 with kstar cut
    new TCanvas;
    h5->RebinX();
    h5->SetTitle("kstar signal cut");
    h5->DrawClone("hist");
    new TCanvas;
    h6->RebinX();
    h6->DrawClone("hist");
    
    TCanvas *c = new TCanvas("c","c");
    c->Divide(2,1);
    c->cd(1);
    h->DrawClone("hist");
    c->cd(2);
    h0->DrawClone("hist");

    // Fit the kstar mass
    new TCanvas;
    h->RebinX();
    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
    rooFitHist(h.GetPtr(), "");
	//df2.Snapshot("kpkpxim_flatTree", (gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_postQVal")+root_file_name+".root").c_str());
}

void rooFitHist(TH1D* hist, string histTitle)
{
  //setStyle();
  Double_t min_mass = hist->GetXaxis()->GetBinLowEdge(hist->FindFirstBinAbove());
  //if(min_mass < 1.26) min_mass = 1.26;

  RooWorkspace* w = new RooWorkspace("ws1");
  RooRealVar mass("mass", "M(K^{+}_{slow}#pi^{-}) (GeV/c^{2})", min_mass, 1.8);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  RooPlot* massframe = mass.frame(RooFit::Title(histTitle.c_str()));
  w->import(RooArgSet(mass));

  if(!data)
    cout << "Null data is the problem" << endl;

  w->factory("Voigtian::signal(mass,mean[0.89555, 0.89535, 0.89575], width[0.0473, 0.0468, 0.0481], sigma[0])");
  //w->factory("Gaussian::signal(mass,mean[0.89555, 0.89535, 0.89575], sigma[0.05, 0.01,0.09])");
  w->factory("Chebychev::bkgd(mass,{a0[-0.8,-5,-0.1], a1[-0.4, -5,-0.1], a2[0.5, 0.1,5]})");
  //w->factory("ArgusBG::bkgd(mass,mo[1.9, 1.8, 2.0], c[0.5, 0, 2.0], p[2])");
  
  //Create model and fit to data
  w->factory("SUM::model( nkstar[200,1,1e6]*signal, nbkgd[200,1,1e6]*bkgd)");
  
  w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::SumW2Error(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));//,RooFit::Range(0.75,1.8)
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.55, 0.9, 0.9));
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(3));
  w->pdf("signal")->plotOn(massframe,RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nkstar")->getVal(), RooAbsReal::NumEvent));
  w->pdf("bkgd")->plotOn(massframe, RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent));
  //RooFit::DrawOption("F"), RooFit::FillColor(kAzure-1),RooFit::FillStyle(4010),RooFit::MoveToBack()

  massframe->Draw();
}
