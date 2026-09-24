#include "gxana/common/Paths.h"
#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

void rooFitHist(TH1 &hist, double* yield, double* yield_err, std::string canName="c", bool sigmabool = true);
void GetPrepedFlatTree(string root_file_name="flatTree_kpkpxim__M23_2017-01_ver45", int n_threads = 15);

//main
int flatTreePrep()
{
  //Data
  // GetPrepedFlatTree();
  GetPrepedFlatTree("flatTree_kpkpxim__M23_2017-01_ver56");
  //GetPrepedFlatTree("flatTree_kpkpxim__M23_2017-01_ver56_Ystar2400_1600_t14_genr8");
  GetPrepedFlatTree("flatTree_kpkpxim__B4_M23_2018-01_ver03");
  //GetPrepedFlatTree("flatTree_kpkpxim__B4_M23_2018-01_ana03_Ystar2400_1600");
  GetPrepedFlatTree("flatTree_kpkpxim__B4_M23_2018-08_ver02");
  //GetPrepedFlatTree("flatTree_kpkpxim__B4_M23_2018-08_ana02_Ystar2400_1600");
  
  return 0;
}

void GetPrepedFlatTree(string root_file_name="flatTree_kpkpxim__M23_2017-01_ver45", int n_threads = 8 ) {
	// Parallelize with n threads
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	//Import select braches to speed things up
    // std::vector<std::string> branches = {"beam_E",
    //                                      "chisqndf",
    //                                      "total_mm2",
    //                                      "beam_vertexZ"
    //                                      "xim_vertexZ",
    //                                      "lambda_vertexZ",
    //                                      "kp_highp_P3",
    //                                      "kp_lowp_P3",
    //                                      "xim_pathlensig",
    //                                      "lambda_pathlensig",
    //                                      "decayxim_M",
    //                                      "t_dist"
    // };
    
	// make data frame
	// format : tree name, file name, branches to open
    // gxana: legacy line redeclared `df` from a QFactors-log RDataFrame that was immediately
    // shadowed by the `df` below; dropped as dead code (compile error, never ran as checked in).
    auto df0 = ROOT::RDataFrame("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "Trees/flatTree/rawTrees/")+root_file_name+".root").c_str());
    auto df = df0.Filter("decayxim_M<1.334 && decayxim_M>1.31","xiMassCut")
      .Filter("best_combo==1");

    // Perform nominal cuts
    auto df1 = df0.Filter("beam_E > 6.4 && beam_E < 11.4")
      .Filter("chisqndf < 4 ","chiSqNdfCut")
      .Filter("abs(total_mm2) < 0.02 ","mm2Cut")
      .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")
      .Filter("xim_vertexZ < lambda_vertexZ","baryonZCut")
      .Filter("best_combo==1","bestCombo")
      .Define("kphigh_theta","kphigh_p4.Theta()*180/TMath::Pi()")
      .Define("kplow_theta","kplow_p4.Theta()*180/TMath::Pi()");
      //.Define("uniq_weight", "1/combos_survived");
    
    //df1.Display({"evnt_num", "combo_num", "chisqndf", "kp_highp_P3","kp_lowp_P3", "decayxim_M"},50)->Print();
    
    auto df2 = df1.Filter("kp_highp_P3 >= 3 && kp_lowp_P3 < 3","kpmomCut");
    auto df3 = df1.Filter("xim_pathlensig > 1.0", "XimPathLenSigCut>1");
    auto df4 = df3.Filter("lambda_pathlensig > 1.0", "LambPathLenSigCut>1");
    auto dfall = df1.Filter("kp_highp_P3 > 3","kpmomCut>3")
      .Filter("kplow_theta > 25  ","kpthetaCut>25")
      .Filter("xim_pathlensig > 1.0", "XimPathLenSigCut>1")
      .Filter("lambda_pathlensig > 1.0", "LambPathLenSigCut>1");
    
    // Print cut analysis
    cout << root_file_name << endl;
    dfall.Report()->Print();//data
        
    // gxana: legacy line streamed `cout` into itself (`<< cout`), which does not compile;
    // dropped the trailing `<< cout` (compile error, never ran as checked in).
    cout << "Saving flat trees with cuts applied...\n";
    df.Snapshot("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_xiMassCut.root").c_str());
    // df1.Snapshot("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal.root").c_str());
    // df2.Snapshot("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_kpMom3.root").c_str());
    // df3.Snapshot("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_ximVertex1.root").c_str());
    // df4.Snapshot("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_ximLambVertex11.root").c_str());
    // dfall.Snapshot("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "flatTrees/")+root_file_name+"_nominal_all.root").c_str());
    cout << "\n" << endl;
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
