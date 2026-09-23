#include "gxana/xsection/Flux.h"

#include <TFile.h>

#include <memory>
#include <stdexcept>

namespace gxana {
namespace xsec {

TH1D* GetFluxHist(const std::string& fluxFile, const std::string& histName)
{
    std::unique_ptr<TFile> file(TFile::Open(fluxFile.c_str(), "READ"));
    if (!file || file->IsZombie())
        throw std::runtime_error("GetFluxHist: cannot open " + fluxFile);
    auto* hist = dynamic_cast<TH1D*>(file->Get(histName.c_str()));
    if (!hist)
        throw std::runtime_error("GetFluxHist: no TH1D '" + histName + "' in " + fluxFile);
    hist = static_cast<TH1D*>(hist->Clone());
    hist->SetDirectory(nullptr);
    return hist;
}

} // namespace xsec
} // namespace gxana
