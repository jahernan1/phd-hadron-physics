// Cut scan with a figure of merit (port of selection/CutAnalysisRF.C; gxana/studies/CutScan.h).
// Planned by `gxana run studies` (packages/studies/python/gxana_studies/stage.py).
#include "gxana/common/Cli.h"
#include "gxana/common/PeriodHists.h"
#include "gxana/studies/CutScan.h"

#include <TFile.h>
#include <TH2.h>
#include <TROOT.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
const char* kUsage =
    "usage: gxana_study_cutscan fill --input FILE --tree T --out FILE.root --mass VAR:N,LO,HI --scan VAR:N,LO,HI\n"
    "                                [--weight COL] [--define NAME=EXPR | --filter EXPR]... [--threads N]\n"
    "         Fills the mass-vs-cut TH2D (key \"cutscan\") after the defines and filters, in the given order.\n"
    "       gxana_study_cutscan fit --hist FILE.root --tables PATH --grid-pdf PDF --first-bin N --panel-label TEXT\n"
    "                                --mass-title TEXT --range LO,HI --param NAME=BRACKET (a0 a1 mu lambda gamma delta nbkgd nxi)\n"
    "         Fits the cumulative projections, writes PATH with {what} = FOM, SB, Yield and the fit grid PDF.\n"
    "       gxana_study_cutscan plot --tables PATH --title TEXT --cut X --name NAME --pdf PDF [--pdf PDF]...\n"
    "         Draws the FOM and S/B tables with the cut line; saves every --pdf.\n";

// "VAR:N,LO,HI" -> var, {N, LO, HI}
std::pair<std::string, std::vector<double>> ParseAxis(const std::string& opt, const std::string& value)
{
    const auto colon = value.find(':');
    if (colon == std::string::npos || colon == 0)
        throw std::invalid_argument(opt + " needs VAR:N,LO,HI: '" + value + "'");
    auto axis = gxana::cli::ParseDoubleList(value.substr(colon + 1));
    if (axis.size() != 3 || axis[0] < 1 || axis[0] != static_cast<int>(axis[0]))
        throw std::invalid_argument(opt + " needs VAR:N,LO,HI with integer N >= 1: '" + value + "'");
    return {value.substr(0, colon), axis};
}

int Fill(const std::vector<std::string>& args)
{
    std::string input, out, mass, scan;
    std::vector<double> massAxis, scanAxis;
    gxana::FillSpec spec;
    gxana::HistDef h;
    int threads = 4; // GetFilterHist's n_threads
    for (size_t i = 0; i < args.size(); i += 2) {
        if (i + 1 >= args.size())
            throw std::invalid_argument(args[i] + " needs a value");
        const std::string& arg = args[i];
        const std::string& value = args[i + 1];
        if (arg == "--input") input = value;
        else if (arg == "--tree") spec.tree = value;
        else if (arg == "--out") out = value;
        else if (arg == "--weight") h.weight = value;
        else if (arg == "--threads") threads = static_cast<int>(gxana::cli::ParseDouble(value));
        else if (arg == "--define") {
            const auto kv = gxana::cli::SplitAssign(value);
            spec.steps.push_back({kv.first, kv.second});
        } else if (arg == "--filter") spec.steps.push_back({"", value});
        else if (arg == "--mass") { auto a = ParseAxis(arg, value); mass = a.first; massAxis = a.second; }
        else if (arg == "--scan") { auto a = ParseAxis(arg, value); scan = a.first; scanAxis = a.second; }
        else throw std::invalid_argument("unknown option " + arg);
    }
    if (input.empty() || out.empty() || spec.tree.empty() || mass.empty() || scan.empty())
        throw std::invalid_argument("fill needs --input, --tree, --out, --mass and --scan");
    h.key = "cutscan";
    h.columns = {mass, scan};
    h.axes = {massAxis[0], massAxis[1], massAxis[2], scanAxis[0], scanAxis[1], scanAxis[2]};
    spec.hists = {h};
    std::unique_ptr<TFile> f(TFile::Open(out.c_str(), "RECREATE"));
    if (!f || f->IsZombie())
        throw std::runtime_error("cannot write " + out);
    gxana::FillHists(input, spec, f.get(), threads);
    f->Close();
    return 0;
}

int Fit(const std::vector<std::string>& args)
{
    std::string histFile;
    gxana::studies::CutScanFit fit;
    bool haveRange = false, haveFirst = false;
    for (size_t i = 0; i < args.size(); i += 2) {
        if (i + 1 >= args.size())
            throw std::invalid_argument(args[i] + " needs a value");
        const std::string& arg = args[i];
        const std::string& value = args[i + 1];
        if (arg == "--hist") histFile = value;
        else if (arg == "--tables") fit.tables = value;
        else if (arg == "--grid-pdf") fit.gridPdf = value;
        else if (arg == "--first-bin") { fit.firstBin = static_cast<int>(gxana::cli::ParseDouble(value)); haveFirst = true; }
        else if (arg == "--panel-label") fit.panelLabel = value;
        else if (arg == "--mass-title") fit.massTitle = value;
        else if (arg == "--range") {
            const auto r = gxana::cli::ParseDoubleList(value);
            if (r.size() != 2)
                throw std::invalid_argument("--range needs LO,HI: '" + value + "'");
            fit.lo = r[0];
            fit.hi = r[1];
            haveRange = true;
        } else if (arg == "--param") {
            const auto kv = gxana::cli::SplitAssign(value);
            fit.params[kv.first] = kv.second;
        } else throw std::invalid_argument("unknown option " + arg);
    }
    if (histFile.empty() || fit.tables.empty() || fit.gridPdf.empty() || fit.panelLabel.empty() ||
        fit.massTitle.empty() || !haveRange || !haveFirst)
        throw std::invalid_argument("fit needs --hist, --tables, --grid-pdf, --first-bin, --panel-label, --mass-title, --range");
    for (const auto& name : gxana::studies::CutScanParams())
        if (!fit.params.count(name))
            throw std::invalid_argument("fit needs --param " + name + "=...");
    for (const auto& kv : fit.params) {
        bool known = false;
        for (const auto& name : gxana::studies::CutScanParams())
            known = known || kv.first == name;
        if (!known)
            throw std::invalid_argument("unknown --param " + kv.first);
    }
    std::unique_ptr<TFile> f(TFile::Open(histFile.c_str()));
    TH2* hist = f ? dynamic_cast<TH2*>(f->Get("cutscan")) : nullptr;
    if (!hist)
        throw std::runtime_error(histFile + ": no TH2 cutscan");
    hist->SetDirectory(nullptr);
    gxana::studies::ApplyCutScanStyle();
    gxana::studies::FitCutScan(*hist, fit);
    return 0;
}

int Plot(const std::vector<std::string>& args)
{
    std::string tables, title, name;
    std::vector<std::string> pdfs;
    double cut = 0;
    bool haveCut = false;
    for (size_t i = 0; i < args.size(); i += 2) {
        if (i + 1 >= args.size())
            throw std::invalid_argument(args[i] + " needs a value");
        const std::string& arg = args[i];
        const std::string& value = args[i + 1];
        if (arg == "--tables") tables = value;
        else if (arg == "--title") title = value;
        else if (arg == "--name") name = value;
        else if (arg == "--pdf") pdfs.push_back(value);
        else if (arg == "--cut") { cut = gxana::cli::ParseDouble(value); haveCut = true; }
        else throw std::invalid_argument("unknown option " + arg);
    }
    if (tables.empty() || title.empty() || name.empty() || pdfs.empty() || !haveCut)
        throw std::invalid_argument("plot needs --tables, --title, --cut, --name and at least one --pdf");
    gxana::studies::ApplyCutScanStyle();
    gxana::studies::ApplyCutScanPlotMargins();
    gxana::studies::PlotCutScan(tables, title, cut, name, pdfs);
    return 0;
}
} // namespace

int main(int argc, char** argv)
{
    gROOT->SetBatch(true);
    if (argc < 2 || std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
        std::cout << kUsage;
        return argc < 2 ? 2 : 0;
    }
    const std::string step = argv[1];
    const std::vector<std::string> args(argv + 2, argv + argc);
    try {
        if (step == "fill") return Fill(args);
        if (step == "fit") return Fit(args);
        if (step == "plot") return Plot(args);
        throw std::invalid_argument("unknown step " + step + " (fill, fit, plot)");
    } catch (const std::invalid_argument& err) {
        std::cerr << "gxana_study_cutscan: error: " << err.what() << "\n" << kUsage;
        return 2;
    } catch (const std::exception& err) {
        std::cerr << "gxana_study_cutscan: error: " << err.what() << "\n";
        return 1;
    }
}
