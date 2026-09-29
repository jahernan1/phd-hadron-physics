// Fit binned trees and write the legacy cross-section tables (gxana::xsec::WriteXSecTables).
#include "CliArgs.h"
#include "gxana/xsection/Flux.h"
#include "gxana/xsection/XSec.h"
#include "gxana/xsection/YieldFit.h"

#include <RooMsgService.h>
#include <TH1D.h>
#include <TROOT.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
const char* kUsage =
    "usage: gxana_xsec_tables --fit TYPE --param NAME=INIT,MIN,MAX [--param ...] --out DIR\n"
    "                         --label LABEL [--cheby 1|2] JOB [JOB ...]\n"
    "                         [[--label LABEL] [--cheby 1|2] JOB [JOB ...] ...]\n"
    "                         [--plots PLOTDIR] [--weight BRANCH]\n"
    "  TYPE    Johnson | Gaussian | Voigtian signal; background Chebychev of order --cheby (default 2)\n"
    "          JohnsonMCShape: thesis fit (legacy MakeXSecFiles.C, data/<combo weight>/);\n"
    "          needs mu, lambda, gamma, delta (MC-fit start,min,max), --cheby 2, fresh per bin\n"
    "  JOB     NAME:DATA:MC:THROWN:FLUX -- binned data/MC/thrown ROOT files and flux file;\n"
    "          NAME prefixes the tables (legacy: flatTree_<tree stem>)\n"
    "  --out   each JOB writes its tables into DIR/LABEL/ (one directory per label)\n"
    "  --plots save fit PDFs under PLOTDIR/LABEL/\n"
    "  --weight event-weight branch (default hybrid_combo; e.g. best_combo, acc_weight)\n"
    "  All jobs run in order, in this one process, sharing one set of fit parameters\n"
    "  (each fit updates them). --label, --cheby and --weight are order-sensitive: each\n"
    "  JOB uses whichever of them last preceded it, so repeating them mid-command-line\n"
    "  runs further JOBs with the same mutated parameters under a new label -- e.g.\n"
    "  --label johnson ... --cheby 1 --label johnson_cheby1 ... reproduces the legacy\n"
    "  johnson -> johnson_cheby1 chaining. A JOB before the first --label is a usage error.\n";
} // namespace

int main(int argc, char** argv)
{
    gROOT->SetBatch(true);
    std::string fitType, outDir, plotDir, weight = "hybrid_combo";
    std::string label;
    bool haveLabel = false;
    int chebyOrder = 2;
    gxana::xsec::FitParams params;
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
                else
                    throw std::invalid_argument("unknown option " + arg);
            } else {
                if (!haveLabel)
                    throw std::invalid_argument("JOB given before --label: '" + arg + "'");
                jobs.push_back(gxana::cli::ParseJob(arg, label, chebyOrder, weight));
            }
        }
        if (fitType.empty() || params.empty() || outDir.empty() || jobs.empty())
            throw std::invalid_argument("missing arguments");
        if (fitType == gxana::xsec::kJohnsonMCShape)
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
                                         fitType, params, gxana::cli::LabelOutDir(outDir, job.label), job.weight,
                                         job.chebyOrder);
        }
    } catch (const std::exception& err) {
        std::cerr << "gxana_xsec_tables: " << err.what() << "\n";
        return 1;
    }
    return 0;
}
