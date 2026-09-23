#include "gxana/common/Paths.h"
#include "gxana/common/Strings.h"
#include "gxana/common/Style.h"

#include <TStyle.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

static int failures = 0;
#define CHECK(cond)                                                                   \
    do {                                                                              \
        if (!(cond)) {                                                                \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #cond "\n"; \
            ++failures;                                                               \
        }                                                                             \
    } while (0)

int main()
{
    CHECK(gxana::NumericCompare("diffxsec_emin_7.40_emax_7.86.txt", "diffxsec_emin_10.18_emax_11.40.txt"));
    CHECK(!gxana::NumericCompare("diffxsec_emin_10.18.txt", "diffxsec_emin_7.40.txt"));
    CHECK(!gxana::NumericCompare("x_emin_8.19.txt", "y_emin_8.45.txt")); // equal integer parts
    bool threw = false;
    try { gxana::NumericCompare("no_marker.txt", "x_emin_7.txt"); } catch (const std::out_of_range&) { threw = true; }
    CHECK(threw);

    setenv("GXANA_DATA", "/data/", 1);
    CHECK(gxana::EnvPath("GXANA_DATA", "Trees/x.root") == "/data/Trees/x.root");
    setenv("GXANA_DATA", "/data", 1);
    CHECK(gxana::EnvPath("GXANA_DATA", "Trees") == "/data/Trees");
    CHECK(gxana::EnvPath("GXANA_DATA") == "/data");
    unsetenv("GXANA_DATA");
    threw = false;
    try { gxana::EnvPath("GXANA_DATA", "x"); } catch (const std::runtime_error&) { threw = true; }
    CHECK(threw);

    gxana::SetStyle();
    CHECK(gStyle->GetOptStat() == 0);
    CHECK(std::fabs(gStyle->GetPadLeftMargin() - 0.13f) < 1e-6);
    CHECK(gStyle->GetCanvasDefW() == 700);

    if (failures == 0) std::cout << "test_common: all checks passed\n";
    return failures == 0 ? 0 : 1;
}
