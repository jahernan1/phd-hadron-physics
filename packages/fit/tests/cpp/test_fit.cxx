#include "gxana/fit/Fit.h"
#include "gxana/fit/Johnson.h"
#include "gxana/fit/Model.h"

#include <RooArgList.h>
#include <RooDataHist.h>
#include <RooDataSet.h>
#include <RooMsgService.h>
#include <RooRandom.h>
#include <RooRealVar.h>
#include <RooWorkspace.h>
#include <TH1D.h>
#include <TRandom3.h>
#include <TTree.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace gxana::fit;

static int failures = 0;
#define CHECK(cond)                                                                   \
    do {                                                                              \
        if (!(cond)) {                                                                \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #cond "\n"; \
            ++failures;                                                               \
        }                                                                             \
    } while (0)

static bool SameBits(double a, double b) { return std::memcmp(&a, &b, sizeof a) == 0; }

static std::string NoBlanks(std::string s)
{
    s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
    return s;
}

// name, value, error, min, max, constant of every RooRealVar, in creation order.
static std::string Dump(RooWorkspace& w)
{
    std::ostringstream os;
    os.precision(17);
    for (const auto* a : w.components()) os << "pdf " << a->GetName() << "\n";
    RooArgList vars(w.allVars());
    for (const auto* a : vars) {
        const auto* v = static_cast<const RooRealVar*>(a);
        os << v->GetName() << " " << v->getVal() << " " << v->getError() << " " << v->getMin() << " " << v->getMax()
           << " " << v->isConstant() << "\n";
    }
    return os.str();
}

// A site's own statements and the builders' statements create the same workspace.
static void CheckSameWorkspace(const std::vector<std::string>& site, const std::vector<std::string>& built,
                               const char* obs, double lo, double hi)
{
    CHECK(site.size() == built.size());
    for (std::size_t i = 0; i < site.size() && i < built.size(); ++i) {
        if (NoBlanks(site[i]) != NoBlanks(built[i]))
            std::cerr << "statement " << i << "\n site:  " << site[i] << "\n built: " << built[i] << "\n";
        CHECK(NoBlanks(site[i]) == NoBlanks(built[i]));
    }
    RooRealVar x1(obs, obs, lo, hi), x2(obs, obs, lo, hi);
    RooWorkspace w1("w1"), w2("w2");
    w1.import(RooArgSet(x1));
    w2.import(RooArgSet(x2));
    for (const auto& s : site) w1.factory(s.c_str());
    BuildModel(w2, built);
    CHECK(Dump(w1) == Dump(w2));
    CHECK(w2.pdf("model") != nullptr);
}

static void TestBuilders()
{
    CHECK(Fx(1.3217) == "1.321700");
    CHECK(Fx(-0.01) == "-0.010000");
    CHECK(Johnson("s", "m", {"mu", "1,0,2"}, {"lambda", "0.004"}, {"gamma", "0"}, {"delta", "1.2,0.2,5."}) ==
          "Johnson::s(m,mu[1,0,2],lambda[0.004],gamma[0],delta[1.2,0.2,5.])");
    CHECK(Gaussian("g2", "m", {"mean", ""}, {"sigma2", "0.01"}) == "Gaussian::g2(m,mean,sigma2[0.01])");
    CHECK(Voigtian("v", "m", {"mean", "1"}, {"width", "2"}, {"sigma", "3"}) == "Voigtian::v(m,mean[1],width[2],sigma[3])");
    CHECK(BreitWigner("b", "m", {"mean", "1"}, {"width", "2"}) == "BreitWigner::b(m,mean[1],width[2])");
    CHECK(Chebychev("c", "m", {{"a0", "0.8"}, {"a1", "-0.2"}}) == "Chebychev::c(m,{a0[0.8],a1[-0.2]})");
    CHECK(Threshold("t", "m", {"m0", "1"}, {"b", "2"}, {"p", "3"}) ==
          "EXPR::t('(m)*(((m)/m0)**2-1.0)**p*exp(b*(((m)/m0)**2-1.0))',m,m0[1],b[2],p[3])");
    CHECK(Sum("model", {{{"n1", "1,0,9"}, "a"}, {{"n2", "0"}, "b"}}) == "SUM::model(n1[1,0,9]*a,n2[0]*b)");
}

// FitMass.C (data and MC fit) statements, as the macro writes them, against the builders.
static void TestSiteStatements()
{
    const std::vector<double> params = {1.3217, 0.004, -0.01, 1.2};
    char johnsonData[256], johnsonMC[256];
    std::snprintf(johnsonData, sizeof(johnsonData),
                  "Johnson::xisignal(mass, mu[%f,1.32,1.33], lambda[%f,%f,0.008], gamma[%f], delta[%f])", params[0],
                  params[1], params[1], params[2], params[3]);
    std::snprintf(johnsonMC, sizeof(johnsonMC),
                  "Johnson::xisignal(mass, mu[%f,1.32,1.33], lambda[%f,0.002,0.007], gamma[%f, -1,1], delta[%f,0.2,5.])",
                  params[0], params[1], params[2], params[3]);
    CheckSameWorkspace(
        {"Chebychev::bkgd(mass,{a0[0.8,1.e-2,2],a1[-0.2,-2,-1e-2]})",
         "Gaussian::sigma(mass,mean[1.385,1.383,1.388],sig[0.0394,0.01,0.05])", johnsonData,
         "SUM::model( nsigma[0,1,1e6]*sigma, nbkgd[1000,1,1e6]*bkgd, nxi[10000,1,1e6]*xisignal)"},
        {Chebychev("bkgd", "mass", {{"a0", "0.8,1.e-2,2"}, {"a1", "-0.2,-2,-1e-2"}}),
         Gaussian("sigma", "mass", {"mean", "1.385,1.383,1.388"}, {"sig", "0.0394,0.01,0.05"}),
         Johnson("xisignal", "mass", {"mu", Fx(params[0]) + ",1.32,1.33"},
                 {"lambda", Fx(params[1]) + "," + Fx(params[1]) + ",0.008"}, {"gamma", Fx(params[2])},
                 {"delta", Fx(params[3])}),
         Sum("model", {{{"nsigma", "0,1,1e6"}, "sigma"}, {{"nbkgd", "1000,1,1e6"}, "bkgd"},
                       {{"nxi", "10000,1,1e6"}, "xisignal"}})},
        "mass", 1.27, 1.45);
    CheckSameWorkspace(
        {johnsonMC,
         "EXPR::bkgd('(mass)*(((mass)/m0)**2-1.0)**p*exp(b*(((mass)/m0)**2-1.0))',mass, m0[1.2602,1.255,1.275], b[-22,-40.,-5.], p[2])",
         "SUM::model(nxi[10000,1,1e6]*xisignal, nbkgd[2000,1,1e6]*bkgd)"},
        {Johnson("xisignal", "mass", {"mu", Fx(params[0]) + ",1.32,1.33"}, {"lambda", Fx(params[1]) + ",0.002,0.007"},
                 {"gamma", Fx(params[2]) + ", -1,1"}, {"delta", Fx(params[3]) + ",0.2,5."}),
         Threshold("bkgd", "mass", {"m0", "1.2602,1.255,1.275"}, {"b", "-22,-40.,-5."}, {"p", "2"}),
         Sum("model", {{{"nxi", "10000,1,1e6"}, "xisignal"}, {{"nbkgd", "2000,1,1e6"}, "bkgd"}})},
        "mass", 1.285, 1.38);
}

static TH1D* ToyHist(unsigned seed)
{
    TRandom3 r(seed);
    auto* h = new TH1D(Form("toy%u", seed), "", 90, 1.27, 1.45);
    for (int i = 0; i < 4000; ++i) h->Fill(r.Gaus(1.3217, 0.006));
    for (int i = 0; i < 3000; ++i) h->Fill(r.Uniform(1.27, 1.45));
    return h;
}

// RunFit makes the same call as a direct fitTo: bit-identical parameters.
static void TestRunFit()
{
    std::unique_ptr<TH1D> h(ToyHist(7));
    const std::vector<std::string> model = {
        Chebychev("bkgd", "mass", {{"a0", "0.1,-1,1"}}),
        Gaussian("sig", "mass", {"mean", "1.322,1.31,1.33"}, {"width", "0.005,0.001,0.02"}),
        Sum("model", {{{"ns", "1000,1,1e6"}, "sig"}, {{"nb", "1000,1,1e6"}, "bkgd"}})};
    RooRealVar x1("mass", "mass", 1.27, 1.45), x2("mass", "mass", 1.27, 1.45);
    RooDataHist d1("data", "d", x1, h.get()), d2("data", "d", x2, h.get());
    RooWorkspace w1("w1"), w2("w2");
    w1.import(RooArgSet(x1));
    w2.import(RooArgSet(x2));
    BuildModel(w1, model);
    BuildModel(w2, model);
    w1.pdf("model")->fitTo(d1, RooFit::Extended(true), RooFit::PrintLevel(-1), RooFit::PrintEvalErrors(-1),
                           RooFit::Verbose(false), RooFit::Warnings(false));
    RunFit(*w2.pdf("model"), d2, RooFit::Extended(true), RooFit::PrintLevel(-1), RooFit::PrintEvalErrors(-1),
           RooFit::Verbose(false), RooFit::Warnings(false));
    for (const char* n : {"a0", "mean", "width", "ns", "nb"}) {
        CHECK(SameBits(w1.var(n)->getVal(), w2.var(n)->getVal()));
        CHECK(SameBits(w1.var(n)->getError(), w2.var(n)->getError()));
    }
    std::ostringstream t1, t2;
    TraceFit(t1, *w1.pdf("model"), d1);
    TraceFit(t2, *w2.pdf("model"), d2);
    CHECK(t1.str() == t2.str());
    CHECK(t1.str().rfind("FITRESULT trace model a0=", 0) == 0);
    CHECK(t1.str().find(" ns_err=") != std::string::npos);
    CHECK(t1.str().back() == '\n');
}

static void TestTraceEnv()
{
    RooRealVar x("mass", "mass", 1.27, 1.45);
    RooWorkspace w("w");
    w.import(RooArgSet(x));
    std::ostringstream captured;
    auto* old = std::cout.rdbuf(captured.rdbuf());
    unsetenv("GXANA_FIT_TRACE");
    BuildModel(w, {Gaussian("g1", "mass", {"m1", "1.3"}, {"s1", "0.01"})});
    setenv("GXANA_FIT_TRACE", "", 1); // empty counts as unset
    BuildModel(w, {Gaussian("g0", "mass", {"m0", "1.3"}, {"s0", "0.01"})});
    setenv("GXANA_FIT_TRACE", "1", 1);
    BuildModel(w, {Gaussian("g2", "mass", {"m2", "1.3"}, {"s2", "0.01"})});
    unsetenv("GXANA_FIT_TRACE");
    std::cout.rdbuf(old);
    CHECK(captured.str() == "FACTORY Gaussian::g2(mass,m2[1.3],s2[0.01])\n");
    CHECK(w.pdf("g0") && w.pdf("g1") && w.pdf("g2"));
}

static void TestImportTree()
{
    TTree t("t", "t");
    double m, wt;
    t.Branch("decayxim_M", &m);
    t.Branch("wgt", &wt);
    const double ms[] = {1.30, 1.31, 1.50, 1.32, 1.33};
    const double ws[] = {1.0, 12.0, 1.0, -0.5, -11.0};
    for (int i = 0; i < 5; ++i) { m = ms[i]; wt = ws[i]; t.Fill(); }
    RooRealVar x("decayxim_M", "M", 1.26, 1.42);
    std::unique_ptr<RooDataSet> d(ImportTree(t, x, "wgt"));
    CHECK(std::string(d->GetName()) == "data");
    CHECK(std::string(d->GetTitle()) == "Dataset of mass");
    CHECK(d->numEntries() == 2);          // |w| > 10 and out-of-range entries dropped
    CHECK(SameBits(d->sumEntries(), 0.5)); // 1.0 - 0.5
    CHECK(d->isWeighted());
}

static void TestFirstPopulatedEdge()
{
    TH1D h("h", "", 100, 1.26, 1.42);
    for (double x : {1.287, 1.30, 1.31}) h.Fill(x);
    auto legacy = [&](double thr, int first, double lastX, int off) {
        return h.GetXaxis()->GetBinLowEdge(h.FindFirstBinAbove(thr, 1, first, h.FindBin(lastX)) + off);
    };
    CHECK(SameBits(FirstPopulatedEdge(h, 0, 1, 1.32), legacy(0, 1, 1.32, 0)));
    CHECK(SameBits(FirstPopulatedEdge(h, 0, 1, 1.32, -1), legacy(0, 1, 1.32, -1)));
    CHECK(SameBits(FirstPopulatedEdge(h, 0, 5, 1.3), legacy(0, 5, 1.3, 0)));
    CHECK(SameBits(FirstPopulatedEdge(h, 1, 1, 1.32), legacy(1, 1, 1.32, 0))); // none above 1: bin -1, as legacy
    CHECK(SameBits(FirstPopulatedEdge(h, 0, 1, 1.32), h.GetXaxis()->GetBinLowEdge(h.FindBin(1.287))));
}

// The statements of FitMass.C, transcribed, against Moments().
static void TestMoments()
{
    RooRealVar mu("mu", "", 1.3221, 1.32, 1.33), lambda("lambda", "", 0.0051, 0, 1), gamma("gamma", "", -0.07, -1, 1),
        delta("delta", "", 1.13, 0.2, 5);
    mu.setError(1.1e-4); lambda.setError(2.2e-4); gamma.setError(0.031); delta.setError(0.09);
    auto legacy = [](double xiMu, double xiMuErr, double xiLambda, double xiLambdaErr, double xiGamma,
                     double xiGammaErr, double xiDelta, double xiDeltaErr) {
        double xiMean = xiMu - xiLambda * exp(1 / (2*pow(xiDelta,2)) ) * sinh(xiGamma/xiDelta);
        double xiMeanErr = sqrt( pow(xiMuErr,2)
                           + pow( -1* xiLambdaErr*exp(1 / (2*pow(xiDelta,2)))*sinh(xiGamma/xiDelta),2 )
                           + pow( -1* xiGammaErr* xiLambda *exp(1 / (2*pow(xiDelta,2)))*cosh(xiGamma/xiDelta) / xiDelta,2)
                           + pow(xiDeltaErr*xiLambda*exp(1 / (2*pow(xiDelta,2)))*(sinh(xiGamma/xiDelta) + xiGamma*xiDelta*cosh(xiGamma/xiDelta)) / pow(xiDelta,3) ,2));
        double xiSigma = sqrt( pow(xiLambda, 2)/2*(exp(pow(xiDelta, -2) ) - 1 )*(exp(pow(xiDelta, -2) )*cosh(2*xiGamma/xiDelta )+1));
        double xiSigmaErr = xiSigma * sqrt( pow( xiDelta/ xiDeltaErr, 2) + pow( xiGammaErr/xiGamma, 2) );
        return JohnsonMoments{xiMean, xiMeanErr, xiSigma, xiSigmaErr};
    };
    JohnsonMoments a = Moments(mu, lambda, gamma, delta);
    JohnsonMoments b = legacy(1.3221, 1.1e-4, 0.0051, 2.2e-4, -0.07, 0.031, 1.13, 0.09);
    CHECK(SameBits(a.mean, b.mean) && SameBits(a.meanErr, b.meanErr));
    CHECK(SameBits(a.sigma, b.sigma) && SameBits(a.sigmaErr, b.sigmaErr));
    // Constant gamma and delta (the data fit): getError() is 0 and the legacy sigma error is infinite.
    RooWorkspace w("w");
    w.factory("Johnson::j(x[1.27,1.45],mu2[1.3221,1.32,1.33],lambda2[0.0051,0.004,0.008],gamma2[-0.07],delta2[1.13])");
    JohnsonMoments c = Moments(*w.var("mu2"), *w.var("lambda2"), *w.var("gamma2"), *w.var("delta2"));
    CHECK(w.var("delta2")->getError() == 0);
    CHECK(std::isinf(c.sigmaErr));
    CHECK(SameBits(c.mean, a.mean) && SameBits(c.sigma, a.sigma));
}

int main()
{
    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
    TestBuilders();
    TestSiteStatements();
    TestRunFit();
    TestTraceEnv();
    TestImportTree();
    TestFirstPopulatedEdge();
    TestMoments();
    if (failures) std::cerr << failures << " check(s) failed\n";
    else std::cout << "test_fit: all checks passed\n";
    return failures ? 1 : 0;
}
