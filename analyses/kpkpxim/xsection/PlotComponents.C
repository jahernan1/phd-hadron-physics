#include "gxana/common/Style.h"
#include "gxana/common/GraphIO.h"
#include "gxana/xsection/Plotting.h"
#include "gxana/common/Paths.h"

// Main function
// gxana: renamed from legacy's PlotDiffXSec() (identical name to
// xsection/PlotDiffXSec.C's entry point) to PlotComponents() so the two
// macros' entry points do not clash when both are loaded.
int PlotComponents() {
    gxana::SetStyle();
    gxana::xsec::SetPlotDir(gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/xsection/plots"));
    //
    vector<string> dataType = {"hybrid_combo", "acc_weight", "best_combo","qvalues", "oneRfBunch"};
    std::vector<TGraphErrors*> weighted_graphs, s17_graphs, s18_graphs, f18_graphs;

    // Call the function, specifying directory and pattern (if needed)
    for(const auto type : dataType )
        {
            // gxana: see xsection/PlotDiffXSec.C for why the output ROOT filename is now
            // derived from (dir, type) instead of legacy's parsed "delim".
            weighted_graphs = gxana::CreateTGraphErrorsFromTxt("weighted_data/"+type,
                                                        "*diffxsec*.txt",
                                                        "WeightedDiffXSecTGraphs_"+type+".root");
            if(type=="oneRfBunch")
                s17_graphs = gxana::CreateTGraphErrorsFromTxt("data/hybrid_combo",
                                                       "diffxsec*2017-01*.txt",
                                                       "DiffXSecTGraphs_2017-01_"+type+".root");
            else
                s17_graphs = gxana::CreateTGraphErrorsFromTxt("data/"+type,
                                                       "diffxsec*2017-01*.txt",
                                                       "DiffXSecTGraphs_2017-01_"+type+".root");
            s18_graphs = gxana::CreateTGraphErrorsFromTxt("data/"+type,
                                                   "diffxsec*2018-01*.txt",
                                                   "DiffXSecTGraphs_2018-01_"+type+".root");
            f18_graphs = gxana::CreateTGraphErrorsFromTxt("data/"+type,
                                                   "diffxsec*2018-08*.txt",
                                                   "DiffXSecTGraphs_2018-08_"+type+".root");

            // Do something with the returned vector of graphs if needed
            std::cout << "Number of graphs created: " << weighted_graphs.size() << std::endl;
            gxana::xsec::plotWeightedXSec(weighted_graphs, 2.5, 20, "diffxsec_phase1_weighted_"+type);
            gxana::xsec::plotDiffXSec({s17_graphs,s18_graphs,f18_graphs}, 2.5, 20, "diffxsec_runs_"+type);
            //Clean vectors for iteration
            s17_graphs.clear(); s18_graphs.clear(); f18_graphs.clear();
        }
    // Return success
    return 0;
}
