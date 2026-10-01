// Track-efficiency kinematics, data/MC angle figures and theta-split counts (port of
// get_hists.C + get_track_efficiency.C; see gxana/systematics/TrackHists.h).
#include "gxana/common/Cli.h"
#include "gxana/systematics/TrackHists.h"

#include <TROOT.h>
#include <TSystem.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
const char* kUsage =
    "usage: gxana_syst_track --out-dir D --tree T --thrown-tree T [--data-weight EXPR] [--mc-weight EXPR]\n"
    "                        --theta-cut DEG --low F --high F [--legend-header TEXT]\n"
    "                        --period NAME:DATA:MC:THROWN [--period ...]\n"
    "                        --particle NAME:P4:THROWNP4:NT,TLO,THI:NP,PLO,PHI:TITLE [--particle ...]\n"
    "  Fills <name>_kin_{qval,mc,thrown} (theta in degrees vs |p|) per period from the data (weighted\n"
    "  by --data-weight), MC (--mc-weight) and thrown (unweighted) trees into D/particle_kinematics.root,\n"
    "  merges the periods, draws D/<name>_kin_angle_phase1_mc_data_mc.pdf and writes the data and MC\n"
    "  counts below/above --theta-cut to D/track_counts.txt. The TITLE field may contain ':'.\n";

gxana::systematics::TrackPeriod ParsePeriod(const std::string& value)
{
    const auto parts = gxana::cli::Split(value, ':');
    if (parts.size() != 4)
        throw std::invalid_argument("--period needs NAME:DATA:MC:THROWN: '" + value + "'");
    for (const auto& part : parts)
        if (part.empty())
            throw std::invalid_argument("empty field in --period '" + value + "'");
    return {parts[0], parts[1], parts[2], parts[3]};
}

// Split on the first five ':' only: the title may contain ':'.
gxana::systematics::TrackParticle ParseParticle(const std::string& value)
{
    const char* kForm = "--particle needs NAME:P4:THROWNP4:NT,TLO,THI:NP,PLO,PHI:TITLE: '";
    std::vector<std::string> fields;
    size_t start = 0;
    for (int k = 0; k < 5; ++k) {
        const auto colon = value.find(':', start);
        if (colon == std::string::npos)
            throw std::invalid_argument(kForm + value + "'");
        fields.push_back(value.substr(start, colon - start));
        start = colon + 1;
    }
    fields.push_back(value.substr(start));
    for (int k = 0; k < 5; ++k)
        if (fields[k].empty())
            throw std::invalid_argument(kForm + value + "'");
    const auto theta = gxana::cli::ParseDoubleList(fields[3]);
    const auto p = gxana::cli::ParseDoubleList(fields[4]);
    if (theta.size() != 3 || p.size() != 3 || theta[0] < 1 || theta[0] != static_cast<int>(theta[0]) ||
        p[0] < 1 || p[0] != static_cast<int>(p[0]))
        throw std::invalid_argument(std::string("--particle binning must be N,LO,HI with integer N >= 1: '") +
                                    value + "'");
    gxana::systematics::TrackParticle part;
    part.name = fields[0];
    part.p4 = fields[1];
    part.thrownP4 = fields[2];
    part.nTheta = static_cast<int>(theta[0]);
    part.thetaLo = theta[1];
    part.thetaHi = theta[2];
    part.nP = static_cast<int>(p[0]);
    part.pLo = p[1];
    part.pHi = p[2];
    part.title = fields[5];
    return part;
}
} // namespace

int main(int argc, char** argv)
{
    gROOT->SetBatch(true);
    gxana::systematics::TrackSpec spec;
    bool haveCut = false, haveLow = false, haveHigh = false;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "-h" || arg == "--help") {
                std::cout << kUsage;
                return 0;
            }
            if (i + 1 >= argc)
                throw std::invalid_argument(arg + " needs a value");
            const std::string value = argv[++i];
            if (arg == "--out-dir") spec.outDir = value;
            else if (arg == "--tree") spec.tree = value;
            else if (arg == "--thrown-tree") spec.thrownTree = value;
            else if (arg == "--data-weight") spec.dataWeight = value;
            else if (arg == "--mc-weight") spec.mcWeight = value;
            else if (arg == "--theta-cut") { spec.thetaCut = gxana::cli::ParseDouble(value); haveCut = true; }
            else if (arg == "--low") { spec.low = gxana::cli::ParseDouble(value); haveLow = true; }
            else if (arg == "--high") { spec.high = gxana::cli::ParseDouble(value); haveHigh = true; }
            else if (arg == "--legend-header") spec.legendHeader = value;
            else if (arg == "--period") spec.periods.push_back(ParsePeriod(value));
            else if (arg == "--particle") spec.particles.push_back(ParseParticle(value));
            else
                throw std::invalid_argument("unknown option " + arg);
        }
        if (spec.outDir.empty() || spec.tree.empty() || spec.thrownTree.empty())
            throw std::invalid_argument("missing arguments: --out-dir, --tree and --thrown-tree are required");
        if (!haveCut || !haveLow || !haveHigh)
            throw std::invalid_argument("missing arguments: --theta-cut, --low and --high are required");
        if (spec.periods.empty())
            throw std::invalid_argument("needs at least one --period");
        if (spec.particles.empty())
            throw std::invalid_argument("needs at least one --particle");
    } catch (const std::invalid_argument& err) {
        std::cerr << "gxana_syst_track: error: " << err.what() << "\n" << kUsage;
        return 2;
    }
    try {
        for (const auto& period : spec.periods)
            for (const auto& path : {period.data, period.mc, period.thrown})
                if (gSystem->AccessPathName(path.c_str()))  // true when the file does not exist
                    throw std::runtime_error("period " + period.name + ": no such file " + path);
        gxana::systematics::MakeKinematics(spec);
        gxana::systematics::CountAndDraw(spec);
    } catch (const std::exception& err) {
        std::cerr << "gxana_syst_track: error: " << err.what() << "\n";
        return 1;
    }
    return 0;
}
