// Load (not run) one macro under cling with the gxana libraries, as a
// migrated macro would be used: root -l -b -q rootlogon.C 'load_macro.C("path")'.
void load_macro(const char* path)
{
    int err = 0;
    gROOT->ProcessLine(Form(".L %s", path), &err);
    if (err != 0) {
        std::cerr << "LOAD FAILED: " << path << std::endl;
        gSystem->Exit(1);
    }
}
