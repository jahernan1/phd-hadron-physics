// Equivalence-check driver: loads Synthetic.h and one macro text (a frozen legacy copy or the
// adopted macro), runs CALL, saves every canvas left in memory as OUT/NN_<name>.pdf.
// usage: root -l -b -q REPO/rootlogon.C 'run_site.C("MACRO","CALL","OUT")'
// (the directory of this file must be on the include path for LegacyTrace.h)
void run_site(const char* macro, const char* call, const char* out)
{
    int err = 0;
    gROOT->ProcessLine(Form("#include \"%s/Synthetic.h\"", gSystem->DirName(__FILE__)), &err);
    if (err == 0) gROOT->ProcessLine(Form(".L %s", macro), &err);
    if (err == 0) gROOT->ProcessLine(call, &err);
    if (err != 0) {
        std::cerr << "RUN_SITE FAILED: " << macro << " " << call << std::endl;
        gSystem->Exit(1);
    }
    gSystem->mkdir(out, kTRUE);
    int n = 0;
    for (auto* obj : *gROOT->GetListOfCanvases()) {
        auto* c = static_cast<TCanvas*>(obj);
        c->SaveAs(Form("%s/%02d_%s.pdf", out, n++, c->GetName()));
    }
    std::cout << "RUN_SITE saved " << n << " canvases" << std::endl;
}
