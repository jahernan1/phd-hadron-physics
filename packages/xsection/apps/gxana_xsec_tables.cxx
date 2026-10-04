// Fit binned trees and write the legacy cross-section tables (gxana::xsec::WriteXSecTables).
#include "CliArgs.h"
#include "gxana/fit/Fit.h"
#include "gxana/xsection/Flux.h"
#include "gxana/xsection/XSec.h"
#include "gxana/xsection/YieldFit.h"

#include <RooMsgService.h>
#include <TH1D.h>
#include <TROOT.h>

#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
const char* kUsage =
    "usage: gxana_xsec_tables --fit TYPE [--param NAME=INIT,MIN,MAX ...] --out DIR\n"
    "                         --weight BRANCH --label LABEL [--cheby 1|2] JOB [JOB ...]\n"
    "                         [[--weight BRANCH] [--label LABEL] [--cheby 1|2] JOB [JOB ...] ...]\n"
    "                         [--plots PLOTDIR] CHANNEL\n"
    "  CHANNEL (all required; from analyses/<channel>/config, passed by gxana run):\n"
    "          --observable BRANCH --observable-title TITLE --gate EXPR --qvalue-branch BRANCH|none\n"
    "          --br VALUE,ERROR --target ZMIN,ZMAX,DENSITY,MOLAR_MASS,ATOMS\n"
    "          --mass-window NAME=GEV for each NAME below\n"
    "  TYPE    Johnson | Gaussian | Voigtian signal; background Chebychev of order --cheby (default 2)\n"
    "          JohnsonMCShape: thesis fit (legacy MakeXSecFiles.C, data/<combo weight>/);\n"
    "          needs mu, lambda, gamma, delta (MC-fit start,min,max), --cheby 2, fresh per bin\n"
    "          JohnsonMCShapeSyst: the same with the literals of the legacy GetXSecFilesUML.C\n"
    "          variation fit (Barlow systematics); same parameters and --cheby 2\n"
    "          MCPdf: signal = RooHistPdf of the MC mass shape, background Chebychev of order\n"
    "          --cheby; takes no --param (legacy MakeXSecFitMC.C)\n"
    "  JOB     NAME:DATA:MC:THROWN:FLUX -- binned data/MC/thrown ROOT files and flux file;\n"
    "          NAME prefixes the tables (legacy: flatTree_<tree stem>). If DATA holds\n"
    "          directories (gxana_xsec_bin variation output), each vary_<cut>_<value>\n"
    "          directory is fitted against MC's vary_<cut>_<value>_mc directory and\n"
    "          THROWN's top-level trees; tables are named <table>_NAME_vary_<cut>_<value>...\n"
    "  --out   each JOB writes its tables into DIR/LABEL/ (one directory per label)\n"
    "  --plots save fit PDFs under PLOTDIR/LABEL/\n"
    "  --weight event-weight branch of the data and MC trees\n"
    "  --observable  fitted mass branch; --observable-title its axis title\n"
    "  --mass-window  fit windows: lo, mc_hi, mc_signal_hi, mc_plot_hi, data_hi, data_edge, mcpdf_data_lo\n"
    "  --gate  selection a bin's data tree must pass (> 10 entries, > 25 JohnsonMCShape) to be fitted\n"
    "  --qvalue-branch  Q-factor branch summed into the qval columns; none: nan columns\n"
    "  --br    branching ratio of the decay chain and its error (added in quadrature per point)\n"
    "  --target  liquid target z range (cm), density (g/cm^3), molar mass (g/mol), atoms per molecule\n"
    "  All jobs run in order, in this one process, sharing one set of fit parameters\n"
    "  (each fit updates them). --label, --cheby and --weight are order-sensitive: each\n"
    "  JOB uses whichever of them last preceded it, so repeating them mid-command-line\n"
    "  runs further JOBs with the same mutated parameters under a new label -- e.g.\n"
    "  --label johnson ... --cheby 1 --label johnson_cheby1 ... reproduces the legacy\n"
    "  johnson -> johnson_cheby1 chaining. A JOB before the first --label or --weight is a\n"
    "  usage error.\n";
} // namespace

int main(int argc, char** argv)
{
    gROOT->SetBatch(true);
    gxana::fit::UseThesisMinimizer();
    std::string fitType, outDir, plotDir, weight;
    std::string label;
    bool haveLabel = false;
    int chebyOrder = 2;
    gxana::xsec::FitParams params;
    gxana::xsec::XSecPhysics physics{};
    std::set<std::string> channelFlags, windows;
    std::vector<gxana::cli::XSecJob> jobs;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "-h" || arg == "--help") {
                std::cout << kUsage;
                return 0;
            }
            if (arg.rfind("--", 0) == 0) {
                if (i + 1 >= argc)
                    throw std::invalid_argument(arg + " needs a value");
                const std::string value = argv[++i];
                if (arg == "--fit")
                    fitType = value;
                else if (arg == "--param")
                    params.insert(gxana::cli::ParseParam(value));
                else if (arg == "--label") {
                    label = value;
                    haveLabel = true;
                } else if (arg == "--out")
                    outDir = value;
                else if (arg == "--plots")
                    plotDir = value;
                else if (arg == "--weight")
                    weight = value;
                else if (arg == "--cheby")
                    chebyOrder = gxana::cli::ParseChebyOrder(value);
                else if (arg == "--gate")
                    physics.gate = value;
                else if (arg == "--qvalue-branch")
                    physics.qvalueBranch = gxana::cli::ParseQValueBranch(value);
                else if (arg == "--br")
                    gxana::cli::ParseBranchingRatio(value, physics.br, physics.brErr);
                else if (arg == "--target")
                    physics.target = gxana::cli::ParseTarget(value);
                else if (arg == "--observable")
                    physics.obs.branch = value;
                else if (arg == "--observable-title")
                    physics.obs.title = value;
                else if (arg == "--mass-window")
                    windows.insert(gxana::cli::SetMassWindow(physics.windows, value));
                else
                    throw std::invalid_argument("unknown option " + arg);
                channelFlags.insert(arg);
            } else {
                if (!haveLabel)
                    throw std::invalid_argument("JOB given before --label: '" + arg + "'");
                if (weight.empty())
                    throw std::invalid_argument("JOB given before --weight: '" + arg + "'");
                jobs.push_back(gxana::cli::ParseJob(arg, label, chebyOrder, weight));
            }
        }
        if (fitType.empty() || outDir.empty() || jobs.empty() ||
            (params.empty() && !gxana::xsec::IsMCPdfFit(fitType)))
            throw std::invalid_argument("missing arguments");
        for (const char* flag : {"--observable", "--observable-title", "--gate", "--qvalue-branch", "--br",
                                 "--target"})
            if (!channelFlags.count(flag))
                throw std::invalid_argument(std::string("missing channel flag ") + flag);
        if (windows.size() != 7)
            throw std::invalid_argument("--mass-window needs all of lo, mc_hi, mc_signal_hi, mc_plot_hi, data_hi, "
                                        "data_edge, mcpdf_data_lo");
        if (gxana::xsec::IsMCShapeFit(fitType))
            gxana::cli::CheckMCShapeArgs(params, jobs);
    } catch (const std::invalid_argument& err) {
        std::cerr << "gxana_xsec_tables: " << err.what() << "\n" << kUsage;
        return 2;
    }

    try {
        // Legacy MakeXSecFitVariations.C main(): style, then RooFit verbosity.
        gxana::xsec::SetFitStyle();
        RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
        gxana::xsec::SetFitPlotDir(plotDir);
        for (const auto& job : jobs) {
            std::unique_ptr<TH1D> flux(gxana::xsec::GetFluxHist(job.flux));
            flux->SetName("tagged_flux");
            gxana::xsec::WriteXSecTables(job.data, job.mc, job.thrown, flux.get(), job.name, job.label,
                                         fitType, params, gxana::cli::LabelOutDir(outDir, job.label), physics,
                                         job.weight, job.chebyOrder);
        }
    } catch (const std::exception& err) {
        std::cerr << "gxana_xsec_tables: " << err.what() << "\n";
        return 1;
    }
    return 0;
}
