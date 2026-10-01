#include "gxana/common/Style.h"

#include <TROOT.h>
#include <TStyle.h>

namespace gxana {

// AnalysisNote/xsection/PlotFunctions.cpp:4-79 (SetStyle); the gStyle state it left is
// ThesisStyle(), checked against the verbatim body by tests/cpp/test_style.cxx.
void SetStyle()
{
    ApplyStyle(ThesisStyle());
}

namespace {

// Settings every preset below writes with the same value (the skeleton of the five original style functions).
StyleParams Skeleton()
{
    StyleParams p;
    p.canvasColor = 0;
    p.canvasBorderMode = 0;
    p.padColor = 0;
    p.padBorderSize = 10;
    p.padBorderMode = 0;
    p.padGridX = 0;
    p.padGridY = 0;
    p.padTickX = 0;
    p.padTickY = 0;
    p.frameFillStyle = 0;
    p.frameFillColor = 0;
    p.frameLineColor = 1;
    p.frameLineStyle = 0;
    p.frameBorderSize = 10;
    p.frameBorderMode = 0;
    p.legendFont = 132;
    p.legendTextSize = 0.06;
    p.markerStyle = 20;
    p.labelFontX = 132;
    p.labelFontY = 132;
    p.titleBorderSize = 0;
    p.textFont = 132;
    p.optStat = 0;
    p.forceStyle = true;
    return p;
}

} // namespace

void ApplyStyle(const StyleParams& p)
{
    if (p.canvasColor.set) gStyle->SetCanvasColor(p.canvasColor.value);
    if (p.canvasBorderSize.set) gStyle->SetCanvasBorderSize(p.canvasBorderSize.value);
    if (p.canvasBorderMode.set) gStyle->SetCanvasBorderMode(p.canvasBorderMode.value);
    if (p.canvasDefH.set) gStyle->SetCanvasDefH(p.canvasDefH.value);
    if (p.canvasDefW.set) gStyle->SetCanvasDefW(p.canvasDefW.value);
    if (p.padColor.set) gStyle->SetPadColor(p.padColor.value);
    if (p.padBorderSize.set) gStyle->SetPadBorderSize(p.padBorderSize.value);
    if (p.padBorderMode.set) gStyle->SetPadBorderMode(p.padBorderMode.value);
    if (p.padBottomMargin.set) gStyle->SetPadBottomMargin(p.padBottomMargin.value);
    if (p.padTopMargin.set) gStyle->SetPadTopMargin(p.padTopMargin.value);
    if (p.padLeftMargin.set) gStyle->SetPadLeftMargin(p.padLeftMargin.value);
    if (p.padRightMargin.set) gStyle->SetPadRightMargin(p.padRightMargin.value);
    if (p.padGridX.set) gStyle->SetPadGridX(p.padGridX.value);
    if (p.padGridY.set) gStyle->SetPadGridY(p.padGridY.value);
    if (p.padTickX.set) gStyle->SetPadTickX(p.padTickX.value);
    if (p.padTickY.set) gStyle->SetPadTickY(p.padTickY.value);
    if (p.frameFillStyle.set) gStyle->SetFrameFillStyle(p.frameFillStyle.value);
    if (p.frameFillColor.set) gStyle->SetFrameFillColor(p.frameFillColor.value);
    if (p.frameLineColor.set) gStyle->SetFrameLineColor(p.frameLineColor.value);
    if (p.frameLineStyle.set) gStyle->SetFrameLineStyle(p.frameLineStyle.value);
    if (p.frameLineWidth.set) gStyle->SetFrameLineWidth(p.frameLineWidth.value);
    if (p.frameBorderSize.set) gStyle->SetFrameBorderSize(p.frameBorderSize.value);
    if (p.frameBorderMode.set) gStyle->SetFrameBorderMode(p.frameBorderMode.value);
    if (p.ndivisionsX.set) gStyle->SetNdivisions(p.ndivisionsX.value, "X");
    if (p.lineWidth.set) gStyle->SetLineWidth(p.lineWidth.value);
    if (p.histLineWidth.set) gStyle->SetHistLineWidth(p.histLineWidth.value);
    if (p.legendFillColor.set) gStyle->SetLegendFillColor(p.legendFillColor.value);
    if (p.legendBorderSize.set) gStyle->SetLegendBorderSize(p.legendBorderSize.value);
    if (p.legendFont.set) gStyle->SetLegendFont(p.legendFont.value);
    if (p.legendTextSize.set) gStyle->SetLegendTextSize(p.legendTextSize.value);
    if (p.markerSize.set) gStyle->SetMarkerSize(p.markerSize.value);
    if (p.markerStyle.set) gStyle->SetMarkerStyle(p.markerStyle.value);
    if (p.labelSizeX.set) gStyle->SetLabelSize(p.labelSizeX.value, "X");
    if (p.labelSizeY.set) gStyle->SetLabelSize(p.labelSizeY.value, "Y");
    if (p.labelOffsetX.set) gStyle->SetLabelOffset(p.labelOffsetX.value, "X");
    if (p.labelOffsetY.set) gStyle->SetLabelOffset(p.labelOffsetY.value, "Y");
    if (p.labelFontX.set) gStyle->SetLabelFont(p.labelFontX.value, "X");
    if (p.labelFontY.set) gStyle->SetLabelFont(p.labelFontY.value, "Y");
    if (p.titleBorderSize.set) gStyle->SetTitleBorderSize(p.titleBorderSize.value);
    if (p.titleFontT.set) gStyle->SetTitleFont(p.titleFontT.value, "T");
    if (p.titleFontX.set) gStyle->SetTitleFont(p.titleFontX.value, "X");
    if (p.titleFontY.set) gStyle->SetTitleFont(p.titleFontY.value, "Y");
    if (p.titleAlign.set) gStyle->SetTitleAlign(p.titleAlign.value);
    if (p.titleX.set) gStyle->SetTitleX(p.titleX.value);
    if (p.titleSizeT.set) gStyle->SetTitleSize(p.titleSizeT.value, "T");
    if (p.titleSizeX.set) gStyle->SetTitleSize(p.titleSizeX.value, "X");
    if (p.titleSizeY.set) gStyle->SetTitleSize(p.titleSizeY.value, "Y");
    if (p.titleOffsetX.set) gStyle->SetTitleOffset(p.titleOffsetX.value, "X");
    if (p.titleOffsetY.set) gStyle->SetTitleOffset(p.titleOffsetY.value, "Y");
    if (p.textSize.set) gStyle->SetTextSize(p.textSize.value);
    if (p.textFont.set) gStyle->SetTextFont(p.textFont.value);
    if (p.optStat.set) gStyle->SetOptStat(p.optStat.value);
    if (p.forceStyle) gROOT->ForceStyle();
}

StyleParams ThesisStyle()
{
    StyleParams p = Skeleton();
    p.canvasBorderSize = 5;
    p.canvasDefH = 600;
    p.canvasDefW = 700;
    p.padBottomMargin = 0.14;
    p.padTopMargin = 0.05;
    p.padLeftMargin = 0.13;
    p.padRightMargin = 0.04;
    p.frameLineWidth = 2;
    p.ndivisionsX = 505;
    p.lineWidth = 1;
    p.histLineWidth = 1;
    p.legendFillColor = 0;
    p.markerSize = 1.0;
    p.labelSizeX = 0.05;
    p.labelSizeY = 0.05;
    p.labelOffsetX = 0.009;
    p.labelOffsetY = 0.009;
    p.titleFontX = 132;
    p.titleFontY = 132;
    p.titleAlign = 33;
    p.titleX = .95;
    p.titleSizeT = 0.1;
    p.titleSizeX = 0.065;
    p.titleSizeY = 0.065;
    p.titleOffsetX = 1.;
    p.titleOffsetY = 0.85;
    p.textSize = 0.055;
    return p;
}

StyleParams FitStyle()
{
    StyleParams p = Skeleton();
    p.canvasBorderSize = 10;
    p.canvasDefH = 600;
    p.canvasDefW = 700;
    p.padBottomMargin = 0.15;
    p.padTopMargin = 0.06;
    p.padLeftMargin = 0.12;
    p.padRightMargin = 0.05;
    p.frameLineWidth = 2;
    p.ndivisionsX = 505;
    p.lineWidth = 2;
    p.histLineWidth = 2;
    p.legendBorderSize = 0;
    p.markerSize = 1.2;
    p.labelSizeX = 0.055;
    p.labelSizeY = 0.055;
    p.labelOffsetX = 0.010;
    p.labelOffsetY = 0.010;
    p.titleFontX = 132;
    p.titleFontY = 132;
    p.titleSizeX = 0.08;
    p.titleSizeY = 0.08;
    p.titleOffsetX = 0.9;
    p.titleOffsetY = 0.65;
    p.textSize = 0.08;
    return p;
}

StyleParams ComparisonStyle()
{
    StyleParams p = Skeleton();
    p.canvasBorderSize = 10;
    p.canvasDefH = 600;
    p.canvasDefW = 700;
    p.padBottomMargin = 0.18;
    p.padTopMargin = 0.08;
    p.padLeftMargin = 0.16;
    p.padRightMargin = 0.05;
    p.frameLineWidth = 1;
    p.ndivisionsX = 505;
    p.legendBorderSize = 0;
    p.markerSize = 1.2;
    p.labelSizeX = 0.07;
    p.labelSizeY = 0.07;
    p.labelOffsetX = 0.008;
    p.labelOffsetY = 0.008;
    p.titleFontT = 132;
    p.titleFontX = 132;
    p.titleFontY = 132;
    p.titleSizeT = 0.08;
    p.titleSizeX = 0.08;
    p.titleSizeY = 0.08;
    p.titleOffsetX = 1.0;
    p.titleOffsetY = 1.0;
    p.textSize = 0.08;
    p.titleAlign = 33;
    p.titleX = 0.99;
    return p;
}

StyleParams TrackStyle()
{
    StyleParams p = Skeleton();
    p.canvasBorderSize = 10;
    p.canvasDefH = 600;
    p.canvasDefW = 700;
    p.padBottomMargin = 0.18;
    p.padTopMargin = 0.08;
    p.padLeftMargin = 0.15;
    p.padRightMargin = 0.04;
    p.frameLineWidth = 1;
    p.ndivisionsX = 505;
    p.legendFillColor = 0;
    p.legendBorderSize = 0;
    p.markerSize = 1.;
    p.labelSizeX = 0.07;
    p.labelSizeY = 0.07;
    p.labelOffsetX = 0.008;
    p.labelOffsetY = 0.008;
    p.titleFontT = 132;
    p.titleFontX = 132;
    p.titleFontY = 132;
    p.titleSizeT = 0.07;
    p.titleSizeX = 0.08;
    p.titleSizeY = 0.08;
    p.titleOffsetX = 1.0;
    p.titleOffsetY = 0.9;
    p.titleAlign = 33;
    p.titleX = .9;
    p.textSize = 0.08;
    return p;
}

StyleParams BarlowStyle(int canvasDefW, double titleOffsetY)
{
    StyleParams p = Skeleton();
    p.canvasBorderSize = 5;
    p.canvasDefH = 800;
    p.canvasDefW = canvasDefW;
    p.padBottomMargin = 0.16;
    p.padTopMargin = 0.08;
    p.padLeftMargin = 0.15;
    p.padRightMargin = 0.05;
    p.frameLineWidth = 2;
    p.ndivisionsX = 510;
    p.lineWidth = 1;
    p.histLineWidth = 1;
    p.legendFillColor = 0;
    p.markerSize = 1.0;
    p.labelSizeX = 0.055;
    p.labelSizeY = 0.055;
    p.labelOffsetX = 0.008;
    p.labelOffsetY = 0.008;
    p.titleFontX = 132;
    p.titleFontY = 132;
    p.titleSizeT = 0.055;
    p.titleSizeX = 0.075;
    p.titleSizeY = 0.075;
    p.titleOffsetX = 1.;
    p.titleOffsetY = titleOffsetY;
    p.textSize = 0.06;
    return p;
}

StyleParams GridTrailingTweak()
{
    StyleParams p;
    p.padBottomMargin = 0.2;
    p.padTopMargin = 0.08;
    p.padLeftMargin = 0.16;
    p.padRightMargin = 0.04;
    p.labelOffsetY = 0.03;
    return p;
}

} // namespace gxana
