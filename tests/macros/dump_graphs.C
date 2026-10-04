// Print every TGraphErrors point (G key title i x y ex ey) and TF1 (F key chi2 ndf) of a ROOT
// file, tab-separated, in key order. Used by the figure tests to read back what a macro drew.
void dump_graphs(const char* path)
{
  TFile f(path);
  TIter next(f.GetListOfKeys());
  while (TKey* key = (TKey*)next()) {
    TObject* obj = key->ReadObj();
    if (auto* g = dynamic_cast<TGraphErrors*>(obj)) {
      for (int i = 0; i < g->GetN(); ++i)
        printf("G\t%s\t%s\t%d\t%.9g\t%.9g\t%.9g\t%.9g\n", key->GetName(), g->GetTitle(), i,
               g->GetX()[i], g->GetY()[i], g->GetEX()[i], g->GetEY()[i]);
    } else if (auto* fn = dynamic_cast<TF1*>(obj)) {
      printf("F\t%s\t%.9g\t%d\n", key->GetName(), fn->GetChisquare(), fn->GetNDF());
    }
  }
}
