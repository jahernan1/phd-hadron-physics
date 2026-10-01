#include "gxana/common/Overlay.h"

#include <TLegend.h>
#include <TPad.h>

namespace gxana {

// The compare_plot body (simulation/validation/in_out_test.C), statement order kept.
TCanvas* DrawOverlay(const TH1* first, const TH1* second, const OverlayOpts& opts)
{
    TH1* merged_hist = (TH1*)first->Clone();
    TH1* merged_hist_mc = (TH1*)second->Clone();

    TCanvas* c = new TCanvas(opts.legendTitle.c_str(), opts.legendTitle.c_str(), 800, 700);
    c->SetGrid();

    if (opts.firstAsPoints) {
        merged_hist->SetMarkerStyle(20);
        merged_hist->SetMarkerColor(kBlack);
    } else {
        merged_hist->SetFillColorAlpha(kGray, 0.9);
        merged_hist->SetLineColor(kBlack);
    }
    merged_hist->GetYaxis()->SetMaxDigits(3);

    merged_hist_mc->SetFillColorAlpha(kAzure - 9, 0.6);
    merged_hist_mc->SetLineColor(kBlack);
    merged_hist_mc->Scale(merged_hist->Integral("width") / merged_hist_mc->Integral("width"));

    if (!opts.firstAsPoints)
        merged_hist->GetYaxis()->SetRangeUser(0, merged_hist->GetMaximum() * 1.2);
    merged_hist->SetTitle(opts.axisTitle.c_str());
    merged_hist->SetTitleOffset(1., "Y");
    if (opts.raiseMaximum && merged_hist->GetMaximum() < merged_hist_mc->GetMaximum())
        merged_hist->SetMaximum(merged_hist_mc->GetMaximum() * 1.2);
    if (opts.firstAsPoints) {
        merged_hist_mc->Draw("hist same");
        merged_hist->Draw("e1 same");
    } else {
        merged_hist->Draw("hist");
        merged_hist_mc->Draw("hist same");
    }

    TLegend* legend = opts.legendTopRight ? new TLegend(0.68, 0.73, 0.92, 0.91) : new TLegend(0.18, 0.73, 0.42, 0.91);
    legend->SetBorderSize(0);
    legend->SetTextSize(0.065);
    legend->SetHeader(opts.legendTitle.c_str(), "C");
    legend->AddEntry(merged_hist, opts.label1.c_str(), "f");
    legend->AddEntry(merged_hist_mc, opts.label2.c_str(), "f");
    if (opts.drawLegend)
        legend->Draw("same");

    gPad->Update();
    c->SaveAs((opts.saveName + ".pdf").c_str());
    return c;
}

} // namespace gxana
