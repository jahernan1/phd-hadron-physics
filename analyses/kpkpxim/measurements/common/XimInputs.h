#ifndef XIM_INPUTS_H
#define XIM_INPUTS_H

#include "gxana/common/Paths.h"
#include "gxana/common/Periods.h"
#include "gxana/xsection/YieldFit.h" // gxana::xsec::SetFitStyle

#include <ROOT/RDataFrame.hxx>
#include <TFile.h>
#include <TStyle.h>
#include <TLatex.h>

#include <stdexcept>
#include <string>
#include <vector>

// The run periods of $GXANA_OUTPUT/kpkpxim/config/channel.kv (gxana config export), in
// config/periods.yaml order: output directory and flat-tree stem.
struct XimPeriod {
    std::string dir;  // directory name inside the output ROOT files
    std::string stem; // flat-tree stem
};

inline const std::vector<XimPeriod>& XimPeriods()
{
    static const std::vector<XimPeriod> periods = [] {
        const auto info = gxana::ChannelInfo::Load("kpkpxim");
        std::vector<XimPeriod> out;
        for (const auto& p : info.PeriodNames())
            out.push_back({info.PeriodValue(p, "dir"), info.Stem(p)});
        return out;
    }();
    return periods;
}

inline std::string XimTreeDir() { return gxana::EnvPath("GXANA_DATA", "flatTrees/"); }

// Plain data tree (weight: hybrid_combo).
inline auto XimDataFrame(const std::string& stem, const std::string& delim = "kphighrap")
{
    return ROOT::RDataFrame("flatTree_kpkpxim", XimTreeDir() + "flatTree_" + stem + "_nominal_" + delim + ".root");
}

// Post-Q-factor data tree (weight: qvalue_acc = hybrid_combo * qvalue_decayxim_M).
inline auto XimQvalFrame(const std::string& stem)
{
    return ROOT::RDataFrame("flatTree_kpkpxim",
                            gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/") + stem + "_nominal_kphighrap_1111111/postQVal_flatTree_" +
                                stem + "_nominal_kphighrap_1111111.root")
        .Define("qvalue_acc", "hybrid_combo*qvalue_decayxim_M");
}

// Reconstructed gen_amp MC (weight: hybrid_combo).
inline auto XimMcFrame(const std::string& stem, const std::string& delim = "kphighrap")
{
    return ROOT::RDataFrame("flatTree_kpkpxim",
                            XimTreeDir() + "flatTree_" + stem + "_gen_amp_V2_ac_YstarRest_nominal_" + delim + ".root");
}

// Thrown gen_amp MC (unweighted).
inline auto XimThrownFrame(const std::string& stem)
{
    return ROOT::RDataFrame("flatTree_thrown_kpkpxim", XimTreeDir() + "flatTree_thrown_" + stem + "_gen_amp_V2_ac_YstarRest.root")
        .Define("decayxim_M", "decayxim_p4.M()");
}

inline TFile* XimOpen(const char* name, const char* mode)
{
    TFile* f = TFile::Open(name, mode);
    if (!f || f->IsZombie()) throw std::runtime_error(std::string("cannot open ") + name);
    return f;
}

// Plot style of the original macros: their setStyle() is statement for statement
// gxana::xsec::SetFitStyle() (checked against both copies before the port).
inline void setStyle() { gxana::xsec::SetFitStyle(); }

#endif
