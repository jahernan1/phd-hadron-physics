"""gxana::fit adoption sites without preserved inputs: the frozen legacy fit function
(packages/fit/tests/legacy_sites/, made by freeze.py) and the adopted macro run on the same
seeded synthetic input, each in its own ROOT process with GXANA_FIT_TRACE=1 and one thread.
Required: FITRESULT lines identical; FACTORY statements identical once blanks are removed
(RooFactoryWSTool drops blanks before parsing; test_fit.cxx checks the workspaces are equal);
every other stdout line identical except the "Processing" line and "took <t>" timings.
Each run keeps run.log, run.err and its PDFs in its pytest tmp directory (see --basetemp)."""
import os
import re
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[4]
LEGACY = ROOT / "packages/fit/tests/legacy_sites"
pytestmark = pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")

FIT_STYLE = "gxana::ApplyStyle(gxana::FitStyle());"
CUT_STYLE = "gxana::ApplyStyle(gxana::CutStudyStyle());"
XI_TITLE = r'\" ;M(#Lambda#pi^{-}) (GeV); Events / 3.67 MeV\"'


def xi_hist(seed):
    """Xi(1320)-like peak + Sigma(1385)-like bump on a flat background, weighted fills."""
    return (f'SynHist({seed}, \\"XiMinus_PhaseI\\", 60, 1.25, 1.47, '
            "{{1.3217, 0.0045, 4000, false}, {1.385, 0.036, 400, false}}, 2500, true)")


def kstar_hist(seed):
    """K*(892)-like Breit-Wigner on a flat background."""
    return f'SynHist({seed}, \\"kstar\\", 60, 0.6, 1.8, {{{{0.8955, 0.047, 3000, true}}}}, 3000, true)'


def ystar_hist(seed):
    """Three broad Breit-Wigner peaks on a flat background."""
    return (f'SynHist({seed}, \\"ystar\\", 80, 1.8, 4.0, '
            "{{2.1, 0.15, 800, true}, {2.3, 0.4, 1500, true}, {2.8, 0.6, 1000, true}}, 500, true)")


# site -> (frozen copy in legacy_sites/, adopted macro, ROOT call for a seed). Tasks add entries.
SITES = {
    "MakeXim1320_IM": ("MakeXim1320_IM.C", "analyses/kpkpxim/measurements/mass/MakeXim1320_IM.C",
                       lambda s: f'{FIT_STYLE} RooFitHist({xi_hist(s)}, {XI_TITLE}, \\"_syn{s}\\")'),
    "MakeXim1320_IM_Res": ("MakeXim1320_IM_Res.C", "analyses/kpkpxim/measurements/mass/MakeXim1320_IM_Res.C",
                           lambda s: f'{FIT_STYLE} RooFitHist({xi_hist(s)}, {XI_TITLE}, \\"_syn{s}\\")'),
    "GetQvalueSum": ("GetQvalueSum.C", "analyses/kpkpxim/signal_extraction/qfactors/scripts/GetQvalueSum.C",
                     lambda s: f'{CUT_STYLE} double y = 0, ye = 0; '
                               f'rooFitHist({xi_hist(s)}, (char*)\\"syn\\", (char*)\\"ws_syn\\", &y, &ye);'),
    "CutAnalysis": ("CutAnalysis.C", "analyses/kpkpxim/selection/CutAnalysis.C",
                    lambda s: f'{FIT_STYLE} double a = 0, b = 0, c = 0, d = 0; '
                              f'rooFitHist({xi_hist(s)}, \\"syn\\", &a, &b, &c, &d);'),
    "CutAnalysisRF": ("CutAnalysisRF.C", "analyses/kpkpxim/selection/CutAnalysisRF.C",
                      lambda s: f'{FIT_STYLE} double a = 0, b = 0, c = 0, d = 0; '
                                f'rooFitHist({xi_hist(s)}, \\"syn\\", &a, &b, &c, &d);'),
    "KstarFit": ("KstarFit.C", "analyses/kpkpxim/backgrounds/KstarFit.C",
                 lambda s: f'{FIT_STYLE} rooFitHist({kstar_hist(s)}, \\"syn\\");'),
    "YstarBWFitsData": ("YstarBWFitsData.C", "analyses/kpkpxim/backgrounds/YstarBWFitsData.C",
                        lambda s: f'{FIT_STYLE} rooFitHist({ystar_hist(s)}, \\"syn\\");'),
}
SEEDS = (1, 2, 3)


def _run(tmp_path, tag, macro, call):
    out = tmp_path / tag
    (out / "output/kpkpxim/prod_plots").mkdir(parents=True)
    env = dict(os.environ, GXANA_ROOT=str(ROOT), GXANA_OUTPUT=str(out / "output"), GXANA_DATA=str(out / "data"),
               GXANA_FIT_TRACE="1", ROOT_MAX_THREADS="1")
    proc = subprocess.run(["root", "-l", "-b", "-q", str(ROOT / "rootlogon.C"),
                           "-e", f'gInterpreter->AddIncludePath("{LEGACY}");',
                           f'{LEGACY / "run_site.C"}("{macro}","{call}","{out / "pdf"}")'],
                          cwd=out, env=env, capture_output=True, text=True, timeout=600)
    (out / "run.log").write_text(proc.stdout)
    (out / "run.err").write_text(proc.stderr)
    assert proc.returncode == 0 and "RUN_SITE saved" in proc.stdout, (proc.stdout + proc.stderr)[-4000:]
    return proc.stdout.splitlines()


def _split(lines):
    fit = [x for x in lines if x.startswith("FITRESULT")]
    factory = [x.replace(" ", "") for x in lines if x.startswith("FACTORY")]
    rest = [re.sub(r"took [0-9.]+ ?(?:s|ms|us|μs)\b", "took T", x) for x in lines
            if not x.startswith(("FITRESULT", "FACTORY", "Processing "))]
    return fit, factory, rest


@pytest.mark.parametrize("seed", SEEDS)
@pytest.mark.parametrize("site", sorted(SITES) or ["<none>"])
def test_site_matches_legacy(site, seed, tmp_path):
    if site == "<none>":
        pytest.skip("no legacy site registered yet")
    frozen, macro, call = SITES[site]
    old = _split(_run(tmp_path, "old", LEGACY / frozen, call(seed)))
    new = _split(_run(tmp_path, "new", ROOT / macro, call(seed)))
    assert old[0], "no FITRESULT line: the frozen copy did not fit"
    assert new[0] == old[0]
    assert new[1] == old[1]
    assert new[2] == old[2]
