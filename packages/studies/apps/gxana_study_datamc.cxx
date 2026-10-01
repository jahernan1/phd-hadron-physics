// Data/MC kinematic comparison (port of archive/root_macros/GetKinematicsDataMC_RF.C;
// gxana/studies/DataMC.h). Planned by `gxana run studies`.
#include "gxana/common/Cli.h"
#include "gxana/common/PeriodHists.h"
#include "gxana/studies/DataMC.h"

#include <ROOT/RDataFrame.hxx>
#include <RConfigure.h>
#include <TFile.h>
#include <TROOT.h>

#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
const char* kUsage =
    "usage: gxana_study_datamc fill --out FILE.root --period NAME:DIR:DATA:MC:THROWN [--period ...]\n"
    "                               --tree T --thrown-tree T --var VAR [--var ...] [--truth-var VAR ...]\n"
    "                               [--define SAMPLE:NAME=EXPR | --filter SAMPLE:EXPR]... [--weight SAMPLE:COL]...\n"
    "                               [--threads N]\n"
    "         SAMPLE is data, mc or thrown. Per period directory DIR: data <VAR> (RDataFrame default\n"
    "         binning), mc <VAR>_mc and thrown <VAR>_thrown with the data histogram's binning. Every\n"
    "         --truth-var must also be a --var. Prints the sum of the data weight per period.\n"
    "       gxana_study_datamc plot --in FILE.root --out-dir D --period DIR:TAG [--period ...]\n"
    "                               --var VAR:POS:TITLE [--var ...] [--truth-var VAR:POS:TITLE ...]\n"
    "         POS tl or tr. Draws data vs mc (<name>_<TAG>_ac.pdf) and thrown vs mc\n"
    "         (<name>_<TAG>_MC_Truth_ac.pdf) for every period.\n";

const std::vector<std::string> kSamples{"data", "mc", "thrown"};

std::pair<std::string, std::string> SplitSample(const std::string& opt, const std::string& value)
{
    const auto colon = value.find(':');
    const std::string sample = colon == std::string::npos ? "" : value.substr(0, colon);
    bool known = false;
    for (const auto& s : kSamples)
        known = known || s == sample;
    if (!known || colon + 1 >= value.size())
        throw std::invalid_argument(opt + " needs SAMPLE:VALUE with SAMPLE data, mc or thrown: '" + value + "'");
    return {sample, value.substr(colon + 1)};
}

int Fill(const std::vector<std::string>& args)
{
    std::string out, tree, thrownTree;
    std::vector<gxana::Period> periods;
    std::vector<std::string> vars, truthVars;
    std::map<std::string, std::vector<gxana::Step>> steps;
    std::map<std::string, std::string> weights;
    int threads = 8; // GetDataMCPlots's n_threads
    for (size_t i = 0; i < args.size(); i += 2) {
        if (i + 1 >= args.size())
            throw std::invalid_argument(args[i] + " needs a value");
        const std::string& arg = args[i];
        const std::string& value = args[i + 1];
        if (arg == "--out") out = value;
        else if (arg == "--tree") tree = value;
        else if (arg == "--thrown-tree") thrownTree = value;
        else if (arg == "--var") vars.push_back(value);
        else if (arg == "--truth-var") truthVars.push_back(value);
        else if (arg == "--threads") threads = static_cast<int>(gxana::cli::ParseDouble(value));
        else if (arg == "--period") {
            const auto f = gxana::cli::Split(value, ':');
            if (f.size() != 5)
                throw std::invalid_argument("--period needs NAME:DIR:DATA:MC:THROWN: '" + value + "'");
            for (const auto& part : f)
                if (part.empty())
                    throw std::invalid_argument("empty field in --period '" + value + "'");
            gxana::Period p;
            p.name = f[0];
            p.dir = f[1];
            p.data = f[2];
            p.mc = f[3];
            p.thrown = f[4];
            periods.push_back(p);
        } else if (arg == "--define") {
            const auto sv = SplitSample(arg, value);
            const auto kv = gxana::cli::SplitAssign(sv.second);
            steps[sv.first].push_back({kv.first, kv.second});
        } else if (arg == "--filter") {
            const auto sv = SplitSample(arg, value);
            steps[sv.first].push_back({"", sv.second});
        } else if (arg == "--weight") {
            const auto sv = SplitSample(arg, value);
            weights[sv.first] = sv.second;
        } else throw std::invalid_argument("unknown option " + arg);
    }
    if (out.empty() || tree.empty() || thrownTree.empty() || periods.empty() || vars.empty())
        throw std::invalid_argument("fill needs --out, --tree, --thrown-tree, --period and --var");
    for (const auto& t : truthVars) {
        bool found = false;
        for (const auto& v : vars)
            found = found || v == t;
        if (!found)
            throw std::invalid_argument("--truth-var " + t + " is not a --var");
    }
    gxana::FillSpec data{tree, steps["data"], {}, {}}, mc{tree, steps["mc"], {}, {}},
        thrown{thrownTree, steps["thrown"], {}, {}};
    for (const auto& v : vars) {
        gxana::HistDef d;
        d.key = v;
        d.columns = {v};
        d.weight = weights["data"];
        data.hists.push_back(d);
        gxana::HistDef m;
        m.key = v + "_mc";
        m.columns = {v};
        m.weight = weights["mc"];
        m.like = v;
        mc.hists.push_back(m);
    }
    for (const auto& v : truthVars) {
        gxana::HistDef t;
        t.key = v + "_thrown";
        t.columns = {v};
        t.weight = weights["thrown"];
        t.like = v;
        thrown.hists.push_back(t);
    }
#ifdef R__USE_IMT
    if (threads > 0)
        ROOT::EnableImplicitMT(threads); // before the sum, as GetDataMCPlots
#endif
    if (!weights["data"].empty()) {
        for (const auto& p : periods) {
            ROOT::RDF::RNode df = ROOT::RDataFrame(tree, p.data);
            for (const auto& s : data.steps)
                df = s.define.empty() ? df.Filter(s.expr) : df.Define(s.define, s.expr);
            std::cout << "Sum of Weighted Q-Values: " << df.Sum<double>(weights["data"]).GetValue() << std::endl;
        }
    }
    std::unique_ptr<TFile> f(TFile::Open(out.c_str(), "RECREATE"));
    if (!f || f->IsZombie())
        throw std::runtime_error("cannot write " + out);
    gxana::FillPeriodHists(periods, {{gxana::Input::Data, data}, {gxana::Input::MC, mc}, {gxana::Input::Thrown, thrown}},
                           f.get(), threads);
    f->Close();
    return 0;
}

struct PlotVar {
    std::string var, pos, title;
};

PlotVar ParsePlotVar(const std::string& opt, const std::string& value)
{
    const auto a = value.find(':');
    const auto b = a == std::string::npos ? a : value.find(':', a + 1);
    if (b == std::string::npos || a == 0)
        throw std::invalid_argument(opt + " needs VAR:POS:TITLE: '" + value + "'");
    PlotVar v{value.substr(0, a), value.substr(a + 1, b - a - 1), value.substr(b + 1)};
    if (v.pos != "tl" && v.pos != "tr")
        throw std::invalid_argument(opt + ": POS must be tl or tr: '" + value + "'");
    return v;
}

// Relies on Get() returning a fresh object for each call (checked for these files): the truth plot
// re-reads "<var>_mc" and must not get the copy the data plot already rebinned and scaled.
TH1D* Read(TFile& f, const std::string& path)
{
    auto* h = dynamic_cast<TH1D*>(f.Get(path.c_str()));
    if (!h)
        throw std::runtime_error(std::string(f.GetName()) + ": no TH1D " + path);
    return h;
}

int Plot(const std::vector<std::string>& args)
{
    std::string in, outDir;
    std::vector<std::pair<std::string, std::string>> periods;
    std::vector<PlotVar> vars, truthVars;
    for (size_t i = 0; i < args.size(); i += 2) {
        if (i + 1 >= args.size())
            throw std::invalid_argument(args[i] + " needs a value");
        const std::string& arg = args[i];
        const std::string& value = args[i + 1];
        if (arg == "--in") in = value;
        else if (arg == "--out-dir") outDir = value;
        else if (arg == "--var") vars.push_back(ParsePlotVar(arg, value));
        else if (arg == "--truth-var") truthVars.push_back(ParsePlotVar(arg, value));
        else if (arg == "--period") {
            const auto colon = value.find(':');
            if (colon == std::string::npos || colon == 0 || colon + 1 == value.size())
                throw std::invalid_argument("--period needs DIR:TAG: '" + value + "'");
            periods.push_back({value.substr(0, colon), value.substr(colon + 1)});
        } else throw std::invalid_argument("unknown option " + arg);
    }
    if (in.empty() || outDir.empty() || periods.empty() || vars.empty())
        throw std::invalid_argument("plot needs --in, --out-dir, --period and --var");
    std::unique_ptr<TFile> f(TFile::Open(in.c_str()));
    if (!f || f->IsZombie())
        throw std::runtime_error("cannot read " + in);
    gxana::studies::ApplyDataMCStyle();
    for (const auto& p : periods) {
        for (const auto& v : vars)
            gxana::studies::DrawStacked(Read(*f, p.first + "/" + v.var), Read(*f, p.first + "/" + v.var + "_mc"), v.title,
                                        p.second, v.pos, "Data", outDir);
        for (const auto& v : truthVars)
            gxana::studies::DrawStacked(Read(*f, p.first + "/" + v.var + "_thrown"), Read(*f, p.first + "/" + v.var + "_mc"),
                                        v.title, p.second + "_MC_Truth", v.pos, "Gen MC", outDir);
    }
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
        if (step == "plot") return Plot(args);
        throw std::invalid_argument("unknown step " + step + " (fill, plot)");
    } catch (const std::invalid_argument& err) {
        std::cerr << "gxana_study_datamc: error: " << err.what() << "\n" << kUsage;
        return 2;
    } catch (const std::exception& err) {
        std::cerr << "gxana_study_datamc: error: " << err.what() << "\n";
        return 1;
    }
}
