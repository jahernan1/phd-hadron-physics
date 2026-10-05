// Figures of the toy walkthrough (examples/toy/README.md), run by
// `gxana run xsection --channel toy --steps figures` (xsection.figures in config/xsection.yaml):
//   <plotDir>/toy_dsigma_dt.{pdf,png}    weighted dsigma/dt per beam-energy bin (statistical bars, grey band:
//                                  run-period systematic of syst_weighted_diffxsec_*) and the injected
//                                  A0*exp(-b t)
//   <plotDir>/toy_sigma_total.{pdf,png}  weighted total cross section per beam-energy bin, direct
//                                  (totxsec_weighted_output.txt) and integrated over -t
//                                  (intxsec_weighted_output.txt), and the injected value
// truthPath is the truth.txt written by examples/toy/make_toy.py. Plain ROOT, no gxana library.
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "TAxis.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TSystem.h"

namespace toyplot {

// Rows of a whitespace table after its header line.
std::vector<std::vector<double>> ReadTable(const std::string& path)
{
    std::vector<std::vector<double>> rows;
    std::ifstream in(path);
    std::string line;
    std::getline(in, line);
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::vector<double> row;
        double v;
        while (ss >> v)
            row.push_back(v);
        if (!row.empty())
            rows.push_back(row);
    }
    return rows;
}

// Sorted names in dir starting with prefix.
std::vector<std::string> List(const std::string& dir, const std::string& prefix)
{
    std::vector<std::string> names;
    void* d = gSystem->OpenDirectory(dir.c_str());
    if (!d)
        return names;
    while (const char* entry = gSystem->GetDirEntry(d))
        if (std::string(entry).rfind(prefix, 0) == 0)
            names.push_back(entry);
    gSystem->FreeDirectory(d);
    std::sort(names.begin(), names.end());
    return names;
}

// x, y, ex, ey columns (0, 1, 2, 3 by default) as a graph.
TGraphErrors* Graph(const std::vector<std::vector<double>>& rows, int ey = 3)
{
    auto* g = new TGraphErrors(rows.size());
    for (size_t i = 0; i < rows.size(); ++i) {
        g->SetPoint(i, rows[i][0], rows[i][1]);
        g->SetPointError(i, rows[i][2], rows[i][ey]);
    }
    return g;
}

} // namespace toyplot

int PlotToyXSec(std::string xsecDir, std::string label, std::string plotDir, std::string truthPath)
{
    using namespace toyplot;
    const std::string wdir = xsecDir + "/weighted_data/" + label;
    std::map<std::string, double> truth;
    {
        std::ifstream in(truthPath);
        std::string key;
        double value;
        std::string line;
        while (std::getline(in, line)) {
            std::istringstream ss(line);
            if (line.empty() || line[0] == '#' || !(ss >> key >> value))
                continue;
            truth[key] = value;
        }
    }
    for (const char* key : {"A0", "b", "t_min", "t_max"})
        if (!truth.count(key)) {
            std::cerr << "PlotToyXSec: no " << key << " in " << truthPath << std::endl;
            return 1;
        }
    const std::vector<std::string> tables = List(wdir, "weighted_diffxsec_");
    if (tables.empty()) {
        std::cerr << "PlotToyXSec: no weighted_diffxsec_* table in " << wdir << std::endl;
        return 1;
    }
    gSystem->mkdir(plotDir.c_str(), true);
    gStyle->SetOptStat(0);

    const double a0 = truth["A0"], b = truth["b"], tMin = truth["t_min"], tMax = truth["t_max"];
    TCanvas c1("c1", "dsigma/dt", 500 * tables.size(), 450);
    c1.Divide(tables.size(), 1);
    for (size_t i = 0; i < tables.size(); ++i) {
        c1.cd(i + 1);
        gPad->SetLogy();
        gPad->SetLeftMargin(0.15);
        TGraphErrors* stat = Graph(ReadTable(wdir + "/" + tables[i]));
        // syst_weighted_diffxsec_*: x, y, ex, syst, ey, S (gxana_xsection.syst_tables).
        const auto systRows = ReadTable(wdir + "/syst_" + tables[i]);
        // weighted_diffxsec_emin_<E>_emax_<E>.txt
        double emin = 0, emax = 0;
        std::sscanf(tables[i].c_str(), "weighted_diffxsec_emin_%lf_emax_%lf", &emin, &emax);
        stat->SetTitle(Form("%.2f < E_{#gamma} < %.2f GeV;-t (GeV^{2});d#sigma/dt (nb/GeV^{2})", emin, emax));
        stat->SetMarkerStyle(20);
        stat->Draw("AP");
        stat->GetXaxis()->SetLimits(0, tMax + 0.1);
        stat->GetYaxis()->SetRangeUser(0.5 * a0 * std::exp(-b * tMax), 2 * a0 * std::exp(-b * tMin));
        if (!systRows.empty()) {
            TGraphErrors* syst = Graph(systRows);
            syst->SetFillColor(kGray);
            syst->Draw("2 SAME");
            stat->Draw("P SAME");
        }
        auto* curve = new TF1(("truth" + std::to_string(i)).c_str(), "[0]*exp(-[1]*x)", tMin, tMax);
        curve->SetParameters(a0, b);
        curve->SetLineColor(kRed);
        curve->Draw("SAME");
        // What a bin measures: the mean of dsigma/dt over the bin, not its value at the bin centre.
        auto* binMean = new TGraph(stat->GetN());
        for (int k = 0; k < stat->GetN(); ++k) {
            const double lo = stat->GetX()[k] - stat->GetEX()[k], hi = stat->GetX()[k] + stat->GetEX()[k];
            binMean->SetPoint(k, stat->GetX()[k], a0 / b * (std::exp(-b * lo) - std::exp(-b * hi)) / (hi - lo));
        }
        binMean->SetMarkerStyle(25);
        binMean->SetMarkerSize(1.6);
        binMean->SetMarkerColor(kRed);
        binMean->Draw("P SAME");
        if (i == 0) {
            auto* leg = new TLegend(0.4, 0.7, 0.88, 0.88);
            leg->AddEntry(stat, "toy, weighted over periods", "pe");
            leg->AddEntry(curve, "injected A_{0}e^{-bt}", "l");
            leg->AddEntry(binMean, "injected, mean over the bin", "p");
            leg->Draw();
        }
    }
    c1.Print((plotDir + "/toy_dsigma_dt.pdf").c_str());
    c1.Print((plotDir + "/toy_dsigma_dt.png").c_str());

    const auto direct = ReadTable(wdir + "/totxsec_weighted_output.txt");
    const auto integrated = ReadTable(wdir + "/intxsec_weighted_output.txt");
    if (direct.empty() || integrated.empty()) {
        std::cerr << "PlotToyXSec: no totxsec/intxsec_weighted_output.txt in " << wdir << std::endl;
        return 1;
    }
    const double sigma = a0 / b * (std::exp(-b * tMin) - std::exp(-b * tMax));
    TCanvas c2("c2", "sigma", 600, 450);
    TGraphErrors* gd = Graph(direct);
    TGraphErrors* gi = Graph(integrated);
    for (int i = 0; i < gi->GetN(); ++i)  // shift the integrated points to the right of the direct ones
        gi->SetPoint(i, gi->GetX()[i] + 0.1, gi->GetY()[i]);
    gd->SetTitle(";E_{#gamma} (GeV);#sigma (nb)");
    gd->SetMarkerStyle(20);
    gi->SetMarkerStyle(24);
    gi->SetMarkerColor(kBlue);
    gi->SetLineColor(kBlue);
    gd->Draw("AP");
    gd->GetYaxis()->SetRangeUser(0, 1.5 * sigma);
    gi->Draw("P SAME");
    TF1 line("injected", "[0]", gd->GetXaxis()->GetXmin(), gd->GetXaxis()->GetXmax());
    line.SetParameter(0, sigma);
    line.SetLineColor(kRed);
    line.Draw("SAME");
    TLegend leg(0.4, 0.15, 0.88, 0.35);
    leg.AddEntry(gd, "direct (energy bins)", "pe");
    leg.AddEntry(gi, "integrated over -t bins", "pe");
    leg.AddEntry(&line, Form("injected, %.2f nb", sigma), "l");
    leg.Draw();
    c2.Print((plotDir + "/toy_sigma_total.pdf").c_str());
    c2.Print((plotDir + "/toy_sigma_total.png").c_str());
    std::cout << "PlotToyXSec: wrote " << plotDir << "/toy_dsigma_dt.pdf and toy_sigma_total.pdf (+ .png)" << std::endl;
    return 0;
}
