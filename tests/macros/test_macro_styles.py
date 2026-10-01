"""Each analysis macro's local style function (setStyle / style_format / SetStyle) leaves gStyle
exactly as its legacy body does (tests/macros/legacy_styles.C), from ROOT's default style and
from a dirty start (style_snapshot.C DirtyStart) that proves members the body does not write
stay untouched. The whole TBufferJSON dump of gStyle and gROOT->GetForceStyle() are compared
as strings. The legacy bodies run in one ROOT process, each site in its own.
With GXANA_STYLE_DUMP_DIR set, every dump is kept there.
"""
import os
import shutil
import subprocess
from itertools import combinations
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
pytestmark = [pytest.mark.macros,
              pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")]

STARTS = ("default", "dirty")
# site macro -> (its style call, its copy in legacy_styles.C)
SITES = {
    "analyses/kpkpkmlamb/measurements/FitXimStar.C": ("setStyle()", "FitXimStar"),
    "analyses/kpkpxim/signal_extraction/qfactors/scripts/GetQvalueSum.C": ("setStyle()", "GetQvalueSum"),
    "analyses/kpkpxim/backgrounds/YstarBWFitsData.C": ("setStyle()", "GetQvalueSum"),
    "analyses/kpkpxim/selection/cut_studies/xim_vertex_cuts/make_plot.C": ("style_format()", "GetQvalueSum"),
    "analyses/kpkpxim/signal_extraction/lineshape/OneUMLFit.C": ("setStyle()", "OneUMLFit"),
    "analyses/kpkpxim/measurements/mass/MakeXim1320_IM.C": ("setStyle()", "OneUMLFit"),
    "analyses/kpkpxim/measurements/mass/MakeXim1320_IM_Res.C": ("setStyle()", "OneUMLFit"),
    "analyses/kpkpxim/signal_extraction/lineshape/SingleGaussianFit.C": ("setStyle()", "GaussianFit"),
    "analyses/kpkpxim/signal_extraction/lineshape/DoubleGaussianFit.C": ("setStyle()", "GaussianFit"),
    "analyses/kpkpxim/selection/XimMassQVal.C": ("setStyle()", "XimMassQVal"),
    "analyses/kpkpxim/selection/cut_studies/lambda_vertex_cut/make_plot.C": ("style_format()", "LambdaVertexMakePlot"),
    "analyses/kpkpxim/selection/cut_studies/xim_vertex_cuts/make_plot_RF.C": ("style_format()", "LambdaVertexMakePlot"),
    "analyses/kpkpxim/selection/cut_studies/accidentals/make_plot.C": ("style_format()", "AccidentalsMakePlot"),
    "analyses/kpkpxim/selection/cut_studies/chisqndf_cut/make_plot_chisqndf.C": ("style_format()", "ChisqNdfMakePlot"),
    "analyses/kpkpxim/selection/GetKinematicsDataMC.C": ("setStyle()", "GetKinematicsDataMC"),
    "analyses/kpkpxim/selection/GetKinematicsDataMC_RF.C": ("setStyle()", "GetKinematicsDataMC_RF"),
    "analyses/kpkpxim/selection/cut_studies/mm2_cut/make_plot.C": ("style_format()", "Mm2MakePlot"),
    "analyses/kpkpxim/selection/flatTreeCuts.C": ("setStyle()", "FlatTreeCuts"),
    "analyses/kpkpxim/selection/mc_studies/make_plot_RF.C": ("style_format()", "McStudiesMakePlotRF"),
    "analyses/kpkpxim/simulation/validation/make_plot_RF.C": ("style_format()", "ValidationMakePlotRF"),
    "analyses/kpkpxim/selection/mc_studies/make_plot.C": ("style_format()", "McStudiesMakePlot"),
    "analyses/kpkpxim/selection/mc_studies/make_plot_acceptcorr.C": ("style_format()", "McStudiesAcceptCorr"),
    "analyses/kpkpxim/selection/CutAnalysis.C": ("setStyle()", "CutAnalysis"),
    "analyses/kpkpxim/selection/CutAnalysisRF.C": ("setStyle()", "CutAnalysisRF"),
    "analyses/kpkpxim/systematics/mc_weight_variations/WeightMC.C": ("setStyle()", "WeightMC"),
    "analyses/kpkpxim/systematics/track_efficiency/WeightMC.C": ("setStyle()", "WeightMC"),
    "analyses/kpkpxim/xsection/PlotXSecComponents.C": ("style_format()", "PlotXSecComponents"),
    **{f"analyses/kpkpxim/selection/cut_studies/rapidity_cuts/{name}": ("style_format()", "RapidityCuts")
       for name in ("PlotKPlusHighComparison.C", "PlotKPlusHighRapidity.C", "PlotKPlusLowComparison.C",
                    "PlotKPlusLowP.C", "PlotKPlusLowPComparison.C", "PlotKPlusLowRapidity.C",
                    "PlotKPlusMomSepComparison.C", "PlotTDistComparison.C")},
    "analyses/kpkpxim/selection/cut_studies/kaon_selection/make_plot.C": ("style_format()", "KaonSelectionMakePlot"),
    "analyses/kpkpxim/selection/cut_studies/accidentals/get_data_hists.C": ("style_format()", "AccidentalsGetDataHists"),
    "analyses/kpkpxim/xsection/PlotTotXsecWithClas.C": ("SetStyle()", "PlotTotXsecWithClas"),
    "analyses/kpkpxim/systematics/GetRunPeriodPctSig.C": ("SetStyle()", "GetRunPeriodPctSig"),
}
COPIES = sorted({copy for _, copy in SITES.values()})
# Copies whose texts differ but whose final gStyle state is the same.
SAME_STATE = {frozenset({"OneUMLFit", "GaussianFit"}),
              frozenset({"GetKinematicsDataMC", "GetKinematicsDataMC_RF"})}


def _env(tmp_path):
    env = dict(os.environ, GXANA_ROOT=str(ROOT))
    for var in ("GXANA_DATA", "GXANA_OUTPUT", "GXANA_SCRATCH", "GXANA_EXTERNALS", "GXANA_ANALYSIS_DATA"):
        env[var] = str(tmp_path / var.lower())
    return env


def _snapshot(macro, calls, outdir, env):
    proc = subprocess.run(["root", "-l", "-b", "-q", "rootlogon.C",
                           f'tests/macros/style_snapshot.C("{macro}","{"|".join(calls)}","{outdir}")'],
                          cwd=ROOT, env=env, capture_output=True, text=True, timeout=300)
    out = proc.stdout + proc.stderr
    assert proc.returncode == 0 and "error:" not in out and "STYLE SNAPSHOT FAILED" not in out, out[-4000:]
    return {(name, start): (outdir / f"{name}__{start}.json").read_text()
            for name in (c.split("=", 1)[0] for c in calls) for start in STARTS}


def _first_difference(want, got):
    for a, b in zip(want.splitlines(), got.splitlines()):
        if a != b:
            return f"legacy: {a.strip()} | site: {b.strip()}"
    return "different length"


@pytest.fixture(scope="module")
def dump_dir(tmp_path_factory):
    keep = os.environ.get("GXANA_STYLE_DUMP_DIR")
    path = Path(keep) if keep else tmp_path_factory.mktemp("styles")
    path.mkdir(parents=True, exist_ok=True)
    return path


@pytest.fixture(scope="module")
def env(tmp_path_factory):
    return _env(tmp_path_factory.mktemp("env"))


@pytest.fixture(scope="module")
def legacy(dump_dir, env):
    dumps = _snapshot("tests/macros/legacy_styles.C", [f"{c}=Legacy_{c}()" for c in COPIES],
                      dump_dir / "legacy", env)
    return {(copy, start): text for (copy, start), text in dumps.items()}


@pytest.mark.parametrize("site", sorted(SITES))
def test_site_style_matches_legacy(site, legacy, dump_dir, env):
    call, copy = SITES[site]
    got = _snapshot(site, [f"site={call}"], dump_dir / "sites" / site.replace("/", "__"), env)
    for start in STARTS:
        want = legacy[(copy, start)]
        assert got[("site", start)] == want, f"from {start}: {_first_difference(want, got[('site', start)])}"


def test_legacy_copies_are_distinct_states(legacy):
    """The harness tells the legacy states apart, and the dirty start survives in every one."""
    for a, b in combinations(COPIES, 2):
        same = legacy[(a, "default")] == legacy[(b, "default")]
        assert same == (frozenset({a, b}) in SAME_STATE), (a, b)
    for copy in COPIES:
        assert legacy[(copy, "dirty")] != legacy[(copy, "default")], copy


def _analysis_sources():
    """The files the load test covers (same exclusions), plus headers."""
    skip_file = Path(__file__).with_name("skip.txt")
    skips = {line.split("#")[0].strip() for line in skip_file.read_text().splitlines()} if skip_file.exists() else set()
    for ext in ("*.C", "*.cpp", "*.cxx", "*.h"):
        for p in sorted((ROOT / "analyses").rglob(ext)):
            rel = p.relative_to(ROOT)
            if "selectors" not in rel.parts and rel.as_posix() not in skips:
                yield rel.as_posix(), p


def test_no_macro_carries_its_own_style_block():
    """Style functions apply a gxana preset, which calls gROOT->ForceStyle(); no macro calls it itself."""
    offenders = [f"{rel}:{n}" for rel, p in _analysis_sources()
                 for n, line in enumerate(p.read_text(errors="replace").splitlines(), 1) if "ForceStyle" in line]
    assert not offenders, (
        "gROOT->ForceStyle() in an analysis macro means a local gStyle block: apply a gxana style preset "
        "instead (gxana::ApplyStyle(gxana::FitStyle()) or another preset in gxana/common/Style.h, with "
        "per-macro overrides) and register the style function in SITES in tests/macros/test_macro_styles.py "
        "with a verbatim copy of the original body in legacy_styles.C. Found: " + ", ".join(offenders))


def test_harness_sees_one_setter(legacy, dump_dir, env):
    """Negative control: a site call plus one extra setter, or no call at all, must not match."""
    site = "analyses/kpkpxim/signal_extraction/lineshape/OneUMLFit.C"
    got = _snapshot(site, ["extra=setStyle();gStyle->SetLabelOffset(0.0291)", "none=(void)0"],
                    dump_dir / "negative_control", env)
    for start in STARTS:
        assert got[("extra", start)] != legacy[("OneUMLFit", start)]
        assert got[("none", start)] != legacy[("OneUMLFit", start)]
