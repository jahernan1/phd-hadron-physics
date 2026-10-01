// ctest fit.cling: the library loads into the interpreter, and Moments() (compiled) equals
// the same FitMass.C statements run by the interpreter, bit for bit (the macros run them
// interpreted; a compiler contracting a*b+c differently would show here).
#include "gxana/fit/Johnson.h"
#include "gxana/fit/Model.h"

#include <RooRealVar.h>

#include <cmath>
#include <cstring>

int cling_fit()
{
    if (gxana::fit::Gaussian("g", "m", {"a", "1"}, {"b", ""}) != "Gaussian::g(m,a[1],b)") return 1;
    int bad = 0;
    const double cases[][8] = {{1.3221, 1.1e-4, 0.0051, 2.2e-4, -0.07, 0.031, 1.13, 0.09},
                               {1.32252125, 1.7e-4, 0.0049, 3.1e-4, 0.12, 0.05, 1.7, 0.2},
                               {1.3217, 2e-4, 0.004, 1e-4, -0.01, 0, 1.2, 0}};
    for (const auto& c : cases) {
        RooRealVar mu("mu", "", c[0]), lambda("lambda", "", c[2]), gamma("gamma", "", c[4]), delta("delta", "", c[6]);
        mu.setError(c[1]); lambda.setError(c[3]); gamma.setError(c[5]); delta.setError(c[7]);
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
        gxana::fit::JohnsonMoments m = gxana::fit::Moments(mu, lambda, gamma, delta);
        const double got[] = {m.mean, m.meanErr, m.sigma, m.sigmaErr};
        const double want[] = {xiMean, xiMeanErr, xiSigma, xiSigmaErr};
        for (int i = 0; i < 4; ++i)
            if (std::memcmp(&got[i], &want[i], sizeof(double)) != 0) {
                printf("MISMATCH case mu=%.17g field %d: library %.17g interpreter %.17g\n", c[0], i, got[i], want[i]);
                ++bad;
            }
    }
    printf("cling_fit: %d mismatches\n", bad);
    gApplication->Terminate(bad ? 1 : 0);
    return bad;
}
