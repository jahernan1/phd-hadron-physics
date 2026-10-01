#ifndef GXANA_COMMON_ACCEPTANCECORRECT_H
#define GXANA_COMMON_ACCEPTANCECORRECT_H

#include <vector>

class TH1;

namespace gxana {

// How the acceptance's errors are built. Bin contents are identical in every mode.
//  Binomial     Divide(reco, thrown, 1, 1, "B")        (3-D sampling macros; acceptance of the 1-D study macros)
//  Plain        clone of reco .Divide(&thrown)          (track study; data/eps division at every site except the 2-D sampling macro)
//  PlainNoSumw2 Sumw2(false), then .Divide(&thrown)     (2-D sampling macro, both halves; acceptance of the 2-D MC study)
enum class AccErrors { Binomial, Plain, PlainNoSumw2 };

// eps = reco / thrown, same dimension and binning (throws std::invalid_argument otherwise).
// Bins with an empty thrown bin get content 0 and error 0 (TH1::Divide behaviour).
// Works for TH1, TH2 and TH3; the result has the class of `reco`, is named `name`
// and is not attached to any TDirectory. The caller owns it.
TH1* Acceptance(const TH1& thrown, const TH1& reco, const char* name, AccErrors errors = AccErrors::Binomial);

// data / eps. A bin with eps = 0 becomes 0 with error 0. Errors propagate eps' errors.
TH1* AcceptanceCorrect(const TH1& data, const TH1& acc, const char* name, AccErrors errors = AccErrors::Binomial);

// Acceptance + AcceptanceCorrect in one call; *accOut (if given) receives eps, owned by the caller.
// Only PlainNoSumw2 differs here; Binomial and Plain both clone data and Divide(&acc).
TH1* AcceptanceCorrect(const TH1& data, const TH1& reco, const TH1& thrown, const char* name,
                       AccErrors errors = AccErrors::Binomial, TH1** accOut = nullptr);

// Populated data bins (non-zero content, no under/overflow) whose eps is 0.
int LostBins(const TH1& data, const TH1& acc);

// Period merge of the original macros: clone of corr[0] (optionally Sumw2()), plus the others.
// *avgAcc (if given and acc non-empty): clone of acc[0] with SetBit(kIsAverage), plus the others.
// Throws std::invalid_argument for an empty list, differing list sizes or differing binning.
TH1* MergeCorrected(const std::vector<const TH1*>& corr, const std::vector<const TH1*>& acc,
                    TH1** avgAcc = nullptr, bool sumw2 = true, const char* name = "");

} // namespace gxana

#endif
