#ifndef GXANA_FIT_JOHNSON_H
#define GXANA_FIT_JOHNSON_H

// Mean and standard deviation of a RooJohnson (mu, lambda, gamma, delta), with the error
// expressions the mass-measurement macro prints. The expressions are the macros' text, kept
// operation for operation so the values stay bit-identical.

class RooRealVar;

namespace gxana {
namespace fit {

struct JohnsonMoments {
    double mean;     // mu - lambda*exp(1/(2 delta^2))*sinh(gamma/delta)
    double meanErr;  // diagonal propagation of the four getError() values (no correlations)
    double sigma;    // sqrt(lambda^2/2 (exp(delta^-2)-1)(exp(delta^-2) cosh(2 gamma/delta)+1))
    double sigmaErr; // legacy: sigma*sqrt((delta/delta_err)^2 + (gamma_err/gamma)^2); infinite when
                     // delta is constant (getError() == 0). See docs/KNOWN_ISSUES.md.
};

JohnsonMoments Moments(const RooRealVar& mu, const RooRealVar& lambda, const RooRealVar& gamma,
                       const RooRealVar& delta);

} // namespace fit
} // namespace gxana

#endif
