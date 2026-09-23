#include "gxana/common/Style.h"

#include <TLatex.h>
#include <TROOT.h>
#include <TStyle.h>

namespace gxana {

// Body copied verbatim from AnalysisNote/xsection/PlotFunctions.cpp:4-79
// (Step 1 verification found the closing brace of SetStyle on line 79,
// not line 80 as the brief's illustrative snippet showed; line 80 is
// blank in the legacy file).
void SetStyle()
{
    
    gStyle->SetCanvasColor(0);
    gStyle->SetCanvasBorderSize(5);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasDefH(600);
    gStyle->SetCanvasDefW(700);

    gStyle->SetPadColor       (0);
    gStyle->SetPadBorderSize  (10);
    gStyle->SetPadBorderMode  (0);
    gStyle->SetPadBottomMargin(0.14);
    gStyle->SetPadTopMargin   (0.05);
    gStyle->SetPadLeftMargin  (0.13);
    gStyle->SetPadRightMargin (0.04);
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

    gStyle->SetNdivisions(505);

    gStyle->SetLineWidth(1);
    gStyle->SetHistLineWidth(1);
    //gStyle->SetLegendBorder(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLegendFillColor(0);
    gStyle->SetLegendFont(132);
    gStyle->SetLegendTextSize(0.06);
    gStyle->SetMarkerSize(1.0);
    gStyle->SetMarkerStyle(20);

    gStyle->SetLabelSize(0.05,"X");
    gStyle->SetLabelSize(0.05,"Y");

    gStyle->SetLabelOffset(0.009,"X");
    gStyle->SetLabelOffset(0.009,"Y");

    gStyle->SetLabelFont(132,"X");
    gStyle->SetLabelFont(132,"Y");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetTitleFont(132);
    gStyle->SetTitleFont(132,"X");
    gStyle->SetTitleFont(132,"Y");
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);

    gStyle->SetTitleSize(0.1,"T");
    gStyle->SetTitleSize(0.065,"X");
    gStyle->SetTitleSize(0.065,"Y");

    gStyle->SetTitleOffset(1.,"X");
    gStyle->SetTitleOffset(0.85,"Y");

    gStyle->SetTextSize(0.055);
    gStyle->SetTextFont(132);

    gStyle->SetOptStat(0);

    gROOT->ForceStyle();

    TLatex* latex = new TLatex();
    latex->SetNDC();
    //latex->SetTextFont(42);
    latex->SetTextSize(0.04);
    latex->SetTextAlign(32);
}

} // namespace gxana
