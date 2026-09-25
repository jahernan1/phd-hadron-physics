// Print "ROW entry qvalue fitStatus chiSqNdf kDim n0 n1 ..." (neighbours
// sorted) for a Q-factor results or postQVal tree; entry is
// flatEntryNumber_<var> when that branch exists, else the tree index.
// Missing chiSqNdf_<var> prints nan.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "TFile.h"
#include "TTree.h"

void dump_qfactors(const char* path, const char* treeName, const char* var, Long64_t first = 0, Long64_t count = -1) {
  TFile f(path);
  TTree* t = nullptr;
  f.GetObject(treeName, t);
  if (!t) { printf("NOTREE %s\n", treeName); return; }
  const std::string v(var);
  float q = NAN, chi = NAN;
  int status = 0, k = 0;
  ULong64_t entryNumber = 0;
  static int neighbors[10000];
  t->SetBranchStatus("*", 0);
  auto use = [&](const std::string& name, void* addr) {
    if (!t->GetBranch(name.c_str())) return false;
    t->SetBranchStatus(name.c_str(), 1);
    t->SetBranchAddress(name.c_str(), addr);
    return true;
  };
  use("qvalue_" + v, &q);
  use("fitStatus_" + v, &status);
  use("chiSqNdf_" + v, &chi);
  use("kDim_" + v, &k);
  use("neighbors_" + v, neighbors);
  const bool hasEntry = use("flatEntryNumber_" + v, &entryNumber);
  const Long64_t last = count < 0 ? t->GetEntries() : std::min(t->GetEntries(), first + count);
  for (Long64_t i = first; i < last; ++i) {
    t->GetEntry(i);
    std::vector<int> nb(neighbors, neighbors + k);
    std::sort(nb.begin(), nb.end());
    printf("ROW %lld %.9g %d %.9g %d", hasEntry ? (Long64_t)entryNumber : i, q, status, chi, k);
    for (int n : nb) printf(" %d", n);
    printf("\n");
  }
}
