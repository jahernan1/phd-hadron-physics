#include "gxana/barlow/Barlow.h"

#include <TGraphErrors.h>

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

static int failures = 0;
#define CHECK(cond)                                                                   \
    do {                                                                              \
        if (!(cond)) {                                                                \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #cond "\n"; \
            ++failures;                                                               \
        }                                                                             \
    } while (0)

using namespace gxana::barlow;

static void TestCalcBarlow()
{
    // Moved from packages/xsection/tests/cpp/test_xsection.cxx.
    TGraphErrors nominal(2), variation(2);
    nominal.SetPoint(0, 0.225, 5.0);   nominal.SetPointError(0, 0.125, 0.5);
    nominal.SetPoint(1, 0.44, 6.0);    nominal.SetPointError(1, 0.09, 0.4);
    variation.SetPoint(0, 0.225, 4.0); variation.SetPointError(0, 0.125, 0.3);
    variation.SetPoint(1, 0.44, 6.5);  variation.SetPointError(1, 0.09, 0.4);
    TGraphErrors* barlow = calc_barlow(&nominal, &variation);
    CHECK(barlow->GetN() == 2);
    CHECK(std::fabs(barlow->GetPointY(0) - 1.0 / 0.4) < 1e-12); // sqrt(0.25 - 0.09) = 0.4
    CHECK(barlow->GetPointY(1) == 0.0);                           // equal errors: sigma 0
    CHECK(barlow->GetPointX(1) == 0.44 && barlow->GetErrorX(0) == 0.125 && barlow->GetErrorY(0) == 0.0);

    // Signed: variation above nominal gives a negative significance.
    TGraphErrors n1(1), v1(1);
    n1.SetPoint(0, 7.0, 4.0); n1.SetPointError(0, 0.5, 0.3);
    v1.SetPoint(0, 7.0, 5.0); v1.SetPointError(0, 0.5, 0.5);
    TGraphErrors* neg = calc_barlow(&n1, &v1);
    CHECK(std::fabs(neg->GetPointY(0) - (-1.0 / 0.4)) < 1e-12);

    // sigma = 0 with a nonzero difference still gives 0 (legacy guard), not inf.
    TGraphErrors n2(1), v2(1);
    n2.SetPoint(0, 7.0, 4.0); n2.SetPointError(0, 0.5, 0.0);
    v2.SetPoint(0, 7.0, 9.0); v2.SetPointError(0, 0.5, 0.0);
    CHECK(calc_barlow(&n2, &v2)->GetPointY(0) == 0.0);
}

static void TestStdDev()
{
    TGraphErrors a(2), b(2);
    a.SetPoint(0, 0.225, 4.0); a.SetPoint(1, 0.44, 6.0);
    b.SetPoint(0, 0.225, 6.0); b.SetPoint(1, 0.44, 6.0);
    TGraphErrors* spread = calculateStdDevGraph({&a, &b});
    CHECK(std::fabs(spread->GetPointY(0) - 1.0) < 1e-12);
    CHECK(spread->GetPointY(1) == 0.0 && spread->GetPointX(1) == 0.44);
    CHECK(calculateStdDevGraph({}) == nullptr);
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    TestCalcBarlow();
    TestStdDev();
    if (failures == 0) std::cout << "test_barlow: all checks passed\n";
    return failures == 0 ? 0 : 1;
}
