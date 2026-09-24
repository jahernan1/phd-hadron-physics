"""README pipeline commands only cd into GXANA_OUTPUT and reference existing repo files."""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
README = (ROOT / "analyses" / "kpkpxim" / "README.md").read_text()


def test_cd_targets_are_output_dirs():
    cds = re.findall(r"\bcd\s+(\S+)", README)
    assert cds, "README has no cd commands"
    assert all(t.startswith("$GXANA_OUTPUT/") for t in cds), cds


def test_referenced_macros_exist():
    for rel in re.findall(r"\$GXANA_ROOT/(analyses/\S+?\.C)", README) + re.findall(r"'(analyses/\S+?\.C)\(", README):
        assert (ROOT / rel).is_file(), rel
