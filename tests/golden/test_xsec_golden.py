"""Golden: gxana_xsec_tables reproduces the legacy cross-section tables.

Legacy runs, the three periods in PERIOD_TREES order, several minutes each:
- hybrid_combo, best_combo, acc_weight: the JohnsonMCShape combo-selection
  study (MakeXSecFiles.C, Mar 2025; fit type JohnsonMCShape) once per
  combo-selection weight: per bin a Johnson fit to MC fixes the signal shape
  of a Johnson + 2nd-order Chebychev data fit; every bin restarts from the
  same parameters.
- johnson, weight hybrid_combo, the dissertation fit (MakeXSecFitVariations.C,
  Jul 2025): Johnson + 2nd-order Chebychev sharing one parameter map across
  bins and periods.

Tolerance: deterministic columns (qval_yield, qval_yield_err, mc_yield,
mc_err, thrown_yield, thrown_err, accept, accept_err/accep_err, flux,
flux_err, and the bin-center/width columns) at GXANA_GOLDEN_RTOL (default
1e-5, i.e. the printed 6 digits) -- these do not depend on the fit and must
match the thesis tables on any ROOT version once the flux windows use
LegacyFindBin. The fit-yield-dependent columns (data_yield, yield_err, and
the cross-section value/error columns sigma/dsigmadt/Yerr) are compared at
GXANA_GOLDEN_FIT_RTOL (default: FIT_RTOL below, the maximum seen on ROOT 6.40
rounded up): RooFit's default minimizer changed from
TMinuit (ROOT 6.24, thesis) to Minuit2 (ROOT 6.32+), and even pinning
Minimizer("Minuit","migrad") leaves ROOT-version numerics differences of a
few percent in some fits. On ROOT 6.40 about 3/4 of the thesis-fit bins still
agree to 1e-5; the rest differ by up to 7% (hybrid_combo) and, in the
MINUIT yield error, 17% (acc_weight) and 28% (best_combo). The legacy
MakeXSecFiles.C macro run on ROOT 6.40 gives the same tables as the port to
1e-9, so these are ROOT-version differences, not porting errors. The authoritative
comparison runs in the ROOT 6.24.04 analysis container, where
GXANA_GOLDEN_FIT_RTOL should be set to 1e-5; on newer ROOT record the
reported max deviation.

Thesis fit: in two 2017-01 -t bins (the last bin of the two lowest energy
bins) the data fails MakeXSecFiles.C's > 25-entry gate. Legacy printed
uninitialized memory in those rows' mc/thrown/acceptance columns; the port
prints nan there, so the test masks those reference cells to nan.

The commands come straight from gxana.stages.xsection.plan_xsection (the
"tables" step): we keep one fit's first label and substitute the JOB paths
for the golden binned_trees/flux layout under $GXANA_ANALYSIS_DATA/kpkpxim
(the stage itself points JOBs at $GXANA_OUTPUT/.../xsection/binned_trees,
which golden data doesn't populate).
"""
import os
import shutil
import subprocess

import pytest
from golden_data import FLUX_FILES, PERIOD_TREES

from gxana import config
from gxana.paths import repo_root
from gxana.stages import xsection as xs
from gxana_systematics import config as sysconfig
from gxana_xsection.compare import compare_dirs

pytestmark = pytest.mark.golden

# Default fit-column tolerance per label on newer ROOT (see module docstring).
FIT_RTOL = {"hybrid_combo": "8e-2", "best_combo": "3e-1", "acc_weight": "2e-1", "johnson": "4e-2"}

# diffout columns legacy left uninitialized when the data gate fails
# (1-based: mc_yield .. accep_err).
GATED_COLUMNS = range(7, 13)


def _stage_command(model, label):
    """--fit/--param and the --weight/--cheby in effect for label in the tables command for model.

    The Johnson group lives in the xsection stage's plan; the JohnsonMCShape
    study (hybrid_combo, best_combo, acc_weight) now lives in the systematics
    variant pool, so both are searched, built with the same tables engine.
    """
    # environ only needs to be well-formed enough for plan_xsection to
    # resolve; its JOB paths and --out are replaced by the caller.
    environ = {"GXANA_ROOT": str(repo_root()), "GXANA_DATA": "/unused", "GXANA_OUTPUT": "/unused"}
    cfg = config.load_channel("kpkpxim")
    commands = xs.plan_xsection(cfg, ["tables"], environ=environ)
    pool = sysconfig.fit_groups(sysconfig.block(cfg))
    commands += xs.tables_commands(cfg, pool, "/unused/out", None, environ)
    argv = next(c.argv for c in commands if c.argv[c.argv.index("--fit") + 1] == model)
    params = [argv[i + 1] for i, a in enumerate(argv) if a == "--param"]
    # --weight/--cheby/--label are order-sensitive: take those in effect at label.
    at = next((i for i in range(1, len(argv)) if argv[i - 1] == "--label" and argv[i] == label), None)
    assert at is not None, f"no label {label!r} for {model}"
    last = lambda option: argv[max(i for i in range(at) if argv[i] == option) + 1]
    return {
        "fit": model,
        "params": [arg for p in params for arg in ("--param", p)],
        "weight": last("--weight"),
        "cheby": last("--cheby"),
        "label": label,
    }


def _run_tables(need, build_bin, tmp_path, cmd):
    jobs = []
    for tree in PERIOD_TREES:
        data, mc, thrown, flux = need(
            f"binned_trees/binned_flatTree_{tree}_nominal_kphighrap.root",
            f"binned_trees/binned_flatTree_{tree}_gen_amp_V2_ac_YstarRest_nominal_kphighrap.root",
            f"binned_trees/binned_thrown_flatTree_{tree}_gen_amp_V2_ac_YstarRest.root",
            FLUX_FILES[tree],
        )
        jobs.append(f"flatTree_{tree}:{data}:{mc}:{thrown}:{flux}")
    # The tables app writes each label into <out>/<label>/, as the weight and
    # components steps expect; the channel flags are the stage's (analyses/kpkpxim/config).
    subprocess.run([str(build_bin / "gxana_xsec_tables"), "--fit", cmd["fit"], *cmd["params"],
                    "--label", cmd["label"], "--weight", cmd["weight"], "--cheby", cmd["cheby"],
                    "--out", str(tmp_path), *jobs, *xs.tables_physics_args(config.load_channel("kpkpxim"))],
                   check=True)
    return tmp_path / cmd["label"]


def _compare(out, ref, default_fit_rtol):
    rtol = float(os.environ.get("GXANA_GOLDEN_RTOL", "1e-5"))
    fit_rtol = float(os.environ.get("GXANA_GOLDEN_FIT_RTOL", default_fit_rtol))
    # Fit-yield-dependent columns: data yield/error and the cross-section
    # value/error columns (totxsec_*: sigma, Yerr; diffxsec_*: dsigmadt, Yerr).
    # Every other numeric column (qval/mc/thrown/acceptance/flux, bin centers
    # and widths) is deterministic given the binned trees and flux histogram,
    # so it stays at the strict rtol above.
    column_rtol = {
        "data_yield": fit_rtol, "yield_err": fit_rtol,
        "sigma": fit_rtol, "dsigmadt": fit_rtol, "Yerr": fit_rtol,
    }
    report = compare_dirs(out, ref, rtol=rtol, column_rtol=column_rtol)
    print(report.summary())
    assert report.ok, report.summary()
    assert len(report.results) == 54


def _mask_gated_rows(ref, masked):
    """Copy ref to masked with the uninitialized cells of gated diffout rows set to nan."""
    shutil.copytree(ref, masked)
    gated = 0
    for path in masked.glob("diffout_*.txt"):
        lines = path.read_text().splitlines()
        for i, line in enumerate(lines[1:], start=1):
            cols = line.split()
            if float(cols[2]) == 0 and float(cols[3]) == 0:  # data_yield, yield_err
                for c in GATED_COLUMNS:
                    cols[c - 1] = "nan"
                lines[i] = "  ".join(cols)
                gated += 1
        path.write_text("\n".join(lines) + "\n")
    return gated


@pytest.mark.parametrize("combo", ["hybrid_combo", "best_combo", "acc_weight"])
def test_mcshape_study_tables_match_legacy(need, build_bin, tmp_path, combo):
    """data/<combo>: the JohnsonMCShape study fit per combo-selection weight."""
    ref = need(f"reference/xsection/{combo}")[0]
    cmd = _stage_command("JohnsonMCShape", combo)
    assert (cmd["weight"], cmd["cheby"]) == (combo, "2")
    out = _run_tables(need, build_bin, tmp_path / "new", cmd)
    masked = tmp_path / "ref"
    assert _mask_gated_rows(ref, masked) == 2
    _compare(out, masked, FIT_RTOL[combo])


def test_johnson_tables_match_legacy(need, build_bin, tmp_path):
    """data/johnson: the dissertation fit (Johnson + 2nd-order Chebychev, weight hybrid_combo)."""
    ref = need("reference/xsection/johnson")[0]
    cmd = _stage_command("Johnson", "johnson")
    assert (cmd["weight"], cmd["cheby"]) == ("hybrid_combo", "2")
    _compare(_run_tables(need, build_bin, tmp_path, cmd), ref, FIT_RTOL["johnson"])
