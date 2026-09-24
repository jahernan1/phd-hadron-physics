"""Each sample's selector exists and writes the file run select expects (spec §8)."""
import re

import pytest

from gxana import config
from gxana.paths import repo_root
from gxana.stages.select import plan_select

CFG = config.load_channel("kpkpxim")
SEL_DIR = repo_root() / CFG["selector_dir"]
ENV = {"GXANA_DATA": "/d", "GXANA_OUTPUT": "/o", "GXANA_SCRATCH": "/s"}


def _output_name(selector):
    text = (SEL_DIR / selector).read_text(errors="ignore")
    m = re.search(r'dOutputFileName\s*=\s*"([^"]+)"', text)
    assert m, f"{selector}: no dOutputFileName"
    return m.group(1)


def _periods(sample):
    launch = CFG["samples"][sample].get("launch")
    return sorted(launch) if isinstance(launch, dict) else sorted(CFG["periods"])


@pytest.mark.parametrize("sample", sorted(CFG["samples"]))
def test_selector_output_matches_config(sample):
    job = plan_select(CFG, _periods(sample)[0], sample, environ=ENV)
    assert job.selector.is_file()
    assert _output_name(job.selector.name) == job.output_basename


@pytest.mark.parametrize("sample", sorted(s for s, v in CFG["samples"].items() if v.get("mc")))
def test_thrown_selector_output(sample):
    """planned_moves looks for thrown_<output_basename>; the thrown selector must write it."""
    job = plan_select(CFG, _periods(sample)[0], sample, thrown=True, environ=ENV)
    assert job.selector.is_file()
    assert _output_name(job.selector.name) == "thrown_" + job.output_basename
