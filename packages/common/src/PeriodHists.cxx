#include "gxana/common/PeriodHists.h"

#include <ROOT/RDataFrame.hxx>
#include <RConfigure.h>
#include <TROOT.h>

#include <functional>
#include <map>
#include <stdexcept>

namespace gxana {

void FillHists(const std::string& file, const FillSpec& spec, TDirectory* out, int nThreads)
{
#ifdef R__USE_IMT
    if (nThreads > 0)
        ROOT::EnableImplicitMT(nThreads);
#else
    (void)nThreads;
#endif
    ROOT::RDF::RNode df = ROOT::RDataFrame(spec.tree, file);
    for (const auto& step : spec.steps)
        df = step.define.empty() ? df.Filter(step.expr) : df.Define(step.define, step.expr);
    std::map<std::string, ROOT::RDF::RNode> frames;
    for (const auto& frame : spec.frames)
        frames.emplace(frame.first, df.Filter(frame.second));

    std::vector<std::function<void()>> writes;
    for (const auto& h : spec.hists) {
        ROOT::RDF::RNode node = df;
        if (!h.frame.empty()) {
            const auto it = frames.find(h.frame);
            if (it == frames.end())
                throw std::invalid_argument("histogram " + h.key + ": unknown frame " + h.frame);
            node = it->second;
        }
        const std::string key = h.key;
        if (h.columns.size() == 1 && h.axes.size() == 3) {
            ROOT::RDF::TH1DModel model(h.model.c_str(), h.title.c_str(), static_cast<int>(h.axes[0]), h.axes[1], h.axes[2]);
            auto r = h.weight.empty() ? node.Histo1D(model, h.columns[0]) : node.Histo1D(model, h.columns[0], h.weight);
            writes.push_back([r, key]() mutable { r->Write(key.c_str(), TObject::kOverwrite); });
        } else if (h.columns.size() == 2 && h.axes.size() == 6) {
            ROOT::RDF::TH2DModel model(h.model.c_str(), h.title.c_str(), static_cast<int>(h.axes[0]), h.axes[1], h.axes[2],
                                       static_cast<int>(h.axes[3]), h.axes[4], h.axes[5]);
            auto r = h.weight.empty() ? node.Histo2D(model, h.columns[0], h.columns[1])
                                      : node.Histo2D(model, h.columns[0], h.columns[1], h.weight);
            writes.push_back([r, key]() mutable { r->Write(key.c_str(), TObject::kOverwrite); });
        } else {
            throw std::invalid_argument("histogram " + key + ": need 1 column and 3 axis values or 2 columns and 6");
        }
    }
    out->cd();
    for (auto& w : writes)
        w();
}

void FillPeriodHists(const std::vector<Period>& periods, const std::vector<std::pair<Input, FillSpec>>& jobs,
                     TFile* out, int nThreads)
{
    for (const auto& p : periods) {
        out->cd();
        out->mkdir(p.Dir().c_str());
    }
    for (const auto& p : periods) {
        out->cd();
        TDirectory* dir = out->GetDirectory(p.Dir().c_str());
        if (!dir)
            throw std::runtime_error(std::string(out->GetName()) + ": no directory " + p.Dir());
        for (const auto& job : jobs) {
            const std::string& file = job.first == Input::Data ? p.data : (job.first == Input::MC ? p.mc : p.thrown);
            if (file.empty())
                throw std::invalid_argument("period " + p.name + ": no input file for a job");
            FillHists(file, job.second, dir, nThreads);
        }
    }
}

} // namespace gxana
