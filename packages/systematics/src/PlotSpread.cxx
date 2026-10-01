// Drawing of the legacy comparison macros (archive/systematics_legacy/comparisons/
// PlotComboComparison.C, PlotFitComparison.C, PlotQValueComparison.C, PlotRunComparison.C),
// copied verbatim; the per-macro
// differences (legend, header, first-graph style, annotations, axis format,
// output path) come from PlotSpec.
#include "gxana/systematics/PlotSpread.h"
#include "gxana/common/GraphIO.h"
#include "gxana/common/Style.h"

#include <TAxis.h>
#include <TCanvas.h>
#include <TGaxis.h>
#include <TGraphErrors.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TMath.h>
#include <TPad.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace gxana {
namespace systematics {

// The legacy bodies are interpreted-macro code (implicit std).
using std::abs;
using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace {

using Graphs = vector<vector<TGraphErrors*>>;

std::string SavePath(const PlotSpec& spec) { return spec.outDir + "/" + spec.name + ".pdf"; }

Color_t AnnotationColor(int graph) { return graph == 1 ? kAzure : kRed + 1; }

std::string AnnotationText(const Annotation& ann, const GraphStats& stats)
{
    if (ann.what == "avg") return Form("#bf{Avg. Pct. Diff.: %.2f%%}", stats.avg_pct_diff);
    return Form("#bf{Max. Pct. Diff.: %.2f%%}", stats.max_pct_diff);
}

// PlotFitComparison.C:235-398 (PlotWeightedXSec); PlotComboComparison.C:213-368 is the
// same with the first graph drawn as points and a third annotation.
void PlotGrid3(const PlotSpec& spec, Graphs arrGraphs)
{
    const double xmax = spec.xmax, ymax = spec.ymax;
    //Initiate variables
    double numBins = arrGraphs[0].size();
    cout << "Num Bins in plotting: " << numBins << endl;
    double small = 1e-5;
    double ymin = 0.15;
    TGaxis  *yax, *xax;
    TLatex xTitle, yTitle;
    // Plot the differential cross sections together
    TCanvas *tC = new TCanvas("canvas", "diff_xsection_cuts", 1000, 700);
    TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);

    C->Draw();
    C->Divide(3, 3, small, small);

    //loop
    for (int j = 0; j < numBins; j++)
    {
        C->cd(j+1);

        // Set Pad style for plots
        //gPad->SetLogy();
        //gPad->SetGrid();
        gPad->SetFillStyle(0);
        gPad->SetFrameFillStyle(0);
        gPad->SetTopMargin(small);
        gPad->SetBottomMargin(small);
        gPad->SetRightMargin(small);
        gPad->SetLeftMargin(small);
        gStyle->SetOptStat(0);

        // Set histograms and draw on Pad
        arrGraphs[0][j]->SetMaximum(ymax);
        arrGraphs[0][j]->SetMinimum(ymin);
        arrGraphs[0][j]->SetMarkerColor(kBlack);
        if (spec.firstStyle == "band")
            arrGraphs[0][j]->SetFillColorAlpha(kBlack,0.2);
        arrGraphs[0][j]->SetLineColor(kBlack);
        arrGraphs[0][j]->SetMarkerStyle(20);
        arrGraphs[0][j]->SetMarkerSize(0.8);
        arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.05,xmax);

        arrGraphs[1][j]->SetMaximum(ymax);
        arrGraphs[1][j]->SetMinimum(ymin);
        arrGraphs[1][j]->SetMarkerColor(kAzure);
        arrGraphs[1][j]->SetLineColor(kAzure);
        arrGraphs[1][j]->SetMarkerStyle(24);
        arrGraphs[1][j]->SetMarkerSize(0.8);
        arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.05,xmax);
        //
        // arrGraphs[2][j]->SetMaximum(ymax);
        // arrGraphs[2][j]->SetMinimum(ymin);
        // arrGraphs[2][j]->SetMarkerColor(kSpring-6);
        // arrGraphs[2][j]->SetLineColor(kSpring-6);
        // arrGraphs[2][j]->SetMarkerStyle(24);
        // arrGraphs[2][j]->SetMarkerSize(0.8);
        // arrGraphs[2][j]->GetXaxis()->SetRangeUser(0.05,xmax);

        arrGraphs[2][j]->SetMaximum(ymax);
        arrGraphs[2][j]->SetMinimum(ymin);
        arrGraphs[2][j]->SetMarkerColor(kRed+1);
        arrGraphs[2][j]->SetLineColor(kRed+1);
        arrGraphs[2][j]->SetMarkerStyle(24);
        arrGraphs[2][j]->SetMarkerSize(0.8);
        arrGraphs[2][j]->GetXaxis()->SetRangeUser(0.05,xmax);
        //
        // Plot all data sets
        arrGraphs[0][j]->DrawClone(spec.firstStyle == "band" ? "ae3" : "ap");
        arrGraphs[1][j]->DrawClone("p same");
        arrGraphs[2][j]->DrawClone("p same");
        //arrGraphs[3][j]->DrawClone("xc same");
        //

        //Get stat difference between cut analysis
        GraphStats stats = GetAvgStdDev(*arrGraphs[0][j], *arrGraphs[1][j]);
        GraphStats stats1 = GetAvgStdDev(*arrGraphs[0][j], *arrGraphs[2][j]);
        std::cout << "Mean Diff: ("
                  << stats.mean_diff << ", " << stats1.mean_diff << ")\n"
                  << "Standard Deviation Diff: ("
                  << stats.std_dev_diff << ", " << stats1.std_dev_diff << ")\n"
                  << "T-Statistic: ("
                  << stats.paired_ttest << ", " << stats1.paired_ttest << ")\n"
                  << "Max Percent Difference: ("
                  << stats.max_pct_diff << ", " << stats1.max_pct_diff << ")\n"
                  << "Avg Percent Difference: ("
                  << stats.avg_pct_diff << ", " << stats1.avg_pct_diff << ")\n"
                  << "Siginificance: ("
                  << stats.signif << ", " << stats1.signif << ")\n"
                  <<  std::endl;

        // Create TLatex to display the result on canvas
        TLatex* latex = new TLatex();
        latex->SetNDC();
        latex->SetTextColor(kAzure);
        //latex->SetTextFont(42);
        latex->SetTextSize(0.08);
        latex->SetTextAlign(32);
        const double pos[3][2] = {{0.97, 0.8}, {0.96, 0.65}, {0.96, 0.5}};
        for (size_t a = 0; a < spec.annotate.size(); ++a) {
            const Annotation& ann = spec.annotate[a];
            latex->SetTextColor(AnnotationColor(ann.graph));
            latex->DrawLatex(pos[a][0], pos[a][1], AnnotationText(ann, ann.graph == 1 ? stats : stats1).c_str());
        }
    }

  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
      {
          yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax, spec.axisFormat);
          yax->SetLabelSize(0.04);
          //yax->SetLabelOffset(-0.01);
          yax->Draw("same");
      }

  //tC->cd();
  for ( int i = 1; i < 3; i++)
      {
          xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.02, xmax);
          xax->SetLabelSize(0.04);
          //xax->SetLabelOffset(-0.01);
          xax->Draw("same");
      }

  for ( int i = 0; i < 1; i++)
      {
          xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.02, xmax);
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

  gxana::ApplyStyle(gxana::GridTrailingTweak());

  // Add Legend
  auto legend = new TLegend(0.7,0.09,0.88,0.32);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  legend->SetFillStyle(0);

  if (!spec.legendHeader.empty())
      legend->SetHeader(spec.legendHeader.c_str(),"C"); // option "C" allows to center the header
  for (size_t k = 0; k < spec.legend.size(); ++k)
      legend->AddEntry(arrGraphs[k][0],spec.legend[k].text.c_str(),spec.legend[k].option.c_str());
  legend->Draw();

  tC->SaveAs(SavePath(spec).c_str());
}

// PlotComboComparison.C:370-499 (PlotAllXSec): two graphs and the spread band.
void PlotPairBand(const PlotSpec& spec, Graphs arrGraphs)
{
    const double xmax = spec.xmax, ymax = spec.ymax;
    //Initiate variables
    double numBins = arrGraphs[0].size();
    cout << "Num Bins in plotting: " << numBins << endl;
    double small = 1e-5;
    double ymin = 0.15;
    TGaxis  *yax, *xax;
    TLatex xTitle, yTitle;
    // Plot the differential cross sections together
    TCanvas *tC = new TCanvas("canvas", "diff_xsection_cuts", 1000, 700);
    TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);

    C->Draw();
    C->Divide(3, 3, small, small);

    //loop
    for (int j = 0; j < numBins; j++)
        {
            C->cd(j+1);

            // Set Pad style for plots
            //gPad->SetLogy();
            //gPad->SetGrid();
            gPad->SetFillStyle(0);
            gPad->SetFrameFillStyle(0);
            gPad->SetTopMargin(small);
            gPad->SetBottomMargin(small);
            gPad->SetRightMargin(small);
            gPad->SetLeftMargin(small);
            gStyle->SetOptStat(0);

            // Set histograms and draw on Pad
            arrGraphs[0][j]->SetMaximum(ymax);
            arrGraphs[0][j]->SetMinimum(ymin);
            arrGraphs[0][j]->SetMarkerColor(kBlack);
            arrGraphs[0][j]->SetLineColor(kBlack);
            arrGraphs[0][j]->SetMarkerStyle(24);
            arrGraphs[0][j]->SetMarkerSize(0.8);
            arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.05,xmax);

            arrGraphs[1][j]->SetMaximum(ymax);
            arrGraphs[1][j]->SetMinimum(ymin);
            arrGraphs[1][j]->SetMarkerColor(kRed+1);
            arrGraphs[1][j]->SetLineColor(kRed+1);
            arrGraphs[1][j]->SetMarkerStyle(24);
            arrGraphs[1][j]->SetMarkerSize(0.8);
            arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.05,xmax);

            arrGraphs[2][j]->SetMaximum(ymax);
            arrGraphs[2][j]->SetMinimum(ymin);
            arrGraphs[2][j]->SetFillColorAlpha(kGray,0.8);
            arrGraphs[2][j]->GetXaxis()->SetRangeUser(0.05,xmax);
            // Plot all data sets
            arrGraphs[0][j]->DrawClone("ap");
            arrGraphs[1][j]->DrawClone("p same");
            arrGraphs[2][j]->DrawClone("2 same");

            GraphStats stats = GetAvgStdDev(*arrGraphs[0][j], *arrGraphs[1][j]);
            // Create TLatex to display the result on canvas
            TLatex* latex = new TLatex();
            latex->SetNDC();
            latex->SetTextColor(kRed+1);
            //latex->SetTextFont(42);
            latex->SetTextSize(0.08);
            latex->SetTextAlign(32);
            for (const Annotation& ann : spec.annotate)
                latex->DrawLatex(0.97, 0.8, AnnotationText(ann, stats).c_str());
        }

    // Set up the matching axis for plot
    tC->cd();
    for ( int i = 0; i < 3; i++)
        {
            yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax, spec.axisFormat);
            yax->SetLabelSize(0.04);
            //yax->SetLabelOffset(-0.01);
            yax->Draw("same");
        }

    //tC->cd();
    for ( int i = 1; i < 3; i++)
        {
            xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.02, xmax);
            xax->SetLabelSize(0.04);
            //xax->SetLabelOffset(-0.01);
            xax->Draw("same");
        }

    for ( int i = 0; i < 1; i++)
        {
            xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.02, xmax);
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

    gxana::ApplyStyle(gxana::GridTrailingTweak());

    // Add Legend
    auto legend = new TLegend(0.7,0.09,0.88,0.32);
    legend->SetBorderSize(0);
    legend->SetTextSize(0.05);
    legend->SetTextFont(132);
    legend->SetFillColor(0);
    legend->SetFillStyle(0);

    if (!spec.legendHeader.empty())
        legend->SetHeader(spec.legendHeader.c_str(),"C"); // option "C" allows to center the header
    for (size_t k = 0; k < spec.legend.size(); ++k)
        legend->AddEntry(arrGraphs[k][0],spec.legend[k].text.c_str(),spec.legend[k].option.c_str());
    legend->Draw();

    tC->SaveAs(SavePath(spec).c_str());
}

// PlotFitComparison.C:400-523 (PlotAllXSec): every graph, the spread band last.
void PlotAllBand(const PlotSpec& spec, Graphs arrGraphs)
{
    const double xmax = spec.xmax, ymax = spec.ymax;
    //Initiate variables
    double numBins = arrGraphs[0].size();
    cout << "Num Bins in plotting: " << numBins << endl;
    double small = 1e-5;
    double ymin = 0.15;
    TGaxis  *yax, *xax;
    TLatex xTitle, yTitle;
    // Plot the differential cross sections together
    TCanvas *tC = new TCanvas("canvas", "diff_xsection_cuts", 1000, 700);
    TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);

    C->Draw();
    C->Divide(3, 3, small, small);

    vector<Style_t> mark = {24,25,26,27,28,30,32,42,46};
    //loop
    for (int i = 0 ; i < arrGraphs.size(); i++){
        for (int j = 0; j < numBins; j++)
            {
                C->cd(j+1);

                // Set Pad style for plots
                //gPad->SetLogy();
                //gPad->SetGrid();
                gPad->SetFillStyle(0);
                gPad->SetFrameFillStyle(0);
                gPad->SetTopMargin(small);
                gPad->SetBottomMargin(small);
                gPad->SetRightMargin(small);
                gPad->SetLeftMargin(small);
                gStyle->SetOptStat(0);

                // Set histograms and draw on Pad
                arrGraphs[i][j]->SetMaximum(ymax);
                arrGraphs[i][j]->SetMinimum(ymin);
                arrGraphs[i][j]->SetMarkerColor(kAzure);
                arrGraphs[i][j]->SetLineColor(kAzure);
                arrGraphs[i][j]->SetMarkerStyle(mark[i]);
                arrGraphs[i][j]->SetMarkerSize(0.8);
                arrGraphs[i][j]->GetXaxis()->SetRangeUser(0.05,xmax);
                //
                if(i==arrGraphs.size()-1){
                    arrGraphs[i][j]->SetMaximum(ymax);
                    arrGraphs[i][j]->SetMinimum(ymin);
                    arrGraphs[i][j]->SetMarkerColor(kBlack);
                    arrGraphs[i][j]->SetLineColor(kBlack);
                    arrGraphs[i][j]->SetFillColorAlpha(kGray,0.8);
                    // arrGraphs[i][j]->SetMarkerStyle(20);
                    // arrGraphs[i][j]->SetMarkerSize(0.8);
                    arrGraphs[i][j]->GetXaxis()->SetRangeUser(0.05,xmax);
                    arrGraphs[i][j]->DrawClone("2 same");
                }


                // Plot all data sets
                if(i==0)
                    arrGraphs[0][j]->DrawClone("ae3");
                else if(i<arrGraphs.size()-1)
                    arrGraphs[i][j]->DrawClone("p same");
            }
    }
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
      {
          yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax, spec.axisFormat);
          yax->SetLabelSize(0.04);
          //yax->SetLabelOffset(-0.01);
          yax->Draw("same");
      }

  //tC->cd();
  for ( int i = 1; i < 3; i++)
      {
          xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.02, xmax);
          xax->SetLabelSize(0.04);
          //xax->SetLabelOffset(-0.01);
          xax->Draw("same");
      }

  for ( int i = 0; i < 1; i++)
      {
          xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.02, xmax);
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

  gxana::ApplyStyle(gxana::GridTrailingTweak());

  // Add Legend
  auto legend = new TLegend(0.7,0.09,0.88,0.32);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  legend->SetFillStyle(0);

  if (!spec.legendHeader.empty())
      legend->SetHeader(spec.legendHeader.c_str(),"C"); // option "C" allows to center the header
  // entries name the inputs in order; the last one names the band
  for (size_t k = 0; k < spec.legend.size(); ++k) {
      const size_t g = k + 1 == spec.legend.size() ? arrGraphs.size() - 1 : k;
      legend->AddEntry(arrGraphs[g][0],spec.legend[k].text.c_str(),spec.legend[k].option.c_str());
  }
  legend->Draw();

  tC->SaveAs(SavePath(spec).c_str());
}

// PlotQValueComparison.C:102-228 (PlotWeightedXSec, 2-graph); PlotBunchComparison.C:110-236 and
// PlotFitBkgdComparison.C:98-224 differ only in the legend and the x-axis ndiv (205).
void PlotGrid2(const PlotSpec& spec, Graphs arrGraphs)
{
  const double xmax = spec.xmax, ymax = spec.ymax;
  //Initiate variables
  double numBins = arrGraphs[0].size();
  cout << "Num Bins in plotting: " << numBins << endl;
  double small = 1e-5;
  double ymin = 0.15;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection_cuts", 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);
  
  C->Draw();
  C->Divide(3, 3, small, small);

  //loop
  for (int j = 0; j < numBins; j++)
    {
      C->cd(j+1);
      
      // Set Pad style for plots
      //gPad->SetLogy();
      //gPad->SetGrid();
      gPad->SetFillStyle(0);
      gPad->SetFrameFillStyle(0);
      gPad->SetTopMargin(small);
      gPad->SetBottomMargin(small);
      gPad->SetRightMargin(small);
      gPad->SetLeftMargin(small);    
      gStyle->SetOptStat(0);

      // Set histograms and draw on Pad
      arrGraphs[0][j]->SetMaximum(ymax);
      arrGraphs[0][j]->SetMinimum(ymin);
      arrGraphs[0][j]->SetMarkerColor(kBlack);
      arrGraphs[0][j]->SetLineColor(kBlack);
      arrGraphs[0][j]->SetMarkerStyle(20);
      arrGraphs[0][j]->SetMarkerSize(0.8);
      arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.05,xmax);

      arrGraphs[1][j]->SetMaximum(ymax);
      arrGraphs[1][j]->SetMinimum(ymin);
      arrGraphs[1][j]->SetMarkerColor(kRed+1);
      arrGraphs[1][j]->SetLineColor(kRed+1);
      arrGraphs[1][j]->SetMarkerStyle(24);
      arrGraphs[1][j]->SetMarkerSize(0.8);
      arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.05,xmax);

      // Plot all data sets
      arrGraphs[0][j]->DrawClone("ap");
      arrGraphs[1][j]->DrawClone("p same");
      
      //Get stat difference between cut analysis
      PairStats stats = GetPairStats(*arrGraphs[0][j], *arrGraphs[1][j], false);
      std::cout << "Average: " << stats.avg << "\nStandard Deviation: " << stats.std_dev << "\nPercent Difference: " << stats.pct_diff << "\nSiginificance: " << stats.signif << "\n" <<  std::endl;
      
      // Create TLatex to display the result on canvas
      TLatex* latex = new TLatex();
      latex->SetNDC();
      latex->SetTextColor(kRed+1);      
      //latex->SetTextFont(42);
      latex->SetTextSize(0.08);
      latex->SetTextAlign(32);
      latex->DrawLatex(0.97, 0.8, Form("#bf{Avg. Pct. Diff.: %.2f%%}", stats.pct_diff));
    }
    
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
      yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax, spec.axisFormat);
      yax->SetLabelSize(0.04);
      //yax->SetLabelOffset(-0.01);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 1; i < 3; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.02, xmax, spec.xAxisFormat);
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  for ( int i = 0; i < 1; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.02, xmax, spec.xAxisFormat);
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
  
  gxana::ApplyStyle(gxana::GridTrailingTweak());
  
  // Add Legend
  auto legend = new TLegend(0.7,0.09,0.88,0.32);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  legend->SetFillStyle(0);
  
  if (!spec.legendHeader.empty())
      legend->SetHeader(spec.legendHeader.c_str(),"C"); // option "C" allows to center the header
  for (size_t k = 0; k < spec.legend.size(); ++k)
      legend->AddEntry(arrGraphs[k][0],spec.legend[k].text.c_str(),spec.legend[k].option.c_str());
  legend->Draw();

  tC->SaveAs(SavePath(spec).c_str());
}

// PlotRunComparison.C:218-368 (PlotWeightedXSec, one graph per run period).
void PlotRunGrid(const PlotSpec& spec, Graphs arrGraphs)
{
  const double xmax = spec.xmax, ymax = spec.ymax;
  //Initiate variables
  double numBins = arrGraphs[0].size();
  cout << "Num Bins in plotting: " << numBins << endl;
  double small = 1e-5;
  double ymin = 0.15;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection_cuts", 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);
  
  C->Draw();
  C->Divide(3, 3, small, small);

  //loop
  for (int j = 0; j < numBins; j++)
    {
      C->cd(j+1);
      
      // Set Pad style for plots
      //gPad->SetLogy();
      //gPad->SetGrid();
      gPad->SetFillStyle(0);
      gPad->SetFrameFillStyle(0);
      gPad->SetTopMargin(small);
      gPad->SetBottomMargin(small);
      gPad->SetRightMargin(small);
      gPad->SetLeftMargin(small);    
      gStyle->SetOptStat(0);

      // Set histograms and draw on Pad
      arrGraphs[0][j]->SetMaximum(ymax);
      arrGraphs[0][j]->SetMinimum(ymin);
      arrGraphs[0][j]->SetMarkerColor(kBlack);
      arrGraphs[0][j]->SetLineColor(kBlack);
      arrGraphs[0][j]->SetMarkerStyle(24);
      arrGraphs[0][j]->SetMarkerSize(0.8);
      arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.05,xmax);

      arrGraphs[1][j]->SetMaximum(ymax);
      arrGraphs[1][j]->SetMinimum(ymin);
      arrGraphs[1][j]->SetMarkerColor(kAzure);
      arrGraphs[1][j]->SetLineColor(kAzure);
      arrGraphs[1][j]->SetMarkerStyle(24);
      arrGraphs[1][j]->SetMarkerSize(0.8);
      arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      //
      // arrGraphs[2][j]->SetMaximum(ymax);
      // arrGraphs[2][j]->SetMinimum(ymin);
      // arrGraphs[2][j]->SetMarkerColor(kSpring-6);
      // arrGraphs[2][j]->SetLineColor(kSpring-6);
      // arrGraphs[2][j]->SetMarkerStyle(24);
      // arrGraphs[2][j]->SetMarkerSize(0.8);
      // arrGraphs[2][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      
      arrGraphs[2][j]->SetMaximum(ymax);
      arrGraphs[2][j]->SetMinimum(ymin);
      arrGraphs[2][j]->SetMarkerColor(kRed+1);
      arrGraphs[2][j]->SetLineColor(kRed+1);
      arrGraphs[2][j]->SetMarkerStyle(24);
      arrGraphs[2][j]->SetMarkerSize(0.8);
      arrGraphs[2][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      // 
      // Plot all data sets
      arrGraphs[0][j]->DrawClone("ap");
      arrGraphs[1][j]->DrawClone("p same");
      arrGraphs[2][j]->DrawClone("p same");
      //arrGraphs[3][j]->DrawClone("xc same");
      //
      
      //Get stat difference between cut analysis
      PairStats stats = GetPairStats(*arrGraphs[0][j], *arrGraphs[1][j], true);
      PairStats stats1 = GetPairStats(*arrGraphs[0][j], *arrGraphs[2][j], true);
      std::cout << "Average: " << stats.avg << "\nStandard Deviation: " << stats.std_dev << "\nPercent Difference: " << stats.pct_diff << "\nSiginificance: " << stats.signif << "\n" <<  std::endl;
      
      // Create TLatex to display the result on canvas
      TLatex* latex = new TLatex();
      latex->SetNDC();
      latex->SetTextColor(kAzure);      
      //latex->SetTextFont(42);
      latex->SetTextSize(0.08);
      latex->SetTextAlign(32);
      latex->DrawLatex(0.97, 0.8, Form("#bf{Avg. Pct. Diff.: %.2f%%}", stats.pct_diff));
      latex->SetTextColor(kRed+1);
      latex->DrawLatex(0.96, 0.65, Form("#bf{Avg. Pct. Diff.: %.2f%%}", stats1.pct_diff));
      //latex->DrawLatex(0.96, 0.65, Form("#bf{Avg. #sigma: %.2f}", stats.signif));
    }
    
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
        yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax, spec.axisFormat, "G");
      yax->SetLabelSize(0.04);
      //yax->SetLabelOffset(-0.01);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 1; i < 3; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.02, xmax);
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  for ( int i = 0; i < 1; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.02, xmax);
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
  
  gxana::ApplyStyle(gxana::GridTrailingTweak());
  
  // Add Legend
  auto legend = new TLegend(0.7,0.09,0.88,0.32);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  legend->SetFillStyle(0);
  
  if (!spec.legendHeader.empty())
      legend->SetHeader(spec.legendHeader.c_str(),"C"); // option "C" allows to center the header
  for (size_t k = 0; k < spec.legend.size(); ++k)
      legend->AddEntry(arrGraphs[k][0],spec.legend[k].text.c_str(),spec.legend[k].option.c_str());
  legend->Draw();

  tC->SaveAs(SavePath(spec).c_str());
}

// PlotRunComparison.C:370-504 (PlotWeightedXSecStdDev): the run periods and the scaled std-dev band.
void PlotStdDevBand(const PlotSpec& spec, Graphs arrGraphs)
{
  const double xmax = spec.xmax, ymax = spec.ymax;
  //Initiate variables
  double numBins = arrGraphs[0].size();
  cout << "Num Bins in plotting: " << numBins << endl;
  double small = 1e-5;
  double ymin = 0.15;
  TGaxis  *yax, *xax;
  TLatex xTitle, yTitle;
  // Plot the differential cross sections together
  TCanvas *tC = new TCanvas("canvas", "diff_xsection_cuts", 1000, 700);
  TPad *C = new TPad("pad", "pad", 0.1, 0.1, 0.95, 0.95);
  
  C->Draw();
  C->Divide(3, 3, small, small);

  //loop
  for (int j = 0; j < numBins; j++)
    {
      C->cd(j+1);
      
      // Set Pad style for plots
      //gPad->SetLogy();
      //gPad->SetGrid();
      gPad->SetFillStyle(0);
      gPad->SetFrameFillStyle(0);
      gPad->SetTopMargin(small);
      gPad->SetBottomMargin(small);
      gPad->SetRightMargin(small);
      gPad->SetLeftMargin(small);    
      gStyle->SetOptStat(0);

      // Set histograms and draw on Pad
      arrGraphs[0][j]->SetMaximum(ymax);
      arrGraphs[0][j]->SetMinimum(ymin);
      arrGraphs[0][j]->SetMarkerColor(kBlack);
      arrGraphs[0][j]->SetLineColor(kBlack);
      arrGraphs[0][j]->SetMarkerStyle(20);
      arrGraphs[0][j]->SetMarkerSize(0.8);
      arrGraphs[0][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      //
      arrGraphs[1][j]->SetMaximum(ymax);
      arrGraphs[1][j]->SetMinimum(ymin);
      arrGraphs[1][j]->SetMarkerColor(kAzure);
      arrGraphs[1][j]->SetLineColor(kAzure);
      arrGraphs[1][j]->SetMarkerStyle(24);
      arrGraphs[1][j]->SetMarkerSize(0.8);
      arrGraphs[1][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      //
      arrGraphs[2][j]->SetMaximum(ymax);
      arrGraphs[2][j]->SetMinimum(ymin);
      arrGraphs[2][j]->SetMarkerColor(kRed+1);
      arrGraphs[2][j]->SetLineColor(kRed+1);
      arrGraphs[2][j]->SetMarkerStyle(24);
      arrGraphs[2][j]->SetMarkerSize(0.8);
      arrGraphs[2][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      //
      arrGraphs[3][j]->SetMaximum(ymax);
      arrGraphs[3][j]->SetMinimum(ymin);
      arrGraphs[3][j]->SetMarkerColor(kBlack);
      arrGraphs[3][j]->SetLineColor(kBlack);
      arrGraphs[3][j]->SetFillColorAlpha(kGray,0.8);
      arrGraphs[3][j]->SetMarkerStyle(20);
      arrGraphs[3][j]->SetMarkerSize(0.8);
      arrGraphs[3][j]->GetXaxis()->SetRangeUser(0.05,xmax);
      // Plot all data sets
      arrGraphs[0][j]->DrawClone("ap");
      arrGraphs[1][j]->DrawClone("p same");
      arrGraphs[2][j]->DrawClone("p same");
      arrGraphs[3][j]->DrawClone("2 same");
      //
    }
    
  // Set up the matching axis for plot
  tC->cd();
  for ( int i = 0; i < 3; i++)
    {
      yax = new TGaxis(0.1, 0.95-.85/3*(i+1), 0.1, 0.95-.85/3*i, ymin, ymax, spec.axisFormat);
      yax->SetLabelSize(0.04);
      //yax->SetLabelOffset(-0.01);
      yax->Draw("same");
    }

  //tC->cd();
  for ( int i = 1; i < 3; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.1, 0.95-.85/3*(i), 0.1, 0.02, xmax);
      xax->SetLabelSize(0.04);
      //xax->SetLabelOffset(-0.01);
      xax->Draw("same");
    }
  
  for ( int i = 0; i < 1; i++)
    {
      xax = new TGaxis(0.95-.85/3*(i+1), 0.95-.85/3*(2), 0.95-.85/3*(i), 0.95-.85/3*(2), 0.02, xmax);
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
  
  gxana::ApplyStyle(gxana::GridTrailingTweak());
  
  // Add Legend
  auto legend = new TLegend(0.7,0.09,0.88,0.32);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.05);
  legend->SetTextFont(132);
  legend->SetFillColor(0);
  legend->SetFillStyle(0);
  
  if (!spec.legendHeader.empty())
      legend->SetHeader(spec.legendHeader.c_str(),"C"); // option "C" allows to center the header
  for (size_t k = 0; k < spec.legend.size(); ++k)
      legend->AddEntry(arrGraphs[k][0],spec.legend[k].text.c_str(),spec.legend[k].option.c_str());
  legend->Draw();

  tC->SaveAs(SavePath(spec).c_str());
}

void Check(bool ok, const std::string& what)
{
    if (!ok) throw std::invalid_argument(what);
}

void CheckSpec(const PlotSpec& spec, size_t minInputs, size_t maxInputs, size_t maxLegend,
               size_t maxAnnotate, int maxGraph)
{
    const std::string& L = spec.layout;
    Check(spec.inputs.size() >= minInputs && spec.inputs.size() <= maxInputs,
          "layout " + L + " takes " + std::to_string(minInputs)
              + (maxInputs == minInputs ? "" : " to " + std::to_string(maxInputs)) + " inputs, got "
              + std::to_string(spec.inputs.size()));
    Check(spec.legend.size() <= maxLegend, "layout " + L + " takes at most " + std::to_string(maxLegend)
                                               + " legend entries");
    Check(spec.annotate.size() <= maxAnnotate, "layout " + L + " takes at most " + std::to_string(maxAnnotate)
                                                   + " annotations");
    for (const auto& ann : spec.annotate)
        Check(ann.graph >= 1 && ann.graph <= maxGraph && (ann.what == "avg" || ann.what == "max"),
              "layout " + L + ": bad annotation " + ann.what + ":" + std::to_string(ann.graph));
    Check(spec.firstStyle == "points" || spec.firstStyle == "band", "first style must be points or band");
}

} // namespace

// PlotComboComparison.C:540-615 (style_format), renamed.
void StyleFormat()
{
    gxana::ApplyStyle(gxana::ComparisonStyle());
}

// Replaces the macros' GetAllTGraphErrors(WeightedDiffXSecTGraphs_<label>.root): reads the
// tables MakeWeightedDiffXSecTGraphs.C (CreateRootFileFromTextFiles) put in that file, in
// the same (ascending emin) order and with the same title.
std::vector<TGraphErrors*> ReadLabelGraphs(const std::string& dir)
{
    return gxana::ReadBinnedGraphs(dir, "weighted_diffxsec");
}

// Replaces PlotRunComparison.C's GetAllTGraphErrors(DiffXSecTGraphs_<stem>_<label>.root): reads
// the per-period tables that file was made from (MakeWeightedDiffXSecTGraphs.C,
// CreateRootFileFromTextFiles on one period's diffxsec_flatTree_<stem>_* tables), in the same
// (ascending emin) order and with the same title.
std::vector<TGraphErrors*> ReadPeriodGraphs(const std::string& prefix)
{
    const auto slash = prefix.find_last_of('/');
    const std::string dir = slash == std::string::npos ? "." : prefix.substr(0, slash);
    const std::string stem = slash == std::string::npos ? prefix : prefix.substr(slash + 1);
    try {
        return gxana::ReadBinnedGraphs(dir, stem);
    } catch (const std::runtime_error& e) {
        // this reader's own wording when nothing matches
        if (std::string(e.what()) == "no " + stem + "_emin_*_emax_*.txt in " + dir)
            throw std::runtime_error("no " + prefix + "_emin_*_emax_*.txt");
        throw;
    }
}

// The band graphs GetPointwiseMeanAndStdDev built (PlotFitComparison.C:216-224), read back
// from the stats file it wrote.
std::vector<TGraphErrors*> ReadBandGraphs(const std::string& stats, const std::vector<TGraphErrors*>& shape)
{
    std::ifstream in(stats);
    if (!in) throw std::runtime_error("cannot open stats file " + stats);
    std::vector<std::vector<double>> rows;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream fields(line);
        std::vector<double> row(4);
        if (fields >> row[0] >> row[1] >> row[2] >> row[3]) rows.push_back(row);
    }
    size_t expected = 0;
    for (const auto* g : shape) expected += g->GetN();
    if (rows.size() != expected)
        throw std::runtime_error(stats + ": " + std::to_string(rows.size()) + " rows, the inputs have "
                                 + std::to_string(expected) + " points");

    std::vector<TGraphErrors*> resultGraphs;
    size_t r = 0;
    for (const auto* g : shape) {
        const int nPoints = g->GetN();
        // Create output TGraphErrors with mean as y-values and std dev as y-errors
        TGraphErrors* resultGraph = new TGraphErrors(nPoints);
        for (int i = 0; i < nPoints; ++i, ++r) {
            resultGraph->SetPoint(i, rows[r][0], rows[r][2]);
            resultGraph->SetPointError(i, rows[r][1], rows[r][3]);  // x-error = 0, y-error = standard deviation
        }
        resultGraphs.push_back(resultGraph);
    }
    return resultGraphs;
}

// PlotFitComparison.C:77-151 (same in PlotComboComparison.C:56-130).
GraphStats GetAvgStdDev(const TGraphErrors& graph1, const TGraphErrors& graph2) {
    int nPoints1 = graph1.GetN();
    int nPoints2 = graph2.GetN();

    if (nPoints1 != nPoints2) {
        throw std::invalid_argument("Graphs must have the same number of points.");
    }

    int nPoints = nPoints1; // Either graph1 or graph2 size, since they're equal
    double totalPercentDifference = 0;
    double maxPercentDifference = 0;
    std::vector<double> yValueDifference, yValues1, yValues2;
    double Delta, sigma, tot_sig = 0; // gxana: tot_sig initialised (uninitialised in the macro)

    // Gather y-values from both graphs
    for (int i = 0; i < nPoints; ++i) {
        double y1, y2;
        y1 = graph1.GetY()[i];
        y2 = graph2.GetY()[i];

        // Calculate the percent difference for this pair of points
        if (y1 != 0) {
            double percentDifference = ((y2 - y1) / y1) * 100.0;
            if(abs(percentDifference)>abs(maxPercentDifference))
                maxPercentDifference = percentDifference;
            totalPercentDifference += percentDifference;
        } else {
            throw std::runtime_error("Encountered a zero value in graph1; cannot compute percent difference.");
        }

        // Add each graph's y-value to the list
        yValueDifference.push_back(y1-y2);
        yValues1.push_back(y1);
        yValues2.push_back(y2);

        // Get significance of difference
        Delta = abs(graph1.GetPointY(i) - graph2.GetPointY(i));
        sigma = TMath::Sqrt( abs( graph1.GetErrorY(i)*graph1.GetErrorY(i) + graph2.GetErrorY(i)*graph2.GetErrorY(i) ) );
        tot_sig += Delta/sigma;
    }
    /*
       Get the paired t-test value
                                    */
    // Compute the mean difference of all y-values
    double sumDiff = 0;
    double sum1 = 0; double sum2 = 0;
    for (int i = 0 ; i<nPoints; i++) {
        sumDiff += yValueDifference[i];
        sum1 += yValues1[i]; sum2 += yValues2[i];  // gxana: yValues1[1] in the macro
    }
    double meanDiff = sumDiff / nPoints;
    double mean1 = sum1 / nPoints;
    double mean2 = sum2 / nPoints;

    // Compute the standard deviation
    double varianceDiff = 0;
    double variance1 = 0; double variance2 = 0;
    for (int i = 0 ; i<nPoints; i++) {
        varianceDiff += (yValueDifference[i] - meanDiff) * (yValueDifference[i] - meanDiff);
        variance1 += (yValues1[i] - mean1) * (yValues1[i] - mean1);
        variance2 += (yValues2[i] - mean2) * (yValues2[i] - mean2);
    }
    varianceDiff /= (nPoints - 1 ); // Sample standard deviation
    variance1 /= (nPoints-1);
    variance2 /= (nPoints-1);

    double stdDevDiff = std::sqrt(varianceDiff);
    double stdDev1 = std::sqrt(variance1);
    double stdDev2 = std::sqrt(variance2);

    double t_stat = meanDiff / (stdDevDiff / std::sqrt(nPoints) );
    double t_stat1 = (mean1-mean2) / sqrt( pow(stdDev1,2) / nPoints + pow(stdDev2,2) / nPoints );
    (void)t_stat1;

    return {meanDiff, stdDevDiff, t_stat, maxPercentDifference, totalPercentDifference/nPoints, tot_sig/nPoints};
}

// PlotQValueComparison.C:46-99 (GetAvgStdDev; the same in PlotBunchComparison.C and
// PlotFitBkgdComparison.C); PlotRunComparison.C:54-107 differs only in std::abs of the percent
// difference (absPct).
PairStats GetPairStats(const TGraphErrors& graph1, const TGraphErrors& graph2, bool absPct) {
    int nPoints1 = graph1.GetN();
    int nPoints2 = graph2.GetN();

    if (nPoints1 != nPoints2) {
        throw std::invalid_argument("Graphs must have the same number of points.");
    }

    int nPoints = nPoints1; // Either graph1 or graph2 size, since they're equal
    double totalPercentDifference = 0;
    std::vector<double> yValues;
    double Delta, sigma, tot_sig = 0; // gxana: tot_sig initialised (uninitialised in the macros)

    // Gather y-values from both graphs
    for (int i = 0; i < nPoints; ++i) {
        double y1, y2;
        y1 = graph1.GetY()[i];
        y2 = graph2.GetY()[i];

         // Calculate the percent difference for this pair of points
        if (y1 != 0) {
            double percentDifference = absPct ? std::abs((y2 - y1) / y1) * 100.0 : ((y2 - y1) / y1) * 100.0;
            totalPercentDifference += percentDifference;
        } else {
            throw std::runtime_error("Encountered a zero value in graph1; cannot compute percent difference.");
        }

        // Add each graph's y-value to the list
        yValues.push_back(y1);
        yValues.push_back(y2);

        // Get significance of difference
        Delta = abs(graph1.GetPointY(i) - graph2.GetPointY(i));
        sigma = TMath::Sqrt( abs( graph1.GetErrorY(i)*graph1.GetErrorY(i) + graph2.GetErrorY(i)*graph2.GetErrorY(i) ) );
        tot_sig += Delta/sigma;
    }

    // Compute the mean of all y-values
    double sum = 0;
    for (double y : yValues) {
        sum += y;
    }
    double mean = sum / yValues.size();

    // Compute the standard deviation
    double variance = 0;
    for (double y : yValues) {
        variance += (y - mean) * (y - mean);
    }
    variance /= (yValues.size() - 1); // Sample standard deviation
    double stdDev = std::sqrt(variance);

    return {mean, stdDev, totalPercentDifference/nPoints, tot_sig/nPoints};
}

void Plot(const PlotSpec& spec)
{
    const std::string& L = spec.layout;
    if (L == "grid3") CheckSpec(spec, 3, 3, 3, 3, 2);
    else if (L == "grid2") CheckSpec(spec, 2, 2, 2, 0, 0);       // legacy: one fixed avg annotation
    else if (L == "run_grid") CheckSpec(spec, 3, 3, 3, 0, 0);    // legacy: two fixed avg annotations
    else if (L == "stddev_band") CheckSpec(spec, 3, 3, 4, 0, 0); // three periods + band
    else if (L == "pair_band") CheckSpec(spec, 2, 2, 3, 1, 1);
    else if (L == "all_band") CheckSpec(spec, 2, 8, 2, 0, 0); // legacy marker list: 8 inputs + band
    else throw std::invalid_argument("unknown layout " + L);
    Check(!spec.name.empty() && !spec.outDir.empty(), "name and out dir are required");

    StyleFormat();
    const bool periods = L == "run_grid" || L == "stddev_band";
    Graphs arrGraphs;
    for (const auto& input : spec.inputs) {
        arrGraphs.push_back(periods ? ReadPeriodGraphs(input) : ReadLabelGraphs(input));
        if (arrGraphs.back().size() != arrGraphs[0].size())
            throw std::runtime_error(input + " has " + std::to_string(arrGraphs.back().size())
                                     + " energy bins, " + spec.inputs[0] + " has "
                                     + std::to_string(arrGraphs[0].size()));
        if (arrGraphs.back().size() > 9)
            throw std::runtime_error(input + ": more than 9 energy bins do not fit the 3x3 grid");
    }
    if (L == "grid3") {
        PlotGrid3(spec, arrGraphs);
        return;
    }
    if (L == "grid2") {
        PlotGrid2(spec, arrGraphs);
        return;
    }
    if (L == "run_grid") {
        PlotRunGrid(spec, arrGraphs);
        return;
    }
    Check(!spec.band.empty(), "layout " + L + " needs a band stats file");
    // stddev_band: the runcompare file, columns XVal XErr YWMean StdDevScaled first
    arrGraphs.push_back(ReadBandGraphs(spec.band, arrGraphs[0]));
    if (L == "pair_band") PlotPairBand(spec, arrGraphs);
    else if (L == "stddev_band") PlotStdDevBand(spec, arrGraphs);
    else PlotAllBand(spec, arrGraphs);
}

} // namespace systematics
} // namespace gxana
