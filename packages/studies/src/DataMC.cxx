// Port of selection/GetKinematicsDataMC_RF.C MakeStackedHist (lines 182-265) and setStyle
// (267-274) at the S7 base; statements in the macro's order.
#include "gxana/studies/DataMC.h"

#include "gxana/common/Style.h"

#include <TAxis.h>
#include <TCanvas.h>
#include <THStack.h>
#include <TLegend.h>
#include <TPad.h>

#include <cstdio>
#include <iostream>
#include <vector>

namespace gxana {
namespace studies {

void DrawStacked(TH1D* first, TH1D* second, const std::string& stack_title, const std::string& identifier,
                 const std::string& leg_pos, const std::string& leg_title, const std::string& plotdir)
{
    std::vector<TH1D*> arr_hist{first, second};
    std::string histName = arr_hist[0]->GetName();
    std::string saveName = plotdir + "/" + histName + "_" + identifier + "_ac.pdf";
    TCanvas* c = new TCanvas((histName + "_" + identifier).c_str(), (histName + "_" + identifier).c_str());
    THStack* hs = new THStack("hs", "");
    double scale_factor;

    if (leg_title == "Gen MC") {
        arr_hist[0]->SetFillColorAlpha(kGray, 0.8);
        arr_hist[0]->SetLineColor(kBlack);
        arr_hist[0]->RebinX();
        hs->Add(arr_hist[0], "hist");
    } else
        arr_hist[0]->RebinX();

    for (size_t i = 1; i < arr_hist.size(); i++) {
        arr_hist[i]->RebinX();
        std::cout << "Bin Width Data: " << arr_hist[0]->GetBinWidth(1) << std::endl;
        std::cout << "Bin Width MC: " << arr_hist[i]->GetBinWidth(1) << std::endl;

        if (arr_hist[i]->Integral("width") < arr_hist[0]->Integral("width"))
            scale_factor = arr_hist[0]->Integral("width") / arr_hist[i]->Integral("width");
        else
            scale_factor = arr_hist[i]->Integral("width") / arr_hist[0]->Integral("width");

        if (arr_hist[i]->GetMaximum() < arr_hist[0]->GetMaximum())
            arr_hist[i]->Scale(scale_factor);
        else {
            scale_factor = 1 / scale_factor;
            arr_hist[i]->Scale(scale_factor);
        }

        arr_hist[i]->SetLineColor(kBlack);
        arr_hist[i]->SetFillColorAlpha(kAzure - 9, 0.6);
        hs->Add(arr_hist[i], "hist");
    }

    if (leg_title == "Data") {
        arr_hist[0]->SetLineColor(kBlack);
        arr_hist[0]->SetMarkerColor(kBlack);
        arr_hist[0]->SetMarkerStyle(20);
        arr_hist[0]->SetMarkerSize(0.9);
        hs->Add(arr_hist[0], "e1");
    }

    double ymax = arr_hist[0]->GetMaximum() > arr_hist[1]->GetMaximum() ? arr_hist[0]->GetMaximum()
                                                                         : arr_hist[1]->GetMaximum();
    hs->SetTitle(stack_title.c_str());
    hs->Draw("nostack");
    hs->GetYaxis()->SetMaxDigits(3);
    hs->SetMaximum(ymax * 1.1);

    TLegend* legend;
    if (leg_pos == "tl")
        legend = new TLegend(0.2, 0.75, 0.4, 0.91);
    else
        legend = new TLegend(0.68, 0.76, 0.88, 0.92);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextSize(0.055);

    if (leg_title == "Data")
        legend->AddEntry(arr_hist[0], leg_title.c_str(), "lep");
    else
        legend->AddEntry(arr_hist[0], leg_title.c_str(), "f");
    legend->AddEntry(arr_hist[1], "Recon MC", "f");
    legend->Draw("same");

    gPad->Update();
    c->SaveAs(saveName.c_str());
}

void ApplyDataMCStyle()
{
    gxana::StyleParams p = gxana::CutStudyStyle();
    p.ndivisionsX.set = false;
    p.labelSizeX = 0.06;
    p.labelSizeY = 0.06;
    gxana::ApplyStyle(p);
}

} // namespace studies
} // namespace gxana
