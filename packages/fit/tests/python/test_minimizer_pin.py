"""Fits run with the ROOT 6.24 defaults (TMinuit/Migrad; on ROOT >= 6.32 the legacy RooFit
evaluation backend): rootlogon.C and every app that fits call gxana::fit::UseThesisMinimizer()."""
import os
import shutil
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[4]
FITTING_APPS = ("packages/xsection/apps/gxana_xsec_tables.cxx",
                "packages/barlow/apps/gxana_barlow_trees.cxx",
                "packages/studies/apps/gxana_study_cutscan.cxx")
PROBE = r"""
#include <Math/MinimizerOptions.h>
#include <RVersion.h>
#include <RooGlobalFunc.h>
#include <TFitResult.h>
#include <TH1D.h>
#include <cmath>
#include <cstdio>
void pin_probe()
{
    std::printf("DEFAULT %s %s\n", ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(),
                ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 32, 0)
    std::printf("LEGACY_BACKEND %d\n", RooFit::EvalBackend::defaultValue() == RooFit::EvalBackend::Value::Legacy);
#endif
    TH1D h("h", "", 30, -3, 3);
    for (int i = 0; i < 30; ++i) h.SetBinContent(i + 1, 100 * std::exp(-0.5 * std::pow(h.GetBinCenter(i + 1), 2)) + 1);
    TFitResultPtr r = h.Fit("gaus", "SQ0");
    std::printf("TH1FIT %s\n", r->MinimizerType().c_str());
}
"""


def test_fitting_apps_pin():
    for app in FITTING_APPS:
        assert "gxana::fit::UseThesisMinimizer();" in (ROOT / app).read_text(), app


@pytest.mark.skipif(shutil.which("root") is None, reason="ROOT not on PATH")
def test_rootlogon_pins(tmp_path):
    probe = tmp_path / "pin_probe.C"
    probe.write_text(PROBE)
    env = dict(os.environ, GXANA_ROOT=str(ROOT), ROOT_MAX_THREADS="1")
    proc = subprocess.run(["root", "-l", "-b", "-q", str(ROOT / "rootlogon.C"), str(probe)],
                          cwd=tmp_path, env=env, capture_output=True, text=True, timeout=300)
    assert proc.returncode == 0, proc.stdout + proc.stderr
    lines = proc.stdout.splitlines()
    assert "DEFAULT Minuit Migrad" in lines, proc.stdout
    assert all(line == "LEGACY_BACKEND 1" for line in lines if line.startswith("LEGACY_BACKEND")), proc.stdout
    assert "TH1FIT Minuit / Migrad" in lines, proc.stdout
