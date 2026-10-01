#ifndef GXANA_COMMON_PERIODHISTS_H
#define GXANA_COMMON_PERIODHISTS_H

// The RDataFrame fill the analysis macros copied as save_from_flattrees: one tree, an ordered
// list of Define/Filter steps, optional filtered sub-frames, 1-D/2-D histograms written with
// Write(key, kOverwrite) in the listed order. Library GxanaPeriodHists (links RDataFrame).

#include "gxana/common/Periods.h"

#include <TDirectory.h>
#include <TFile.h>

#include <string>
#include <utility>
#include <vector>

namespace gxana {

// define empty: Filter(expr); otherwise Define(define, expr).
struct Step {
    std::string define;
    std::string expr;
};

struct HistDef {
    std::string key;                  // Write() key
    std::string model;                // model name = stored object name (legacy fills use "")
    std::string title;                // "title;x;y"
    std::vector<std::string> columns; // 1 (Histo1D) or 2 (Histo2D)
    std::vector<double> axes;         // nx,xlo,xhi[,ny,ylo,yhi]
    std::string weight;               // weight column; "" = unweighted
    std::string frame;                // "" = main frame, else a FillSpec::frames name
};

struct FillSpec {
    std::string tree;
    std::vector<Step> steps;                                // applied in order
    std::vector<std::pair<std::string, std::string>> frames; // name -> Filter(expr) on the main frame
    std::vector<HistDef> hists;                             // booked, then written in this order
};

// Fills spec from `file` and writes every histogram into `out` (out->cd() first; gDirectory is
// left at `out`). nThreads > 0 calls ROOT::EnableImplicitMT(nThreads) first, as the legacy
// save_from_flattrees did.
void FillHists(const std::string& file, const FillSpec& spec, TDirectory* out, int nThreads = 16);

enum class Input { Data, MC, Thrown };

// mkdir every period's Dir() in `out` first (in order), then per period, per job in order:
// FillHists(<the period's file for job.first>, job.second, out/<Dir()>, nThreads).
void FillPeriodHists(const std::vector<Period>& periods, const std::vector<std::pair<Input, FillSpec>>& jobs,
                     TFile* out, int nThreads = 16);

} // namespace gxana

#endif
