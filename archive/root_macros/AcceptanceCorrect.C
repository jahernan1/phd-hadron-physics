#include "TH2.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH2F.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>

//initalize Functions
TH1* GetAcceptanceHist(TH2* hist_genr, TH2* hist_recon);
TH1* GetAcceptanceCorrHist(TH2* hist_data, TH1* hist_acc);
TH2* GetAccidentalHist(TH2* hist, TH2* hist_wacc);
TH1* GetAcceptanceHist1D(TH1* hist_genr, TH1* hist_recon);
TH1* GetAcceptanceCorrHist1D(TH1* hist_data, TH1* hist_accept);

//main
int AccCorrect()
{
  //Initialize variables
  TFile* f17[3];
  TH2* PiMinus1Kin[3]; 
  TH2* PiMinus1Kin_wacc[2];
  TH2* PiMinus1Kin_accsub[2];
  TH2* LambdaKin[3]; 
  TH2* LambdaKin_wacc[2];
  TH2* LambdaKin_accsub[2];
  TH1* XiLifetimeRestFrame[3];
  //retrieve root files
  f17[0] = TFile::Open("/d/grid17/hjesse/analysis/kpkpxim/kpkpxim__M23_2017-01_ver45.root");
  f17[1] = TFile::Open("/d/grid17/hjesse/analysis/kpkpxim/kpkpxim__M23_2017-01_ver45_ystar2400_genr8.root");
  f17[2] = TFile::Open("/d/grid17/hjesse/analysis/kpkpxim/thrown_kpkpxim__M23_2017-01_ver45_ystar2400_genr8.root");

  //retrieve histograms
  for(int i=0; i<3; i++)
    {
      if(i==2)
	{
	  //PiMinus1Kin[i] = (TH2*)f17[i]->Get("thrown_PiMinus1P3VsCosTheta_HF");
	  //LambdaKin[i] = (TH2*)f17[i]->Get("thrown_LambdaP3VsCosTheta_HF");
      XiLifetimeRestFrame[i] = (TH1*)f17[i]->Get("thrown_XiLifetimeRestFrame_postCL");
    }
      else
	{
	  //PiMinus1Kin[i] = (TH2*)f17[i]->Get("PiMinus1P3VsCosTheta_HF");
	  //LambdaKin[i] = (TH2*)f17[i]->Get("LambdaP3CosTheta_HF");   
      XiLifetimeRestFrame[i] = (TH1*)f17[i]->Get("XiLifetimeRestFrame_postCL");
	}

      //printf("hist %d Bins: %d\n", i, PiMinus1Kin[i]->GetNbinsX());
      //printf("hist %d Bins: %d\n", i, LambdaKin[i]->GetNbinsX());
    }
  
  //get the acceptance histogram
  // TH1* PiMinus1_acceptance = (TH1*)GetAcceptanceHist(PiMinus1Kin[2], PiMinus1Kin_accsub[1])->Clone();
  // TH1* Lambda_acceptance = (TH1*)GetAcceptanceHist(LambdaKin[2], LambdaKin_accsub[1])->Clone();
  // //get acceptance corrected histogram
  // TH1* PiMinusTheta_AccCorr = (TH1*)GetAcceptanceCorrHist(PiMinus1Kin[0], PiMinus1_acceptance)->Clone();
  // TH1* LambdaTheta_AccCorr = (TH1*)GetAcceptanceCorrHist(LambdaKin[0], Lambda_acceptance)->Clone();

  TH1* XiLifetimeRestFrame_Acceptance = (TH1*)GetAcceptanceHist1D(XiLifetimeRestFrame[2], XiLifetimeRestFrame[1])->Clone();
  TH1* XiLifetimeRestFrame_AccCorr = (TH1*)GetAcceptanceCorrHist1D(XiLifetimeRestFrame[0], XiLifetimeRestFrame_Acceptance)->Clone();
  
  
  //Plot
  // TCanvas* c = new TCanvas("c","c");
  // c->Divide(2,1);
  // c->cd(1);
  // PiMinus1_acceptance->Draw("pe");
  // c->cd(2);
  // PiMinusTheta_AccCorr->Draw("pe");
  
  // TCanvas* c1 = new TCanvas("c1","c1");
  // c1->Divide(2,1);
  // c1->cd(1);
  // Lambda_acceptance->Draw("pe");
  // c1->cd(2);
  // LambdaTheta_AccCorr->Draw("pe");
  

  TCanvas* c2 = new TCanvas("Acceptance","c2");
  XiLifetimeRestFrame_Acceptance->Draw("pe1");
  new TCanvas;
  XiLifetimeRestFrame_AccCorr->Draw("pe1");
  
  return 0;
}

TH1* GetAcceptanceHist(TH2* hist_genr, TH2* hist_recon)
{
  TH1* hist_genr_px = (TH1*)hist_genr->ProjectionX()->Clone();
  TH1* hist_recon_px = (TH1*)hist_recon->ProjectionX()->Clone();
  TH1* hist_accept = (TH1*)hist_genr_px->Clone("PiMinus1Acceptance");
  hist_accept->SetTitle("; #cos(#Theta_{HF}); Acceptance");
  hist_accept->Sumw2();
  hist_accept->Divide(hist_recon_px);
    
  printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
  return hist_accept;
}

TH1* GetAcceptanceCorrHist(TH2* hist_data, TH1* hist_accept)
{
  const char *histName = hist_data->GetName();
  char newName[100];
  sprintf(newName,"%s_acccorr", histName);
  
  TH1* hist_data_acccorr = (TH1*)hist_data->ProjectionX()->Clone(newName);
  hist_data_acccorr->Divide(hist_accept);
  
  return hist_data_acccorr;
}

TH1* GetAcceptanceHist1D(TH1* hist_genr, TH1* hist_recon)
{
  TH1* hist_accept = (TH1*)hist_recon->Clone("PiMinus1Acceptance");
  hist_accept->SetTitle("; #Xi Lifetime RestFrame (ns)); Acceptance");
  //hist_accept->Sumw2();
  hist_accept->Divide(hist_genr);
    
  printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
  return hist_accept;
}

TH1* GetAcceptanceCorrHist1D(TH1* hist_data, TH1* hist_accept)
{
  const char *histName = hist_data->GetName();
  char newName[100];
  sprintf(newName,"%s_acccorr", histName);
  
  TH1* hist_data_acccorr = (TH1*)hist_data->Clone(newName);
  hist_data_acccorr->Divide(hist_accept);
  
  return hist_data_acccorr;
}

TH2* GetAccidentalHist(TH2* hist, TH2* hist_wacc)
{  
  const char *histName = hist->GetName();
  char newName[100];
  sprintf(newName,"%s_accsub", histName);

  TH2* hist_accsub = (TH2*)hist->Clone(newName);
  hist_accsub->Add(hist_wacc, -0.5);    

  return hist_accsub;
}
