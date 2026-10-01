#ifndef GXANA_FIT_MODEL_H
#define GXANA_FIT_MODEL_H

// RooWorkspace factory statements for the lineshape pdfs the analysis macros use, and the
// call that issues them. A statement is plain text: the builders below only assemble it,
// so a site keeps its own numbers verbatim (Param::bracket) and its own call order.

#include <string>
#include <utility>
#include <vector>

class RooWorkspace;

namespace gxana {
namespace fit {

// One factory argument: "name[bracket]", or the bare "name" (a reference to an object the
// workspace already holds) when bracket is empty. bracket is the text between the square
// brackets exactly as the site wrote it, e.g. "1.3217,1.32,1.33" or "0" (a constant).
struct Param {
    std::string name;
    std::string bracket;
};

// A double as printf "%f" writes it (6 decimals): the format of the macros' Form() starts.
std::string Fx(double v);

// "Kind::pdf(obs,p1,p2,...)"
std::string Johnson(const std::string& pdf, const std::string& obs, const Param& mu, const Param& lambda,
                    const Param& gamma, const Param& delta);
std::string Gaussian(const std::string& pdf, const std::string& obs, const Param& mean, const Param& sigma);
std::string Voigtian(const std::string& pdf, const std::string& obs, const Param& mean, const Param& width,
                     const Param& sigma);
std::string BreitWigner(const std::string& pdf, const std::string& obs, const Param& mean, const Param& width);
// "Chebychev::pdf(obs,{a0,a1,...})"
std::string Chebychev(const std::string& pdf, const std::string& obs, const std::vector<Param>& coefs);
// Threshold background "EXPR::pdf('(x)*(((x)/m0)**2-1.0)**p*exp(b*(((x)/m0)**2-1.0))',x,m0,b,p)"
// with x = obs and m0, b, p the given parameter names.
std::string Threshold(const std::string& pdf, const std::string& obs, const Param& m0, const Param& b,
                      const Param& p);
// "SUM::name(y1*pdf1,y2*pdf2,...)"
std::string Sum(const std::string& name, const std::vector<std::pair<Param, std::string>>& yieldTimesPdf);

// Issues the statements through w.factory() in the given order (the creation order fixes
// the Minuit parameter order). Return values are not checked, as in the macros. With the
// environment variable GXANA_FIT_TRACE set (non-empty), prints "FACTORY <statement>" to
// stdout before each call.
void BuildModel(RooWorkspace& w, const std::vector<std::string>& statements);

} // namespace fit
} // namespace gxana

#endif
