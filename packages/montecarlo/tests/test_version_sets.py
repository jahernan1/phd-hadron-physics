"""Sim version-set templates agree with external.lock."""
import re
from pathlib import Path

import pytest
import yaml

ROOT = Path(__file__).resolve().parents[3]
LOCK = yaml.safe_load((ROOT / "packages/montecarlo/external.lock").read_text())["externals"]
TEMPLATES = sorted((ROOT / "env/version_sets").glob("*.xml.in"))


def pkg(text, name, attr):
    m = re.search(rf'<package name="{name}"[^>]*\b{attr}="([^"]+)"', text)
    assert m, f"{name} {attr}"
    return m.group(1)


def test_templates_present():
    assert [t.name for t in TEMPLATES] == [
        "recon-2017_01-ver03_40.xml.in", "recon-2018_01-ver02_32.xml.in",
        "recon-2018_08-ver02_31.xml.in", "recon-2019_11-ver01_13.xml.in"]


@pytest.mark.parametrize("path", TEMPLATES, ids=lambda p: p.name)
def test_template_matches_lock(path):
    text = path.read_text()
    name = path.name[:-len(".xml.in")]
    assert pkg(text, "halld_sim", "home") == "${GXANA_EXTERNALS}/halld_sim-" + name
    assert pkg(text, "gluex_MCwrapper", "home") == "${GXANA_EXTERNALS}/gluex_MCwrapper"
    assert pkg(text, "hdgeant4", "version") == LOCK["HDGeant4"]["ref"]
    assert pkg(text, "amptools", "version") == LOCK["AmpTools"]["ref"].lstrip("v")
    assert pkg(text, "root", "version") == "6.08.06"
    assert set(re.findall(r"\$\{(\w+)\}", text)) == {"GXANA_EXTERNALS"}
