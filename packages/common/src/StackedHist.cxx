#include "gxana/common/StackedHist.h"

#include <TCanvas.h>
#include <TH1D.h>
#include <THStack.h>
#include <TLegend.h>
#include <TPad.h>
#include <TROOT.h>

#include <cstdio>
#include <iostream>

using namespace std;

namespace gxana {

void MakeStackedHist(std::vector<TH1D*> arr_hist, std::string stack_title, std::string identifier,
                     std::string leg_pos, std::string leg_title, std::string plot_dir)
{
  string plotdir = plot_dir;
  if (!plotdir.empty() && plotdir.back() != '/')
    plotdir += "/";
  string histName = arr_hist[0]->GetName();
  string saveName = plotdir + histName + "_" + identifier + "_ac.pdf";
  char entries[4][100];
  TCanvas *c = new TCanvas((histName+"_"+identifier).c_str(), (histName+"_"+identifier).c_str());
  THStack *hs = new THStack("hs","");
  double scale_factor;

  if(leg_title=="Gen MC"){
      arr_hist[0]->SetFillColorAlpha(kGray,0.8);
      arr_hist[0]->SetLineColor(kBlack);
      arr_hist[0]->RebinX();
      hs->Add(arr_hist[0],"hist");
  }
  else
      arr_hist[0]->RebinX();

  for(Int_t i = 1; i < arr_hist.size(); i++){
    arr_hist[i]->RebinX();
    cout << "Bin Width Data: " << arr_hist[0]->GetBinWidth(1) << endl;
    cout << "Bin Width MC: " << arr_hist[i]->GetBinWidth(1) << endl;

    // scale the histos to area
    if(arr_hist[i]->Integral("width") < arr_hist[0]->Integral("width"))
      scale_factor = arr_hist[0]->Integral("width") / arr_hist[i]->Integral("width");
    else
      scale_factor = arr_hist[i]->Integral("width") / arr_hist[0]->Integral("width");

    if(arr_hist[i]->GetMaximum() < arr_hist[0]->GetMaximum())
      arr_hist[i]->Scale(scale_factor );
    else{
        scale_factor = 1/scale_factor ;
        arr_hist[i]->Scale( scale_factor );
    }

    arr_hist[i]->SetLineColor(kBlack);
    arr_hist[i]->SetFillColorAlpha(kAzure-9,0.6);
    //arr_hist[i]->SetFillStyle(4010);
    sprintf(entries[i], "%.0f Events", arr_hist[i]->GetEntries());

    hs->Add(arr_hist[i],"hist");
  }

  if(leg_title=="Data"){
      arr_hist[0]->SetLineColor(kBlack);
      arr_hist[0]->SetMarkerColor(kBlack);
      arr_hist[0]->SetMarkerStyle(20);
      arr_hist[0]->SetMarkerSize(0.9);
      hs->Add(arr_hist[0],"e1");
  }

  double ymax = arr_hist[0]->GetMaximum() > arr_hist[1]->GetMaximum()
      ? arr_hist[0]->GetMaximum() : arr_hist[1]->GetMaximum();
  hs->SetTitle(stack_title.c_str());
  hs->Draw("nostack");
  hs->GetYaxis()->SetMaxDigits(3);
  hs->SetMaximum(ymax*1.1);
  //hs->GetXaxis()->SetLimits(xmin,xmax);

  // Draw legend
  TLegend* legend;
  if(leg_pos=="tl")
    legend = new TLegend(0.2,0.75,0.4,0.91); //top left corner
  else
    legend = new TLegend(0.68,0.76,0.88,0.92); //top right corner

  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  legend->SetTextSize(0.055);
  //legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header

  if(leg_title=="Data")
      legend->AddEntry(arr_hist[0], leg_title.c_str(), "lep");
  else
      legend->AddEntry(arr_hist[0], leg_title.c_str(), "f");
  legend->AddEntry(arr_hist[1], "Recon MC", "f");
  legend->Draw("same");

  gPad->Update();
  c->SaveAs(saveName.c_str());
  //c->Close();
}

} // namespace gxana
