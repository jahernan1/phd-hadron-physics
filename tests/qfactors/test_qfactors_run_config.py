"""kpkpxim Q-factor run config (config/qfactors.yaml, signal_extraction/qfactors, fit models in packages/qfactors)."""
import hashlib
import re
from pathlib import Path

import pytest

from gxana import config

ROOT = Path(__file__).resolve().parents[2]
CFG = config.load_channel("kpkpxim")
Q = CFG["qfactors"]
ENGINE = ROOT / "packages/qfactors"
MODELS = ("configPDFs.h", "configPDFs_Johnson.h", "configPDFs_JohnsonGaus.h", "configPDFs_Gaussian.h")
THESIS_MODEL_SHA256 = "6ddbe7d70cd3d664763d5ed2b293c625d82c96b649190c595fac7dbb0548aa02"
SETTINGS = {
    "fitWeights": "hybrid_combo", "sigWeights": "hybrid_combo", "altWeights": "hybrid_combo",
    "varStringBase": "beam_E;kp_highp_CosTheta;kp_highp_Phi;kplow_costheta_hf;kplow_phi_hf;pim1_costheta_hf;decaylamb_M_meas",
    "discrimVars": "decayxim_M", "extraVars": "decayxim_M", "neighborReqs": "",
    "nProcess": 16, "kDim": 200, "nentries": -1, "numberEventsToSavePerProcess": -1,
    "standardizationType": "range", "redistributeBkgSigFits": 0, "nRndRepSubset": 0, "doKRandomNeighbors": 0,
    "nBS": 0, "runTag": "", "seedShift": 1341, "saveBShistsAlso": 0, "alwaysSaveTheseEvents": "",
    "saveBranchOfNeighbors": 1, "saveMemUsage": 1, "saveEventLevelProcessSpeed": 1, "emailWhenFinished": "",
    "runBatch": 0, "runAllPhaseCombos": 0, "extraLibs": [],
}


def test_keys_and_thesis_settings():
    assert set(Q) == {"engine_dir", "model", "sample", "variant", "input", "tree", "work_dir",
                      "output_dir", "plots_dir", "settings", "extra_settings", "diagnostic_vars"}
    assert Q["settings"] == SETTINGS
    assert Q["extra_settings"] == {"verbose_outputDistCalc": "false"}
    assert Q["model"] == "configPDFs.h"
    assert Q["diagnostic_vars"][:2] == ["ystar_M", "decayxim_M"] and len(Q["diagnostic_vars"]) == 22


@pytest.mark.skipif(not (ENGINE / "main.C").is_file(), reason="packages/qfactors not checked out")
def test_every_model_pins_minuit_and_returns_chisq():
    for model in MODELS:
        text = (ENGINE / model).read_text()
        assert len(re.findall(r'RooFit::Minimizer\("Minuit","migrad"\), // ROOT 6\.24 default; newer ROOT defaults to Minuit2',
                              text)) == 1, model
        assert re.search(r"void drawFitPlots\([^)]*double best_qvalue, float\* chisqndf, int iBS", text), model
        assert re.search(r"draw1DPlots\([^;]*NLL,\s*chisqndf,", text, re.S), model


@pytest.mark.skipif(not (ENGINE / "main.C").is_file(), reason="packages/qfactors not checked out")
def test_thesis_model_is_unchanged():
    # configPDFs.h reproduces the thesis q-factors (tests/golden/test_qfactors_golden.py); keep it byte-identical.
    assert hashlib.sha256((ENGINE / "configPDFs.h").read_bytes()).hexdigest() == THESIS_MODEL_SHA256


def test_scripts_copied_without_site_paths():
    scripts = sorted(p.name for p in (ROOT / "analyses/kpkpxim/signal_extraction/qfactors/scripts").glob("*.C"))
    assert scripts == ["GetQvalueSum.C", "MakeParamTrees.C", "PlotTOF.C", "QValWeightExample.C"]
