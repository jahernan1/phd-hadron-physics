#include "gxana/xsection/Plotting.h"

#include <TAxis.h>
#include <TCanvas.h>
#include <TGaxis.h>
#include <TGraphErrors.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TPad.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace gxana {
namespace xsec {

namespace {
std::string gPlotDir;
} // namespace

void SetPlotDir(const std::string& dir)
{
  gPlotDir = dir;
  if (!dir.empty()) gSystem->mkdir(dir.c_str(), true); // TCanvas::SaveAs does not create it
}

std::string PlotDir() { return gPlotDir.empty() ? "." : gPlotDir; }

namespace {
// A label with no text files yields empty graph vectors; the plotters below index
// arrGraphs[0] (and every group up to the first group's size), so skip those inputs.
bool skipPlot(const std::vector<std::vector<TGraphErrors*>>& groups, size_t minBins, const std::string& saveName)
{
  for (const auto& g : groups)
    if (g.size() < minBins || g.size() < groups[0].size()) {
      std::cout << "Skipping " << saveName << ": missing graphs" << std::endl;
      return true;
    }
  return false;
}
} // namespace

void plotDiffXSec(std::vector<std::vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, std::string saveName)
{
  if (arrGraphs.size() < 3 || skipPlot(arrGraphs, 2, saveName)) return;
  //Initiate variables
  double numBins = arrGraphs[0].size();
  std::cout << "Num Bins in plotting: " << numBins << std::endl;
  double small = 1e-5;
  double ymin = 0.15;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection", 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);

  C->Draw();
  C->Divide(3, 3, small, small);

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
      arrGraphs[0][j]->SetMaximum(ymax);
      arrGraphs[0][j]->SetMinimum(ymin);
      arrGraphs[0][j]->SetMarkerColor(kAzure);
      arrGraphs[0][j]->SetLineColor(kAzure);
      arrGraphs[0][j]->SetMarkerStyle(20);
      arrGraphs[0][j]->SetMarkerSize(0.8);

      arrGraphs[1][j]->SetMaximum(ymax);
      arrGraphs[1][j]->SetMinimum(ymin);
      arrGraphs[1][j]->SetMarkerColor(kSpring-6);
      arrGraphs[1][j]->SetLineColor(kSpring-6);
      arrGraphs[1][j]->SetMarkerStyle(20);
      arrGraphs[1][j]->SetMarkerSize(0.8);

      arrGraphs[2][j]->SetMaximum(ymax);
      arrGraphs[2][j]->SetMinimum(ymin);
      arrGraphs[2][j]->SetMarkerColor(kRed+1);
      arrGraphs[2][j]->SetLineColor(kRed+1);
      arrGraphs[2][j]->SetMarkerStyle(20);
      arrGraphs[2][j]->SetMarkerSize(0.8);

      // Plot all data sets
      arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      arrGraphs[2][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      arrGraphs[0][j]->Draw("ap");
      arrGraphs[1][j]->Draw("p same");
      arrGraphs[2][j]->Draw("p same");
    }
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
      yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax, 510, "G");
      yax->SetLabelSize(0.04);
      //yax->SetLabelOffset(-0.01);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 1; i < 3; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.05, xmax, 205, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }

  for ( int i = 0; i < 1; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.05, xmax, 205, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }

  // Draw plot axis labels
  xTitle.SetTextFont(132);
  xTitle.SetTextSize(0.06);
  xTitle.DrawLatex(0.5, 0.01, "-t (GeV^{2} )");

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
  auto legend = new TLegend(0.7,0.15,0.95,0.3);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  legend->AddEntry(arrGraphs[0][1],"Spring 2017","lep");
  legend->AddEntry(arrGraphs[1][1],"Spring 2018","lep");
  legend->AddEntry(arrGraphs[2][1],"Fall 2018","lep");
  //legend->AddEntry(diff_xsec_weighted[1],"GlueX#lower[-0.15]{-}#kern[0.2]{I}","lep");
  //legend->AddEntry(diff_xsec_weighted[1],"Spring 2018 GlueX-I","lep");
  legend->Draw();

  const std::string savePath = PlotDir() + "/" + saveName + ".pdf";
  tC->SaveAs(savePath.c_str());
}

void plotWeightedXSec(std::vector<TGraphErrors*> arrGraphs, double xmax, double ymax, std::string saveName)
{
  if (skipPlot({arrGraphs}, 1, saveName)) return;
  //Initiate variables
  double numBins = arrGraphs.size();
  std::cout << "Num Bins in plotting: " << numBins << std::endl;
  double small = 1e-5;
  double ymin = 0.15;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection_weighted", 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);

  C->Draw();
  C->Divide(3,3, small, small);

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
      latex->SetTextSize(0.04);
      latex->SetTextAlign(32);

      // Set histograms and draw on Pad
      arrGraphs[j]->SetMaximum(ymax);
      arrGraphs[j]->SetMinimum(ymin);
      arrGraphs[j]->SetMarkerColor(kAzure);
      arrGraphs[j]->SetLineColor(kAzure);
      arrGraphs[j]->SetMarkerStyle(20);
      arrGraphs[j]->SetMarkerSize(0.8);

      // Plot all data sets
      arrGraphs[j]->GetXaxis()->SetRangeUser(0.05,xmax);
      arrGraphs[j]->DrawClone("ap");
    }
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
      yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax, 510, "G");
      yax->SetLabelSize(0.04);
      //yax->SetLabelOffset(-0.01);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 1; i < 3; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.05, xmax, 205, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }

  for ( int i = 0; i < 1; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.05, xmax, 205, "");
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }

  // Draw plot axis labels
  xTitle.SetTextFont(132);
  xTitle.SetTextSize(0.06);
  xTitle.DrawLatex(0.5, 0.01, "-t (GeV^{2} )");

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
  auto legend = new TLegend(0.7,0.15,0.95,0.3);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  legend->AddEntry(arrGraphs[0],"GlueX#lower[-0.15]{-}#kern[0.2]{I}","lep");
  legend->Draw();

  const std::string savePath = PlotDir() + "/" + saveName + ".pdf";
  tC->SaveAs(savePath.c_str());
}

void plotOneWeightedXSec(std::vector<TGraphErrors*> arrGraphs, double xmax, double ymax, std::string saveName)
{
  if (skipPlot({arrGraphs}, 1, saveName)) return;
  //Initiate variables
  double numBins = arrGraphs.size();
  std::cout << "Num Bins in plotting: " << numBins << std::endl;
  double small = 1e-5;
  double ymin = 0.15;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection_weighted");

  for (int j = 0; j < numBins; j++)
    {
        // Set Pad style for plots
      gPad->SetLogy();
      gPad->SetGrid();
      // gPad->SetFillStyle(0);
      // gPad->SetFrameFillStyle(0);
      // gPad->SetTopMargin(small);
      // gPad->SetBottomMargin(small);
      // gPad->SetRightMargin(small);
      // gPad->SetLeftMargin(small);
      gStyle->SetOptStat(0);

      TLatex* latex = new TLatex();
      latex->SetNDC();
      //latex->SetTextFont(42);
      latex->SetTextSize(0.04);
      latex->SetTextAlign(32);

      // Set histograms and draw on Pad
      arrGraphs[j]->SetMaximum(ymax);
      arrGraphs[j]->SetMinimum(ymin);
      arrGraphs[j]->SetMarkerColor(kAzure);
      arrGraphs[j]->SetLineColor(kAzure);
      arrGraphs[j]->SetMarkerStyle(20);
      arrGraphs[j]->SetMarkerSize(0.8);

      // Plot all data sets
      arrGraphs[j]->GetXaxis()->SetRangeUser(0.05,xmax);
      arrGraphs[j]->DrawClone("ap");
    }

  gStyle->SetPadBottomMargin(0.2);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.04);
  gStyle->SetLabelOffset(0.03,"Y");

  // Add Legend
  auto legend = new TLegend(0.7,0.15,0.95,0.3);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
  legend->AddEntry(arrGraphs[0],"GlueX#lower[-0.15]{-}#kern[0.2]{I}","lep");
  legend->Draw();

  const std::string savePath = PlotDir() + "/" + saveName + ".pdf";
  tC->SaveAs(savePath.c_str());
}

void plotFinalWeightedXSec(std::vector<std::vector<TGraphErrors*>> arrGraphs, double xmax, double ymax, std::string saveName)
{
  if (arrGraphs.size() < 2 || skipPlot(arrGraphs, 1, saveName)) return;
    //Initiate variables
    double numBins = arrGraphs[0].size();
    std::cout << "Num Bins in plotting: " << numBins << std::endl;
    double small = 1e-5;
    double ymin = 0.15;
    TGaxis  *yax, *xax;
    TLatex xTitle, yTitle;
    // Plot the differential cross sections together
    TCanvas *tC = new TCanvas("canvas", "diff_xsection", 1000, 700);
    TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);

    C->Draw();
    C->Divide(3, 3, small, small);

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
            arrGraphs[0][j]->SetMaximum(ymax);
            arrGraphs[0][j]->SetMinimum(ymin);
            arrGraphs[0][j]->SetMarkerColor(kAzure);
            arrGraphs[0][j]->SetLineColor(kAzure);
            arrGraphs[0][j]->SetMarkerStyle(20);
            arrGraphs[0][j]->SetMarkerSize(0.8);

            arrGraphs[1][j]->SetMaximum(ymax);
            arrGraphs[1][j]->SetMinimum(ymin);
            arrGraphs[1][j]->SetFillColorAlpha(kAzure,0.5);

            // Plot all data sets
            arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.05,xmax);
            arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.05,xmax);
            //
            arrGraphs[0][j]->Draw("ap");
            arrGraphs[1][j]->Draw("2|| same");
        }
    // Set up the matching axis for plot
    tC->cd();
    for ( int i = 0; i < 3; i++)
        {
            yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax, 510, "G");
            yax->SetLabelSize(0.04);
            //yax->SetLabelOffset(-0.01);
            yax->Draw("same");
        }

    //tC->cd();
    for ( int i = 1; i < 3; i++)
        {
            xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.05, xmax, 205, "");
            xax->SetLabelSize(0.04);
            //xax->SetLabelOffset(-0.01);
            xax->Draw("same");
        }

    for ( int i = 0; i < 1; i++)
        {
            xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.05, xmax, 205, "");
            xax->SetLabelSize(0.04);
            //xax->SetLabelOffset(-0.01);
            xax->Draw("same");
        }

    // Draw plot axis labels
    xTitle.SetTextFont(132);
    xTitle.SetTextSize(0.06);
    xTitle.DrawLatex(0.5, 0.01, "-t (GeV^{2} )");

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
    auto legend = new TLegend(0.7,0.15,0.95,0.3);
    legend->SetBorderSize(0);
    legend->SetTextSize(0.05);
    legend->SetTextFont(132);
    legend->SetFillColor(0);
    //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
    legend->AddEntry(arrGraphs[0][1],"GlueX#lower[-0.15]{-}#kern[0.2]{I}","lep");
    legend->AddEntry(arrGraphs[1][1],"#splitline{Systematic}{Uncertainty}","f");

    //legend->AddEntry(diff_xsec_weighted[1],"GlueX#lower[-0.15]{-}#kern[0.2]{I}","lep");
    //legend->AddEntry(diff_xsec_weighted[1],"Spring 2018 GlueX-I","lep");
    legend->Draw();

    const std::string savePath = PlotDir() + "/" + saveName + ".pdf";
    tC->SaveAs(savePath.c_str());
}

} // namespace xsec
} // namespace gxana
