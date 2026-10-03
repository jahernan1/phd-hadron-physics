"""Release metadata: MIT license, upstream credits, citation, one version."""
import re
import subprocess
from pathlib import Path

import yaml

ROOT = Path(__file__).resolve().parents[1]


def _lock_entries():
    data = yaml.safe_load((ROOT / "packages/montecarlo/external.lock").read_text())
    entries = data.get("externals", data) if isinstance(data, dict) else data
    return list(entries.values()) if isinstance(entries, dict) else entries


def test_license_is_mit():
    text = (ROOT / "LICENSE").read_text()
    assert text.startswith("MIT License")
    assert "Jesse A. Hernandez" in text


def test_notice_credits_every_upstream():
    notice = (ROOT / "NOTICE.md").read_text()
    for entry in _lock_entries():
        assert entry["url"].rstrip("/").removesuffix(".git") in notice, entry["name"]
    for must in ("https://github.com/lan13005/QFactors", "https://github.com/jahernan1/QFactors",
                 "gluex_root_analysis", "archive/", "packages/montecarlo/NOTICE.md", "CLAS"):
        assert must in notice, must


def test_notice_names_the_submodule_url():
    url = subprocess.run(["git", "config", "-f", ".gitmodules", "submodule.packages/qfactors.url"],
                         cwd=ROOT, capture_output=True, text=True, check=True).stdout.strip()
    assert url.removesuffix(".git") in (ROOT / "NOTICE.md").read_text()


def test_citation_cff():
    cff = yaml.safe_load((ROOT / "CITATION.cff").read_text())
    assert cff["cff-version"] == "1.2.0"
    assert cff["type"] == "software"
    assert cff["license"] == "MIT"
    assert cff["repository-code"] == "https://github.com/jahernan1/phd-hadron-physics"
    assert cff["preferred-citation"]["type"] == "thesis"
    notes = [r for r in cff.get("references", []) if r.get("type") == "report"]
    assert notes, "analysis-note reference"
    # The analysis note is internal to GlueX: a url is optional, but never empty
    # (the CFF schema rejects "").
    assert notes[0].get("url", "x"), "analysis-note url must be omitted, not empty"


def test_one_version_everywhere():
    py = re.search(r'^version\s*=\s*"([^"]+)"', (ROOT / "pyproject.toml").read_text(), re.M).group(1)
    cm = re.search(r"project\(gxana\s+VERSION\s+(\S+)", (ROOT / "CMakeLists.txt").read_text()).group(1)
    cff = str(yaml.safe_load((ROOT / "CITATION.cff").read_text())["version"])
    gxana_init = re.search(
        r'^__version__\s*=\s*"([^"]+)"',
        (ROOT / "packages/common/python/gxana/__init__.py").read_text(),
        re.M,
    ).group(1)
    assert py == cm == cff == gxana_init == "1.0.0"


def test_pyproject_license_and_author():
    text = (ROOT / "pyproject.toml").read_text()
    assert re.search(r'^license\s*=\s*\{\s*text\s*=\s*"MIT"\s*\}', text, re.M)
    assert 'name = "Jesse A. Hernandez"' in text
