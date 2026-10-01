// gStyle snapshots for tests/macros/test_macro_styles.py.
// usage (repo root): root -l -b -q rootlogon.C 'tests/macros/style_snapshot.C("MACRO","CALLS","OUTDIR")'
// Loads MACRO (.L). CALLS is NAME=EXPRESSION[|NAME=EXPRESSION...]. For each call and each
// start state ("default": the style this process started with; "dirty": that style after
// DirtyStart) the style is restored, the start state applied, EXPRESSION run, and
// OUTDIR/NAME__<start>.json written: TBufferJSON::ToJSON(gStyle) and gROOT->GetForceStyle().
#include <TBufferJSON.h>
#include <TROOT.h>
#include <TString.h>
#include <TStyle.h>
#include <TSystem.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace style_snapshot_detail {

TStyle* gPristine = nullptr;
std::string gPristineDump;

std::string Dump()
{
    return std::string(TBufferJSON::ToJSON(gStyle).Data()) + "\nForceStyle=" +
           std::to_string(gROOT->GetForceStyle()) + "\n";
}

void Fail(const std::string& what, int code)
{
    std::cerr << "STYLE SNAPSHOT FAILED: " << what << std::endl;
    gSystem->Exit(code);
}

// Back to the style the process started with; the check proves the restore is complete.
void Reset()
{
    gPristine->Copy(*gStyle);
    gROOT->ForceStyle(kFALSE);
    if (Dump() != gPristineDump) Fail("gStyle reset incomplete", 2);
}

// Every gStyle member that a legacy style body or a gxana::StyleParams field writes gets a
// value that differs from ROOT's default and from every value any of them writes, so a member
// the style under test must leave alone keeps a value no style could have written.
void DirtyStart()
{
    gStyle->SetCanvasColor(3);
    gStyle->SetCanvasBorderSize(3);
    gStyle->SetCanvasBorderMode(1);
    gStyle->SetCanvasDefH(333);
    gStyle->SetCanvasDefW(444);
    gStyle->SetPadColor(3);
    gStyle->SetPadBorderSize(3);
    gStyle->SetPadBorderMode(1);
    gStyle->SetPadBottomMargin(0.301);
    gStyle->SetPadTopMargin(0.302);
    gStyle->SetPadLeftMargin(0.303);
    gStyle->SetPadRightMargin(0.304);
    gStyle->SetPadGridX(1);
    gStyle->SetPadGridY(1);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
    gStyle->SetGridStyle(5);
    gStyle->SetGridWidth(3);
    gStyle->SetFrameFillStyle(3001);
    gStyle->SetFrameFillColor(3);
    gStyle->SetFrameLineColor(3);
    gStyle->SetFrameLineStyle(3);
    gStyle->SetFrameLineWidth(3);
    gStyle->SetFrameBorderSize(3);
    gStyle->SetFrameBorderMode(1);
    gStyle->SetNdivisions(333, "X");
    gStyle->SetLineWidth(3);
    gStyle->SetHistLineWidth(3);
    gStyle->SetLegendFillColor(3);
    gStyle->SetLegendBorderSize(3);
    gStyle->SetLegendFont(22);
    gStyle->SetLegendTextSize(0.0333);
    gStyle->SetMarkerSize(3.3);
    gStyle->SetMarkerStyle(5);
    gStyle->SetLabelSize(0.0301, "X");
    gStyle->SetLabelSize(0.0302, "Y");
    gStyle->SetLabelOffset(0.0303, "X");
    gStyle->SetLabelOffset(0.0304, "Y");
    gStyle->SetLabelFont(22, "X");
    gStyle->SetLabelFont(23, "Y");
    gStyle->SetTitleBorderSize(3);
    gStyle->SetTitleFont(22, "T");
    gStyle->SetTitleFont(23, "X");
    gStyle->SetTitleFont(32, "Y");
    gStyle->SetTitleFont(33, "Z");
    gStyle->SetTitleAlign(11);
    gStyle->SetTitleX(0.333);
    gStyle->SetTitleSize(0.0311, "T");
    gStyle->SetTitleSize(0.0312, "X");
    gStyle->SetTitleSize(0.0313, "Y");
    gStyle->SetTitleOffset(1.33, "X");
    gStyle->SetTitleOffset(1.44, "Y");
    gStyle->SetTextSize(0.0333);
    gStyle->SetTextFont(22);
    gStyle->SetOptStat(111111);
    gStyle->SetOptFit(111);
}

} // namespace style_snapshot_detail

void style_snapshot(const char* macro, const char* calls, const char* outdir)
{
    using namespace style_snapshot_detail;
    gPristine = new TStyle(*gStyle);
    gPristineDump = Dump();
    int err = 0;
    gROOT->ProcessLine(Form(".L %s", macro), &err);
    if (err != 0) Fail(std::string("cannot load ") + macro, 3);
    gSystem->mkdir(outdir, kTRUE);
    std::stringstream list(calls);
    std::string call;
    while (std::getline(list, call, '|')) {
        const auto eq = call.find('=');
        if (eq == std::string::npos) Fail("call without NAME=: " + call, 4);
        const std::string name = call.substr(0, eq), expr = call.substr(eq + 1);
        for (const char* start : {"default", "dirty"}) {
            Reset();
            if (std::string(start) == "dirty") DirtyStart();
            gROOT->ProcessLine(expr.c_str(), &err);
            if (err != 0) Fail("cannot run " + expr, 5);
            std::ofstream(std::string(outdir) + "/" + name + "__" + start + ".json") << Dump();
        }
    }
}
