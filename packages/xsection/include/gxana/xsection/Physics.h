#ifndef GXANA_XSECTION_PHYSICS_H
#define GXANA_XSECTION_PHYSICS_H

#include <string>

namespace gxana {
namespace xsec {

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
};

// Target atoms per barn, atoms * Na * (zMax - zMin) * density * 1e-24 / molarMass, with the
// operations in the order of the legacy expression (zMax - zMin evaluated first).
double TargetDensity(const Target& target);

// S6 transition: the kpkpxim values the package used before the channel config passed them.
inline XSecPhysics LegacyXSecPhysics()
{
    return {"(hybrid_combo)*(decayxim_M>1.3&&decayxim_M<1.35)", "qvalue_decayxim_M", 0.641, 0.005,
            {50.4, 79.1, 70.08e-3, 2.01588, 2}};
}

} // namespace xsec
} // namespace gxana

#endif
