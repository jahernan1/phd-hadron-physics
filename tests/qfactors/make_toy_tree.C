// Toy flat tree for the QFactors smoke test: a Xi- peak on a flat
// background in decayxim_M, the seven thesis phase-space branches drawn
// uniformly, unit weights. Generated at test time; not analysis data.
#include <string>
#include "TFile.h"
#include "TRandom3.h"
#include "TTree.h"

void make_toy_tree(const char* path, int n = 600, int seed = 7) {
  const char* names[7] = {"beam_E", "kp_highp_CosTheta", "kp_highp_Phi", "kplow_costheta_hf",
                          "kplow_phi_hf", "pim1_costheta_hf", "decaylamb_M_meas"};
  TRandom3 rng(seed);
  TFile f(path, "RECREATE");
  TTree t("flatTree_kpkpxim", "toy");
  float m = 0, w = 1, v[7];
  t.Branch("decayxim_M", &m, "decayxim_M/F");
  t.Branch("hybrid_combo", &w, "hybrid_combo/F");
  for (int i = 0; i < 7; ++i) t.Branch(names[i], &v[i], (std::string(names[i]) + "/F").c_str());
  for (int e = 0; e < n; ++e) {
    m = rng.Rndm() < 0.4 ? rng.Gaus(1.3217, 0.004) : rng.Uniform(1.28, 1.45);
    for (int i = 0; i < 7; ++i) v[i] = rng.Uniform(-1, 1);
    t.Fill();
  }
  t.Write();
  f.Close();
}
