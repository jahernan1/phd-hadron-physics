"""kpkpkmlamb channel config: legacy tree names, selector output names, no MC."""
import os
import re
import subprocess
from pathlib import Path

import pytest

from gxana import config
from gxana.config import ConfigError
from gxana.stages.select import plan_select

ROOT = Path(__file__).resolve().parents[2]
CFG = config.load_channel("kpkpkmlamb")
ENV = {"GXANA_DATA": "/d", "GXANA_OUTPUT": "/o", "GXANA_SCRATCH": "/s"}
LEGACY = {  # stems of the legacy flat trees (_workdir/kpkpkmlamb/flatTreePrep.C) and tree dirs (runMultiDSelector.sh)
    "2017-01": "kpkpkmlamb__B4_M18_2017-01_ana55",
    "2018-01": "kpkpkmlamb__B4_M18_2018-01_ana22",
    "2018-08": "kpkpkmlamb__B4_M18_2018-08_ana19",
}


@pytest.mark.parametrize("period", sorted(LEGACY))
def test_stem_matches_legacy_names(period):
    assert config.tree_stem(CFG, period, "data") == LEGACY[period]
    assert config.tree_dir(CFG, period, "data") == f"Trees/kpkpkmlamb/tree_{LEGACY[period]}/trees/"


def test_plan_select_paths():
    job = plan_select(CFG, "2018-08", "data", environ=ENV)
    assert job.tree_dir == Path(f"/d/Trees/kpkpkmlamb/tree_{LEGACY['2018-08']}/trees").resolve()
    assert job.save_name == LEGACY["2018-08"]
    assert job.hist_dir == Path("/o/kpkpkmlamb/selector_hists").resolve()
    assert job.selector.name == "DSelector_kpkpkmlamb.C" and job.selector.is_file()


def test_selector_writes_the_configured_files():
    text = (ROOT / CFG["selector_dir"] / CFG["default_selector"]).read_text(errors="ignore")
    out = re.search(r'dOutputFileName\s*=\s*"([^"]+)"', text).group(1)
    flat = re.search(r'dFlatTreeFileName\s*=\s*"([^"]+)"', text).group(1)
    assert out == CFG["output_basename"]
    assert flat == "flatTree_" + CFG["output_basename"]  # planned_moves looks for flatTree_<output_basename>


def test_thrown_is_a_config_error():
    with pytest.raises(ConfigError, match="no thrown trees"):
        plan_select(CFG, "2017-01", "data", thrown=True, environ=ENV)


def test_cli_dry_run(tmp_path):
    env = dict(os.environ, GXANA_ROOT=str(ROOT), GXANA_DATA=str(tmp_path / "d"),
               GXANA_OUTPUT=str(tmp_path / "o"), GXANA_SCRATCH=str(tmp_path / "s"))
    p = subprocess.run(["uv", "run", "gxana", "run", "select", "--channel", "kpkpkmlamb", "--period", "2017-01",
                        "--sample", "data", "--dry-run"], cwd=ROOT, env=env, capture_output=True, text=True)
    assert p.returncode == 0, p.stderr
    assert f"save name: {LEGACY['2017-01']}" in p.stdout
    assert "DSelector_kpkpkmlamb.C" in p.stdout


def test_cli_thrown_names_the_problem(tmp_path):
    env = dict(os.environ, GXANA_ROOT=str(ROOT), GXANA_DATA=str(tmp_path / "d"),
               GXANA_OUTPUT=str(tmp_path / "o"), GXANA_SCRATCH=str(tmp_path / "s"))
    p = subprocess.run(["uv", "run", "gxana", "run", "select", "--channel", "kpkpkmlamb", "--period", "2017-01",
                        "--sample", "data", "--thrown", "--dry-run"], cwd=ROOT, env=env, capture_output=True, text=True)
    assert p.returncode != 0
    assert "no thrown trees" in p.stdout + p.stderr
    assert "Traceback" not in p.stderr
