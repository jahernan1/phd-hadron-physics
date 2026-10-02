"""`gxana run measurements --channel kpkpxim` on the preserved thesis trees: the same FITRESULT lines as
the original macros (tests/golden/data/measurements_reference.txt, recorded single-threaded from the
original GetXimProperties.C and PlotGlueXSpin.C), the three ROOT files in the output directory and the
thirteen PDFs in prod_plots, which the stage creates. The macros run with n_threads 0 (implicit MT
off), as for the reference."""
import copy
import os
import shutil
import subprocess

import pytest
from golden_data import PERIOD_TREES, assert_fitresults_equal, measurements_farm, parse_fitresults

from gxana import config
from gxana.paths import repo_root
from gxana.stages import measurements

pytestmark = [pytest.mark.golden, pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")]

REFERENCE = repo_root() / "tests" / "golden" / "data" / "measurements_reference.txt"
PDFS = sorted(["accepted_pimcostheta_gluex_phase1.pdf"] + [f"{p}{s}.pdf" for s in PERIOD_TREES for p in (
    "XimIM_mc_", "XimIM_dataCorr_", "accepted_ximlifetime_", "accepted_pimcostheta_")])


def test_stage_reproduces_the_original_measurements(tmp_path, need):
    data, out = measurements_farm(tmp_path, need)
    (out / "kpkpxim" / "prod_plots").rmdir()  # the stage must create it
    env = dict(os.environ, GXANA_ROOT=str(repo_root()), GXANA_DATA=str(data), GXANA_OUTPUT=str(out),
               ROOT_MAX_THREADS="1")
    cfg = copy.deepcopy(config.load_channel("kpkpxim"))
    for item in cfg["measurements"]["items"].values():
        for call in item.values():
            call["args"] = [0]
    texts = []

    def runner(argv, **kwargs):
        proc = subprocess.run(argv, env=env, capture_output=True, text=True, timeout=1800, **kwargs)
        texts.append(proc.stdout + proc.stderr)
        return proc

    rc = measurements.run_measurements(cfg, list(measurements.STEPS), runner=runner, environ=env)
    text = "".join(texts)
    assert rc == 0, text[-3000:]
    got, ref = parse_fitresults(text), parse_fitresults(REFERENCE.read_text())
    assert set(got) == set(ref), f"missing: {sorted(set(ref) - set(got))}\nextra: {sorted(set(got) - set(ref))}"
    assert_fitresults_equal(got, ref)
    assert sorted(p.name for p in (out / "kpkpxim" / "measurements").iterdir()) == [
        "xim_lifetime.root", "xim_mass.root", "xim_spin.root"]
    assert sorted(p.name for p in (out / "kpkpxim" / "prod_plots").iterdir()) == PDFS
