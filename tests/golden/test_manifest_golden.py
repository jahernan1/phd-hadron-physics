"""Golden: files on disk match the sha256 recorded in analysis_data.yaml."""
import pytest

from gxana import analysis_data

pytestmark = pytest.mark.golden


def test_no_changed_files(golden):
    manifest = analysis_data.load_manifest("kpkpxim")
    changed = [s.path for s in analysis_data.status(manifest, golden) if s.state == "diff"]
    assert changed == [], "files differ from analysis_data.yaml: " + ", ".join(changed)
