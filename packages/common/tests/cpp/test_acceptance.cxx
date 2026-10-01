#include "gxana/common/AcceptanceCorrect.h"

#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>

#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

static int failures = 0;
#define CHECK(cond)                                                                   \
    do {                                                                              \
        if (!(cond)) {                                                                \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #cond "\n"; \
            ++failures;                                                               \
        }                                                                             \
    } while (0)
#define CHECK_NEAR(a, b, tol) CHECK(std::fabs((a) - (b)) < (tol))

static void fill(TH1D& h, std::vector<double> v)
{
    for (size_t i = 0; i < v.size(); ++i) h.SetBinContent(i + 1, v[i]);
}

int main()
{
    // 1-D: thrown {100,100,0,100}, reco {50,25,0,100}, data {10,10,7,20}
    TH1D thrown("thrown", "", 4, 0, 4), reco("reco", "", 4, 0, 4), data("data", "", 4, 0, 4);
    fill(thrown, {100, 100, 0, 100});
    fill(reco, {50, 25, 0, 100});
    fill(data, {10, 10, 7, 20});

    for (auto mode : {gxana::AccErrors::Binomial, gxana::AccErrors::Plain, gxana::AccErrors::PlainNoSumw2}) {
        std::unique_ptr<TH1> acc(gxana::Acceptance(thrown, reco, "acc", mode));
        CHECK_NEAR(acc->GetBinContent(1), 0.5, 1e-12);
        CHECK_NEAR(acc->GetBinContent(2), 0.25, 1e-12);
        CHECK(acc->GetBinContent(3) == 0.0); // empty thrown bin: 0, never inf/NaN
        CHECK(std::isfinite(acc->GetBinError(3)));
        CHECK_NEAR(acc->GetBinContent(4), 1.0, 1e-12);
        CHECK(acc->GetDirectory() == nullptr);

        std::unique_ptr<TH1> corr(gxana::AcceptanceCorrect(data, *acc, "corr", mode));
        CHECK_NEAR(corr->GetBinContent(1), 20.0, 1e-12);
        CHECK_NEAR(corr->GetBinContent(2), 40.0, 1e-12);
        CHECK(corr->GetBinContent(3) == 0.0);
        CHECK(corr->GetBinError(3) == 0.0);
        CHECK_NEAR(corr->GetBinContent(4), 20.0, 1e-12);
        CHECK(gxana::LostBins(data, *acc) == 1); // bin 3: data 7, eps 0
    }

    // Binomial error of eps in bin 1: sqrt(w(1-w)/N) = 0.05 (unweighted histograms)
    std::unique_ptr<TH1> accB(gxana::Acceptance(thrown, reco, "accB", gxana::AccErrors::Binomial));
    CHECK_NEAR(accB->GetBinError(1), 0.05, 1e-9);

    // one-call form returns the acceptance too
    TH1* accOut = nullptr;
    std::unique_ptr<TH1> one(gxana::AcceptanceCorrect(data, reco, thrown, "one", gxana::AccErrors::Binomial, &accOut));
    std::unique_ptr<TH1> accKeep(accOut);
    CHECK(accOut != nullptr);
    CHECK_NEAR(one->GetBinContent(2), 40.0, 1e-12);
    CHECK_NEAR(accOut->GetBinContent(2), 0.25, 1e-12);

    // mismatched binning throws and names both histograms
    TH1D wrong("wrong", "", 5, 0, 4);
    bool threw = false;
    try { delete gxana::Acceptance(thrown, wrong, "x"); }
    catch (const std::invalid_argument& e) {
        threw = true;
        std::string m = e.what();
        CHECK(m.find("thrown") != std::string::npos && m.find("wrong") != std::string::npos);
    }
    CHECK(threw);
    TH2D wrong2("wrong2", "", 4, 0, 4, 2, 0, 2);
    threw = false;
    try { delete gxana::Acceptance(thrown, wrong2, "x"); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw);

    // 2-D: same code path
    TH2D t2("t2", "", 2, 0, 2, 2, 0, 2), r2("r2", "", 2, 0, 2, 2, 0, 2), d2("d2", "", 2, 0, 2, 2, 0, 2);
    t2.SetBinContent(1, 1, 100); t2.SetBinContent(2, 1, 100); t2.SetBinContent(1, 2, 0); t2.SetBinContent(2, 2, 100);
    r2.SetBinContent(1, 1, 50);  r2.SetBinContent(2, 1, 25);  r2.SetBinContent(1, 2, 0); r2.SetBinContent(2, 2, 100);
    d2.SetBinContent(1, 1, 10);  d2.SetBinContent(2, 1, 10);  d2.SetBinContent(1, 2, 5); d2.SetBinContent(2, 2, 20);
    std::unique_ptr<TH1> c2(gxana::AcceptanceCorrect(d2, r2, t2, "c2", gxana::AccErrors::PlainNoSumw2));
    CHECK(c2->GetDimension() == 2);
    CHECK_NEAR(c2->GetBinContent(1, 1), 20.0, 1e-12);
    CHECK_NEAR(c2->GetBinContent(2, 1), 40.0, 1e-12);
    CHECK(c2->GetBinContent(1, 2) == 0.0);
    CHECK(std::dynamic_pointer_cast<TH2D>(std::shared_ptr<TH1>(std::move(c2))) != nullptr);

    // 3-D
    TH3D t3("t3", "", 2, 0, 2, 2, 0, 2, 2, 0, 2), r3("r3", "", 2, 0, 2, 2, 0, 2, 2, 0, 2), d3("d3", "", 2, 0, 2, 2, 0, 2, 2, 0, 2);
    t3.SetBinContent(1, 1, 1, 80); r3.SetBinContent(1, 1, 1, 20); d3.SetBinContent(1, 1, 1, 3);
    std::unique_ptr<TH1> c3(gxana::AcceptanceCorrect(d3, r3, t3, "c3"));
    CHECK_NEAR(c3->GetBinContent(1, 1, 1), 12.0, 1e-12);

    // merge: corrected summed; eps merged with the original SetBit(kIsAverage)+Add operations
    TH1D thrownB("thrownB", "", 4, 0, 4), recoB("recoB", "", 4, 0, 4), dataB("dataB", "", 4, 0, 4);
    fill(thrownB, {200, 100, 100, 100}); fill(recoB, {50, 50, 50, 50}); fill(dataB, {10, 10, 10, 10});
    TH1* accA = nullptr; TH1* accB2 = nullptr;
    std::unique_ptr<TH1> cA(gxana::AcceptanceCorrect(data, reco, thrown, "cA", gxana::AccErrors::Binomial, &accA));
    std::unique_ptr<TH1> cB(gxana::AcceptanceCorrect(dataB, recoB, thrownB, "cB", gxana::AccErrors::Binomial, &accB2));
    std::unique_ptr<TH1> kA(accA), kB(accB2);
    TH1* avg = nullptr;
    std::unique_ptr<TH1> merged(gxana::MergeCorrected({cA.get(), cB.get()}, {accA, accB2}, &avg));
    std::unique_ptr<TH1> avgKeep(avg);
    CHECK_NEAR(merged->GetBinContent(2), cA->GetBinContent(2) + cB->GetBinContent(2), 1e-12);
    std::unique_ptr<TH1D> ref((TH1D*)accA->Clone("ref"));
    ref->SetDirectory(nullptr);
    ref->SetBit(TH1::kIsAverage);
    ref->Add(accB2);
    for (int b = 1; b <= 4; ++b) CHECK_NEAR(avg->GetBinContent(b), ref->GetBinContent(b), 1e-12);
    threw = false;
    try { gxana::MergeCorrected({}, {}); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw);

    if (failures) std::cerr << failures << " check(s) failed\n";
    return failures ? 1 : 0;
}
