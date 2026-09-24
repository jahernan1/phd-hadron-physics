#include "TF1.h"
#include "TH1.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include "TMath.h"
#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"

void rooFitHist(TH1* hist, string histTitle, double* sigYield, double* sigYieldErr, double* bkgYield, double* bkgYieldErr);
TH1* AccSubTH1(string rootFilePath, string histName);
void plotRatio(string plotName, string dataSetName, string plotTitle, vector<double> arr_yield, double cutVal=8);
void setStyle();
void GetFilterHist(TH2 *hist, string root_file_path="flatTree_kpkpxim__M23_2017-01_ver45", string cutName="chisqndf",int n_threads = 16);
void GetCutAnalysis(vector<double> vec_CutParamBin, string rootFileName, double cutVal=8,string cutNameDelim="chisqndf", string cutPlotName="#chi^{2}_{#nu}");

int CutAnalysis()
{
  vector<string> rootFile = {"flatTree_kpkpxim__M23_2017-01_ana56", "flatTree_kpkpxim__B4_M23_2018-01_ana03", "flatTree_kpkpxim__B4_M23_2018-08_ana02","kpkpxim_2018-08_ana-05_LE","kpkpxim_2019-11_ana-04_b05_b12"};
  vector<double> chiSqNdfBins{24, 0, 12, 2};
  vector<double> mm2Bins{20, 0, 0.05, 1};
  
  //run cut analysis
  for(int loc_i=0; loc_i < 3; loc_i++)
    {
      GetCutAnalysis(chiSqNdfBins, rootFile[loc_i]);
      GetCutAnalysis(mm2Bins, rootFile[loc_i], 0.02 , "total_mm2", "#left|MM_{#it{x}}(#gammapK^{+}K^{+}#Xi^{-})#right|^{2} #left(GeV^{2}#right)" );
    }

  return 0;
}

void GetCutAnalysis(vector<double> vec_CutParamBin, string rootFileName, double cutVal=5, string cutNameDelim="chisqndf", string cutPlotName="#chi^{2}_{#nu}")
{// vec_CutParamBins 0=numbins, 1=minCutVal, 2=maxCutVal, 3=minBinNum  
  string thisDir = "/d/grid17/hjesse/AnalysisNote/cut_analysis_plots/";  
  vector<TH1*> hist_XiMass;
  vector<double> ratioFOM;
  vector<double> ratioFOMErr;
  vector<double> ratioSB;
  vector<double> ratioSBErr;
  vector<double> arr_yield;
  FILE *fCutFOM = fopen((thisDir+"data/"+cutNameDelim+"Cut_FOM_"+rootFileName+".txt").c_str(), "w+");
  FILE *fCutSB = fopen((thisDir+"data/"+cutNameDelim+"Cut_SB_"+rootFileName+".txt").c_str(), "w+");
  FILE *fCutYield = fopen((thisDir+"data/"+cutNameDelim+"Cut_Yield_"+ rootFileName+".txt").c_str(), "w+");
  int cnt = 0;
  
  TH2* hist_XiMass_Cut = new TH2D("",(";M(#Lambda#pi^{-}) (GeV);"+cutPlotName).c_str(),125,1.25,1.5, vec_CutParamBin[0],vec_CutParamBin[1],vec_CutParamBin[2]);
  //Get the histogram with cuts
  GetFilterHist(hist_XiMass_Cut, rootFileName.c_str(), cutNameDelim);
  double numBins = hist_XiMass_Cut->GetNbinsY();
    
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  TCanvas* cut_Can = new TCanvas( ("XiMinus_"+cutNameDelim+"Cut").c_str(), (cutNameDelim+"Cut Fits").c_str(), 1200, 900);// Plot Fits;
  //cut_Can->DivideSquare(numBins,1e-4,1e-4);
  cut_Can->Divide(4,numBins/4,1e-4,1e-4);
  //XiMinusInvariantMass_ChiSqNdf->Draw("e1");
  
  for(int bin_i = int(vec_CutParamBin[3]); bin_i < hist_XiMass_Cut->GetNbinsY(); ++bin_i )
    //for(int bin_i = int(vec_CutParamBin[3]); bin_i < 20; bin_i++ )
    {
      double sigYield, sigYieldErr, bkgYield, bkgYieldErr;
      double cut_val = hist_XiMass_Cut->GetYaxis()->GetBinCenter( bin_i ) + hist_XiMass_Cut->GetYaxis()->GetBinWidth(bin_i)/2 ;
      
      // Get mass projection for cut
      hist_XiMass.push_back(hist_XiMass_Cut->ProjectionX(("_px_"+rootFileName+"_"+to_string(cut_val)).c_str(),1,bin_i,"e"));
      //hist_XiMass.push_back(hist_XiMass_Cut->ProjectionX(("_px_"+rootFileName+"_"+to_string(cut_val)).c_str(),bin_i,numBins,"e"));
      
      cut_Can->cd(cnt+1);
      cut_Can->SetLeftMargin(0.05);
      hist_XiMass[cnt]->SetMarkerSize(0.5);
      
      // Fit mass distribution
      rooFitHist(hist_XiMass[cnt], (cutNameDelim +"Cut < " + to_string(cut_val)).c_str(), &sigYield, &sigYieldErr, &bkgYield, &bkgYieldErr);
      cout << "Fit Results: " << cut_val << "\t" << sigYield << "\t" << bkgYield << "\n" << endl;

      ratioFOM.push_back( sigYield / sqrt (sigYield +  bkgYield));
      ratioSB.push_back(sigYield /  bkgYield);
      //ratioSBErr.push_back( ratioSB[bin_i] * sqrt( pow( sigYieldErr / sigYield, 2 ) + pow( bkgYieldErr / bkgYield, 2 ) ));
      arr_yield.push_back(sigYield);
      
      // Record significance values
      fprintf(fCutFOM, "%f \t %f \n", cut_val, ratioFOM[cnt]);//, ratioSBErr[i]);
      fprintf(fCutSB, "%f \t %f \n", cut_val, ratioSB[cnt]);//, ratioSBErr[i]);
      fprintf(fCutYield, "%f \t %f \n", cut_val, sigYield);//, sigYieldErr);//, ratioSBErr[i]);
      cnt=cnt+1;
    }
  
  // Close Files
  fclose(fCutFOM);
  fclose(fCutSB);
  fclose(fCutYield);

  // Save Fits
  cut_Can->SaveAs((thisDir+"results/"+"XiMinus_"+cutNameDelim+"Cut_Fits_"+ rootFileName +".pdf").c_str());
  //cut_Can->Close();
  // Plot FOM and SB
  plotRatio(cutNameDelim+"Cut", rootFileName, " ;"+cutPlotName +"; N_{S}#scale[1.6]{/}#sqrt{N_{S}+N_{B}}", arr_yield, cutVal);
}

void GetFilterHist(TH2 *hist, string root_file_name="flatTree_kpkpxim__M23_2017-01_ver45", string cutName="chisqndf",int n_threads = 4) {
 	// Parallelize with n threads
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
	// Branches you want to use get
	// std::vector<std::string> branches = {"decayxim_M",
    //                                      "best_combo","best_combo_rf","acc_weight",
    //                                      "xim_pathlensig", "lambda_pathlensig",
    //                                      "chisqndf",
    //                                      "total_mm2",
    //                                      "beam_E","kplow_p4"
	// };
    
	// make data frame
	// format : tree name, file name, branches to open
    auto df = ROOT::RDataFrame("flatTree_kpkpxim", ("/d/grid17/hjesse/Trees/flatTree/rawTrees/"+root_file_name+".root").c_str());
    auto df1 = df.Define("hybrid_combo","best_combo_rf*acc_weight")
        .Filter("beam_E > 6.4 && beam_E < 11.4")
        .Filter("abs(total_mm2) < 0.02 ","mm2Cut")
        .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")
        .Filter("xim_pathlensig > 2", "XimPathLenSigCut>2")
        .Filter("lambda_pathlensig > 0", "LambPathLenSigCut>0")
        .Filter("kphigh_p4.Rapidity() > 2","KaonRapidityCut");
        
    auto df2 = df.Define("hybrid_combo","best_combo_rf*acc_weight")
        .Filter("beam_E > 6.4 && beam_E < 11.4") 
        .Filter("chisqndf < 10")
        .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")
        .Filter("xim_pathlensig > 2", "XimPathLenSigCut>2")
        .Filter("lambda_pathlensig > 0", "LambPathLenSigCut>0")
        .Filter("kphigh_p4.Rapidity() > 2","KaonRapidityCut")
        .Define("total_mm22", [](double mm2){return TMath::Abs(mm2);}, {"total_mm2"});
    
    if(cutName=="chisqndf")
        df1.Foreach([&hist](double mass, double cut, double weight){hist->Fill(mass, cut, weight);},{"decayxim_M", cutName.c_str(), "hybrid_combo"});
    else if(cutName=="total_mm2")
        df2.Foreach([&hist](double mass, double cut, double weight){hist->Fill(mass, cut, weight);},{"decayxim_M", "total_mm22","hybrid_combo"});
    //hist->DrawClone("colz");
    //cout << " Count:" << *cnt1 << " Signal: " << yield << endl;
}

void plotRatio(string plotName, string dataSetName, string plotTitle, vector<double> arr_yield, double cutVal=5)
{
  setStyle();
  string thisDir = "/d/grid17/hjesse/AnalysisNote/cut_analysis_plots/";
  string analysisDir = "/d/grid17/hjesse/AnalysisNote/analysis/event_selection/chisqndf_cut/";
  TCanvas* c = new TCanvas(plotName.c_str(), plotName.c_str());
  TPad *pad1 = new TPad((plotName+"_Pad1").c_str(),"",0,0,1,1);
  TPad *pad2 = new TPad((plotName+"_Pad2").c_str(),"",0,0,1,1);
  pad2->SetFillStyle(4000); //will be transparent
  pad2->SetFrameFillStyle(0);
  //pad2->SetFrameLineColor(kRed+1);
  //pad2->GetFrame()->SetLineWidth(10);
  pad1->SetGrid();
    
  TGraphErrors *g1 = new TGraphErrors( (thisDir+"data/"+plotName+"_FOM_"+dataSetName+".txt").c_str(), "%lg %lg");//, option=" \t,;");
  g1->SetTitle(plotTitle.c_str());
  g1->SetMarkerStyle(20);
  g1->SetMarkerSize(1.4);
  g1->SetDrawOption("APL");
  g1->SetMarkerColor(kBlack);
  g1->SetMarkerStyle(24);
  g1->SetLineWidth(2);
  g1->SetFillStyle(0);
  //g1->GetXaxis()->SetNdivisions(510);
  //gStyle->SetPadTickX(1);
  pad1->Draw();
  pad1->cd();
  g1->Draw("APL");
  
  TGraphErrors *g2 = new TGraphErrors( (thisDir+"data/"+plotName+"_SB_"+dataSetName+".txt").c_str(), "%lg %lg");//, option=" \t,;");
  g2->SetTitle(" ; ; N_{S}/N_{B}");
  g2->SetMarkerStyle(20);
  g2->SetMarkerSize(1.4);
  g2->SetDrawOption("APLY+");
  g2->SetMarkerColor(kRed+1);
  g2->SetLineColor(kRed+1);
  g2->SetLineWidth(2);
  g2->SetFillStyle(0);  
  g2->GetYaxis()->SetAxisColor(kRed+1);
  //g2->GetYaxis()->SetTickSize();
  g2->GetYaxis()->SetTitleOffset(0.9);
  g2->GetYaxis()->SetTitleColor(kRed+1);
  g2->GetYaxis()->SetLabelColor(kRed+1);
  pad2->Draw();
  pad2->cd();
  g2->Draw("APLY+");

  c->Update();
  //double ypadmax = pad2->PixeltoY(UtoPixel(pad2->GetUymax()));
  double ypadmin,ypadmax,xpadmin,xpadmax;
  pad2->GetRangeAxis(xpadmin,ypadmin,xpadmax,ypadmax);
  cout << "Pad Values: " << xpadmin << "\t "<< xpadmax << "\t" << ypadmin << "\t" << ypadmax << endl;
  TLine* cutLine = new TLine(cutVal, ypadmin, cutVal, ypadmax);
  cutLine->SetLineWidth(3);
  cutLine->SetLineStyle(10);
  cutLine->SetLineColor(kBlue+1);
  cutLine->Draw();
  
  TArrow *arCut = new TArrow(cutVal,(ypadmax+ypadmin)/2,cutVal*0.5,(ypadmax+ypadmin)/2, 0.05,"|>");
  //ar4->SetAngle(60);
  arCut->SetLineWidth(3);
  arCut->SetFillColor(kBlue+1);
  arCut->Draw();
  // Save Plot
  c->Update();
  c->SaveAs((thisDir+"results/"+plotName+"_"+dataSetName+".pdf").c_str());
  c->SaveAs((analysisDir+plotName+"_"+dataSetName+".pdf").c_str());
}

void rooFitHist(TH1* hist, string histTitle, double* sigYield, double* sigYieldErr, double* bkgYield, double* bkgYieldErr)
{
  //setStyle();
  Double_t min_mass = hist->GetXaxis()->GetBinLowEdge(hist->FindFirstBinAbove(0,1,5, hist->FindBin(1.3)));
  if(min_mass < 1.28) min_mass = 1.28;

  RooWorkspace* w = new RooWorkspace(histTitle.c_str());
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", min_mass, 1.45);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  RooPlot* massframe = mass.frame(RooFit::Title(histTitle.c_str()));
  w->import(RooArgSet(mass));

  w->factory("Chebychev::bkgd(mass,{a0[0.8,0.1,1.5],a1[-0.2,-1,-0.1]})");//,a1[-0.1,-2,-1e-2]
  //w->factory("Voigtian::sigma(mass,mean[1.385,1.383,1.388],sig[0.0055. 0.004, 0.006], width[0.015, 0.01, 0.042])");
  w->factory("Gaussian::sigma(mass,mean[1.385,1.383,1.390],sig[0.019,0.018,0.022])");
  w->factory("Johnson::xigaus(mass,mu[1.3217,1.32,1.33],lambda[0.0055,0.004,0.006], gamma[0], delta[1.3,1.,2.])");
  //w->factory("Gaussian::xigaus(mass,mean_xi[1.322,1.31,1.33],sigma_xi[0.0055,0.004,0.008])");
    
  //Create model and fit to data
  w->factory("SUM::model( nsigma[300,1,1e6]*sigma, nbkgd[2000,1,1e6]*bkgd, nxi[1000,1,1e6]*xigaus)");//nbkgd[200,1,1e6]*bkgd,
  
  w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::SumW2Error(false),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.45, 0.8, 0.91) ,RooFit::Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nsigma"), *w->var("nbkgd"))));//,RooFit::Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("lambda"), *w->var("nsigma"), *w->var("mean"), *w->var("sig"), *w->var("nbkgd") 
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(1));
  w->pdf("xigaus")->plotOn(massframe,RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent));
  w->pdf("sigma")->plotOn(massframe, RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nsigma")->getVal(), RooAbsReal::NumEvent));
  w->pdf("bkgd")->plotOn(massframe,RooFit::LineWidth(1), RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent));

  //RooFit::DrawOption("F"), RooFit::FillColor(kAzure-1),RooFit::FillStyle(4010),RooFit::MoveToBack()
  //Get background under signal region
  double xiMu = w->var("mu")->getVal();
  double xiLambda = w->var("lambda")->getVal();
  double xiDelta = w->var("delta")->getVal();
  double xiGamma = w->var("gamma")->getVal();
  double xiMean = xiMu - xiLambda*exp(1 / (2*pow(xiDelta,2)) )*sinh(xiGamma/xiDelta);
  double xiSigma = sqrt( pow(xiLambda, 2)/2*(exp(pow(xiDelta, -2) ) - 1 )*(exp(pow(xiDelta, -2) )*cosh(2*xiGamma/xiDelta )+1));
  //For Gaussian signal
  // double xiMean = w->var("mean_xi")->getVal();
  // double xiSigma = w->var("sigma_xi")->getVal();
  
  printf("Mean and Sigma: %f, %f \n", xiMean, xiSigma);
  double xCutL = xiMean - 2*xiSigma;
  double xCutR = xiMean + 2*xiSigma;
  w->var("mass")->setRange("signal", xCutL, xCutR);
  
  RooAbsReal* sig_sig = w->pdf("xigaus")->createIntegral(mass, RooFit::NormSet(mass), RooFit::Range("signal"));
  printf("Signal Fraction in 2Sigma Window: %f \n", sig_sig->getVal());
  printf("Signal Yield in 2Sigma Window: %f \n", w->var("nxi")->getVal()*sig_sig->getVal());

  *sigYield = w->var("nxi")->getVal()*sig_sig->getVal();
  *sigYieldErr = w->var("nxi")->getError()*sig_sig->getVal();
  
  RooAbsReal* bkg_sig = w->pdf("bkgd")->createIntegral(mass, RooFit::NormSet(mass), RooFit::Range("signal"));
  RooAbsReal* bkg_sigma = w->pdf("sigma")->createIntegral(mass, RooFit::NormSet(mass), RooFit::Range("signal"));
  printf("Background Fraction of 2Sigma Window: %f, %f \n", bkg_sig->getVal(),bkg_sigma->getVal());
  printf("Background Yield in 2Sigma Window: %f \n", w->var("nbkgd")->getVal()*bkg_sig->getVal()+w->var("nsigma")->getVal()*bkg_sigma->getVal());
  
  *bkgYield = w->var("nbkgd")->getVal()*bkg_sig->getVal()+w->var("nsigma")->getVal()*bkg_sigma->getVal();
  *bkgYieldErr = w->var("nbkgd")->getError()*bkg_sig->getVal();
  
  //massframe->SetMaximum(massframe->GetMaximum()+100);
  double ypadmax = massframe->GetMaximum();
  TLine* cutLineL = new TLine(xCutL, 0.0, xCutL, ypadmax);
  cutLineL->SetLineWidth(1);
  cutLineL->SetLineColor(kRed+1);
  massframe->addObject(cutLineL, " ");
  TLine* cutLineR = new TLine(xCutR, 0.0, xCutR, ypadmax);
  cutLineR->SetLineWidth(1);
  cutLineR->SetLineColor(kRed+1);
  massframe->addObject(cutLineR, " ");

  massframe->Draw();
}

// input specific style formatting of user choice
void setStyle()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(600);
  gStyle->SetCanvasDefW(800);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.18);
  gStyle->SetPadRightMargin (0.18);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);

  gStyle->SetFrameFillStyle ( 0);
  gStyle->SetFrameFillColor ( 0);
  gStyle->SetFrameLineColor ( 1);
  gStyle->SetFrameLineStyle ( 0);
  gStyle->SetFrameLineWidth ( 2);
  gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);
  //gStyle->SetTickSize( 0.05);

  gStyle->SetNdivisions(510);

  //gStyle->SetLineWidth(1);
  //gStyle->SetHistLineWidth(1);
  //gStyle->SetFrameLineWidth(2);
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.);
  gStyle->SetMarkerStyle(24);

  gStyle->SetLabelSize(0.07,"X");
  gStyle->SetLabelSize(0.07,"Y");

  gStyle->SetLabelOffset(0.008,"X");
  gStyle->SetLabelOffset(0.008,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132,"T");
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");

  gStyle->SetTitleSize(0.105,"T");
  gStyle->SetTitleSize(0.08,"X");
  gStyle->SetTitleSize(0.08,"Y");
  
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(1.0,"Y");

  gStyle->SetTextSize(0.08);
  gStyle->SetTextFont(132);

  gStyle->SetOptStat(0);

  gROOT->ForceStyle();

  TLatex* latex = new TLatex();
  latex->SetNDC();
  latex->SetTextFont(132);
  latex->SetTextSize(0.08);
  latex->SetTextAlign(32);
  gROOT->ForceStyle();
}
