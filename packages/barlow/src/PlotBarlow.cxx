#include "gxana/barlow/Barlow.h"

#include <TBox.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include <TPad.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>

#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace gxana {
namespace barlow {

// PlotXSecBarlow*.C SetStyle(), verbatim except the two per-family values.
void SetBarlowStyle(const BarlowPlotStyle& style)
{
    gStyle->SetCanvasColor(0);
    gStyle->SetCanvasBorderSize(5);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasDefH(800);
    gStyle->SetCanvasDefW(style.canvasDefW);

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
    gStyle->SetTitleOffset(style.titleOffsetY,"Y");

    gStyle->SetTextSize(0.06);
    gStyle->SetTextFont(132);

    gStyle->SetOptStat(0);

    gROOT->ForceStyle();
}

namespace {

void CheckSamePoints(const TGraphErrors* nominal, const TGraphErrors* variation, const std::string& path)
{
    if (nominal->GetN() != variation->GetN())
        throw std::runtime_error(path + ": " + std::to_string(variation->GetN()) + " points, the nominal has "
                                 + std::to_string(nominal->GetN()));
    for (int i = 0; i < nominal->GetN(); ++i)
        if (std::fabs(nominal->GetPointX(i) - variation->GetPointX(i)) > 1e-9)
            throw std::runtime_error(path + ": x of point " + std::to_string(i) + " differs from the nominal");
}

void WriteSigmaB(const std::string& path, const BarlowPlotSpec& spec, const std::vector<TGraphErrors*>& graphs,
                 const std::vector<TGraphErrors*>& barlowGraphs)
{
    FILE* out = std::fopen(path.c_str(), "w");
    if (!out)
        throw std::runtime_error("cannot write " + path);
    std::fprintf(out, "# id x y_nom ey_nom y_var ey_var sigma_B\n");
    for (size_t v = 0; v < barlowGraphs.size(); ++v)
        for (int i = 0; i < graphs[0]->GetN(); ++i)
            std::fprintf(out, "%s %.10g %.10g %.10g %.10g %.10g %.10g\n", spec.variations[v].first.c_str(),
                         graphs[0]->GetPointX(i), graphs[0]->GetPointY(i), graphs[0]->GetErrorY(i),
                         graphs[v + 1]->GetPointY(i), graphs[v + 1]->GetErrorY(i), barlowGraphs[v]->GetPointY(i));
    std::fclose(out);
}

// One canvas: nominal band and variations (top pad), sigma_B per variation (bottom pad).
// Lifted from plotTotXSecAndBarlow (diff = false) / plotDiffXSecAndBarlow (diff = true).
void DrawOne(const BarlowPlotSpec& spec, const std::vector<std::string>& files, bool diff,
             const std::string& title, const std::string& saveName)
{
    const BarlowPlotStyle& st = spec.style;
    std::vector<TGraphErrors*> graphs;
    for (const auto& file : files)
        graphs.push_back(new TGraphErrors(file.c_str()));
    for (size_t i = 1; i < graphs.size(); ++i)
        CheckSamePoints(graphs[0], graphs[i], files[i]);
    std::vector<TGraphErrors*> barlowGraphs;
    for (size_t i = 1; i < graphs.size(); ++i)
        barlowGraphs.push_back(calc_barlow(graphs[0], graphs[i]));
    WriteSigmaB(spec.outDir + "/" + saveName + ".txt", spec, graphs, barlowGraphs);

    const double xlo = diff ? 0 : 6.2, xhi = diff ? 2.5 : 11.6;
    TCanvas* c1 = st.canvasW > 0 ? new TCanvas("c1", "Graphs and Barlow", st.canvasW, st.canvasH)
                                 : new TCanvas("c1", "Graphs and Barlow");
    TPad* topPad = new TPad("topPad", "Top Pad", 0.0, 0.3, 1.0, 1.0);
    TPad* bottomPad = new TPad("bottomPad", "Bottom Pad", 0.0, 0.0, 1.0, 0.3);
    topPad->SetBottomMargin(0.001);
    bottomPad->SetTopMargin(0.001);
    bottomPad->SetBottomMargin(0.4);

    c1->cd();
    topPad->Draw();
    bottomPad->Draw();

    topPad->cd();
    const auto& lb = diff ? st.legendDiff : st.legendTot;
    TLegend* legend = new TLegend(lb[0], lb[1], lb[2], lb[3]);
    legend->SetFillStyle(0);
    legend->SetBorderSize(0);
    gPad->SetGrid();

    int colorIndex = 1;
    int cutIndex = 0;
    TMultiGraph* mg = new TMultiGraph();
    for (auto* graph : graphs) {
        graph->SetMarkerStyle(20);
        graph->SetMarkerColor(colorIndex);
        graph->SetLineColor(colorIndex);
        graph->GetXaxis()->SetLimits(6, 12);
        if (colorIndex == 1) {
            graph->SetFillColorAlpha(kBlack, 0.3);
            graph->SetLineWidth(3);
            mg->Add(graph, "AE3");
            legend->AddEntry(graph, "Nominal", "f");
        } else {
            mg->Add(graph, "P SAME");
            legend->AddEntry(graph, (spec.label + spec.variations[cutIndex].second).c_str(), "lep");
            cutIndex++;
        }
        colorIndex++;
        if (colorIndex == 5) colorIndex++;
    }
    if (diff) {
        gStyle->SetTitleAlign(33);
        gStyle->SetTitleX(.95);
        mg->SetTitle(title.c_str());
    }
    mg->Draw("ap");
    mg->GetXaxis()->SetLimits(xlo, xhi);
    legend->Draw();

    bottomPad->cd();
    gPad->SetGrid();
    TMultiGraph* mg1 = new TMultiGraph();
    colorIndex = 2;
    for (auto* barlowGraph : barlowGraphs) {
        barlowGraph->SetMarkerStyle(20);
        barlowGraph->SetMarkerColor(colorIndex);
        barlowGraph->SetLineColor(colorIndex);
        if (!diff) barlowGraph->GetXaxis()->SetLimits(6, 12);
        mg1->Add(barlowGraph, colorIndex == 2 ? "AP" : "P SAME");
        colorIndex++;
        if (colorIndex == 5) colorIndex++;
    }
    mg1->Draw("ap");
    mg1->GetXaxis()->SetLimits(xlo, xhi);

    double ymin = mg1->GetHistogram()->GetMinimum();
    double ymax = mg1->GetHistogram()->GetMaximum();
    double ysym = std::fabs(ymax) > std::fabs(ymin) ? std::fabs(ymax) : std::fabs(ymin);
    if (ysym < st.yFloor) ysym = st.yFloor;

    if (diff) {
        mg->GetYaxis()->SetTitle("d#sigma/dt (nb/GeV^{2} )");
        mg1->GetXaxis()->SetTitle("-t (GeV^{2 })");
        mg->SetMinimum(0.01);
    } else {
        mg->GetYaxis()->SetTitle("#sigma(#gamma p#rightarrow K^{+}K^{+}#Xi^{-}) (nb)");
        mg1->GetXaxis()->SetTitle("E_{#gamma} (GeV)");
    }
    mg->GetXaxis()->SetNdivisions(510);
    mg1->GetYaxis()->SetTitle("#sigma_{#it{B}}");
    mg1->GetYaxis()->CenterTitle(true);
    const double pad = diff ? st.yPadDiff : 0;
    mg1->GetYaxis()->SetRangeUser(-ysym - pad, ysym + pad);
    if (diff || st.totYNdiv) mg1->GetYaxis()->SetNdivisions(505);
    mg1->GetXaxis()->SetNdivisions(510);
    mg1->GetXaxis()->SetLabelSize(0.13);
    mg1->GetYaxis()->SetLabelSize(0.13);
    const double titleSize = diff ? 0.17 : 0.16;
    mg1->GetXaxis()->SetTitleSize(titleSize);
    mg1->GetYaxis()->SetTitleSize(titleSize);
    const auto& offsets = diff ? st.titleOffsetsDiff : st.titleOffsetsTot;
    mg1->GetXaxis()->SetTitleOffset(offsets[0]);
    mg1->GetYaxis()->SetTitleOffset(offsets[1]);

    // Shaded band at +-threshold (legacy TBox at +-4).
    TBox* box = new TBox(xlo, -spec.threshold, xhi, spec.threshold);
    box->SetFillColorAlpha(kAzure, 0.1);
    box->Draw();
    c1->SaveAs((spec.outDir + "/" + saveName + ".pdf").c_str());

    // The multigraphs own the graphs.
    delete c1;
    delete legend;
    delete box;
    delete mg;
    delete mg1;
}

} // namespace

void PlotBarlow(const BarlowPlotSpec& spec)
{
    if (spec.variations.empty())
        throw std::invalid_argument("no variations");
    std::vector<std::string> totFiles{spec.nominalDir + "/totxsec_weighted_output.txt"};
    for (const auto& v : spec.variations)
        totFiles.push_back(spec.varDir + "/weighted_totxsec_vary_" + v.first + ".txt");
    std::vector<std::vector<std::string>> diffFiles;
    for (const auto& en : spec.energies) {
        const std::string bin = "_emin_" + en.first + "_emax_" + en.second;
        std::vector<std::string> files{spec.nominalDir + "/weighted_diffxsec" + bin + ".txt"};
        for (const auto& v : spec.variations)
            files.push_back(spec.varDir + "/weighted_diffxsec_vary_" + v.first + bin + ".txt");
        diffFiles.push_back(files);
    }
    std::string missing;
    for (const auto& group : diffFiles)
        for (const auto& file : group)
            if (gSystem->AccessPathName(file.c_str())) missing += "\n  " + file;
    for (const auto& file : totFiles)
        if (gSystem->AccessPathName(file.c_str())) missing = "\n  " + file + missing;
    if (!missing.empty())
        throw std::runtime_error("missing tables:" + missing);

    gSystem->mkdir(spec.outDir.c_str(), true);
    SetBarlowStyle(spec.style);
    DrawOne(spec, totFiles, false, "", "barlow_weighted_totxsec_vary_" + spec.family);
    for (size_t e = 0; e < spec.energies.size(); ++e) {
        const auto& en = spec.energies[e];
        DrawOne(spec, diffFiles[e], true, "#bf{E_{#gamma} (GeV): (" + en.first + ", " + en.second + ")}",
                "barlow_weighted_diffxsec_vary_" + spec.family + "_emin_" + en.first + "_emax_" + en.second);
    }
}

} // namespace barlow
} // namespace gxana
