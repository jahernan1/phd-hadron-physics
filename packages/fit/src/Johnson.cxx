#include "gxana/fit/Johnson.h"

#include <RooRealVar.h>

#include <cmath>

namespace gxana {
namespace fit {

// Origin: AnalysisNote/analysis/analysis/cascade_properties/GetXimProperties.C (now
// analyses/kpkpxim/measurements/mass/FitMass.C), statements copied unchanged.
JohnsonMoments Moments(const RooRealVar& mu, const RooRealVar& lambda, const RooRealVar& gamma,
                       const RooRealVar& delta)
{
    using std::cosh; using std::exp; using std::pow; using std::sinh; using std::sqrt;
    double xiMu = mu.getVal(); double xiMuErr = mu.getError();
    double xiLambda = lambda.getVal(); double xiLambdaErr = lambda.getError();
    double xiDelta = delta.getVal(); double xiDeltaErr = delta.getError();
    double xiGamma = gamma.getVal(); double xiGammaErr = gamma.getError();
    double xiMean = xiMu - xiLambda * exp(1 / (2*pow(xiDelta,2)) ) * sinh(xiGamma/xiDelta);
    double xiMeanErr = sqrt( pow(xiMuErr,2)
                           + pow( -1* xiLambdaErr*exp(1 / (2*pow(xiDelta,2)))*sinh(xiGamma/xiDelta),2 )
                           + pow( -1* xiGammaErr* xiLambda *exp(1 / (2*pow(xiDelta,2)))*cosh(xiGamma/xiDelta) / xiDelta,2)
                           + pow(xiDeltaErr*xiLambda*exp(1 / (2*pow(xiDelta,2)))*(sinh(xiGamma/xiDelta) + xiGamma*xiDelta*cosh(xiGamma/xiDelta)) / pow(xiDelta,3) ,2));
    double xiSigma = sqrt( pow(xiLambda, 2)/2*(exp(pow(xiDelta, -2) ) - 1 )*(exp(pow(xiDelta, -2) )*cosh(2*xiGamma/xiDelta )+1));
    double xiSigmaErr = xiSigma * sqrt( pow( xiDelta/ xiDeltaErr, 2) + pow( xiGammaErr/xiGamma, 2) );
    return {xiMean, xiMeanErr, xiSigma, xiSigmaErr};
}

} // namespace fit
} // namespace gxana
