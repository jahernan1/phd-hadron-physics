// Style harness. For every plot style, starting from ROOT's default
// style and from the state each legacy style leaves, the style function under test must
// leave gStyle exactly as the legacy function body does: the full TBufferJSON dump of
// gStyle and gROOT->GetForceStyle() are compared as strings. The legacy bodies below are
// verbatim copies of the original macros (only the function names changed); they are the
// reference and must not be edited.
// With GXANA_STYLE_DUMP_DIR set, every compared pair is also written there as JSON.
#include "gxana/barlow/Barlow.h"
#include "gxana/common/Style.h"
#include "gxana/studies/CutScan.h"
#include "gxana/studies/DataMC.h"
#include "gxana/systematics/PlotSpread.h"
#include "gxana/systematics/TrackHists.h"
#include "gxana/xsection/YieldFit.h"

#include <TBufferJSON.h>
#include <TLatex.h>
#include <TROOT.h>
#include <TStyle.h>

#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

// AnalysisNote/xsection/PlotFunctions.cpp:4-79 (SetStyle), renamed.
void LegacyThesis()
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

// AnalysisNote/xsection/FitFunctions.cpp:537-611 (setStyle), renamed.
void LegacyFit()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(600);
  gStyle->SetCanvasDefW(700);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadTopMargin   (0.06);
  gStyle->SetPadLeftMargin  (0.12);
  gStyle->SetPadRightMargin (0.05);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);

  gStyle->SetFrameFillStyle ( 0);
  gStyle->SetFrameFillColor ( 0);
  gStyle->SetFrameLineColor ( 1);
  gStyle->SetFrameLineStyle ( 0);
  gStyle->SetFrameLineWidth ( 1);
  gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);

  gStyle->SetNdivisions(505);

  gStyle->SetLineWidth(2);
  gStyle->SetHistLineWidth(2);
  gStyle->SetFrameLineWidth(2);
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.055,"X");
  gStyle->SetLabelSize(0.055,"Y");

  gStyle->SetLabelOffset(0.010,"X");
  gStyle->SetLabelOffset(0.010,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132);
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");

  gStyle->SetTitleSize(0.08,"X");
  gStyle->SetTitleSize(0.08,"Y");

  gStyle->SetTitleOffset(0.9,"X");
  gStyle->SetTitleOffset(0.65,"Y");

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

// AnalysisNote/xsection/PlotComboComparison.C:538-613 (style_format), renamed.
void LegacyComparison()
{
    //gStyle->SetCanvasPreferGL(true);
    gStyle->SetCanvasColor(0);
    gStyle->SetCanvasBorderSize(10);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasDefH(600);
    gStyle->SetCanvasDefW(700);

    gStyle->SetPadColor       (0);
    gStyle->SetPadBorderSize  (10);
    gStyle->SetPadBorderMode  (0);
    gStyle->SetPadBottomMargin(0.18);
    gStyle->SetPadTopMargin   (0.08);
    gStyle->SetPadLeftMargin  (0.16);
    gStyle->SetPadRightMargin (0.05);
    gStyle->SetPadGridX       (0);
    gStyle->SetPadGridY       (0);
    gStyle->SetPadTickX       (0);
    gStyle->SetPadTickY       (0);

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
    //gStyle->SetLegendFillColor(1);
  
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
  
    gStyle->SetTitleSize(0.08,"T");
    gStyle->SetTitleSize(0.08,"X");
    gStyle->SetTitleSize(0.08,"Y");
  
    gStyle->SetTitleOffset(1.0,"X");
    gStyle->SetTitleOffset(1.0,"Y");

    gStyle->SetTextSize(0.08);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(0.99);
    gStyle->SetTextFont(132);
    gStyle->SetOptStat(0);

    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextFont(132);
    latex->SetTextSize(0.08);
    latex->SetTextAlign(32);
    gROOT->ForceStyle();
}

// AnalysisNote/systematics/track_efficiency/get_track_efficiency.C:232-308 (style_format), renamed.
void LegacyTrack()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(600);
  gStyle->SetCanvasDefW(700);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.15);
  gStyle->SetPadRightMargin (0.04);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);

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
  gStyle->SetMarkerSize(1.);
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
  
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(0.9,"Y");
  gStyle->SetTitleAlign(33);
  gStyle->SetTitleX(.9);
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

// AnalysisNote/systematics/PlotXSecBarlowXimFlightSig.C:469-542 (SetStyle; also KHighRapidity,
// KLowRapidity, LambdaFlightSig: canvas width 700, Y title offset 0.85), renamed.
void LegacyBarlow700()
{
    gStyle->SetCanvasColor(0);
    gStyle->SetCanvasBorderSize(5);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasDefH(800);
    gStyle->SetCanvasDefW(700);

    gStyle->SetPadColor       (0);
    gStyle->SetPadBorderSize  (10);
    gStyle->SetPadBorderMode  (0);
    gStyle->SetPadBottomMargin(0.16);
    gStyle->SetPadTopMargin   (0.08);
    gStyle->SetPadLeftMargin  (0.15);
    gStyle->SetPadRightMargin (0.05);
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

    gStyle->SetNdivisions(510);

    gStyle->SetLineWidth(1);
    gStyle->SetHistLineWidth(1);
    //gStyle->SetLegendBorder(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLegendFillColor(0);
    gStyle->SetLegendFont(132);
    gStyle->SetLegendTextSize(0.06);
    gStyle->SetMarkerSize(1.0);
    gStyle->SetMarkerStyle(20);

    gStyle->SetLabelSize(0.055,"X");
    gStyle->SetLabelSize(0.055,"Y");

    gStyle->SetLabelOffset(0.008,"X");
    gStyle->SetLabelOffset(0.008,"Y");

    gStyle->SetLabelFont(132,"X");
    gStyle->SetLabelFont(132,"Y");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetTitleFont(132);
    gStyle->SetTitleFont(132,"X");
    gStyle->SetTitleFont(132,"Y");

    gStyle->SetTitleSize(0.055,"T");
    gStyle->SetTitleSize(0.075,"X");
    gStyle->SetTitleSize(0.075,"Y");

    gStyle->SetTitleOffset(0,"T");
    gStyle->SetTitleOffset(1.,"X");
    gStyle->SetTitleOffset(0.85,"Y");

    gStyle->SetTextSize(0.06);
    gStyle->SetTextFont(132);

    gStyle->SetOptStat(0);

    gROOT->ForceStyle();

    TLatex* latex = new TLatex();
    latex->SetNDC();
    //latex->SetTextFont(42);
    latex->SetTextSize(0.04);
    latex->SetTextAlign(32);
}

// AnalysisNote/systematics/PlotXSecBarlowChiSqNdf.C:472-545 (SetStyle; also MissingMass:
// canvas width 600, Y title offset 0.8), renamed.
void LegacyBarlow600()
{
    gStyle->SetCanvasColor(0);
    gStyle->SetCanvasBorderSize(5);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasDefH(800);
    gStyle->SetCanvasDefW(600);

    gStyle->SetPadColor       (0);
    gStyle->SetPadBorderSize  (10);
    gStyle->SetPadBorderMode  (0);
    gStyle->SetPadBottomMargin(0.16);
    gStyle->SetPadTopMargin   (0.08);
    gStyle->SetPadLeftMargin  (0.15);
    gStyle->SetPadRightMargin (0.05);
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

    gStyle->SetNdivisions(510);

    gStyle->SetLineWidth(1);
    gStyle->SetHistLineWidth(1);
    //gStyle->SetLegendBorder(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLegendFillColor(0);
    gStyle->SetLegendFont(132);
    gStyle->SetLegendTextSize(0.06);
    gStyle->SetMarkerSize(1.0);
    gStyle->SetMarkerStyle(20);

    gStyle->SetLabelSize(0.055,"X");
    gStyle->SetLabelSize(0.055,"Y");

    gStyle->SetLabelOffset(0.008,"X");
    gStyle->SetLabelOffset(0.008,"Y");

    gStyle->SetLabelFont(132,"X");
    gStyle->SetLabelFont(132,"Y");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetTitleFont(132);
    gStyle->SetTitleFont(132,"X");
    gStyle->SetTitleFont(132,"Y");

    gStyle->SetTitleSize(0.055,"T");
    gStyle->SetTitleSize(0.075,"X");
    gStyle->SetTitleSize(0.075,"Y");

    gStyle->SetTitleOffset(0,"T");
    gStyle->SetTitleOffset(1.,"X");
    gStyle->SetTitleOffset(0.8,"Y");

    gStyle->SetTextSize(0.06);
    gStyle->SetTextFont(132);

    gStyle->SetOptStat(0);

    gROOT->ForceStyle();

    TLatex* latex = new TLatex();
    latex->SetNDC();
    //latex->SetTextFont(42);
    latex->SetTextSize(0.04);
    latex->SetTextAlign(32);
}

// AnalysisNote/xsection/PlotFunctions.cpp:300-304: the gStyle writes that end every 3x3 grid
// function (same five lines in the comparison macros).
void LegacyGridTail()
{
  gStyle->SetPadBottomMargin(0.2);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.04);
  gStyle->SetLabelOffset(0.03,"Y");
}

// AnalysisNote/QFactors/scripts/GetQvalueSum.C:169-244 (setStyle; same body in utilities/YstarBWFitsData.C and
// analysis/event_selection/xim_vertex_cuts/make_plot.C), renamed.
void LegacyCutStudy()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(600);
  gStyle->SetCanvasDefW(700);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.05);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);

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
  //gStyle->SetLegendFillColor(1);
  
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

// AnalysisNote/analysis/event_selection/rapidity_cuts/PlotKPlusHighRapidity.C:117-193 (style_format; same body in the
// seven other rapidity_cuts/Plot*.C), renamed.
void LegacyDistribution()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  //gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(650);
  gStyle->SetCanvasDefW(800);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.17);
  gStyle->SetPadTopMargin   (0.04);
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
  gStyle->SetFrameLineWidth ( 1);
  //gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);

  gStyle->SetNdivisions(510);
  
  //gStyle->SetLineWidth(1);
  //gStyle->SetHistLineWidth(1);
  //gStyle->SetFrameLineWidth(2);
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.05,"X");
  gStyle->SetLabelSize(0.05,"Y");

  gStyle->SetLabelOffset(0.008,"X");
  gStyle->SetLabelOffset(0.008,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132,"T");
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");
  gStyle->SetTitleFont(132,"Z");
  
  gStyle->SetTitleSize(0.07,"T");
  gStyle->SetTitleSize(0.07,"X");
  gStyle->SetTitleSize(0.07,"Y");
  
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(0.9,"Y");

  gStyle->SetTextSize(0.07);
  gStyle->SetTextFont(132);

  gStyle->SetOptStat(0);

  gROOT->ForceStyle();

  TLatex* latex = new TLatex();
  latex->SetNDC();
  latex->SetTextFont(132);
  latex->SetTextSize(0.07);
  latex->SetTextAlign(32);
  gROOT->ForceStyle();
}

// AnalysisNote/analysis/event_selection/CutAnalysisRF.C:323-401 (setStyle), renamed.
void LegacyCutScan()
{
    //gStyle->SetCanvasPreferGL(true);
    gStyle->SetCanvasColor(0);
    gStyle->SetCanvasBorderSize(10);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasDefH(700);
    gStyle->SetCanvasDefW(800);

    gStyle->SetPadColor       (0);
    gStyle->SetPadBorderSize  (10);
    gStyle->SetPadBorderMode  (0);
    gStyle->SetPadBottomMargin(0.17);
    gStyle->SetPadTopMargin   (0.11);
    gStyle->SetPadLeftMargin  (0.2);
    gStyle->SetPadRightMargin (0.05);
    gStyle->SetPadGridX       (0);
    gStyle->SetPadGridY       (0);
    gStyle->SetPadTickX       (0);
    gStyle->SetPadTickY       (0);

    gStyle->SetFrameFillStyle ( 0);
    gStyle->SetFrameFillColor ( 0);
    gStyle->SetFrameLineColor ( 1);
    gStyle->SetFrameLineStyle ( 0);
    gStyle->SetFrameLineWidth ( 1);
    gStyle->SetFrameBorderSize(10);
    gStyle->SetFrameBorderMode( 0);
    //gStyle->SetTickSize( 0.05);

    gStyle->SetNdivisions(505);
    //gStyle->SetLineWidth(1);
    //gStyle->SetHistLineWidth(1);
    //gStyle->SetFrameLineWidth(2);
    //gStyle->SetLegendFillColor(1);
  
    gStyle->SetLegendBorderSize(0);
    gStyle->SetLegendFont(132);
    gStyle->SetLegendTextSize(0.06);
    gStyle->SetMarkerSize(1.);
    gStyle->SetMarkerStyle(24);

    gStyle->SetLabelSize(0.055,"X");
    gStyle->SetLabelSize(0.055,"Y");

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
    gStyle->SetTitleOffset(0.9,"X");
    gStyle->SetTitleOffset(1.2,"Y");

    gStyle->SetTextSize(0.09);
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);
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

// AnalysisNote/analysis/GetKinematicsDataMC_RF.C:265-341 (setStyle), renamed.
void LegacyDataMC()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(600);
  gStyle->SetCanvasDefW(700);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
  gStyle->SetPadRightMargin (0.05);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);

  gStyle->SetFrameFillStyle ( 0);
  gStyle->SetFrameFillColor ( 0);
  gStyle->SetFrameLineColor ( 1);
  gStyle->SetFrameLineStyle ( 0);
  gStyle->SetFrameLineWidth ( 1);
  gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);

  //gStyle->SetNdivisions(510);

  //gStyle->SetLineWidth(1);
  //gStyle->SetHistLineWidth(1);
  //gStyle->SetFrameLineWidth(2);
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.06,"X");
  gStyle->SetLabelSize(0.06,"Y");

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
  latex->SetIndiceSize(0.2);
  gROOT->ForceStyle();
}

TStyle* gPristine = nullptr;

void Reset()
{
    gPristine->Copy(*gStyle);
    gROOT->ForceStyle(kFALSE);
}

std::string Dump()
{
    return std::string(TBufferJSON::ToJSON(gStyle).Data()) + "\nForceStyle=" +
           std::to_string(gROOT->GetForceStyle()) + "\n";
}

using Fn = std::function<void()>;
using Named = std::pair<std::string, Fn>;

// Start states: ROOT's default style, then the state each legacy style leaves.
std::vector<Named> StartStates()
{
    return {{"default", [] {}},
            {"thesis", LegacyThesis},
            {"fit", LegacyFit},
            {"comparison", LegacyComparison},
            {"track", LegacyTrack},
            {"barlow700", LegacyBarlow700},
            {"barlow600", LegacyBarlow600},
            {"gridtail", LegacyGridTail}};
}

std::string StateAfter(const Fn& start, const Fn& f)
{
    Reset();
    start();
    f();
    return Dump();
}

void WriteDump(const std::string& name, const std::string& text)
{
    const char* dir = std::getenv("GXANA_STYLE_DUMP_DIR");
    if (dir && *dir) std::ofstream(std::string(dir) + "/" + name + ".json") << text;
}

std::string FirstDifference(const std::string& a, const std::string& b)
{
    size_t i = 0;
    while (i < a.size() && i < b.size() && a[i] == b[i]) ++i;
    const size_t from = a.rfind('\n', i) == std::string::npos ? 0 : a.rfind('\n', i) + 1;
    return "legacy: " + a.substr(from, a.find('\n', i) - from) + " | new: " +
           b.substr(from, b.find('\n', i) - from);
}

int failures = 0;
int compared = 0;

// legacy and candidate must leave the same gStyle from every start state.
void Compare(const std::string& name, const Fn& legacy, const Fn& candidate)
{
    for (const auto& start : StartStates()) {
        const std::string want = StateAfter(start.second, legacy);
        const std::string got = StateAfter(start.second, candidate);
        WriteDump(name + "_from_" + start.first + "_legacy", want);
        WriteDump(name + "_from_" + start.first + "_new", got);
        ++compared;
        if (want != got) {
            ++failures;
            std::cerr << "STYLE MISMATCH " << name << " from " << start.first << ": "
                      << FirstDifference(want, got) << "\n";
        }
    }
}

// The comparison must see a one-setter change and the ForceStyle flag.
void SensitivityCheck()
{
    Reset();
    const std::string base = Dump();
    gStyle->SetLabelOffset(0.0301, "Y");
    const std::string oneSetter = Dump();
    Reset();
    gROOT->ForceStyle();
    const std::string forced = Dump();
    Reset();
    if (base == oneSetter || base == forced || base != Dump()) {
        ++failures;
        std::cerr << "STYLE HARNESS INSENSITIVE\n";
    }
}

// The track style is file-local to TrackHists.cxx; CountAndDraw applies it first and then
// fails on the missing input, leaving gStyle as the style left it.
void CurrentTrackStyle()
{
    gxana::systematics::TrackSpec spec;
    spec.outDir = "/nonexistent/gxana_test_style";
    try {
        gxana::systematics::CountAndDraw(spec);
    } catch (const std::runtime_error&) {
    }
}

gxana::barlow::BarlowPlotStyle BarlowFamily(int canvasDefW, double titleOffsetY)
{
    gxana::barlow::BarlowPlotStyle style;
    style.canvasDefW = canvasDefW;
    style.titleOffsetY = titleOffsetY;
    return style;
}

} // namespace

int main()
{
    gPristine = new TStyle(*gStyle);
    SensitivityCheck();

    // The existing style functions against the legacy bodies.
    Compare("SetStyle", LegacyThesis, [] { gxana::SetStyle(); });
    Compare("SetFitStyle", LegacyFit, [] { gxana::xsec::SetFitStyle(); });
    Compare("StyleFormat", LegacyComparison, [] { gxana::systematics::StyleFormat(); });
    Compare("style_format", LegacyTrack, CurrentTrackStyle);
    Compare("SetBarlowStyle700", LegacyBarlow700, [] { gxana::barlow::SetBarlowStyle(BarlowFamily(700, 0.85)); });
    Compare("SetBarlowStyle600", LegacyBarlow600, [] { gxana::barlow::SetBarlowStyle(BarlowFamily(600, 0.8)); });

    // The presets applied with ApplyStyle against the legacy bodies.
    Compare("ThesisStyle", LegacyThesis, [] { gxana::ApplyStyle(gxana::ThesisStyle()); });
    Compare("FitStyle", LegacyFit, [] { gxana::ApplyStyle(gxana::FitStyle()); });
    Compare("ComparisonStyle", LegacyComparison, [] { gxana::ApplyStyle(gxana::ComparisonStyle()); });
    Compare("TrackStyle", LegacyTrack, [] { gxana::ApplyStyle(gxana::TrackStyle()); });
    Compare("BarlowStyle700", LegacyBarlow700, [] { gxana::ApplyStyle(gxana::BarlowStyle(700, 0.85)); });
    Compare("BarlowStyle600", LegacyBarlow600, [] { gxana::ApplyStyle(gxana::BarlowStyle(600, 0.8)); });
    Compare("GridTrailingTweak", LegacyGridTail, [] { gxana::ApplyStyle(gxana::GridTrailingTweak()); });
    // The presets of the analysis-macro styles against their legacy bodies.
    Compare("CutStudyStyle", LegacyCutStudy, [] { gxana::ApplyStyle(gxana::CutStudyStyle()); });
    Compare("DistributionStyle", LegacyDistribution, [] { gxana::ApplyStyle(gxana::DistributionStyle()); });
    // The style functions of the two studies (packages/studies) against the legacy setStyle bodies.
    Compare("ApplyCutScanStyle", LegacyCutScan, [] { gxana::studies::ApplyCutScanStyle(); });
    Compare("ApplyDataMCStyle", LegacyDataMC, [] { gxana::studies::ApplyDataMCStyle(); });

    // The fields commit 4c3adbe added: each set field writes its member, and an unset field writes nothing
    // (the members first get values that neither ROOT's default nor any start state has).
    const auto unusual = [] {
        gStyle->SetGridStyle(5);
        gStyle->SetGridWidth(3);
        gStyle->SetTitleFont(33, "Z");
        gStyle->SetOptFit(111);
    };
    Compare("NewFieldsSet",
            [] {
                gStyle->SetGridStyle(4);
                gStyle->SetGridWidth(2);
                gStyle->SetTitleFont(22, "Z");
                gStyle->SetOptFit(1);
            },
            [] {
                gxana::StyleParams p;
                p.gridStyle = 4;
                p.gridWidth = 2;
                p.titleFontZ = 22;
                p.optFit = 1;
                gxana::ApplyStyle(p);
            });
    Compare("NewFieldsUnset", unusual, [unusual] {
        unusual();
        gxana::ApplyStyle(gxana::StyleParams());
    });

    std::cout << "test_style: " << compared << " comparisons, " << failures << " mismatches\n";
    return failures == 0 ? 0 : 1;
}
