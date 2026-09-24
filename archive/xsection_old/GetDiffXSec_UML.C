//kpkpxim Differential XSec
#include "TF1.h"
#include "TH1.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include "TMath.h"
#include "RooPlot.h"
#include "RooCBShape.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
using namespace RooFit;

// Initalize methods to have main function first 
void setStyle();
void rooFitHist(TH1* hist, char* histTitle, char *ws_name, double *yield, double *yield_err);
void rooFitHistMC(TH1* hist, char* histTitle, char *ws_name, double *yield, double *yield_err);
void getXSec (string root_file_name, TH1D* diff_xsec[], Int_t enbins,  Double_t tmax, Double_t twidth, string delim="_allCuts");
void plotDiffXSec(TH1D* hist1[], TH1D* hist2[], TH1D* hist3[], double xmax, double ymax, const char* saveName);
void plotAllXSec(TH1D* hist1[], TH1D* hist2[], TH1D* hist3[], TH1D* hist4[],double xmax, double ymax, const char* saveName);
void getWeightedAvgHist(TH1D* hist1[], TH1D* hist2[], TH1D* hist3[], TH1D* weighted_hist[10], Double_t tmax, Double_t twidth, string delim);
void getTotWeightedAvgHist(TH1D* hist1, TH1D* hist2, TH1D* hist3, TH1D* weighted_hist);
void plotWeightedAvg(TH1D* hist1[],double xmax, double ymax, const char* saveName);
void getDiffXSec(string delim="_allCuts");
  
// Main function to display the differential Xsec 
int GetDiffXSec()
{
  // getDiffXSec("_vertex");//vertex_sig>0
  // getDiffXSec("_ximVertexCut");//xim_vertexsig>2  
  //getDiffXSec("_vertexCuts");//xim and lambda vertex sig
  //getDiffXSec();
  getDiffXSec("_allKaonSep");
  
  return 0;
}

void getDiffXSec(string delim="_allCuts")
{
  // set style
  setStyle();
  RooMsgService::instance().setGlobalKillBelow(ERROR) ;

  /************************************************
    Get differential cross section for Y* mass 2400
  ************************************************/
  Int_t numEnBins = 10;
  Double_t tmax = 3.6;
  Double_t twidth = 0.35;

  //2017
  TH1D* diff_xsec_201701_2400[numEnBins];
  getXSec("xsec_kpkpxim__M23_2017-01_ana56_nominal"+delim, diff_xsec_201701_2400, numEnBins, tmax, twidth, delim);
  TH1D* diff_xsec_201701_2400_weighted[numEnBins];
  getXSec("xsec_kpkpxim__M23_2017-01_ana56_nominal"+delim+"_weighted", diff_xsec_201701_2400_weighted, numEnBins, tmax, twidth, delim);
  //2018-01
  TH1D* diff_xsec_201801_2400[numEnBins];
  getXSec("xsec_kpkpxim__B4_M23_2018-01_ana03_nominal"+delim, diff_xsec_201801_2400, numEnBins, tmax, twidth, delim);
  TH1D* diff_xsec_201801_2400_weighted[numEnBins];
  getXSec("xsec_kpkpxim__B4_M23_2018-01_ana03_nominal"+delim+"_weighted", diff_xsec_201801_2400_weighted, numEnBins, tmax, twidth, delim);
  //2018-08
  TH1D* diff_xsec_201808_2400[numEnBins];
  getXSec("xsec_kpkpxim__B4_M23_2018-08_ana02_nominal"+delim, diff_xsec_201808_2400, numEnBins, tmax, twidth, delim);
  TH1D* diff_xsec_201808_2400_weighted[numEnBins];
  getXSec("xsec_kpkpxim__B4_M23_2018-08_ana02_nominal"+delim+"_weighted", diff_xsec_201808_2400_weighted, numEnBins, tmax, twidth, delim);
  //2019-11
  // vector<TH1D*> *diff_xsec_201911_2400(numEnBins);
  // getXSec("kpkpxim_2019-11_ana-04_b05_b12","kpkpxim_2019-11_ana-04_b05_b12_genr8","thrown_kpkpxim_2019-11_ana-04_b05_b12_genr8","flux/flux_71943_73266", diff_xsec_201911_2400, tmax, twidth);
  
  // Plot the diff cross section
  plotDiffXSec(diff_xsec_201701_2400_weighted, diff_xsec_201801_2400_weighted, diff_xsec_201808_2400_weighted, tmax, 20, ("diff_xsec_Ystar2400_"+delim+"_Weighted_Phase1").c_str());
  plotDiffXSec(diff_xsec_201701_2400, diff_xsec_201801_2400, diff_xsec_201808_2400, tmax, 20, ("diff_xsec_Ystar2400_"+delim+"_Phase1").c_str());
  // Get weighted average of above
  TH1D* diff_xsec_weightedavg_2400[numEnBins];
  getWeightedAvgHist(diff_xsec_201701_2400_weighted, diff_xsec_201801_2400_weighted, diff_xsec_201808_2400_weighted, diff_xsec_weightedavg_2400, tmax, twidth, delim);
  plotWeightedAvg(diff_xsec_weightedavg_2400, tmax, 20, ("kpkpxim_diff_xsec"+delim+"_Phase1_weightedavg").c_str());
  //record integrated xsection
  
  return 0;
}

void GetXSecComponentsFromFile()
{
  

}

// argv[1] is dataName
// argv[2] is mcName
// argv[3] is thrownName
// argv[4] is fluxName
void getXSec (string dataFile, TH1D* diff_xsec[], Int_t numEnBins, Double_t tmax, Double_t twidth, string delim="_allCuts")
{
  
  ///////////////////////
  // import root files //
  ///////////////////////
  // array for histogram storage
  TObjArray hList(0);
  // directory names 
  string dir = "/d/grid17/hjesse/AnalysisNote/xsection/";
  string datadir = "/d/grid17/hjesse/AnalysisNote/xsection/data_files/";
  string plotdir = "/d/grid17/hjesse/AnalysisNote/xsection/plots/";
    
  // new file name and creation
  string integrated_xsec_filepath = datadir+"Integrated_"+dataFile+".txt";
  string total_xsec_filepath = datadir+"Total_"+dataFile+".txt";  
  FILE *integrated_xsecf = fopen(integrated_xsec_filepath.c_str(),"w+");
  FILE *total_xsecf = fopen(total_xsec_filepath.c_str(),"w+");
  // importing root files
  TFile *f =  TFile::Open( (rootdir+dataFile+".root").c_str());
  
  /////////////////////////////////////////
  // import histograms and rebin to data //
  /////////////////////////////////////////;
  Double_t tmin = 0.1;
  Int_t numtBins = tmax/twidth;
  Double_t small = 1e-4;
  // Data and MC is accidental subtracted
  TH3D *Xi_Egamma_t_accsub = (TH3D*)f->Get("Xi_Egamma_t");
  TH3D *Xi_Egamma_t_MC_accsub = (TH3D*)f->Get("Xi_Egamma_t_MC");
  // store in root file
  hList.Add(Xi_Egamma_t_accsub);
  hList.Add(Xi_Egamma_t_MC_accsub);
  
  // example for manual subtraction
  // TH3I *Xi_Egamma_t_wacc = (TH3*)f->Get("Xi_Egamma_t_wacc");
  // TH3I *Xi_Egamma_t_accsub = (TH3*)Xi_Egamma_t->Clone("Xi_Egamma_t_accsub");
  // Xi_Egamma_t_accsub->Add(Xi_Egamma_t_wacc);
  Int_t tbinrange = Xi_Egamma_t_accsub->GetZaxis()->FindBin(tmax) - Xi_Egamma_t_accsub->GetZaxis()->FindBin(tmin);
  // change tbin range and rebin
  Xi_Egamma_t_accsub->RebinX(Xi_Egamma_t_accsub->GetNbinsX()/numEnBins);
  Xi_Egamma_t_accsub->RebinY(2); 
  Xi_Egamma_t_accsub->RebinZ(tbinrange/numtBins); 
  Xi_Egamma_t_MC_accsub->RebinX(Xi_Egamma_t_MC_accsub->GetNbinsX()/numEnBins);
  Xi_Egamma_t_MC_accsub->RebinY(2);  
  Xi_Egamma_t_MC_accsub->RebinZ(tbinrange/numtBins);
  Int_t ximBins = Xi_Egamma_t_accsub->GetNbinsY();
  Double_t ximBinWidth = Xi_Egamma_t_accsub->GetYaxis()->GetBinWidth(1);
  Int_t tBins = Xi_Egamma_t_accsub->GetNbinsZ();
  Double_t tBinWidth = Xi_Egamma_t_accsub->GetZaxis()->GetBinWidth(1);
  Double_t enBinWidth = Xi_Egamma_t_accsub->GetXaxis()->GetBinWidth(1);
  // Check the bin sizes are correct
  printf("Energy Bin Width: %f\n", enBinWidth);
  printf("Number of Energy Bins: %d\n", numEnBins);
  printf("t Bin Width: %f\n", tBinWidth);
  printf("Number of t Bins for t<%f: %d\n", tmax, numtBins);
  // Flux
  TH1D *tag_flux = (TH1D*)f->Get("tagged_flux");
  tag_flux->Rebin(tag_flux->GetNbinsX()/numEnBins);
  hList.Add(tag_flux);
  // Thrown
  TH2D *thrownT = (TH2D*)f->Get("thrown_Egamma_t");
  TH1D *thrownE = (TH1D*)thrownT->ProjectionX()->Clone();
  thrownE->RebinX(thrownE->GetNbinsX()/numEnBins);
  thrownT->RebinX(thrownT->GetNbinsX()/numEnBins);
  thrownT->RebinY(tbinrange/numtBins);
  hList.Add(thrownE);
  hList.Add(thrownT);
  // functions and fit canvas
  TCanvas *fitCan = new TCanvas("fitCan", "Fit Data Projections", 800,1000);
  fitCan->DivideSquare(numEnBins,small,small);
  TCanvas *fitCanMC = new TCanvas("fitCanMC", "Fit MC Projections", 800,1000);
  fitCanMC->DivideSquare(numEnBins,small,small);
  // Initiate variables for energy loop
  TH1D *py[numEnBins], *pyMC[numEnBins];
  char name_py[50], name_pyMC[50];
  char title_py[50], title_pyMC[50];
  char ws_name[50];
  // total xsec values storage
  Double_t yield[numEnBins] ,yield_err[numEnBins];
  Double_t yieldMC[numEnBins], yieldMC_err[numEnBins];
  Double_t yieldThrown[numEnBins], yieldThrown_err[numEnBins];
  Double_t flux_val[numEnBins], flux_err[numEnBins];
  Double_t eff_val[numEnBins], eff_val_err[numEnBins];
  Double_t xsec_val[numEnBins], xsec_val_err[numEnBins];
  Double_t int_xsec_val[numEnBins], int_xsec_val_err[numEnBins];
  Double_t BR_Lamb = 1;//  0.639;
  Double_t BR_Xi = 1;// 0.99887;
  string xsecName = dataFile+"; E_{#gamma} (GeV); #sigma(#gamma p #rightarrow K^{+}K^{+}#Xi^{-}) (nb)";
  // diff xsec value storage
  Double_t yield_diff[numtBins], yield_diff_err[numtBins];
  Double_t yield_diffMC[numtBins], yield_diffMC_err[numtBins];
  Double_t yield_diffThrown[numtBins], yield_diffThrown_err[numtBins];
  Double_t eff_diff_val[numtBins], eff_diff_val_err[numtBins];
  Double_t diff_xsec_val[numtBins], diff_xsec_val_err[numtBins];
  // Target values
  Double_t Na = 6.02214e23; //[atoms/mol]
  Double_t tar_Len = 29.50; //[cm]
  Double_t tar_Den = 70.11e-3; // 2018 (+/- 0.0035) [g/cm^3]
  //Double_t tar_Den = 71.53e-3; // 2017 (+/- 0.0035) [g/cm^3]
  Double_t hy_MM = 2.01588; //[g/mol]
  //Double_t tar_val = 2 * Na * tar_Len * tar_Den * pow(10,-24) / hy_MM;
  //Double_t tar_val = 1.271;// 2017 (+/- 0.006 b^-1)
  Double_t tar_val = 1.22;
  printf("The target value is: %f\n", tar_val); 

  // Initiate histograms for energy loop
  TH1D *yields_Egamma = new TH1D("yields_Egamma", " ; E_{#gamma} (GeV); #Xi^{-}  Yield", numEnBins, 6.4, 11.4); 
  //total xsec histogram
  TH1D *yields_Egamma_MC = new TH1D("yields_Egamma_MC", " ; E_{#gamma} (GeV); #Xi^{-}  Reconstructed", numEnBins, 6.4, 11.4);
  TH1D *eff_Egamma = new TH1D("eff_Egamma", " ;  E_{#gamma} (GeV); #epsilon(#gamma p#rightarrow K^{+}K^{+}#Xi^{-}", numEnBins, 6.4, 11.4);
  TH1D *xsec_Egamma = new TH1D("xsec_Egamma", xsecName.c_str(), numEnBins, 6.4, 11.4);
  TH1D *int_xsec_Egamma = new TH1D("int_xsec_Egamma", xsecName.c_str(), numEnBins, 6.4, 11.4);

  // diff xsec histogram
  char histName[100], histTitle[100];
  TH1D *yields_Egamma_t[numEnBins];
  TH1D *yields_Egamma_t_MC[numEnBins];
  TH1D *thrown_Egamma_t[numEnBins];
  TH1D *eff_Egamma_t[numEnBins];// = new TH1D("eff_Egamma", " ;  -t (GeV^{2}; #epsilon ", tBins, 0.0, 3.0);
    
  //diffXSec canvas
  TCanvas *yield_t_canvas = new TCanvas("yield_t_canvas", "yield_t", 800, 1000);
  yield_t_canvas->DivideSquare(numEnBins,small,small);
  char savePath_yield_t[200];
  TCanvas *yield_t_MC_canvas = new TCanvas("yield_t_MC_canvas", "yield_t_MC", 800, 1000);
  yield_t_MC_canvas->DivideSquare(numEnBins,small,small);
  char savePath_yield_t_MC[200];
  TCanvas *thrown_t_canvas = new TCanvas("thrown_t_canvas", "thrown_t", 800, 1000);
  thrown_t_canvas->DivideSquare(numEnBins,small,small);      
  char savePath_thrown_t[200];
  TCanvas *eff_t_canvas = new TCanvas("eff_t_canvas", "eff_t", 800, 1000);
  eff_t_canvas->DivideSquare(numEnBins,small,small);
  char savePath_eff_t[200];
  TCanvas *diff_xsec_canvas = new TCanvas("diff_xsec_canvas", "diff_xsec", 800, 1000);
  diff_xsec_canvas->DivideSquare(numEnBins,small,small);
  char savePath_diff_xsec[200];

  char fit_t_name[100]; char fit_t_MC_name[100];
  char fit_t_title[100]; char fit_t_MC_title[100];
  TCanvas *fit_t_canvas[numEnBins];
  TCanvas *fit_t_canvasMC[numEnBins];
  char savePath_fit_t[numEnBins][300];
  char savePath_fit_t_MC[numEnBins][300];
  char savePath_thrown_t_Ebin[numEnBins][300];  
  char thrown_name[100];
    
  TCanvas *thrown_can[numEnBins];

  //////////////////////////
  // Energy Loop for Xsec //
  //////////////////////////
  for(Int_t iE=0; iE < numEnBins; iE++)
    {
      printf("\n########### Begin Totol XSection Bin %d #############\n", iE+1);
      // Get data yields
      fitCan->cd(iE+1);
      fitCan->SetLeftMargin(0.05);
      fitCan->SetRightMargin(0.05);
      //
      sprintf(name_py, "pyBin%d_ye", iE+1);
      sprintf(title_py," #bf{E_{#gamma} (GeV): (%.2f, %.2f)}; M(#Lambda#pi^{-}) (GeV); Counts/ %.0f MeV", 6.4+(iE)*enBinWidth, 6.4+(iE+1)*enBinWidth, ximBinWidth*pow(10,3));
      Xi_Egamma_t_accsub->GetXaxis()->SetRange(iE+1,iE+1);
      TH1D *Xi_Egamma = (TH1D*) Xi_Egamma_t_accsub->Project3D(name_py);
      Xi_Egamma->SetTitle(title_py);
      hList.Add(Xi_Egamma);
      Int_t signalBinLow = Xi_Egamma->FindBin(1.3), signalBinHigh = Xi_Egamma->FindBin(1.34);
      
      // Possibly do an error matrix condition for fits instead of minimum entries
      sprintf(ws_name, "ws_ebin_%d", iE);
      rooFitHist(Xi_Egamma,title_py, ws_name, &yield[iE], &yield_err[iE]);
      printf("Events in Energy Bin %d: %f +/- %f\n", iE+1, yield[iE], yield_err[iE]);
      //
      yields_Egamma->SetBinContent(iE+1, yield[iE]);
      yields_Egamma->SetBinError(iE+1, yield_err[iE]);
    
      //Get MC yields
      fitCanMC->cd(iE+1);
      fitCanMC->SetLeftMargin(0.05);
      fitCanMC->SetRightMargin(0.05);
      gPad->SetLogy();
      //
      sprintf(name_pyMC, "pyMCBin%d_ye", iE+1);
      sprintf(title_pyMC,"#bf{E_{#gamma} (GeV): (%.2f, %.2f)}}; M(#Lambda#pi^{-}) (GeV); Counts/ %.0f MeV", 6.4+(iE)*enBinWidth, 6.4+(iE+1)*enBinWidth, ximBinWidth*pow(10,3));
      Xi_Egamma_t_MC_accsub->GetXaxis()->SetRange(iE+1,iE+1);
      TH1D *Xi_Egamma_MC =(TH1D*) Xi_Egamma_t_MC_accsub->Project3D(name_pyMC);
      //Xi_Egamma_MC->RebinX();
      Xi_Egamma_MC->SetTitle(title_pyMC);
      hList.Add(Xi_Egamma_MC);

      //
      sprintf(ws_name, "ws_ebin_%d_mc", iE);
      if(dataFile.find("weighted")>dataFile.size())
        rooFitHistMC(Xi_Egamma_MC, title_pyMC, ws_name, &yieldMC[iE], &yieldMC_err[iE]);
      else
        {
          yieldMC[iE] = Xi_Egamma_MC->IntegralAndError(Xi_Egamma_MC->FindBin(1.306),Xi_Egamma_MC->FindBin(1.338), yieldMC_err[iE]);
          //yieldMC[iE] = Xi_Egamma_MC->Integral();
          //yieldMC_err[iE] = sqrt( yieldMC[iE]);
        } 
      printf("Events in MC Energy Bin %d: %f +/- %f\n", iE+1, yieldMC[iE], yieldMC_err[iE]);
      printf("Total Events %d: %f \t PCT Diff: %f\n", iE+1, Xi_Egamma_MC->Integral(), (Xi_Egamma_MC->Integral()-yieldMC[iE])/yieldMC[iE]*100);
      
      // Bin yields
      yields_Egamma_MC->SetBinContent(iE+1, yieldMC[iE]);
      yields_Egamma_MC->SetBinError(iE+1, yieldMC_err[iE]);
      // Bin Flux
      flux_val[iE] = tag_flux->GetBinContent(iE+1); flux_err[iE] = tag_flux->GetBinError(iE+1);
      printf("Photon Flux %d: %f +/- %f\n", iE+1, flux_val[iE], flux_err[iE]);
      yieldThrown[iE] = thrownE->GetBinContent(iE+1); yieldThrown_err[iE] = thrownE->GetBinError(iE+1);
      printf("Events in Thrown Bin %d: %f +/- %f\n", iE+1, yieldThrown[iE], yieldThrown_err[iE]);
      
      // Get efficiency and Xsec
      if(yield[iE] == 0 || yieldMC[iE] == 0 )
        {
          eff_val[iE] = 0; eff_val_err[iE] = 0;
          eff_Egamma->SetBinContent(iE+1, eff_val[iE]);
          eff_Egamma->SetBinError(iE+1, eff_val_err[iE]);
          
          xsec_val[iE] = 0; xsec_val_err[iE] = 0;
          xsec_Egamma->SetBinContent(iE+1, xsec_val[iE]);
          xsec_Egamma->SetBinError(iE+1, xsec_val_err[iE]);
        }
      else
        {
          // get efficiency
          eff_val[iE] = yieldMC[iE]/yieldThrown[iE];
          eff_val_err[iE] = eff_val[iE] * sqrt( pow( yieldMC_err[iE]/yieldMC[iE], 2  ) + pow (yieldThrown_err[iE]/yieldThrown[iE], 2) );
          printf("Efficiency value in Bin %d: %f +/- %f\n", iE+1, eff_val[iE], eff_val_err[iE]); 
          eff_Egamma->SetBinContent(iE+1, eff_val[iE]);
          eff_Egamma->SetBinError(iE+1, eff_val_err[iE]);
          
          // get xsec value
          xsec_val[iE] = yield[iE]/(tar_val * flux_val[iE] * eff_val[iE] * BR_Lamb * BR_Xi);
          xsec_val_err[iE] = xsec_val[iE] * sqrt( pow( yield_err[iE]/yield[iE], 2) + pow( flux_err[iE]/flux_val[iE], 2) + pow( yieldMC_err[iE]/yieldMC[iE], 2  ) + pow (yieldThrown_err[iE]/yieldThrown[iE], 2) );
          printf("XSec value in Bin %d: %f +/- %f (nb)\n\n", iE+1, xsec_val[iE]*pow(10,9), xsec_val_err[iE]*pow(10,9));
          // Bin Xsec
          xsec_Egamma->SetBinContent(iE+1, xsec_val[iE]);
          xsec_Egamma->SetBinError(iE+1, xsec_val_err[iE]);
        }
      
      // make 2d histo in this energy bin
      sprintf(histName, "Xi_Ebin_%0.1f_t",6.4+(iE)*enBinWidth);
      TH3 *Xi_Ebin_t = (TH3*) Xi_Egamma_t_accsub->Clone(histName);
      sprintf(histName, "Xi_Ebin_%0.1f_t_MC",6.4+(iE)*enBinWidth);
      TH3 *Xi_Ebin_t_MC = (TH3*) Xi_Egamma_t_MC_accsub->Clone(histName);
    
      sprintf(fit_t_name, "fit_t_canvas_%0.1f", 6.4+(iE)*enBinWidth);
      sprintf(fit_t_title, "Data fit %.1f > E_{#gamma} > %.1f", 6.4+(iE)*enBinWidth, 6.4+(iE+1)*enBinWidth);
      fit_t_canvas[iE] = new TCanvas(fit_t_name, fit_t_title, 800, 1000);
      fit_t_canvas[iE]->DivideSquare(numtBins,small,small);
      sprintf(fit_t_name, "fit_t_canvasMC_%0.1f", 6.4+(iE)*enBinWidth);
      sprintf(fit_t_title, "MC fit %.1f > E_{#gamma} > %.1f", 6.4+(iE)*enBinWidth, 6.4+(iE+1)*enBinWidth);
      fit_t_canvasMC[iE] = new TCanvas(fit_t_name, fit_t_title, 800, 1000);
      fit_t_canvasMC[iE]->DivideSquare(numtBins,small,small);
      //
      sprintf(histName, "yields_Ebin_%.1f_t", 6.4+(iE)*enBinWidth);
      sprintf(histTitle, "#bf{E_{#gamma} (GeV): (%.2f, %.2f)}; -t (Gev^{2} ); #Xi^{-}  Yield", 6.4+(iE)*enBinWidth, 6.4+(iE+1)*enBinWidth);
      yields_Egamma_t[iE] = new TH1D(histName, histTitle, numtBins, tmin, tmax);
      //
      sprintf(histName, "yields_Ebin_%.1f_t_MC", 6.4+(iE)*enBinWidth);
      sprintf(histTitle, "#bf{E_{#gamma} (GeV): (%.2f, %.2f)}; -t (Gev^{2} ); #Xi^{-}  Reconstructed", 6.4+(iE)*enBinWidth, 6.4+(iE+1)*enBinWidth);
      yields_Egamma_t_MC[iE] = new TH1D(histName, histTitle, numtBins, tmin, tmax);
      //
      sprintf(histName, "thrown_Ebin_%.1f_t", 6.4+(iE)*enBinWidth);
      sprintf(histTitle, "#bf{E_{#gamma} (GeV): (%.2f, %.2f)}; -t (Gev^{2} ); #Xi^{-}  Generated", 6.4+(iE)*enBinWidth, 6.4+(iE+1)*enBinWidth);
      thrown_Egamma_t[iE] = new TH1D(histName, histTitle, numtBins, tmin, tmax);
      //
      sprintf(histTitle, "#bf{E_{#gamma} (GeV): (%.2f, %.2f)}; -t (Gev^{2} ); #epsilon(#gamma p#rightarrow K^{+}K^{+}#Xi^{-}", 6.4+(iE)*enBinWidth, 6.4+(iE+1)*enBinWidth); 
      sprintf(histName, "eff_Ebin_%.1f_t", 6.4+(iE)*enBinWidth);
      eff_Egamma_t[iE] = new TH1D(histName, histTitle, numtBins, tmin, tmax);

      sprintf(histTitle, "#bf{E_{#gamma} (GeV): (%.2f, %.2f)} ;  -t (Gev^{2} ) ; #frac{d#sigma}{dt} (nb/GeV^2 )", 6.4+(iE)*enBinWidth, 6.4+(iE+1)*enBinWidth);
      sprintf(histName, "diff_xsec_Ebin_%.1f_t", 6.4+(iE)*enBinWidth);
      diff_xsec[iE] = new TH1D(histName, histTitle, numtBins, tmin, tmax);

      //File to save diff xsec
      FILE *diff_xsecf = fopen((datadir+"Diff_"+dataFile+"_enBin"+to_string(iE)+".txt").c_str(),"w+");
      /******************************
       BEGIN DIFF XSEC FOR ENERGY BIN
      *******************************/
      for(Int_t it=0; it < numtBins; it++)
      	{
          printf("\n######### Differntial XSection t Bin %d #########\n", it+1); 
      	  fit_t_canvas[iE]->cd(it+1);
          fit_t_canvas[iE]->SetLeftMargin(0.05);
          fit_t_canvas[iE]->SetRightMargin(0.05);
          //gPad->SetLogy();
          //get yields of all tBins
      	  //set tBin
          Xi_Ebin_t->GetZaxis()->SetRange(it+1,it+1);
          sprintf(histName,"Xi_ebin_%f_tbin_%f_ye", 6.4+(iE*enBinWidth),it*tBinWidth);
          TH1D *Xi_Ebin_tbin = (TH1D*) Xi_Ebin_t->Project3D(histName);
          Xi_Ebin_tbin->RebinX();
          sprintf(histTitle, " -t (GeV^{2} ): (%0.2f, %0.2f); M(#Lambda#pi^{-}) GeV; Counts/ %.0f MeV",(it)*tBinWidth,(it+1)*tBinWidth, Xi_Ebin_tbin->GetXaxis()->GetBinWidth(1)*pow(10,3));
          Xi_Ebin_tbin->SetTitle(histTitle);
	  
      	  Double_t ymaxt = Xi_Ebin_tbin->GetMaximum();
      	  Int_t binMaxt = Xi_Ebin_tbin->GetMaximumBin();
      	  Double_t xcentert = Xi_Ebin_tbin->GetXaxis()->GetBinCenter(binMaxt);
          
          sprintf(ws_name, "ws_ebin_%d_tbin_%d", iE+1,it+1);
          rooFitHist(Xi_Ebin_tbin, histTitle, ws_name, &yield_diff[it], &yield_diff_err[it]);
          printf("Events in t Bin %d: %f +/- %f\n", it+1, yield_diff[it], yield_diff_err[it]);
          
          yields_Egamma_t[iE]->SetBinContent(it+1, yield_diff[it]);
          yields_Egamma_t[iE]->SetBinError(it+1, yield_diff_err[it]);
	  
          // diffxsec Monte Carlo	
          // get yields of all tBins
          // set tBin
          fit_t_canvasMC[iE]->cd(it+1);
          fit_t_canvasMC[iE]->SetLeftMargin(0.05);
          fit_t_canvasMC[iE]->SetRightMargin(0.05);
          gPad->SetLogy();
          //    
          Xi_Ebin_t_MC->GetZaxis()->SetRange(it+1,it+1);
          sprintf(histName,"XiMC_ebin_%f_tbin_%f_ye", 6.4+(iE*enBinWidth),it*tBinWidth);
          sprintf(histTitle, "-t (GeV^{2} ): (%0.2f, %0.2f); M(#Lambda#pi^{-}) GeV; Counts/ %.0f MeV",(it)*tBinWidth,(it+1)*tBinWidth, ximBinWidth*pow(10,3));
          TH1D *Xi_Ebin_tbin_MC = (TH1D*) Xi_Ebin_t_MC->Project3D(histName);
          Xi_Ebin_tbin_MC->RebinX();
          Xi_Ebin_tbin_MC->SetTitle(histTitle);
          //
          sprintf(ws_name, "ws_ebin_%d_tbin_%d_mc", iE+1, it+1);
          if(dataFile.find("weighted")>dataFile.size())
            rooFitHistMC(Xi_Ebin_tbin_MC, histTitle, ws_name, &yield_diffMC[it], &yield_diffMC_err[it]);
          else
            {
              yield_diffMC[it] = Xi_Ebin_tbin_MC->IntegralAndError(Xi_Ebin_tbin_MC->FindBin(1.306),Xi_Ebin_tbin_MC->FindBin(1.338), yield_diffMC_err[it]);
              //yield_diffMC[it] = Xi_Ebin_tbin_MC->GetEntries();
              //yield_diffMC_err[it] = sqrt(yield_diffMC[it]);
            }
          printf("MC Events in t Bin %d: %f +/- %f\n", it+1, yield_diffMC[it], yield_diffMC_err[it]);
	  
          yields_Egamma_t_MC[iE]->SetBinContent(it+1, yield_diffMC[it]);
          yields_Egamma_t_MC[iE]->SetBinError(it+1, yield_diffMC_err[it]);
	  
          // Diff Xsec efficiency	  
          yield_diffThrown[it] = thrownT->GetBinContent(iE+1, it+1);
          yield_diffThrown_err[it] = thrownT->GetBinError(iE+1, it+1);
          printf("Events in Thrown t Bin %d: %f +/- %f\n", it+1, yield_diffThrown[it], yield_diffThrown_err[it]);
	  
          thrown_Egamma_t[iE]->SetBinContent(it+1, yield_diffThrown[it]);
          thrown_Egamma_t[iE]->SetBinError(it+1, yield_diffThrown_err[it]);
	  
          if(yield_diffMC[it] == 0 || yield_diffThrown[it] == 0)
            {
              eff_diff_val[it] = 0;
              eff_diff_val_err[it] = 0;
              eff_Egamma_t[iE]->SetBinContent(it+1, eff_diff_val[it]);
              eff_Egamma_t[iE]->SetBinError(it+1, eff_diff_val_err[it]);
	      
              diff_xsec_val[it] = 0;
              diff_xsec_val_err[it] = 0;
              diff_xsec[iE]->SetBinContent(it+1, diff_xsec_val[it]);
              diff_xsec[iE]->SetBinError(it+1, diff_xsec_val_err[it]);
            }
          else
            {
              // get diff xsec efficiency
              eff_diff_val[it] = yield_diffMC[it]/yield_diffThrown[it];
              eff_diff_val_err[it] = eff_diff_val[it] * sqrt( pow( yield_diffMC_err[it]/yield_diffMC[it], 2  ) + pow (yield_diffThrown_err[it]/yield_diffThrown[it], 2) );
              printf("Diff_Efficiency value in t Bin %d: %f +/- %f\n", it+1, eff_diff_val[it], eff_diff_val_err[it]); 
              eff_Egamma_t[iE]->SetBinContent(it+1, eff_diff_val[it]);
              eff_Egamma_t[iE]->SetBinError(it+1, eff_diff_val_err[it]);
    
              // get diff_xsec value
              diff_xsec_val[it] = yield_diff[it]/(tar_val * flux_val[iE] * eff_diff_val[it] * BR_Lamb * BR_Xi * tBinWidth);
              if(diff_xsec_val[it] == 0)    diff_xsec_val_err[it] = 0;
              else
                diff_xsec_val_err[it] = diff_xsec_val[it] * sqrt( pow( yield_diff_err[it]/yield_diff[it], 2) + pow( flux_err[iE]/flux_val[iE], 2) + pow( yield_diffMC_err[it]/yield_diffMC[it], 2  ) + pow (yield_diffThrown_err[it]/yield_diffThrown[it], 2) );
              printf("Diff_Xsec value in t Bin %d: %f +/- %f (nb)\n\n", it+1, diff_xsec_val[it]*pow(10,9), diff_xsec_val_err[it]*pow(10,9));
	  
              diff_xsec[iE]->SetBinContent(it+1, diff_xsec_val[it]);
              diff_xsec[iE]->SetBinError(it+1, diff_xsec_val_err[it]);
            }
          //Save differential cross section to file
          fprintf(diff_xsecf, "%f %f %f %f\n", diff_xsec[iE]->GetBinCenter(it+1), diff_xsec_val[it]*TMath::Power(10,9), diff_xsec[iE]->GetBinWidth(it)/2, diff_xsec_val_err[it]*TMath::Power(10,9));
          
        } //end t loop
      
      //Get weighted integrated cross section
      int_xsec_val[iE] = diff_xsec[iE]->IntegralAndError(1, numtBins, int_xsec_val_err[iE], "width");
      fprintf(integrated_xsecf, "%f %f %f %f\n", xsec_Egamma->GetBinCenter(iE+1), int_xsec_val[iE]*TMath::Power(10,9), xsec_Egamma->GetBinWidth(iE+1)/2,int_xsec_val_err[iE]*TMath::Power(10,9));
      int_xsec_Egamma->SetBinContent(iE+1, int_xsec_val[iE]);
      int_xsec_Egamma->SetBinError(iE+1, int_xsec_val_err[iE]);
      
      // Save fits to binned data
      setStyle();
      sprintf(savePath_fit_t[iE], "%sfit_t_Ebin_%f_%s.pdf", fitsdir.c_str(), 6.4+iE*enBinWidth, dataFile.c_str());
      fit_t_canvas[iE]->SaveAs(savePath_fit_t[iE]);
      sprintf(savePath_fit_t[iE], "%sfit_t_Ebin_%f_%s_MC.pdf", fitsdir.c_str(), 6.4+iE*enBinWidth, dataFile.c_str());
      fit_t_canvasMC[iE]->SaveAs(savePath_fit_t[iE]);

      //plot diffXsec and components
      yield_t_canvas->cd(iE+1);
      yield_t_canvas->SetLeftMargin(0.05);
      yield_t_canvas->SetRightMargin(0.05);
      yields_Egamma_t[iE]->Draw("E1");      
      hList.Add(yields_Egamma_t[iE]);
      //
      yield_t_MC_canvas->cd(iE+1);
      yield_t_MC_canvas->SetLeftMargin(0.05);
      yield_t_MC_canvas->SetRightMargin(0.05);
      yields_Egamma_t_MC[iE]->GetYaxis()->SetMaxDigits(3);
      yields_Egamma_t_MC[iE]->Draw("E1");
      hList.Add(yields_Egamma_t_MC[iE]);
      //
      thrown_t_canvas->cd(iE+1);
      thrown_t_canvas->SetLeftMargin(0.05);
      thrown_t_canvas->SetRightMargin(0.05);
      thrown_Egamma_t[iE]->GetYaxis()->SetMaxDigits(3);
      thrown_Egamma_t[iE]->Draw("E1");
      hList.Add(thrown_Egamma_t[iE]);
      //
      eff_t_canvas->cd(iE+1);
      eff_t_canvas->SetLeftMargin(0.05);
      eff_t_canvas->SetRightMargin(0.05);
      eff_Egamma_t[iE]->SetMinimum(0);
      eff_Egamma_t[iE]->Draw("E1");
      hList.Add(eff_Egamma_t[iE]);
      //
      diff_xsec_canvas->cd(iE+1);
      gPad->SetLogy();
      diff_xsec[iE]->Scale(pow(10,9));
      diff_xsec[iE]->Draw("E1");
      hList.Add(diff_xsec[iE]);

    }//end energy loop  
  
     // TCanvas *cc = new TCanvas("cc", ""); 
  // TH1D *histBlank = new TH1D("blank", "", 13, 0.0, 6.4);
  // TList *list = new TList;
  // list->Add(histBlank);
  // list->Add(xsec_Egamma);
  // TH1D *histMerge = (TH1D*)histBlank->Clone("xsec_Egamma_merged");
  // histMerge->Reset();
  // histMerge->Merge(list);
  // histMerge->Draw();
  
  string diffCanSave = plotdir+"diff_"+dataFile+".pdf";
  diff_xsec_canvas->SaveAs(diffCanSave.c_str());
  
  hList.Add(yields_Egamma);
  hList.Add(yields_Egamma_MC);
  hList.Add(eff_Egamma);
  
  xsec_Egamma->Scale(pow(10,9));
  int_xsec_Egamma->Scale(pow(10,9));
  hList.Add(xsec_Egamma);
  hList.Add(int_xsec_Egamma);
  
  string root_name = datadir+"diff_"+dataFile+".root";
  TFile *froot = new TFile(root_name.c_str(), "RECREATE");
  hList.Write();
  froot->Close();
  
  string xsecFileName = datadir + "Total_"+ dataFile +".txt";
  ofstream xsecFile(xsecFileName.c_str());
  
  for(int i = 0; i < numEnBins; i++)
    {
      double xval = xsec_Egamma->GetBinCenter(i+1);
      double xval_err = xsec_Egamma->GetBinWidth(i+1)/2; 
      double yval = xsec_Egamma->GetBinContent(i+1);
      double yval_err = xsec_Egamma->GetBinError(i+1);
      xsecFile << xval << "\t"  << yval << "\t" << xval_err << "\t" << yval_err << "\n" ; 
    }
  
  xsecFile.close();
  fclose(integrated_xsecf);
  fclose(total_xsecf);
  
  //////////////////////////////
  // Draw and save histograms //
  //////////////////////////////
  string fitCanName = fitsdir+"data_proj_"+dataFile+".pdf";
  string fitCanNameMC = fitsdir+"mc_proj_"+dataFile+".pdf";
  fitCan->SaveAs(fitCanName.c_str());
  fitCanMC->SaveAs(fitCanNameMC.c_str());

  sprintf(savePath_yield_t, "%syield_Egamma_t_%s.pdf", plotdir.c_str(), dataFile.c_str()); 
  sprintf(savePath_yield_t_MC, "%syield_Egamma_t_%s_MC.pdf", plotdir.c_str(), dataFile.c_str()); 
  sprintf(savePath_thrown_t, "%sthrown_Egamma_t_%s.pdf", plotdir.c_str(), dataFile.c_str()); 
  sprintf(savePath_eff_t, "%seff_Egamma_t_%s.pdf", plotdir.c_str(), dataFile.c_str()); 
  sprintf(savePath_diff_xsec, "%sdiff_xsec_Egamma_t_%s_MC.pdf", plotdir.c_str(), dataFile.c_str()); 
  yield_t_canvas->SaveAs(savePath_yield_t); 
  yield_t_MC_canvas->SaveAs(savePath_yield_t_MC);
  thrown_t_canvas->SaveAs(savePath_thrown_t);
  eff_t_canvas->SaveAs(savePath_eff_t);
  diff_xsec_canvas->SaveAs(savePath_diff_xsec);
 
  //Draw total xsec
  TCanvas *c3 = new TCanvas("c3", "Yield Plots", 1000, 600);
  c3->Divide(2,1,1e-4,1e-4);
  c3->cd(1);
  yields_Egamma->SetMarkerStyle(20);
  yields_Egamma->GetYaxis()->SetMaxDigits(3);
  //gPad->SetLeftMargin(1.1);
  yields_Egamma->Draw("E1");
  c3->cd(2);
  yields_Egamma_MC->SetMarkerStyle(20);
  yields_Egamma_MC->GetYaxis()->SetMaxDigits(3);
  yields_Egamma_MC->Draw("E1");
  c3->SaveAs( (plotdir + "yield_Egamma_data_MC_" + dataFile + ".pdf").c_str() );

  TCanvas *c4 = new TCanvas("c4", "Eff/Xsec", 1000, 600);
  c4->Divide(2,1);
  c4->cd(1);
  eff_Egamma->SetMarkerStyle(20);
  eff_Egamma->Scale(100);
  //eff_Egamma->SetMinimum(0);
  eff_Egamma->GetYaxis()->SetTitle("% Efficiency");
  eff_Egamma->SetMinimum(0);
  eff_Egamma->Draw("E1");
  c4->cd(2);
  xsec_Egamma->SetMarkerStyle(20);
  xsec_Egamma->SetMinimum(0);
  xsec_Egamma->Draw("E1");
  //int_xsec_Egamma->SetMarkerStyle(21);
  //int_xsec_Egamma->Draw("e1 same");
  //c4->SaveAs( (plotdir.c_str() + "eff_xsec_Egamma_data_MC_" + dataFile + ".pdf").c_str() );

  string eff_xsec_save = plotdir+"eff_"+dataFile+".pdf";
  printf("%s", eff_xsec_save.c_str());
  c4->SaveAs(eff_xsec_save.c_str());

  TCanvas *c5 = new TCanvas("c5", "#Xi(IM) vs Egamma", 1200,750);
  c5->Divide(2,1);
  c5->cd(1);
  thrownE->SetMarkerStyle(20);
  thrownE->Draw("E1");
  c5->cd(2);
  tag_flux->SetMarkerStyle(20);
  tag_flux->Draw("E1");
}

void plotDiffXSec(TH1D* hist1[], TH1D* hist2[], TH1D* hist3[], double xmax, double ymax, const char* saveName)
{
    
  //Initiate variables
  char savePath[200];
  double numBins = hist1[0]->GetNbinsX();
  cout << "Num Bins in plotting: " << numBins << endl;
  double small = 1e-5;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection", 30, 10, 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);

  C->Draw();
  C->Divide(4, 3, small, small);

  //rpad->Draw();
  //rpad->Divide(1,4,small,small);
  
  //setStyle();
  for (int j = 0; j < 10; j++)
    {
      C->cd(j+1);
       
      // Set Pad style for plots
      gPad->SetLogy();
      //gPad->SetGrid();
      gPad->SetFillStyle(0);
      gPad->SetFrameFillStyle(0);
      gPad->SetTopMargin(small);
      gPad->SetBottomMargin(small);
      gPad->SetRightMargin(small);
      gPad->SetLeftMargin(small);    
      gStyle->SetOptStat(0);

      TLatex* latex = new TLatex();
      latex->SetNDC();
      //latex->SetTextFont(42);
      latex->SetTextSize(0.04);
      latex->SetTextAlign(32);

      // Set histograms and draw on Pad
      hist1[j]->SetMaximum(ymax);
      hist1[j]->SetMinimum(0.1);
      hist1[j]->SetMarkerColor(kBlack);
      hist1[j]->SetLineColor(3);
      hist1[j]->SetMarkerStyle(20);
      hist1[j]->SetMarkerSize(0.5);
      //hist1[j]->GetYaxis()->SetNdivisions(4);
 
      hist2[j]->SetMaximum(ymax);
      hist2[j]->SetMinimum(0.1);
      hist2[j]->SetMarkerColor(kBlack);
      hist2[j]->SetLineColor(2);
      hist2[j]->SetMarkerStyle(20);
      hist2[j]->SetMarkerSize(0.5);
      
      hist3[j]->SetMaximum(ymax);
      hist3[j]->SetMinimum(0.1);
      hist3[j]->SetMarkerColor(kBlack);
      hist3[j]->SetLineColor(4);
      hist3[j]->SetMarkerStyle(20);
      hist3[j]->SetMarkerSize(0.5);
      
      // Plot all data sets
      hist1[j]->Draw("E1");
      hist2[j]->Draw("E1same");
      hist3[j]->Draw("E1same");
      
    }
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
      yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, 0.11, ymax, 4, "G");
      yax->SetLabelSize(0.04);
      //yax->SetLabelOffset(-0.01);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 2; i < 4; i++)
    {
      xax = new TGaxis(0.95-.85/4*(i+1), 0.1, 0.95-.85/4*(i), 0.1, 2*small, xmax, 4, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  for ( int i = 0; i < 2; i++)
    {
      xax = new TGaxis(0.95-.85/4*(i+1), 0.95-.85/3*(2), 0.95-.85/4*(i), 0.95-.85/3*(2), 2*small, xmax, 4, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  // Draw plot axis labels
  xTitle.SetTextFont(132);
  xTitle.SetTextSize(0.06);
  xTitle.DrawLatex(0.37, 0.01, "-t (GeV^{2} )");
  
  yTitle.SetTextFont(132);
  yTitle.SetTextSize(0.06);
  yTitle.SetTextAngle(90);
  yTitle.DrawLatex(0.05, 0.55, "d#sigma/dt (nb/GeV^{2} )");
  
  gStyle->SetPadBottomMargin(0.2);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.04);
  gStyle->SetLabelOffset(0.03,"Y");
 
  // Add Legend
  auto legend = new TLegend(0.53,0.16,0.73,0.3);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  legend->AddEntry(hist1[1],"Spring 2017","lep");
  legend->AddEntry(hist2[1],"Spring 2018","lep");
  legend->AddEntry(hist3[1],"Fall 2018","lep");
  //legend->AddEntry(diff_xsec_weighted[1],"GlueX-I","lep");
  //legend->AddEntry(diff_xsec_weighted[1],"Spring 2018 GlueX-I","lep");
  legend->Draw();
  
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_201808set.pdf");
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_allSets+weighted.pdf");
  sprintf(savePath, "/d/grid17/hjesse/AnalysisNote/xsection/plots/%s.pdf", saveName);
  tC->SaveAs(savePath);
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_2017_201808.pdf");
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_weighted.pdf");
}

void plotAllXSec(TH1D* hist1[], TH1D* hist2[], TH1D* hist3[], TH1D* hist4[], double xmax, double ymax, const char* saveName)
{ 
  //Initiate variables
  char savePath[200];
  double small = 1e-5;
  double numBins = hist1[0]->GetNbinsX();
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection", 30, 10, 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);

  C->Draw();
  C->Divide(4, 3, small, small);

  //rpad->Draw();
  //rpad->Divide(1,4,small,small);
  
  //setStyle();
  for (int j = 0; j < numBins; j++)
    {
      C->cd(j+1);
       
      // Set Pad style for plots
      gPad->SetLogy();
      //gPad->SetGrid();
      gPad->SetFillStyle(0);
      gPad->SetFrameFillStyle(0);
      gPad->SetTopMargin(small);
      gPad->SetBottomMargin(small);
      gPad->SetRightMargin(small);
      gPad->SetLeftMargin(small);    
      gStyle->SetOptStat(0);

      TLatex* latex = new TLatex();
      latex->SetNDC();
      //latex->SetTextFont(42);
      latex->SetTextSize(0.04);
      latex->SetTextAlign(32);

      // Set histograms and draw on Pad
      hist1[j]->SetMaximum(ymax);
      hist1[j]->SetMinimum(0.1);
      hist1[j]->SetMarkerColor(kBlack);
      hist1[j]->SetLineColor(3);
      hist1[j]->SetMarkerStyle(20);
      hist1[j]->SetMarkerSize(0.5);
      //hist1[j]->GetYaxis()->SetNdivisions(4);
 
      hist2[j]->SetMaximum(ymax);
      hist2[j]->SetMinimum(0.1);
      hist2[j]->SetMarkerColor(kBlack);
      hist2[j]->SetLineColor(2);
      hist2[j]->SetMarkerStyle(20);
      hist2[j]->SetMarkerSize(0.5);
      
      hist3[j]->SetMaximum(ymax);
      hist3[j]->SetMinimum(0.1);
      hist3[j]->SetMarkerColor(kBlack);
      hist3[j]->SetLineColor(4);
      hist3[j]->SetMarkerStyle(20);
      hist3[j]->SetMarkerSize(0.5);
      
      hist4[j]->SetMaximum(ymax);
      hist4[j]->SetMinimum(0.1);
      hist4[j]->SetMarkerColor(kBlack);
      hist4[j]->SetLineColor(kMagenta-4);
      hist4[j]->SetMarkerStyle(20);
      hist4[j]->SetMarkerSize(0.5);

      // Plot all data sets
      hist1[j]->Draw("E1");
      hist2[j]->Draw("E1same");
      hist3[j]->Draw("E1same");
      hist4[j]->Draw("E1same");
    }
  
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 4; i++)
    {
      yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, 0.1+small, ymax, 4, "G");
      yax->SetLabelSize(0.04);
      //yax->SetLabelOffset(-0.04);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 2; i < 4; i++)
    {
      xax = new TGaxis(0.95-.85/4*(i+1), 0.1, 0.95-.85/4*(i), 0.1, 2*small, xmax, 4, "");
      xax->SetLabelSize(0.04);
      //yax->SetLabelOffset(-0.04);
      xax->Draw("same");
    }
  
  // Draw plot axis labels
  xTitle.SetTextFont(132);
  xTitle.SetTextSize(0.065);
  xTitle.DrawLatex(0.4, 0.01, "-t (GeV^{2})");
  
  yTitle.SetTextFont(132);
  yTitle.SetTextSize(0.065);
  yTitle.SetTextAngle(90);
  yTitle.DrawLatex(0.04, 0.55, "d#sigma/dt (nb/GeV^{2})");
  
  // Add Legend
  auto legend = new TLegend(0.52,0.155,0.7,0.33);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  legend->AddEntry(hist1[1],"2017-01 ANA ver45","lep");
  legend->AddEntry(hist2[1],"2018-01 ANA ver03","lep");
  legend->AddEntry(hist3[1],"2018-08 ANA ver02","lep");
  legend->AddEntry(hist4[1],"2019-11 ANA batch04-12","lep");
  //legend->AddEntry(diff_xsec_weighted[1],"GlueX-I","lep");
  //legend->AddEntry(diff_xsec_weighted[1],"Spring 2018 GlueX Phase-I","lep");
  legend->Draw();
  
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_201808set.pdf");
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_allSets+weighted.pdf");
  sprintf(savePath, "/d/grid17/hjesse/macros/xsection/%s.pdf", saveName);
  tC->SaveAs(savePath);
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_2017_201808.pdf");
  //tC->SaveAs("/d/grid17/hjesse/analysis/plots/diff_xsec_weighted.pdf");
}

void getWeightedAvgHist(TH1D* hist1[], TH1D* hist2[], TH1D* hist3[], TH1D* weighted_hist[], Double_t tmax, Double_t twidth, string delim )
{ 
  Int_t numtBins = tmax/twidth;
  Int_t enBins = 10;//hist1[0]->GetNbinsX();
  Double_t enBinWidth = 0.5; hist1[0]->GetBinWidth(1);
  double diff_xsec_weighted_val[enBins][numtBins];
  double diff_xsec_weighted_err[enBins][numtBins];
  double val_2017[enBins][numtBins];
  double val_201801[enBins][numtBins];
  double val_201808[enBins][numtBins];
  double err_2017[enBins][numtBins];
  double err_201801[enBins][numtBins];
  double err_201808[enBins][numtBins];
  char histName[50];
  char histTitle[100];
  string dataDir = "/d/grid17/hjesse/AnalysisNote/xsection/data_files/";
  FILE *xsecf = fopen((dataDir+"intxsec_avgw"+delim+".txt").c_str(),"w+");
  double xsec_val[enBins];
  double xsec_err[enBins];
  
  for(int iE = 0; iE < enBins; iE++){
    sprintf(histTitle, "#bf{E_{#gamma} (GeV): (%.1f, %.1f)} ;  -t (GeV^{2} ) ; #frac{d#sigma}{dt} (nb/GeV^{2} )", 6.4+(iE)*0.5, 6.4+(iE+1)*0.5);
    sprintf(histName, "diff_xsec_weighted_Ebin_%.1f_t", 6.4+(iE)*enBinWidth);
    weighted_hist[iE] = new TH1D(histName, histTitle ,numtBins, 0.0, tmax);
    FILE *diff_xsecf = fopen((dataDir+"diff_xsec_avgw"+delim+"_enBin"+to_string(iE)+".txt").c_str(),"w+");
    
    for(int it = 0; it < numtBins; it++){
      val_2017[iE][it] = hist1[iE]->GetBinContent(it+1);
      err_2017[iE][it] = hist1[iE]->GetBinError(it+1);
      val_201801[iE][it] = hist2[iE]->GetBinContent(it+1);
      err_201801[iE][it] = hist2[iE]->GetBinError(it+1);
      val_201808[iE][it] = hist3[iE]->GetBinContent(it+1);
      err_201808[iE][it] = hist3[iE]->GetBinError(it+1);
    
      if(val_2017[iE][it] == 0 && val_201801[iE][it] != 0 && val_201808[iE][it] != 0)
        {
          diff_xsec_weighted_val[iE][it] = ( val_201801[iE][it]*pow( 1/err_201801[iE][it], 2 ) + val_201808[iE][it]*pow( 1/err_201808[iE][it], 2 ) ) / ( pow( 1/err_201801[iE][it], 2 ) + pow( 1/err_201808[iE][it], 2 ) );
          diff_xsec_weighted_err[iE][it] =  1 / sqrt( pow( 1/err_201801[iE][it], 2) +  pow( 1/err_201808[iE][it], 2 ) );
        }
      else if(val_2017[iE][it] != 0 && val_201801[iE][it] == 0 && val_201808[iE][it] != 0)
        {
          diff_xsec_weighted_val[iE][it] = ( val_2017[iE][it]*pow( 1/err_2017[iE][it], 2 ) + val_201808[iE][it]*pow( 1/err_201808[iE][it], 2 ) ) / ( pow( 1/err_2017[iE][it], 2 ) + pow( 1/err_201808[iE][it], 2 ) );
          diff_xsec_weighted_err[iE][it] =  1 / sqrt( pow( 1/err_2017[iE][it], 2) +  pow( 1/err_201808[iE][it], 2 ) );
        }
      else if(val_2017[iE][it] != 0 && val_201801[iE][it] != 0 && val_201808[iE][it] == 0)
        {
          diff_xsec_weighted_val[iE][it] = ( val_201801[iE][it]*pow( 1/err_201801[iE][it], 2 ) + val_2017[iE][it]*pow( 1/err_2017[iE][it], 2 ) ) / ( pow( 1/err_201801[iE][it], 2 ) + pow( 1/err_2017[iE][it], 2 ) );
          diff_xsec_weighted_err[iE][it] =  1 / sqrt( pow( 1/err_201801[iE][it], 2) +  pow( 1/err_2017[iE][it], 2 ) );
        }
      else if(val_2017[iE][it] == 0 && val_201801[iE][it] == 0 && val_201808[iE][it] != 0)
        {
          diff_xsec_weighted_val[iE][it] = val_201808[iE][it];
          diff_xsec_weighted_err[iE][it] = err_201808[iE][it];
        }
      else if(val_2017[iE][it] == 0 && val_201801[iE][it] != 0 && val_201808[iE][it] == 0)
        {
          diff_xsec_weighted_val[iE][it] = val_201801[iE][it];
          diff_xsec_weighted_err[iE][it] = err_201801[iE][it];
        }
      else if( val_2017[iE][it] == 0 && val_201801[iE][it] == 0 && val_201808[iE][it] == 0 ) 
        {
          diff_xsec_weighted_val[iE][it] = 0;
          diff_xsec_weighted_err[iE][it] = 0;
        }
      else
        {
          diff_xsec_weighted_val[iE][it] = ( val_2017[iE][it]*pow( 1/err_2017[iE][it], 2 ) + val_201801[iE][it]*pow( 1/err_201801[iE][it], 2 ) + val_201808[iE][it]*pow( 1/err_201808[iE][it], 2 ) ) / ( pow( 1/err_2017[iE][it], 2 ) + pow( 1/err_201801[iE][it], 2 ) + pow( 1/err_201808[iE][it], 2 ) ); 
          diff_xsec_weighted_err[iE][it] =  1 / sqrt( pow( 1/err_2017[iE][it], 2) + pow( 1/err_201801[iE][it], 2) +  pow( 1/err_201808[iE][it], 2 ) );
        }
      
      printf("################## Differential XSec Weighted Values ################# \n");
      printf("%f +/- %f\n", diff_xsec_weighted_val[iE][it], diff_xsec_weighted_err[iE][it]);
      
      weighted_hist[iE]->SetBinContent(it+1, diff_xsec_weighted_val[iE][it]);
      weighted_hist[iE]->SetBinError(it+1, diff_xsec_weighted_err[iE][it]);

      //Save differential cross section to file
      fprintf(diff_xsecf, "%f %f %f %f\n", weighted_hist[iE]->GetBinCenter(it+1), diff_xsec_weighted_val[iE][it], weighted_hist[iE]->GetBinWidth(it+1)/2, diff_xsec_weighted_err[iE][it]);
    }//end t bin
    //Get weighted integrated cross section
    xsec_err[iE] = 0;
    xsec_val[iE] = weighted_hist[iE]->IntegralAndError(1, numtBins, xsec_err[iE], "width");
    fprintf(xsecf, "%f %f 0.25 %f\n", 6.65+(iE)*enBinWidth, xsec_val[iE], xsec_err[iE]);
    printf("################## Total XSec Weighted Values ################# \n");
    printf("%f +/- \n",  weighted_hist[iE]->IntegralAndError(1, numtBins, xsec_err[iE]));
    printf("%f +/- %f\n", xsec_val[iE], xsec_err[iE]);
  }//end energy bin

}

void getTotWeightedAvgHist(TH1D* hist1, TH1D* hist2, TH1D* hist3, TH1D* weighted_hist)
{ 
  Int_t numEnBins = hist1->GetNbinsX();
  Double_t enMax = hist1->GetBinLowEdge(numEnBins) + hist1->GetBinWidth(1);
  double totxsec_weighted_val[numEnBins];
  double totxsec_weighted_err[numEnBins];
  double val_2017[numEnBins];
  double val_201801[numEnBins];
  double val_201808[numEnBins];
  double err_2017[numEnBins];
  double err_201801[numEnBins];
  double err_201808[numEnBins];
  string histName = "xim_totxsec_weighted";
  string histTitle = " ; E_{#gamma} (GeV) ; #frac{d#sigma}{dt} (nb/GeV^{2} )";
  FILE *xsecf = fopen("/d/grid17/hjesse/AnalysisNote/xsection/data_files/weighted_totxsec_data.txt","w+");
  double xsec_val[numEnBins];
  double xsec_err[numEnBins];
  
  for(int iE = 0; iE < numEnBins; iE++){
    //
    weighted_hist = new TH1D(histName.c_str(), histTitle.c_str() ,numEnBins, 0.0, enMax);
    
    val_2017[iE] = hist1->GetBinContent(iE+1);
    err_2017[iE] = hist1->GetBinError(iE+1);
    val_201801[iE] = hist2->GetBinContent(iE+1);
    err_201801[iE] = hist2->GetBinError(iE+1);
    val_201808[iE] = hist3->GetBinContent(iE+1);
    err_201808[iE] = hist3->GetBinError(iE+1);
    
    if(val_2017[iE] == 0 && val_201801[iE] != 0 && val_201808[iE] != 0)
      {
        totxsec_weighted_val[iE] = ( val_201801[iE]*pow( 1/err_201801[iE], 2 ) + val_201808[iE]*pow( 1/err_201808[iE], 2 ) ) / ( pow( 1/err_201801[iE], 2 ) + pow( 1/err_201808[iE], 2 ) );
        totxsec_weighted_err[iE] =  1 / sqrt( pow( 1/err_201801[iE], 2) +  pow( 1/err_201808[iE], 2 ) );
      }
    else if(val_2017[iE] != 0 && val_201801[iE] == 0 && val_201808[iE] != 0)
      {
        totxsec_weighted_val[iE] = ( val_2017[iE]*pow( 1/err_2017[iE], 2 ) + val_201808[iE]*pow( 1/err_201808[iE], 2 ) ) / ( pow( 1/err_2017[iE], 2 ) + pow( 1/err_201808[iE], 2 ) );
          totxsec_weighted_err[iE] =  1 / sqrt( pow( 1/err_2017[iE], 2) +  pow( 1/err_201808[iE], 2 ) );
      }
    else if(val_2017[iE] != 0 && val_201801[iE] != 0 && val_201808[iE] == 0)
      {
        totxsec_weighted_val[iE] = ( val_201801[iE]*pow( 1/err_201801[iE], 2 ) + val_2017[iE]*pow( 1/err_2017[iE], 2 ) ) / ( pow( 1/err_201801[iE], 2 ) + pow( 1/err_2017[iE], 2 ) );
        totxsec_weighted_err[iE] =  1 / sqrt( pow( 1/err_201801[iE], 2) +  pow( 1/err_2017[iE], 2 ) );
      }
    else if(val_2017[iE] == 0 && val_201801[iE] == 0 && val_201808[iE] != 0)
      {
        totxsec_weighted_val[iE] = val_201808[iE];
        totxsec_weighted_err[iE] = err_201808[iE];
      }
      else if(val_2017[iE] == 0 && val_201801[iE] != 0 && val_201808[iE] == 0)
        {
          totxsec_weighted_val[iE] = val_201801[iE];
          totxsec_weighted_err[iE] = err_201801[iE];
        }
      else if( val_2017[iE] == 0 && val_201801[iE] == 0 && val_201808[iE] == 0 ) 
        {
          totxsec_weighted_val[iE] = 0;
          totxsec_weighted_err[iE] = 0;
        }
      else
        {
          totxsec_weighted_val[iE] = ( val_2017[iE]*pow( 1/err_2017[iE], 2 ) + val_201801[iE]*pow( 1/err_201801[iE], 2 ) + val_201808[iE]*pow( 1/err_201808[iE], 2 ) ) / ( pow( 1/err_2017[iE], 2 ) + pow( 1/err_201801[iE], 2 ) + pow( 1/err_201808[iE], 2 ) ); 
          totxsec_weighted_err[iE] =  1 / sqrt( pow( 1/err_2017[iE], 2) + pow( 1/err_201801[iE], 2) +  pow( 1/err_201808[iE], 2 ) );
        }

    //Get weighted cross section
    printf("################## Total XSec Weighted Values ################# \n");
    printf("%f +/- %f\n", totxsec_weighted_val[iE], totxsec_weighted_err[iE]);
    
    weighted_hist->SetBinContent(iE+1, totxsec_weighted_val[iE]);
    weighted_hist->SetBinError(iE+1, totxsec_weighted_err[iE]);
    fprintf(xsecf, "%f %f 0.25 %f\n", 6.65+(iE)*0.5, totxsec_weighted_val[iE], totxsec_weighted_err[iE]);
  }
}

void plotWeightedAvg(TH1D* hist1[], double xmax, double ymax, const char* saveName)
{
  cout << "AM I BREAKING IN PLOTWEIGHTEDAVG??" << endl;
  //Initiate variables
  char savePath[300];
  double small = 1e-5;
  double numBins = 10;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("can", "diff_xsection_weighted", 30, 10, 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);
  
  C->Draw();
  C->Divide(4, 3, small, small);
   
  //setStyle();
  for (int j = 0; j < numBins; j++)
    {
      C->cd(j+1);
       
      // Set Pad style for plots
      gPad->SetLogy();
      gPad->SetGrid();
      gPad->SetFillStyle(0);
      gPad->SetFrameFillStyle(0);
      gPad->SetTopMargin(small);
      gPad->SetBottomMargin(small);
      gPad->SetRightMargin(small);
      gPad->SetLeftMargin(small);    
      gStyle->SetOptStat(0);

      TLatex* latex = new TLatex();
      latex->SetNDC();
      //latex->SetTextFont(42);
      latex->SetTextSize(0.05);
      latex->SetTextAlign(32);

      // Set histograms and draw on Pad
      hist1[j]->SetMaximum(ymax);
      hist1[j]->SetMinimum(0.11);
      hist1[j]->SetMarkerColor(kAzure);
      hist1[j]->SetLineColor(kBlack);
      hist1[j]->SetMarkerStyle(20);
      hist1[j]->SetMarkerSize(1.0);
      
      //hist1[j]->GetXaxis()->SetNdivisions(4);
     
      // Plot all data sets
      hist1[j]->Draw("E1");
    }
  
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
      yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, 0.11, ymax, 4, "G");
      yax->SetLabelSize(0.04);
      //yax->SetLabelOffset(-0.01);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 2; i < 4; i++)
    {
      xax = new TGaxis(0.95-.85/4*(i+1), 0.1, 0.95-.85/4*(i), 0.1, 2*small, xmax, 4, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  for ( int i = 0; i < 2; i++)
    {
      xax = new TGaxis(0.95-.85/4*(i+1), 0.95-.85/3*(2), 0.95-.85/4*(i), 0.95-.85/3*(2), 2*small, xmax, 4, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  // Draw plot axis labels
  xTitle.SetTextFont(132);
  xTitle.SetTextSize(0.06);
  xTitle.DrawLatex(0.37, 0.01, "-t (GeV^{2} )");
  
  yTitle.SetTextFont(132);
  yTitle.SetTextSize(0.06);
  yTitle.SetTextAngle(90);
  yTitle.DrawLatex(0.05, 0.55, "d#sigma/dt (nb/GeV^{2} )");
  
  gStyle->SetPadBottomMargin(0.2);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.04);
  gStyle->SetLabelOffset(0.03,"Y");
 
  // Add Legend
  auto legend = new TLegend(0.53,0.16,0.73,0.3);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  legend->AddEntry(hist1[1],"GlueX-I","lep");
  legend->Draw();
  
  sprintf(savePath, "/d/grid17/hjesse/AnalysisNote/xsection/%s.pdf", saveName);
  tC->SaveAs(savePath);

}

void rooFitHist(TH1* hist,char* histTitle, char* ws_name, double* yield, double* yield_err)
{
  Double_t min_mass = hist->GetXaxis()->GetBinLowEdge(hist->FindFirstBinAbove(0,1,1, hist->FindBin(1.3)));
  if(min_mass < 1.27) min_mass = 1.27;

  RooWorkspace* w = new RooWorkspace(ws_name);
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV)", min_mass, 1.47);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  //XiMassKinFit_Ebin_accsub->Print();
  w->import(RooArgSet(mass)); 

  w->factory("Chebychev::bkgd(mass,{a0[0.8,0.1,1.5],a1[-0.2,-1,-0.1]})");//,a1[-0.1,-2,-1e-2]
  //w->factory("Voigtian::sigma(mass,mean[1.385,1.383,1.388],sig[0.0055. 0.004, 0.006], width[0.015, 0.01, 0.042])");
  //w->factory("Gaussian::sigma(mass,mean[1.385],sig[0.019,0.018,0.02])");
  //w->factory("Gaussian::xigaus(mass,gausm[1.3217,1.32,1.33],gaussig[0.005,0.004,0.01])");
  w->factory("Johnson::xigaus(mass,mu[1.3217,1.32,1.33],lambda[0.0055,0.002,0.006], gamma[0], delta[1.2])");
  w->factory("SUM::model(nbkgd[200,1,1e8]*bkgd, nxi[100,1,1e8]*xigaus)");//, nsigma[50,0,1e5]*sigma

  RooPlot* massframe = mass.frame(Title(histTitle));
  massframe->GetYaxis()->SetMaxDigits(3);
  massframe->SetNdivisions(505);
  
  w->pdf("model")->fitTo(*data,Extended(kTRUE), SumW2Error(kTRUE),PrintLevel(-1),PrintEvalErrors(-1),Verbose(false),Warnings(false));
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, Format("N",AutoPrecision(1)), Layout(0.55, 0.95, 0.88) ,Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nbkgd"))));
  massframe->getAttText()->SetTextSize(0.08) ;
  massframe->getAttFill()->SetFillStyle(0) ;
  massframe->getAttLine()->SetLineColor(0) ;
  w->pdf("model")->plotOn(massframe, LineWidth(2) );
  w->pdf("xigaus")->plotOn(massframe,DrawOption("F"), FillColor(kBlue-9),FillStyle(4010),MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent) );
  //w->pdf("sigma")->plotOn(massframe, DrawOption("F"), FillColor(kBlue-9),FillStyle(4010),MoveToBack() , Normalization(w->var("nsigma")->getVal(), RooAbsReal::NumEvent));
  w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent) );
  
  *yield = w->var("nxi")->getVal();
  *yield_err = w->var("nxi")->getError();

  if(*yield<10){ *yield = 0; *yield_err = 0;}
  
  massframe->Draw();
}

void rooFitHistMC(TH1* hist, char* histTitle,char *ws_name, double *yield, double *yield_err)
{
  Double_t min_mass = hist->GetXaxis()->GetBinLowEdge(hist->FindFirstBinAbove(0,1,1, hist->FindBin(1.3)));
  //if(min_mass < 1.275) min_mass = 1.275;

  RooWorkspace* w = new RooWorkspace(ws_name);
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.26, 1.42);
  mass.setRange("signal", min_mass, 1.37);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  //XiMassKinFit_Ebin_accsub->Print();
  w->import(RooArgSet(mass));

  //w->factory("Chebychev::bkgd(mass,{a0[0.1,0.01,10],a1[-1.1,-10,-0.1],a2[1.1,0.1,10]})");//,a1[-0.1,-2,-1e-2]
  //w->factory("Landau::bkgd(mass,m[1.29,1.285,1.31],s[0.01,0.001,0.03])");//,a1[-0.1,-2,-1e-2]
  w->factory(("EXPR::bkgd('(mass)*(((mass)/m0)**2-1.0)**p*exp(b*(((mass)/m0)**2-1.0))',mass, m0["+to_string(min_mass)+",1.25,1.275], b[-22,-40.,-5.], p[2])").c_str());
  //w->factory("Gaussian::xigaus(mass,mean1[1.32,1.31,1.33], sigma[0.005,0.003,0.007])");
  w->factory("Johnson::xigaus(mass,mu[1.3217,1.32,1.33],lambda[0.0045,0.003,0.006], gamma[0], delta[1.2,0.2,3.])");
  w->factory("SUM::model( nxi[1000,1,1e6]*xigaus, nbkgd[100,1,1e6]*bkgd)");// nbkgd[200,1,1e6]*bkgd,

  RooPlot* massframe = mass.frame(Title(histTitle));
  massframe->GetYaxis()->SetMaxDigits(3);
  massframe->SetNdivisions(505);
  
  w->pdf("model")->fitTo(*data,Extended(kTRUE), SumW2Error(kTRUE),PrintLevel(-1),PrintEvalErrors(-1),Verbose(false),Warnings(false),Range("signal"));
  //massframe->SetXTitle("#Lambda#pi^{-} mass");
  data->plotOn(massframe);
  w->pdf("model")->paramOn(massframe, Format("N",AutoPrecision(1)), Layout(0.57, 0.95, 0.87), Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nbkgd"))));
  massframe->getAttText()->SetTextSize(0.08) ;
  massframe->getAttFill()->SetFillStyle(0) ;
  massframe->getAttLine()->SetLineColor(0) ;
  w->pdf("model")->plotOn(massframe, LineWidth(1), Range("signal"));
  w->pdf("xigaus")->plotOn(massframe,DrawOption("F"), FillColor(kBlue-9),FillStyle(3144),MoveToBack(), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), Range("signal"));
  w->pdf("bkgd")->plotOn(massframe,LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Range("signal"));
  
  //RooAbsReal* sig = w->pdf("xigaus")->createIntegral(mass,  Range("signal"));
  *yield = w->var("nxi")->getVal();//*sig->getVal();
  *yield_err = w->var("nxi")->getError();//*sig->getVal();

  if(*yield<10){ *yield = 0; *yield_err = 0;}
  
  massframe->Draw();
}

//void fitRatio(TH1* hist, double xmin, double xmax)
//{
//hist->Fit("pol0", xmin, xmax);
//}

void setStyle()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(700);
  gStyle->SetCanvasDefW(600);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.17);
  gStyle->SetPadTopMargin   (0.10);
  gStyle->SetPadLeftMargin  (0.2);
  gStyle->SetPadRightMargin (0.02);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);
  gStyle->SetGridStyle(3);
  gStyle->SetGridWidth(1);

  gStyle->SetFrameFillStyle ( 0);
  gStyle->SetFrameFillColor ( 0);
  gStyle->SetFrameLineColor ( 1);
  gStyle->SetFrameLineStyle ( 0);
  gStyle->SetFrameLineWidth ( 1);
  gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);

  gStyle->SetNdivisions(505);

  //gStyle->SetLineWidth(1);
  //gStyle->SetHistLineWidth(1);
  //gStyle->SetFrameLineWidth(2);
  gStyle->SetLegendFillColor(0);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetMarkerStyle(20);

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
  
  gStyle->SetTitleSize(0.07,"T");
  gStyle->SetTitleSize(0.08,"X");
  gStyle->SetTitleSize(0.08,"Y");

  //gStyle->SetTitleOffset(1.0,"T");
  gStyle->SetTitleAlign(33);
  gStyle->SetTitleX(.99);
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(1.1,"Y");

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

void help()
{
  printf("This script produces the Xi XSec\n");
  printf("Syntax: root -l kpkpxim_xsec.C dataName mcName thrownName fluxName");    
}

void ratioPlots()
{
  
  // setStyle();
  // Make ratio plots6.4
  // TCanvas *ratioCan[10];
  // TPad *rpad[10]; 
  // char pname[100];
  // double small = 1e-5;
  // auto enBins = diff_xsec_201701_2400[0]->GetNbinsX();
  // TH1D* ratiores_17_1801 = new TH1D("ratiores_17_1801","Ratio Residuals 2017/201801; Energy Bins; Ratio", 10, 6.4, 11.4);
  // //ratiores_17_1801->SetMaximum(1.6);
  // //ratiores_17_1801->SetMinimum(0.5);
  
  // TH1D* ratiores_17_1808 = new TH1D("ratiores_17_1808","Ratio Residuals 2017/201808; Energy Bins; Ratio", 10, 6.4, 11.4);
  // //ratiores_17_1808->SetMaximum(1.6);
  // //ratiores_17_1808->SetMinimum(0.5);

  // TH1D* ratiores_1801_1808 = new TH1D("ratiores_1801_1808","Ratio Residuals 201801/201808; Energy Bins; Ratio", 10, 6.4, 11.4);
  // //ratiores_1801_1808->SetMaximum(1.6);
  // //ratiores_1801_1808->SetMinimum(0.5);

  // TH1D *ratio_2017_201801[10];
  // TH1D *ratio_2017_201808[10];
  // TH1D *ratio_201801_201808[10];
  // TF1* fit = new TF1("fit", "pol0(0)", 0.0, tmax); 
  // //fit->SetParameters(1.0,0.0);
  // gStyle->SetOptFit(1);

  // for(int j=0; j<10; j++)
  //   {
  //     sprintf(pname,"ratio_Ebin_%f",(j)*0.5+6.4);
  //     ratioCan[j] =  new TCanvas(pname,"xsec ratio", 30, 10, 1000, 700);
  //     rpad[j] = new TPad(pname,"ratios", 0.1,0.1,0.95,0.95);
      
  //     rpad[j]->Draw();
  //     rpad[j]->Divide(1,3,small,small);
     
  //     gStyle->SetOptStat(0);
  //     TLatex* latex = new TLatex();
  //     latex->SetNDC();
  //     //latex->SetTextFont(42);
  //     latex->SetTextSize(0.04);
  //     latex->SetTextAlign(32);

  //     // Ratio Plots
  //     ratio_2017_201801[j] = (TH1D*) diff_xsec_201801_2400[j]->Clone("2017/201801");
  //     ratio_2017_201801[j]->GetYaxis()->SetTitle("2017/201801");
  //     ratio_2017_201801[j]->Divide(diff_xsec_201701_2400[j]);
  //     ratio_2017_201801[j]->SetMaximum(2.);
  //     ratio_2017_201801[j]->SetMinimum(small);
  //     ratio_2017_201801[j]->SetMarkerColor(kBlack);
  //     ratio_2017_201801[j]->SetLineColor(kBlack);
  //     ratio_2017_201801[j]->SetMarkerStyle(20);
  //     ratio_2017_201801[j]->SetMarkerSize(1.0);
  //     //ratio_2017_201801[j]->SetLabelSize(0.1,"XY");
  //     //ratio_2017_201801[j]->SetTitle(" ; ; 2017/201801");      
     
  //     ratio_2017_201808[j] = (TH1D*) diff_xsec_201808_2400[j]->Clone("2017/201808");
  //     ratio_2017_201808[j]->GetYaxis()->SetTitle("2017/201808");
  //     ratio_2017_201808[j]->Divide(diff_xsec_201701_2400[j]);
  //     ratio_2017_201808[j]->SetMaximum(2.);
  //     ratio_2017_201808[j]->SetMinimum(small);
  //     ratio_2017_201808[j]->SetMarkerColor(kBlack);
  //     ratio_2017_201808[j]->SetLineColor(kBlack);
  //     ratio_2017_201808[j]->SetMarkerStyle(20);
  //     ratio_2017_201808[j]->SetMarkerSize(1.0);
            
  //     //ratio_2017_201808[j]->SetTitle(" ; ; 2017/201808");
  //     ratio_201801_201808[j] = (TH1D*) diff_xsec_201808_2400[j]->Clone("201801/201808");
  //     ratio_201801_201808[j]->GetYaxis()->SetTitle("201801/201808");
  //     ratio_201801_201808[j]->Divide(diff_xsec_201801_2400[j]);
  //     ratio_201801_201808[j]->SetMaximum(2.);
  //     ratio_201801_201808[j]->SetMinimum(small);
  //     ratio_201801_201808[j]->SetMarkerColor(kBlack);
  //     ratio_201801_201808[j]->SetLineColor(kBlack);
  //     ratio_201801_201808[j]->SetMarkerStyle(20);
  //     ratio_201801_201808[j]->SetMarkerSize(1.0);
      
  //     //ratio_201801_201808[j]->SetTitle(" ; -t (GeV^{2}); 201808/201801");
      
  //     rpad[j]->cd(1);
  //     gPad->SetFillStyle(0);
  //     gPad->SetFrameFillStyle(0);
  //     // gPad->SetTopMargin(small);
  //     // gPad->SetBottomMargin(small);
  //     // gPad->SetRightMargin(small);
  //     // gPad->SetLeftMargin(small);    

  //     ratio_2017_201801[j]->Fit("fit","Q");
  //     ratiores_17_1801->SetBinContent(j+1, fit->GetParameter(0));
  //     ratiores_17_1801->SetBinError(j+1 ,fit->GetParError(0));
  //     ratio_2017_201801[j]->Draw("e1");
      
  //     rpad[j]->cd(2);
  //     gPad->SetFillStyle(0);
  //     gPad->SetFrameFillStyle(0);
  //     // gPad->SetTopMargin(small);
  //     // gPad->SetBottomMargin(small);
  //     // gPad->SetRightMargin(small);
  //     // gPad->SetLeftMargin(small);    

  //     ratio_2017_201808[j]->Fit("fit","Q");
  //     ratiores_17_1808->SetBinContent(j+1, fit->GetParameter(0));
  //     ratiores_17_1808->SetBinError(j+1 ,fit->GetParError(0));
  //     ratio_2017_201808[j]->Draw("e1");

  //     rpad[j]->cd(3);
  //     gPad->SetFillStyle(0);
  //     gPad->SetFrameFillStyle(0);
  //     // gPad->SetTopMargin(small);
  //     // gPad->SetBottomMargin(small);
  //     // gPad->SetRightMargin(small);
  //     // gPad->SetLeftMargin(small);    
      
  //     ratio_201801_201808[j]->Fit("fit", "Q");
  //     ratiores_1801_1808->SetBinContent(j+1 ,fit->GetParameter(0));
  //     ratiores_1801_1808->SetBinError(j+1 ,fit->GetParError(0));
  //     ratio_201801_201808[j]->Draw("e1");
      
  //     sprintf(pname,"/d/grid17/hjesse/macros/xsection/ratio/ratio_Ebin_%f.pdf",(j)*0.5+6.4);
  //     //ratioCan[j]->Print();
  //     ratioCan[j]->SaveAs(pname);
  //   }

  // TCanvas *ratiores_Can = new TCanvas("ratiores_Can","");
  // ratiores_Can->Divide(1,3);
  // ratiores_Can->cd(1);
  // ratiores_17_1801->Fit("pol0");
  // ratiores_17_1801->Draw("e1");
  // ratiores_Can->cd(2);
  // ratiores_17_1808->Fit("pol0");
  // ratiores_17_1808->Draw("e1");
  // ratiores_Can->cd(3);
  // ratiores_1801_1808->Fit("pol0");
  // ratiores_1801_1808->Draw("e1");
  
  // ratiores_Can->Print();
  // gStyle->SetOptFit(0);
  // ratiores_Can->SaveAs("/d/grid17/hjesse/macros/xsection/ratio/ratiores_Canvas.pdf");
}
