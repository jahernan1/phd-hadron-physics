#include "gxana/common/Periods.h"

#include "gxana/common/Paths.h"

#include <TMD5.h>
#include <TSystem.h>

#include <fstream>
#include <memory>
#include <sstream>

namespace gxana {

namespace {

std::vector<std::string> SplitComma(const std::string& text)
{
    std::vector<std::string> out;
    std::stringstream ss(text);
    std::string item;
    while (std::getline(ss, item, ','))
        out.push_back(item);
    return out;
}

} // namespace

ChannelInfo ChannelInfo::Load(const std::string& channel)
{
    const std::string path = EnvPath("GXANA_OUTPUT", channel + "/config/channel.kv");
    if (!std::ifstream(path))
        throw std::runtime_error("cannot read " + path + ": run gxana config export --channel " + channel);
    return LoadFile(path);
}

ChannelInfo ChannelInfo::LoadFile(const std::string& path)
{
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("cannot read " + path + ": run gxana config export --channel <channel>");
    ChannelInfo info;
    info.path_ = path;
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (line.empty() || line[0] == '#')
            continue;
        const auto eq = line.find('=');
        if (eq == std::string::npos || eq == 0)
            throw std::runtime_error(path + ":" + std::to_string(lineNo) + ": expected key=value");
        info.values_[line.substr(0, eq)] = line.substr(eq + 1);
    }
    info.channel_ = info.Get("channel");
    const std::string stale = "stale " + path + ": run gxana config export --channel " + info.channel_;
    const std::string dir = info.Get("source_dir");
    for (const auto& kv : info.values_) {
        const std::string& key = kv.first;
        const std::string prefix = "source.", suffix = ".md5";
        if (key.compare(0, prefix.size(), prefix) != 0 || key.size() <= prefix.size() + suffix.size() ||
            key.compare(key.size() - suffix.size(), suffix.size(), suffix) != 0)
            continue;
        const std::string file = dir + "/" + key.substr(prefix.size(), key.size() - prefix.size() - suffix.size());
        if (gSystem->AccessPathName(file.c_str()))  // true when the file does not exist
            throw std::runtime_error(stale + " (" + file + " is missing)");
        std::unique_ptr<TMD5> md5(TMD5::FileChecksum(file.c_str()));
        if (!md5 || kv.second != md5->AsString())
            throw std::runtime_error(stale + " (" + file + " changed)");
    }
    info.periods_ = SplitComma(info.Get("periods"));
    return info;
}

std::string ChannelInfo::Get(const std::string& key) const
{
    const auto it = values_.find(key);
    if (it == values_.end())
        throw std::runtime_error(path_ + ": no key " + key);
    return it->second;
}

std::string ChannelInfo::PeriodValue(const std::string& period, const std::string& key) const
{
    return Get("period." + period + "." + key);
}

std::string ChannelInfo::Stem(const std::string& period, const std::string& sample) const
{
    return Get("stem." + period + "." + sample);
}

std::string ExpandPattern(const std::string& pattern, const std::map<std::string, std::string>& vars)
{
    std::string out;
    size_t pos = 0;
    while (pos < pattern.size()) {
        const auto open = pattern.find('{', pos);
        if (open == std::string::npos) {
            out += pattern.substr(pos);
            break;
        }
        const auto close = pattern.find('}', open);
        if (close == std::string::npos)
            throw std::invalid_argument("unclosed '{' in pattern '" + pattern + "'");
        const std::string name = pattern.substr(open + 1, close - open - 1);
        const auto it = vars.find(name);
        if (it == vars.end())
            throw std::invalid_argument("unknown placeholder {" + name + "} in pattern '" + pattern + "'");
        out += pattern.substr(pos, open - pos) + it->second;
        pos = close + 1;
    }
    return out;
}

std::vector<Period> MakePeriods(const ChannelInfo& info, const InputPatterns& patterns)
{
    const bool needMc = (patterns.data + patterns.mc + patterns.thrown).find("{mc_stem}") != std::string::npos;
    const std::string sample = !needMc ? "" : (patterns.mcSample.empty() ? info.Get("mc_sample") : patterns.mcSample);
    std::vector<Period> out;
    for (const auto& name : info.PeriodNames()) {
        std::map<std::string, std::string> vars = {{"period", name}, {"stem", info.Stem(name)}};
        if (needMc)
            vars["mc_stem"] = info.Stem(name, sample);
        Period p;
        p.name = name;
        p.dir = info.PeriodValue(name, "dir");
        p.data = patterns.data.empty() ? "" : ExpandPattern(patterns.data, vars);
        p.mc = patterns.mc.empty() ? "" : ExpandPattern(patterns.mc, vars);
        p.thrown = patterns.thrown.empty() ? "" : ExpandPattern(patterns.thrown, vars);
        out.push_back(p);
    }
    return out;
}

} // namespace gxana
