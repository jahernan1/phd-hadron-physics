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
    "usage: gxana_xsec_tables --fit TYPE --param NAME=INIT,MIN,MAX [--param ...] --label LABEL --out DIR\n"
    "                         [--plots DIR] [--weight BRANCH] [--cheby 1|2] JOB [JOB ...]\n"
    "  TYPE    Johnson | Gaussian | Voigtian signal; background Chebychev of order --cheby (default 2)\n"
    "  JOB     NAME:DATA:MC:THROWN:FLUX -- binned data/MC/thrown ROOT files and flux file;\n"
    "          NAME prefixes the tables (legacy: flatTree_<tree stem>)\n"
    "  --plots save fit PDFs under DIR/LABEL/; --weight defaults to hybrid_combo\n"
    "  Jobs run in order and share the fit parameters (each fit updates them).\n";
}

int main(int argc, char** argv)
{
    gROOT->SetBatch(true);
    std::string fitType, label, outDir, plotDir, weight = "hybrid_combo";
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
                else if (arg == "--label")
                    label = value;
                else if (arg == "--out")
                    outDir = value;
                else if (arg == "--plots")
                    plotDir = value;
                else if (arg == "--weight")
                    weight = value;
                else if (arg == "--cheby")
                    chebyOrder = static_cast<int>(gxana::cli::ParseDouble(value));
                else
                    throw std::invalid_argument("unknown option " + arg);
            } else {
                jobs.push_back(gxana::cli::ParseJob(arg));
            }
        }
        if (fitType.empty() || params.empty() || label.empty() || outDir.empty() || jobs.empty())
            throw std::invalid_argument("missing arguments");
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
            gxana::xsec::WriteXSecTables(job.data, job.mc, job.thrown, flux.get(), job.name, label,
                                         fitType, params, outDir, weight, chebyOrder);
        }
    } catch (const std::exception& err) {
        std::cerr << "gxana_xsec_tables: " << err.what() << "\n";
        return 1;
    }
    return 0;
}
