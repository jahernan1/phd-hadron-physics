// Run by ROOT when started from the repo root: makes the gxana libraries
// usable from interpreted macros (build first: see docs/environment.md).
{
    const char* gxanaRoot = gSystem->Getenv("GXANA_ROOT");
    if (gxanaRoot) {
        // Explicit dynamic path: macOS SIP may strip DYLD_LIBRARY_PATH.
        gSystem->AddDynamicPath(Form("%s/build/lib", gxanaRoot));
        gInterpreter->AddIncludePath(Form("%s/packages/common/include", gxanaRoot));
        gInterpreter->AddIncludePath(Form("%s/packages/xsection/include", gxanaRoot));
        for (const char* lib : {"libGxanaCommon", "libGxanaXsec"})
            if (gSystem->Load(lib) < 0)
                Warning("rootlogon", "%s not found; run cmake --build build", lib);
    }
}
