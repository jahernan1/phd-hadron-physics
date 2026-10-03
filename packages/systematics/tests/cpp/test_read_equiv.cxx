// Reader equivalence: ReadLabelGraphs, ReadPeriodGraphs and
// gxana::ReadBinnedGraphs against frozen copies of the two readers as they were before commit 21f6727,
// on synthetic directories whose files are created in shuffled order. Order, names, titles,
// points and errors must be bit-identical, and errors must have the same type and message.
#include "gxana/common/GraphIO.h"
#include "gxana/systematics/PlotSpread.h"

#include <TGraphErrors.h>
#include <TSystem.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <regex>
#include <stdexcept>
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

namespace frozen {

using std::string;

// packages/systematics/src/PlotSpread.cxx:1007-1040 before commit 21f6727 (ReadLabelGraphs), verbatim.
std::vector<TGraphErrors*> ReadLabelGraphs(const std::string& dir)
{
    static const std::regex kWeighted(R"(^weighted_diffxsec_emin_(\d+\.\d+)_emax_(\d+\.\d+)\.txt$)");
    void* handle = gSystem->OpenDirectory(dir.c_str());
    if (!handle) throw std::runtime_error("cannot open directory " + dir);
    struct Bin {
        double emin;
        std::string file, enMin, enMax;
    };
    std::vector<Bin> bins;
    while (const char* entry = gSystem->GetDirEntry(handle)) {
        std::smatch m;
        const std::string file = entry;
        if (std::regex_match(file, m, kWeighted)) bins.push_back({std::stod(m[1].str()), file, m[1], m[2]});
    }
    gSystem->FreeDirectory(handle);
    if (bins.empty()) throw std::runtime_error("no weighted_diffxsec_emin_*_emax_*.txt in " + dir);
    std::sort(bins.begin(), bins.end(), [](const Bin& a, const Bin& b) { return a.emin < b.emin; });

    std::vector<TGraphErrors*> graphs;
    for (const auto& bin : bins) {
        const std::string fullPath = dir + "/" + bin.file;
        const std::string name = bin.file.substr(0, bin.file.find_last_of("."));
        TGraphErrors* graph = new TGraphErrors(fullPath.c_str());
        if (graph->GetN() == 0) throw std::runtime_error("no points in " + fullPath);
        const std::string& enMin = bin.enMin;
        const std::string& enMax = bin.enMax;
        graph->SetName(name.c_str());
        // gxana: the panel title of the dissertation figures
        graph->SetTitle(("#bf{E_{#gamma} (GeV): (" + enMin + ", " + enMax + ")}").c_str());
        graphs.push_back(graph);
    }
    return graphs;
}

// packages/systematics/src/PlotSpread.cxx:1046-1082 before commit 21f6727 (ReadPeriodGraphs), verbatim.
std::vector<TGraphErrors*> ReadPeriodGraphs(const std::string& prefix)
{
    static const std::regex kBin(R"(^_emin_(\d+\.\d+)_emax_(\d+\.\d+)\.txt$)");
    const auto slash = prefix.find_last_of('/');
    const std::string dir = slash == std::string::npos ? "." : prefix.substr(0, slash);
    const std::string stem = slash == std::string::npos ? prefix : prefix.substr(slash + 1);
    void* handle = gSystem->OpenDirectory(dir.c_str());
    if (!handle) throw std::runtime_error("cannot open directory " + dir);
    struct Bin {
        double emin;
        std::string file, enMin, enMax;
    };
    std::vector<Bin> bins;
    while (const char* entry = gSystem->GetDirEntry(handle)) {
        std::smatch m;
        const std::string file = entry;
        if (file.compare(0, stem.size(), stem) != 0) continue;
        const std::string rest = file.substr(stem.size());
        if (std::regex_match(rest, m, kBin)) bins.push_back({std::stod(m[1].str()), file, m[1], m[2]});
    }
    gSystem->FreeDirectory(handle);
    if (bins.empty()) throw std::runtime_error("no " + prefix + "_emin_*_emax_*.txt");
    std::sort(bins.begin(), bins.end(), [](const Bin& a, const Bin& b) { return a.emin < b.emin; });

    std::vector<TGraphErrors*> graphs;
    for (const auto& bin : bins) {
        const std::string fullPath = dir + "/" + bin.file;
        const std::string name = bin.file.substr(0, bin.file.find_last_of("."));
        TGraphErrors* graph = new TGraphErrors(fullPath.c_str());
        if (graph->GetN() == 0) throw std::runtime_error("no points in " + fullPath);
        graph->SetName(name.c_str());
        // gxana: the panel title of the dissertation figures
        graph->SetTitle(("#bf{E_{#gamma} (GeV): (" + bin.enMin + ", " + bin.enMax + ")}").c_str());
        graphs.push_back(graph);
    }
    return graphs;
}

} // namespace frozen

namespace {

const std::vector<std::string> kEmin{"6.40", "7.40", "7.86", "8.19", "8.45", "8.68", "9.26", "10.18"};
const std::vector<std::string> kEmax{"7.40", "7.86", "8.19", "8.45", "8.68", "9.26", "10.18", "11.40"};

std::string TempDir(const std::string& name)
{
    const std::string dir = std::string(gSystem->TempDirectory()) + "/" + name;
    gSystem->Exec(("rm -rf " + dir).c_str());
    gSystem->mkdir(dir.c_str(), true);
    return dir;
}

// The eight configured energy bins of one table stem plus decoys that must not be read,
// written in an order shuffled with a fixed seed.
void WriteTables(const std::string& dir, const std::string& stem, unsigned seed)
{
    std::vector<std::pair<std::string, std::string>> files;
    for (size_t b = 0; b < kEmin.size(); ++b) {
        std::string text = "tBinCenter\tdsigmadt\ttBinWidth\tYerr\n";
        for (int i = 0; i < 3; ++i)
            text += std::to_string(0.2 + 0.3 * i) + " " + std::to_string(1.0 / (3.0 + b + 7.0 * i + seed)) + " 0.1 " +
                    std::to_string(0.1 / (1.0 + b + i)) + "\n";
        files.push_back({stem + "_emin_" + kEmin[b] + "_emax_" + kEmax[b] + ".txt", text});
    }
    files.push_back({"syst_" + stem + "_emin_6.40_emax_7.40.txt", "0.2 99 0.1 1\n"});
    files.push_back({stem + "x_emin_6.40_emax_7.40.txt", "0.2 99 0.1 1\n"});
    files.push_back({stem + "_emin_6.40_emax_7.40.txt.bak", "0.2 99 0.1 1\n"});
    files.push_back({stem + "_emin_6_emax_7.txt", "0.2 99 0.1 1\n"});
    std::mt19937 rng(seed);
    std::shuffle(files.begin(), files.end(), rng);
    for (const auto& f : files) std::ofstream(dir + "/" + f.first) << f.second;
}

using Reader = std::function<std::vector<TGraphErrors*>()>;

// "ok" or "<type>: <message>"
std::string Outcome(const Reader& read, std::vector<TGraphErrors*>& graphs)
{
    try {
        graphs = read();
    } catch (const std::runtime_error& e) {
        return std::string("runtime_error: ") + e.what();
    } catch (const std::exception& e) {
        return std::string("exception: ") + e.what();
    }
    return "ok";
}

bool SameGraphs(const std::vector<TGraphErrors*>& a, const std::vector<TGraphErrors*>& b)
{
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::string(a[i]->GetName()) != b[i]->GetName() || std::string(a[i]->GetTitle()) != b[i]->GetTitle() ||
            a[i]->GetN() != b[i]->GetN())
            return false;
        for (int p = 0; p < a[i]->GetN(); ++p)
            if (a[i]->GetX()[p] != b[i]->GetX()[p] || a[i]->GetY()[p] != b[i]->GetY()[p] ||
                a[i]->GetEX()[p] != b[i]->GetEX()[p] || a[i]->GetEY()[p] != b[i]->GetEY()[p])
                return false;
    }
    return true;
}

void Expect(const std::string& what, const Reader& old, const Reader& now)
{
    std::vector<TGraphErrors*> a, b;
    const std::string oa = Outcome(old, a), ob = Outcome(now, b);
    if (oa != ob || !SameGraphs(a, b)) {
        ++failures;
        std::cerr << "READER MISMATCH " << what << ": old [" << oa << "] new [" << ob << "]\n";
    }
}

} // namespace

int main()
{
    using namespace gxana::systematics;
    const std::string base = TempDir("gxana_read_equiv_" + std::to_string(gSystem->GetPid()));

    // Label directories: weighted_diffxsec_emin_*_emax_*.txt
    for (unsigned seed : {1u, 2u, 3u}) {
        const std::string dir = base + "/label" + std::to_string(seed);
        gSystem->mkdir(dir.c_str(), true);
        WriteTables(dir, "weighted_diffxsec", seed);
        Expect("label " + dir, [&] { return frozen::ReadLabelGraphs(dir); }, [&] { return ReadLabelGraphs(dir); });
    }
    std::vector<TGraphErrors*> label;
    CHECK(Outcome([&] { return ReadLabelGraphs(base + "/label1"); }, label) == "ok");
    CHECK(label.size() == 8 && std::string(label[0]->GetName()) == "weighted_diffxsec_emin_6.40_emax_7.40" &&
          std::string(label[7]->GetTitle()) == "#bf{E_{#gamma} (GeV): (10.18, 11.40)}");

    // Period prefixes: <dir>/<stem>_emin_*_emax_*.txt, several stems in one directory.
    const std::string pdir = base + "/periods";
    gSystem->mkdir(pdir.c_str(), true);
    WriteTables(pdir, "diffxsec_flatTree_a", 4);
    WriteTables(pdir, "diffxsec_flatTree_ab", 5);
    WriteTables(pdir, "diffout_flatTree_a", 6);
    for (const std::string stem : {"diffxsec_flatTree_a", "diffxsec_flatTree_ab"})
        Expect("period " + stem, [&] { return frozen::ReadPeriodGraphs(pdir + "/" + stem); },
               [&] { return ReadPeriodGraphs(pdir + "/" + stem); });

    // Errors: missing directory, nothing matches, a table without points, prefix without '/'.
    Expect("label missing dir", [&] { return frozen::ReadLabelGraphs(base + "/absent"); },
           [&] { return ReadLabelGraphs(base + "/absent"); });
    Expect("label no match", [&] { return frozen::ReadLabelGraphs(pdir); }, [&] { return ReadLabelGraphs(pdir); });
    Expect("period missing dir", [&] { return frozen::ReadPeriodGraphs(base + "/absent/diffxsec_x"); },
           [&] { return ReadPeriodGraphs(base + "/absent/diffxsec_x"); });
    Expect("period no match", [&] { return frozen::ReadPeriodGraphs(pdir + "/diffxsec_none"); },
           [&] { return ReadPeriodGraphs(pdir + "/diffxsec_none"); });
    Expect("period no slash", [&] { return frozen::ReadPeriodGraphs("gxana_no_such_stem"); },
           [&] { return ReadPeriodGraphs("gxana_no_such_stem"); });
    const std::string edir = base + "/empty";
    gSystem->mkdir(edir.c_str(), true);
    WriteTables(edir, "weighted_diffxsec", 7);
    std::ofstream(edir + "/weighted_diffxsec_emin_8.45_emax_8.68.txt") << "tBinCenter\tdsigmadt\ttBinWidth\tYerr\n";
    Expect("label empty table", [&] { return frozen::ReadLabelGraphs(edir); }, [&] { return ReadLabelGraphs(edir); });

    // gxana::ReadBinnedGraphs directly, with the label readers' prefix.
    for (unsigned seed : {1u, 2u, 3u}) {
        const std::string dir = base + "/label" + std::to_string(seed);
        Expect("binned " + dir, [&] { return frozen::ReadLabelGraphs(dir); },
               [&] { return gxana::ReadBinnedGraphs(dir, "weighted_diffxsec"); });
    }
    Expect("binned missing dir", [&] { return frozen::ReadLabelGraphs(base + "/absent"); },
           [&] { return gxana::ReadBinnedGraphs(base + "/absent", "weighted_diffxsec"); });
    Expect("binned no match", [&] { return frozen::ReadLabelGraphs(pdir); },
           [&] { return gxana::ReadBinnedGraphs(pdir, "weighted_diffxsec"); });
    Expect("binned empty table", [&] { return frozen::ReadLabelGraphs(edir); },
           [&] { return gxana::ReadBinnedGraphs(edir, "weighted_diffxsec"); });
    // Two tables with the same emin value (6.4 and 6.40): the old readers' order was
    // unspecified (std::sort on equal keys); ReadBinnedGraphs refuses.
    const std::string ddir = base + "/duplicate";
    gSystem->mkdir(ddir.c_str(), true);
    WriteTables(ddir, "weighted_diffxsec", 8);
    std::ofstream(ddir + "/weighted_diffxsec_emin_6.4_emax_7.40.txt") << "0.2 1 0.1 0.1\n";
    std::vector<TGraphErrors*> dup;
    CHECK(Outcome([&] { return gxana::ReadBinnedGraphs(ddir, "weighted_diffxsec"); }, dup) ==
          "runtime_error: two tables with the same emin in " + ddir +
              ": weighted_diffxsec_emin_6.40_emax_7.40.txt, weighted_diffxsec_emin_6.4_emax_7.40.txt");

    std::cout << "test_read_equiv: " << (failures == 0 ? "all checks passed" : "FAILED") << "\n";
    gSystem->Exec(("rm -rf " + base).c_str());
    return failures == 0 ? 0 : 1;
}
