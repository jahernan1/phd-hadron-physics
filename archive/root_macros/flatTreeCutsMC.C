#include "gxana/common/Paths.h"
#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

void rooFitHist(TH1 &hist, double* yield, double* yield_err, std::string canName="c");

void flatTreeCutsMC(int n_threads = 4, string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111") {
	// Parallelize with n threads
        if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M",
                                         "xim_vertexZ",
                                         "lambda_vertexZ",
                                         "xim_pathlensig",
                                         "lambda_pathlensig",
                                         "acc_weight",
                                         "chisqndf",
                                         "confidencelvl",
                                         "total_mm2",
                                         "kp1_P3",
                                         "kp_highp_P3",
                                         "kp_highp_CosTheta",
                                         "kp2_P3",
                                         "kp_lowp_P3",
                                         "kp_lowp_Ystar_CosTheta",
                                         "beam_E",
                                         "t_dist"
	};

	// make data frame
	// format : tree name, file name, branches to open
	// auto df = ROOT::RDataFrame("kpkpxim_flatTree", (gxana::EnvPath("GXANA_OUTPUT", "legacy_macros/QFactors/logs/")+root_file_name+"/postQVal_flatTree_"+root_file_name+".root").c_str(), branches);
	auto df = ROOT::RDataFrame("flatTree_kpkpxim", gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_kpkpxim__M23_2017-01_ver45_Ystar2400_t001_genr8.root").c_str(), branches);
    auto df0 = ROOT::RDataFrame("flatTree_kpkpxim", gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_kpkpxim__M23_2017-01_ver45_Ystar2400_t05_genr8.root").c_str(), branches);
    auto df00 = ROOT::RDataFrame("flatTree_kpkpxim", gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_kpkpxim__M23_2017-01_ver45_Ystar2400_t1_genr8.root").c_str(), branches);
    auto df000 = ROOT::RDataFrame("flatTree_kpkpxim", gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_kpkpxim__M23_2017-01_ver45_ystar2400_genr8.root").c_str(), branches);//t=1.4
    
    //auto accCut = [](double f, double e) {return f == 1. && e >= 6.4;};
	auto df1 = df.Filter("beam_E > 6.4 && beam_E < 11.4");
    auto df01 = df0.Filter("beam_E > 6.4 && beam_E < 11.4");
    auto df001 = df00.Filter("beam_E > 6.4 && beam_E < 11.4");
    auto df0001 = df000.Filter("beam_E > 6.4 && beam_E < 11.4");
    auto cnt1 = df1.Count();
    auto df2 = df1.Filter("chisqndf < 4.0");
    auto df02 = df01.Filter("chisqndf < 4.0");
    auto df002 = df001.Filter("chisqndf < 4.0");
    auto df0002 = df0001.Filter("chisqndf < 4.0");
    auto cnt2 = df2.Count();
    auto df3 = df2.Filter("total_mm2 < 0.02 && total_mm2 > -0.02");
    auto cnt3 = df3.Count();
	auto df4 = df2.Filter("kp_highp_P3 >= 3 && kp_lowp_P3 < 3");
    auto cnt4 = df4.Count();
    auto df5 = df2.Filter("xim_vertexZ < lambda_vertexZ");
    auto cnt5 = df5.Count();
    auto df6 = df2.Filter("xim_pathlensig > 2.0");
    auto df06 = df02.Filter("xim_pathlensig > 2.0");
    auto df006 = df002.Filter("xim_pathlensig > 2.0");
    auto df0006 = df0002.Filter("xim_pathlensig > 2.0");
    auto cnt6 = df6.Count();
    auto df7 = df2.Filter("decayxim_M > 1.308 && decayxim_M < 1.338");
    auto df71 = df6.Filter("decayxim_M > 1.308 && decayxim_M < 1.338");
    auto df07 = df02.Filter("decayxim_M > 1.308 && decayxim_M < 1.338");
    auto df071 = df06.Filter("decayxim_M > 1.308 && decayxim_M < 1.338");
    auto df0071 = df006.Filter("decayxim_M > 1.308 && decayxim_M < 1.338");
    auto df00071 = df006.Filter("decayxim_M > 1.308 && decayxim_M < 1.338");
    
    // Draw q-value subtracted mass distribution
	auto h = df2.Histo2D({"","; M(#Lambda#pi^{-}) (GeV); Pathlength Significance",500,1.25,1.5,200,0,50},"decayxim_M", "xim_pathlensig", "acc_weight");
	auto h1 = df2.Histo2D({"","; M(#Lambda#pi^{-}) (GeV); #vec{#bf{p}}(K^{+}_{1}) (GeV/c)",250,1.25,1.5,100,0,10},"decayxim_M", "kp2_P3");
	//auto h2 = df2.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "kp_lowp_P3");
	//auto h22 = df2.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "kp_highp_P3");
	
	auto h2 = df2.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
	auto h3 = df3.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    auto h4 = df4.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    auto h5 = df5.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    auto h6 = df6.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    
    auto h7 = df2.Histo1D({"","ChiSqNdf > 4; Z_{#Xi} (cm); Events", 90, 20, 200},"xim_vertexZ", "acc_weight");
    auto h71 = df6.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; Z_{#Xi} (cm); Events", 90, 20, 200},"xim_vertexZ", "acc_weight");
    
    auto h8 = df7.Histo1D({"","ChiSqNdf > 4; cos#theta_{K^{+}_{Prod}}; Events", 50, 0.5, 1},"kp_highp_CosTheta", "acc_weight");
    auto h81 = df71.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; cos#theta_{K^{+}_{Prod}}; Events", 50, 0.5, 1},"kp_highp_CosTheta", "acc_weight");
    auto h08 = df71.Histo1D({"","ChiSqNdf > 4; cos#theta_{K^{+}_{Prod}}; Events", 50, 0.5, 1},"kp_highp_CosTheta", "acc_weight");
    auto h081 = df071.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; cos#theta_{K^{+}_{Prod}}; Events", 50, 0.5, 1},"kp_highp_CosTheta", "acc_weight"); 
    auto h0081 = df0071.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; cos#theta_{K^{+}_{Prod}}; Events", 50, 0.5, 1},"kp_highp_CosTheta", "acc_weight"); 

    auto h91 = df71.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #overrightarrow{p}({K^{+}_{Prod}}) (GeV/c); Events", 100, 0.0, 10},"kp_highp_P3", "acc_weight");
    auto h091 = df071.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #overrightarrow{p}({K^{+}_{Prod}}) (GeV/c); Events", 100, 0.0, 10},"kp_highp_P3", "acc_weight");
    auto h0091 = df0071.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #overrightarrow{p}({K^{+}_{Prod}}) (GeV/c); Events", 100, 0.0, 10},"kp_highp_P3", "acc_weight");
    auto h00091 = df00071.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #overrightarrow{p}({K^{+}_{Prod}}) (GeV/c); Events", 100, 0.0, 10},"kp_highp_P3", "acc_weight");

    auto h10 = df71.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #overrightarrow{p}({K^{+}_{Prod}}) (GeV/c); Events", 50, 0.0, 5},"kp_lowp_P3", "acc_weight");
    auto h010 = df071.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #overrightarrow{p}({K^{+}_{Prod}}) (GeV/c); Events", 50, 0.0, 5},"kp_lowp_P3", "acc_weight");
    auto h0010 = df0071.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #overrightarrow{p}({K^{+}_{Prod}}) (GeV/c); Events", 50, 0.0, 5},"kp_lowp_P3", "acc_weight");
    auto h00010 = df00071.Histo1D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #overrightarrow{p}({K^{+}_{Prod}}) (GeV/c); Events", 50, 0.0, 5},"kp_lowp_P3", "acc_weight");

    // auto h8 = df7.Histo2D({"","ChiSqNdf > 4; #vec{p}(K^{+}_{Prod}) (GeV/c); #cos#theta_{K^{+}_{Prod}}", 100, 0, 10, 50, 0.5, 1},"kp_highp_P3", "kp_highp_CosTheta", "acc_weight");
    // auto h81 = df71.Histo2D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #vec{p}(K^{+}_{Prod}) (GeV/c); cos#theta_{K^{+}_{Prod}}", 100, 0, 10, 50, 0.5, 1},"kp_highp_P3", "kp_highp_CosTheta", "acc_weight");
    //auto h81 = df71.Histo2D({"","ChiSqNdf > 4 and PathLenSig < 2.0; #vec{p}(K^{+}_{#Xi}) (GeV/c); Events", 100, 0, 10},"kp_highp_P3", "acc_weight");

    //auto h9 = df2.Histo2D({"","; Chisq/Ndf; M(#Lambda#pi^{-}) (GeV)",350, 0, 4, 125,1.25,1.5},"chisqndf", "decayxim_M");//, "acc_weight");
	//auto h5 = df4.Histo1D({"","",500,0,10},"kp_highp_P3");
	//auto h6 = df5.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "kp_highp_P3");
	//auto h7 = df5.Histo2D({"","",250,1.25,1.5,100,0,10},"decayxim_M", "kp_lowp_P3");
    //auto h10 = df2.Histo2D({"","; Confidence Lvl; M(#Lambda#pi^{-}) (GeV)", 5000, 0, 1, 125,1.25,1.5},"confidencelvl", "decayxim_M", "acc_weight");
    //h->DrawClone("colz");
    // h81->Scale(h081->GetEntries()/h81->GetEntries());
    // h081->Scale(h081->GetEntries()/h81->GetEntries());
    // h81->SetLineColor(kRed);
    // h81->DrawClone("e1");
    // h081->DrawClone("e1 same");    
    //h71->SetLineColor(kRed);
    //h71->DrawClone("colz same");
    
    gStyle->SetMarkerStyle(20);
    
    
    new TCanvas();
    h91->Scale(h91->GetEntries()/h0091->GetEntries());
    h091->Scale(h091->GetEntries()/h0091->GetEntries());
    h00091->Scale(h00091->GetEntries()/h0091->GetEntries());
    h91->SetLineColor(kRed);
    h091->SetLineColor(kBlue);
    h0091->SetLineColor(kBlack);
    h00091->SetLineColor(kGreen);

    h0091->DrawClone("e1");
    h91->DrawClone("e1 same");
    h091->DrawClone("e1 same");    
    h00091->DrawClone("e1 same");

    new TCanvas();
    h10->Scale(h10->GetEntries()/h0010->GetEntries());
    h010->Scale(h010->GetEntries()/h0010->GetEntries());
    h00010->Scale(h00010->GetEntries()/h0010->GetEntries());
    h10->SetLineColor(kRed);
    h010->SetLineColor(kBlue);
    h0010->SetLineColor(kBlack);
    h00010->SetLineColor(kGreen);
    h0010->DrawClone("e1");
    h10->DrawClone("e1 same");
    h010->DrawClone("e1 same");    
    h00010->DrawClone("e1 same");

    // Plot and fit the invariant mass of cascade
    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;
    Double_t yield;
    Double_t yield_err;
    
    cout << "beam_E: " << *cnt1 << endl;
    
    rooFitHist(*h2, &yield, &yield_err, "ChiSqNdf Cut");
    cout << "chisqndf: " << *cnt2 << " Signal: " << yield << endl;
    
    rooFitHist(*h3, &yield, &yield_err, "MM2 Cut");
    cout << "total_mm2: " << *cnt3  << " Signal: " << yield << endl;
    
    rooFitHist(*h4, &yield, &yield_err, "Kaon Momentum Cut");
    cout << "kaon_momentum: " << *cnt4  << " Signal: " << yield << endl;
    
    rooFitHist(*h5, &yield, &yield_err, "Vertex Cut");
    cout << "vertex_z: " << *cnt5  << " Signal: " << yield << endl;
    
    rooFitHist(*h6, &yield, &yield_err, "Path Length Sig Cut");
    cout << "xim_pathlensig: " << *cnt6  << " Signal: " << yield << endl;
    
	//h3->SetLineColor(kRed);
	//h4->SetLineColor(kGreen);
	//h6->SetLineColor(kMagenta);

    TLine* hl = new TLine(1.25,3,1.5,3);
	hl->SetLineColor(2); hl->SetLineWidth(3);
	// TArrow* ar = new TArrow(1.35, 3, 1.35, 1.5, 0.05, "|>");
	// ar->SetLineColor(2); ar->SetFillColor(2); ar->SetLineWidth(3);
	TArrow* ar = new TArrow(1.35, 3, 1.35, 4.5, 0.05, "|>");
	ar->SetLineColor(2); ar->SetFillColor(2); ar->SetLineWidth(3);

	// h2->DrawClone("hist same");
    //	h9->DrawClone("colz same");

	// h3->DrawClone("same");
	// h4->DrawClone("same");
	// h5->DrawClone("same");
	// //hl->Draw("same");ar->Draw("");
	//h->DrawClone("colz");
	
	// h2->SetFillColorAlpha(kMagenta,0.1);
	// h2->DrawClone("hist same");
	// h1->SetLineColor(kRed);
	// h1->DrawClone("hist same");

	//df2.Snapshot("kpkpxim_flatTree", (gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_postQVal")+root_file_name+".root").c_str());
}

void rooFitHist(TH1 &hist, double* yield, double* yield_err, std::string canName = "c")
{
  Double_t min_mass = hist.GetXaxis()->GetBinLowEdge(hist.FindFirstBinAbove(0,1,1, hist.FindBin(1.3)));
  if(min_mass < 1.28) min_mass = 1.28;

  TCanvas *c = new TCanvas(canName.c_str(), canName.c_str());
  RooWorkspace* w = new RooWorkspace("ws_name");
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", min_mass, 1.47);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, &hist);
  //XiMassKinFit_Ebin_accsub->Print();
  w->import(RooArgSet(mass)); 
  
  w->factory("Chebychev::bkgd(mass,{a0[0.8,0.01,1.2]})");//,a1[-0.1,-0.5,-0.05]
  //w->factory("CBShape::xigaus(mass,xi_mean[1.322],xi_sig[0.005,0.003,0.01], alpha[5,1,20], n[5,0,10])");
  w->factory("Johnson::xigaus(mass,mu[1.322,1.31, 1.33],lambda[0.005], gamma[0], delta[1.5, 1, 2])");
  //w->factory("Gaussian::xigaus(mass,xi_mean[1.322],xi_sig[0.005, 0.004, 0.006])");
  //w->factory("Voigtian::sigma(mass,mean[1.387],sig[0.005], width[0.018, 0.01, 0.024])");
  //w->factory("Gaussian::siggaus(mass,sigma_mean[1.385],sigma_sig[0.016])");
  w->factory("SUM::model(nbkgd[200,0,1e6]*bkgd, nxi[100,1,1e6]*xigaus)");

  RooPlot* massframe = mass.frame(RooFit::Title(" "));
  w->pdf("model")->fitTo(*data,RooFit::Extended(kTRUE),RooFit::AsymptoticError(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.5, 0.95, 0.92) );
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(2) );
  w->pdf("xigaus")->plotOn(massframe,RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(4010),RooFit::MoveToBack(), RooFit::Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent) );
    w->pdf("bkgd")->plotOn(massframe, RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent) );
  
  *yield = w->var("nxi")->getVal();
  *yield_err = w->var("nxi")->getError();
  
  massframe->Draw();

}
