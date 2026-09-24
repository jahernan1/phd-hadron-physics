#include "gxana/common/Paths.h"
#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

void rooFitHist(TH1 &hist, double* yield, double* yield_err, std::string canName="c", bool sigmabool = true);
void GetPlotFromFlat(int n_threads = 4, string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111");

int flatTreePlots()
{
  GetPlotFromFlat(6,"flatTree_kpkpxim__M23_2017-01_ver45");

  return 0;
}

void GetPlotFromFlat(int n_threads = 4, string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111") {
	// Parallelize with n threads
        if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M",
                                         "beam_vertexZ"
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
                                         "kp_lowp_YstarRest_CosTheta",
                                         "beam_E",
                                         "t_dist"
	};

	// make data frame
	// format : tree name, file name, branches to open
    auto df = ROOT::RDataFrame("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "Trees/flatTree/")+root_file_name+".root").c_str(), branches);
    //auto df = ROOT::RDataFrame("flatTree_kpkpxim", gxana::EnvPath("GXANA_DATA", "Trees/flatTree/flatTree_kpkpxim__M23_2017-01_ver45_Ystar2400_t05_genr8.root").c_str(), branches);
    
    //auto accCut = [](double f, double e) {return f == 1. && e >= 6.4;};
	auto df1 = df.Filter("beam_E > 6.4 && beam_E < 11.4")
      .Filter("chisqndf < 4.0","chisqndfCut");
      
    auto df2 = df1.Filter("abs(total_mm2) < 0.04 ","mm2Cut");
    auto df3 = df1.Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut");
    auto df4 = df3.Filter("xim_vertexZ < lambda_vertexZ","baryonZCut");
    auto df5 = df1.Filter("xim_pathlensig > 2.0", "ximPathLenSigCut");
    //.Filter("kp_highp_P3 >= 3 && kp_lowp_P3 < 3","kpmomCut")  
    auto df7 = df1.Filter("decayxim_M > 1.308 && decayxim_M < 1.338");
    
    auto h1 = df7.Histo2D({"","; cos#theta_{#gamma#K^{+}_{Prod}};#bf{#vec{p}}(K^{+}_{Prod}) (GeV/c)}", 100, 0, 1,100,0,10}, "kp_highp_CosTheta","kp_highp_P3", "acc_weight");
    auto h2 = df7.Histo2D({""," ; cos#theta_{Y^{*}#K^{+}_{Y^{*}}};;#bf{#vec{p}}(K^{+}_{Y^{*}}) (GeV/c)", 200, -1, 1,100,0,10}, "kp_lowp_YstarRest_CosTheta","kp_lowp_P3", "acc_weight");
    auto h3 = df7.Histo2D({"","; cos#theta_{#gamma#K^{+}_{1}} ;#bf{#vec{p}}(K^{+}_{1}) (GeV/c)", 200, 0, 1,100,0,10}, "kp1_CosTheta","kp1_P3", "acc_weight");
    auto h4 = df4.Histo2D({"","; M(#Lambda#pi^{-}) (GeV/c^{2}); #Xi^{-} Flight Significance", 125, 1.25, 1.5,60,0,30}, "decayxim_M","xim_pathlensig", "acc_weight");
    auto h9 = df1.Histo1D({"",";-t (GeV^2); Events",100,0,10},"t_dist", "acc_weight");
    auto h10 = df5.Histo1D({"",";-t (GeV^2); Events",100,0,10},"t_dist", "acc_weight");
    //h8->SetLineColor(kRed);
    //h8->Scale(h8->GetEntries()/h7->GetEntries());
    double yield, yield_err;
    
    gStyle->SetOptStat(0);
    TCanvas *c = new TCanvas();
    h4->DrawClone("colz");
    c->Update();
    TLine *l = new TLine(1.25,2,1.5,2);
    l->SetLineWidth(3);
    l->SetLineColor(kRed);
    TArrow *ar = new TArrow(1.38,2,1.38,15,0.03,"|>");
    ar->SetLineWidth(3);
    ar->SetLineColor(kRed);
    ar->SetFillColor(kRed);
    l->Draw();
    ar->Draw();
    
    //rooFitHist(*h, &yield, &yield_err, "ChiSqNdf Cut");
    //rooFitHist(*h5, &yield, &yield_err, "All Cut",false);
    //rooFitHist(*h2, &yield, &yield_err, "All Cut");
    //auto cut_report = df1.Report();
    //df1.Report()->Print();
	df5.Report()->Print();
    //df1.Snapshot("kpkpxim_flatTree", (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_ChiSqCut.root").c_str());
}

void rooFitHist(TH1 &hist, double* yield, double* yield_err, std::string canName = "c", bool sigmabool = true )
{
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR) ;
  Double_t min_mass = hist.GetXaxis()->GetBinLowEdge(hist.FindFirstBinAbove(0,1,1, hist.FindBin(1.3)));
  if(min_mass < 1.26) min_mass = 1.26;

  //TCanvas *c = new TCanvas(canName.c_str(), canName.c_str());
  RooWorkspace* w = new RooWorkspace("ws_name");
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", min_mass, 1.45);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, &hist);
  RooPlot* massframe = mass.frame(RooFit::Title(" "));
  //XiMassKinFit_Ebin_accsub->Print();
  w->import(RooArgSet(mass)); 
  
  if (sigmabool)
    {
      w->factory("Chebychev::bkgd(mass,{a0[0.8,0.01,1.2]})");//,a1[-0.1,-0.5,-0.05]
      // w->factory("Voigtian::sigma(mass,mean[1.3872,1.3867,1.3877],sig[0.005. 0.004, 0.007], width[0.02, 0.01, 0.0415])");
      // w->factory("Johnson::xigaus(mass,mu[1.322,1.31,1.33],lambda[0.005,0.004,0.007], gamma[0], delta[1.1,0.1,10])");
      w->factory("Johnson::xigaus(mass,mu[1.322,1.31, 1.33],lambda[0.005, 0.004, 0.006], gamma[0], delta[1.5, 0.1, 5])");
      w->factory("Voigtian::sigma(mass,mean[1.387],sig[0.005], width[0.018, 0.01, 0.024])");
      w->factory("SUM::model(nbkgd[200,1,1e6]*bkgd, nxi[100,1,1e6]*xigaus, nsigma[0,0,1e5]*sigma)");
    }
  else
    {
      w->factory("Chebychev::bkgd(mass,{a0[0.8,0.01,1.2]})");//a1[-0.1,-0.5,-0.05]
      w->factory("Johnson::xigaus(mass,mu[1.322,1.31,1.33],lambda[0.005,0.004,0.006], gamma[0], delta[1.5,0.1,5])");
      // w->factory("Johnson::xigaus(mass,mu[1.322,1.31, 1.33],lambda[0.005], gamma[0], delta[1.5, 1, 2])");
      w->factory("SUM::model(nbkgd[200,1,1e6]*bkgd, nxi[100,1,1e6]*xigaus)");
    }
  
  w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::SumW2Error(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false)); //RooFit::AsymptoticError(true)
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.5, 0.95, 0.92) );
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(2) );
  w->pdf("xigaus")->plotOn(massframe,RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(4010),RooFit::MoveToBack(), RooFit::Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent) );
  if(sigmabool)
    w->pdf("sigma")->plotOn(massframe, RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(4010),RooFit::MoveToBack() , RooFit::Normalization(w->var("nsigma")->getVal(), RooAbsReal::NumEvent));
  
  w->pdf("bkgd")->plotOn(massframe, RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent) );
  
  *yield = w->var("nxi")->getVal();
  *yield_err = w->var("nxi")->getError();
  
  massframe->Draw("same");
}
