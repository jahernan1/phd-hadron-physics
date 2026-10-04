"""Channel README commands only cd into GXANA_OUTPUT and reference existing repo files."""
import re
from typing import cast
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
CHANNELS = sorted(p.parent.name for p in (ROOT / "analyses").glob("*/config"))
READMES = {c: ROOT / "analyses" / c / "README.md" for c in CHANNELS}


@pytest.mark.parametrize("channel", CHANNELS)
def test_readme_exists(channel):
    assert READMES[channel].is_file()


@pytest.mark.parametrize("channel", CHANNELS)
def test_cd_targets_are_output_dirs(channel):
    cds = re.findall(r"\bcd\s+(\S+)", READMES[channel].read_text())
    assert cds, "README has no cd commands"
    assert all(t.startswith("$GXANA_OUTPUT/") for t in cds), cds


@pytest.mark.parametrize("channel", CHANNELS)
def test_referenced_macros_exist(channel):
    text = READMES[channel].read_text()
    for rel in re.findall(r"\$GXANA_ROOT/(analyses/\S+?\.C)", text) + re.findall(r"'(analyses/\S+?\.C)\(", text) \
            + re.findall(r"\s(analyses/\S+?\.C)\b", text):
        assert (ROOT / rel).is_file(), rel


def test_kpkpkmlamb_hadd_target_is_the_fit_input():
    text = READMES["kpkpkmlamb"].read_text()
    target = cast(re.Match, re.search(r"hadd\s+(?:-f\s+)?(\S+)", text)).group(1)
    assert target == "$GXANA_DATA/kpkpkmlamb/flatTree_kpkpkmlamb_GlueX-I.root"
    macro = (ROOT / "analyses/kpkpkmlamb/measurements/FitXimStar.C").read_text()
    assert '"kpkpkmlamb/flatTree_kpkpkmlamb_GlueX-I.root"' in macro
