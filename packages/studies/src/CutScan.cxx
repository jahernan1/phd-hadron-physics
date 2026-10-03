// Port of archive/root_macros/CutAnalysisRF.C (GetCutAnalysis lines 63-117, rooFitHist 244-324,
// plotRatio 174-242, setStyle 327-345). Statements are kept in the macro's
// order; only the inputs became arguments.
#include "gxana/studies/CutScan.h"

#include "gxana/common/Style.h"
#include "gxana/fit/Fit.h"
#include "gxana/fit/Johnson.h"
#include "gxana/fit/Model.h"

#include <RooAbsReal.h>
#include <RooArgSet.h>
#include <RooDataHist.h>
#include <RooMsgService.h>
#include <RooPlot.h>
#include <RooRealVar.h>
#include <RooWorkspace.h>
#include <TArrow.h>
#include <TAxis.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TLine.h>
#include <TPad.h>
#include <TStyle.h>

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdexcept>

using namespace RooFit;

namespace gxana {
namespace studies {

namespace {

std::string Table(const std::string& pattern, const std::string& what)
{
    const auto at = pattern.find("{what}");
    if (at == std::string::npos)
        throw std::invalid_argument("table path needs {what}: " + pattern);
    return pattern.substr(0, at) + what + pattern.substr(at + 6);
}

const std::string& Param(const CutScanFit& fit, const std::string& name)
{
    const auto it = fit.params.find(name);
    if (it == fit.params.end())
        throw std::invalid_argument("cut scan: missing parameter " + name);
    return it->second;
}

// rooFitHist (CutAnalysisRF.C:244-324) without the unused min_mass.
void FitOne(TH1* hist, const std::string& histTitle, const CutScanFit& fit, double* sigYield, double* sigYieldErr,
            double* bkgYield, double* bkgYieldErr)
{
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooRealVar mass("mass", fit.massTitle.c_str(), fit.lo, fit.hi);
    RooDataHist* data = new RooDataHist("data", "Dataset of mass", mass, hist);
    RooPlot* massframe = mass.frame(RooFit::Title(histTitle.c_str()));
    w->import(RooArgSet(mass));
    massframe->GetYaxis()->SetMaxDigits(3);
    massframe->SetNdivisions(505);

    gxana::fit::BuildModel(*w, {
        gxana::fit::Chebychev("bkgd", "mass", {{"a0", Param(fit, "a0")}, {"a1", Param(fit, "a1")}}),
        gxana::fit::Johnson("xigaus", "mass", {"mu", Param(fit, "mu")}, {"lambda", Param(fit, "lambda")},
                            {"gamma", Param(fit, "gamma")}, {"delta", Param(fit, "delta")}),
        gxana::fit::Sum("model", {{{"nbkgd", Param(fit, "nbkgd")}, "bkgd"}, {{"nxi", Param(fit, "nxi")}, "xigaus"}})});
    gxana::fit::RunFit(*w->pdf("model"), *data, Extended(true), SumW2Error(true), PrintLevel(-1), PrintEvalErrors(-1),
                       Verbose(false), Warnings(false));
    data->plotOn(massframe, MarkerStyle(24), MarkerSize(0.4));
    w->pdf("model")->paramOn(massframe, Format("N", AutoPrecision(0)), Layout(0.45, 0.9, 0.85),
                             Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nbkgd"))));
    massframe->getAttText()->SetTextSize(0.08);
    massframe->getAttFill()->SetFillStyle(0);
    massframe->getAttLine()->SetLineColor(0);
    w->pdf("model")->plotOn(massframe, LineWidth(1));
    w->pdf("xigaus")->plotOn(massframe, LineWidth(1), LineStyle(kDotted),
                             Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent));
    w->pdf("bkgd")->plotOn(massframe, LineWidth(1), LineStyle(kDotted),
                           Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent));

    gxana::fit::JohnsonMoments xiMoments =
        gxana::fit::Moments(*w->var("mu"), *w->var("lambda"), *w->var("gamma"), *w->var("delta"));
    double xiMean = xiMoments.mean;
    double xiSigma = xiMoments.sigma;

    printf("Mean and Sigma: %f, %f \n", xiMean, xiSigma);
    double xCutL = xiMean - 2 * xiSigma;
    double xCutR = xiMean + 2 * xiSigma;
    w->var("mass")->setRange("signal", xCutL, xCutR);

    RooAbsReal* sig_sig = w->pdf("xigaus")->createIntegral(mass, RooFit::NormSet(mass), RooFit::Range("signal"));
    printf("Signal Fraction in 2Sigma Window: %f \n", sig_sig->getVal());
    printf("Signal Yield in 2Sigma Window: %f \n", w->var("nxi")->getVal() * sig_sig->getVal());

    *sigYield = w->var("nxi")->getVal() * sig_sig->getVal();
    *sigYieldErr = w->var("nxi")->getError() * sig_sig->getVal();

    RooAbsReal* bkg_sig = w->pdf("bkgd")->createIntegral(mass, RooFit::NormSet(mass), RooFit::Range("signal"));
    printf("Background Fraction of 2Sigma Window: %f \n", bkg_sig->getVal());
    printf("Background Yield in 2Sigma Window: %f \n", w->var("nbkgd")->getVal() * bkg_sig->getVal());

    *bkgYield = w->var("nbkgd")->getVal() * bkg_sig->getVal();
    *bkgYieldErr = w->var("nbkgd")->getError() * bkg_sig->getVal();

    double ypadmax = massframe->GetMaximum();
    TLine* cutLineL = new TLine(xCutL, 0.0, xCutL, ypadmax);
    cutLineL->SetLineWidth(1);
    cutLineL->SetLineColor(kRed + 1);
    massframe->addObject(cutLineL, " ");
    TLine* cutLineR = new TLine(xCutR, 0.0, xCutR, ypadmax);
    cutLineR->SetLineWidth(1);
    cutLineR->SetLineColor(kRed + 1);
    massframe->addObject(cutLineR, " ");

    massframe->Draw();
    massframe->SetMarkerStyle(24);
    massframe->SetMarkerSize(0.2);
}

} // namespace

const std::vector<std::string>& CutScanParams()
{
    static const std::vector<std::string> names{"a0", "a1", "mu", "lambda", "gamma", "delta", "nbkgd", "nxi"};
    return names;
}

void FitCutScan(const TH2& histIn, const CutScanFit& fit)
{
    if (fit.firstBin < 1 || fit.firstBin >= histIn.GetNbinsY())
        throw std::invalid_argument("--first-bin " + std::to_string(fit.firstBin) + " out of range: the scan has " +
                                    std::to_string(histIn.GetNbinsY()) + " bins");
    TH2* hist_XiMass_Cut = const_cast<TH2*>(&histIn);
    std::vector<TH1*> hist_XiMass;
    std::vector<double> ratioFOM;
    std::vector<double> ratioSB;
    FILE* fCutFOM = fopen(Table(fit.tables, "FOM").c_str(), "w+");
    FILE* fCutSB = fopen(Table(fit.tables, "SB").c_str(), "w+");
    FILE* fCutYield = fopen(Table(fit.tables, "Yield").c_str(), "w+");
    if (!fCutFOM || !fCutSB || !fCutYield)
        throw std::runtime_error("cannot write the tables " + fit.tables);
    int cnt = 0;
    double numBins = hist_XiMass_Cut->GetNbinsY() - fit.firstBin;

    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
    TCanvas* cut_Can = new TCanvas("cutscan_fits", "Cut Fits", 800, 1100);
    cut_Can->Divide(4, std::ceil(numBins / 4), 1e-4, 1e-4);

    for (int bin_i = fit.firstBin; bin_i < hist_XiMass_Cut->GetNbinsY() + 1; ++bin_i) {
        double sigYield, sigYieldErr, bkgYield, bkgYieldErr;
        double cut_val = hist_XiMass_Cut->GetYaxis()->GetBinCenter(bin_i) + hist_XiMass_Cut->GetYaxis()->GetBinWidth(bin_i) / 2;

        hist_XiMass.push_back(hist_XiMass_Cut->ProjectionX(("_px_" + std::to_string(cut_val)).c_str(), 1, bin_i, "e"));

        cut_Can->cd(cnt + 1);
        cut_Can->SetLeftMargin(0.07);
        cut_Can->SetRightMargin(0.07);

        const std::string text = std::to_string(cut_val);
        FitOne(hist_XiMass[cnt], (fit.panelLabel + " < " + text.substr(0, text.find_last_not_of('0') + 1)).c_str(), fit,
               &sigYield, &sigYieldErr, &bkgYield, &bkgYieldErr);

        std::cout << "Fit Results: " << cut_val << "\t" << sigYield << "\t" << bkgYield << "\n" << std::endl;

        ratioFOM.push_back(sigYield / sqrt(sigYield + bkgYield));
        ratioSB.push_back(sigYield / bkgYield);

        fprintf(fCutFOM, "%f \t %f \n", cut_val, ratioFOM[cnt]);
        fprintf(fCutSB, "%f \t %f \n", cut_val, ratioSB[cnt]);
        fprintf(fCutYield, "%f \t %f \n", cut_val, sigYield);
        cnt = cnt + 1;
    }

    fclose(fCutFOM);
    fclose(fCutSB);
    fclose(fCutYield);

    cut_Can->SaveAs(fit.gridPdf.c_str());
    cut_Can->Close();
}

void PlotCutScan(const std::string& tables, const std::string& plotTitle, double cutVal, const std::string& plotName,
                 const std::vector<std::string>& pdfs)
{
    TCanvas* c = new TCanvas(plotName.c_str(), plotName.c_str());
    TPad* pad1 = new TPad((plotName + "_Pad1").c_str(), "", 0, 0, 1, 1);
    TPad* pad2 = new TPad((plotName + "_Pad2").c_str(), "", 0, 0, 1, 1);
    pad2->SetFillStyle(4000);
    pad2->SetFrameFillStyle(0);
    pad1->SetGrid();

    TGraphErrors* g1 = new TGraphErrors(Table(tables, "FOM").c_str(), "%lg %lg");
    g1->SetTitle(plotTitle.c_str());
    g1->SetMarkerStyle(20);
    g1->SetMarkerSize(1.4);
    g1->SetDrawOption("APL");
    g1->SetMarkerColor(kBlack);
    g1->SetMarkerStyle(24);
    g1->SetLineWidth(2);
    g1->SetFillStyle(0);
    pad1->Draw();
    pad1->cd();
    g1->Draw("APL");

    TGraphErrors* g2 = new TGraphErrors(Table(tables, "SB").c_str(), "%lg %lg");
    g2->SetTitle(" ; ; N_{S}/N_{B}");
    g2->SetMarkerStyle(20);
    g2->SetMarkerSize(1.4);
    g2->SetDrawOption("APLY+");
    g2->SetMarkerColor(kRed + 1);
    g2->SetLineColor(kRed + 1);
    g2->SetLineWidth(2);
    g2->SetFillStyle(0);
    g2->GetYaxis()->SetAxisColor(kRed + 1);
    g2->GetYaxis()->SetTitleOffset(0.85);
    g2->GetYaxis()->SetTitleColor(kRed + 1);
    g2->GetYaxis()->SetLabelColor(kRed + 1);
    pad2->Draw();
    pad2->cd();
    g2->Draw("APLY+");

    c->Update();
    double ypadmin, ypadmax, xpadmin, xpadmax;
    pad2->GetRangeAxis(xpadmin, ypadmin, xpadmax, ypadmax);
    std::cout << "Pad Values: " << xpadmin << "\t " << xpadmax << "\t" << ypadmin << "\t" << ypadmax << std::endl;
    TLine* cutLine = new TLine(cutVal, ypadmin, cutVal, ypadmax);
    cutLine->SetLineWidth(4);
    cutLine->SetLineStyle(10);
    cutLine->SetLineColor(kBlue + 1);
    cutLine->Draw();

    TArrow* arCut = new TArrow(cutVal, (ypadmax + ypadmin) / 2, cutVal * 0.5, (ypadmax + ypadmin) / 2, 0.05, "|>");
    arCut->SetLineWidth(4);
    arCut->SetFillColor(kBlue + 1);
    arCut->SetLineColor(kBlue + 1);
    arCut->Draw();
    c->Update();
    for (const auto& pdf : pdfs)
        c->SaveAs(pdf.c_str());
}

void ApplyCutScanStyle()
{
    gxana::StyleParams p = gxana::ComparisonStyle();
    p.canvasDefH = 700;
    p.canvasDefW = 800;
    p.padBottomMargin = 0.17;
    p.padTopMargin = 0.11;
    p.padLeftMargin = 0.2;
    p.markerSize = 1.0;
    p.markerStyle = 24;
    p.labelSizeX = 0.055;
    p.labelSizeY = 0.055;
    p.titleX = 0.95;
    p.titleSizeT = 0.07;
    p.titleOffsetX = 0.9;
    p.titleOffsetY = 1.2;
    p.textSize = 0.09;
    gxana::ApplyStyle(p);
}

void ApplyCutScanPlotMargins()
{
    gStyle->SetPadBottomMargin(0.20);
    gStyle->SetPadTopMargin(0.03);
    gStyle->SetPadLeftMargin(0.18);
    gStyle->SetPadRightMargin(0.16);
    gStyle->SetTitleOffset(0.98, "Y");
}

} // namespace studies
} // namespace gxana
