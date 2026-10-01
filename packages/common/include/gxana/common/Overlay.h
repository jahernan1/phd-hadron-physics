#ifndef GXANA_COMMON_OVERLAY_H
#define GXANA_COMMON_OVERLAY_H

// The two-histogram comparison plot (`compare_plot`) of simulation/validation/compare_iters.C,
// compare_iters_2D.C, in_out_test.C and make_plot_RF.C: clones of both histograms, the second
// scaled to the first's area, on an 800x700 grid canvas named after the legend title, saved as
// <saveName>.pdf. No gStyle change: the caller sets the pad margins and style first.

#include <TCanvas.h>
#include <TH1.h>

#include <string>

namespace gxana {

struct OverlayOpts {
    std::string saveName;       // written as saveName + ".pdf"
    std::string legendTitle;    // canvas name and title; legend header (centred)
    std::string axisTitle;      // SetTitle of the first histogram
    std::string label1, label2; // legend entries, option "f"
    bool legendTopRight = true; // NDC (0.68,0.73,0.92,0.91); false: (0.18,0.73,0.42,0.91)
    bool drawLegend = true;     // false: the legend is built but not drawn (compare_iters.C)
    bool firstAsPoints = false; // first: markers 20 drawn "e1 same" after the second, no y range;
                                // otherwise grey fill drawn "hist" first, y range 0..1.2*max
    bool raiseMaximum = true;   // first->SetMaximum(1.2 * second max) when the second is higher
};

// Returns the canvas (left open).
TCanvas* DrawOverlay(const TH1* first, const TH1* second, const OverlayOpts& opts);

} // namespace gxana

#endif
