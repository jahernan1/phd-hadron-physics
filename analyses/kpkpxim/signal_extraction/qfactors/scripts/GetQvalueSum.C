#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
#include "gxana/fit/Fit.h"
#include "gxana/fit/Model.h"
//#include <RooAbsDataHelper.h>

void setStyle();
void rooFitHist(TH1* hist,char* histTitle, char* ws_name, double* yield, double* yield_err);

void GetQvalueSum(string root_file_name="kpkpxim__M23_2017-01_ver56_nominal_all_1111111", int n_threads = 4) {
	// Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	//setStyle();

	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M",
                                         "qvalue_decayxim_M",
                                         "acc_weight",
                                         "t_dist",
                                         "beam_E",
                                         "ystar_P3",
                                         "ystar_M",
                                         "chisqndf",
                                         "kp_highp_P3",
                                         "kp_lowp_P3",
                                         "pim1_p3",
                                         "pim2_P3",
                                         "proton_P3"
                                         
	};
    std::vector<std::string> branchesMC = {"decayxim_M",
                                           "acc_weight",
                                           "t_dist",
                                           "beam_E",
                                           "ystar_P3",
                                           "ystar_M",
                                           "chisqndf",
                                           "kp_highp_P3",
                                           "kp_lowp_P3",
                                           "pim1_p3",
                                           "pim2_P3",
                                           "proton_P3",
                                           "xim_pathlensig"
                                         
	};
	// make data frame
	// format : tree name, file name, branches to open
	auto df = ROOT::RDataFrame("kpkpxim_flatTree", (gxana::EnvPath("GXANA_OUTPUT", "legacy_macros/QFactors/logs/")+root_file_name+"/postQVal_flatTree_"+root_file_name+".root"), branches);
    auto df0 = ROOT::RDataFrame("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_kpkpxim__M23_2017-01_ver45_Ystar2400_t1_genr8.root").c_str()), branchesMC);
	// cut out any nan q-value events
	auto nanCut = [](float f) {return !isnan(f);};
	auto df1 = df.Filter(nanCut, {"qvalue_decayxim_M"})
      //.Filter("beam_E > 6.4 && beam_E < 11.4")
      .Define("qacc_decayxim_M", "acc_weight*qvalue_decayxim_M")
      .Define("qbkg_decayxim_M", "acc_weight*(1-qvalue_decayxim_M)");
      //.Filter("decayxim_M > 1.308 && decayxim_M < 1.338");

    auto df01 = df0.Filter("beam_E > 6.4 && beam_E < 11.4")
                   .Filter("chisqndf < 4.0")
      .Filter("abs(total_mm2)<2.0")
      //      .Filter("xim_pathlensig>2.0")
                   .Filter("decayxim_M > 1.308 && decayxim_M < 1.338");
	
    // Print Yield Value
	cout << "Sum of Weighted Q-Values: " << df1.Sum("qacc_decayxim_M").GetValue() << endl;

	// Draw q-value subtracted mass distribution
	auto h = df1.Histo1D({"","; M(#Lambda#pi^{-}) (GeV); Events/ 2 MeV/c^{2}",100,1.27,1.47},"decayxim_M", "acc_weight");
	auto h1 = df1.Histo1D({"","",125,1.27,1.27},"decayxim_M", "qacc_decayxim_M");
	auto h2 = df1.Histo1D({"","",125,1.27,1.27},"decayxim_M", "qbkg_decayxim_M");
	// auto h22 = df3.Histo1D({"","",125,1.25,1.5},"decayxim_M", "qbkg_decayxim_M");
    // //    auto h3 = df3.Histo2D({"","",120,6,12,100,0,5},"beam_E", "t_dist","qacc_decayxim_M");
    // auto h3 = df3.Histo2D({"","; E_{#gamma} (GeV); #chi^{2}/ndf",60,6,12,80,0,4},"beam_E", "chisqndf","qacc_decayxim_M");
    
    //auto h1 = df1.Histo1D({"",";#vec{#bf{p}}(K^{+}_{Prod}) (GeV/c); Arbitrary Units",100,0,10}, "kp_highp_P3","qacc_decayxim_M");
    //auto h10 = df01.Histo1D({"",";#vec{#bf{p}}(K^{+}_{Prod}); Arbitrary Units",100,0,10}, "kp_highp_P3","acc_weight");
    //auto h1 = df1.Histo1D({"",";#vec{#bf{p}}(K^{+}_{Y^{*}}) (GeV/c); Arbitrary Units",100,0,10}, "kp_lowp_P3","qacc_decayxim_M");
    //auto h10 = df01.Histo1D({"",";#vec{#bf{p}}(K^{+}_{Prod}); Arbitrary Units",100,0,10}, "kp_lowp_P3","acc_weight");
    
    setStyle();
    // h1->Scale(h10->GetMaximum()/h1->GetMaximum());
    // h1->SetFillColorAlpha(kGray,0.8);
    // h1->SetLineColor(kGray);
    // h10->SetMarkerStyle(20);
    // h10->SetMarkerColor(kBlue);
    // h10->SetLineColor(kBlue);
    // gStyle->SetOptStat(0);
    // // h6->DrawClone("e1");
    // TCanvas *c = new TCanvas("c","c");	
    // c->SetGrid();
    // h1->DrawClone("hist");
    // h10->DrawClone("e1 same");

    // auto *legend = new TLegend(0.65,0.68,0.949,0.92);
    // legend->SetBorderSize(2);
    // legend->SetTextSize(0.045);
    // legend->SetTextFont(132);
    // legend->SetFillColor(18);
    // // legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
    // legend->AddEntry(h1.GetPtr(),"Data","lep");
    // legend->AddEntry(h10.GetPtr(),"Simulation","lep");
    // legend->DrawClone();
     
    //Get yield as string
	double fit_yield, fit_yield_err; 
	std::string yield  = to_string(df1.Sum("qacc_decayxim_M").GetValue());
	
	yield.erase(yield.find_first_of("."), yield.size());
	
    new TCanvas;
	rooFitHist(h.GetPtr(),"","",&fit_yield, &fit_yield_err);
    TCanvas *c = new TCanvas("c","c");	
    h->DrawClone("hist");
	//h2->SetLineColorAlpha(kOrange,0);
	h2->SetFillColorAlpha(kOrange+6,0.7);
	//h2->DrawClone("hist same");
	h1->SetFillStyle(1001);
	h1->SetFillColorAlpha(kAzure-4,0.7);
	//h1->DrawClone("hist same");
	TLatex latex;
	latex.DrawLatex(1.36,250, "Spring 2017" );
	latex.DrawLatex(1.35,150, ("#Xi^{-} Events: "+yield).c_str());
	c->SaveAs("decayxim_M_201701.png");
	//
	// latex.DrawLatex(1.36,750, "Spring 2018" );
	// latex.DrawLatex(1.35,600, ("#Xi^{-} Events: "+yield).c_str());
	// c->SaveAs("decayxim_M_201801.png");
	// latex.DrawLatex(1.36,950, "Fall 2018" );
	// latex.DrawLatex(1.35,800, ("#Xi^{-} Events: "+yield).c_str());
	// c->SaveAs("decayxim_M_201808.png");
	
	//latex.DrawLatex(1.35,110, "8.4 < E_{#gamma} < 8.9");
	//latex.DrawLatex(1.35,80, ("#Xi^{-} Events: "+yield).c_str());
	//c->SaveAs(("decayxim_M_"+root_file_name+".png").c_str());
	
	//df2.Snapshot("kpkpxim_flatTree", gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_postQVal")+root_file_name+".root");
}

void rooFitHist(TH1* hist,char* histTitle, char* ws_name, double* yield, double* yield_err)
{
  Double_t min_mass = gxana::fit::FirstPopulatedEdge(*hist, 0, 1, 1.3);
  if(min_mass < 1.28) min_mass = 1.28;

  RooWorkspace* w = new RooWorkspace(ws_name);
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", min_mass, 1.45);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  //XiMassKinFit_Ebin_accsub->Print();
  w->import(RooArgSet(mass)); 
  
  gxana::fit::BuildModel(*w, {
      gxana::fit::Chebychev("bkgd", "mass", {{"a0", "0.8,0.1,1.2"}}),//,a1[-0.1,-0.5,-0.05]
      //w->factory("CBShape::xigaus(mass,xi_mean[1.322],xi_sig[0.005,0.003,0.01], alpha[5,1,20], n[5,0,10])");
      gxana::fit::Johnson("xigaus", "mass", {"mu", "1.322,1.31, 1.33"}, {"lambda", "0.006, 0.005, 0.01"}, {"gamma", "0"},
                          {"delta", "1.5, 1, 2"}),
      //w->factory("Gaussian::xigaus(mass,xi_mean[1.322],xi_sig[0.005, 0.001, 0.01])");
      //w->factory("Voigtian::sigma(mass,mean[1.387],sig[0.006, 0.005,0.01], width[0.0394, 0.0373, 0.0415])");
      //w->factory("Gaussian::siggaus(mass,sigma_mean[1.385],sigma_sig[0.016])");
      gxana::fit::Sum("model", {{{"nbkgd", "500,0,1e6"}, "bkgd"}, {{"nxi", "100,1,1e6"}, "xigaus"}})});

  RooPlot* massframe = mass.frame(RooFit::Title(histTitle));
  gxana::fit::RunFit(*w->pdf("model"), *data, RooFit::Extended(kTRUE), RooFit::PrintLevel(-1), RooFit::PrintEvalErrors(-1),
                     RooFit::Verbose(false), RooFit::Warnings(false));
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.5, 0.95, 0.92) );
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(2) );
  w->pdf("xigaus")->plotOn(massframe,RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(4010),RooFit::MoveToBack(), RooFit::Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent) );
  //w->pdf("sigma")->plotOn(massframe, RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(4010),RooFit::MoveToBack() , RooFit::Normalization(w->var("nsigma")->getVal(), RooAbsReal::NumEvent));
  w->pdf("bkgd")->plotOn(massframe, RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent) );
  
  *yield = w->var("nxi")->getVal();
  *yield_err = w->var("nxi")->getError();
  
  massframe->Draw();
}

void setStyle()
{
    gxana::ApplyStyle(gxana::CutStudyStyle());
}
