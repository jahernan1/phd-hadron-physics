// Run by ROOT when started from the repo root: makes the gxana libraries
// usable from interpreted macros (build first: see docs/environment.md).
{
    const char* gxanaRoot = gSystem->Getenv("GXANA_ROOT");
    if (gxanaRoot) {
        // Explicit dynamic path: macOS SIP may strip DYLD_LIBRARY_PATH.
        gSystem->AddDynamicPath(Form("%s/build/lib", gxanaRoot));
        gInterpreter->AddIncludePath(Form("%s/packages/common/include", gxanaRoot));
        gInterpreter->AddIncludePath(Form("%s/packages/xsection/include", gxanaRoot));
        gInterpreter->AddIncludePath(Form("%s/packages/barlow/include", gxanaRoot));
        gInterpreter->AddIncludePath(Form("%s/packages/systematics/include", gxanaRoot));
        gInterpreter->AddIncludePath(Form("%s/packages/fit/include", gxanaRoot));
        for (const char* lib : {"libGxanaCommon", "libGxanaPeriodHists", "libGxanaXsec", "libGxanaBarlow", "libGxanaSystematics", "libGxanaFit"})
            if (gSystem->Load(lib) < 0)
                Warning("rootlogon", "%s not found; run cmake --build build", lib);
        // Every fit uses the ROOT 6.24 minimiser and RooFit evaluation backend
        // (packages/fit/README.md). Called by name: the library may be missing.
        if (gSystem->Load("libGxanaFit") >= 0)
            gROOT->ProcessLine("gxana::fit::UseThesisMinimizer();");
    }
}
