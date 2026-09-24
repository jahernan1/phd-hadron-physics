#include "gxana/common/Style.h"
#include "gxana/common/GraphIO.h"
#include "gxana/xsection/Plotting.h"
#include "gxana/common/Paths.h"

// Main function
int PlotDiffXSec() {
    gxana::SetStyle();
    gxana::xsec::SetPlotDir(gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/plots"));
    vector<string> dataType = {"acc_weight", "best_combo", "hybrid_combo",
        "qvalues","oneRfBunch", "s17_rest3", "oneEBin",
        "gaus","gaus_cheby1", "voigt", "voigt_cheby1",
        "johnson", "johnson_cheby1", "mcPdf", "mcPdf_cheby1"};
    std::vector<TGraphErrors*> weighted_graphs, syst_weighted_graphs, s17_graphs, s18_graphs, f18_graphs;

    // Call the function, specifying directory and pattern (if needed)
    for(const auto type : dataType )
        {
            // gxana: legacy computed the output ROOT filename from a "delim" it parsed out of the
            // first matched file's name (kpkpxim..._emin); CreateTGraphErrorsFromTxt no longer
            // does that itself, so we name the file from the (dir, type) it already has instead.
            weighted_graphs = gxana::CreateTGraphErrorsFromTxt("weighted_data/"+type,
                                                        "weighted_diffxsec*",
                                                        "WeightedDiffXSecTGraphs_"+type+".root");
            syst_weighted_graphs = gxana::CreateTGraphErrorsFromTxt("weighted_data/"+type,
                                                        "syst_weighted_diffxsec*",
                                                        "SystWeightedDiffXSecTGraphs_"+type+".root");

            //
            s17_graphs = gxana::CreateTGraphErrorsFromTxt("data/"+type,
                                                   "diffxsec*2017-01*",
                                                   "DiffXSecTGraphs_2017-01_"+type+".root");
            s18_graphs = gxana::CreateTGraphErrorsFromTxt("data/"+type,
                                                   "diffxsec*2018-01*",
                                                   "DiffXSecTGraphs_2018-01_"+type+".root");
            f18_graphs = gxana::CreateTGraphErrorsFromTxt("data/"+type,
                                                   "diffxsec*2018-08*",
                                                   "DiffXSecTGraphs_2018-08_"+type+".root");

            // Do something with the returned vector of graphs if needed
            std::cout << "Number of graphs created: " << weighted_graphs.size() << std::endl;
            if(weighted_graphs.size()>1)
                {
                    gxana::xsec::plotWeightedXSec(weighted_graphs, 2.5, 20, "diffxsec_phase1_weighted_"+type);
                    gxana::xsec::plotDiffXSec({s17_graphs,s18_graphs,f18_graphs}, 2.5, 20, "diffxsec_runs_"+type);
                }
            else
                gxana::xsec::plotOneWeightedXSec(weighted_graphs, 2.5, 20, "diffxsec_phase1_weighted_"+type);

            if(type=="hybrid_combo" || type=="johnson")
                gxana::xsec::plotFinalWeightedXSec({weighted_graphs,syst_weighted_graphs},2.5,20,"diffxsec_phase1_systematics_"+type);

            //Clean vectors for iteration
            s17_graphs.clear(); s18_graphs.clear(); f18_graphs.clear();
        }

    // Return success
    return 0;
}
