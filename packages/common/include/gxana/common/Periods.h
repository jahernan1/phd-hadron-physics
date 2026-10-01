#ifndef GXANA_COMMON_PERIODS_H
#define GXANA_COMMON_PERIODS_H

// Run periods for C++: the channel's period list, directory names and tree stems, read from the
// flat file `gxana config export --channel C` writes ($GXANA_OUTPUT/<C>/config/channel.kv), and
// the per-period read-back and merge loops the analysis macros used to copy.

#include <TDirectory.h>

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace gxana {

// One run period and its three input files (empty = not used).
struct Period {
    std::string name;                 // period key, e.g. "2017-01"
    std::string data, mc, thrown;     // input files
    std::string dir;                  // ROOT directory; empty = name
    const std::string& Dir() const { return dir.empty() ? name : dir; }
};

class ChannelInfo {
public:
    // $GXANA_OUTPUT/<channel>/config/channel.kv
    static ChannelInfo Load(const std::string& channel);
    // Reads `path`; throws when a line is malformed or a listed source YAML file is missing or
    // has another MD5 than recorded ("stale channel.kv: run gxana config export --channel C").
    static ChannelInfo LoadFile(const std::string& path);

    const std::vector<std::string>& PeriodNames() const { return periods_; }
    // period.<period>.<key> (dir, title, label)
    std::string PeriodValue(const std::string& period, const std::string& key) const;
    // stem.<period>.<sample>
    std::string Stem(const std::string& period, const std::string& sample = "data") const;
    // Any key; throws naming the key and the file when it is absent.
    std::string Get(const std::string& key) const;

private:
    std::string path_, channel_;
    std::map<std::string, std::string> values_;
    std::vector<std::string> periods_;
};

// Replaces every {name} by vars.at(name); an unknown name or an unclosed '{' throws.
std::string ExpandPattern(const std::string& pattern, const std::map<std::string, std::string>& vars);

// File-name patterns of one site. Placeholders: {period}, {stem} (data stem), {mc_stem}
// (stem of mcSample, or of the channel's mc_sample when mcSample is empty; only expanded when
// the pattern uses it).
struct InputPatterns {
    std::string data, mc, thrown;
    std::string mcSample;
};

// One Period per channel period, in channel order, dir = period.<p>.dir.
std::vector<Period> MakePeriods(const ChannelInfo& info, const InputPatterns& patterns);

// f->Get(<dir>/<name>)->Clone(cloneName) for every period, in order; throws on a missing key.
template <class T>
std::vector<T*> GetPeriodHists(TDirectory* f, const std::vector<Period>& periods, const std::string& name,
                               const std::string& cloneName = "")
{
    std::vector<T*> out;
    for (const auto& p : periods) {
        const std::string path = p.Dir() + "/" + name;
        auto* h = dynamic_cast<T*>(f->Get(path.c_str()));
        if (!h)
            throw std::runtime_error(std::string(f->GetName()) + ": no " + T::Class()->GetName() + " " + path);
        out.push_back(static_cast<T*>(h->Clone(cloneName.c_str())));
    }
    return out;
}

// hists[0]->Clone(name) plus hists[k]->Clone() for k >= 1, added in order: the legacy
// "Clone the first, Add clones of the others" merge. name "" keeps the first one's name.
template <class T>
T* MergeHists(const std::vector<T*>& hists, const std::string& name = "")
{
    if (hists.empty())
        throw std::invalid_argument("MergeHists: no histograms");
    T* sum = static_cast<T*>(hists[0]->Clone(name.c_str()));
    for (size_t k = 1; k < hists.size(); ++k)
        sum->Add(static_cast<T*>(hists[k]->Clone()));
    return sum;
}

} // namespace gxana

#endif
