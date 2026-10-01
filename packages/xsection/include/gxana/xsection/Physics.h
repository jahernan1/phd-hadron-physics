#ifndef GXANA_XSECTION_PHYSICS_H
#define GXANA_XSECTION_PHYSICS_H

#include <string>

namespace gxana {
namespace xsec {

// The fitted invariant mass: branch name and RooFit axis title.
struct Observable {
    std::string branch, title;
};

// Mass windows (GeV) of the YieldFit functions:
//   lo           lower edge of every fit variable, fit range and plot
//   mcHi         upper edge of the MC fit variable (RooFitMC, RooFitMCShapeSeed)
//   mcSignalHi   upper edge of the MC "signal" fit range
//   mcPlotHi     upper edge of the MC fit plot
//   dataHi       upper edge of the data fit variable and plot (start of the upper-edge scan)
//   dataEdge     the lower-edge scan of the data fit stops here
//   mcPdfDataLo  lower edge of the MCPdf data fit variable
struct MassWindows {
    double lo, mcHi, mcSignalHi, mcPlotHi, dataHi, dataEdge, mcPdfDataLo;
};

// Liquid target: z range (cm), density (g/cm^3), molar mass (g/mol), atoms per molecule.
struct Target {
    double zMin, zMax, density, molarMass, atoms;
};

// The channel facts the cross-section tables need (gxana_xsec_tables flags, from
// analyses/<channel>/config). Nothing here has a default: every channel names its own.
struct XSecPhysics {
    std::string gate;         // TTree::GetEntries selection a bin must pass to be fitted
    std::string qvalueBranch; // Q-factor branch of the data; "" = none (qval columns nan)
    double br, brErr;         // branching ratio of the reconstructed decay chain
    Target target;
    Observable obs;
    MassWindows windows;
};

// Target atoms per barn, atoms * Na * (zMax - zMin) * density * 1e-24 / molarMass, with the
// operations in the order of the legacy expression (zMax - zMin evaluated first).
double TargetDensity(const Target& target);

// S6 transition: the kpkpxim values the package used before the channel config passed them.
inline Observable LegacyObservable() { return {"decayxim_M", "M(#Lambda#pi^{-}) (GeV/c^{2})"}; }
inline MassWindows LegacyMassWindows() { return {1.27, 1.40, 1.38, 1.42, 1.45, 1.28, 1.275}; }
inline XSecPhysics LegacyXSecPhysics()
{
    return {"(hybrid_combo)*(decayxim_M>1.3&&decayxim_M<1.35)", "qvalue_decayxim_M", 0.641, 0.005,
            {50.4, 79.1, 70.08e-3, 2.01588, 2}, LegacyObservable(), LegacyMassWindows()};
}

} // namespace xsec
} // namespace gxana

#endif
