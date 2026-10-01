#include "gxana/fit/Model.h"

#include <RooWorkspace.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace gxana {
namespace fit {

namespace {

std::string Arg(const Param& p)
{
    return p.bracket.empty() ? p.name : p.name + "[" + p.bracket + "]";
}

std::string Join(const std::vector<Param>& ps)
{
    std::string out;
    for (std::size_t i = 0; i < ps.size(); ++i)
        out += (i ? "," : "") + Arg(ps[i]);
    return out;
}

std::string Pdf(const std::string& kind, const std::string& pdf, const std::string& obs,
                const std::vector<Param>& ps)
{
    return kind + "::" + pdf + "(" + obs + "," + Join(ps) + ")";
}

} // namespace

std::string Fx(double v)
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%f", v);
    return buf;
}

std::string Johnson(const std::string& pdf, const std::string& obs, const Param& mu, const Param& lambda,
                    const Param& gamma, const Param& delta)
{
    return Pdf("Johnson", pdf, obs, {mu, lambda, gamma, delta});
}

std::string Gaussian(const std::string& pdf, const std::string& obs, const Param& mean, const Param& sigma)
{
    return Pdf("Gaussian", pdf, obs, {mean, sigma});
}

std::string Voigtian(const std::string& pdf, const std::string& obs, const Param& mean, const Param& width,
                     const Param& sigma)
{
    return Pdf("Voigtian", pdf, obs, {mean, width, sigma});
}

std::string BreitWigner(const std::string& pdf, const std::string& obs, const Param& mean, const Param& width)
{
    return Pdf("BreitWigner", pdf, obs, {mean, width});
}

std::string Chebychev(const std::string& pdf, const std::string& obs, const std::vector<Param>& coefs)
{
    return "Chebychev::" + pdf + "(" + obs + ",{" + Join(coefs) + "})";
}

std::string Threshold(const std::string& pdf, const std::string& obs, const Param& m0, const Param& b,
                      const Param& p)
{
    const std::string x = "(" + obs + ")";
    const std::string t = "((" + x + "/" + m0.name + ")**2-1.0)";
    return "EXPR::" + pdf + "('" + x + "*" + t + "**" + p.name + "*exp(" + b.name + "*" + t + ")'," + obs + "," +
           Join({m0, b, p}) + ")";
}

std::string Sum(const std::string& name, const std::vector<std::pair<Param, std::string>>& yieldTimesPdf)
{
    std::string out = "SUM::" + name + "(";
    for (std::size_t i = 0; i < yieldTimesPdf.size(); ++i)
        out += (i ? "," : "") + Arg(yieldTimesPdf[i].first) + "*" + yieldTimesPdf[i].second;
    return out + ")";
}

void BuildModel(RooWorkspace& w, const std::vector<std::string>& statements)
{
    const char* trace = std::getenv("GXANA_FIT_TRACE");
    for (const auto& s : statements) {
        if (trace && *trace) std::cout << "FACTORY " << s << std::endl;
        w.factory(s.c_str());
    }
}

} // namespace fit
} // namespace gxana
