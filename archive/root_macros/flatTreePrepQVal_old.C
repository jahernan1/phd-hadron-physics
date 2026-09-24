#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

void rooFitHist(TH1 &hist, double* yield, double* yield_err, std::string canName="c", bool sigmabool = true);
void GetPrepedFlatTree(int n_threads = 4, string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111");

int flatTreePrepQVal()
{
  GetPrepedFlatTree(6,"flatTree_kpkpxim__M23_2017-01_ver45");

  return 0;
}

void GetPrepedFlatTree(int n_threads = 4, string root_file_name="kpkpxim_2017-01_ana45_JohnsonFit_111111") {
	// Parallelize with n threads
        if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
	// Branches you want to use get
	std::vector<std::string> branches = {	};

	// make data frame
	// format : tree name, file name, branches to open
	auto df = ROOT::RDataFrame("kpkpxim_flatTree", ("/d/grid17/hjesse/macros/QFactors/logs/"+root_file_name+"/postQVal_flatTree_"+root_file_name+".root"));
    //auto df = ROOT::RDataFrame("flatTree_kpkpxim", "/d/grid17/hjesse/Trees/flatTree/flatTree_kpkpxim__M23_2017-01_ver45_Ystar2400_t05_genr8.root", branches);
    
    //auto accCut = [](double f, double e) {return f == 1. && e >= 6.4;};
	auto df1 = df.Filter("beam_E > 6.4 && beam_E < 11.4")
      .Define("qacc_decayxim_M", "acc_weight*qvalue_decayxim_M")
      .Define("qbkg_decayxim_M", "acc_weight*(1-qvalue_decayxim_M)");
    //                .Filter("decayxim_M > 1.308 && decayxim_M < 1.338");

    auto df2 = df1.Filter("abs(total_mm2) < 0.04 ","mm2Cut");
    auto df3 = df1.Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut");
    auto df4 = df1.Filter("xim_vertexZ < lambda_vertexZ","baryonZCut");
    auto df5 = df.Filter("xim_pathlensig > 2.0", "ximPathLenSigCut");
    //.Filter("kp_highp_P3 >= 3 && kp_lowp_P3 < 3","kpmomCut")
      
    auto df7 = df1.Filter("decayxim_M > 1.308 && decayxim_M < 1.338");
    
    auto h1 = df1.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    auto h2 = df2.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    auto h3 = df3.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    auto h4 = df4.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    auto h5 = df5.Histo1D({"",";M(#Lambda#pi^{-}) (GeV); Events",125,1.25,1.5},"decayxim_M", "acc_weight");
    
    auto h7 = df1.Histo1D({"",";-t (GeV^2); Events",100,0,10},"t_dist", "acc_weight");
    auto h8 = df5.Histo1D({"",";-t (GeV^2); Events",100,0,10},"t_dist", "acc_weight");
    h8->SetLineColor(kRed);
    h8->Scale(h8->GetEntries()/h7->GetEntries());
    double yield, yield_err;
    new TCanvas;
    h7->DrawClone("e1");
    //h2->DrawClone("e1 same");
    //h3->DrawClone("e1 same");
    h8->DrawClone("e1 same");  
    //h5->DrawClone("e1 same");  
    //rooFitHist(*h, &yield, &yield_err, "ChiSqNdf Cut");
    //rooFitHist(*h5, &yield, &yield_err, "All Cut",false);
    //rooFitHist(*h2, &yield, &yield_err, "All Cut");
    //auto cut_report = df1.Report();
    //df1.Report()->Print();
	df5.Report()->Print();
    //df1.Snapshot("kpkpxim_flatTree", ("/d/grid17/hjesse/AnalysisNote/flatTrees/"+root_file_name+"_ChiSqCut.root").c_str());
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
